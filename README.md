### 🔌 PYTES Battery to Home Assistant via ESPHome

ESPHome custom component for monitoring **PYTES LiFePO4 battery systems** 🔋 via RS232 serial port. This application runs on an ESP32 board 📡, reads the battery console port and integrates automatically with Home Assistant 🏠.

Support for up to 6️⃣ individual battery modules with real-time monitoring of voltage ⚡, current 🔌, temperature 🌡️, SoC 📊, and status 🚥.

-----

## ✨ Features

- 📊 **Real-time monitoring** of battery bank and individual battery metrics
- 🔋 **Multi-battery support** (up to 6 battery modules)
- 🌡️ **Temperature monitoring** for each battery module
- ⚡ **Power flow tracking** (voltage, current, power)
- 📈 **State of Charge (SoC)** percentage for bank and individual batteries
- 🚥 **Status monitoring** (operational state, voltage status, alerts)
- 🔌 **Native Home Assistant integration** via ESPHome API
- 🌐 **Built-in web server** for standalone monitoring
- 🔄 **Configurable polling interval** (default 30s)
- 🆔 **Serial number and firmware tracking** for each battery
- 📏 **Configurable battery capacity** for accurate calculations

-----

## 🔌 Supported Hardware

### 🔋 Tested Pytes Battery Models

- ✅ **Pytes V5°** (48V 5kWh module)
- ✅ **Pytes E-Box-48100R** (48V 10kWh system)
- ✅ **Pytes E-Box-12100** (12V systems)
- ✅ **Pytes Pi-LV1** (Low voltage systems)

> 💡 **Note:** This component should work with any Pytes battery system that supports the RS232 console protocol.

### 🖥️ ESP32 Development Boards

- ✅ ESP32-DevKitC
- ✅ ESP32-WROOM-32
- ✅ ESP32-WROVER
- ✅ Any ESP32 board with ESP-IDF framework support (minimum chip revision 3.1)

-----

## 🛠️ Hardware Requirements

### 📦 Required Components

|Component                |Quantity|Description                |Example/Notes                    |
|-------------------------|--------|---------------------------|---------------------------------|
|🖥️ **ESP32 DevKit**       |1       |ESP32 development board    |Minimum chip revision 3.1        |
|🔋 **Pytes Battery**      |1-6     |LiFePO4 battery system     |With RS232 console port          |
|🔌 **RS232-to-TTL Module**|1       |MAX3232 or equivalent      |3.3V logic level                 |
|🔗 **Dupont Wires**       |~10     |Jumper cables              |Female-to-female                 |
|🔌 **USB Cable**          |1       |For ESP32 power/programming|Micro-USB or USB-C               |
|⚡ **Power Supply**       |1       |5V power adapter           |Optional for standalone operation|

### 🎁 Optional Components

- 📦 **Enclosure** - To protect the ESP32 and connections
- 🏭 **DIN rail mount** - For professional installation
- 💡 **Status LEDs** - For visual monitoring

-----

## 🔌 Wiring Diagram

### 📌 Pin Connections

```
┌─────────────────┐              ┌──────────────────┐
│   ESP32 DevKit  │              │  MAX3232 Module  │
│      🖥️         │              │   (RS232-TTL)    │
├─────────────────┤              ├──────────────────┤
│ GPIO16 (RX)  ───┼──────────────┤ TX (TTL out)     │
│ GPIO17 (TX)  ───┼──────────────┤ RX (TTL in)      │
│ GND          ───┼──────────────┤ GND              │
│ 3.3V         ───┼──────────────┤ VCC (3.3V)       │
└─────────────────┘              └──────────────────┘
                                          │
                                          │ 🔌 DB9 cable
                                          ▼
                                 ┌──────────────────┐
                                 │  Pytes Battery   │
                                 │  🔋 RS232 Console │
                                 └──────────────────┘
```

### 📋 Detailed Pin Mapping

|ESP32 GPIO|Function|MAX3232 Pin|Pytes Console|
|----------|--------|-----------|-------------|
|📍 GPIO16  |UART RX |TX (TTL)   |TX (RS232)   |
|📍 GPIO17  |UART TX |RX (TTL)   |RX (RS232)   |
|⚫ GND     |Ground  |GND        |GND          |
|🔴 3.3V    |Power   |VCC        |-            |

### ⚙️ UART Configuration

- ⚡ **Baud Rate:** 115200
- 📊 **Data Bits:** 8
- ❌ **Parity:** None
- 🛑 **Stop Bits:** 1
- 🔀 **Flow Control:** None

-----

## 📥 Installation

### 🌐 Method 1: Using GitHub (Recommended)

Add this to your ESPHome YAML configuration:

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/s3pt1c0/Pytes_ESP32_RS232
      ref: main
    components: [ pytes_rs232 ]
    refresh: 1d
```

### 💾 Method 2: Local Installation

1️⃣ Clone or download this repository
2️⃣ Copy the `Components/pytes_rs232` folder to your ESPHome configuration directory:

```bash
mkdir -p /config/esphome/components
cp -r Pytes_ESP32_RS232/Components/pytes_rs232 /config/esphome/components/
```

3️⃣ Reference in your YAML:

```yaml
external_components:
  - source:
      type: local
      path: components
```

-----

## ⚙️ Configuration

### 🚀 Basic Configuration

Create a file `pytes-esp32.yaml`:

```yaml
substitutions:
  device_name: pytes-esp32
  friendly_name: 🔋Pytes Battery Monitor🔋
  poll_s: "30"              # ⏱️ Polling interval in seconds
  capacity_ah: "100"        # 🔋 Battery capacity in Ah
  num_batts: "6"            # 🔢 Number of battery modules

esphome:
  name: ${device_name}
  friendly_name: ${friendly_name}
  comment: "Pytes Battery Monitoring via RS232"

esp32:
  board: esp32dev
  framework:
    type: esp-idf
    advanced: 
      minimum_chip_revision: "3.1"

# 📝 Enable logging
logger:
  level: INFO

# 🌐 Enable Web Server (optional)
web_server:
  port: 80

# 🏠 Enable Home Assistant API
api:
  encryption:
    key: "YOUR_ENCRYPTION_KEY_HERE"

# 🔄 Enable OTA updates
ota:
  - platform: esphome
    password: "YOUR_OTA_PASSWORD_HERE"

# 📡 WiFi configuration
wifi:
  ssid: !secret wifi_ssid
  password: !secret wifi_password
  
  # 🆘 Enable fallback hotspot (optional)
  ap:
    ssid: "${device_name} Fallback"
    password: "fallback12345"

# 🔌 UART Configuration for Pytes RS232
uart:
  id: uart_pytes
  tx_pin: GPIO17
  rx_pin: GPIO16
  baud_rate: 115200
  data_bits: 8
  parity: NONE
  stop_bits: 1

# 🔋 Pytes RS232 Component Configuration
pytes_rs232:
  uart_id: uart_pytes
  num_batteries: ${num_batts}
  update_interval: ${poll_s}s
  capacity_ah: ${capacity_ah}
  
  # 📊 Summary sensors (entire battery bank)
  summary:
    voltage: sys_voltage
    current: sys_current
    temperature: sys_temp_f
    coulomb: sys_coulomb
    basic_status: sys_basic_status
  
  # 🔋 Individual battery sensors (repeat for each battery)
  batteries:
    - voltage: b1_voltage
      current: b1_current
      temperature: b1_temp_f
      coulomb: b1_coulomb
      barcode: b1_barcode
      devtype: b1_devtype
      firm_version: b1_firmver
      basic_status: b1_basic_status
      volt_status: b1_volt_status
    # ... repeat for batteries 2-6
```

### 🔐 Secrets Configuration

Create or edit `secrets.yaml`:

```yaml
wifi_ssid: "YourWiFiSSID"
wifi_password: "YourWiFiPassword"
```

-----

## 📊 Available Sensors

### 📈 Summary (Bank-Level) Sensors

|Sensor Name       |Type  |Unit|Description                                   |
|------------------|------|----|----------------------------------------------|
|⚡ `Bank Voltage ⚠️`|Number|V   |Total battery bank voltage                    |
|🔌 `Bank Current⚡` |Number|A   |Total bank current (+ charging, - discharging)|
|🌡️ `Bank Temp🌡️`    |Number|°F  |Average bank temperature                      |
|📊 `Bank SoC📈`     |Number|%   |State of Charge (0-100%)                      |
|🚥 `Bank Status🚥`  |Text  |-   |Operational status                            |

### 🔋 Individual Battery Sensors (B1-B6)

|Sensor Name          |Type  |Unit|Description                   |
|---------------------|------|----|------------------------------|
|⚡ `B[X] Volt⚠️`       |Number|V   |Individual battery voltage    |
|🔌 `B[X] Current⚡`    |Number|A   |Individual battery current    |
|🌡️ `B[X] Temp🌡️`       |Number|°F  |Individual battery temperature|
|📊 `B[X] SoC📈`        |Number|%   |Individual State of Charge    |
|🔢 `B[X] Serial#️⃣`     |Text  |-   |Battery serial/barcode        |
|🆔 `B[X] Model🆔`      |Text  |-   |Battery model/type            |
|⚙️ `B[X] Firmware⚙️`   |Text  |-   |Firmware version              |
|🚥 `B[X] Status🚥`     |Text  |-   |Operational status            |
|📊 `B[X] Volt Status📊`|Text  |-   |Voltage status details        |

### 📝 Sensor Template Definitions

All sensors are defined as templates in your YAML:

```yaml
sensor:
  # 📊 Summary sensors
  - platform: template
    name: "Bank Voltage ⚠️"
    id: sys_voltage
    unit_of_measurement: "V"
    accuracy_decimals: 2

  - platform: template
    name: "Bank Current⚡"
    id: sys_current
    unit_of_measurement: "A"
    accuracy_decimals: 2

  # 🔋 Battery 1 sensors
  - platform: template
    name: "B1 Volt⚠️"
    id: b1_voltage
    unit_of_measurement: "V"
    accuracy_decimals: 2

  # ... additional sensors

text_sensor:
  # 🚥 Summary status
  - platform: template
    name: "Bank Status🚥"
    id: sys_basic_status

  # 🔋 Battery 1 info
  - platform: template
    name: "B1 Serial#️⃣"
    id: b1_barcode

  # ... additional text sensors
```

-----

## 🏠 Home Assistant Integration

### 🔍 Automatic Discovery

Once deployed, the device will automatically appear in Home Assistant under:

**⚙️ Settings → 🔌 Devices & Services → 🏠 ESPHome**

### 📱 Creating Dashboard Cards

#### 🔋 Battery Overview Card

```yaml
type: entities
title: 🔋 Pytes Battery Bank
entities:
  - entity: sensor.pytes_esp32_bank_voltage
    name: ⚡ Voltage
  - entity: sensor.pytes_esp32_bank_current
    name: 🔌 Current
  - entity: sensor.pytes_esp32_bank_soc
    name: 📊 State of Charge
  - entity: sensor.pytes_esp32_bank_temp
    name: 🌡️ Temperature
  - entity: sensor.pytes_esp32_bank_status
    name: 🚥 Status
```

#### 🔋 Individual Battery Card

```yaml
type: glance
title: 🔋 Individual Battery Status
entities:
  - entity: sensor.pytes_esp32_b1_soc
    name: Battery 1️⃣
  - entity: sensor.pytes_esp32_b2_soc
    name: Battery 2️⃣
  - entity: sensor.pytes_esp32_b3_soc
    name: Battery 3️⃣
  - entity: sensor.pytes_esp32_b4_soc
    name: Battery 4️⃣
  - entity: sensor.pytes_esp32_b5_soc
    name: Battery 5️⃣
  - entity: sensor.pytes_esp32_b6_soc
    name: Battery 6️⃣
```

#### ⚡ Power Calculation Helper

Create a template sensor in Home Assistant for power calculation:

```yaml
template:
  - sensor:
      - name: "Pytes Battery Power"
        unique_id: pytes_battery_power
        unit_of_measurement: "W"
        device_class: power
        state: >
          {% set v = states('sensor.pytes_esp32_bank_voltage')|float(0) %}
          {% set a = states('sensor.pytes_esp32_bank_current')|float(0) %}
          {{ (v * a) | round(1) }}
```

### 📊 Energy Dashboard Integration

To add battery data to the Energy Dashboard:

1️⃣ Go to **⚙️ Settings → 📊 Dashboards → ⚡ Energy**
2️⃣ Click **➕ Add Battery System**
3️⃣ Select your power sensor (created above)
4️⃣ Configure charge/discharge sensors if available

-----

## 🔧 Troubleshooting

### ❌ No Data from Batteries

**🔴 Symptom:** All sensors show “unavailable” or “unknown”

**✅ Solutions:**

1. ✅ Check physical wiring connections (TX/RX crossed)
1. ✅ Verify RS232-TTL module power (3.3V not 5V)
1. ✅ Confirm baud rate is 115200
1. ✅ Check ESPHome logs for UART errors
1. ✅ Verify Pytes console port is active (some models require activation)

### ⚠️ Wrong Data or Garbled Output

**🔴 Symptom:** Sensors show incorrect values or random characters

**✅ Solutions:**

1. ✅ Double-check TX/RX wiring (may be swapped)
1. ✅ Ensure ground connection between ESP32 and RS232 module
1. ✅ Verify 8N1 UART configuration
1. ✅ Check for loose connections
1. ✅ Try reducing update interval (increase `poll_s`)

### 📡 Device Disconnects from WiFi

**🔴 Symptom:** ESP32 goes offline periodically

**✅ Solutions:**

1. ✅ Move ESP32 closer to WiFi router
1. ✅ Use stronger WiFi antenna
1. ✅ Set static IP in router
1. ✅ Enable AP fallback mode
1. ✅ Check power supply (use quality 5V 2A adapter)

### 🔋 Only Bank Sensors Work, Individual Batteries Don’t

**🔴 Symptom:** Summary sensors update but B1-B6 sensors remain unavailable

**✅ Solutions:**

1. ✅ Verify `num_batteries` matches actual battery count
1. ✅ Check that all battery IDs are defined in configuration
1. ✅ Ensure batteries are properly connected in series
1. ✅ Review ESPHome logs for parsing errors

### 🌡️ High Temperature Readings

**🔴 Symptom:** Temperature sensors show unusually high values

**✅ Solutions:**

1. ✅ Check if temperature is in Fahrenheit (default) or Celsius
1. ✅ Verify battery ventilation
1. ✅ Confirm sensor calibration
1. ✅ Check for actual overheating condition

### 💻 Compilation Errors

**🔴 Symptom:** ESPHome fails to compile the configuration

**✅ Solutions:**

1. ✅ Update ESPHome to version 2025.1 or newer
1. ✅ Verify ESP-IDF framework is selected (not Arduino)
1. ✅ Check minimum chip revision setting (3.1)
1. ✅ Ensure all required IDs are defined in sensor templates
1. ✅ Review YAML syntax for errors

### 📝 Viewing Logs

To troubleshoot issues, view real-time logs:

```bash
# 🖥️ From ESPHome dashboard
esphome logs pytes-esp32.yaml

# 📡 Or via network
esphome logs pytes-esp32.yaml --device pytes-esp32.local
```

-----

## 🎨 Advanced Configuration

### 🌡️ Adding BME280 Temperature Sensor (Optional)

To monitor ambient temperature near the batteries:

```yaml
i2c:
  sda: GPIO21
  scl: GPIO22
  scan: true
  id: bus_a

sensor:
  # ... existing sensors ...
  
  # 🌡️ BME280 sensors
  - platform: bme280
    temperature:
      name: "🌡️ Ambient Temperature"
      id: ambient_temp
      oversampling: 16x
    pressure:
      name: "🌀 Ambient Pressure"
      id: ambient_pressure
    humidity:
      name: "💧 Ambient Humidity"
      id: ambient_humidity
    address: 0x76  # or 0x77
    update_interval: 60s
```

### ⏱️ Custom Update Intervals per Sensor

```yaml
pytes_rs232:
  # ... existing config ...
  
  # ⚡ Fast update for critical sensors
  update_interval: 10s
  
# 🐌 Slower update for less critical info
text_sensor:
  - platform: template
    name: "B1 Serial#️⃣"
    id: b1_barcode
    update_interval: 300s  # Update every 5 minutes
```

### 🔔 Alerts and Notifications

Create automations in Home Assistant:

```yaml
automation:
  - alias: "⚠️ Battery Low SoC Alert"
    trigger:
      - platform: numeric_state
        entity_id: sensor.pytes_esp32_bank_soc
        below: 20
    action:
      - service: notify.mobile_app
        data:
          title: "⚠️ Battery Low"
          message: "Battery SoC is below 20%"

  - alias: "🔥 Battery High Temperature Alert"
    trigger:
      - platform: numeric_state
        entity_id: sensor.pytes_esp32_bank_temp
        above: 113  # 45°C in Fahrenheit
    action:
      - service: notify.mobile_app
        data:
          title: "🔥 Battery Overheating"
          message: "Battery temperature exceeds safe limits!"
```

-----

## 🏗️ Repository Structure

```
Pytes_ESP32_RS232/
├── 📁 Components/
│   └── 📁 pytes_rs232/
│       ├── 📄 __init__.py
│       ├── 📄 pytes_rs232.cpp
│       ├── 📄 pytes_rs232.h
│       ├── 📄 sensor.py
│       └── 📄 text_sensor.py
├── 📁 examples/
│   ├── 📄 basic-config.yaml
│   ├── 📄 6-battery-system.yaml
│   └── 📄 with-bme280.yaml
├── 📁 docs/
│   ├── 🖼️ wiring-diagram.png
│   ├── 📄 installation-guide.md
│   └── 📄 troubleshooting.md
├── 📄 LICENSE
└── 📄 README.md
```

-----

## 🤝 Contributing

Contributions are welcome! Here’s how you can help:

1. 🍴 **Fork** the repository
1. 🌿 **Create** a feature branch (`git checkout -b feature/amazing-feature`)
1. ✍️ **Commit** your changes (`git commit -m 'Add amazing feature'`)
1. 📤 **Push** to the branch (`git push origin feature/amazing-feature`)
1. 🔀 **Open** a Pull Request

### 💡 Areas for Improvement

- 📖 Additional documentation and examples
- 🐛 Bug fixes and testing
- ✨ New features (Celsius support, additional sensors)
- 🌍 Multi-language support
- 🎨 Dashboard templates

-----

## 📜 License

This project is licensed under the MIT License - see the <LICENSE> file for details.

-----

## 🙏 Acknowledgments

- 🔋 **Pytes** - For manufacturing quality LiFePO4 battery systems
- 🏠 **ESPHome** - For the excellent home automation framework
- 🏡 **Home Assistant** - For the best home automation platform
- 🖥️ **ESP32 Community** - For extensive documentation and support
- 👨‍💻 **chinezbrun** - For inspiration from [pytes_esp](https://github.com/chinezbrun/pytes_esp)

-----

## 📞 Support

- 🐛 **Issues:** [GitHub Issues](https://github.com/s3pt1c0/Pytes_ESP32_RS232/issues)
- 💬 **Discussions:** [GitHub Discussions](https://github.com/s3pt1c0/Pytes_ESP32_RS232/discussions)
- ⭐ **Star this repo** if you find it useful!

-----

## ⚠️ Disclaimer

This is an independent project and is **not officially affiliated** with or endorsed by Pytes. Use at your own risk. Always follow proper safety procedures when working with battery systems.

-----

## 🔄 Changelog

### Version 1.0.0 (2026-01-31)

- ✅ Initial release
- ✅ Support for up to 6 battery modules
- ✅ RS232 communication via UART
- ✅ Home Assistant integration
- ✅ Web server interface
- ✅ OTA updates

-----

**Made with ❤️ for the Home Assistant and Solar/Battery community**

🎉 **Enjoy!** 🎉

### How does this software work?

ESPHome component reads the RS232 serial port with configurable frequency (default 30s), parses the battery data and sends it to Home Assistant via the ESPHome API. Home Assistant autodiscovery is built-in, so all sensors will show up automatically.

A signal converter from RS232 to TTL is needed between Pytes serial console port and ESP32 UART pins.

Program creates the following sensors for each battery:

- Bank level: voltage, current, temperature, SoC, status
- Individual batteries (1-6): voltage, current, temperature, SoC, serial#, model, firmware, status, volt_status

### Hardware used:

**ESP32 board** –––––––––––––– tested on ESP32 WROOM 38 pins / ESP32 DevKitC

**RS232 to TTL converter** —————– tested on MAX3232 module

**RJ45 to RS232 DB9(male) cable** –––– tested on standard console cable

Simple connection flow:

```
ESP32 [UART pins GPIO16/17] -> TTL to RS232 converter -> DB09 to RJ45 cable -> Pytes RJ45 console port
```

**Connection with RS232 to TTL converter using RJ45 to RS232 DB9(male) cable - TESTED:**

```
ESP32 Pin: 16 RXD2 ------ RX converter  RX  DB9 Female ------ DB9 Male console cable RJ45 Pin: 3 TXD 
ESP32 Pin: 17 TXD2 ------ TX converter  TX  DB9 Female ------ DB9 Male console cable RJ45 Pin: 6 RXD 
ESP32 PIN:     GND ------ GND converter GND DB9 Female ------ DB9 Male console cable RJ45 Pin: 4 GND 
ESP32 PIN:    3.3V ------ VCC converter     DB9 Female ------ DB9 Male console cable RJ45
```

**UART Configuration:**

- Baud Rate: 115200
- Data Bits: 8
- Parity: NONE
- Stop Bits: 1

**Important:** Both connection types work, but respect the PYTES console port pinout. Main source of issues is incorrect TX/RX connection. If something goes wrong, swap RXD and TXD on one side.

### Installation and Execution

**Method 1: Using GitHub external component (Recommended)**

Add to your ESPHome YAML configuration:

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/s3pt1c0/Pytes_ESP32_RS232
      ref: main
    components: [ pytes_rs232 ]
    refresh: 1d
```

**Method 2: Local component**

1. Clone or download this repository:

```bash
git clone https://github.com/s3pt1c0/Pytes_ESP32_RS232.git
```

1. Copy `Components/pytes_rs232` folder to your ESPHome config directory:

```bash
mkdir -p /config/esphome/components
cp -r Pytes_ESP32_RS232/Components/pytes_rs232 /config/esphome/components/
```

1. Reference in your YAML:

```yaml
external_components:
  - source:
      type: local
      path: components
```

### Basic Configuration Example

Create `pytes-esp32.yaml` file:

```yaml
substitutions:
  device_name: pytes-esp32
  friendly_name: 🔋Pytes Battery Monitor🔋
  poll_s: "30"              # Polling interval in seconds
  capacity_ah: "100"        # Battery capacity in Ah
  num_batts: "6"            # Number of battery modules (1-6)

esphome:
  name: ${device_name}
  friendly_name: ${friendly_name}

esp32:
  board: esp32dev
  framework:
    type: esp-idf
    advanced: 
      minimum_chip_revision: "3.1"

logger:
  level: INFO

web_server:
  port: 80

api:
  encryption:
    key: "YOUR_ENCRYPTION_KEY"

ota:
  - platform: esphome
    password: "YOUR_OTA_PASSWORD"

wifi:
  ssid: !secret wifi_ssid
  password: !secret wifi_password

# UART for Pytes RS232
uart:
  id: uart_pytes
  tx_pin: GPIO17
  rx_pin: GPIO16
  baud_rate: 115200
  data_bits: 8
  parity: NONE
  stop_bits: 1

# Pytes Component
pytes_rs232:
  uart_id: uart_pytes
  num_batteries: ${num_batts}
  update_interval: ${poll_s}s
  capacity_ah: ${capacity_ah}
  
  summary:
    voltage: sys_voltage
    current: sys_current
    temperature: sys_temp_f
    coulomb: sys_coulomb
    basic_status: sys_basic_status
  
  batteries:
    - voltage: b1_voltage
      current: b1_current
      temperature: b1_temp_f
      coulomb: b1_coulomb
      barcode: b1_barcode
      devtype: b1_devtype
      firm_version: b1_firmver
      basic_status: b1_basic_status
      volt_status: b1_volt_status
    # Repeat for batteries 2-6...

# Define template sensors
sensor:
  - platform: template
    name: "Bank Voltage ⚠️"
    id: sys_voltage
    unit_of_measurement: "V"
    accuracy_decimals: 2
  
  - platform: template
    name: "Bank Current⚡"
    id: sys_current
    unit_of_measurement: "A"
    accuracy_decimals: 2
  # ... more sensors

text_sensor:
  - platform: template
    name: "Bank Status🚥"
    id: sys_basic_status
  # ... more text sensors
```

Complete example with all sensors defined can be found in `examples/` folder.

### Normal Operation

When ESP32 is powered and configured:

1. Device connects to WiFi
1. Establishes UART communication with Pytes battery
1. Reads serial data at configured interval (default 30s)
1. Automatically creates device and sensors in Home Assistant
1. Updates sensor values continuously

After firmware updates it is recommended to delete previous devices in Home Assistant and let them be recreated at next boot.

### Available Sensors

**Bank Level (Summary):**

- Bank Voltage (V)
- Bank Current (A)
- Bank Temperature (°F)
- Bank SoC (%)
- Bank Status

**Individual Batteries (B1-B6):**

- Voltage (V)
- Current (A)
- Temperature (°F)
- SoC (%)
- Serial Number
- Model/Device Type
- Firmware Version
- Basic Status
- Voltage Status

### Home Assistant Integration

Device will automatically appear in Home Assistant under **Settings → Devices & Services → ESPHome** after first connection.

Example dashboard card:

```yaml
type: entities
title: 🔋 Pytes Battery
entities:
  - entity: sensor.pytes_esp32_bank_voltage
  - entity: sensor.pytes_esp32_bank_current
  - entity: sensor.pytes_esp32_bank_soc
  - entity: sensor.pytes_esp32_bank_temp
  - entity: sensor.pytes_esp32_bank_status
```

### Troubleshooting

**No data from batteries / All sensors “unavailable”**

- ✅ Check TX/RX wiring (may need to be swapped)
- ✅ Verify RS232-TTL module power (3.3V not 5V)
- ✅ Confirm baud rate is 115200
- ✅ Check ESPHome logs for errors
- ✅ Verify Pytes console port is enabled

**Wrong data or garbled output**

- ✅ Swap TX/RX connections on one side
- ✅ Ensure ground connection between ESP32 and converter
- ✅ Check for loose connections
- ✅ Try increasing polling interval

**WiFi disconnections**

- ✅ Move ESP32 closer to router
- ✅ Use quality 5V 2A power supply
- ✅ Set static IP in router

**Viewing logs**

```bash
esphome logs pytes-esp32.yaml
```

-----

## 🏗️ Repository Structure

```
Pytes_ESP32_RS232/
├── Components/
│   └── pytes_rs232/
│       ├── __init__.py
│       ├── pytes_rs232.cpp
│       ├── pytes_rs232.h
│       ├── sensor.py
│       └── text_sensor.py
├── examples/
│   ├── basic-config.yaml
│   └── 6-battery-system.yaml
├── docs/
│   ├── wiring-diagram.png
│   └── installation-guide.md
├── LICENSE
└── README.md
```

-----

## 📜 License

This project is licensed under the MIT License - see the <LICENSE> file for details.

-----

## 📞 Support & Community

- 🐛 **Issues:** [GitHub Issues](https://github.com/s3pt1c0/Pytes_ESP32_RS232/issues)
- 💬 **Discussions:** [GitHub Discussions](https://github.com/s3pt1c0/Pytes_ESP32_RS232/discussions)
- ⭐ **Star this repo** if you find it useful!

-----

## 🙏 Acknowledgments

- **Pytes** - For manufacturing quality LiFePO4 battery systems
- **ESPHome** - For the excellent home automation framework
- **Home Assistant** - For the best home automation platform
- **ESP32 Community** - For extensive documentation and support
- **chinezbrun** - For inspiration from [pytes_esp](https://github.com/chinezbrun/pytes_esp)

-----

## ⚠️ Disclaimer

This is an independent project and is **not officially affiliated** with or endorsed by Pytes. Use at your own risk. Always follow proper safety procedures when working with battery systems.

-----

**Made with ❤️ for the Home Assistant and Solar/Battery community**

enjoy