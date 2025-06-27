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
  static bool lastState = LOW;
  bool currentState = digitalRead(BUTTON_PIN);

  if (currentState == HIGH && lastState == LOW) {
    Serial.println("Button pressed!");
  }

  lastState = currentState;
}

