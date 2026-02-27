#include "remote.h"
#include "esphome/core/log.h"

namespace esphome {
namespace denon232 {

static const char *const TAG = "denon232.remote";

void Denon232Remote::setup() {
  ESP_LOGI(TAG, "Setting up Denon232 Remote");
}

void Denon232Remote::loop() {
}

float Denon232Remote::get_setup_priority() const {
  return esphome::setup_priority::AFTER_WIFI;
}

void Denon232Remote::send_command(const std::string &command_name) {
  if (receiver_) {
    ESP_LOGI(TAG, "Sending remote command: %s", command_name.c_str());
    receiver_->execute_command(command_name);
  }
}

std::vector<std::string> Denon232Remote::get_command_list() const {
  std::vector<std::string> commands;
  
  if (receiver_) {
    auto cmds = receiver_->get_available_commands();
    for (const auto &cmd : cmds) {
      commands.push_back(cmd.name);
    }
  }
  
  return commands;
}

}  // namespace denon232
}  // namespace esphome
