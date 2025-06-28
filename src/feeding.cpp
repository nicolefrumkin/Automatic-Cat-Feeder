#include "header.h"

FeedingMode getFeedingMode() {
  return (digitalRead(SWITCH_PIN) == LOW) ? MANUAL : SCHEDULED;
}

void addFoodToBowl() {
  // checking if bowl will overflow
  if(feeder.portionSize + feeder.bowlLevel >= BOWL_FULL_THRESHOLD) {
    Serial.println("Bowl is already full!");
    feeder.bowlIsFull = true; // Set bowl full flag
    return;
  }
  // checking if there is enough food in the tank
  if(feeder.tankLevel < feeder.portionSize) {
    Serial.println("Not enough food in tank!");
    return;
  }
  // dispensing food
  int servoTime = map(feeder.portionSize, MIN_PORTION, MAX_PORTION, 200, 500); // Map portion size to servo time
  // fixme - servo isnt working properly
  feederServo.write(30); // Move servo to dispense food - angle relevant to portion size
  delay(servoTime); // Wait for servo to move
  feederServo.write(90); // Reset servo position

  feeder.bowlIsFull = false;
  feeder.feedEventsToday++;
  feeder.bowlLevel += feeder.portionSize; // Update bowl level
  feeder.tankLevel -= feeder.portionSize; // Update tank level
  Serial.println("Food added to bowl!");
}

void resetFeederForNextDay() {
  feeder.feedEventsToday = 0;       // Reset daily feed counter
  feeder.dayCycle += 1;            // Increment day cycle
  feeder.lastFeedTime = millis();   // Reset or update to "start of day"
  // add all the health checks here and check abnormal behavior
  Serial.println("Feeder reset for new day.");
}
