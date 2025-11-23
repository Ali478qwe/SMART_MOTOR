#include <TinyGPS++.h>
#include <TinyGPSPlus.h>
#include <HardwareSerial.h>


//UART PIN FOR GPS CONNECTION
static const int8_t RX_PIN_tTX_GPS = 16 , TX_PIN_tRX_GPS = 17;

//UART PIN FOR SIM800 CONNECTION
static const int8_t RX_PIN_tTX_SIM800 = 39 , TX_PIN_tRX_SIM800 = 4;

//GPS CLASS FOR DATA ANALYSIS
TinyGPSPlus gps; 

//DEFINE UART NAME PORT FOR GSM
HardwareSerial UART_SIM800 (1);

//DEFINE UART NAME PORT FOR GPS
HardwareSerial UART_GPS (2);




void setup() 
{

 //ACTIVATE SERIAL MONITRO
 Serial.begin(9600);

 //ACTIVATE GPS UART
 //TIP : Baud rate = 9600
 UART_GPS.begin(9600,SERIAL_8N1,RX_PIN_tTX_GPS,TX_PIN_tRX_GPS);
 Serial.println("UART GPS Started");
  //ACTIVATE GPS UART
 //TIP : Baud rate = 9600
 UART_SIM800.begin(9600,SERIAL_8N1,RX_PIN_tTX_SIM800,TX_PIN_tRX_SIM800);
 Serial.println("UART SIM800 Started");

 


}

void loop() 
{
  if(UART_SIM800.available())
  {
    Serial.println("SIM800 READ SERIAL:");
    Serial.write(UART_SIM800.read());
  }

    
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
              //GOOGLE
              Serial.println(String("https://google.com/maps?p=" + String(gps.location.lat(),6) + "," +  String(gps.location.lng(),6)));
              //NESHAN
              Serial.println(String("https://neshan.org/maps?lat=" + String(gps.location.lat(),6) + "&lng=" + String(gps.location.lng(),6)));
              //BALAD
              Serial.println(String("https://balad.ir/location/" + String(gps.location.lat(),6) + "," +  String(gps.location.lng(),6)));

              Serial.println("_________________");
          }
        }
        else
        {
          Serial.println("Satellite Not Found");
        }//IS_VALID
      }
    }
  }//GPS AVAILABLE
 
}//loop
