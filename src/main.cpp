#include <Arduino.h>

#include "strfun.h"
#include "cactus_io_SHT15.h"

#define SHT15_dataPin   2
#define SHT15_clockPin  3

SHT15 sht(SHT15_dataPin, SHT15_clockPin);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(1000);
  Serial.println( "Booting sensorbox..." ); 
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(3000);
  sht.readSensor();
  Serial.println( String("Temp: ") + Float2String(sht.getTemperature_C()) + String(", Humidity: ") + Float2String(sht.getHumidity()) );
}