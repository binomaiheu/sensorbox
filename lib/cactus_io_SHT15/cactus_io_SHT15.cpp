/*
 This is a library for the Sensiron SHT15 Humidity Pressure & Temp Sensor
 
 For detials on hooking up the sensor are here
 ----> http://cactus.io/hookups/sensors/temperature-humidity/sht15/hookup-arduino-to-sensirion-SHT15-temp-humidity-sensor
 
 MIT license
 written by cactus.io with inspiration from the http://bildr.org/2012/11/sht15-arduino/ code.

 */

#include "cactus_io_SHT15.h"

SHT15::SHT15(uint8_t dpin, uint8_t cpin) {
    dataPin = dpin;
    clockPin = cpin;
}

void SHT15::readSensor(void) {
    humidity = get_Humidity(dataPin, clockPin);
    temperature_C = get_Temperature(dataPin, clockPin);
}

float SHT15::getHumidity(void) {
    return humidity;
}

float SHT15::getTemperature_C(void) {
    return temperature_C;
}

float SHT15::getTemperature_F(void) {
    temperature_F = temperature_C * 1.8 + 32;
    return temperature_F;
}

float SHT15::getDewPoint(void) {
    float k;
    k = log(humidity/100) + (17.62 * temperature_C) / (243.12 + temperature_C);
    return 243.12 * k / (17.62 - k);
}

float SHT15::get_Humidity(int dPin, int cPin){
    //Return  Relative Humidity
    sht_SendCommand(B00000101, dPin, cPin);
    sht_WaitForResult(dataPin);
    int val = sht_GetData(dPin, cPin);
    sht_SkipCrc(dPin, cPin);
    return -4.0 + 0.0405 * val + -0.0000028 * val * val;
}

float SHT15::get_Temperature(int dPin, int cPin){
    //Return Temperature in Celsius
    sht_SendCommand(B00000011, dPin, cPin);
    sht_WaitForResult(dataPin);
    
    int val = sht_GetData(dPin, cPin);
    sht_SkipCrc(dPin, cPin);
    return (float)val * 0.01 - 40; //convert to celsius
}


void SHT15::sht_SendCommand(int command, int dPin, int cPin){
    // send a command to the SHTx sensor
    // transmission start
    pinMode(dPin, OUTPUT);
    pinMode(cPin, OUTPUT);
    digitalWrite(dPin, HIGH);
    digitalWrite(cPin, HIGH);
    digitalWrite(dPin, LOW);
    digitalWrite(cPin, LOW);
    digitalWrite(cPin, HIGH);
    digitalWrite(dPin, HIGH);
    digitalWrite(cPin, LOW);
    
    // shift out the command (the 3 MSB are address and must be 000, the last 5 bits are the command)
    shiftOut(dataPin, clockPin, MSBFIRST, command);
    
    // verify we get the right ACK
    digitalWrite(clockPin, HIGH);
    pinMode(dataPin, INPUT);
    
    // if (digitalRead(dataPin))
    digitalWrite(clockPin, LOW);
    // if (!digitalRead(dataPin)) Serial.println("ACK error 1");
}


void SHT15::sht_WaitForResult(int dataPin){
    // wait for the SHTx answer
    pinMode(dataPin, INPUT);
    
    int ack; //acknowledgement
    
    //need to wait up to 2 seconds for the value
    for (int i = 0; i < 1000; ++i){
        delay(2);
        ack = digitalRead(dataPin);
        if (ack == LOW) break;
    }
    
    // if (ack == HIGH) Serial.println("ACK error 2");
}

int SHT15::sht_GetData(int dPin, int cPin){
    // get data from the SHTx sensor
    
    // get the MSB (most significant bits)
    pinMode(dPin, INPUT);
    pinMode(cPin, OUTPUT);
    byte MSB = shiftIn(dPin, cPin, MSBFIRST);
    
    // send the required ACK
    pinMode(dPin, OUTPUT);
    digitalWrite(dPin, HIGH);
    digitalWrite(dPin, LOW);
    digitalWrite(cPin, HIGH);
    digitalWrite(cPin, LOW);
    
    // get the LSB (less significant bits)
    pinMode(dPin, INPUT);
    byte LSB = shiftIn(dPin, cPin, MSBFIRST);
    return ((MSB << 8) | LSB); //combine bits
}

void SHT15::sht_SkipCrc(int dPin, int cPin){
    // skip CRC data from the SHTx sensor
    pinMode(dPin, OUTPUT);
    pinMode(cPin, OUTPUT);
    digitalWrite(dPin, HIGH);
    digitalWrite(cPin, HIGH);
    digitalWrite(cPin, LOW);
}

