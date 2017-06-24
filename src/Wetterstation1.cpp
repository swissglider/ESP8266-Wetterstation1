// #################################################
// ## Framework include
// #################################################
#include <Arduino.h>
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

// needed for WiFiManager
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

// needed for OTA
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

// #################################################
// ## Application include
// #################################################
#include <ESP8266HTTPClient.h>
// includes for JSON
#include <ArduinoJson.h>
// includes for DHT22
#include <DHT.h>
#include <DHT_U.h>
// includes for HP206C
#include <KalmanFilter.h>
#include <HP20x_dev.h>
#include <SPI.h>
#include <Wire.h>

// #################################################
// ## Framework parameter
// #################################################
const char* apName = "esp8266_Wetterstation1";
const char* hostName = "esp8266_Wetterstation1";
const char* passwort = "8266";

// #################################################
// ## Application parameter
// #################################################
// global parameter
#define SERIAL_BAUD 115200
const int sleepTimeS = 240;
String device = "ArduinoSensor";
String channel = "11";
String name = "Weatherstation_Arduino1";

// wifi settings
String webSockerURI1 = "http://192.168.86.167:1880/ArduinoSensorWS";
HTTPClient http;

// parameter for DHT22
#define DHTPIN            D7         // Pin which is connected to the DHT sensor.
#define DHTTYPE           DHT22     // DHT 22 (AM2302)
DHT_Unified dht(DHTPIN, DHTTYPE);

// Raindrop Detection YL-83
const int analogPin=A0; //the AO of the module attach to A0
const int digitalPin=D6; //D0 attach to pin D6
boolean Dstate=0; //store the value of D0

// parameter for HP206C
unsigned char ret = 0;
KalmanFilter t_filter;    //temperature filter
KalmanFilter p_filter;    //pressure filter
KalmanFilter a_filter;    //altitude filter

// #################################################
// ## Application Functions
// #################################################
// **********************************************
// Write to WebSocket

void writeWebSocketMessage(String json, String sockerURI){
  http.begin(sockerURI);
  int httpCode = http.POST(json);
  http.end();
}

// **********************************************
// DHT22 Temperatur Sensor

void initDHT22(){
  dht.begin();
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
}

void readDHT22(String& json){
  float temp(NAN), humidity(NAN);
  sensors_event_t event;
  // Get Temperatur event and print its value.
  dht.temperature().getEvent(&event);
  if (isnormal(event.temperature)) {
    temp = event.temperature;
  }
  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  if (isnormal(event.relative_humidity)) {
    humidity = event.relative_humidity;
  }

  DynamicJsonBuffer  jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  JsonObject& values = root.createNestedObject("values");
  values["temp"] = temp;
  values["hum"] = humidity;
  root["device"] = device;
  root["channel"] = channel;
  root["name"] = name;
  root.printTo(json);
}

// **********************************************
// Raint Trop Sensor YL-83

void initRainSensor(){
  pinMode(digitalPin,INPUT); //set digitalPin as INPUT
}

void readRainSensor(String& json){
  int Astate=analogRead(analogPin); //read the value of A0
  boolean Dstate=digitalRead(digitalPin); //read the value of D0

  DynamicJsonBuffer  jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  JsonObject& values = root.createNestedObject("values");
  values["digital"] = Dstate;
  values["rain_level"] = Astate;
  root["device"] = device;
  root["channel"] = channel;
  root["name"] = name;
  root.printTo(json);
}

// **********************************************
// Barometer Sensor HP206C

void initHP206C(){
  HP20x.begin();
  delay(100);
}

void readHP206C(String& json){
  float hp206C_temp = HP20x.ReadTemperature()/100.0;
  float hp206C_pres = HP20x.ReadPressure()/100.0;
  float hp206C_alt = HP20x.ReadAltitude()/100.0;
  //float hp206C_temp_filter = t_filter.Filter(hp206C_temp);
  //float hp206C_pres_filter = p_filter.Filter(hp206C_pres);
  //float hp206C_alt_filter = a_filter.Filter(hp206C_alt);

  DynamicJsonBuffer  jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  JsonObject& values = root.createNestedObject("values");
  values["temp"] = hp206C_temp;
  values["pres"] = hp206C_pres;
  values["alt"] = hp206C_alt;
  //values["temp_filter"] = hp206C_temp_filter;
  //values["pres_filter"] = hp206C_pres_filter;
  //values["alt_filter"] = hp206C_alt_filter;
  root["device"] = device;
  root["channel"] = channel;
  root["name"] = name;
  root.printTo(json);
}

void setup() {
    Serial.begin(115200);
    Serial.println("Booting");

    // #################################################
    // ## Framework - WiFiManager
    // #################################################
    WiFiManager wifiManager;
    //reset saved settings
    //wifiManager.resetSettings();
    wifiManager.autoConnect(apName, passwort);

    //if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");

    // #################################################
    // ## Framework - OTA
    // #################################################
    ArduinoOTA.setPort(8266);
    ArduinoOTA.setHostname(hostName);

    ArduinoOTA.setPassword(passwort);
      ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // #################################################
  // ## Start Application Setup here
  // #################################################
  //debug ausgaben auf die serielle schnittstelle
  Serial.begin(SERIAL_BAUD);
  // setup DHT22
  initDHT22();
  // setup Rain Sensor YL-83
  initRainSensor();
  // setup HP206C
  initHP206C();

}

void loop() {
  // #################################################
  // ## Framework - OTA
  // #################################################
  ArduinoOTA.handle();

  // #################################################
  // ## Start Application loop here
  // #################################################
  // read dht22
  String dht22_json = "";
  readDHT22(dht22_json);
  writeWebSocketMessage(dht22_json, webSockerURI1);

  // read rain sensor YL-83
  String rs_json = "";
  readRainSensor(rs_json);
  writeWebSocketMessage(rs_json, webSockerURI1);

  // read hp206C
  String hp206C_json = "";
  readHP206C(hp206C_json);
  writeWebSocketMessage(hp206C_json, webSockerURI1);

  delay(sleepTimeS * 1000);
}
