/** 
Repeating Web client

 This sketch connects to a Splunk server and makes a POST request to the
 HTTP event collector endpoint. The hardware is an arduino duemilannove
 using a Wiznet Ethernet shield and DHT11 temp sensor. You can use the 
 Arduino Ethernet shield, or the Adafruit Ethernet shield, either one will work, as long as it's got
 a Wiznet Ethernet module on board.

 This example does NOT use DNS and assigns the Ethernet client with a MAC address,
 IP address. In this case, it requires that you hard-code your Splunk Endpoint via
 its IP address


 created 19 Apr 2012
 by Tom Igoe
 modified 21 Jan 2014
 by Federico Vanzati
 modified 19 Oct 2015
 by Kyle Champlin

 For the ethernet and HTTP client code
 http://www.arduino.cc/en/Tutorial/WebClientRepeating
 This code is in the public domain.

 The DHT11 code was lovingly crafted by Ladyada @ Adafruit
 https://www.adafruit.com/products/386
 Written by ladyada, public domain
 
 **/

#include <SPI.h>
#include <Ethernet.h>
#include "DHT.h"

//Sensor Setup
#define DHTPIN 2 
#define DHTTYPE DHT11
//Ethernet Shield Setup
// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
IPAddress server(10,14,0,85);  // numeric IP for Google (no DNS)
//char server[] = "www.google.com";    // name address for Google (using DNS)
//If you want to use DNS for your server
// Set the static IP address for the ethernet shield
// to use if the DHCP fails to assign
IPAddress ip(10,14,0,1);

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
EthernetClient client;
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  //just a startup delay, to let the little guy wake up
  delay(1000);
  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  }
  // give the Ethernet shield a second to initialize:
  Serial.print("My IP address: ");
  // Print out the IP address, in case you need to wireshark the transaction
  Serial.println(Ethernet.localIP());
  
  //init our DHT11 temp sensor
  dht.begin();
}

void loop()
{
  delay(5000);
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
  }

  //quick read of the incoing traffic on the shield...if any
  if (client.available()) {
    char c = client.read();
  }

  //wait a bit more 10000 millis before sending the request to Splunk
  delay(10000);
  sendHttpRequest(h, f);
  


}

void sendHttpRequest(float h, float f){
  //close any current connections
  client.stop();
  //set up our payload as a string
  String stringF = String(f, 3);
  String stringH = String(h, 3);
  String payload = "{ \"host\" : \"arduino\", \"sourcetype\" : \"arduino\", \"index\" : \"arduino\", \"event\" :  {\"temp\" : \"" + stringF + "\" , \"humidity\": \"" + stringH + "\" }}";

  // if you get a connection, report back via serial
  //also set client port to 8088 default for HTTP event collector
  if (client.connect(server, 8088)) {
    Serial.println("connected");
    // Make an HTTP POST request to our event collector endpoint
    client.println("POST /services/collector HTTP/1.1");
    // add our authorization header
    // add your key below after "Splunk"
    client.println("Authorization: Splunk 2B6461AC-FE4E-4EE6-80B0-11E38DF58C2D");
    //send our JSON payload
    //uncomment below if you care about the content-type headers...I do not
    //client.println("Content-Type: application/x-www-form-urlencoded;");
    //Content-Length header is ABSOLUTELY required, otherwise Splunk doesnt know
    //how long to keep the connection open
    client.print("Content-Length: ");
    client.println(payload.length());
    //required to add a space to dilenate our payload from the header info
    client.println();
    client.println(payload);
    client.println();
  }
  else {
    // if you didn't get a connection to the server:
    Serial.println("connection failed");
  }

}



