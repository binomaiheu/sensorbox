#include "sds011.h"

sds011::sds011( uint8_t rxPin, uint8_t txPin )
 : m_serial( rxPin, txPin, false )
 , m_starttime( millis() ) {

    // configure pins
    pinMode( rxPin, INPUT );
    pinMode( txPin, OUTPUT );

    // init software serial port
    m_serial.begin(9600);
    m_serial.listen();

    // power cycle

    set_report_mode( SDS011_REPORT_QUERY );
    set_state( sds011::SDS011_WORK );    
 }

 sds011::~sds011()
 {


 }

// send a command to the sensor
// See the docs for spects, here we report 
size_t sds011::send_cmd( const uint8_t *data, size_t len ) {
    
    uint8_t buffer[19];
    uint16_t checksum = 0; 

    // max 11 databytes, the rest should be the selected device id's which we will keep to 0xFF anyway (all)
    if ( len > 11 ) len = 11;           

    memset(buffer, 0, 19 );   

    buffer[0] = 0xAA;
    buffer[1] = 0xB4;
    for ( uint8_t i = 0; i < len; i++ ) {
        buffer[i+2] = data[i];
        checksum += data[i];
    }

    buffer[15] = 0xFF; checksum += 0xFF;
    buffer[16] = 0xFF; checksum += 0xFF;
    buffer[17] = checksum % 256;
    buffer[18] = 0xAB;

    //Serial.print( "Writing :" );
    //for ( uint8_t i = 0; i < 19; i ++ ) Serial.print( " " + String(buffer[i], HEX) );
    //Serial.println(".");

    uint8_t n = m_serial.write( buffer, 19 );
    m_serial.flush();    

    return n;
}


// mode
//  0xC0 : read data mode, data byte1 already is PM concentration part, command id is C0
//  for the modes below, the command id is C5 and the mode should match the 3th byte
//  0x02 : reply to set data reporting mode
//  0x05 : reply to set device id
//  0x06 : reply to set sleep and work
//  0x07 : reply to check firmware version
//  0x08 : reply to set working period

uint8_t sds011::read( uint8_t mode, uint8_t* buffer, uint8_t len ) {

    uint8_t value = 0, retval = 0;
    uint8_t n = 0, curr_len = 0;
    uint16_t checksum = 0x0;
    bool checksum_ok = false;

    // clear the buffer
    memset( buffer, 0, len );

    // read the result
    while ( m_serial.available() > 0 )
    {
        value = m_serial.read();                
        switch(curr_len) {
            case 0:
                if ( value != 0xAA ) { curr_len = -1; n = 0; checksum = 0x0; }
                break;
            case 1:
                if ( mode != 0xC0 ) {
                    if ( value != 0xC5 ) { curr_len = -1; n = 0; checksum = 0x0; }
                } else {
                    if ( value != 0xC0 ) { curr_len = -1; n = 0; checksum = 0x0; }
                }
                break;
            case 2: 
                if ( mode != 0xC0 ) {
                    if ( value != mode ) { curr_len = -1; n = 0; checksum = 0x0; } // data head doesn"t match requested mode
                } 
                // start reading pm
                buffer[n++] = value;
                checksum = value;
                break;
            case 3:
                buffer[n++] = value;
                break;     
            case 4:
                buffer[n++] = value;
                break;     
            case 5:
                buffer[n++] = value;
                break;     
            case 6:
                buffer[n++] = value;
                break;     
            case 7:
                buffer[n++] = value;
                break;            
            case 8:
                if ( value == ( checksum % 256 ) ) {
                    checksum_ok = true;
                } else {
                    Serial.println( F("checksum error in read()"));
                    curr_len = -1;
                    checksum = 0x0;
                    n = 0;
                }
                break;
            case 9:
                if (value != 0xAB ) { curr_len = -1; checksum = 0x0; n = 0; }
                break;
        } // switch

        // add data byte to the checksum
        if (curr_len > 2 ) checksum += value;
        
        // increment the data byte
        curr_len++;

        // when we have read 10 bytes (last one should be 0xab (not checked...))
        // break & return the n (along with the buffer)
        if ( ( curr_len == 10 ) && checksum_ok ) {
            curr_len = 0;
            checksum_ok = false;
            retval = n;         

            //Serial.print("Read data:" );
            //for ( uint8_t i = 0; i < n ; i++ ) { Serial.print(" "); Serial.print(buffer[i], HEX); }
            //Serial.println(".");  
        }

    }
    
    return retval;
}

/*
void sds011::get_info( char* msg )
{    
    
    uint8_t buffer[6]; 
    set_state( sds011::SDS011_WORK );    
    delay(100);

    uint8_t send_buf[1] = {0x07};    
    send_cmd( send_buf, 1 );
    delay(500);
    uint8_t n = read( 0x07, buffer, 6 );

    if ( n < 6 ) {        
        sprintf( msg, "unknown" );
    } else {        
        sprintf( msg, "%04d-%02d-%02d, chipId: 0x%04X", 2000+buffer[1], buffer[2], buffer[3], (buffer[4]<<8)+buffer[5] );
    }
    
    return;
}
*/

void sds011::set_report_mode( sds011::report_mode mode )
{
    uint8_t buf[3] = { 0x02, 0x01, mode };
    send_cmd( buf, 3 );
    while( m_serial.available()>0) m_serial.read();

    return;
}

void sds011::set_state( sds011::work_state mode )
{
    uint8_t buf[3] = { 0x06, 0x01, mode };    
    send_cmd( buf, 3 );
    while( m_serial.available()>0) m_serial.read();

    return;
}

void sds011::query_data( uint8_t* buffer, uint8_t len )
{
    uint8_t buf[1] = {0x04};    
    send_cmd( buf, 1 );    
    
    read( 0xC0, buffer, 6 );
    return;
}

void sds011::get_values( float& pm10, float& pm25 )
{
    int pm10_serial;
    int pm25_serial;

    pm10 = -9999.;
    pm25 = -9999.;

    uint8_t buffer[6];
    query_data( buffer, 6 );   

    pm25_serial  = buffer[0];
    pm25_serial += ( buffer[1] << 8 );
    pm10_serial  = buffer[2];
    pm10_serial += ( buffer[3] << 8 );

    pm10 = float(pm10_serial)/10.;
    pm25 = float(pm25_serial)/10.;
}