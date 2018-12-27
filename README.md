# sensorbox
Code for my own gardenshed sensorbox for arduino uno in combination with an ethernet shield. 

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


Enjoy !