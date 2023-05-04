/*
 * Rui Santos 
 * Complete Project Details https://randomnerdtutorials.com
 */

#include <OneWire.h>
#include <DallasTemperature.h>
#include "ESPRotary.h"

#define ROTARY_PIN1	15
#define ROTARY_PIN2	13
#define CLICKS_PER_STEP 4   
#define ROTARY_BUTTON	12

#define RELAY_PIN	2

#define TEMPERATURE_PIN	0



OneWire oneWire(TEMPERATURE_PIN);
DallasTemperature sensors(&oneWire);
ESPRotary r;


void setup(void) {
  Serial.begin(74880);
  sensors.begin();

  r.begin(ROTARY_PIN1, ROTARY_PIN2, CLICKS_PER_STEP);
  r.setChangedHandler(rotate);
  r.setLeftRotationHandler(showDirection);
  r.setRightRotationHandler(showDirection);

  pinMode(RELAY_PIN, OUTPUT);
}

void loop(void) {
  sensors.requestTemperatures(); 
  float temperatureF = sensors.getTempFByIndex(0);
  float temperatureF2 = sensors.getTempFByIndex(1);
  Serial.print(temperatureF);
  Serial.print("ºF, ");
  Serial.print(temperatureF2);
  Serial.println("ºF");
  delay(500);
}

// on change
void rotate(ESPRotary& r) {
   Serial.println(r.getPosition());
}

// on left or right rotattion
void showDirection(ESPRotary& r) {
  Serial.println(r.directionToString(r.getDirection()));
}