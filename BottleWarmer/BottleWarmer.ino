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
#include <ESP8266WebServer.h>
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
#define SCREEN_HEIGHT 64     // OLED display height, in pixels
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

long buttonPressTime = 0;

static const unsigned char PROGMEM logo_bmp[] = { 0x1, 0x80, 0x2, 0x40, 0x2, 0x40, 0x2, 0x40, 0x2, 0x40, 0x4, 0x20, 0x8, 0x10, 0x10, 0x8, 0x3f, 0xfc, 0x40, 0x2,
                                                  0x40, 0x2, 0x40, 0x2, 0x7f, 0xfe, 0x20, 0x4, 0x40, 0x2, 0x81, 0x81, 0x80, 0x1, 0x80, 0x1, 0x80, 0x1, 0x81, 0x81,
                                                  0x80, 0x1, 0x80, 0x1, 0x80, 0x1, 0x81, 0x81, 0x80, 0x1, 0x80, 0x1, 0x80, 0x1, 0x81, 0x81, 0x80, 0x1, 0x80, 0x1,
                                                  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf9, 0x9f, 0xf6, 0x6f, 0xf7, 0xef, 0xfb, 0xdf, 0xfd, 0xbf,
                                                  0xfe, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0xfe, 0x3f, 0xfc };

OneWire oneWire(TEMPERATURE_PIN);
DallasTemperature sensors(&oneWire);
ESPRotary r;
Ticker t;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
ESP8266WebServer server(80);

void setup(void) {
  Serial.begin(74880);
  Serial.println("BottleWarmer Starting");

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    delay(3000);
    ESP.reset();
    delay(5000);
  }
  display.clearDisplay();
  display.setTextSize(1);               // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);  // Draw white text
  display.setCursor(0, 0);              // Start at top-left corner
  display.println(F("Bottle Warmer"));
  display.drawBitmap(8, 16, logo_bmp, 16, 45, SSD1306_WHITE);
  display.drawBitmap(40, 16, logo_bmp, 16, 45, SSD1306_WHITE);
  display.drawBitmap(72, 16, logo_bmp, 16, 45, SSD1306_WHITE);
  display.drawBitmap(104, 16, logo_bmp, 16, 45, SSD1306_WHITE);
  display.display();

  WiFi.mode(WIFI_STA);
  WiFiManager wifiManager;
  if (!wifiManager.autoConnect(DEVICE_NAME)) {
    display.clearDisplay();
    display.println("WIFI not connected.");
    display.display();
    delay(3000);
    ESP.reset();
    delay(5000);
  }
  display.print("Connected to ");
  display.println(WiFi.SSID());
  display.print("With IP:");
  display.println(WiFi.localIP());
  display.display();

  t.attach(30, reconnect_wifi);

  setupOTA();

  setupServer();

  sensors.begin();
  t.attach_ms(500, updateTemperature);

  r.begin(ROTARY_PIN1, ROTARY_PIN2, CLICKS_PER_STEP);
  r.setChangedHandler(rotate);
  r.setLeftRotationHandler(showDirection);
  r.setRightRotationHandler(showDirection);
  pinMode(ROTARY_BUTTON, INPUT);
  attachInterrupt(digitalPinToInterrupt(ROTARY_PIN1), handleLoop, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ROTARY_PIN2), handleLoop, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ROTARY_BUTTON), buttonDown, FALLING);
  attachInterrupt(digitalPinToInterrupt(ROTARY_BUTTON), buttonUp, RISING);

  pinMode(RELAY_PIN, OUTPUT);


  timeClient.begin();
  t.attach(3600, updateTime);
}

void loop(void) {
  ArduinoOTA.handle();
  server.handleClient();
}

void setupOTA() {
  ArduinoOTA.setPassword((const char*)"admin");
  ArduinoOTA.onStart([]() {
    Serial.println("OTA Start");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("OTA Start");
    display.drawRect(0, 9, 104, 7, SSD1306_WHITE);
    display.display();
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nDone");
    display.println("\nDone");
    display.display();
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    int percent = (progress / (total / 100));
    Serial.printf("Progress: %u%%\r", percent);
    display.fillRect(106, 8, 24, 8, 0x00);               // bank out the current progress
    display.fillRect(2, 11, percent, 3, SSD1306_WHITE);  // bank out the current progress
    display.setCursor(106, 8);
    display.printf("%u%%\r", percent);
    display.display();
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    String msg = "";
    if (error == OTA_AUTH_ERROR) msg = "Auth Failed";
    else if (error == OTA_BEGIN_ERROR) msg = "Begin Failed";
    else if (error == OTA_CONNECT_ERROR) msg = "Connect Failed";
    else if (error == OTA_RECEIVE_ERROR) msg = "Receive Failed";
    else if (error == OTA_END_ERROR) msg = "End Failed";
    Serial.println(msg);
    display.println("\n" + msg);
    display.display();
  });
  ArduinoOTA.begin();
  display.println("OTA Ready");
}

void setupServer() {
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", BuildSensorJson());
  });

  server.onNotFound(handle_NotFound);

  server.begin();
}

void handle_NotFound() {
  server.send(404, "text/plain", F("Not Found\n\n"));
}

void reconnect_wifi() {  //function to reconnect wifi if its not connected
  if (!WiFi.status() == WL_CONNECTED) {
    Serial.print("Connecting to: ");
    Serial.println(WiFi.SSID());

    WiFiManager wifiManager;
    if (!wifiManager.autoConnect(DEVICE_NAME)) {
      delay(1000);
      Serial.print(".");
    }

    Serial.println();
    Serial.print("Got IP: ");
    Serial.println(WiFi.localIP());
  }
}

ICACHE_RAM_ATTR void buttonUp() {}

ICACHE_RAM_ATTR void buttonDown() {
  buttonPressTime = millis();
}

void updateTemperature() {
  sensors.requestTemperatures();
  temperatureF = sensors.getTempFByIndex(0);
  temperatureF2 = sensors.getTempFByIndex(1);
}

void updateTime() {
  timeClient.update();
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

String BuildSensorJson() {
  DynamicJsonDocument result_json(1024);
  result_json["t1"] = temperatureF;
  result_json["t2"] = temperatureF;
  result_json["time"] = timeClient.getFormattedTime();

  String message = "";
  serializeJson(result_json, message);

  return message;
}