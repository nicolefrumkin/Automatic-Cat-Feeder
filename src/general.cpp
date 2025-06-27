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

bool detectButtonPress() {
  static bool lastState = HIGH;
  bool currentState = digitalRead(BUTTON_PIN);
  bool is_pressed = false;

  if (currentState == HIGH && lastState == LOW) {
    is_pressed = true;
  }

  lastState = currentState;
  return is_pressed;
}

