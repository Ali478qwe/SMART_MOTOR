#include <TinyGPS++.h>
#include <TinyGPSPlus.h>
#include <HardwareSerial.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <Preferences.h>



uint32_t last_time = 0;
uint32_t last_runtime = 0;
uint32_t last_send_data = 0;

//Access Point Variable
const char *ssid = "َc++";
const char *password = "12345678";

//main file set on web server
const char *file = "/index.html";

//Sensor Pin
uint8_t PIN_TEMP_OIL = 32;
uint8_t PIN_TEMP_AIR = 33;
uint8_t PIN_LEVEL_OIL = 34;
uint8_t PIN_VOLTAGE_BATTERY = 35;

//UART PIN FOR GPS CONNECTION
static const uint8_t RX_PIN_tTX_GPS = 4, TX_PIN_tRX_GPS = 5;

//UART PIN FOR SIM800 CONNECTION
static const uint8_t RX_PIN_tTX_SIM800 = 16, TX_PIN_tRX_SIM800 = 17;

//EEPROM Class instance
Preferences _flash;

//GPS CLASS FOR DATA ANALYSIS
TinyGPSPlus gps;


AsyncWebServer server(80);         //Create Web-Server on Port 80
AsyncWebSocket web_socket("/ws");  //Create Web-Socket

//IP Config
IPAddress local_IP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

//DEFINE UART NAME PORT FOR GSM
HardwareSerial UART_SIM800(2);

//DEFINE UART NAME PORT FOR GPS
HardwareSerial UART_GPS(1);

//Global Variable For Analysis Recived Message From WebSocket
String wsMessage = "";
bool wsNewMessage = false;

//For Parsing SMS Message Structure
String status, sender, dateTime, text;

//This  Found Number Of SMS Function
String SMS_NUM(String text) {
  int token = text.indexOf(",");
  if (token != -1) {
    return text.substring(token + 1);
  }
}

//This is SMS parser function.
void SMS_PARSER(String massage, String &status, String &sender, String &dateTime, String &text) {
  int firstQuote = massage.indexOf('"');
  int secondQuote = massage.indexOf('"', firstQuote + 1);
  status = massage.substring(firstQuote + 1, secondQuote);

  int thirdQuote = massage.indexOf('"', secondQuote + 1);
  int fourthQuote = massage.indexOf('"', thirdQuote + 1);
  sender = massage.substring(thirdQuote + 1, fourthQuote);

  int fifthQuote = massage.indexOf('"', fourthQuote + 4);
  int sixthQuote = massage.indexOf('"', fifthQuote + 1);
  dateTime = massage.substring(fifthQuote + 1, sixthQuote);

  text = massage.substring(sixthQuote + 1);
}

//Use This function For Sent Messaage for Any Phone Number
void SEND_SMS(String number, String text) {
  UART_SIM800.println("AT+CMGS=\"" + number + "\"");
  unsigned long start = millis();

  while (!UART_SIM800.available() && millis() - start < 3000) {}
  delay(100);
  UART_SIM800.println(text);
  delay(500);
  UART_SIM800.write(26);  // CTRL+Z
}

// Clean Uart data from noise and unreadable characters
String cleanResponse(String raw) {
  String result = "";
  for (int i = 0; i < raw.length(); i++) {
    char c = raw[i];
    if (isPrintable(c) || c == '\n' || c == '\r') {
      result += c;
    }
  }
  return result;
}

//َA function to verify authentication
String Verification(String text, String Key_Word, String Password) {

  text.trim();

  int first_index = text.indexOf(Key_Word);
  int second_index = text.indexOf(":");

  if (first_index != -1 && second_index != -1) {

    String User_Key_Word = text.substring(first_index, second_index);
    String User_Password = text.substring(second_index + 1, second_index + Password.length() + 1);
    // User_Password.trim();
    Serial.println(String("User_Key_Word:" + User_Key_Word));
    Serial.println(String("User_Password:" + User_Password));

    User_Key_Word.trim();
    User_Password.trim();

    if (User_Key_Word == Key_Word) {
      if (User_Password == Password) {
        return "VERIFIED";
      } else {
        return "F_PASSWORD";
      }
    } else {
      return "F_KEY_WORD";
    }
  }

  return "FORMAT_ERR";
}

//setup sim800
void SIM800_init(void) {
  String commands[] = { "ATE0", "AT", "ATI", "AT+CSQ", "AT+CCID", "AT+CPIN?",
                        "AT+CREG?", "AT+CBC", "AT+CMGF=1", "AT+CFUN?", "AT+CSCLK?" };
  size_t n = sizeof(commands) / sizeof(commands[0]);
  for (size_t command = 0; command < n; command) {
    UART_SIM800.println(commands[command]);
    delay(500);
  }
}

//web socket messaage handler
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {

    String msg = "";
    for (size_t i = 0; i < len; i++) {
      msg += (char)data[i];
    }
    wsMessage = msg;
    wsNewMessage = true;
  }
}

//web socket connection handler
void web_socket_handler(AsyncWebSocket *server, AsyncWebSocketClient *client,
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

//Read Sensor Data and Sent to web page
void Read_Sensor(void) {
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

  serializeJson(json, output);

  web_socket.textAll(output);
}

//Checking existence of the File System and file
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

void setup() {
  //INPUT Sensor Pin
  pinMode(PIN_TEMP_OIL, INPUT);
  pinMode(PIN_TEMP_AIR, INPUT);
  pinMode(PIN_LEVEL_OIL, INPUT);
  pinMode(PIN_VOLTAGE_BATTERY, INPUT);


  //ACTIVATE SERIAL MONITRO
  Serial.begin(115200);

  _flash.begin("runtime", false);



  WiFi.softAP(ssid, password);
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  //add web socket connection event
  web_socket.onEvent(web_socket_handler);
  server.addHandler(&web_socket);  //add web socket handler to web server route

  initSPIFFS();

  //set static file to main address of server
  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html").setCacheControl("no-cache, no-store, must-revalidate");

  //ACTIVATE GPS UART
  //TIP : Baud rate = 9600
  UART_GPS.begin(9600, SERIAL_8N1, RX_PIN_tTX_GPS, TX_PIN_tRX_GPS);
  Serial.println("UART GPS Started");

  //ACTIVATE GSM UART
  //TIP : Baud rate = 9600
  UART_SIM800.begin(9600, SERIAL_8N1, RX_PIN_tTX_SIM800, TX_PIN_tRX_SIM800);
  Serial.println("UART SIM800 Started");
  delay(1000);
  SIM800_init();

  server.begin();
}

void loop() {

  static uint32_t now = millis();

  if (now - last_send_data >= 1000) {
    last_send_data = now;
    Read_Sensor();
  }


  if (now - last_time >= 5000) {
    last_time = now;

    last_runtime = _flash.getUInt("runtime", 0);
    last_runtime += 5;

    _flash.putUInt("runtime", last_runtime);
  }

  //Condition relative to the message
  if (wsNewMessage) {

    wsNewMessage = false;

    if (wsMessage == "message_test") {
      Serial.println("Message Test Triggered");
      wsMessage = "";
      SEND_SMS("+989*********", "TEST MESSAGE");
    } else if (wsMessage == "call_test") {
      Serial.println("Call Test Triggered");
      wsMessage = "";
    } else if (wsMessage == "reset_runtime") {
      Serial.println("Reset Runtime Triggered");
      _flash.putUInt("runtime", 0);
      wsMessage = "";
    }
  }

  //Read Data from Uart sim800
  if (Serial.available()) {
    UART_SIM800.println(Serial.readStringUntil('\n'));
  }

  //Response to sim800
  if (UART_SIM800.available()) {
    String response = UART_SIM800.readString();
    response = cleanResponse(response);
    Serial.println(response);

    if (response.indexOf("+CMTI:") != -1) {

      String sms_num = SMS_NUM(response);
      UART_SIM800.println(String("AT+CMGR=" + sms_num));
      Serial.println(String("AT+CMGR=" + sms_num + "serial"));


      String RESPONSE_CMGR = UART_SIM800.readString();

      Serial.println(RESPONSE_CMGR);
      if (RESPONSE_CMGR.indexOf("+CMGR:") != -1) {

        SMS_PARSER(RESPONSE_CMGR, status, sender, dateTime, text);
        Serial.println(status);
        Serial.println(sender);
        Serial.println(dateTime);
        Serial.println(text);
        //verification text : location:1234
        String verify = Verification(text, "location", "Ali478qwe");
        Serial.println(verify);
        if (verify == "VERIFIED") {

          while (UART_GPS.available()) {
            gps.encode(UART_GPS.read());
          }

          if (gps.satellites.isValid()) {
            Serial.println("Satellite Found");
            //GPS VALUE
            Serial.println(String("Satellite Value: " + String(gps.satellites.value())));
          } else {
            Serial.println("Satellite Not Found");
          }  //IS_VALID

          if (gps.location.isValid()) {
            Serial.println("Location Found");

            if (gps.location.isUpdated()) {
              //PRINT DAtA
              Serial.println("GEOGRAPHICAL LOCATION");
              Serial.println("Geo width:");
              Serial.println(gps.location.lat(), 6);
              Serial.println("Geo Length:");
              Serial.println(gps.location.lng(), 6);

              //LOCATION LINKS

              String location_links = String("https://google.com/maps?q=" + String(gps.location.lat(), 6) + "," + String(gps.location.lng(), 6)) + "\n\n" + String("https://neshan.org/maps?lat=" + String(gps.location.lat(), 6) + "&lng=" + String(gps.location.lng(), 6)) + "\n\n" + String("https://balad.ir/location?latitude=" + String(gps.location.lat(), 6) + "&longitude=" + String(gps.location.lng(), 6));
              Serial.println(location_links);
              Serial.println("_________________");
              SEND_SMS(sender, location_links);
            }
          }
        }
      }
    }
  }
  // Serial.write(UART_SIM800.read());

  web_socket.cleanupClients();

}  //loop
