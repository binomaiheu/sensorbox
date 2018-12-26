#include <Arduino.h>

// Include project libraries in the lib folder
#include <strfun.h>

// Inclde project headers
#include "sensors.h"

float pm10, pm25;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(1000);
  Serial.println( "Booting sensorbox..." ); 
  String info = sds.get_info();
  Serial.println( info );
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(10000);
  sht.readSensor();
  Serial.println( String("Temp: ") + Float2String(sht.getTemperature_C()) + String(", Humidity: ") + Float2String(sht.getHumidity()) );



  sds.get_values( pm10, pm25 );
  Serial.println( String("PM25: ") + Float2String(pm25));
  Serial.println( String("PM10: ") + Float2String(pm10));
}