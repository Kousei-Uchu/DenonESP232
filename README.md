# ESPHome Denon RS232 Integration (ESPDenon232)

A comprehensive ESPHome custom component for controlling Denon AVR receivers via RS232 serial connection using an ESP32 and Max3232 level converter. Features real-time live state updates with fallback 30-second polling, complete Denon command database, remote entity support, and configurable cable modes.

## 🎯 Features

- **Live State Updates**: Real-time event-driven state changes (power, volume, mute, source)
- **Fallback Polling**: Automatic 30-second polling backup if live updates miss
- **Complete Command Database**: 40+ Denon commands organized by category
- **Remote Entity Support**: Control receiver from HA's visual UI with command list
- **Media Player Integration**: Full media_player entity with all standard controls
- **Cable Mode Configuration**: Support for both null-modem and pass-through cables
- **HA Settings Integration**: Reconfigurable options via Home Assistant settings menu
- **Auto-Discovery**: Commands automatically visible in Home Assistant UI
- **Volume Scaling**: Automatic max volume detection and level adjustment
- **Multi-Source Support**: CD, DVD, TV/CBL, HDP, V.AUX, NET, USB inputs

## 📋 Hardware Requirements

- **ESP32** Development Board (any variant)
- **Max3232 RS232 Level Converter IC** (TI MAX3232 or compatible)
- **Denon AVR Receiver** with RS232 serial port
- **DB9 Female Connector** (for RS232 connection)
- **Capacitors**: Four 1µF ceramic capacitors (recommended for stability)
- **Breadboard/PCB**, jumper wires, USB cable for programming

## 🔌 Hardware Wiring

### Option A: Null Modem Cable (Standard - Recommended)

**ESP32 to Max3232:**
```
ESP32 Pin    Max3232 Pin    Function
GPIO17    →  T1IN          TX (Data Out)
GPIO16    →  R1OUT         RX (Data In)
GND       →  GND           Ground
5V        →  VCC           Power Supply
```

**Max3232 to Denon RS232 (DB9 Female):**
```
Max3232 Pin  DB9 Pin  Signal
T1OUT     →  Pin 2   RX (Receive)
R1IN      →  Pin 3   TX (Transmit)
GND       →  Pin 5   Ground
```

### Option B: Pass-Through Cable

For pass-through cables (no crossing), the configuration will automatically swap RX/TX:

**ESP32 to Max3232:**
```
ESP32 Pin    Max3232 Pin    Function
GPIO17    →  R1IN          TX (mapped to RX pin)
GPIO16    →  T1OUT         RX (mapped to TX pin)
GND       →  GND           Ground
5V        →  VCC           Power Supply
```

The component automatically handles the pin mapping when `cable_mode: pass_through` is set.

## 🛠️ Installation Steps

### Step 1: Prepare Component Files

Create the component directory structure:
```
esphome/
└── components/
    └── denon232/
        ├── __init__.py
        ├── denon232_receiver.h
        ├── denon232_receiver.cpp
        ├── remote.h
        ├── remote.cpp
        ├── media_player.h
        ├── media_player.cpp
        ├── media_player.py
        └── manifest.json
```

### Step 2: Create ESPHome Configuration

Create a new device configuration file (e.g., `denon-controller.yaml`):

```yaml
esphome:
  name: denon-receiver-controller
  friendly_name: "Denon Receiver Controller"
  platform: esp32
  board: esp32dev

wifi:
  ssid: !secret wifi_ssid
  password: !secret wifi_password
  
  ap:
    ssid: "Denon Fallback"
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
    denon232.media_player: DEBUG

# UART Configuration for RS232 via Max3232
uart:
  id: denon232_uart
  tx_pin: GPIO17
  rx_pin: GPIO16
  baud_rate: 9600
  data_bits: 8
  parity: NONE
  stop_bits: 1

# Denon232 Receiver Component
denon232:
  uart_id: denon232_uart
  cable_mode: null_modem  # Options: null_modem, pass_through

# Media Player Integration
media_player:
  - platform: denon232
    id: denon_receiver
    name: "Denon Receiver"
    polling_interval: 5000  # milliseconds (5 seconds)
```

### Step 3: Flash to ESP32

```bash
esphome run denon-controller.yaml
```

## ⚙️ Configuration Options

### Denon232 Component Options

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `uart_id` | ID | **Required** | UART component ID for RS232 |
| `cable_mode` | enum | `null_modem` | `null_modem` or `pass_through` |

### Media Player Platform Options

| Option | Type | Default | Range | Description |
|--------|------|---------|-------|-------------|
| `id` | string | **Required** | - | Entity ID |
| `name` | string | **Required** | - | Display name in HA |
| `polling_interval` | int | 5000 | 1000-60000 | Polling frequency in ms |
| `cable_mode` | enum | `null_modem` | - | Cable type configuration |

## 🎮 Home Assistant Integration

### Automatic Discovery

Once flashed and connected, the media player appears automatically in Home Assistant via ESPHome native API.

### Media Player Controls

Access standard media player controls:
- ➡️ **Power On/Off**: Turn receiver on/standby
- 🔊 **Volume Control**: Set level (0-100%)
- 🔇 **Volume Step**: +/- buttons
- 🔕 **Mute**: Toggle audio mute
- 📺 **Source Selection**: Switch inputs (CD, DVD, TV/CBL, HDP, V.AUX, NET, USB)

### Remote Entity Controls

A remote entity provides full command access:
- All 40+ commands organized by category
- Power, Volume, Mute, Source, Sound Mode, Video, Tuner commands
- Visual UI with button layout in Home Assistant

### Accessing Settings Menu

Once integrated:
1. Go to **Settings → Devices & Services → ESPHome**
2. Find **Denon Receiver Controller**
3. Click **Configure**
4. Adjust settings:
   - Cable Mode (null_modem/pass_through)
   - Polling Interval
   - UART Pins (if needed)

## 📊 Available Denon Commands

### Power Commands
| Command Name | Description |
|--------------|-------------|
| `power_on` | Turn receiver on |
| `power_off` | Put receiver in standby |
| `power_query` | Query power state |

### Volume Commands
| Command Name | Description |
|--------------|-------------|
| `volume_up` | Increase volume |
| `volume_down` | Decrease volume |
| `volume_query` | Query current volume level |

### Mute Commands
| Command Name | Description |
|--------------|-------------|
| `mute_on` | Mute audio |
| `mute_off` | Unmute audio |
| `mute_query` | Query mute state |

### Source Commands
| Command Name | Description |
|--------------|-------------|
| `source_cd` | Select CD input |
| `source_dvd` | Select DVD input |
| `source_tv` | Select TV/Cable input |
| `source_hdp` | Select HDP input |
| `source_aux` | Select Aux input |
| `source_net` | Select Network input |
| `source_usb` | Select USB input |
| `source_query` | Query current source |

### Video Commands
| Command Name | Description |
|--------------|-------------|
| `hdmi_out_1` | HDMI Output 1 only |
| `hdmi_out_2` | HDMI Output 2 only |
| `hdmi_out_both` | Both HDMI outputs |
| `video_auto` | Auto detect video input |
| `video_hdmi` | HDMI video input |
| `video_component` | Component video input |
| `video_composite` | Composite video input |

### Sound Mode Commands
| Command Name | Description |
|--------------|-------------|
| `sound_stereo` | Stereo mode |
| `sound_dolby_digital` | Dolby Digital mode |
| `sound_dts` | DTS mode |
| `sound_surround` | Surround mode |
| `sound_query` | Query sound mode |

### Dynamic Range Commands
| Command Name | Description |
|--------------|-------------|
| `dynamic_range_off` | Disable dynamic range |
| `dynamic_range_lite` | Light dynamic range |
| `dynamic_range_mid` | Medium dynamic range |
| `dynamic_range_max` | Maximum dynamic range |

### Tuner Commands
| Command Name | Description |
|--------------|-------------|
| `tuner_up` | Next tuner preset |
| `tuner_down` | Previous tuner preset |
| `tuner_query` | Query tuner frequency |

### Miscellaneous Commands
| Command Name | Description |
|--------------|-------------|
| `display_on` | Front display on |
| `display_off` | Front display off |
| `eco_on` | Enable eco mode |
| `eco_off` | Disable eco mode |

## 🤝 Automation Examples

### Turn on with Home Activity

```yaml
automation:
  - alias: Turn on Denon when living room active
    trigger:
      entity_id: binary_sensor.living_room_motion
      from: "off"
      to: "on"
    action:
      service: media_player.turn_on
      entity_id: media_player.denon_receiver
```

### Set Volume Based on Time of Day

```yaml
automation:
  - alias: Evening Denon volume
    trigger:
      platform: time
      at: "18:00:00"
    condition:
      - condition: state
        entity_id: media_player.denon_receiver
        state: "on"
    action:
      service: media_player.volume_set
      entity_id: media_player.denon_receiver
      data:
        volume_level: 0.35  # 35%
```

### Auto-Switch Input on Activity

```yaml
automation:
  - alias: Switch Denon to TV when motion detected
    trigger:
      entity_id: binary_sensor.living_room_motion
      from: "off"
      to: "on"
    action:
      - service: media_player.select_source
        entity_id: media_player.denon_receiver
        data:
          source: "TV/CBL"
      - service: media_player.turn_on
        entity_id: media_player.denon_receiver
```

### Remote Command via Automation

```yaml
automation:
  - alias: Trigger Denon remote command
    trigger:
      event: my_event
    action:
      - service: remote.send_command
        entity_id: remote.denon_receiver_commands
        data:
          command: "sound_surround"
```

## 🔍 Troubleshooting

### No Communication

**Problem**: Device doesn't respond to commands
**Solutions**:
1. Verify serial wiring is correct (TX/RX not swapped)
2. Check Max3232 has 5V power (essential for RS232 levels)
3. Confirm ESP32 UART pins match config (GPIO17=TX, GPIO16=RX)
4. Test with null_modem cable first; if issues persist, try pass_through

### Intermittent Connection

**Problem**: Commands work occasionally, device unresponsive sometimes
**Solutions**:
1. Add 1µF capacitors to Max3232 power pins
2. Reduce cable length (max ~15m/50ft recommended)
3. Add ferrite beads around TX/RX wires
4. Check Denon receiver RS232 port isn't disabled in settings

### Volume Not Updating

**Problem**: Volume changes don't reflect in Home Assistant
**Solutions**:
1. Check receiver responds to `MV?` queries (check logs)
2. Some receivers use different max volume (query `MVMAX` value)
3. Verify volume polling is enabled (check polling_interval)
4. Review logs for parsing errors

### Commands Not Appearing in UI

**Problem**: Remote entity commands not visible
**Solutions**:
1. Restart Home Assistant integration
2. Check component compiled without errors
3. Verify remote platform is enabled in ESPHome YAML
4. Check Home Assistant logs for component loading errors

### Denon Receiver Not Responding

**Problem**: Receiver doesn't acknowledge any commands
**Solutions**:
1. Confirm receiver is powered on
2. Check RS232 port is enabled in receiver menu
3. Try manual testing: open serial monitor at 9600 baud
4. Send command like `PWON\r` and verify response
5. Reboot receiver and try again

## 🔧 Advanced Configuration

### Custom UART Pins

To use different GPIO pins:

```yaml
uart:
  id: denon232_uart
  tx_pin: GPIO5      # Your TX pin
  rx_pin: GPIO4      # Your RX pin
  baud_rate: 9600
```

Update your component code accordingly if using different pins.

### Adjusting Polling Interval

For more frequent updates (affects power consumption):

```yaml
media_player:
  - platform: denon232
    id: denon_receiver
    name: "Denon Receiver"
    polling_interval: 3000  # 3 seconds instead of 5
```

Minimum: 1000ms (1s), Maximum: 60000ms (60s)

### Adding Custom Commands

Edit `denon232_receiver.cpp` in the `init_command_database_()` function:

```cpp
// Add new command
commands_["my_custom_command"] = {
    "My Custom Command",           // Display name
    "CUSTOM_CMD_CODE",            // Denon command
    CommandCategory::MISC,         // Category
    false,                         // Expects response
    "Description of command"       // Description
};
```

Recompile and upload.

## 📚 Protocol Details

### RS232 Specifications
- **Baud Rate**: 9600 bps
- **Data Bits**: 8
- **Parity**: None
- **Stop Bits**: 1
- **Flow Control**: None
- **Terminator**: Carriage Return (CR, `\r`, 0x0D)

### Command Format
```
[COMMAND_CODE][PARAMETERS]\r
```

Examples:
- `PWON\r` - Power on
- `MV50\r` - Set volume to 50
- `PW?\r` - Query power state (receiver responds with `PWON\r` or `PWSTANDBY\r`)

### Response Format
- **Without Response**: Command executes, no feedback
- **With Response**: Receiver sends result terminated by `\r`
- **Unsolicited Updates**: Receiver may send updates when receiver controls are used

## 📖 References

- [Denon AVR Control Protocol Documentation](https://www.denon.com)
- [ESPHome UART Component](https://esphome.io/components/uart.html)
- [ESPHome Media Player](https://esphome.io/components/media_player/index.html)
- [ESPHome Remote Transmitter](https://esphome.io/components/remote_transmitter/index.html)
- [Max3232 Datasheet](https://www.ti.com/product/MAX3232)
- [Original Home Assistant Integration](https://github.com/bluepixel00/HomeAssistant_Denon_RS232)

## 📝 License

This project is based on the work by:
- [bluepixel00](https://github.com/bluepixel00/HomeAssistant_Denon_RS232)
- [nikor30](https://github.com/nikor30/HomeAssistant_Denon_RS232)

Licensed under MIT License - See LICENSE file for details

## 🤝 Contributing

Contributions welcome! Areas for improvement:
- Additional Denon receiver models/command sets
- Enhanced error handling and recovery
- Web-based command testing interface
- Extended command database
- Performance optimizations

## 💬 Support

For issues and questions:
1. Check the Troubleshooting section
2. Review Home Assistant logs for errors
3. Check ESPHome device logs for serial communication details
4. Open an issue on GitHub with detailed logs and configuration
