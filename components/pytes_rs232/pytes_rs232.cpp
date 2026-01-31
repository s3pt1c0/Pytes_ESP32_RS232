#include "pytes_rs232.h"

#include "esphome/core/log.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cmath>

namespace esphome {
namespace pytes_rs232 {

static const char *const TAG = "pytes_rs232";

void PytesRS232::setup() {
  this->bats_.resize(this->num_batteries_);
  this->flush_input_();
}

void PytesRS232::dump_config() {
  ESP_LOGCONFIG(TAG, "Pytes RS232");
  ESP_LOGCONFIG(TAG, "  Batteries: %d", this->num_batteries_);
  ESP_LOGCONFIG(TAG, "  Capacity: %.1f Ah", this->capacity_ah_);
}

void PytesRS232::flush_input_() {
  while (this->available()) {
    this->read();
    delay(1);
  }
}

void PytesRS232::send_cmd_(const std::string &cmd) {
  this->flush_input_();
  this->write_str(cmd.c_str());
  this->write_str("\r\n");
}

std::vector<std::string> PytesRS232::read_block_(uint32_t max_ms, uint32_t idle_gap_ms) {
  std::vector<std::string> out;
  uint32_t t0 = millis();
  uint32_t last = t0;

  std::string line;
  while ((millis() - t0) < max_ms) {
    bool got = false;

    while (this->available()) {
      char ch;
      if (!this->read_byte((uint8_t *) &ch)) break;
      got = true;

      if (ch == '\r') continue;
      if (ch == '\n') {
        if (!line.empty()) {
          out.push_back(line);
          std::string low = line;
          std::transform(low.begin(), low.end(), low.begin(), [](unsigned char c) { return (char) std::tolower(c); });
          if (low.find("command completed") != std::string::npos || low == "ok") {
            return out;
          }
          line.clear();
        }
      } else {
        // Keep it ASCII-ish
        if ((unsigned char) ch >= 32 || ch == '\t')
          line.push_back(ch);
      }
      last = millis();
    }

    if (!got) {
      if ((millis() - last) >= idle_gap_ms) break;
      delay(10);
    }
  }

  if (!line.empty()) out.push_back(line);
  return out;
}

bool PytesRS232::icontains_(const std::string &hay, const std::string &needle) {
  if (needle.empty()) return true;
  auto it = std::search(hay.begin(), hay.end(), needle.begin(), needle.end(),
                        [](char a, char b) { return std::tolower((unsigned char) a) == std::tolower((unsigned char) b); });
  return it != hay.end();
}

std::string PytesRS232::trim_(std::string s) {
  auto not_space = [](int ch) { return !std::isspace(ch); };
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), not_space));
  s.erase(std::find_if(s.rbegin(), s.rend(), not_space).base(), s.end());
  return s;
}

static bool find_int_token_(const std::string &s, const std::string &unit, int *out) {
  auto pos = std::search(s.begin(), s.end(), unit.begin(), unit.end(),
                         [](char a, char b) { return std::tolower((unsigned char) a) == std::tolower((unsigned char) b); });
  if (pos == s.end()) return false;

  // walk backwards from pos to find number
  int i = (int) (pos - s.begin()) - 1;
  while (i >= 0 && std::isspace((unsigned char) s[i])) i--;

  int end = i;
  while (i >= 0 && (std::isdigit((unsigned char) s[i]) || s[i] == '-' )) i--;
  int start = i + 1;
  if (start > end) return false;

  char *ep = nullptr;
  const std::string sub = s.substr(start, end - start + 1);
  long v = strtol(sub.c_str(), &ep, 10);
  if (ep == sub.c_str() || *ep != '\0') return false;
  *out = (int) v;
  return true;
}

bool PytesRS232::parse_millivolts_(const std::string &s, float *out_v) {
  int mv;
  if (!find_int_token_(s, "mV", &mv) && !find_int_token_(s, "mv", &mv)) return false;
  *out_v = ((float) mv) / 1000.0f;
  return true;
}

bool PytesRS232::parse_milliamps_(const std::string &s, float *out_a) {
  int ma;
  if (!find_int_token_(s, "mA", &ma) && !find_int_token_(s, "ma", &ma)) return false;
  *out_a = ((float) ma) / 1000.0f;
  return true;
}

bool PytesRS232::parse_millic_(const std::string &s, float *out_c) {
  int mc;
  if (!find_int_token_(s, "mC", &mc) && !find_int_token_(s, "mc", &mc)) return false;
  *out_c = ((float) mc) / 1000.0f;
  return true;
}

bool PytesRS232::parse_percent_(const std::string &s, float *out_pct) {
  // find a number followed by %
  auto pctpos = s.find('%');
  if (pctpos == std::string::npos) return false;
  int i = (int) pctpos - 1;
  while (i >= 0 && std::isspace((unsigned char) s[i])) i--;
  int end = i;
  while (i >= 0 && (std::isdigit((unsigned char) s[i]) || s[i] == '-' )) i--;
  int start = i + 1;
  if (start > end) return false;
  char *ep = nullptr;
  const std::string sub = s.substr(start, end - start + 1);
  long v = strtol(sub.c_str(), &ep, 10);
  if (ep == sub.c_str() || *ep != '\0') return false;
  *out_pct = (float) v;
  return true;
}

bool PytesRS232::parse_milliamp_hours_(const std::string &s, float *out_ah) {
  int mah;
  if (!find_int_token_(s, "mAh", &mah) && !find_int_token_(s, "mah", &mah)) return false;
  *out_ah = ((float) mah) / 1000.0f;
  return true;
}

void PytesRS232::parse_and_publish_(int idx, const std::vector<std::string> &lines) {
  if (idx < 1 || idx > this->num_batteries_) return;
  auto &b = this->bats_[idx - 1];

  float voltage = NAN;
  float soc_voltage = NAN;
  float current = NAN;
  float temp_c = NAN;
  float soc_pct = NAN;
  float soc_from_mah = NAN;

  std::string barcode, devtype, firm, basic, voltst;

  for (const auto &raw : lines) {
    auto s = trim_(raw);
    if (s.empty()) continue;

    std::string low = s;
    std::transform(low.begin(), low.end(), low.begin(), [](unsigned char c) { return (char) std::tolower(c); });

    // split label:value if present
    std::string label;
    std::string value;
    auto cpos = s.find(':');
    if (cpos != std::string::npos) {
      label = trim_(s.substr(0, cpos));
      value = trim_(s.substr(cpos + 1));
    }
    std::string labell = label;
    std::transform(labell.begin(), labell.end(), labell.begin(), [](unsigned char c) { return (char) std::tolower(c); });

    if (icontains_(low, "firm version")) {
      if (!value.empty()) firm = value;
      continue;
    }
    if (icontains_(low, "barcode")) {
      if (!value.empty()) barcode = value;
      continue;
    }
    if (icontains_(low, "devtype") || icontains_(low, "dev type")) {
      if (!value.empty()) devtype = value;
      continue;
    }

    if (icontains_(low, "basic") && icontains_(low, "status")) {
      if (!value.empty()) basic = value;
      continue;
    }
    if (icontains_(low, "volt") && icontains_(low, "status")) {
      if (!value.empty()) voltst = value;
      continue;
    }

    // Voltage (exclude SOC Voltage)
    if ((labell == "voltage") && !icontains_(low, "soc")) {
      if (std::isnan(voltage)) parse_millivolts_(low, &voltage);
      continue;
    }
    // SOC Voltage
    if (icontains_(low, "soc voltage") || labell == "soc voltage") {
      if (std::isnan(soc_voltage)) parse_millivolts_(low, &soc_voltage);
      continue;
    }

    // Current
    if (labell == "current" || (icontains_(low, "current") && !icontains_(low, "chg") && !icontains_(low, "dsg"))) {
      if (std::isnan(current)) parse_milliamps_(low, &current);
      continue;
    }

    // Temperature
    if (icontains_(low, "temperature")) {
      if (std::isnan(temp_c)) parse_millic_(low, &temp_c);
      continue;
    }

    // Coulomb / SOC
    if (icontains_(low, "coulomb") || labell == "soc") {
      if (std::isnan(soc_pct)) parse_percent_(low, &soc_pct);
      if (std::isnan(soc_from_mah)) parse_milliamp_hours_(low, &soc_from_mah);
      continue;
    }
  }

  // fallbacks to last
  if (std::isnan(voltage)) voltage = b.last_voltage;
  if (std::isnan(soc_voltage)) soc_voltage = b.last_soc_voltage;
  if (std::isnan(current)) current = b.last_current;
  if (std::isnan(temp_c)) temp_c = std::isnan(b.last_temp_f) ? NAN : ((b.last_temp_f - 32.0f) * 5.0f / 9.0f);

  // SOC: percent preferred, else compute from mAh
  if (std::isnan(soc_pct)) {
    if (!std::isnan(soc_from_mah) && this->capacity_ah_ > 0.0f) {
      soc_pct = std::round(std::max(0.0f, std::min(100.0f, (soc_from_mah / this->capacity_ah_) * 100.0f)));
    } else {
      soc_pct = b.last_soc;
    }
  }

  // Temperature to F
  float temp_f = NAN;
  if (!std::isnan(temp_c)) temp_f = c_to_f_(temp_c);
  else temp_f = b.last_temp_f;

  // Sign current using basic status if present
  float signed_current = current;
  std::string basic_l = basic.empty() ? b.last_basic : basic;
  std::string bl = basic_l;
  std::transform(bl.begin(), bl.end(), bl.begin(), [](unsigned char c) { return (char) std::tolower(c); });
  if (!std::isnan(current) && !bl.empty()) {
    if (bl.rfind("chg", 0) == 0 || bl.find("charge") != std::string::npos) signed_current = std::abs(current);
    else if (bl.rfind("dis", 0) == 0 || bl.rfind("dsg", 0) == 0 || bl.find("discharge") != std::string::npos) signed_current = -std::abs(current);
    else if (bl.rfind("idle", 0) == 0) signed_current = 0.0f;
  }

  // publish numeric
  if (!std::isnan(voltage) && b.voltage) b.voltage->publish_state(voltage);
  if (!std::isnan(soc_voltage) && b.soc_voltage) b.soc_voltage->publish_state(soc_voltage);
  if (!std::isnan(signed_current) && b.current) b.current->publish_state(signed_current);
  if (!std::isnan(temp_f) && b.temp_f) b.temp_f->publish_state(temp_f);
  if (!std::isnan(soc_pct) && b.soc_pct) b.soc_pct->publish_state(soc_pct);

  // publish text
  auto use_or_last = [](const std::string &v, const std::string &last) -> std::string { return v.empty() ? last : v; };
  barcode = use_or_last(barcode, b.last_barcode);
  devtype = use_or_last(devtype, b.last_devtype);
  firm = use_or_last(firm, b.last_firm);
  basic = use_or_last(basic, b.last_basic);
  voltst = use_or_last(voltst, b.last_volt);

  if (b.barcode && !barcode.empty()) b.barcode->publish_state(barcode);
  if (b.devtype && !devtype.empty()) b.devtype->publish_state(devtype);
  if (b.firm_version && !firm.empty()) b.firm_version->publish_state(firm);
  if (b.basic_status && !basic.empty()) b.basic_status->publish_state(basic);
  if (b.volt_status && !voltst.empty()) b.volt_status->publish_state(voltst);

  // store last
  b.last_voltage = voltage;
  b.last_soc_voltage = soc_voltage;
  b.last_current = signed_current;
  b.last_temp_f = temp_f;
  b.last_soc = soc_pct;
  b.last_barcode = barcode;
  b.last_devtype = devtype;
  b.last_firm = firm;
  b.last_basic = basic;
  b.last_volt = voltst;
}

void PytesRS232::publish_summary_() {
  // Compute summary based on last values
  float v_sum = 0.0f;
  float t_sum = 0.0f;
  float soc_sum = 0.0f;
  float i_sum = 0.0f;
  int n_v = 0, n_t = 0, n_soc = 0, n_i = 0;

  for (auto &b : this->bats_) {
    if (!std::isnan(b.last_voltage)) { v_sum += b.last_voltage; n_v++; }
    if (!std::isnan(b.last_temp_f)) { t_sum += b.last_temp_f; n_t++; }
    if (!std::isnan(b.last_soc)) { soc_sum += b.last_soc; n_soc++; }
    if (!std::isnan(b.last_current)) { i_sum += b.last_current; n_i++; }
  }

  if (this->sys_voltage_ && n_v > 0) this->sys_voltage_->publish_state(v_sum / n_v);
  if (this->sys_temp_f_ && n_t > 0) this->sys_temp_f_->publish_state(t_sum / n_t);
  if (this->sys_soc_ && n_soc > 0) this->sys_soc_->publish_state(std::round(soc_sum / n_soc));
  if (this->sys_current_ && n_i > 0) this->sys_current_->publish_state(i_sum);

  if (this->sys_basic_status_) {
    std::string st = "Idle";
    if (n_i > 0) {
      if (i_sum > 0.2f) st = "Charge";
      else if (i_sum < -0.2f) st = "Discharge";
    }
    this->sys_basic_status_->publish_state(st);
  }
}

void PytesRS232::update() {
  // poll all batteries in one cycle
  for (int i = 1; i <= this->num_batteries_; i++) {
    this->send_cmd_("pwr " + std::to_string(i));
    auto lines = this->read_block_();
    if (lines.empty()) {
      ESP_LOGW(TAG, "No data for battery %d", i);
      continue;
    }
    this->parse_and_publish_(i, lines);
    delay(50);
  }

  this->publish_summary_();
}

}  // namespace pytes_rs232
}  // namespace esphome
