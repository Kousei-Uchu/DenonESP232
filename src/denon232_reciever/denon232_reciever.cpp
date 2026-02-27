#include "denon232_receiver.h"
#include "esphome/core/log.h"

namespace esphome {
namespace denon232 {

static const char *const TAG = "denon232";

void Denon232Receiver::init_command_database_() {
  // Power Commands
  commands_["power_on"] = {"Power On", "PWON", CommandCategory::POWER, false, "Turn receiver on"};
  commands_["power_off"] = {"Power Off", "PWSTANDBY", CommandCategory::POWER, false, "Put receiver in standby"};
  commands_["power_query"] = {"Power Status", "PW?", CommandCategory::POWER, true, "Query power state"};

  // Volume Commands
  commands_["volume_up"] = {"Volume Up", "MVUP", CommandCategory::VOLUME, false, "Increase volume"};
  commands_["volume_down"] = {"Volume Down", "MVDOWN", CommandCategory::VOLUME, false, "Decrease volume"};
  commands_["volume_query"] = {"Volume Status", "MV?", CommandCategory::VOLUME, true, "Query volume level"};

  // Mute Commands
  commands_["mute_on"] = {"Mute On", "MUON", CommandCategory::MUTE, false, "Mute audio"};
  commands_["mute_off"] = {"Mute Off", "MUOFF", CommandCategory::MUTE, false, "Unmute audio"};
  commands_["mute_query"] = {"Mute Status", "MU?", CommandCategory::MUTE, true, "Query mute state"};

  // Source/Input Commands
  commands_["source_cd"] = {"Source: CD", "SICD", CommandCategory::SOURCE, false, "Select CD input"};
  commands_["source_dvd"] = {"Source: DVD", "SIDVD", CommandCategory::SOURCE, false, "Select DVD input"};
  commands_["source_tv"] = {"Source: TV/CBL", "SITV", CommandCategory::SOURCE, false, "Select TV/Cable input"};
  commands_["source_hdp"] = {"Source: HDP", "SIHDP", CommandCategory::SOURCE, false, "Select HDP input"};
  commands_["source_aux"] = {"Source: V.AUX", "SIAUX", CommandCategory::SOURCE, false, "Select Aux input"};
  commands_["source_net"] = {"Source: NET", "SINET", CommandCategory::SOURCE, false, "Select Network input"};
  commands_["source_usb"] = {"Source: USB", "SIUSB", CommandCategory::SOURCE, false, "Select USB input"};
  commands_["source_query"] = {"Source Status", "SI?", CommandCategory::SOURCE, true, "Query current source"};

  // HDMI Commands
  commands_["hdmi_out_1"] = {"HDMI Out 1", "DOHDMI", CommandCategory::VIDEO, false, "Select HDMI output 1"};
  commands_["hdmi_out_2"] = {"HDMI Out 2", "DOHDMI/2", CommandCategory::VIDEO, false, "Select HDMI output 2"};
  commands_["hdmi_out_both"] = {"HDMI Out 1+2", "DOHDMI/1+2", CommandCategory::VIDEO, false, "Both HDMI outputs"};

  // Video Input Commands
  commands_["video_auto"] = {"Video Auto", "DVDVI/AUTO", CommandCategory::VIDEO, false, "Video auto detect"};
  commands_["video_hdmi"] = {"Video HDMI", "DVDVI/HDMI", CommandCategory::VIDEO, false, "HDMI video input"};
  commands_["video_component"] = {"Video Component", "DVDVI/COMPONENT", CommandCategory::VIDEO, false, "Component video"};
  commands_["video_composite"] = {"Video Composite", "DVDVI/COMPOSITE", CommandCategory::VIDEO, false, "Composite video"};

  // Sound Mode Commands
  commands_["sound_stereo"] = {"Stereo", "MSSTD", CommandCategory::SOUND, false, "Stereo sound mode"};
  commands_["sound_dolby_digital"] = {"Dolby Digital", "MSDD", CommandCategory::SOUND, false, "Dolby Digital mode"};
  commands_["sound_dts"] = {"DTS", "MSDTS", CommandCategory::SOUND, false, "DTS sound mode"};
  commands_["sound_surround"] = {"Surround", "MSSUR", CommandCategory::SOUND, false, "Surround mode"};
  commands_["sound_query"] = {"Sound Mode Status", "MS?", CommandCategory::SOUND, true, "Query sound mode"};

  // Dynamic Range Commands
  commands_["dynamic_range_off"] = {"Dynamic Range Off", "MDOFF", CommandCategory::SOUND, false, "Disable dynamic range"};
  commands_["dynamic_range_lite"] = {"Dynamic Range Lite", "MDLITE", CommandCategory::SOUND, false, "Light dynamic range"};
  commands_["dynamic_range_mid"] = {"Dynamic Range Mid", "MDMID", CommandCategory::SOUND, false, "Medium dynamic range"};
  commands_["dynamic_range_max"] = {"Dynamic Range Max", "MDMAX", CommandCategory::SOUND, false, "Maximum dynamic range"};

  // Tuner Commands
  commands_["tuner_up"] = {"Tuner Up", "TPUP", CommandCategory::TUNER, false, "Next tuner preset"};
  commands_["tuner_down"] = {"Tuner Down", "TPDN", CommandCategory::TUNER, false, "Previous tuner preset"};
  commands_["tuner_query"] = {"Tuner Status", "TP?", CommandCategory::TUNER, true, "Query tuner frequency"};

  // Miscellaneous Commands
  commands_["display_on"] = {"Display On", "DISON", CommandCategory::MISC, false, "Front display on"};
  commands_["display_off"] = {"Display Off", "DISOFF", CommandCategory::MISC, false, "Front display off"};
  commands_["eco_on"] = {"Eco Mode On", "ECON", CommandCategory::MISC, false, "Enable eco mode"};
  commands_["eco_off"] = {"Eco Mode Off", "ECOFF", CommandCategory::MISC, false, "Disable eco mode"};
}

void Denon232Receiver::setup() {
  ESP_LOGI(TAG, "Setting up Denon232 Receiver (Cable Mode: %s)", 
           cable_mode_ == CableMode::NULL_MODEM ? "Null Modem" : "Pass-through");
  
  init_command_database_();
  
  // Request initial state
  get_power_state();
  get_volume();
  get_mute_state();
  get_source();
}

void Denon232Receiver::loop() {
  // Check for unsolicited responses from receiver
  while (uart_->available()) {
    std::string response = read_response_line_();
    if (!response.empty()) {
      process_unsolicited_response_(response);
    }
  }
}

void Denon232Receiver::update() {
  uint32_t now = millis();
  if (now - last_poll_ > poll_interval_) {
    last_poll_ = now;
    
    // Poll all states as backup
    ESP_LOGD(TAG, "Polling device state");
    get_power_state();
    get_volume();
    get_mute_state();
    get_source();
  }
}

float Denon232Receiver::get_setup_priority() const {
  return esphome::setup_priority::AFTER_WIFI;
}

void Denon232Receiver::write_command_(const std::string &cmd) {
  if (!uart_) {
    ESP_LOGE(TAG, "UART not initialized");
    return;
  }
  
  std::string final_cmd = cmd + "\r";
  uart_->write_array((const uint8_t *)final_cmd.c_str(), final_cmd.length());
  delayMicroseconds(100);
}

std::string Denon232Receiver::read_response_line_() {
  std::string line;
  uint32_t start_time = millis();
  const uint32_t timeout_ms = 500;
  
  if (!uart_) {
    return line;
  }
  
  while (millis() - start_time < timeout_ms) {
    if (uart_->available()) {
      uint8_t byte = uart_->read();
      
      if (byte == '\r' || byte == '\n') {
        if (!line.empty()) {
          return line;
        }
      } else if (byte >= 32 && byte < 127) {
        line += (char)byte;
      }
    }
  }
  
  return line;
}

std::string Denon232Receiver::serial_command(const std::string &cmd, bool response) {
  ESP_LOGD(TAG, "Command: %s", cmd.c_str());
  
  write_command_(cmd);
  
  if (!response) {
    return "";
  }
  
  std::string result = read_response_line_();
  ESP_LOGD(TAG, "Response: %s", result.c_str());
  
  return result;
}

void Denon232Receiver::process_unsolicited_response_(const std::string &response) {
  ESP_LOGD(TAG, "Unsolicited response: %s", response.c_str());
  
  // Parse response and trigger callback
  if (response_callback_) {
    std::string key = response.substr(0, 2);  // First 2 chars usually identify response type
    response_callback_(key, response);
  }
}

void Denon232Receiver::execute_command(const std::string &command_name) {
  auto it = commands_.find(command_name);
  if (it != commands_.end()) {
    serial_command(it->second.command);
  } else {
    ESP_LOGW(TAG, "Unknown command: %s", command_name.c_str());
  }
}

std::string Denon232Receiver::query_state(const std::string &query_cmd) {
  auto it = commands_.find(query_cmd);
  if (it != commands_.end()) {
    return serial_command(it->second.command, true);
  }
  return "";
}

std::vector<DenonCommand> Denon232Receiver::get_available_commands() const {
  std::vector<DenonCommand> cmds;
  for (const auto &pair : commands_) {
    cmds.push_back(pair.second);
  }
  return cmds;
}

DenonCommand Denon232Receiver::get_command_info(const std::string &name) const {
  auto it = commands_.find(name);
  if (it != commands_.end()) {
    return it->second;
  }
  return {"Unknown", "", CommandCategory::MISC, false, ""};
}

// Power commands
void Denon232Receiver::power_on() {
  serial_command("PWON");
}

void Denon232Receiver::power_off() {
  serial_command("PWSTANDBY");
}

std::string Denon232Receiver::get_power_state() {
  return serial_command("PW?", true);
}

// Volume commands
void Denon232Receiver::volume_up() {
  serial_command("MVUP");
}

void Denon232Receiver::volume_down() {
  serial_command("MVDOWN");
}

void Denon232Receiver::set_volume(uint8_t volume) {
  char cmd[16];
  snprintf(cmd, sizeof(cmd), "MV%02d", volume);
  serial_command(cmd);
}

uint8_t Denon232Receiver::get_volume() {
  std::string response = serial_command("MV?", true);
  
  if (response.length() >= 2 && response.substr(0, 2) == "MV") {
    try {
      return std::stoi(response.substr(2));
    } catch (...) {
      return 0;
    }
  }
  return 0;
}

uint8_t Denon232Receiver::get_volume_max() {
  std::string response = serial_command("MV?", true);
  
  if (response.find("MVMAX") != std::string::npos) {
    try {
      volume_max_ = std::stoi(response.substr(6, 2));
    } catch (...) {
      volume_max_ = 60;
    }
  }
  return volume_max_;
}

// Mute commands
void Denon232Receiver::mute() {
  serial_command("MUON");
}

void Denon232Receiver::unmute() {
  serial_command("MUOFF");
}

void Denon232Receiver::set_mute(bool muted) {
  serial_command(muted ? "MUON" : "MUOFF");
}

bool Denon232Receiver::get_mute_state() {
  return serial_command("MU?", true) == "MUON";
}

// Source commands
void Denon232Receiver::select_source(const std::string &source) {
  std::string cmd = "SI" + source;
  serial_command(cmd);
}

std::string Denon232Receiver::get_source() {
  std::string response = serial_command("SI?", true);
  if (response.length() > 2) {
    return response.substr(2);
  }
  return "";
}

// Additional commands
void Denon232Receiver::set_hdmi_output(uint8_t output) {
  std::string cmd = "DOHDMI";
  if (output == 2) cmd += "/2";
  else if (output == 3) cmd += "/1+2";
  serial_command(cmd);
}

void Denon232Receiver::set_video_input(const std::string &input) {
  serial_command("DVDVI/" + input);
}

void Denon232Receiver::set_surround_mode(const std::string &mode) {
  serial_command("MS" + mode);
}

void Denon232Receiver::set_dynamic_range(uint8_t range) {
  std::string cmd = "MD";
  switch (range) {
    case 0: cmd += "OFF"; break;
    case 1: cmd += "LITE"; break;
    case 2: cmd += "MID"; break;
    case 3: cmd += "MAX"; break;
  }
  serial_command(cmd);
}

void Denon232Receiver::tuner_preset_up() {
  serial_command("TPUP");
}

void Denon232Receiver::tuner_preset_down() {
  serial_command("TPDN");
}

void Denon232Receiver::set_tuner_frequency(const std::string &frequency) {
  serial_command("TF" + frequency);
}

}  // namespace denon232
}  // namespace esphome
