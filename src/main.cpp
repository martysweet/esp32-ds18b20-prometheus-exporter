#include <Arduino.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>

extern "C" {
    #include "Esp.h"
}

// One Wire Data data port and max sensors
#define ONE_WIRE_BUS 14
#define MAX_SENSORS 10

struct DeviceInfo {
  DeviceAddress address;
  String addressStr;
  float lastReading;
};

// Global Parameters
WiFiManager wifiManager;
AsyncWebServer server(8080);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Vars
uint loopCounter = 0;
DeviceInfo oneWireDevices[MAX_SENSORS] = {};
int actualSensorCount = 0;

// Formatting Functions
String createTempMetric(const char* sensorId, float value){
  char strBuf[50];
  sprintf(strBuf, "temperature{sensorId=\"%s\"} %.2f\n", sensorId, value);  // Change this line to change the labels for the metric
  return String(strBuf);
}

String createStatusMetrics(){
  char strBuf[60];
  sprintf(strBuf, "iterations{} %d\n", loopCounter);
  return String(strBuf);
}

String formatSensorId(DeviceAddress deviceAddress){
  String adm;
  for (uint8_t i = 0; i < 8; i++){  
    adm = adm + String(deviceAddress[i], HEX);
  }
  return adm;
}

// Setup
void setup(){
  // Serial
  Serial.begin(115200);  

  // Discover the one-wire devices
  // Bug Workaround: https://github.com/PaulStoffregen/OneWire/issues/57
  delay(500);
  sensors.begin();
  delay(1000);
  sensors.begin();
  
  Serial.printf("Dallas count: %d\n", sensors.getDeviceCount());

  for(int i =0; i<MAX_SENSORS; i++){
    if(!sensors.getAddress(oneWireDevices[i].address, i)){
      break;
    }
    oneWireDevices[i].addressStr = formatSensorId(oneWireDevices[i].address);
    actualSensorCount++;
  }

  Serial.printf("Found %d sensors!\n", actualSensorCount);

  // // Setup WiFi
  WiFi.mode(WIFI_STA);
  wifiManager.setClass("invert"); // dark theme
  wifiManager.setConfigPortalTimeout(180);

  // Set an unique name for the device
  uint32_t id = 0;
  for(int i=0; i<17; i=i+8) {
    id |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  String apString = String("ESP32PROMTEMP_" + String(id, HEX));
  apString.toUpperCase();
  
  wifiManager.setHostname(apString.c_str());
  bool res = wifiManager.autoConnect(); // password protected ap
  if(!res) {
    Serial.println("Failed to connect or hit timeout");
    ESP.restart();
  } 

  // Web Server
  server.on("/metrics", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String payload = createStatusMetrics();

    // Add the Temp Sensors
    for(int i=0; i<actualSensorCount; i++){
      float reading = oneWireDevices[i].lastReading;
      // Sanity check so we don't emit error codes
      // This will also highlight any BUS issues
      if(reading > 5){  
        payload += createTempMetric(oneWireDevices[i].addressStr.c_str(), reading);
      }else{
        Serial.println(reading);
      }
    }

    request->send(200, "text/plain", payload);
  });
  server.begin();

}

void loop(){

  loopCounter++;

  // Request One Wire Temps
  sensors.requestTemperatures();
  for(int i=0; i<actualSensorCount; i++){
    oneWireDevices[i].lastReading = sensors.getTempC(oneWireDevices[i].address);
  }


  delay(5000);  
}