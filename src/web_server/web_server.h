#pragma once

#include "esphome/core/component.h"
#include "esphome/components/web_server/web_server.h"
#include "denon232_receiver.h"
#include <string>
#include <vector>

namespace esphome {
namespace denon232 {

class Denon232WebServer : public Component {
 public:
  void set_denon_receiver(Denon232Receiver *receiver) { receiver_ = receiver; }
  void set_web_server(web_server::WebServer *server) { web_server_ = server; }
  
  void setup() override;
  void loop() override;
  float get_setup_priority() const override;

 protected:
  Denon232Receiver *receiver_;
  web_server::WebServer *web_server_;
  
 private:
  void setup_routes_();
  void handle_status_(web_server::AsyncWebServerRequest *request);
  void handle_command_(web_server::AsyncWebServerRequest *request, const std::string &command);
  void handle_query_(web_server::AsyncWebServerRequest *request, const std::string &query);
  void send_json_response_(web_server::AsyncWebServerRequest *request, const std::string &json);
  std::string build_status_json_();
};

}  // namespace denon232
}  // namespace esphome
