#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <LittleFS.h>
#include <ArduinoJson.h>



const char* ssid = "ESP32_WS_AP";
const char* password = "12345678";

AsyncWebServer server(80);
AsyncWebSocket web_socket("/ws");

//create a customer class from websocket header

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    String msg = (char*)data;
    Serial.printf("Received WS message: %s\n", msg.c_str());

    if (msg == "PING") {
      web_socket.textAll("PONG");
    }
  }
}

//create a customer class from websocket header

void web_socket_handler(AsyncWebSocket * server, AsyncWebSocketClient * client,
             AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void Read_Sensor(void){
  float level_oil = analogRead(26);
  float temp_oil = analogRead(14);
  float temp_air = analogRead(27);
  float voltage_battery = analogRead(12);

  StaticJsonDocument<200> json;

  json["level_oil"] = level_oil;
  json["temp_oil"] = temp_oil;
  json["temp_air"] = temp_air;
  json["voltage_battery"] = voltage_battery;
  
  String output;

  serializeJson(json , output);

  web_socket.textAll(output);

}

void setup(){
  Serial.begin(115200);
  WiFi.softAP(ssid, password);
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  web_socket.onEvent(web_socket_handler);
  server.addHandler(&web_socket);

  if (!LittleFS.begin()) {
    Serial.println("LittleFS Mount Failed");
    return;
}

server.serveStatic("/", LittleFS, "/");





  server.begin();
}

void loop(){
  Read_Sensor();
  delay(200); // هر 200 میلی‌ثانیه یکبار ارسال می‌شود (قابل تنظیم)
}
