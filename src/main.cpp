#include <Arduino.h>
#include <Ethernet.h>
#include <SPI.h>

#include "memcheck.h"

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

int getHTTPResponse(String & response)
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

	if(!client.find(const_cast<char *>("\r\n"))) return ERR_BAD_RESPONSE;
	if(!client.find(const_cast<char *>("\n\r\n"))) return ERR_BAD_RESPONSE;
		
	String tempString = client.readString();
	response = tempString;

	return status;
};

int abortWriteRaw()
{
  client.stop();
  return ERR_UNEXPECTED_FAIL;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(1000);
  Serial.println( "Booting sensorbox..." ); 
  sds_info = sds.get_info();
  Serial.println( sds_info );
  delay(2000);
  sds.get_values( pm10, pm25 );
  Serial.println( String("PM25: ") + Float2String(pm25));
  Serial.println( String("PM10: ") + Float2String(pm10));

  Serial.println( "Freemem: " + String(freeMemory()));

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
  Serial.println( "Freemem: " + String(freeMemory()));

  // put your main code here, to run repeatedly:
  sht.readSensor();
  float t = sht.getTemperature_C();
  float rh = sht.getHumidity();
  Serial.println( String("Temp: ") + Float2String(t) + String(", Humidity: ") + Float2String(rh) );


  sds.get_values( pm10, pm25 );
  Serial.println( String("PM25: ") + Float2String(pm25));
  Serial.println( String("PM10: ") + Float2String(pm10));


  // connect to thingspeak
  if ( client.connect( THINGSPEAK_URL, THINGSPEAK_PORT_NUMBER ) ) {
    String postMessage = String("field1=") + Float2String(pm25) 
                       + String("&field2=") + Float2String(pm10)
                       + String("&field3=") + Float2String(t)
                       + String("&field4=") + Float2String(rh)
                       + String("&headers=false");
    
    Serial.println( "POST:" + postMessage );

    if (!client.print("POST /update HTTP/1.1\r\n")) abortWriteRaw();
		if (!client.print("Host: api.thingspeak.com\r\n")) abortWriteRaw();
		if (!client.print("Connection: close\r\n")) abortWriteRaw();
		if (!client.print("User-Agent: ")) abortWriteRaw();
		if (!client.print(TS_USER_AGENT)) abortWriteRaw();
		if (!client.print("\r\n")) abortWriteRaw();
		if (!client.print("X-THINGSPEAKAPIKEY: ")) abortWriteRaw();
		if (!client.print(tsAPIKey)) abortWriteRaw();
		if (!client.print("\r\n")) abortWriteRaw();
		if (!client.print("Content-Type: application/x-www-form-urlencoded\r\n")) abortWriteRaw();
		if (!client.print("Content-Length: ")) abortWriteRaw();
		if (!client.print(postMessage.length())) abortWriteRaw();
		if (!client.print("\r\n\r\n")) abortWriteRaw();
		if (!client.print(postMessage)) abortWriteRaw();
		
    String entryIDText = String();
		int status = getHTTPResponse(entryIDText);
		if(status != OK_SUCCESS)
		{
			client.stop();
      Serial.println("TS update failed... ");			
		}
		long entryID = entryIDText.toInt();
		Serial.print("Inserted Entry ID \"");Serial.print(entryIDText);Serial.print("\" (");Serial.print(entryID);Serial.println(")");
		client.stop();
  }


  // thingspeak only allows updates everey 15 sec
  // so do this every 30
  delay(30000);
}