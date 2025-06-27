#include "header.h"

void handleSerialCommands() {
  if (Serial.available() > 0) {
    String command = Serial.readString();
    command.trim();

    Serial.print("Processing command: ");
    Serial.println(command);

    if (command == "status") {
      // printSystemStatus();
    }
  }
}

void detectButtonPress() {
  static bool lastState = HIGH;
  static unsigned long lastPressTime = 0;
  bool currentState = digitalRead(BUTTON_PIN);

  if (lastState == HIGH && currentState == LOW) {
    unsigned long now = millis();
    if (now - lastPressTime > 200) { // 200ms debounce
      Serial.println("Button pressed!");
      lastPressTime = now;
    }
  }

  lastState = currentState;
}



