#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

unsigned long previousMillis = 0;
const long interval = 200;

const char* ssid = "َc++";
const char* password = "12345678";

const char * file = "/index.html";

AsyncWebServer server(80);
AsyncWebSocket web_socket("/ws");

IPAddress local_IP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

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

void initSPIFFS() {
  if (!SPIFFS.begin(false)) {
    Serial.println("An error has occurred while mounting SPIFFS");
    return;
  }
  Serial.println("SPIFFS mounted successfully");
  if (SPIFFS.exists(file)) {
    Serial.println("file exists");
  } else {
    Serial.println("file not found");
  }
}

void setup(){
  Serial.begin(115200);
  // WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(ssid, password);
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  web_socket.onEvent(web_socket_handler);
  server.addHandler(&web_socket);

  initSPIFFS();
  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");
  // server.serveStatic("/", SPIFFS, file);
// server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
//     File file = SPIFFS.open("/index.html", "r");
//    AsyncWebServerResponse *response =
//     request->beginResponse(file, String("/index.html"), String("text/html"));
//     request->send(response);
// });






  server.begin();
}

void loop(){
  // Read_Sensor();
  // delay(200); // هر 200 میلی‌ثانیه یکبار ارسال می‌شود (قابل تنظیم)
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis >= interval){
    previousMillis = currentMillis;
    Read_Sensor();
  }
  web_socket.cleanupClients(); // نگهداری از WebSocket‌ها

}
