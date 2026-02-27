# ESPDenon232 Debugging Guide

Complete troubleshooting and diagnostic procedures for ESPDenon232.

## Quick Diagnostics

### 1. Check Device is Online

```bash
# Ping the device
ping denon-controller.local

# Or use IP address
ping 192.168.x.x
```

### 2. View Serial Logs

```bash
# Real-time logs
esphome logs denon-controller.yaml --device /dev/ttyUSB0

# Save logs to file
esphome logs denon-controller.yaml --device /dev/ttyUSB0 > debug.log
```

### 3. Access Web Debug Console

1. Open browser: `http://device_ip`
2. Click "Debug Console" tab
3. Watch real-time commands and responses
4. Click "Download Log" to save session

## Debug Levels

### Configure Debug Output

```yaml
logger:
  level: DEBUG
  logs:
    denon232: DEBUG              # Core component
    denon232.media_player: DEBUG # Media player
    denon232.debug: DEBUG        # Debug monitor
    denon232.web_server: DEBUG   # Web server
```

### Log Levels (from least to most verbose)

1. **ERROR** - Only critical errors
2. **WARNING** - Warnings and errors
3. **INFO** - General information (default)
4. **DEBUG** - Detailed debugging info
5. **VERY_VERBOSE** - Maximum detail

## Common Issues & Diagnostics

### Issue: No Serial Communication

**Diagnostic Steps:**

1. **Check UART Configuration:**
   ```bash
   esphome config denon-controller.yaml | grep -A 10 "uart:"
   ```
   Expected output shows TX=GPIO17, RX=GPIO16, baud=9600

2. **Verify Physical Connections:**
   ```
   ESP32 GPIO17 ──→ Max3232 T1IN (Pin 11)
   ESP32 GPIO16 ←── Max3232 R1OUT (Pin 12)
   ESP32 GND ──→ Max3232 GND (Pin 15)
   ESP32 5V ──→ Max3232 VCC (Pin 16)
   ```

3. **Check Max3232 Capacitors:**
   - All four 1µF capacitors installed?
   - Check for loose connections

4. **Test with Serial Monitor:**
   - Use putty or screen at 9600 baud
   - Connect directly to Denon receiver
   - Send: `PWON\r`
   - Should get response or state change

### Issue: Commands Sent But No Response

**Diagnostic Checklist:**

```yaml
# 1. Enable verbose logging
logger:
  level: VERY_VERBOSE
  logs:
    denon232: VERBOSE

# 2. Check command is correct format
# Command should be: COMMAND\r
# Example: PWON\r, MV50\r, SI?\r

# 3. Verify Denon settings
# - Menu → Setup → Option → RS232 Control → Enable
# - Baud rate: 9600
# - Data bits: 8
# - Parity: None
# - Stop bits: 1
```

**Serial Test:**
```bash
# Use serial monitor (screen, miniterm, PuTTY)
# Connect to /dev/ttyUSB0 at 9600 baud

# Send commands:
PWON\r           # Turn on receiver
PW?\r            # Query power state (should respond)
MV?\r            # Query volume (should respond)
SI?\r            # Query source (should respond)
MVUP\r           # Volume up (no response expected)
```

### Issue: Incorrect Volume Reading

**Diagnostic Steps:**

1. **Query Max Volume:**
   ```bash
   # Via debug console or logs, look for:
   MV?
   # Response like: MV50MVMAX60
   # Max volume is 60 in this case
   ```

2. **Check Receiver MVMAX:**
   Some receivers use different ranges:
   ```yaml
   # Different max volumes by model:
   # AVR-2806: 98
   # AVR-3806: 98
   # AVR-4306: 98
   # AVR-X1200W: 98
   # Older models: 60
   ```

3. **Enable Volume Tracking:**
   ```yaml
   logger:
     logs:
       denon232.media_player: VERBOSE
   ```
   Look for: `Volume: ##/##` in logs

### Issue: Intermittent Connection Loss

**Diagnostic Procedure:**

1. **Check for Noise:**
   - Add shielded cable for RS232
   - Keep RS232 cable away from power cables
   - Add ferrite beads to signal wires

2. **Check Cable Quality:**
   - Test with shorter cable (< 5m)
   - Verify all pins connected
   - No bent or damaged connectors

3. **Monitor Connection Stability:**
   ```bash
   # Enable verbose logging
   esphome logs denon-controller.yaml --device /dev/ttyUSB0 | grep -E "(Error|UART|timeout)"
   ```

4. **Check Power Supply:**
   - ESP32 needs stable 5V power
   - Max3232 needs stable 5V power
   - Use powered USB hub if needed

### Issue: Web Interface Slow or Unresponsive

**Diagnostic:**

1. **Check Network:**
   ```bash
   ping -c 5 denon-controller.local
   
   # Look for consistent response times
   # High variance or timeouts = network issue
   ```

2. **Check Device Load:**
   ```bash
   esphome logs denon-controller.yaml | grep -E "(CPU|RAM|Heap)"
   ```

3. **Reduce Polling Frequency:**
   ```yaml
   media_player:
     - platform: denon232
       polling_interval: 10000  # Increase to 10 seconds
   ```

## Advanced Diagnostics

### Enable Full Component Diagnostics

```yaml
denon232_debug:
  receiver_id: denon232
  debug_level: VERBOSE
  enable_serial_debug: true
  enable_state_tracking: true
```

This provides:
- Command/response tracking
- State change monitoring
- Error counting
- Diagnostic reports

### Generate Diagnostic Report

Access via API endpoint:
```bash
curl http://device_ip/api/diagnostics
```

Returns:
- Component status
- Command statistics
- Recent log entries
- Hardware state

### Manual Command Testing

**Via Web Interface:**
1. Open `http://device_ip`
2. Go to Debug Console
3. Click command buttons to test
4. Watch debug output

**Via cURL:**
```bash
# Send power on command
curl -X POST http://device_ip/api/command/power_on

# Query power state
curl http://device_ip/api/query/power_query

# Get device status
curl http://device_ip/api/status
```

**Via Home Assistant:**
```yaml
# Service call to test
service: rest_command.denon_command
data:
  command: "PWON"
```

## Performance Optimization

### Reduce Polling Overhead

```yaml
# Default is 5000ms
media_player:
  - platform: denon232
    polling_interval: 10000  # 10 seconds instead of 5
```

### Optimize for Slow Networks

```yaml
web_server:
  port: 80
  version: 3  # Use REST API only, no WebSockets

logger:
  level: WARNING  # Reduce logging overhead
  logs:
    denon232: INFO
```

### Disable Unnecessary Logging

```yaml
logger:
  level: WARNING
  # Don't log these components
  logs:
    homeassistant: WARNING
    esphome.api: WARNING
```

## Collecting Debug Logs for Help

When reporting an issue, collect:

### 1. Configuration
```bash
# Device config (no sensitive data)
esphome config denon-controller.yaml > device_config.txt
```

### 2. Device Logs
```bash
# Full logs (30 seconds)
timeout 30 esphome logs denon-controller.yaml --device /dev/ttyUSB0 > device_logs.txt
```

### 3. Web Console Debug
```bash
# Visit http://device_ip
# Open Debug Console
# Let it run for 1 minute
# Click "Download Log"
# Save as web_debug.log
```

### 4. System Information
```bash
# ESPHome version
esphome version

# Python version
python --version

# OS info
# On Linux: cat /etc/os-release
# On Mac: system_profiler SPSoftwareDataType
# On Windows: systeminfo
```

### 5. Include in Bug Report
- device_config.txt
- device_logs.txt
- web_debug.log
- System info output
- Description of issue
- Steps to reproduce

## Serial Port Debugging

### Windows (Using PuTTY)

1. Download PuTTY
2. Select "Serial" connection type
3. Set COM port (e.g., COM3)
4. Set speed to 9600
5. Set Data bits: 8, Stop bits: 1, Parity: None
6. Click Open

### Mac/Linux (Using screen)

```bash
# Connect to device
screen /dev/tty.usbserial-12345 9600

# Disconnect
Ctrl+A, then Ctrl+D
```

### Mac/Linux (Using minicom)

```bash
# Install if needed
sudo apt install minicom  # Linux
brew install minicom      # Mac

# Connect
minicom -D /dev/ttyUSB0 -b 9600

# Disconnect
Ctrl+A, then Ctrl+X
```

### Windows (Using PuTTY SSH/Telnet Alternative)

Use WSL (Windows Subsystem for Linux):
```bash
# In WSL terminal
screen /dev/ttyUSB0 9600
```

## Packet Sniffing (Advanced)

### Capture RS232 Traffic

Option 1: Use logic analyzer
- Connect CH340 logic analyzer to RS232 lines
- Use Pulseview to capture and decode

Option 2: Software spy
```bash
# Linux - capture serial data
sudo timeout 30 strace -e trace=write,read -p $(pgrep esphome) > serial_traffic.txt
```

## Memory & Resource Monitoring

### Check Heap Usage

```bash
esphome logs denon-controller.yaml | grep -i heap
```

Look for heap becoming too small (<10KB free):
```
[denon232] Free heap: 152384 bytes
```

### If Running Out of Memory

1. Disable unnecessary components:
   ```yaml
   web_server:
     skip_web_server: true  # Disable if not needed
   ```

2. Reduce log buffer:
   ```yaml
   logger:
     buffer_size: 1024  # Default is 4096
   ```

3. Increase polling interval:
   ```yaml
   media_player:
     polling_interval: 30000  # Poll every 30 seconds
   ```

## Getting Help

When asking for help, include:

1. **Device Info:**
   - ESP32 model/board
   - ESPHome version
   - Component version

2. **Configuration:**
   - Your denon-controller.yaml (sanitized)
   - UART pin setup
   - Cable type

3. **Logs:**
   - Device logs (30 seconds of issue)
   - Web console debug log
   - Any error messages

4. **Steps to Reproduce:**
   - Exact steps that cause the issue
   - What you expected to happen
   - What actually happened

5. **Denon Receiver Info:**
   - Model number
   - Firmware version
   - RS232 settings (if accessible)

---

For more help, visit:
- GitHub Issues: https://github.com/Kousei-Uchu/ESPDenon232/issues
- Home Assistant Community: https://community.home-assistant.io/
- ESPHome Docs: https://esphome.io/
