void cleanup() {
  interfaceState = CLEANUP;
  shortPress = true;
}


void handleInterface() {
  if (updateInterface) {
    switch (interfaceState) {
      case SET_TEMP:
        setTemperature = r.getPosition();
        updateGUISetTemperature();
        break;
      case SET_STERILIZE_TEMP:
        sterilizeTemperature = r.getPosition();
        updateGUISetSterilizeTemp();
        break;
      case SET_STERILIZE_TIME:
        sterilizeHour = r.getPosition();
        updateGUISetSterilizeTime();
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
        setupGUI();
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
            cleanup();
            break;
        }
        interfaceState = p;
        break;
      case SET_TEMP:
        EEPROM.put(0, setTemperature);
        cleanup();
        break;
      case SET_STERILIZE_TEMP:
        EEPROM.put(1, sterilizeTemperature);
        cleanup();
        break;
      case SET_STERILIZE_TIME:
        EEPROM.put(2, sterilizeHour);
        cleanup();
        break;
      case CLEANUP:
      default:
        interfaceState = HOME;
        break;
    }
  }
}
