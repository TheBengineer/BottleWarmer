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
float setTemperature = 104;
float sterilizeTemperature = 160;
float sterilizeHour = 4;  // 11PM UTC
bool updateTemperatureNow = true;
uint8_t setPoint = 0;
float temperatureErrorAccumulator = 0;

float PID_p = 10.0;
float PID_i = 30.0 / (15.0 * 500.0);


bool updateWIFINow = false;
bool updateNTPNow = true;
bool updateScreenNow = true;


long buttonPressTime = 0;
bool shortPress = false;
bool updateInterface = false;

#define HOME 0
#define SET_VARIABLE 10
#define SET_TEMP 1
#define SET_STERILIZE_TEMP 2
#define SET_STERILIZE_TIME 3
#define CLEANUP 11
int interfaceTime = 0;
int interfaceState = HOME;


static const unsigned char PROGMEM bottle_bmp[] = { 0x1, 0x80, 0x2, 0x40, 0x2, 0x40, 0x2, 0x40, 0x2, 0x40, 0x4, 0x20, 0x8, 0x10, 0x10, 0x8, 0x3f, 0xfc, 0x40, 0x2,
                                                    0x40, 0x2, 0x40, 0x2, 0x7f, 0xfe, 0x20, 0x4, 0x40, 0x2, 0x81, 0x81, 0x80, 0x1, 0x80, 0x1, 0x80, 0x1, 0x81, 0x81,
                                                    0x80, 0x1, 0x80, 0x1, 0x80, 0x1, 0x81, 0x81, 0x80, 0x1, 0x80, 0x1, 0x80, 0x1, 0x81, 0x81, 0x80, 0x1, 0x80, 0x1,
                                                    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf9, 0x9f, 0xf6, 0x6f, 0xf7, 0xef, 0xfb, 0xdf, 0xfd, 0xbf,
                                                    0xfe, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0xfe, 0x3f, 0xfc };

OneWire oneWire(TEMPERATURE_PIN);
DallasTemperature sensors(&oneWire);
ESPRotary r;
Ticker tickerTemperature;
Ticker tickerWifi;
Ticker tickerNTP;
Ticker tickerPWM;
Ticker tickerScreen;
Ticker tickerPID;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
ESP8266WebServer server(80);

void setup(void) {
  Serial.begin(74880);
  Serial.println(F("BottleWarmer Starting"));

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
  display.drawBitmap(8, 16, bottle_bmp, 16, 45, SSD1306_WHITE);
  display.drawBitmap(40, 16, bottle_bmp, 16, 45, SSD1306_WHITE);
  display.drawBitmap(72, 16, bottle_bmp, 16, 45, SSD1306_WHITE);
  display.drawBitmap(104, 16, bottle_bmp, 16, 45, SSD1306_WHITE);
  display.display();
  tickerScreen.attach_ms(250, []() {
    updateScreenNow = true;
  });

  WiFi.mode(WIFI_STA);
  WiFiManager wifiManager;
  if (!wifiManager.autoConnect(DEVICE_NAME)) {
    display.clearDisplay();
    display.println(F("WIFI not connected."));
    display.display();
    delay(3000);
    ESP.reset();
    delay(5000);
  }
  clearText();
  display.print(F("Connected to "));
  display.println(WiFi.SSID());
  display.print(F("With IP:"));
  display.println(WiFi.localIP());
  display.display();
  delay(1500);

  tickerWifi.attach(30, []() {
    updateWIFINow = true;
  });

  setupOTA();

  pinMode(ROTARY_BUTTON, INPUT);
  // if (!digitalRead(ROTARY_BUTTON)) {
  //   Serial.println(F("Waiting for OTA"));
  //   display.clearDisplay();
  //   display.println(F("Waiting for OTA"));
  //   display.display();
  //   while (true) {
  //     ArduinoOTA.handle();
  //   }
  // }

  setupServer();

  sensors.begin();
  tickerTemperature.attach_ms(500, []() {
    updateTemperatureNow = true;
  });

  r.begin(ROTARY_PIN1, ROTARY_PIN2, CLICKS_PER_STEP, INT16_MIN, INT16_MAX, setTemperature);
  r.setChangedHandler(rotate);

  attachInterrupt(digitalPinToInterrupt(ROTARY_PIN1), handleLoop, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ROTARY_PIN2), handleLoop, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ROTARY_BUTTON), buttonDown, FALLING);
  attachInterrupt(digitalPinToInterrupt(ROTARY_BUTTON), buttonUp, RISING);

  pinMode(RELAY_PIN, OUTPUT);
  tickerPID.attach(1, runPID);

  timeClient.begin();
  tickerNTP.attach(3600, []() {
    updateNTPNow = true;
  });
  delay(1000);
  setupScreen();
}

void loop(void) {
  reconnect_wifi();
  ArduinoOTA.handle();
  updateTemperature();
  server.handleClient();
  updateTime();
  handleInterface();
  updateScreen();
}

void setupOTA() {
  ArduinoOTA.setPassword((const char*)"admin");
  ArduinoOTA.onStart([]() {
    Serial.println(F("OTA Start"));
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println(F("OTA Updating"));
    display.drawRect(0, 9, 104, 7, SSD1306_WHITE);
    display.display();
  });
  ArduinoOTA.onEnd([]() {
    Serial.println(F("\nDone"));
    display.println(F("\nDone"));
    display.display();
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    int percent = (progress / (total / 100));
    Serial.printf("Progress: %u%%\r", percent);
    display.fillRect(106, 8, 24, 8, 0x00);               // bank out the current progress
    display.fillRect(2, 11, percent, 3, SSD1306_WHITE);  // bank out the current progress
    display.setCursor(106, 8);
    display.setTextSize(1);
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
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("\n" + msg);
    display.display();
  });
  ArduinoOTA.begin();
  clearText();
  display.println("OTA Ready");
  display.display();
  delay(500);
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
  if (updateWIFINow) {
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
    updateWIFINow = false;
  }
}

ICACHE_RAM_ATTR void buttonUp() {
  if (millis() - buttonPressTime > 10) {
    shortPress = true;
  }
}

ICACHE_RAM_ATTR void buttonDown() {
  buttonPressTime = millis();
}

void wipeYellow() {
  display.fillRect(0, 0, 128, 16, 0x00);
  display.display();
}

void clearText() {
  wipeYellow();
  display.setCursor(0, 0);
}

void updateTemperature() {
  if (updateTemperatureNow) {
    sensors.requestTemperatures();
    float t = sensors.getTempFByIndex(0);
    if (temperatureF == 0) {
      temperatureF = t;
    }
    temperatureF = (temperatureF * .9) + (t * .1);
    t = sensors.getTempFByIndex(1);
    if (temperatureF2 == 0) {
      temperatureF2 = t;
    }
    temperatureF2 = (temperatureF2 * .9) + (t * .1);
    updateTemperatureNow = false;
  }
}


void updateTime() {
  if (updateNTPNow) {
    timeClient.update();
    updateNTPNow = false;
  }
}


void handleInterface() {
  if (updateInterface) {
    switch (interfaceState) {
      case SET_TEMP:
        setTemperature = r.getPosition();
        updateScreenTemperature();
        break;
      case SET_STERILIZE_TEMP:
        sterilizeTemperature = r.getPosition();
        updateScreenSterilizeTemp();
        break;
      case SET_STERILIZE_TIME:
        sterilizeHour = r.getPosition();
        updateScreenSterilizeTime();
        break;
      case SET_VARIABLE:
        updateSelection();
        break;
    }
    updateInterface = false;
  }

  if (shortPress) {
    shortPress = false;
    int p = r.getPosition();
    switch (interfaceState) {
      case HOME:
        interfaceState = SET_VARIABLE;
        display.drawCircle(120, 8, 7, SSD1306_WHITE);
        display.drawLine(120, 1, 120, 6, SSD1306_WHITE);
        display.display();
        r.resetPosition(1);
        break;
      case SET_VARIABLE:
        switch (p) {
          case SET_TEMP:
            r.resetPosition(int(setTemperature));
            break;
          case SET_STERILIZE_TEMP:
            r.resetPosition(int(sterilizeTemperature));
            break;
          case SET_STERILIZE_TIME:
            r.resetPosition(int(sterilizeHour));
            break;
          default:
            interfaceState = CLEANUP;
            break;
        }
        interfaceState = p;
        display.drawCircle(120, 8, 3, SSD1306_WHITE);
        display.display();
        break;
      case SET_TEMP:
        // EEPROM save
        interfaceState = CLEANUP;
        shortPress = true;
        break;
      case SET_STERILIZE_TEMP:
        // EEPROM save
        interfaceState = CLEANUP;
        shortPress = true;
        break;
      case SET_STERILIZE_TIME:
        // EEPROM save
        interfaceState = CLEANUP;
        shortPress = true;
        break;
      case CLEANUP:
      default:
        display.drawCircle(120, 8, 7, 0x00);
        display.drawCircle(120, 8, 3, 0x00);
        display.drawLine(120, 1, 120, 6, 0x00);
        updateSelection();
        interfaceState = HOME;
        break;
    }
  }
}


void setupScreen() {
  display.clearDisplay();
  display.setCursor(1, 1);
  display.setTextSize(1);
  display.print("Set:");
  display.print(setTemperature, 0);
  display.setCursor(1, 9);
  display.print("Hot:");
  display.print(sterilizeTemperature, 0);
  display.setCursor(50, 1);
  display.print("Set:");
  display.print(sterilizeHour, 0);
  display.print(":00");
  display.setCursor(50, 9);
  display.print("Now:");
  display.print(timeClient.getHours());
  display.print(":");
  display.print(timeClient.getMinutes());
  display.drawBitmap(0, 16, bottle_bmp, 16, 45, SSD1306_WHITE);

  display.setTextSize(6);
  display.setCursor(20, 16);
  display.println(temperatureF, 0);

  display.setTextSize(1);
  display.display();
}

void updateScreenSetTemp() {
  display.setTextSize(1);
  display.fillRect(24, 1, 24, 8, 0x00);  // blank set temperature
  display.setCursor(24, 1);
  display.print(setTemperature, 0);
  display.display();
}

void updateScreenSterilizeTemp() {
  display.setTextSize(1);
  display.fillRect(24, 9, 24, 8, 0x00);  // blank set temperature
  display.setCursor(24, 9);
  display.print(sterilizeTemperature, 0);
  display.display();
}

void updateScreenSterilizeTime() {
  display.setTextSize(1);
  display.fillRect(74, 1, 32, 8, 0x00);  // blank Times
  display.setCursor(74, 1);
  display.print(sterilizeHour, 0);
  display.print(":00");
  display.display();
}

void updateScreenTimeNow() {
  display.setTextSize(1);
  display.fillRect(74, 9, 32, 8, 0x00);  // blank Times
  display.setCursor(74, 9);
  display.print(timeClient.getHours());
  display.print(":");
  display.print(timeClient.getMinutes());
  display.display();
}

void updateScreenTemperature() {
  display.fillRect(20, 16, 108, 48, 0x00);  // blank temperature
  display.setCursor(20, 18);
  display.setTextSize(6);
  display.print(temperatureF, 0);
  display.display();
  display.setTextSize(1);
}

void clearSelection() {
  display.drawRect(0, 0, 24, 9, 0x00);
  display.drawRect(0, 8, 24, 9, 0x00);
  display.drawRect(49, 0, 24, 9, 0x00);
}

void updateSelection() {
  int p = r.getPosition();
  clearSelection();
  if (interfaceState == SET_VARIABLE) {
    switch (p) {
      case SET_TEMP:
        display.drawRect(0, 0, 24, 9, SSD1306_WHITE);
        break;
      case SET_STERILIZE_TEMP:
        display.drawRect(0, 8, 24, 9, SSD1306_WHITE);
        break;
      case SET_STERILIZE_TIME:
        display.drawRect(49, 0, 24, 9, SSD1306_WHITE);
        break;
    }
  }
  display.display();
}

void updateScreen() {
  if (updateScreenNow) {
    updateScreenSetTemp();
    updateScreenSterilizeTemp();
    updateScreenSterilizeTime();
    updateScreenSterilizeTime();
    updateScreenTimeNow();
    updateScreenTemperature();
    updateInterface = true;
    updateScreenNow = false;
  }
}


ICACHE_RAM_ATTR void handleLoop() {
  r.loop();
}


// on change
void rotate(ESPRotary& r) {
  updateInterface = true;
}

String BuildSensorJson() {
  DynamicJsonDocument result_json(1024);
  result_json["t1"] = temperatureF;
  result_json["t2"] = temperatureF2;
  result_json["set"] = setTemperature;
  result_json["hot"] = sterilizeTemperature;
  result_json["st_h"] = sterilizeHour;
  result_json["pwm"] = setPoint;
  result_json["p"] = PID_p;
  result_json["i"] = PID_i;
  result_json["acc"] = temperatureErrorAccumulator;
  result_json["state"] = interfaceState;
  result_json["r"] = r.getPosition();
  result_json["time"] = timeClient.getFormattedTime();

  String message = "";
  serializeJson(result_json, message);

  return message;
}

void runPID() {
  float targetTemp = setTemperature;
  if (timeClient.getHours() == sterilizeHour) {
    targetTemp = sterilizeTemperature;
  }
  float error = targetTemp - temperatureF;
  if (error < 15) {
    temperatureErrorAccumulator += error;
  }
  float out = (PID_p * error) + (PID_i * temperatureErrorAccumulator);
  out = bound(out, 0.0, 100.0);
  setPoint = int(out);
  activateRelay();
}


void activateRelay() {
  if (setPoint) {
    digitalWrite(RELAY_PIN, HIGH);
    tickerPWM.once_ms(setPoint * 10, deactivateRelay);
  }
}

void deactivateRelay() {
  if (setPoint != 100) {
    digitalWrite(RELAY_PIN, LOW);
  }
}

uint8_t boundPWM(uint8_t x) {
  if (x < 0) { x = 0; }
  if (x > 100) { x = 100; }
  return x;
}

// Bound the input value between x_min and x_max.
float bound(float x, float x_min, float x_max) {
  if (x < x_min) { x = x_min; }
  if (x > x_max) { x = x_max; }
  return x;
}