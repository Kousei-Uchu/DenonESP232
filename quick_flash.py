#!/usr/bin/env python3
"""
ESPDenon232 Quick Flash Utility
Streamlined flashing for ESP32 with ESPHome
"""

import os
import sys
import subprocess
import serial.tools.list_ports
import argparse
from pathlib import Path
from typing import Optional, Tuple
import json
from datetime import datetime

class Colors:
    HEADER = '\033[95m'
    BLUE = '\033[94m'
    CYAN = '\033[96m'
    GREEN = '\033[92m'
    YELLOW = '\033[93m'
    RED = '\033[91m'
    END = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

def print_header():
    """Print welcome header"""
    print(f"\n{Colors.BLUE}{Colors.BOLD}")
    print("╔═══════════════════════════════════════════╗")
    print("║  ESPDenon232 Quick Flash Utility        ║")
    print("║  Version 1.0                             ║")
    print("╚═══════════════════════════════════════════╝")
    print(f"{Colors.END}\n")

def check_esphome() -> bool:
    """Check if ESPHome is installed"""
    try:
        result = subprocess.run(['esphome', 'version'], capture_output=True, text=True)
        version = result.stdout.strip()
        print(f"{Colors.GREEN}✓{Colors.END} ESPHome found: {version}")
        return True
    except FileNotFoundError:
        print(f"{Colors.RED}✗{Colors.END} ESPHome is not installed")
        print(f"{Colors.YELLOW}Install with: pip install esphome{Colors.END}")
        return False

def detect_serial_ports() -> list:
    """Detect available serial ports"""
    ports = []
    for port_info in serial.tools.list_ports.comports():
        ports.append((port_info.device, port_info.description))
    return ports

def select_serial_port() -> Optional[str]:
    """Interactive serial port selection"""
    ports = detect_serial_ports()
    
    if not ports:
        print(f"{Colors.RED}✗{Colors.END} No serial ports detected")
        print(f"{Colors.YELLOW}Please connect your ESP32 and try again{Colors.END}")
        return None
    
    if len(ports) == 1:
        port, desc = ports[0]
        print(f"{Colors.GREEN}✓{Colors.END} Found serial port: {Colors.CYAN}{port}{Colors.END} ({desc})")
        return port
    
    print(f"{Colors.BLUE}Multiple serial ports detected:{Colors.END}")
    for i, (port, desc) in enumerate(ports, 1):
        print(f"  {i}. {Colors.CYAN}{port}{Colors.END} - {desc}")
    
    max_attempts = 3
    attempts = 0
    while attempts < max_attempts:
        try:
            choice = int(input(f"\n{Colors.YELLOW}Select port (1-{len(ports)}): {Colors.END}"))
            if 1 <= choice <= len(ports):
                return ports[choice - 1][0]
            else:
                print(f"{Colors.RED}Please select a number between 1 and {len(ports)}{Colors.END}")
        except ValueError:
            print(f"{Colors.RED}Invalid input. Please enter a number.{Colors.END}")
        attempts += 1
    
    print(f"{Colors.RED}Too many invalid attempts. Exiting.{Colors.END}")
    return None

def create_config(device_name: str, board: str = "esp32dev") -> str:
    """Create ESPHome configuration"""
    config_file = f"denon_config_{device_name}.yaml"
    
    config_content = f"""esphome:
  name: {device_name}
  friendly_name: "Denon Receiver Controller"

esp32:
  board: {board}
  framework:
    type: esp-idf

external_components:
  - source:
      type: local
      path: components

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

logger:
  level: DEBUG
  logs:
    denon232: DEBUG
    denon232.media_player: DEBUG

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
  id: my_denon
  uart_id: denon232_uart
  cable_mode: pass_through

media_player:
  - platform: denon232
    id: denon_receiver
    name: "Denon Receiver"
    denon232_id: my_denon
    polling_interval: 5000
"""
    
    with open(config_file, 'w') as f:
        f.write(config_content)
    
    print(f"{Colors.GREEN}✓{Colors.END} Configuration created: {Colors.CYAN}{config_file}{Colors.END}")
    return config_file

def create_secrets(force: bool = False) -> str:
    """Create secrets.yaml file"""
    secrets_file = "secrets.yaml"
    
    if os.path.exists(secrets_file) and not force:
        print(f"{Colors.YELLOW}⚠{Colors.END} Secrets file already exists")
        return secrets_file
    
    secrets_content = """# WiFi Credentials
wifi_ssid: "YOUR_SSID"
wifi_password: "YOUR_PASSWORD"

# API Encryption Key (generate with: esphome new-secret)
api_encryption_key: "YOUR_API_KEY"

# OTA Update Password
ota_password: "esphome"

# Web Interface Credentials
web_username: "admin"
web_password: "admin"
"""
    
    with open(secrets_file, 'w') as f:
        f.write(secrets_content)
    
    print(f"{Colors.YELLOW}⚠{Colors.END} Secrets file created: {Colors.CYAN}{secrets_file}{Colors.END}")
    print(f"{Colors.YELLOW}   Please edit it with your WiFi credentials before flashing{Colors.END}")
    return secrets_file

def validate_config(config_file: str) -> bool:
    """Validate ESPHome configuration"""
    try:
        result = subprocess.run(
            ['esphome', 'config', config_file],
            capture_output=True,
            text=True,
            timeout=30
        )
        if result.returncode == 0:
            print(f"{Colors.GREEN}✓{Colors.END} Configuration is valid")
            return True
        else:
            print(f"{Colors.RED}✗{Colors.END} Configuration validation failed:")
            print(result.stderr)
            return False
    except subprocess.TimeoutExpired:
        print(f"{Colors.RED}✗{Colors.END} Configuration validation timed out")
        return False
    except Exception as e:
        print(f"{Colors.RED}✗{Colors.END} Error validating configuration: {e}")
        return False

def flash_device(config_file: str, serial_port: str) -> bool:
    """Flash the device"""
    print(f"\n{Colors.BLUE}Compiling and flashing firmware...{Colors.END}")
    print(f"{Colors.YELLOW}This may take 2-5 minutes on first build{Colors.END}\n")
    
    try:
        result = subprocess.run(
            ['esphome', 'run', config_file, '--device', serial_port, '--no-logs'],
            timeout=600  # 10 minutes timeout
        )
        
        if result.returncode == 0:
            print(f"\n{Colors.GREEN}✓{Colors.END} Flash successful!")
            return True
        else:
            print(f"\n{Colors.RED}✗{Colors.END} Flash failed")
            return False
    except subprocess.TimeoutExpired:
        print(f"\n{Colors.RED}✗{Colors.END} Flash operation timed out")
        return False
    except Exception as e:
        print(f"\n{Colors.RED}✗{Colors.END} Error during flash: {e}")
        return False

def print_next_steps(device_name: str):
    """Print post-flash instructions"""
    print(f"\n{Colors.BLUE}{Colors.BOLD}")
    print("╔═══════════════════════════════════════════╗")
    print("║  Next Steps                             ║")
    print("╚═══════════════════════════════════════════╝")
    print(f"{Colors.END}\n")
    
    print(f"{Colors.CYAN}1. Device Setup (30 seconds){Colors.END}")
    print(f"   - Device is booting and connecting to WiFi")
    print(f"   - Wait for LED to stabilize\n")
    
    print(f"{Colors.CYAN}2. WiFi Connection{Colors.END}")
    print(f"   - Look for WiFi network: {Colors.YELLOW}Denon-Fallback{Colors.END}")
    print(f"   - Connect from your phone/computer")
    print(f"   - Captive portal should open automatically")
    print(f"   - If not, visit: {Colors.YELLOW}http://192.168.4.1{Colors.END}\n")
    
    print(f"{Colors.CYAN}3. Enter WiFi Credentials{Colors.END}")
    print(f"   - Select your WiFi network (SSID)")
    print(f"   - Enter password")
    print(f"   - Device will save and connect\n")
    
    print(f"{Colors.CYAN}4. Find Device IP{Colors.END}")
    print(f"   - Check your router's connected devices")
    print(f"   - Device name: {Colors.YELLOW}{device_name}{Colors.END}")
    print(f"   - Or use: {Colors.YELLOW}http://{device_name}.local{Colors.END}\n")
    
    print(f"{Colors.CYAN}5. Access Web Interface{Colors.END}")
    print(f"   - Open: {Colors.YELLOW}http://<device_ip>/denon{Colors.END}")
    print(f"   - Control your Denon receiver from any device\n")
    
    print(f"{Colors.CYAN}6. Home Assistant Integration{Colors.END}")
    print(f"   - Go to Settings → Devices & Services → ESPHome")
    print(f"   - Device should appear automatically (if on same network)")
    print(f"   - Or add manually via IP address\n")
    
    print(f"{Colors.GREEN}All done! Enjoy your Denon controller!{Colors.END}\n")

def main():
    """Main function"""
    parser = argparse.ArgumentParser(
        description='ESPDenon232 Quick Flash Utility',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python quick_flash.py                           # Interactive mode
  python quick_flash.py my-denon /dev/ttyUSB0    # Specify device and port
  python quick_flash.py --board esp32-poe         # Custom board
  python quick_flash.py --setup-only              # Just create config
        """
    )
    
    parser.add_argument('device', nargs='?', default='denon-controller',
                       help='Device name (default: denon-controller)')
    parser.add_argument('port', nargs='?', default=None,
                       help='Serial port (auto-detect if not specified)')
    parser.add_argument('--board', default='esp32dev',
                       help='ESP32 board type (default: esp32dev)')
    parser.add_argument('--setup-only', action='store_true',
                       help='Only create config files, do not flash')
    parser.add_argument('--no-secrets', action='store_true',
                       help='Skip creating secrets.yaml')
    parser.add_argument('--force', action='store_true',
                       help='Overwrite existing configuration')
    
    args = parser.parse_args()
    
    print_header()
    
    # Check ESPHome installation
    if not check_esphome():
        sys.exit(1)
    
    print()
    
    # Create configuration
    try:
        config_file = create_config(args.device, args.board)
        print()
        
        # Create secrets if needed
        if not args.no_secrets and not os.path.exists('secrets.yaml'):
            create_secrets()
            print()
            
            # Ask user to edit secrets
            input(f"{Colors.YELLOW}Please edit secrets.yaml with your WiFi credentials, then press Enter...{Colors.END}")
            print()
        
        # Validate configuration
        if not validate_config(config_file):
            print(f"{Colors.YELLOW}Please fix configuration errors before flashing{Colors.END}")
            sys.exit(1)
        
        print()
        
        # Setup only mode
        if args.setup_only:
            print(f"{Colors.GREEN}Configuration setup complete!{Colors.END}")
            print(f"{Colors.YELLOW}To flash, run:{Colors.END}")
            print(f"  esphome run {config_file}")
            sys.exit(0)
        
        # Select serial port
        if args.port:
            serial_port = args.port
        else:
            serial_port = select_serial_port()
            if not serial_port:
                sys.exit(1)
        
        print()
        
        # Summary
        print(f"{Colors.BLUE}{Colors.BOLD}")
        print("╔═══════════════════════════════════════════╗")
        print("║  Ready to Flash                         ║")
        print("╚═══════════════════════════════════════════╝")
        print(f"{Colors.END}\n")
        
        print(f"Device Name:   {Colors.CYAN}{args.device}{Colors.END}")
        print(f"Serial Port:   {Colors.CYAN}{serial_port}{Colors.END}")
        print(f"Board Type:    {Colors.CYAN}{args.board}{Colors.END}")
        print(f"Config File:   {Colors.CYAN}{config_file}{Colors.END}")
        print()
        
        response = input(f"{Colors.YELLOW}Start flashing? (y/N): {Colors.END}").strip().lower()
        
        if response != 'y':
            print(f"{Colors.YELLOW}Cancelled{Colors.END}")
            sys.exit(0)
        
        # Flash device
        if flash_device(config_file, serial_port):
            print_next_steps(args.device)
        else:
            print(f"{Colors.RED}Please check the error messages above and try again{Colors.END}")
            sys.exit(1)
    
    except KeyboardInterrupt:
        print(f"\n{Colors.YELLOW}Cancelled by user{Colors.END}")
        sys.exit(0)
    except Exception as e:
        print(f"{Colors.RED}Unexpected error: {e}{Colors.END}")
        sys.exit(1)

if __name__ == '__main__':
    main()
