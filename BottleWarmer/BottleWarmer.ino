/*
 * Rui Santos 
 * Complete Project Details https://randomnerdtutorials.com
 */

#include <OneWire.h>
#include <DallasTemperature.h>
#include "ESPRotary.h"
#include "Ticker.h"
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <sequencer4.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <NTPClient.h>

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
Ticker t;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdateServer;



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

  sensors.begin();

  r.begin(ROTARY_PIN1, ROTARY_PIN2, CLICKS_PER_STEP);
  r.setChangedHandler(rotate);
  r.setLeftRotationHandler(showDirection);
  r.setRightRotationHandler(showDirection);
  t.attach_ms(3, handleLoop);

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(ROTARY_PIN1, INPUT);
  pinMode(ROTARY_PIN2, INPUT);
  pinMode(ROTARY_BUTTON, INPUT);

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
}

void loop(void) {
  updateTemperature();
  r.loop();
  ArduinoOTA.handle();
  Serial.print(temperatureF);
  Serial.print("ºF, ");
  Serial.print(temperatureF2);
  Serial.print("ºF, ");
  Serial.print(digitalRead(ROTARY_BUTTON));
  Serial.print(", ");
  Serial.println(r.getPosition());

  timeClient.update();

  Serial.println(timeClient.getFormattedTime());

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

void updateTemperature() {
  sensors.requestTemperatures();
  temperatureF = sensors.getTempFByIndex(0);
  temperatureF2 = sensors.getTempFByIndex(1);
}

void handleLoop() {
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