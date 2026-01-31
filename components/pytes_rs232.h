#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"

#include <string>
#include <vector>

namespace esphome {
namespace pytes_rs232 {

struct BatterySensors {
  esphome::sensor::Sensor *voltage{nullptr};
  esphome::sensor::Sensor *soc_voltage{nullptr};
  esphome::sensor::Sensor *current{nullptr};
  esphome::sensor::Sensor *temp_f{nullptr};
  esphome::sensor::Sensor *soc_pct{nullptr};

  esphome::text_sensor::TextSensor *barcode{nullptr};
  esphome::text_sensor::TextSensor *devtype{nullptr};
  esphome::text_sensor::TextSensor *firm_version{nullptr};
  esphome::text_sensor::TextSensor *basic_status{nullptr};
  esphome::text_sensor::TextSensor *volt_status{nullptr};

  // last values (for fallbacks)
  float last_voltage{NAN};
  float last_soc_voltage{NAN};
  float last_current{NAN};
  float last_temp_f{NAN};
  float last_soc{NAN};
  std::string last_barcode;
  std::string last_devtype;
  std::string last_firm;
  std::string last_basic;
  std::string last_volt;
};

class PytesRS232 : public esphome::PollingComponent, public esphome::uart::UARTDevice {
 public:
  explicit PytesRS232(esphome::uart::UARTComponent *parent) : UARTDevice(parent) {}

  void set_num_batteries(int n) { this->num_batteries_ = n; this->bats_.resize(n); }
  void set_capacity_ah(float ah) { this->capacity_ah_ = ah; }

  void set_summary(esphome::sensor::Sensor *v, esphome::sensor::Sensor *c, esphome::sensor::Sensor *t,
                   esphome::sensor::Sensor *soc, esphome::text_sensor::TextSensor *basic) {
    this->sys_voltage_ = v;
    this->sys_current_ = c;
    this->sys_temp_f_ = t;
    this->sys_soc_ = soc;
    this->sys_basic_status_ = basic;
  }

  void set_battery(int idx,
                   esphome::sensor::Sensor *v, esphome::sensor::Sensor *soc_v, esphome::sensor::Sensor *cur,
                   esphome::sensor::Sensor *tmp_f, esphome::sensor::Sensor *soc,
                   esphome::text_sensor::TextSensor *barcode, esphome::text_sensor::TextSensor *devtype,
                   esphome::text_sensor::TextSensor *firm, esphome::text_sensor::TextSensor *basic,
                   esphome::text_sensor::TextSensor *volt) {
    if (idx < 1 || idx > this->num_batteries_) return;
    auto &b = this->bats_[idx - 1];
    b.voltage = v;
    b.soc_voltage = soc_v;
    b.current = cur;
    b.temp_f = tmp_f;
    b.soc_pct = soc;
    b.barcode = barcode;
    b.devtype = devtype;
    b.firm_version = firm;
    b.basic_status = basic;
    b.volt_status = volt;
  }

  void setup() override;
  void update() override;
  void dump_config() override;

 protected:
  int num_batteries_{6};
  float capacity_ah_{100.0f};

  std::vector<BatterySensors> bats_;

  esphome::sensor::Sensor *sys_voltage_{nullptr};
  esphome::sensor::Sensor *sys_current_{nullptr};
  esphome::sensor::Sensor *sys_temp_f_{nullptr};
  esphome::sensor::Sensor *sys_soc_{nullptr};
  esphome::text_sensor::TextSensor *sys_basic_status_{nullptr};

  // Low-level helpers
  void flush_input_();
  void send_cmd_(const std::string &cmd);
  std::vector<std::string> read_block_(uint32_t max_ms = 5000, uint32_t idle_gap_ms = 350);

  // Parse + publish
  void parse_and_publish_(int idx, const std::vector<std::string> &lines);
  static bool icontains_(const std::string &hay, const std::string &needle);
  static std::string trim_(std::string s);
  static float c_to_f_(float c) { return (c * 9.0f / 5.0f) + 32.0f; }

  static bool parse_millivolts_(const std::string &s, float *out_v);
  static bool parse_milliamps_(const std::string &s, float *out_a);
  static bool parse_millic_(const std::string &s, float *out_c);
  static bool parse_percent_(const std::string &s, float *out_pct);
  static bool parse_milliamp_hours_(const std::string &s, float *out_ah);

  void publish_summary_();
};

}  // namespace pytes_rs232
}  // namespace esphome
