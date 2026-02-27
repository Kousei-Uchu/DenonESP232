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
  traits.set_supports_turn_off_on(true);
  traits.set_supports_volume(true);
  traits.set_supports_mute(true);

  traits.set_supported_sources({
      "CD",
      "DVD",
      "TV/CBL",
      "HDP",
      "V.AUX",
      "NET",
      "USB"
  });

  return traits;
}

void Denon232MediaPlayer::control(const media_player::MediaPlayerCall &call) {
  if (!receiver_) return;

  if (call.get_command().has_value()) {
    if (call.get_command().value() == media_player::MEDIA_PLAYER_COMMAND_TURN_OFF) {
      receiver_->power_off();
    }
    if (call.get_command().value() == media_player::MEDIA_PLAYER_COMMAND_TURN_ON) {
      receiver_->power_on();
    }
  }

  if (call.get_volume().has_value()) {
    float vol = call.get_volume().value();
    uint8_t vol_level = (uint8_t)(vol * receiver_->get_volume_max());
    receiver_->set_volume(vol_level);
  }

  if (call.get_muted().has_value()) {
    receiver_->set_mute(call.get_muted().value());
  }

  if (call.get_source().has_value()) {
    // Map source names to command codes
    std::string source_ = call.get_source().value();
    if (source_ == "CD") receiver_->select_source("CD");
    else if (source_ == "DVD") receiver_->select_source("DVD");
    else if (source_ == "TV/CBL") receiver_->select_source("TV");
    else if (source_ == "HDP") receiver_->select_source("HDP");
    else if (source_ == "V.AUX") receiver_->select_source("AUX");
    else if (source_ == "NET") receiver_->select_source("NET");
    else if (source_ == "USB") receiver_->select_source("USB");
  }

  // Immediate state update
  update_state_();
}

void Denon232MediaPlayer::on_receiver_update(const std::string &key, const std::string &response) {
  ESP_LOGD(TAG, "Live update: %s = %s", key.c_str(), response.c_str());
  parse_response_(key, response);
}

// Parse a decimal integer string without using std::stoi / exceptions.
// Returns true and writes the value to 'out' on success.
// ESP-IDF does not enable C++ exceptions, so try/catch around std::stoi
// would result in a compile error or undefined behaviour.
static bool parse_uint8(const std::string &s, uint8_t &out) {
  if (s.empty()) return false;
  char *end = nullptr;
  long val = std::strtol(s.c_str(), &end, 10);
  if (end == s.c_str() || *end != '\0') return false;  // no digits or trailing garbage
  if (val < 0 || val > 255) return false;
  out = static_cast<uint8_t>(val);
  return true;
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
    // Volume — response is "MV" followed by a decimal number, e.g. "MV45"
    // Use strtol instead of stoi; ESP-IDF builds with -fno-exceptions.
    if (response.size() > 2) {
      uint8_t vol = 0;
      if (parse_uint8(response.substr(2), vol)) {
        uint8_t max_vol = receiver_->get_volume_max();
        if (max_vol > 0) {
          this->volume = static_cast<float>(vol) / max_vol;
        }
      }
    }
  } else if (key == "MU") {
    // Mute
    this->muted_ = (response == "MUON");
  } else if (key == "SI") {
    // Source
    std::string src = response.substr(2);
    if (src == "CD") this->source_ = "CD";
    else if (src == "DVD") this->source_ = "DVD";
    else if (src == "TV") this->source_ = "TV/CBL";
    else if (src == "HDP") this->source_ = "HDP";
    else if (src == "AUX") this->source_ = "V.AUX";
    else if (src == "NET") this->source_ = "NET";
    else if (src == "USB") this->source_ = "USB";
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
      this->volume = static_cast<float>(volume) / max_vol;
    }

    this->muted_ = receiver_->get_mute_state();

    std::string source_ = receiver_->get_source();
    if (source_ == "CD") {
      this->source_ = "CD";
    } else if (source_ == "DVD") {
      this->source_ = "DVD";
    } else if (source_ == "TV") {
      this->source_ = "TV/CBL";
    } else if (source_ == "HDP") {
      this->source_ = "HDP";
    } else if (source_ == "AUX") {
      this->source_ = "V.AUX";
    } else if (source_ == "NET") {
      this->source_ = "NET";
    } else if (source_ == "USB") {
      this->source_ = "USB";
    }
  }

  this->publish_state();
}

}  // namespace denon232
}  // namespace esphome