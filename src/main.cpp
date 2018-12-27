#include <Arduino.h>
#include <Ethernet.h>
#include <SPI.h>
#include <ThingSpeak.h>

// Include project libraries in the lib folder
#include <strfun.h>

// Inclde project headers
#include "sensors.h"

String sds_info = "";
float pm10, pm25;


// ThingSpeak
unsigned long tsChannelNum = 662834;
const char*   tsAPIKey     = "60LARLDF6SNAEDV0";

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {
  0x90, 0xA2, 0xDA, 0x0A, 0x00, 0xF3
};

// Ethernet
EthernetClient client;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(1000);
  Serial.println( "Booting sensorbox..." ); 
  sds_info = sds.get_info();
  Serial.println( sds_info );

  // start the Ethernet connection:
  Serial.println("Init Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");   
    // no point in carrying on, so do nothing forevermore:
    while (true) {
      delay(1);
    }
  } else {
    Serial.print("  DHCP assigned IP ");
    Serial.println(Ethernet.localIP());
  }

  Serial.println("Initializing THingspeak API...");
  ThingSpeak.begin( client );  
}

void loop() {

  // check DHCP 
  switch (Ethernet.maintain()) {
    case 1:
      //renewed fail
      Serial.println("Error: renewed fail");
      break;

    case 2:
      //renewed success
      Serial.println("Renewed success");
      //print your local IP address:
      Serial.print("My IP address: ");
      Serial.println(Ethernet.localIP());
      break;

    case 3:
      //rebind fail
      Serial.println("Error: rebind fail");
      break;

    case 4:
      //rebind success
      Serial.println("Rebind success");
      //print your local IP address:
      Serial.print("My IP address: ");
      Serial.println(Ethernet.localIP());
      break;

    default:
      //nothing happened
      break;
  }

  // put your main code here, to run repeatedly:
  sht.readSensor();
  float t = sht.getTemperature_C();
  float rh = sht.getHumidity();
  Serial.println( String("Temp: ") + Float2String(t) + String(", Humidity: ") + Float2String(rh) );


  sds.get_values( pm10, pm25 );
  Serial.println( String("PM25: ") + Float2String(pm25));
  Serial.println( String("PM10: ") + Float2String(pm10));

  // ThinkSpeak, convert the fields to string, otherwise somethings
  // seems not to work with seting more than 3 floats. When setting to string it does seem 
  // to work and the ThingSpeak will still interpret the values correctly
  ThingSpeak.setField(1, String(pm25));
  ThingSpeak.setField(2, String(pm10));
  //ThingSpeak.setField(3, String(t));
  //ThingSpeak.setField(4, String(rh));

 // write to the ThingSpeak channel 
  int x = ThingSpeak.writeFields(tsChannelNum, tsAPIKey);
  if ( x == 200 ) {
    Serial.println("OK");
  } else {
    Serial.println("HTTP Error:" + String(x));
  }

  // thingspeak only allows updates everey 15 sec
  // so do this every 30
  delay(30000);
}