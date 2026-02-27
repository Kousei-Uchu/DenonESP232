#pragma once

#include "esphome/core/component.h"
#include "denon232_receiver.h"
#include <string>
#include <vector>
#include <cstring>

namespace esphome {
namespace denon232 {

enum class DebugLevel {
  NONE = 0,
  ERROR = 1,
  WARNING = 2,
  INFO = 3,
  DEBUG = 4,
  VERBOSE = 5
};

struct DebugLogEntry {
  uint32_t timestamp;
  DebugLevel level;
  std::string message;
  std::string tag;
};

class Denon232DebugMonitor : public Component {
 public:
  void set_denon_receiver(Denon232Receiver *receiver) { receiver_ = receiver; }
  void set_debug_level(DebugLevel level) { debug_level_ = level; }
  void enable_serial_debug(bool enable) { serial_debug_ = enable; }
  void enable_state_tracking(bool enable) { track_state_ = enable; }
  
  void log_debug(const std::string &message, const std::string &tag = "denon232");
  void log_info(const std::string &message, const std::string &tag = "denon232");
  void log_warning(const std::string &message, const std::string &tag = "denon232");
  void log_error(const std::string &message, const std::string &tag = "denon232");
  
  void setup() override;
  void loop() override;
  float get_setup_priority() const override;
  
  // Get diagnostic info
  std::string get_diagnostics();
  std::vector<DebugLogEntry> get_log_buffer() const { return log_buffer_; }
  void clear_log_buffer();
  
  // State tracking
  void track_command(const std::string &cmd);
  void track_response(const std::string &response);
  
 protected:
  Denon232Receiver *receiver_;
  DebugLevel debug_level_ = DebugLevel::INFO;
  bool serial_debug_ = true;
  bool track_state_ = true;
  
  std::vector<DebugLogEntry> log_buffer_;
  const size_t max_log_entries_ = 256;
  
  struct StateTracking {
    uint32_t command_count = 0;
    uint32_t response_count = 0;
    uint32_t error_count = 0;
    uint32_t last_command_time = 0;
    std::string last_command;
    std::string last_response;
  } state_;

 private:
  void add_log_entry_(DebugLevel level, const std::string &tag, const std::string &message);
  void print_log_entry_(const DebugLogEntry &entry);
};

}  // namespace denon232
}  // namespace esphome
