#pragma once

#include "esphome/components/remote_transmitter/remote_transmitter.h"
#include "denon232_receiver.h"
#include <string>
#include <map>

namespace esphome {
namespace denon232 {

class Denon232Remote : public remote_transmitter::RemoteTransmitterBase {
 public:
  void set_denon_receiver(Denon232Receiver *receiver) { receiver_ = receiver; }
  
  void setup() override;
  void loop() override;
  float get_setup_priority() const override;
  
  // Send command via receiver
  void send_command(const std::string &command_name);
  
  // Get available commands for UI
  std::vector<std::string> get_command_list() const;

 protected:
  Denon232Receiver *receiver_;

 private:
  std::map<std::string, std::string> command_mapping_;
};

}  // namespace denon232
}  // namespace esphome
