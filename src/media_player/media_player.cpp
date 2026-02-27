#include "media_player.h"
#include "esphome/core/log.h"

namespace esphome {
namespace denon232 {

static const char *const TAG = "denon232.media_player";

void Denon232MediaPlayer::setup() {
  ESP_LOGI(TAG, "Setting up Denon232 Media Player");
  
  // Set response callback for live updates
  if (receiver_) {
    receiver_->set_response_callback([this](const std::string &key, const std::string &response) {
      this->on_receiver_update(key, response);
    });
  }
  
  update_state_();
}

void Denon232MediaPlayer::loop() {
  // Loop is handled by receiver component
}

void Denon232MediaPlayer::update() {
  uint32_t now = millis();
  if (now - last_update_ > polling_interval_) {
    last_update_ = now;
    update_state_();
  }
}

float Denon232MediaPlayer::get_setup_priority() const {
  return esphome::setup_priority::AFTER_WIFI;
}

media_player::MediaPlayerTraits Denon232MediaPlayer::get_traits() {
  auto traits = media_player::MediaPlayerTraits();
  traits.set_supports_pause(false);
  traits.set_supports_seek(false);
  traits.set_supports_play(false);
  traits.set_supports_turn_off(true);
  traits.set_supports_turn_on(true);
  traits.set_supports_volume_set(true);
  traits.set_supports_volume_step(true);
  traits.set_supports_mute(true);
  traits.set_supports_select_source(true);
  
  // Define available sources
  traits.add_source("CD");
  traits.add_source("DVD");
  traits.add_source("TV/CBL");
  traits.add_source("HDP");
  traits.add_source("V.AUX");
  traits.add_source("NET");
  traits.add_source("USB");
  
  return traits;
}

void Denon232MediaPlayer::control(const media_player::MediaPlayerCall &call) {
  if (!receiver_) return;
  
  if (call.get_state().has_value()) {
    if (call.get_state().value() == media_player::MEDIA_PLAYER_STATE_OFF) {
      receiver_->power_off();
    } else if (call.get_state().value() == media_player::MEDIA_PLAYER_STATE_ON) {
      receiver_->power_on();
    }
  }
  
  if (call.get_volume().has_value()) {
    float vol = call.get_volume().value();
    uint8_t vol_level = (uint8_t)(vol * receiver_->get_volume_max());
    receiver_->set_volume(vol_level);
  }
  
  if (call.get_mute().has_value()) {
    receiver_->set_mute(call.get_mute().value());
  }
  
  if (call.get_source().has_value()) {
    // Map source names to command codes
    std::string source = call.get_source().value();
    if (source == "CD") receiver_->select_source("CD");
    else if (source == "DVD") receiver_->select_source("DVD");
    else if (source == "TV/CBL") receiver_->select_source("TV");
    else if (source == "HDP") receiver_->select_source("HDP");
    else if (source == "V.AUX") receiver_->select_source("AUX");
    else if (source == "NET") receiver_->select_source("NET");
    else if (source == "USB") receiver_->select_source("USB");
  }
  
  // Immediate state update
  update_state_();
}

void Denon232MediaPlayer::on_receiver_update(const std::string &key, const std::string &response) {
  ESP_LOGD(TAG, "Live update: %s = %s", key.c_str(), response.c_str());
  parse_response_(key, response);
}

void Denon232MediaPlayer::parse_response_(const std::string &key, const std::string &response) {
  if (key == "PW") {
    // Power state
    if (response == "PWON") {
      this->state = media_player::MEDIA_PLAYER_STATE_ON;
    } else if (response == "PWSTANDBY") {
      this->state = media_player::MEDIA_PLAYER_STATE_OFF;
    }
  } else if (key == "MV") {
    // Volume
    try {
      uint8_t vol = std::stoi(response.substr(2));
      uint8_t max_vol = receiver_->get_volume_max();
      if (max_vol > 0) {
        this->volume = (float)vol / max_vol;
      }
    } catch (...) {}
  } else if (key == "MU") {
    // Mute
    this->muted = (response == "MUON");
  } else if (key == "SI") {
    // Source
    std::string src = response.substr(2);
    if (src == "CD") this->source = "CD";
    else if (src == "DVD") this->source = "DVD";
    else if (src == "TV") this->source = "TV/CBL";
    else if (src == "HDP") this->source = "HDP";
    else if (src == "AUX") this->source = "V.AUX";
    else if (src == "NET") this->source = "NET";
    else if (src == "USB") this->source = "USB";
  }
  
  this->publish_state();
}

void Denon232MediaPlayer::update_state_() {
  if (!receiver_) {
    ESP_LOGE(TAG, "Receiver not initialized");
    return;
  }
  
  // Poll all states
  std::string pw_state = receiver_->get_power_state();
  if (pw_state == "PWON") {
    this->state = media_player::MEDIA_PLAYER_STATE_ON;
  } else if (pw_state == "PWSTANDBY") {
    this->state = media_player::MEDIA_PLAYER_STATE_OFF;
  }
  
  if (this->state == media_player::MEDIA_PLAYER_STATE_ON) {
    uint8_t volume = receiver_->get_volume();
    uint8_t max_vol = receiver_->get_volume_max();
    if (max_vol > 0) {
      this->volume = (float)volume / max_vol;
    }
    
    this->muted = receiver_->get_mute_state();
    
    std::string source = receiver_->get_source();
    if (source == "CD") {
      this->source = "CD";
    } else if (source == "DVD") {
      this->source = "DVD";
    } else if (source == "TV") {
      this->source = "TV/CBL";
    } else if (source == "HDP") {
      this->source = "HDP";
    } else if (source == "AUX") {
      this->source = "V.AUX";
    } else if (source == "NET") {
      this->source = "NET";
    } else if (source == "USB") {
      this->source = "USB";
    }
  }
  
  this->publish_state();
}

}  // namespace denon232
}  // namespace esphome
