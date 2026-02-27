#include "debug_monitor.h"
#include "esphome/core/log.h"
#include <ctime>
#include <sstream>
#include <iomanip>

namespace esphome {
namespace denon232 {

static const char *const TAG = "denon232.debug";

void Denon232DebugMonitor::setup() {
  ESP_LOGI(TAG, "Debug Monitor initialized - Level: %d", static_cast<int>(debug_level_));
  log_info("Denon232 Debug Monitor started");
}

void Denon232DebugMonitor::loop() {
  // Continuous monitoring could be added here
}

void Denon232DebugMonitor::update() {
  // Periodic diagnostics could be printed here
  if (track_state_) {
    // Could periodically log state info
  }
}

float Denon232DebugMonitor::get_setup_priority() const {
  return esphome::setup_priority::AFTER_WIFI - 1;
}

void Denon232DebugMonitor::add_log_entry_(DebugLevel level, const std::string &tag, 
                                          const std::string &message) {
  if (level > debug_level_) {
    return;  // Don't log if below current level
  }
  
  DebugLogEntry entry;
  entry.timestamp = millis();
  entry.level = level;
  entry.tag = tag;
  entry.message = message;
  
  log_buffer_.push_back(entry);
  
  if (log_buffer_.size() > max_log_entries_) {
    log_buffer_.erase(log_buffer_.begin());
  }
  
  if (serial_debug_) {
    print_log_entry_(entry);
  }
}

void Denon232DebugMonitor::print_log_entry_(const DebugLogEntry &entry) {
  const char *level_str = "";
  switch (entry.level) {
    case DebugLevel::ERROR:
      level_str = "ERR";
      break;
    case DebugLevel::WARNING:
      level_str = "WRN";
      break;
    case DebugLevel::INFO:
      level_str = "INF";
      break;
    case DebugLevel::DEBUG:
      level_str = "DBG";
      break;
    case DebugLevel::VERBOSE:
      level_str = "VRB";
      break;
    default:
      level_str = "???";
  }
  
  uint32_t seconds = entry.timestamp / 1000;
  uint32_t millis_part = entry.timestamp % 1000;
  
  ESP_LOGI(TAG, "[%s][%s][%5lu.%03lu] %s", 
           level_str, entry.tag.c_str(), seconds, millis_part, entry.message.c_str());
}

void Denon232DebugMonitor::log_error(const std::string &message, const std::string &tag) {
  add_log_entry_(DebugLevel::ERROR, tag, message);
  if (track_state_) {
    state_.error_count++;
  }
}

void Denon232DebugMonitor::log_warning(const std::string &message, const std::string &tag) {
  add_log_entry_(DebugLevel::WARNING, tag, message);
}

void Denon232DebugMonitor::log_info(const std::string &message, const std::string &tag) {
  add_log_entry_(DebugLevel::INFO, tag, message);
}

void Denon232DebugMonitor::log_debug(const std::string &message, const std::string &tag) {
  add_log_entry_(DebugLevel::DEBUG, tag, message);
}

void Denon232DebugMonitor::track_command(const std::string &cmd) {
  if (!track_state_) return;
  
  state_.command_count++;
  state_.last_command = cmd;
  state_.last_command_time = millis();
  
  std::string msg = "→ CMD: " + cmd;
  log_debug(msg, "serial");
}

void Denon232DebugMonitor::track_response(const std::string &response) {
  if (!track_state_) return;
  
  state_.response_count++;
  state_.last_response = response;
  
  std::string msg = "← RSP: " + response;
  log_debug(msg, "serial");
}

void Denon232DebugMonitor::clear_log_buffer() {
  log_buffer_.clear();
  log_info("Log buffer cleared");
}

std::string Denon232DebugMonitor::get_diagnostics() {
  std::stringstream ss;
  
  ss << "=== Denon232 Diagnostics ===\n";
  ss << "Debug Level: " << static_cast<int>(debug_level_) << "\n";
  ss << "Commands sent: " << state_.command_count << "\n";
  ss << "Responses received: " << state_.response_count << "\n";
  ss << "Errors: " << state_.error_count << "\n";
  ss << "Last command: " << state_.last_command << "\n";
  ss << "Last response: " << state_.last_response << "\n";
  
  if (receiver_) {
    ss << "\nReceiver Status:\n";
    ss << "Power: " << receiver_->get_power_state() << "\n";
    ss << "Volume: " << (int)receiver_->get_volume() << "/" 
       << (int)receiver_->get_volume_max() << "\n";
    ss << "Muted: " << (receiver_->get_mute_state() ? "yes" : "no") << "\n";
    ss << "Source: " << receiver_->get_source() << "\n";
  }
  
  ss << "\nRecent Log (last 10 entries):\n";
  int start = std::max(0, static_cast<int>(log_buffer_.size()) - 10);
  for (size_t i = start; i < log_buffer_.size(); i++) {
    const auto &entry = log_buffer_[i];
    ss << "[" << entry.tag << "] " << entry.message << "\n";
  }
  
  return ss.str();
}

}  // namespace denon232
}  // namespace esphome
