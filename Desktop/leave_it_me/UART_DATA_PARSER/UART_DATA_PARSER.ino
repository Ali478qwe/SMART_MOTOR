String text =  "+CMGR: \"REC UNREAD\",\"+989024381736\",\"\",\"25/11/13,19:25:18+14\"";

String status , sender , dateTime;






void setup() {
  int firstQuote = text.indexOf('"');
  int secondQuote =  text.indexOf('"', firstQuote + 1);
  status =  text.substring(firstQuote + 1, secondQuote);

  int thirdQuote =  text.indexOf('"', secondQuote + 1);
  int fourthQuote =  text.indexOf('"', thirdQuote + 1);
  sender =  text.substring(thirdQuote + 1, fourthQuote);

  int fifthQuote =  text.indexOf('"', fourthQuote + 4);
  int sixthQuote =  text.indexOf('"', fifthQuote + 1);
  dateTime =  text.substring(fifthQuote + 1, sixthQuote);
  Serial.begin(115200);
  Serial.println("LENGTH TEXT :" + String(text.length()));
  Serial.println("firstQuote :" + String(firstQuote));
  Serial.println("secondQuote :" + String(secondQuote));
  Serial.println("status :" + String(status));

  Serial.println("thirdQuote :" + String(thirdQuote));
  Serial.println("fourthQuote :" + String(fourthQuote));
  Serial.println("sender :" + String(sender));

  Serial.println("fifthQuote :" + String(fifthQuote));
  Serial.println("sixthQuote :" + String(sixthQuote));
  Serial.println("dateTime :" + String(dateTime));




 
}

void loop() {


}
