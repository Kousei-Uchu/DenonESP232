#include "web_server.h"
#include "esphome/core/log.h"

namespace esphome {
namespace denon232 {

static const char *const TAG = "denon232.web_server";

void Denon232WebServer::setup() {
  ESP_LOGI(TAG, "Setting up Denon232 Web Server");
  
  if (!web_server_ || !receiver_) {
    ESP_LOGE(TAG, "Web server or receiver not initialized");
    return;
  }
  
  setup_routes_();
  ESP_LOGI(TAG, "Web server routes configured");
}

void Denon232WebServer::loop() {
  // No continuous loop needed
}

float Denon232WebServer::get_setup_priority() const {
  return esphome::setup_priority::AFTER_WIFI;
}

void Denon232WebServer::setup_routes_() {
  // Status endpoint
  web_server_->on_url_match([this](web_server::AsyncWebServerRequest *request) {
    if (request->url() == "/api/status" && request->method() == HTTP_GET) {
      handle_status_(request);
      return true;
    }
    return false;
  });
  
  // Command endpoint
  web_server_->on_url_match([this](web_server::AsyncWebServerRequest *request) {
    if (request->url().startswith("/api/command/") && request->method() == HTTP_POST) {
      std::string command = request->url().substr(13);  // Remove "/api/command/"
      handle_command_(request, command);
      return true;
    }
    return false;
  });
  
  // Query endpoint
  web_server_->on_url_match([this](web_server::AsyncWebServerRequest *request) {
    if (request->url().startswith("/api/query/") && request->method() == HTTP_GET) {
      std::string query = request->url().substr(11);  // Remove "/api/query/"
      handle_query_(request, query);
      return true;
    }
    return false;
  });
  
  // Web interface
  web_server_->on_url_match([this](web_server::AsyncWebServerRequest *request) {
    if (request->url() == "/" || request->url() == "/index.html") {
      // Serve embedded web interface (would be from PROGMEM in production)
      request->send(200, "text/html", "<!-- Web interface would be embedded here -->");
      return true;
    }
    return false;
  });
}

void Denon232WebServer::handle_status_(web_server::AsyncWebServerRequest *request) {
  std::string json = build_status_json_();
  send_json_response_(request, json);
}

void Denon232WebServer::handle_command_(web_server::AsyncWebServerRequest *request, 
                                       const std::string &command) {
  ESP_LOGI(TAG, "Execute command: %s", command.c_str());
  
  receiver_->execute_command(command);
  
  std::string response = R"({"status":"ok","command":")" + command + R"("})";
  send_json_response_(request, response);
}

void Denon232WebServer::handle_query_(web_server::AsyncWebServerRequest *request, 
                                     const std::string &query) {
  ESP_LOGI(TAG, "Query: %s", query.c_str());
  
  std::string result = receiver_->query_state(query);
  
  std::string response = R"({"status":"ok","query":")" + query + R"(","response":")" + result + R"("})";
  send_json_response_(request, response);
}

void Denon232WebServer::send_json_response_(web_server::AsyncWebServerRequest *request, 
                                           const std::string &json) {
  auto response = request->beginResponse(200, "application/json", json.c_str());
  response->addHeader("Access-Control-Allow-Origin", "*");
  response->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  response->addHeader("Access-Control-Allow-Headers", "Content-Type");
  request->send(response);
}

std::string Denon232WebServer::build_status_json_() {
  std::string json;
  json += "{";
  
  std::string power = receiver_->get_power_state();
  json += R"("power":")" + (power == "PWON" ? std::string("on") : std::string("standby")) + R"(",";
  
  uint8_t volume = receiver_->get_volume();
  uint8_t max_vol = receiver_->get_volume_max();
  uint8_t volume_percent = (volume * 100) / max_vol;
  json += R"("volume":)" + std::to_string(volume_percent) + R"(,";
  
  bool muted = receiver_->get_mute_state();
  json += R"("muted":)" + std::string(muted ? "true" : "false") + R"(,";
  
  std::string source = receiver_->get_source();
  json += R"("source":")" + source + R"("";
  
  json += "}";
  return json;
}

}  // namespace denon232
}  // namespace esphome
