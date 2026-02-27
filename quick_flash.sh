#!/bin/bash

# ESPDenon232 Quick Flash Script
# This script automates the flashing process for ESP32

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
DEVICE_NAME="${1:-denon-controller}"
SERIAL_PORT="${2:-}"
BOARD="${3:-esp32dev}"

echo -e "${BLUE}╔════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║  ESPDenon232 Quick Flash Utility      ║${NC}"
echo -e "${BLUE}╚════════════════════════════════════════╝${NC}"
echo ""

# Check if esphome is installed
if ! command -v esphome &> /dev/null; then
    echo -e "${RED}✗ ESPHome is not installed${NC}"
    echo -e "${YELLOW}Install it with: pip install esphome${NC}"
    exit 1
fi

echo -e "${GREEN}✓ ESPHome found: $(esphome version)${NC}"
echo ""

# Auto-detect serial port if not provided
if [ -z "$SERIAL_PORT" ]; then
    echo -e "${YELLOW}Detecting ESP32 serial port...${NC}"
    
    # Try common ports
    for port in /dev/ttyUSB0 /dev/ttyUSB1 /dev/ttyACM0 /dev/ttyACM1 /dev/cu.usbserial-* /dev/cu.SLAB_USBtoUART COM3 COM4 COM5; do
        if [ -e "$port" ] 2>/dev/null; then
            SERIAL_PORT="$port"
            echo -e "${GREEN}✓ Found serial port: $SERIAL_PORT${NC}"
            break
        fi
    done
    
    if [ -z "$SERIAL_PORT" ]; then
        echo -e "${RED}✗ Could not detect serial port${NC}"
        echo -e "${YELLOW}Please connect your ESP32 and try again, or specify port manually:${NC}"
        echo -e "${YELLOW}  ./quick_flash.sh $DEVICE_NAME /dev/ttyUSB0${NC}"
        exit 1
    fi
fi

echo ""

# Create ESPHome configuration
CONFIG_FILE="denon_config_${DEVICE_NAME}.yaml"

echo -e "${BLUE}Creating ESPHome configuration...${NC}"

cat > "$CONFIG_FILE" << 'EOF'
esphome:
  name: $DEVICE_NAME
  friendly_name: "Denon Receiver Controller"
  platform: esp32
  board: $BOARD
  includes:
    - components/denon232/
  libraries:
    - "https://github.com/espressif/arduino-esp32.git"

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
  reboot_timeout: 15min

ota:
  password: !secret ota_password

logger:
  level: DEBUG
  logs:
    denon232: DEBUG
    denon232.media_player: DEBUG
    denon232.debug: DEBUG

web_server:
  port: 80
  version: 3
  auth:
    username: admin
    password: !secret web_password

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
EOF

# Replace placeholders
sed -i "s/\$DEVICE_NAME/$DEVICE_NAME/g" "$CONFIG_FILE"
sed -i "s/\$BOARD/$BOARD/g" "$CONFIG_FILE"

echo -e "${GREEN}✓ Configuration created: $CONFIG_FILE${NC}"
echo ""

# Create secrets file if it doesn't exist
SECRETS_FILE="secrets.yaml"
if [ ! -f "$SECRETS_FILE" ]; then
    echo -e "${BLUE}Creating secrets file...${NC}"
    cat > "$SECRETS_FILE" << 'EOF'
# WiFi credentials
wifi_ssid: "YOUR_SSID"
wifi_password: "YOUR_PASSWORD"

# API encryption key (generate with: esphome new-secret)
api_encryption_key: "YOUR_API_KEY"

# OTA password
ota_password: "YOUR_OTA_PASSWORD"

# Web interface password
web_password: "admin"
EOF
    echo -e "${YELLOW}⚠ Secrets file created: $SECRETS_FILE${NC}"
    echo -e "${YELLOW}  Please edit it with your WiFi credentials${NC}"
    echo ""
fi

# Validate configuration
echo -e "${BLUE}Validating configuration...${NC}"
esphome config "$CONFIG_FILE" > /dev/null 2>&1
if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ Configuration is valid${NC}"
else
    echo -e "${RED}✗ Configuration validation failed${NC}"
    esphome config "$CONFIG_FILE"
    exit 1
fi

echo ""
echo -e "${BLUE}╔════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║  Ready to Flash                       ║${NC}"
echo -e "${BLUE}╚════════════════════════════════════════╝${NC}"
echo ""
echo -e "${YELLOW}Device: ${GREEN}$DEVICE_NAME${NC}"
echo -e "${YELLOW}Serial Port: ${GREEN}$SERIAL_PORT${NC}"
echo -e "${YELLOW}Board: ${GREEN}$BOARD${NC}"
echo ""

read -p "Press Enter to start flashing or Ctrl+C to cancel..."

echo -e "${BLUE}Compiling and flashing firmware...${NC}"
esphome run "$CONFIG_FILE" --device "$SERIAL_PORT" --no-logs

if [ $? -eq 0 ]; then
    echo ""
    echo -e "${GREEN}✓ Flash successful!${NC}"
    echo ""
    echo -e "${BLUE}Next steps:${NC}"
    echo -e "  1. Wait for device to reboot (30 seconds)"
    echo -e "  2. Connect to WiFi network from your phone/computer"
    echo -e "  3. Follow the captive portal to enter credentials"
    echo -e "  4. Device will connect and appear in Home Assistant"
    echo -e "  5. Access web interface at: http://$(hostname -I | awk '{print $1}')/denon"
    echo ""
else
    echo -e "${RED}✗ Flash failed${NC}"
    exit 1
fi
