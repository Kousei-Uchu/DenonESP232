#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include <string>
#include <vector>
#include <map>
#include <functional>

namespace esphome {
namespace denon232 {

// Command categories for organization
enum class CommandCategory {
  POWER,
  VOLUME,
  MUTE,
  SOURCE,
  PICTURE,
  SOUND,
  TUNER,
  VIDEO,
  MISC
};

// Cable mode for RX/TX pin mapping
enum class CableMode {
  NULL_MODEM,      // Standard - RX on pin 2, TX on pin 3
  PASS_THROUGH     // Reversed - RX on pin 3, TX on pin 2
};

struct DenonCommand {
  std::string name;
  std::string command;
  CommandCategory category;
  bool expects_response;
  std::string description;
};

class Denon232Receiver : public Component {
 public:
  void set_uart_parent(uart::UARTComponent *parent) { uart_ = parent; }
  void set_cable_mode(CableMode mode) { cable_mode_ = mode; }
  void set_response_callback(std::function<void(const std::string &, const std::string &)> callback) {
    response_callback_ = callback;
  }

  // Core serial command function
  std::string serial_command(const std::string &cmd, bool response = false);
  
  // Execute command by name
  void execute_command(const std::string &command_name);
  
  // Query state
  std::string query_state(const std::string &query_cmd);

  // Power commands
  void power_on();
  void power_off();
  std::string get_power_state();

  // Volume commands
  void volume_up();
  void volume_down();
  void set_volume(uint8_t volume);
  uint8_t get_volume();
  uint8_t get_volume_max();

  // Mute commands
  void mute();
  void unmute();
  void set_mute(bool muted);
  bool get_mute_state();

  // Source commands
  void select_source(const std::string &source);
  std::string get_source();

  // Input/Output commands
  void set_hdmi_output(uint8_t output);
  void set_video_input(const std::string &input);

  // Audio mode commands
  void set_surround_mode(const std::string &mode);
  void set_dynamic_range(uint8_t range);

  // Tuner commands
  void tuner_preset_up();
  void tuner_preset_down();
  void set_tuner_frequency(const std::string &frequency);

  // Get all available commands
  std::vector<DenonCommand> get_available_commands() const;
  DenonCommand get_command_info(const std::string &name) const;

  void setup() override;
  void loop() override;
  float get_setup_priority() const override;

 protected:
  uart::UARTComponent *uart_ = nullptr;
  CableMode cable_mode_ = CableMode::NULL_MODEM;
  uint8_t volume_max_ = 60;
  uint32_t last_poll_ = 0;
  const uint32_t poll_interval_ = 30000;  // 30 seconds
  bool connection_active_ = false;
  std::function<void(const std::string &, const std::string &)> response_callback_;

  // Command database
  std::map<std::string, DenonCommand> commands_;
  void init_command_database_();

 private:
  std::string read_response_line_();
  void write_command_(const std::string &cmd);
  void process_unsolicited_response_(const std::string &response);
};

}  // namespace denon232
}  // namespace esphome
