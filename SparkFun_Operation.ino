/*
 Example using the SparkFun HX711 breakout board with a scale
 By: Nathan Seidle
 SparkFun Electronics
 Date: November 19th, 2014
 License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

 This example demonstrates basic scale output. See the calibration sketch to get the calibration_factor for your
 specific load cell setup.

 This example code uses bogde's excellent library: https://github.com/bogde/HX711
 bogde's library is released under a GNU GENERAL PUBLIC LICENSE

 The HX711 does one thing well: read load cells. The breakout board is compatible with any wheat-stone bridge
 based load cell which should allow a user to measure everything from a few grams to tens of tons.
 Arduino pin 2 -> HX711 CLK
 3 -> DAT
 5V -> VCC
 GND -> GND

 The HX711 board can be powered from 2.7V to 5V so the Arduino 5V power should be fine.

*/

#include "HX711.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ArduinoJson.h>

#define calibration_factor -7050.0 //This value is obtained using the SparkFun_HX711_Calibration sketch

#define DOUT  4
#define CLK  5

HX711 scale;

// Replace with your network credentials (STATION)
const char* ssid = "IphoneZ";
const char* password = "motorolaradio";

WiFiClient espClient;
PubSubClient mqttClient(espClient);
const char* server = "tracker.my";    // MQTT server (of your choice)

/********** For any JSON packet creation ************/
long lastMsg = 0;
char msg[100];
char temp[20], temp2[20], temp3[20];
int value = 0;
/************* end for JSON packet ******************/


void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);

  initWiFi();
  Serial.print("RRSI: ");
  Serial.println(WiFi.RSSI());

  Serial.println("HX711 scale demo");

  scale.begin(DOUT, CLK);
  scale.set_scale(calibration_factor); //This value is obtained by using the SparkFun_HX711_Calibration sketch
  scale.tare(); //Assuming there is no weight on the scale at start up, reset the scale to 0

  Serial.println("Readings:");

  mqttClient.setServer(server, 1883);
  //mqttClient.setCallback(callback);
}

void loop() {

  StaticJsonBuffer<50> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  if (!mqttClient.connected()) {
    reconnect();
  }

  delay(1000);
  mqttClient.loop();
  
  Serial.print("Reading: ");
  Serial.print(scale.get_units(), 1); //scale.get_units() returns a float
  Serial.print(" kg"); //You can change this to kg but you'll need to refactor the calibration_factor
  Serial.println();

    /*
  /******* Load Cell Sensor ***************/
  dtostrf( (float) scale.get_units(), 3, 2, temp );
  root["LoadCell_01"] = temp;
  /**************************************/
  
  root.printTo(msg);

  Serial.println();
  Serial.println(F("Publish message : "));
  Serial.println(msg);
  mqttClient.publish("LoadCell_01", msg);
}

void reconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.println();
    Serial.print(F("Attempting MQTT connection..."));
    // Attempt to connect
    if (mqttClient.connect("loadcell", "guest", "guest")) {
      Serial.println(F("connected"));
      // Once connected, publish an announcement...
      mqttClient.publish("LoadCell_01", "hello world");
      // ... and resubscribe
      //mqttClient.subscribe("LoadCell_01_Rx");
    } else {
      Serial.print(F("failed, rc="));
      Serial.print(mqttClient.state());
      Serial.println(F(" try again in 5 seconds"));
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
