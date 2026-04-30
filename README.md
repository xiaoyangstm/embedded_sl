# BMM150 Magnetometer MQTT Project

STM32F103C8T6 + BMM150 + ESP8266 MQTT Data Upload System

---

## Project Overview

This project implements a magnetic field sensor system that:
- Reads BMM150 3-axis magnetometer data
- Transmits data to MQTT server via ESP8266 WiFi module
- Supports remote LED control through MQTT subscription

---

## Hardware Requirements

| Component | Model | Description |
|-----------|-------|-------------|
| MCU | STM32F103C8T6 (BluePill) | 72MHz, 64KB Flash |
| Sensor | BMM150 | 3-axis magnetometer |
| WiFi Module | ESP8266 | ESP-01 or ESP-12 series |
| Debug UART | USB-TTL | 115200 baud rate |

---

## 1. Hardware Connection

### 1.1 BMM150 Sensor Wiring

| BMM150 Pin | STM32 Pin | Description |
|------------|-----------|-------------|
| VCC | 3.3V | Power supply |
| GND | GND | Ground |
| SDA | PB7 (I2C1_SDA) | I2C data |
| SCL | PB6 (I2C1_SCL) | I2C clock |
| PS | GND | I2C mode select |
| CSB | VCC | Chip select (I2C mode) |
| SDO | GND | I2C address bit (0x10) |

**I2C Address**: `0x10` (when SDO = GND)

### 1.2 ESP8266 WiFi Module Wiring

| ESP8266 Pin | STM32 Pin | Description |
|-------------|-----------|-------------|
| VCC | 3.3V | Power supply |
| GND | GND | Ground |
| TX | PA3 (USART2_RX) | ESP TX to STM32 RX |
| RX | PA2 (USART2_TX) | ESP RX to STM32 TX |
| EN/CH_PD | 3.3V | Enable pin (must be high) |

**Important Notes**:
- ESP8266 requires **3.3V** (NOT 5V!)
- TX/RX are **crossed**: ESP TX → STM32 RX
- Add 100nF capacitor between VCC and GND

### 1.3 Debug UART Wiring

| STM32 Pin | USB-TTL | Description |
|-----------|---------|-------------|
| PA9 (TX) | RX | Debug output |
| PA10 (RX) | TX | Debug input |
| GND | GND | Ground |

---

## 2. Software Configuration

### 2.1 WiFi Settings

Edit `NET/inc/esp8266.h`:

```c
#define ESP8266_WIFI_INFO  "AT+CWJAP=\"YOUR_SSID\",\"YOUR_PASSWORD\"\r\n"
```

Example:
```c
#define ESP8266_WIFI_INFO  "AT+CWJAP=\"Tenda_mqtt\",\"12345678\"\r\n"
```

### 2.2 MQTT Server Settings

Edit `NET/inc/esp8266.h`:

```c
#define ESP8266_EMQX_INFO  "AT+CIPSTART=\"TCP\",\"broker.emqx.io\",1883\r\n"
```

| Server | Address | Port |
|--------|---------|------|
| EMQX Public | broker.emqx.io | 1883 |
| Local Server | Your IP | 1883 |

### 2.3 MQTT Credentials

Edit `NET/src/onenet.c`:

```c
#define DEVICE_NAME    "bmm150_device"
#define MQTT_USER      "bmm150"
#define MQTT_PASSWORD  "bmm150123"
```

---

## 3. BMM150 Sensor Usage

### 3.1 Initialization

```c
#include "bmm150.h"

// Initialize BMM150
if (BMM150_Init() != 0) {
    printf("BMM150 Init Failed!\n");
    return -1;
}
```

### 3.2 Read Magnetometer Data

```c
int16_t x, y, z;
uint16_t status;

// Perform self-test first
BMM150_SelfTest();

// Read magnetic field data
BMM150_GetData(&x, &y, &z);

// Calculate magnitude
int32_t mag = sqrt(x*x + y*y + z*z);
```

### 3.3 Data Range

| Axis | Range | Typical Earth Field |
|------|-------|---------------------|
| X | ±200 | ±25 uT |
| Y | ±200 | ±25 uT |
| Z | ±300 | ±65 uT |
| Mag | 0~500 | 25~65 uT |

### 3.4 Self-Test

```c
// Check sensor functionality
if (BMM150_SelfTest() == 0) {
    printf("BMM150 Self-Test OK\n");
} else {
    printf("BMM150 Self-Test Failed\n");
}
```

### 3.5 Calibration Notes

- Earth magnetic field: **25-65 uT**
- Hard iron offset calibration required
- Soft iron distortion calibration recommended
- Keep away from ferromagnetic materials

---

## 4. MQTT Data Transmission

### 4.1 Publish Topic

**Topic**: `sl_pst`

**JSON Format**:
```json
{
    "x": 157,
    "y": -98,
    "z": 245,
    "mag": 298
}
```

| Field | Type | Description |
|-------|------|-------------|
| x | int16 | X-axis magnetic field |
| y | int16 | Y-axis magnetic field |
| z | int16 | Z-axis magnetic field |
| mag | int32 | Total magnitude sqrt(x²+y²+z²) |

### 4.2 Subscribe Topic

**Topic**: `sl_sub`

**Control LED**:
```json
{"led": 1}   // Turn ON LED
{"led": 0}   // Turn OFF LED
```

---

## 5. Initialization Process

### 5.1 ESP8266 Initialization Flow

```
1. AT Test                    → Check module response
2. Set Station Mode           → AT+CWMODE=1
3. Enable DHCP                → AT+CWDHCP=1,1
4. Connect WiFi               → AT+CWJAP="SSID","PWD"
5. Connect MQTT Server        → AT+CIPSTART="TCP","server",1883
```

### 5.2 MQTT Connection Flow

```
1. MQTT Connect               → Send CONNECT packet
2. Subscribe Topic            → Subscribe "sl_sub"
3. Start Publishing           → Publish data every 1 second
4. Heartbeat                  → Send PING every 30 seconds
```

### 5.3 Normal Debug Output

```
USART1 Test OK!
System Init...
USART2 Init OK

=================================
STM32F103C8T6 @ 72MHz
ESP8266 MQTT Test
=================================

Initializing ESP8266...
1. AT Test
   AT OK
2. Set Station Mode
   CWMODE OK
3. Enable DHCP
   CWDHCP OK
4. Connect WiFi: Your_SSID
   WiFi Connected
5. ESP8266 Init OK
6. Connect MQTT Server
   MQTT Server Connected

MQTT Connect...
   MQTT Connected OK
Subscribe Topic: sl_sub
MQTT Subscribe OK

Start sending data...

[0] X:157 Y:-98 Z:245 Mag:298
Upload: {"x":157,"y":-98,"z":245,"mag":298}
```

---

## 6. Testing with MQTT Client

### 6.1 Using MQTTX (Recommended)

Download: https://mqttx.app/

**Connection Settings**:
- Name: BMM150_Test
- Host: broker.emqx.io
- Port: 1883
- Username: bmm150
- Password: bmm150123

**Subscribe to sensor data**:
```
Topic: sl_pst
```

**Control LED**:
```
Topic: sl_sub
Message: {"led": 1}  // LED ON
Message: {"led": 0}  // LED OFF
```

### 6.2 Python Test Script

```python
import paho.mqtt.client as mqtt
import json

def on_connect(client, userdata, flags, rc):
    print(f"Connected: {rc}")
    client.subscribe("sl_pst")

def on_message(client, userdata, msg):
    data = json.loads(msg.payload.decode())
    print(f"X:{data['x']} Y:{data['y']} Z:{data['z']} Mag:{data['mag']}")

client = mqtt.Client()
client.username_pw_set("bmm150", "bmm150123")
client.on_connect = on_connect
client.on_message = on_message
client.connect("broker.emqx.io", 1883, 60)
client.loop_forever()
```

---

## 7. Build and Upload

### 7.1 Prerequisites

- Python 3.x installed
- PlatformIO installed: `pip install platformio`

### 7.2 Build Commands

```bash
# Build project
pio run

# Build and upload to STM32
pio run -t upload

# Clean build
pio run -t clean

# Serial monitor
pio device monitor -b 115200
```

### 7.3 Build Output

```
RAM:   [=         ]   6.3% (used 1292 bytes from 20480 bytes)
Flash: [====      ]   39.7% (used 26024 bytes from 65536 bytes)
```

---

## 8. Common Issues & Solutions

### 8.1 BMM150 Not Detected

**Symptoms**: I2C scan fails, no sensor found

**Check**:
- PS pin connected to GND
- CSB pin connected to VCC
- SDO pin connected to GND (address 0x10)
- I2C pull-up resistors (4.7kΩ)

### 8.2 ESP8266 Not Responding

**Symptoms**: "AT Test" loops forever

**Check**:
- Power is 3.3V (not 5V)
- EN/CH_PD connected to 3.3V
- TX/RX wiring crossed correctly
- Baud rate is 115200

### 8.3 WiFi Connection Failed

**Symptoms**: "Connect WiFi" loops forever

**Check**:
- Correct SSID and password
- WiFi is 2.4GHz (not 5GHz)
- ESP8266 within WiFi range

### 8.4 MQTT Connection Failed

**Symptoms**: MQTT timeout

**Check**:
- Server address correct
- Port 1883 not blocked
- Username/password match

### 8.5 Serial Output Garbled

**Solution**: All messages use English to avoid encoding issues

---

## 9. Project Structure

```
BMM150/
├── src/
│   └── main.c              # Main program
├── NET/
│   ├── inc/
│   │   ├── esp8266.h       # ESP8266 driver header
│   │   ├── onenet.h        # MQTT functions header
│   │   ├── MqttKit.h       # MQTT protocol header
│   │   └── cJSON.h         # JSON parser header
│   └── src/
│   │   ├── esp8266.c       # ESP8266 driver
│   │   ├── onenet.c        # MQTT functions
│   │   ├── MqttKit.c       # MQTT packet builder
│   │   └── cJSON.c         # JSON parser
├── docs/
│   └ ESP8266_MQTT_Guide.md # Detailed MQTT guide
├── bmm150.c                # BMM150 driver
├── bmm150.h                # BMM150 header
└── platformio.ini          # Build config
```

---

## 10. Key Functions

| Function | File | Description |
|----------|------|-------------|
| BMM150_Init() | bmm150.c | Initialize BMM150 sensor |
| BMM150_GetData() | bmm150.c | Read X/Y/Z magnetic data |
| BMM150_SelfTest() | bmm150.c | Perform sensor self-test |
| ESP8266_Init() | esp8266.c | Initialize ESP8266 WiFi |
| OneNet_DevLink() | onenet.c | Connect to MQTT server |
| OneNet_SendData() | onenet.c | Publish sensor data |
| OneNet_RevPro() | onenet.c | Handle received commands |
| DebugPrint() | main.c | Debug output via UART |

---

## 11. References

- BMM150 Datasheet: [Bosch BMM150](https://www.bosch-sensortec.com/products/motion-sensors/magnetometers/bmm150/)
- ESP8266 AT Commands: [Espressif Docs](https://docs.espressif.com/projects/esp-at/)
- MQTT Protocol: [mqtt.org](https://mqtt.org/)
- EMQX Public Broker: [EMQX](https://www.emqx.com/en/mqtt/public-mqtt5-broker)
- PlatformIO: [docs.platformio.org](https://docs.platformio.org/)

---

## 12. License

MIT License

---

## Author

xiaoyangstm