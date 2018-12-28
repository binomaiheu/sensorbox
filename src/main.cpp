#include <Arduino.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <SPI.h>
#include <SD.h>

// Include project libraries in the lib folder
#include <strfun.h>

// Inclde project headers
#include "sensors.h"
//#include "memcheck.h"

//const char version_info[] = "v0.2";
//#define DEBUG_OUTPUT

#define INTERVAL_READOUT  5000
#define INTERVAL_SUBMIT  60000
unsigned long curr_time    = millis();
unsigned long last_update  = 0;
unsigned long last_readout = 0;
bool          send_data    = false;

#define timeSince( t ) ( curr_time - (t) )

// put the variable inside a structure
float pm10, pm25;
typedef struct {
  char  avg_str[10];
  float avg;
  unsigned int n;
} var_t; 
var_t pm10_agg, pm25_agg, t_agg, rh_agg;

void aggregate_var( var_t& x, float value, float valid_min , float valid_max ) {
  if ( ( value >= valid_min ) && ( value <= valid_max ) ) {
    x.avg += (value - x.avg)/x.n;
    x.n++;
    dtostrf( x.avg, 6, 2, x.avg_str ); // negative width for left adjustment
    trim( x.avg_str, NULL );
  }
}

void reset_var( var_t& x ) {
  x.n = 1;
  x.avg = 0;
  strcpy( x.avg_str, "" );
}

// Ethernet
EthernetClient client;
EthernetUDP    udp;
byte mac[] = { 0x90, 0xA2, 0xDA, 0x0A, 0x00, 0xF3 };

// ThingSpeak
#define TS_VER "1.5"
#define THINGSPEAK_URL "api.thingspeak.com"
#define THINGSPEAK_PORT_NUMBER 80
#define TS_USER_AGENT "tslib-arduino/" TS_VER " (arduino uno or mega)"
#define TIMEOUT_MS_SERVERRESPONSE 5000  // Wait up to five seconds for server to respond

#define OK_SUCCESS              200     // OK / Success
#define ERR_BADAPIKEY           400     // Incorrect API key (or invalid ThingSpeak server address)
#define ERR_BADURL              404     // Incorrect API key (or invalid ThingSpeak server address)
#define ERR_OUT_OF_RANGE        -101    // Value is out of range or string is too long (> 255 bytes)
#define ERR_INVALID_FIELD_NUM   -201    // Invalid field number specified
#define ERR_SETFIELD_NOT_CALLED -210    // setField() was not called before writeFields()
#define ERR_CONNECT_FAILED      -301    // Failed to connect to ThingSpeak
#define ERR_UNEXPECTED_FAIL     -302    // Unexpected failure during write to ThingSpeak
#define ERR_BAD_RESPONSE        -303    // Unable to parse response
#define ERR_TIMEOUT             -304    // Timeout waiting for server to respond
#define ERR_NOT_INSERTED        -401    // Point was not inserted (most probable cause is the rate limit of once every 15 seconds)

unsigned long tsChannelNum = 662834;
const char*   tsAPIKey     = "60LARLDF6SNAEDV0";

bool updateThingSpeak( const char* msg, uint16_t len ) {

  Serial.print( "POST: ");      
  Serial.println( msg );
  if (!client.print(F("POST /update HTTP/1.1\r\n"))) return false;
  if (!client.print(F("Host: api.thingspeak.com\r\n"))) return false;
 	if (!client.print(F("Connection: close\r\n"))) return false;
  if (!client.print(F("User-Agent: "))) return false;
  if (!client.print(TS_USER_AGENT)) return false;
  if (!client.print(F("\r\n"))) return false;
  if (!client.print(F("X-THINGSPEAKAPIKEY: "))) return false;
  if (!client.print(tsAPIKey)) return false;
  if (!client.print(F("\r\n"))) return false;
  if (!client.print(F("Content-Type: application/x-www-form-urlencoded\r\n"))) return false;
  if (!client.print(F("Content-Length: "))) return false;
  if (!client.print(len)) return false;
  if (!client.print(F("\r\n\r\n"))) return false;
  if (!client.print(msg)) return false;
  
  return true;
}


int getHTTPResponse( char* response, uint16_t buflen )
{
  long startWaitForResponseAt = millis();
  while(client.available() == 0 && millis() - startWaitForResponseAt < TIMEOUT_MS_SERVERRESPONSE)
  {
    delay(100);
  }
  if(client.available() == 0) return ERR_TIMEOUT; // Didn't get server response in time
	if(!client.find(const_cast<char *>("HTTP/1.1"))) return ERR_BAD_RESPONSE; // Couldn't parse response (didn't find HTTP/1.1)

  int status = client.parseInt();
  if(status != OK_SUCCESS) return status;		

	//if(!client.find(const_cast<char *>("\r\n"))) return ERR_BAD_RESPONSE;
	//if(!client.find(const_cast<char *>("\n\r\n"))) return ERR_BAD_RESPONSE;
		
	//String tempString = client.readString();
	//tempString.toCharArray( response, buflen ); 

	return status;
};

/*
 * © Francesco Potortì 2013 - GPLv3 - Revision: 1.13
 *
 * Send an NTP packet and wait for the response, return the Unix time
 *
 * To lower the memory footprint, no buffers are allocated for sending
 * and receiving the NTP packets.  Four bytes of memory are allocated
 * for transmision, the rest is random garbage collected from the data
 * memory segment, and the received packet is read one byte at a time.
 * The Unix time is returned, that is, seconds from 1970-01-01T00:00.
 */
unsigned long inline ntpUnixTime (UDP &udp)
{
  static int udpInited = udp.begin(123); // open socket on arbitrary port

  const char timeServer[] = "pool.ntp.org";  // NTP server

  // Only the first four bytes of an outgoing NTP packet need to be set
  // appropriately, the rest can be whatever.
  const long ntpFirstFourBytes = 0xEC0600E3; // NTP request header

  // Fail if WiFiUdp.begin() could not init a socket
  if (! udpInited)
    return 0;

  // Clear received data from possible stray received packets
  udp.flush();

  // Send an NTP request
  if (! (udp.beginPacket(timeServer, 123) // 123 is the NTP port
	 && udp.write((byte *)&ntpFirstFourBytes, 48) == 48
	 && udp.endPacket()))
    return 0;				// sending request failed

  // Wait for response; check every pollIntv ms up to maxPoll times
  const int pollIntv = 150;		// poll every this many ms
  const byte maxPoll = 15;		// poll up to this many times
  int pktLen;				// received packet length
  for (byte i=0; i<maxPoll; i++) {
    if ((pktLen = udp.parsePacket()) == 48)
      break;
    delay(pollIntv);
  }
  if (pktLen != 48)
    return 0;				// no correct packet received

  // Read and discard the first useless bytes
  // Set useless to 32 for speed; set to 40 for accuracy.
  const byte useless = 40;
  for (byte i = 0; i < useless; ++i)
    udp.read();

  // Read the integer part of sending time
  unsigned long time = udp.read();	// NTP time
  for (byte i = 1; i < 4; i++)
    time = time << 8 | udp.read();

  // Round to the nearest second if we want accuracy
  // The fractionary part is the next byte divided by 256: if it is
  // greater than 500ms we round to the next second; we also account
  // for an assumed network delay of 50ms, and (0.5-0.05)*256=115;
  // additionally, we account for how much we delayed reading the packet
  // since its arrival, which we assume on average to be pollIntv/2.
  time += (udp.read() > 115 - pollIntv/8);

  // Discard the rest of the packet
  udp.flush();

  return time - 2208988800ul;		// convert NTP time to Unix time
}

void setup() {
  // Start hardware terminal
  Serial.begin(9600);
  delay(500);

#ifdef DEBUG_OUTPUT
  Serial.println( F("Booting sensorbox...") ); 
  Serial.print(F("Freemem  : ")); Serial.println(freeMemory());  
  Serial.print(F("Firwmare : ")); Serial.println(version_info);
#endif
    
  // start the Ethernet connection:
  Serial.print(F("DHCP :"));
  if (Ethernet.begin(mac) == 0) {
    Serial.println(F("failed."));   
    // no point in carrying on, so do nothing forevermore:
    while (true) {
      delay(1);
    }
  } else {
    Serial.println(Ethernet.localIP());    
  }

  /*
  Serial.println( F("Initialising sensors..." ) );   
  char msg[40];
  sds.get_info( msg );
  Serial.println( msg );
  */

  reset_var( pm10_agg );
  reset_var( pm25_agg );
  reset_var( t_agg );
  reset_var( rh_agg );

#ifdef DEBUG_OUTPUT  
  Serial.println("Init SD card...");
#endif
  if ( ! SD.begin(4) ) { // chipSelect: pin 4 --> see ethernet board
    Serial.println( F("Card failed...") );
    // doing nothing...
    while(true) {
      delay(1);
    }
  }

  File dataFile = SD.open( "DATA.TXT", FILE_WRITE );
  if ( dataFile ) {
    dataFile.println("\n#restart sensor");
    dataFile.close();
  }

#ifdef DEBUG_OUTPUT
  Serial.println( F("Init timers...") );
#endif
  last_readout = millis();
  last_update  = millis();
  send_data    = false;  
}

void loop() {

  // update timing
  curr_time = millis();
  send_data = timeSince( last_update ) > INTERVAL_SUBMIT;
 
  //wdt_reset();  
  Ethernet.maintain();

  // Sensor readout at INTERVAL_READOUT times
  if ( timeSince(last_readout) > INTERVAL_READOUT || send_data ) {
    float pm10, pm25, t, rh; 

    // timestamp
    unsigned long unixTime = ntpUnixTime( udp );

    // Readout SHT15 sensor
    sht.readSensor();        
    t  = sht.getTemperature_C();
    rh = sht.getHumidity();
    aggregate_var( t_agg, t, -50., 100. );
    aggregate_var( rh_agg, rh, 0., 100. );

    // Readout SDS011 sensor    
    sds.get_values( pm10, pm25 );    
    aggregate_var( pm10_agg, pm10, 0.001, 9999. );
    aggregate_var( pm25_agg, pm25, 0.001, 9999. );
    
#ifdef DEBUG_OUTPUT
      Serial.print(F("[READOUT] timestamp=")); Serial.print(unixTime); 
      Serial.print(F(", T=")); Serial.print(t); 
      Serial.print(F(", RH=")); Serial.print(rh);
      Serial.print(F(", PM10=")); Serial.print(pm10);
      Serial.print(F(", PM25=")); Serial.println(pm25);
#endif

    File dataFile = SD.open( F("DATA.TXT"), FILE_WRITE );
    if ( dataFile ) {
      dataFile.print(unixTime); dataFile.print("\t");
      dataFile.print(pm25); dataFile.print("\t");
      dataFile.print(pm10); dataFile.print("\t");
      dataFile.print(t); dataFile.print("\t");
      dataFile.println(rh); 
      dataFile.close();
      Serial.println("- tic -");
    } else {
      Serial.println(F("Error updating datafile"));
    }

    last_readout = curr_time;
  }

  // Send data to Thingspeak
  if ( send_data ) {

#ifdef DEBUG_OUTPUT    
    Serial.println( F("[UPDATING THINGSPEAK]") );
#endif

    // connect to thingspeak
    if ( client.connect( THINGSPEAK_URL, THINGSPEAK_PORT_NUMBER ) ) {
      char msg_buf[75];
      sprintf( msg_buf, "field1=%s&field2=%s&field3=%s&field4=%s&headers=false", 
        pm25_agg.avg_str, pm10_agg.avg_str, t_agg.avg_str, rh_agg.avg_str );                        
      
      if ( ! updateThingSpeak( msg_buf, strlen( msg_buf ) ) ) {
        Serial.println( F("Error: unable to post message...") ); 
        client.stop();
      } else {
        // get response        
  		  int status = getHTTPResponse( msg_buf, 75 );
  		  if ( status != OK_SUCCESS ) {
  		    Serial.println(F("Error: TS update failed... "));
          client.stop();          
  		  } else {
          // successful update  		    
  		    //Serial.print(F("Inserted Entry ID \""));Serial.print(msg_buf);Serial.println("\"");          
  		    client.stop();
        }
      }
      
    } else {
      Serial.println( F("Error: cannot connect to Thingspeak..." ) );
    }
    
    // resetting variable buffers after update
    reset_var( pm10_agg );
    reset_var( pm25_agg );
    reset_var( t_agg );
    reset_var( rh_agg );

    last_update = curr_time;
  }

}