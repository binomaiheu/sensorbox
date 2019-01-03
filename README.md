# sensorbox
Code for my own gardenshed sensorbox for arduino uno in combination with an ethernet shield. 

![foto](https://raw.githubusercontent.com/binomaiheu/sensorbox/master/docs/IMG_20181228_175019.jpg | width=300)


Currently it includes :
- a Sensirion SHT15 temperature and humidity sensor
- a Nova PM sensor for PM10 and PM2.5 measurements

A large part of the SDS011 readout code started from the great opendata-stuttgart project, see : https://github.com/opendata-stuttgart/sensors-software,but i made some of the SDS011 sensor communication routine a bit more generic to be able to explore other functionalities of the SDS011 sensor, such as the query reporting mode instead of continuous. Since i don't have wifi at my garden shed, I'm too lazy to run to the shop to by an access point but did think of running a fixed network cable to the shed when i was building it... this code works for arduino and no longer for the ESP6288 orginally used by the luftdaten project. 

I'm using platformio for development and building (https://platformio.org/), the platformio.ini file is included in the repository. 

In the local lib folder i've included 
* the cactus_io_SHT15 library (http://cactus.io/hookups/sensors/temperature-humidity/sht15/hookup-arduino-to-sensirion-sht15-temp-humidity-sensor) for SHT15 readout. 
* As well as my own attempt of creating a wrapper arount the sds011 readout. 
* And some helper routines

The public thingspeak channel is : https://thingspeak.com/channels/662834

Notes: 
* The sketch includes an SD card writer as well as a lightweight NTP Client, had to do some serious sketch size optimization to be able to run both : 
    * String literals are encapsulated in teh F() macro
    * I'm not making use of the official ThingSpeak library as it gave very poor behaviour for writing multiple fields as once. Hence, the code
builds the url itself to post the values and directly writes it using the EthernetClient library. That is much more lightweight and works a lot better.
    * Using a lightweight NTP polling routine instead of the procedure in the NTP example
* Removed as much of the String dependencies as conveniently possible since there were some really strange things happening. With replacing the Strings by low level character arrays, things are a lot more stable & seem to works nicely. 

Enjoy !