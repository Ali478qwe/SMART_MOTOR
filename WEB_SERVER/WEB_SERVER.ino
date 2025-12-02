#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

bool flag = true; 

unsigned long previousMillis = 0;
const long interval = 1000;

const char* ssid = "َc++";
const char* password = "12345678";

const char * file = "/index.html";

uint8_t PIN_TEMP_OIL = 32;
uint8_t PIN_TEMP_AIR = 33;
uint8_t PIN_LEVEL_OIL = 34;
uint8_t PIN_VOLTAGE_BATTERY = 35;

AsyncWebServer server(80);
AsyncWebSocket web_socket("/ws");

IPAddress local_IP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

String wsMessage = "";
bool wsNewMessage = false;


//create a customer class from websocket header

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    
    String msg = "";
    for(size_t i = 0 ; i < len ; i++)
    {
        msg += (char)data[i];
    }
    wsMessage = msg ; 
    wsNewMessage = true;   

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
  int level_oil = analogRead(PIN_LEVEL_OIL);
  int temp_oil = analogRead(PIN_TEMP_OIL);
  int temp_air = analogRead(PIN_TEMP_AIR);
  int voltage_battery = analogRead(PIN_VOLTAGE_BATTERY);

    // Serial.println("level_oil -> " + String(level_oil));
    // Serial.println("temp_oil ->" + String(temp_oil));
    //   Serial.println("temp_air ->" + String(temp_air));
    //     Serial.println("voltage_battery ->" + String(voltage_battery));

  StaticJsonDocument<200> json;

  json["level_oil"] = String(level_oil);
  json["temp_oil"] = String(temp_oil);
  json["temp_air"] = String(temp_air);
  json["voltage_battery"] = String(voltage_battery);
  
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

void analog_test(void){
    uint8_t pin[8] = {36,37,38,39,32,33,34,35};
    for(uint8_t i = 0 ; i < 8 ; i++ ){     
      pinMode(pin[i],INPUT);
      float val = analogRead(pin[i]);
      if(val > 0){
        Serial.println("PIN -> " + String(int(pin[i])) + " VALUE ->" + String(val));
        
      }
      delay(1000);
    }
}

void setup(){

  pinMode(PIN_TEMP_OIL,INPUT);
  pinMode(PIN_TEMP_AIR,INPUT);
  pinMode(PIN_LEVEL_OIL,INPUT);
  pinMode(PIN_VOLTAGE_BATTERY,INPUT);

  Serial.begin(115200);
  // WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(ssid, password);
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  web_socket.onEvent(web_socket_handler);
  server.addHandler(&web_socket);

  initSPIFFS();
  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html").setCacheControl("no-cache, no-store, must-revalidate");
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
  // // delay(200); // هر 200 میلی‌ثانیه یکبار ارسال می‌شود (قابل تنظیم)
  
  // if(flag){
  //     analog_test();
  //     flag = false;
  // }
  // unsigned long currentMillis = millis();
  // if(currentMillis - previousMillis >= interval){
  //   previousMillis = currentMillis;
  //   Read_Sensor();
  //   // analog_test();
  // }


  if(wsNewMessage) {

    wsNewMessage = false;  // پرچم ریست شود
    
    if(wsMessage == "message_test") {
      Serial.println("Message Test Triggered");
      wsMessage = "";
      // عملیات سنگین نکن!
    }
    else if(wsMessage == "call_test") {
      Serial.println("Call Test Triggered");
      wsMessage = "";
    }
    else if(wsMessage == "reset_runtime") {
      Serial.println("Reset Runtime Triggered");
      wsMessage = "";
    }
  }

  web_socket.cleanupClients(); 

}
