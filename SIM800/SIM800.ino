
#include <HardwareSerial.h>
// #include <string.h>

//UART PIN FOR SIM800 CONNECTION
static const int8_t RX_PIN_tTX_SIM800 = 16, TX_PIN_tRX_SIM800 = 17;

//GPS CLASS FOR DATA ANALYSIS


//DEFINE UART NAME PORT FOR GSM
HardwareSerial UART_SIM800(2);

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

// String BODY_CODE(String text){
//   int first = text.indexOf('\n');
//   int second = text.indexOf('\n', first + 1);
//   int third = text.indexOf('\n', second + 1);
//   int fourth = text.indexOf('\n' + third + 1);
//      //second = text.length();
//   if(first == -1) Serial.println(text.substring(first +1,second));

//   if(second == -1) Serial.println(text.substring(second +1, third));

//    if(third == -1) Serial.println(text.substring(third +1, fourth));

//     if(fourth == -1) Serial.println(text.substring(fourth + 1));
  


//   return text.substring( third + 1);
// }

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

void setup() {

  //ACTIVATE SERIAL MONITRO
  Serial.begin(115200);

  //ACTIVATE GPS UART
  //TIP : Baud rate = 9600
  UART_SIM800.begin(9600, SERIAL_8N1, RX_PIN_tTX_SIM800, TX_PIN_tRX_SIM800);
  Serial.println("UART SIM800 Started");
  delay(1000);
  UART_SIM800.println("ATE0");
  delay(500);
  // Serial.print("AT: ");
  UART_SIM800.println("AT");
  delay(500);
  // Serial.print("ATI: ");
  UART_SIM800.println("ATI");
  delay(500);
  // Serial.print("AT+CSQ: ");
  UART_SIM800.println("AT+CSQ");
  delay(500);
  // Serial.print("AT+CCID: ");
  UART_SIM800.println("AT+CCID");
  delay(500);
  UART_SIM800.println("AT+CPIN?");
  delay(500);
  // Serial.print("AT+CREG: ");
  UART_SIM800.println("AT+CREG?");
  delay(500);
  // Serial.print("AT+CBC: ");
  UART_SIM800.println("AT+CBC");
  delay(500);
  // Serial.print("AT+CMGF: ");
  UART_SIM800.println("AT+CMGF=1");
  delay(500);
  UART_SIM800.println("AT+CFUN?");
  delay(500);
  UART_SIM800.println("AT+CSCLK?");
  delay(500);


}

void loop() {
   
  if (Serial.available()) {
    UART_SIM800.println(Serial.readStringUntil('\n'));
   
  }

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
              SEND_SMS(sender,"MASSAGE RECIVED");
            }
        }
      }
      

    }
    // Serial.write(UART_SIM800.read());
  }



}  //loop
