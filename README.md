### PYTES Battery to Home Assistant via ESPHome

ESPHome custom component for monitoring **PYTES LiFePO4 battery systems** via RS232 serial port. This application runs on an ESP32 board, reads the battery console port and integrates automatically with Home Assistant.

Support for up to 6 individual battery modules with real-time monitoring of voltage, current, temperature, SoC, and status.

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
