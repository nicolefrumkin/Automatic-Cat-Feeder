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

    if (command == "choose feeding time") {
      Serial.println("Please enter the feeding time in HH:MM format:");
      String timeInput = Serial.readStringUntil('\n');
      timeInput.trim();
      // parseTime(timeInput); // Implement this function to handle time input
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

void feederInit() {
  feeder.Mode = getFeedingMode();
  feeder.portionSize = getPortionFromPot();
  feeder.bowlLevel = 0; // Initialize bowl level
  feeder.tankLevel = FULL_TANK; // Initialize tank level 2KG
  feeder.feedEventsToday = 0; // Reset feed events count
  feeder.bowlIsFull = false; // Assume bowl is not full initially
  feeder.tankLow = false; // Assume tank is not low initially
  feeder.dayCycle = 0; // Initialize day cycle (0-23)
  feeder.lastFeedTime = millis(); // Set last feed time to current time
  feeder.feedInterval = DEFAULT_FEED_RATE; // Set default feed interval - 6 times per day
}

void functionsUpdate() {
  FeedingMode nextMode = getFeedingMode();
  if( nextMode == SCHEDULED && feeder.Mode == MANUAL) {
    feeder.lastFeedTime = millis(); // Reset last feed time when switching to scheduled mode
  }
  feeder.Mode = nextMode;
  feeder.portionSize = getPortionFromPot();
  displayFunctionScreen();
  handleSerialCommands(); // checking serial every second
}