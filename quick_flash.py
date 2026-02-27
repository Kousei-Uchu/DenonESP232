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
    
    while True:
        try:
            choice = int(input(f"\n{Colors.YELLOW}Select port (1-{len(ports)}): {Colors.END}"))
            if 1 <= choice <= len(ports):
                return ports[choice - 1][0]
        except ValueError:
            pass
        print(f"{Colors.RED}Invalid selection{Colors.END}")

def create_config(device_name: str, board: str = "esp32dev") -> str:
    """Create ESPHome configuration"""
    config_file = f"denon_config_{device_name}.yaml"
    
    config_content = f"""esphome:
  name: {device_name}
  friendly_name: "Denon Receiver Controller"
  platform: esp32
  board: {board}

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

web_server:
  port: 80
  version: 3

uart:
  id: denon232_uart
  tx_pin: GPIO17
  rx_pin: GPIO16
  baud_rate: 9600
  data_bits: 8
  parity:
