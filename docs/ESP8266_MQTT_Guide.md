# ESP8266 MQTT Connection Guide

## 1. Hardware Connection

### 1.1 ESP8266 Module Wiring

| ESP8266 Pin | STM32 Pin | Description |
|-------------|-----------|-------------|
| VCC | 3.3V | Power supply |
| GND | GND | Ground |
| TX | PA3 (USART2_RX) | ESP8266 TX to STM32 RX |
| RX | PA2 (USART2_TX) | ESP8266 RX to STM32 TX |
| EN/CH_PD | 3.3V | Enable pin (must be high) |
| RST | Optional | Reset pin |

**Important Notes**:
- ESP8266 requires **3.3V** power supply (not 5V!)
- Add a 100nF capacitor between VCC and GND for stability
- TX/RX are crossed: ESP8266 TX connects to STM32 RX

### 1.2 Debug UART Connection

| STM32 Pin | USB-TTL | Description |
|-----------|---------|-------------|
| PA9 (USART1_TX) | RX | Debug output |
| PA10 (USART1_RX) | TX | Debug input |
| GND | GND | Ground |

**Baud Rate**: 115200

---

## 2. Software Configuration

### 2.1 WiFi Settings

File: `NET/src/esp8266.h`

```c
/* WiFi Configuration */
#define ESP8266_WIFI_INFO  "AT+CWJAP=\"YOUR_SSID\",\"YOUR_PASSWORD\"\r\n"
```

**Example**:
```c
#define ESP8266_WIFI_INFO  "AT+CWJAP=\"Tenda_mqtt\",\"12345678\"\r\n"
```

### 2.2 MQTT Server Settings

File: `NET/src/esp8266.h`

```c
/* MQTT Server Configuration */
#define ESP8266_EMQX_INFO  "AT+CIPSTART=\"TCP\",\"broker.emqx.io\",1883\r\n"
```

**Options**:
| Server | Address | Port |
|--------|---------|------|
| EMQX Public | broker.emqx.io | 1883 |
| Local Server | 192.168.x.x | 1883 |

### 2.3 MQTT Credentials

File: `NET/src/onenet.c`

```c
#define DEVICE_NAME    "bmm150_device"
#define MQTT_USER      "bmm150"
#define MQTT_PASSWORD  "bmm150123"
```

---

## 3. Data Format

### 3.1 Publish Topic

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

| Field | Type | Range | Description |
|-------|------|-------|-------------|
| x | int16 | -200~200 | Magnetic field X axis |
| y | int16 | -200~200 | Magnetic field Y axis |
| z | int16 | -300~300 | Magnetic field Z axis |
| mag | int32 | 0~500 | Total magnitude sqrt(x²+y²+z²) |

### 3.2 Subscribe Topic

**Topic**: `sl_sub`

**Control LED**:
```json
{"led": 1}   // LED ON
{"led": 0}   // LED OFF
```

---

## 4. Initialization Flow

### 4.1 ESP8266 Init Sequence

```
1. AT Test                    → Check module response
2. Set Station Mode           → AT+CWMODE=1
3. Enable DHCP                → AT+CWDHCP=1,1
4. Connect WiFi               → AT+CWJAP="SSID","PASSWORD"
5. Connect MQTT Server        → AT+CIPSTART="TCP","broker.emqx.io",1883
```

### 4.2 MQTT Connection

```
1. MQTT Connect               → Send CONNECT packet
2. Subscribe Topic            → Subscribe to "sl_sub"
3. Start Publishing           → Publish data every 1 second
4. Heartbeat                  → Send PING every 30 seconds
```

---

## 5. MQTT Client Testing

### 5.1 Using MQTTX (Recommended)

Download: https://mqttx.app/

**Connection Settings**:
- Name: BMM150_Test
- Protocol: mqtt://
- Host: broker.emqx.io
- Port: 1883
- Username: bmm150
- Password: bmm150123

**Subscribe to data**:
- Topic: `sl_pst`
- See real-time sensor data

**Control LED**:
- Topic: `sl_sub`
- Message: `{"led": 1}` or `{"led": 0}`

### 5.2 Using Python Script

```python
import paho.mqtt.client as mqtt

def on_connect(client, userdata, flags, rc):
    print(f"Connected with result code {rc}")
    client.subscribe("sl_pst")

def on_message(client, userdata, msg):
    print(f"Topic: {msg.topic}")
    print(f"Payload: {msg.payload.decode()}")

client = mqtt.Client()
client.username_pw_set("bmm150", "bmm150123")
client.on_connect = on_connect
client.on_message = on_message

client.connect("broker.emqx.io", 1883, 60)
client.loop_forever()
```

---

## 6. Debug Output

### 6.1 Normal Boot Sequence

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
4. Connect WiFi: Tenda_mqtt
   WiFi Connected
5. ESP8266 Init OK
6. Connect MQTT Server
   MQTT Server Connected

MQTT Connect...
   MQTT Connected OK
Subscribe Topic: sl_sub
MQTT Subscribe OK

Start sending fake data...

[0] X:157 Y:-98 Z:245
Upload: {"x":157,"y":-98,"z":245,"mag":298}
```

### 6.2 Error Messages

| Message | Cause | Solution |
|---------|-------|----------|
| AT timeout | No response | Check wiring/power |
| WIFI timeout | Wrong SSID/PWD | Check WiFi config |
| MQTT timeout | Server unreachable | Check server address |
| Protocol Error | Wrong packet | Check MQTT config |
| User/Password Error | Wrong credentials | Check MQTT credentials |

---

## 7. Common Issues

### 7.1 ESP8266 Not Responding

**Symptoms**: "1. AT Test" loops forever

**Check**:
1. Power supply is 3.3V (not 5V)
2. EN/CH_PD pin connected to 3.3V
3. TX/RX wiring is correct (crossed)
4. Baud rate is 115200

### 7.2 WiFi Connection Failed

**Symptoms**: "4. Connect WiFi" loops forever

**Check**:
1. WiFi SSID and password are correct
2. WiFi network is 2.4GHz (not 5GHz)
3. ESP8266 is within WiFi range
4. Router allows new devices

### 7.3 MQTT Connection Failed

**Symptoms**: "MQTT timeout" or "User/Password Error"

**Check**:
1. MQTT server address is correct
2. Port 1883 is not blocked by firewall
3. Username/password match server config
4. Server is online

### 7.4 Serial Output Garbled

**Symptoms**: Chinese text shows as gibberish

**Solution**:
- All debug messages use English to avoid encoding issues
- Set serial tool to UTF-8 encoding if needed

---

## 8. Code Structure

### 8.1 File Organization

```
BMM150/
├── src/
│   └── main.c           # Main program, UART init
├── NET/
│   ├── inc/
│   │   ├── esp8266.h    # ESP8266 driver header
│   │   ├── onenet.h     # MQTT functions header
│   │   ├── MqttKit.h    # MQTT protocol header
│   │   └── cJSON.h      # JSON parser header
│   └── src/
│   │   ├── esp8266.c    # ESP8266 driver
│   │   ├── onenet.c     # MQTT publish/subscribe
│   │   ├── MqttKit.c    # MQTT packet builder
│   │   └── cJSON.c      # JSON parser
└── platformio.ini       # Build configuration
```

### 8.2 Key Functions

| Function | File | Description |
|----------|------|-------------|
| ESP8266_Init() | esp8266.c | Initialize ESP8266 module |
| OneNet_DevLink() | onenet.c | Connect to MQTT server |
| OneNet_SendData() | onenet.c | Publish sensor data |
| OneNet_RevPro() | onenet.c | Handle received messages |
| DebugPrint() | main.c | Debug output via UART1 |

---

## 9. Build and Upload

### 9.1 Commands

```bash
# Build only
pio run

# Build and upload
pio run -t upload

# Clean build
pio run -t clean

# Monitor serial output
pio device monitor -b 115200
```

### 9.2 Build Output

```
RAM:   [=         ]   6.3% (used 1292 bytes from 20480 bytes)
Flash: [====      ]   39.7% (used 26024 bytes from 65536 bytes)
```

---

## 10. References

- ESP8266 AT Commands: https://docs.espressif.com/projects/esp-at/en/latest/
- MQTT Protocol: https://mqtt.org/
- EMQX Public Broker: https://www.emqx.com/en/mqtt/public-mqtt5-broker
- PlatformIO Documentation: https://docs.platformio.org/