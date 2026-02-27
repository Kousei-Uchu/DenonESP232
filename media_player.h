#pragma once

#include "esphome/components/media_player/media_player.h"
#include "denon232_receiver.h"
#include <string>

namespace esphome {
namespace denon232 {

class Denon232MediaPlayer : public media_player::MediaPlayer, public Component {
 public:
  void set_denon_receiver(Denon232Receiver *receiver) { receiver_ = receiver; }
  void set_cable_mode(CableMode mode) { 
    if (receiver_) receiver_->set_cable_mode(mode);
  }
  void set_polling_interval(uint32_t interval) { polling_interval_ = interval; }
  
  void setup() override;
  void loop() override;
  void update() override;
  float get_setup_priority() const override;
  
  // Media player methods
  media_player::MediaPlayerTraits get_traits() override;
  void control(const media_player::MediaPlayerCall &call) override;
  
  // State callbacks
  void on_receiver_update(const std::string &key, const std::string &response);

 protected:
  Denon232Receiver *receiver_;
  uint32_t last_update_ = 0;
  uint32_t polling_interval_ = 5000;
  bool has_source_info_ = false;

 private:
  void update_state_();
  void parse_response_(const std::string &key, const std::string &response);
};

}  // namespace denon232
}  // namespace esphome
