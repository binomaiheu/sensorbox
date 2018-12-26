/*
    Readout library for sds011 sensor 
    See docs

    Written by: Bino Maiheu
    Heavily inspired by : 

*/
#pragma once

#include <Arduino.h>
#include <SoftwareSerial.h>


class sds011 {
public:
    sds011( uint8_t rxPin, uint8_t txPin );
    ~sds011();

    enum report_mode : uint8_t {
        SDS011_REPORT_ACTIVE = 0x00,
        SDS011_REPORT_QUERY = 0x01        
    };

    enum work_state : uint8_t {
        SDS011_SLEEP = 0x00,
        SDS011_WORK  = 0x01
    };

    String get_info( void );
    void get_values( float& pm10, float& pm25 );

private:
    void set_state( sds011::work_state mode );
    void set_report_mode( sds011::report_mode mode );
    void query_data( uint8_t* buffer, uint8_t len );

private:
    SoftwareSerial m_serial;    // the serial writer 
    unsigned long  m_starttime; // millis() since sds was started

    // low level command send routine                
    //   Set data reporting mode       
    //   0 : 0x02
    //   1 : 0 = query current mode, 1 = set reporting mode
    //   2 : 0 = report active mode, 1 = report query mode
    //
    //   Query data command
    //   0 : 0x04
    //
    //   Set Device ID  --> not supported in this code
    //   0 : 0x05
    //
    //   Set sleep and work
    //   0 : 0x06
    //   1 : 0 = query the current mode, 1 : set the mode
    //   2 : 0 = sleep, 1 = work
    //
    //   Set duty cycle
    //   0 : 0x08
    //   1 : 0 = query the current mode, 1 : set the mode
    //   2 : 0 = continuous mode(default)
    //       n (1-30) : work 30 seconds & sleep n*60-30 seconds
    //
    //   CHeck firmware version
    //   0 : 0x07
    //   
    size_t send_cmd( const uint8_t *data, size_t len );

    // reads back the sensor response        
    uint8_t read( uint8_t mode, uint8_t* buffer, uint8_t len );

};