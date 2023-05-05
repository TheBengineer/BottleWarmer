/*
 * Rui Santos 
 * Complete Project Details https://randomnerdtutorials.com
 */

// Temp sensors
#include <OneWire.h>
#include <DallasTemperature.h>
// Rotary + button
#include "ESPRotary.h"  // https://github.com/LennartHennigs/ESPRotary
#include "Button2.h"    // https://github.com/LennartHennigs/Button2
// Wifi
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
// HTTP
#include <ESPAsyncWebServer.h>
// OTA
#include <ArduinoOTA.h>
// Screen
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
// Tools
#include <NTPClient.h>
#include "Ticker.h"
#include <ArduinoJson.h>

#define SCREEN_SDA 4
#define SCREEN_SCL 5
#define SCREEN_WIDTH 128     // OLED display width, in pixels
#define SCREEN_HEIGHT 32     // OLED display height, in pixels
#define SCREEN_ADDRESS 0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

#define ROTARY_PIN1 14   //D5
#define ROTARY_PIN2 12   //D6
#define ROTARY_BUTTON 2  //D4
#define CLICKS_PER_STEP 4

#define RELAY_PIN 15  //D8

#define TEMPERATURE_PIN 13  //D7

#define DEVICE_NAME "BottleWarmer"

float temperatureF = 0;
float temperatureF2 = 0;


OneWire oneWire(TEMPERATURE_PIN);
DallasTemperature sensors(&oneWire);
ESPRotary r;
Button2 b;
Ticker t;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
AsyncWebServer server(80);

void setup(void) {
  Serial.begin(74880);
  Serial.println("BottleWarmer Starting");

  WiFi.mode(WIFI_STA);
  WiFiManager wifiManager;
  if (!wifiManager.autoConnect(DEVICE_NAME)) {
    delay(3000);
    ESP.reset();
    delay(5000);
  }
  t.attach(30, reconnect_wifi);

  setupOTA();

  setupServer();

  sensors.begin();
  t.attach_ms(500, updateTemperature);

  r.begin(ROTARY_PIN1, ROTARY_PIN2, CLICKS_PER_STEP);
  r.setChangedHandler(rotate);
  r.setLeftRotationHandler(showDirection);
  r.setRightRotationHandler(showDirection);
  b.begin(ROTARY_BUTTON);
  b.setTapHandler(click);
  b.setLongClickHandler(longClick);

  pinMode(RELAY_PIN, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(ROTARY_PIN1), handleLoop, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ROTARY_PIN2), handleLoop, CHANGE);

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.setTextSize(1);               // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);  // Draw white text
  display.setCursor(0, 0);              // Start at top-left corner
  display.println(F("Bottle Warmer"));
  display.display();

  timeClient.begin();
  t.attach(3600, timeClient.update);
}

void loop(void) {
  ArduinoOTA.handle();

}

void setupOTA() {
  ArduinoOTA.setPassword((const char*)"admin");
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
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
  Serial.println("OTA Ready");
}

void setupServer(){
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(200, "text/html", BuildSensorJson());
  });
  server.on("/calibrate", HTTP_ANY, [](AsyncWebServerRequest* request) {
    int paramsNr = request->params();
    Serial.println(paramsNr);
    bool updated = false;

    for (int i = 0; i < paramsNr; i++) {

      AsyncWebParameter* p = request->getParam(i);
      if (p->name() == "phm") {
        PH_m = p->value().toFloat();
        updated = true;
      }
      if (p->name() == "phb") {
        PH_b = p->value().toFloat();
        updated = true;
      }
      if (p->name() == "ecm") {
        EC_m = p->value().toFloat();
        updated = true;
      }
      if (p->name() == "ecb") {
        EC_b = p->value().toFloat();
        updated = true;
      }
    }
    if (updated) {
      EEPROM.put(0, PH_m);
      EEPROM.put(4, PH_b);
      EEPROM.put(8, EC_m);
      EEPROM.put(12, EC_b);
      EEPROM.commit();
    }
    request->send(200, "text/html", BuildCalibrationPage());
  });
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void reconnect_wifi() {  //function to reconnect wifi if its not connected
  if (!wifi_isconnected()) {
    WiFi.begin(ssid.c_str(), password.c_str());
    Serial.print("Connecting to: ");
    Serial.println(ssid);
    while (!wifi_isconnected()) {
      delay(1000);
      Serial.print(".");
    }
    Serial.println();
    Serial.print("Got IP: ");
    Serial.println(WiFi.localIP());
  }
}

void click(){}

void longClick(){}

void updateTemperature() {
  sensors.requestTemperatures();
  temperatureF = sensors.getTempFByIndex(0);
  temperatureF2 = sensors.getTempFByIndex(1);
}

void updateScreen() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print(temperatureF);
  display.println("ºF");
  display.print(temperatureF2);
  display.println("ºF");
  display.println(digitalRead(ROTARY_BUTTON));
  display.println(r.getPosition());
  display.display();
}


ICACHE_RAM_ATTR void handleLoop() {
  r.loop();
}


// on change
void rotate(ESPRotary& r) {
  Serial.println(r.getPosition());
}

// on left or right rotattion
void showDirection(ESPRotary& r) {
  Serial.println(r.directionToString(r.getDirection()));
}