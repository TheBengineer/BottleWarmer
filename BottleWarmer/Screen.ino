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
  display.printf("%02d", sterilizeHour);
  display.print(":00");
  display.setCursor(50, 9);
  display.print("Now:");
  display.printf("%02d", timeClient.getHours());
  display.print(":");
  display.printf("%02d", timeClient.getMinutes());
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
  display.printf("%02d", sterilizeHour);
  display.print(":00");
  display.display();
}

void updateScreenTimeNow() {
  display.setTextSize(1);
  display.fillRect(74, 9, 32, 8, 0x00);  // blank Times
  display.setCursor(74, 9);
  display.printf("%02d", timeClient.getHours());
  display.print(":");
  display.printf("%02d", timeClient.getMinutes());
  display.display();
}

void blankTemperature() {
  display.fillRect(20, 16, 108, 48, 0x00);  // blank temperature
}

void updateScreenTemperature() {
  blankTemperature();
  display.setCursor(20, 18);
  display.setTextSize(6);
  display.print(temperatureF, 0);
  display.display();
  display.setTextSize(1);
}

void setupGUI() {
  blankTemperature();
  display.setTextSize(2);
  display.setCursor(22, 17);
  display.print("Set:");
  display.setCursor(22, 33);
  display.print("Hot:");
  display.setCursor(22, 49);
  display.print("Time:");
  display.display();
}

void updateGUISetTemperature() {
  display.fillRect(82, 17, 58, 14, 0x00);
  display.setTextSize(2);
  display.setCursor(82, 17);
  display.print(float(setPoint), 0);
  display.display();
  display.setTextSize(1);
}

void updateGUISetSterilizeTemp() {
  display.fillRect(82, 33, 58, 14, 0x00);
  display.setTextSize(2);
  display.setCursor(82, 33);
  display.print(float(sterilizeTemperature), 0);
  display.display();
  display.setTextSize(1);
}
void updateGUISetSterilizeTime() {
  display.fillRect(82, 49, 58, 14, 0x00);
  display.setTextSize(2);
  display.setCursor(82, 49);
  display.printf("%02d", float(sterilizeHour));
  display.print(":00");
  display.display();
  display.setTextSize(1);
}

void clearSelection() {
  display.drawRect(20, 16, 60, 16, 0x00);
  display.drawRect(20, 32, 60, 16, 0x00);
  display.drawRect(20, 48, 60, 16, 0x00);
}



void updateSelection() {
  int p = r.getPosition();
  clearSelection();
  if (interfaceState == SET_VARIABLE) {
    switch (p) {
      case SET_TEMP:
        display.drawRect(20, 16, 60, 16, SSD1306_WHITE);
        break;
      case SET_STERILIZE_TEMP:
        display.drawRect(20, 32, 60, 16, SSD1306_WHITE);
        break;
      case SET_STERILIZE_TIME:
        display.drawRect(20, 48, 60, 16, SSD1306_WHITE);
        break;
    }
  }
  display.display();
}

void updateScreen() {
  if (updateScreenNow) {
    if (interfaceState == HOME) {
      updateScreenSetTemp();
      updateScreenSterilizeTemp();
      updateScreenSterilizeTime();
      updateScreenSterilizeTime();
      updateScreenTimeNow();
      updateScreenTemperature();
    } else {
      updateInterface = true;
    }
    updateScreenNow = false;
  }
}
