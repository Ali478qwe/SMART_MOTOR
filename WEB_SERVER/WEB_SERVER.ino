#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

const char* ssid = "ESP32_WS_AP";
const char* password = "12345678";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

//create a customer class from websocket header

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    String msg = (char*)data;
    Serial.printf("Received WS message: %s\n", msg.c_str());

    if (msg == "PING") {
      ws.textAll("PONG");
    }
  }
}

//create a customer class from websocket header

void onEvent(AsyncWebSocket * server, AsyncWebSocketClient * client,
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

const char html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    <title>ESP32 WebSocket Demo</title>
  </head>
  <body>
    <h2>Analog Value (via WebSocket):</h2>
    <p id="val">---</p>
    <button onclick="sendPing()">Send PING</button>
    <script>
      var websocket = new WebSocket('ws://' + location.host + '/ws');
      websocket.onopen = function(event) {
        console.log('WebSocket open');
      };
      websocket.onmessage = function(event) {
        document.getElementById('val').innerText = event.data;
      };
      function sendPing() {
        websocket.send('PING');
      }
    </script>
  </body>
</html>
)rawliteral";

void setup(){
  Serial.begin(115200);
  WiFi.softAP(ssid, password);
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  ws.onEvent(onEvent);
  server.addHandler(&ws);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", html);
  });

  server.begin();
}

void loop(){
  int analogValue = analogRead(34);  // مثال: پین 34 برای ADC
  ws.textAll(String(analogValue));
  delay(200); // هر 200 میلی‌ثانیه یکبار ارسال می‌شود (قابل تنظیم)
}
