/**
  LED Candles with accuflicker
  Uses a NodeMCU with control via MQTT to drive 4 LED circuits chock full of realistic flame flickering action
  
  Resources:
    - .ledhacker's blog         - https://ledhacker.blogspot.com/2011/12/led-candle-light-flicker-with-arduino.html
    - Christian Haschek's blog  - https://blog.haschek.at/2019/diy-garden-irrigation-for-less-than-20-bucks.html
 
  Notes:
    - Set your wifi and mqqt details in Credentials.h (Credentials.template.h provided)
    - Change the pin assignments in lin 28-31 as needed

    - To turn on, send a `candlecontrol/candle` topic with a payload of ON
    - To turn off, use a payload of OFF. ;)
    - The board will send a `status/candlecontrol` topic with a payload of ONLINE or OFFLINE
 
  Created by Shane Ballman (shaneballman@gmail.com)
  https://github.com/sacarino
**/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "Credentials.h"

/*
 * Make the changes you need for your setup
 */

// these pins need to match whatever pins you used on the NodeMCU (or whatever)
const int pin1 = D1;
const int pin2 = D2;
const int pin3 = D5;
const int pin4 = D6;

/*
 * NO FURTHER CHANGES
 */

// the start of the flicker (low)
static int flicker_low_min = 200;
static int flicker_low_max = 240;

// the end value of the flicker (high)
static int flicker_high_min = 230;
static int flicker_high_max = 256;

// delay between each low-high-low cycle
// low->high |flicker_hold| high->low
static int flicker_hold_min = 40; // milliseconds
static int flicker_hold_max = 80; // milliseconds

// delay after each low-high-low cycle
// low->high->low |flicker_pause| low->high...
static int flicker_pause_min = 100; // milliseconds
static int flicker_pause_max = 200;  // milliseconds

// delay low to high and high to low cycle
static int flicker_speed_min = 900; // microseconds
static int flicker_speed_max = 1000; // microseconds

bool lightCandles = false;

WiFiClient espClient;
PubSubClient client(espClient);

int status = WL_IDLE_STATUS;

void setup() {

  Serial.begin(115200);

  pinMode(pin1, OUTPUT);
  pinMode(pin2, OUTPUT);
  pinMode(pin3, OUTPUT);
  pinMode(pin4, OUTPUT);

  // if some of these pins boot high we can explicitly set it low now.
  // killLEDs();
  
  reconnectWifi();

  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
}

int flicker_random_low_start = 0;
int flicker_random_low_end = 0;
int flicker_random_high = 0;
int flicker_random_speed_start = 0;
int flicker_random_speed_end = 0;

void callback(char* topic, byte* payload, unsigned int length) {

  char message_buff[100];
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);

  Serial.println("Message:");
  int i = 0;
  for (i = 0; i < length; i++) {
    message_buff[i] = (char)payload[i];
  }
  message_buff[i] = '\0';

  String msgString = String(message_buff);
  Serial.println(msgString);

  if (strcmp(topic, "candlecontrol/candle") == 0)
  {
    if (msgString == "OFF") {
      lightCandles = false;
      client.publish("status/candle", "OFF", true);
    }
    else if (msgString == "ON") {
      lightCandles = true;
      client.publish("status/candle", "ON", true);
    }
  }
  Serial.println("-----------------------");
  
}

void mqttreconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Connecting to server ...");
    if ( client.connect("candles", NULL, NULL, "status/candlecontrol", 1, 1, "OFFLINE") ) {
      Serial.println( "[DONE]" );
      client.publish("status/candlecontrol", "ONLINE", true);
      client.subscribe("candlecontrol/#");
    } else {
      Serial.print( "[FAILED] [ rc = " );
      Serial.print( client.state() );
      Serial.println( " : retrying in 5 seconds]" );
      // Wait 5 seconds before retrying
      delay( 5000 );
    }
  }
}

void reconnectWifi()
{
  if ( WiFi.status() != WL_CONNECTED) {
    WiFi.begin(wifi_ssid, wifi_password);
    int count = 0;

    while (WiFi.status() != WL_CONNECTED) {
      count++;
      delay(500);
      Serial.println("Connecting to WiFi..");
      delay(500);
      if (count > 50) reconnectWifi();
    }
  }
}

void killLEDs()
{
  analogWrite(pin1, 0);
  analogWrite(pin2, 0);
  analogWrite(pin3, 0);
  analogWrite(pin4, 0);
}

void flickerLEDs()
{ 
  // picking a new intensity value for each of the pins
  setFlicker(pin1);
  setFlicker(pin2);
  setFlicker(pin3);
  setFlicker(pin4);
  
  // pause
  delay(random(flicker_pause_min, flicker_pause_max));  
}

void setFlicker(int pin) 
{
// random time for low
  flicker_random_low_start = random(flicker_low_min, flicker_low_max);
  flicker_random_low_end = random(flicker_low_min, flicker_low_max);
  
  // random time for high
  flicker_random_high = random(flicker_high_min, flicker_high_max);
  
  // random time for speed
  flicker_random_speed_start = random(flicker_speed_min, flicker_speed_max);
  flicker_random_speed_end = random(flicker_speed_min, flicker_speed_max);
  
  // low -> high
  for (int i = flicker_random_low_start; i<flicker_random_high; i++) {
    analogWrite(pin, i);
    delayMicroseconds(flicker_random_speed_start);
  }
  
  // hold
  delay(random(flicker_hold_min, flicker_hold_max));
  
  // high -> low
  for (int i = flicker_random_high; i>=flicker_random_low_end; i--) {
    analogWrite(pin, i);
    delayMicroseconds(flicker_random_speed_end);
  }
}

void loop() {
  reconnectWifi();

  if (lightCandles == true) 
  {
    flickerLEDs();
  } else {
    killLEDs();
  }

  if ( !client.connected() ) {
    mqttreconnect();
  }


  client.loop();
}
