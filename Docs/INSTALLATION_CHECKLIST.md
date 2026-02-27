# ESPDenon232 Installation Checklist

Use this checklist to verify your installation is complete and working.

## Pre-Installation

- [ ] Python 3.7+ installed and working
- [ ] ESPHome installed (`pip install esphome`)
- [ ] ESP32 board available
- [ ] Max3232 IC available
- [ ] DB9 Female connector available
- [ ] 4× 1µF capacitors available
- [ ] USB cable for ESP32 programming
- [ ] Denon receiver with RS232 port
- [ ] Computer and receiver on same network

## Hardware Assembly

- [ ] Max3232 capacitors installed
  - [ ] Between VCC and GND
  - [ ] Between charge pump pins 2-3
  - [ ] Between charge pump pins 4-5
  - [ ] Fourth capacitor on any unused pair

- [ ] ESP32 to Max3232 connections
  - [ ] GPIO17 → T1IN (Pin 11)
  - [ ] GPIO16 → R1OUT (Pin 12)
  - [ ] GND → GND (Pin 15)
  - [ ] 5V → VCC (Pin 16)

- [ ] Max3232 to Denon RS232 DB9
  - [ ] T1OUT (Pin 10) → DB9 Pin 2 (RX)
  - [ ] R1IN (Pin 13) → DB9 Pin 3 (TX)
  - [ ] GND → DB9 Pin 5 (GND)

- [ ] USB cable connected to ESP32
- [ ] Denon receiver RS232 port cable connected

## Software Setup

- [ ] Component files copied to correct directory structure
- [ ] denon-controller.yaml created and in project folder
- [ ] secrets.yaml created with WiFi credentials
- [ ] API encryption key generated and added to secrets.yaml
- [ ] Configuration validated: `esphome config denon-controller.yaml`

## Flash & Boot

- [ ] Device successfully flashed: `esphome run denon-controller.yaml`
- [ ] Device boots without errors (check serial monitor if needed)
- [ ] Device appears in ESPHome dashboard
- [ ] Device connects to WiFi (check logs)
- [ ] Device has assigned IP address

## Network & Home Assistant

- [ ] Device responds to ping: `ping denon-controller.local`
- [ ] Web interface accessible: `http://device_ip`
- [ ] Home Assistant can discover device (if running)
- [ ] Device added to Home Assistant
- [ ] Media player entity appears in HA

## Denon Communication

- [ ] Debug logs show UART activity
- [ ] Commands appear in logs: `Command: PWON`
- [ ] Responses appear in logs: `Response: PWON`
- [ ] Denon receiver responds to power on command
- [ ] Denon receiver responds to power query
- [ ] Volume control works

## Media Player Testing

- [ ] Power On button works in HA
- [ ] Power Off button works in HA
- [ ] Volume slider responds
- [ ] Mute button works
- [ ] Source selection works
- [ ] All sources available in dropdown
- [ ] Device status updates in real-time

## Web Interface Testing

- [ ] Web interface loads at `http://device_ip`
- [ ] Power controls work
- [ ] Volume controls work
- [ ] Source selection dropdown works
- [ ] All command buttons respond
- [ ] Debug console shows activity
- [ ] Status bar shows correct values

## Advanced Features

- [ ] Live updates working (vs polling backup)
- [ ] Polling interval configured (if customized)
- [ ] Debug monitor enabled (optional)
- [ ] Web server accessible (if enabled)
- [ ] API endpoints responding

## Optional Integrations

- [ ] Home Assistant automation created (if desired)
- [ ] Custom remote commands configured (if desired)
- [ ] Lovelace custom card installed (if using HA)

## Troubleshooting Checklist

If any of the above fails, check:

### Device Won't Flash
- [ ] USB drivers installed (CH340 for ESP32)
- [ ] Serial port detected by OS
- [ ] ESPHome can access serial port
- [ ] Try different USB cable
- [ ] Try different USB port on computer

### No WiFi Connection
- [ ] WiFi SSID and password correct in secrets.yaml
- [ ] WiFi is 2.4GHz (not 5GHz)
- [ ] No special characters in password (or properly quoted)
- [ ] Try captive portal: Connect to "Denon-Fallback" network

### No Denon Communication
- [ ] UART TX/RX pins not swapped (GPIO17=TX, GPIO16=RX)
- [ ] GND connection secure
- [ ] Max3232 powered with 5V (not 3.3V)
- [ ] All four capacitors installed on Max3232
- [ ] RS232 cable properly seated on Denon
- [ ] Denon RS232 port enabled in receiver menu

### Slow or Intermittent
- [ ] RS232 cable not damaged
- [ ] Cable length under 5m (preferably under 3m)
- [ ] No power cables running parallel to RS232 cable
- [ ] Ferrite beads added to RS232 wires (recommended)
- [ ] WiFi signal strength adequate

### Web Interface Not Working
- [ ] web_server component enabled in yaml
- [ ] Device and computer on same network
- [ ] Firewall not blocking port 80
- [ ] Correct IP address used
- [ ] Device fully booted (wait 30 seconds after flash)

## Final Verification

- [ ] Device shown in ESPHome dashboard as "Connected"
- [ ] Device shown in Home Assistant with all entities
- [ ] At least one command executes successfully
- [ ] Status updates are reflected in UI
- [ ] Debug logs show no errors (warnings are OK)
- [ ] Web interface responsive (< 1 second load time)

## Next Steps

Once everything is working:

1. [ ] Configure Home Assistant automations
2. [ ] Set up custom Lovelace UI (optional)
3. [ ] Create automation for common tasks
4. [ ] Test all remote commands
5. [ ] Fine-tune polling interval for performance
6. [ ] Document your setup for future reference

## Documentation

- [ ] Saved copy of configuration files
- [ ] Noted device IP address
- [ ] Documented API key (if needed for manual API calls)
- [ ] Recorded any custom modifications made
- [ ] Saved wiring diagram photo for reference

## Support Contact

If you get stuck:
1. Check [DEBUG_GUIDE.md](DEBUG_GUIDE.md)
2. Review logs with `esphome logs denon-controller.yaml`
3. Check GitHub issues for similar problems
4. Post on Home Assistant Community forum
5. Open GitHub issue with full debug logs

---

**Installation Date:** _______________

**Device Name:** _______________

**Denon Model:** _______________

**ESP32 Board Type:** _______________

**Notes:**

_______________________________________________

_______________________________________________

_______________________________________________
