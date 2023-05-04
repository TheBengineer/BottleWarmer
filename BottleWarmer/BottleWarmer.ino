/*
 * Rui Santos 
 * Complete Project Details https://randomnerdtutorials.com
 */

#include <OneWire.h>
#include <DallasTemperature.h>

// GPIO where the DS18B20 is connected to
const int oneWireBus = 0;     

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);

DallasTemperature sensors(&oneWire);
// Based on the OneWire library example


void setup(void) {
  Serial.begin(74880);
  sensors.begin();
}

void loop(void) {
  sensors.requestTemperatures(); 
  float temperatureF = sensors.getTempFByIndex(0);
  float temperatureF2 = sensors.getTempFByIndex(1);
  Serial.print(temperatureF);
  Serial.print("ºF, ");
  Serial.print(temperatureF2);
  Serial.println("ºF");
  delay(5000);
}