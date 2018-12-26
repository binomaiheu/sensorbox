/* SHT15 library
 MIT license
 written by cactus.io with inspiration from the http://bildr.org/2012/11/sht15-arduino/
 */

#ifndef DHT_H
#define DHT_H
#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

class SHT15 {

private:
    uint8_t data[6];
    uint8_t dataPin, clockPin;
    
    float humidity;
    float temperature_C;
    float temperature_F;
    
    float get_Humidity(int dPin, int cPin);
    float get_Temperature(int dPin, int cPin);

    void sht_SendCommand(int,int,int);
    int sht_GetData(int, int);
    void sht_WaitForResult(int);
    void sht_SkipCrc(int, int);
    
public:
    SHT15(uint8_t datapin, uint8_t clockpin);
    void readSensor(void);
    
    float getHumidity(void);
    float getTemperature_C(void);
    float getTemperature_F(void);
    float getDewPoint(void);
    
};
#endif
