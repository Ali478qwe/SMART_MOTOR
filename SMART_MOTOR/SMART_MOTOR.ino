#include <TinyGPS++.h>
#include <TinyGPSPlus.h>
#include <HardwareSerial.h>
#include <WiFi.h>


const char* ssid = "َc++";
const char* password = "12345678";

//UART PIN FOR GPS CONNECTION
static const uint8_t RX_PIN_tTX_GPS = 4 , TX_PIN_tRX_GPS = 5;

//UART PIN FOR SIM800 CONNECTION
static const uint8_t RX_PIN_tTX_SIM800 = 16 , TX_PIN_tRX_SIM800 = 17;

//GPS CLASS FOR DATA ANALYSIS
TinyGPSPlus gps; 

//DEFINE UART NAME PORT FOR GSM
HardwareSerial UART_SIM800 (1);

//DEFINE UART NAME PORT FOR GPS
HardwareSerial UART_GPS (2);


String status, sender, dateTime , text;

String SMS_NUM(String text)
{
  int token = text.indexOf(",");
  if(token != -1){
      return text.substring(token + 1);
  }
}

void SMS_PARSER(String massage, String &status, String &sender, String &dateTime , String &text) {
  int firstQuote = massage.indexOf('"');
  int secondQuote =  massage.indexOf('"', firstQuote + 1);
  status =  massage.substring(firstQuote + 1, secondQuote);

  int thirdQuote =  massage.indexOf('"', secondQuote + 1);
  int fourthQuote =  massage.indexOf('"', thirdQuote + 1);
  sender =  massage.substring(thirdQuote + 1, fourthQuote);

  int fifthQuote =  massage.indexOf('"', fourthQuote + 4);
  int sixthQuote =  massage.indexOf('"', fifthQuote + 1);
  dateTime =  massage.substring(fifthQuote + 1, sixthQuote);

  text = massage.substring(sixthQuote + 1);

}

void SEND_SMS(String number, String text) {
  UART_SIM800.println("AT+CMGS=\"" + number + "\"");
  unsigned long start = millis();

  while (!UART_SIM800.available() && millis() - start < 3000) { }
  delay(100);
  UART_SIM800.println(text);
  delay(500);
  UART_SIM800.write(26); // CTRL+Z
}

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

String Verification(String text , String Key_Word , String Password){

  text.trim();

  int first_index = text.indexOf(Key_Word);
  int second_index = text.indexOf(":");
  
  if(first_index != -1 && second_index != -1)
  {

    String User_Key_Word = text.substring(first_index , second_index);
    String User_Password = text.substring(second_index + 1,second_index + Password.length() + 1);
    // User_Password.trim();
    Serial.println(String("User_Key_Word:" + User_Key_Word));
    Serial.println(String("User_Password:" + User_Password));

    User_Key_Word.trim();
    User_Password.trim();

    if(User_Key_Word == Key_Word)
    {
      if(User_Password == Password)
      {
        return "VERIFIED";
      }
      else
      {
        return "F_PASSWORD";
      }
    }
      else
      {   
          return "F_KEY_WORD";
      }
  }

  return "FORMAT_ERR";
  
}

void setup() 
{

 //ACTIVATE SERIAL MONITRO
 Serial.begin(115200);

 //ACTIVATE GPS UART
 //TIP : Baud rate = 9600
 
  WiFi.softAP(ssid, password);
  UART_GPS.begin(9600,SERIAL_8N1,RX_PIN_tTX_GPS,TX_PIN_tRX_GPS);
  Serial.println("UART GPS Started");
  //ACTIVATE GPS UART
  //TIP : Baud rate = 9600
  UART_SIM800.begin(9600,SERIAL_8N1,RX_PIN_tTX_SIM800,TX_PIN_tRX_SIM800);
  Serial.println("UART SIM800 Started");

 


}

void loop() 
{
   if (UART_SIM800.available())
  {
    String response = UART_SIM800.readString();
    response = cleanResponse(response);
    Serial.println(response);   

    if(response.indexOf("+CMTI:") != -1){

      String sms_num = SMS_NUM(response);
      UART_SIM800.println(String("AT+CMGR=" + sms_num));
      Serial.println(String("AT+CMGR=" + sms_num + "serial"));
      

      String RESPONSE_CMGR = UART_SIM800.readString();
    
      Serial.println(RESPONSE_CMGR);
      if(RESPONSE_CMGR.indexOf("+CMGR:") != -1)
      {
        
        SMS_PARSER(RESPONSE_CMGR, status, sender, dateTime , text);
        Serial.println(status);
        Serial.println(sender);
        Serial.println(dateTime);
        Serial.println(text);//BODY_CODE(RESPONSE_CMGR)
        if(sender == "+989024381736")
        {
            //verification text : location:1234
            String verify = Verification(text,"location","Ali478qwe");
              Serial.println(verify);
            if(verify == "VERIFIED")
            {
                if(UART_GPS.available())
                { 

                  // char c = UART_GPS.read();
                  // Serial.write(c);

                  //ENCODE UART DATA FROM GPS
                  gps.encode(UART_GPS.read());
                  
                  if(Serial.available())
                  {

                    
                    if(Serial.read() == '1')
                    {
                      
                      Serial.println("Serial Command Recived");

                      if(gps.location.isValid())
                      { 
                        Serial.println("Satellite Found");
                        //GPS VALUE
                        Serial.println(String("Satellite Value: " + String(gps.satellites.value()) ));

                        if(gps.location.isUpdated())
                        {
                            //PRINT DAtA
                            Serial.println("GEOGRAPHICAL LOCATION");
                            Serial.println("Geo width:");
                            Serial.println(gps.location.lat(),6);
                            Serial.println("Geo Length:");
                            Serial.println(gps.location.lng(),6);

                            //LOCATION LINKS
                           
                            String location_links = String("https://google.com/maps?p=" + String(gps.location.lat(),6) + "," +  String(gps.location.lng(),6)) + "\n" +           
                                                    String("https://neshan.org/maps?lat=" + String(gps.location.lat(),6) + "&lng=" + String(gps.location.lng(),6)) + "\n" +     
                                                    String("https://balad.ir/location/" + String(gps.location.lat(),6) + "," +  String(gps.location.lng(),6));
                            Serial.println(location_links);
                            Serial.println("_________________");
                            SEND_SMS(sender,location_links);
                        }
                      }
                      else
                      {
                        Serial.println("Satellite Not Found");
                      }//IS_VALID
                    }
                  }
                }//GPS AVAILABLE
              
            }
        }
      }
      

    }
    // Serial.write(UART_SIM800.read());
  }
 
}//loop
