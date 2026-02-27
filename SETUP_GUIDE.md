# ESPDenon232 Setup & Installation Guide

## Table of Contents
1. [Prerequisites](#prerequisites)
2. [Quick Start (5 minutes)](#quick-start-5-minutes)
3. [Manual Setup](#manual-setup)
4. [Wiring Diagram](#wiring-diagram)
5. [Configuration](#configuration)
6. [Troubleshooting](#troubleshooting)

## Prerequisites

### Required Hardware
- **ESP32 Development Board** (any variant: DevKit, WROVER, etc.)
- **Max3232 RS232 Level Converter IC** (TI MAX3232 or compatible like SP3232)
- **Denon AVR Receiver** with RS232 serial port
- **Capacitors**: 4× 1µF ceramic capacitors (for Max3232 stability)
- **DB9 Female Connector** (for RS232)
- **USB Cable** (for programming ESP32)
- **Breadboard/Perfboard** for connections
- **Jumper wires**

### Required Software
- **Python 3.7+**
- **ESPHome** (install via: `pip install esphome`)
- **Git** (optional, for cloning repository)

### Optional but Recommended
- **Home Assistant** (for full integration)
- **VS Code** with ESPHome extension
- **Serial Monitor** (for debugging)

## Quick Start (5 minutes)

### Step 1: Install ESPHome

```bash
# Install ESPHome globally
pip install esphome

# Verify installation
esphome version
```

### Step 2: Clone/Download Component

```bash
# Clone the repository
git clone https://github.com/Kousei-Uchu/ESPDenon232.git
cd ESPDenon232

# Or download as ZIP and extract
```

### Step 3: Run Quick Flash Script

**On Linux/macOS:**
```bash
chmod +x quick_flash.sh
./quick_flash.sh my-denon-controller
```

**On Windows (PowerShell):**
```powershell
python quick_flash.py my-denon-controller
```

**On Mac/Linux (Python):**
```bash
python3 quick_flash.py my-denon-controller
```

### Step 4: Follow Prompts

The script will:
1. ✓ Detect your ESP32 (or let you select port)
2. ✓ Create configuration files
3. ✓ Validate setup
4. ✓ Compile and flash firmware

**Total time: 2-5 minutes depending on internet speed**

## Manual Setup

### Step 1: Set Up Directory Structure

```
your-esphome-config/
├── denon-controller.yaml
├── secrets.yaml
└── components/
    └── denon232/
        ├── __init__.py
        ├── media_player.py
        ├── denon232_receiver.h
        ├── denon232_receiver.cpp
        ├── media_player.h
        ├── media_player.cpp
        ├── web_server.h
        ├── web_server.cpp
        ├── debug_monitor.h
        └── debug_monitor.cpp
```

### Step 2: Create Configuration Files

**denon-controller.yaml:**
```yaml
esphome:
  name: denon-controller
  friendly_name: "Denon Receiver Controller"
  platform: esp32
  board: esp32dev

wifi:
  ssid: !secret wifi_ssid
  password: !secret wifi_password
  
  ap:
    ssid: "Denon-Fallback"
    password: "denonfallback"

captive_portal:

api:
  encryption:
    key: !secret api_encryption_key

ota:
  password: !secret ota_password

logger:
  level: DEBUG
  logs:
    denon232: DEBUG

web_server:
  port: 80
  version: 3

uart:
  id: denon232_uart
  tx_pin: GPIO17
  rx_pin: GPIO16
  baud_rate: 9600
  data_bits: 8
  parity: NONE
  stop_bits: 1

denon232:
  uart_id: denon232_uart
  cable_mode: null_modem

media_player:
  - platform: denon232
    id: denon_receiver
    name: "Denon Receiver"
    polling_interval: 5000
```

**secrets.yaml:**
```yaml
wifi_ssid: "YOUR_WIFI_SSID"
wifi_password: "YOUR_WIFI_PASSWORD"
api_encryption_key: "YOUR_GENERATED_KEY"
ota_password: "esphome"
```

### Step 3: Generate API Key

```bash
esphome new-secret
# This generates a random encryption key - copy it to secrets.yaml
```

### Step 4: Validate Configuration

```bash
esphome config denon-controller.yaml
```

### Step 5: Connect Hardware & Flash

```bash
# Compile and flash
esphome run denon-controller.yaml

# Or specify serial port explicitly
esphome run denon-controller.yaml --device /dev/ttyUSB0
```

## Wiring Diagram

### ESP32 to Max3232

```
ESP32 (3.3V)          Max3232 IC          Power Supply
──────────────────────────────────────────────────────
GPIO17 (TX)  ───────→ T1IN (Pin 11)
GPIO16 (RX)  ←─────── R1OUT (Pin 12)
GND          ───────→ GND (Pin 15)
5V           ───────→ VCC (Pin 16)

Max3232 Capacitors (1µF ceramic):
Between VCC (16) and GND (15): 1µF
Between Pins 2-3 (Charge pump): 1µF
Between Pins 4-5 (Charge pump): 1µF
Between any unused charge pump pair: 1µF
```

### Max3232 to Denon RS232 (DB9 Female)

```
Max3232                    Denon DB9 Female
────────────────────────────────────────
T1OUT (Pin 10)  ──────→  Pin 2 (RX)
R1IN (Pin 13)   ────────→ Pin 3 (TX)
GND             ───────→ Pin 5 (GND)

DB9 Female Pinout (looking at port):
   1 2 3 4 5
    \ | | | /
     \| | |/
      ┌─────┐
      │ DCD │ 1
      │ RXD │ 2  ← Connect from Max3232 T1OUT
      │ TXD │ 3  ← Connect to Max3232 R1IN
      │ DTR │ 4
      │ GND │ 5  ← Connect Max3232 GND
      │ DSR │ 6
      │ RTS │ 7
      │ CTS │ 8
      │ RI  │ 9
      └─────┘
```

### Cable Type Configuration

**Null Modem Cable (Recommended - Standard):**
```
ESP32 TX (GPIO17) → Max3232 T1IN → Max3232 T1OUT → Denon RX (Pin 2)
ESP32 RX (GPIO16) → Max3232 R1OUT ← Max3232 R1IN ← Denon TX (Pin 3)

Configuration:
cable_mode: null_modem  # Default
```

**Pass-Through Cable (Straight):**
```
ESP32 TX (GPIO17) → Max3232 R1IN → Max3232 R1OUT → Denon TX (Pin 3)
ESP32 RX (GPIO16) → Max3232 T1OUT ← Max3232 T1IN ← Denon RX (Pin 2)

Configuration:
cable_mode: pass_through  # For straight cables
```

## Configuration

### Basic Configuration

```yaml
denon232:
  uart_id: denon232_uart
  cable_mode: null_modem      # null_modem or pass_through
```

### Media Player Options

```yaml
media_player:
  - platform: denon232
    id: denon_receiver
    name: "Denon Receiver"
    polling_interval: 5000    # milliseconds (5 seconds)
    cable_mode: null_modem    # Can override here too
```

### Enable Debug Monitor

```yaml
# Add to your config for enhanced debugging
denon232_debug:
  receiver_id: denon232
  debug_level: DEBUG           # ERROR, WARNING, INFO, DEBUG, VERBOSE
  enable_serial_debug: true
  enable_state_tracking: true
```

## Troubleshooting

### Device Won't Flash

**Problem:** "esphome command not found" or similar

**Solution:**
```bash
# Ensure ESPHome is installed
pip install --upgrade esphome

# Try with python module
python -m esphome run denon-controller.yaml
```

### No Serial Port Detected

**Problem:** Script can't find ESP32

**Troubleshooting:**
1. **Check USB connection:** Try different USB port
2. **Install drivers:**
   - **Windows:** [CH340 driver](https://github.com/nodemcu/ch340g-ch340-ch340c-mac-linux-driver)
   - **Mac:** `brew install wch-ch34x-usb-serial-driver`
   - **Linux:** Usually built-in, or `sudo apt install ch340-dkms`

3. **List ports manually:**
   ```bash
   # Linux/Mac
   ls /dev/tty.* /dev/cu.* /dev/ttyUSB* /dev/ttyACM*
   
   # Windows (PowerShell)
   [System.IO.Ports.SerialPort]::GetPortNames()
   ```

### Device Flashes but Won't Boot

**Problem:** Device restarts immediately after flashing

**Solutions:**
1. Check serial cable connections (especially GND)
2. Try different USB power source (use powered hub)
3. Check UART pins in configuration match physical setup
4. Use serial monitor to see boot messages:
   ```bash
   esphome logs denon-controller.yaml --device /dev/ttyUSB0
   ```

### No WiFi Connection

**Problem:** Device boots but doesn't connect to WiFi

**Solutions:**
1. Check `secrets.yaml` has correct SSID and password
2. Ensure WiFi is 2.4GHz (ESP32 doesn't support 5GHz)
3. Check password has no special characters (or quote them)
4. Use captive portal to connect:
   - Look for "Denon-Fallback" WiFi network
   - Connect with password: "denonfallback"
   - Open browser to `http://192.168.4.1`

### No Denon Communication

**Problem:** Device connects to WiFi but can't control receiver

**Troubleshooting:**
1. **Check wiring:**
   ```bash
   # Use serial monitor to test
   esphome logs denon-controller.yaml
   
   # Look for debug messages about UART communication
   ```

2. **Verify cable type:**
   - Test with null_modem first
   - If not working, try pass_through

3. **Check Denon settings:**
   - Verify RS232 port is enabled in receiver menu
   - Try manually sending command via serial monitor:
     ```
     PWON\r     (Turn on)
     PW?\r      (Query power state)
     ```

4. **Test with simple serial monitor:**
   ```bash
   # Linux/Mac
   screen /dev/ttyUSB0 9600
   
   # Windows (use PuTTY)
   # Set: Serial line, Speed 9600, 8N1
   ```

### Volume Not Updating

**Problem:** Can set volume but doesn't read back correctly

**Solution:** Check max volume setting:
```yaml
# In media_player section
polling_interval: 3000  # Poll more frequently
```

Query max volume via debug console:
```
Query: volume_query
Response should show: MV##MVMAX##
```

### Web Interface Not Accessible

**Problem:** Can't reach `http://device_ip/denon`

**Solutions:**
1. Check device is on same network as computer
2. Find device IP:
   ```bash
   # Use mDNS (if supported)
   ping denon-controller.local
   
   # Or check router's DHCP client list
   ```

3. Check web_server is enabled in config:
   ```yaml
   web_server:
     port: 80
     version: 3
   ```

### Commands Execute but Receiver Doesn't Respond

**Problem:** No errors but receiver doesn't change

**Troubleshooting:**
1. Check command is valid for your receiver model
2. Enable debug mode to see exact commands sent:
   ```yaml
   logger:
     level: VERY_VERBOSE
     logs:
       denon232: DEBUG
   ```

3. Test manually with serial command:
   ```
   PWON\r      → Should turn on
   MVUP\r      → Should increase volume
   SI?\r       → Should query current source
   ```

## Advanced Debugging

### Enable Serial Logging

```bash
esphome logs denon-controller.yaml --device /dev/ttyUSB0
```

Watch for these debug messages:
- `[denon232] Command: ...` - Commands being sent
- `[denon232] Response: ...` - Responses received
- `[denon232] Error: ...` - Communication errors

### Download Debug Log from Web Interface

1. Go to device web interface: `http://device_ip`
2. Click "Debug Console"
3. Click "Download Log" to save full debugging session

### Check Hardware Connection

```bash
# Use serial monitor at 9600 baud
# Send test command: PWON\r
# Should respond with acknowledgment

# If no response:
# 1. Check TX/RX connections aren't swapped
# 2. Check GND connection
# 3. Check Max3232 power supply (should be 5V)
```

## Next Steps

1. **Add to Home Assistant:**
   - Settings → Devices & Services → ESPHome
   - Add device by IP or wait for auto-discovery

2. **Create Automations:**
   ```yaml
   automation:
     - alias: Turn on Denon at sunset
       trigger:
         event: sun_event_sunset
       action:
         service: media_player.turn_on
         entity_id: media_player.denon_receiver
   ```

3. **Access Web Panel:**
   - Navigate to `http://device_ip/denon`
   - Full control with real-time status
   - Debug console for troubleshooting

4. **Configure Remote:**
   - Use remote.send_command service
   - Send any Denon command from automations

## Support & Documentation

- **GitHub Issues:** Report bugs or request features
- **Home Assistant Community:** Discuss integrations
- **Denon Protocol:** See command documentation
- **ESPHome Docs:** https://esphome.io

## License

MIT License - See LICENSE file in repository
