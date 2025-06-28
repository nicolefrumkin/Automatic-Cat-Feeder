#include "header.h"

FeedingMode getFeedingMode()
{
  return (digitalRead(SWITCH_PIN) == LOW) ? MANUAL : SCHEDULED;
}

void addFoodToBowl()
{
  // checking if bowl will overflow
  if (feeder.portionSize + feeder.bowlLevel >= BOWL_FULL_THRESHOLD)
  {
    Serial.println("Bowl is already full!");
    feeder.bowlIsFull = true; // Set bowl full flag
    return;
  }
  // checking if there is enough food in the tank
  if (feeder.tankLevel < feeder.portionSize)
  {
    Serial.println("Not enough food in tank!");
    return;
  }
  // dispensing food
  int servoTime = map(feeder.portionSize, MIN_PORTION, MAX_PORTION, 200, 500); // Map portion size to servo time
  // fixme - servo isnt working properly
  feederServo.write(30); // Move servo to dispense food - angle relevant to portion size
  delay(servoTime);      // Wait for servo to move
  feederServo.write(90); // Reset servo position

  feeder.bowlIsFull = false;
  feeder.feedEventsToday++;
  feeder.bowlLevel += feeder.portionSize; // Update bowl level
  feeder.tankLevel -= feeder.portionSize; // Update tank level
  feeder.lastFeedTime = millis();
  Serial.println("Food added to bowl!");
  logFeedingEvent();
}

void resetFeederForNextDay()
{
  feeder.feedEventsToday = 0; // Reset daily feed counter
  feeder.dayCycle += 1;       // Increment day cycle
  // add all the health checks here and check abnormal behavior
  if (feeder.bowlLevel >= BOWL_FULL_THRESHOLD)
  {
    feeder.portionSize = max(feeder.portionSize - 1, MIN_PORTION);
    Serial.println("Bowl was full all day – decreasing portion by 1g.\n");
  }
  if (feeder.bowlEmptyTime > 0 &&
      (feeder.bowlEmptyTime - feeder.lastFeedTime) <= 5000)
  {
    feeder.portionSize = min(feeder.portionSize + 1, MAX_PORTION);
    Serial.println("Bowl emptied quickly – increasing portion by 1g.");
  }
  feeder.bowlEmptyTime = 0;
  checkEatingTrendAndAlert();
  Serial.println("Feeder reset for new day.\n");
}

void simulateEating()
{
  const int eatingRate = 10;                  // grams per eating event
  const unsigned long eatingInterval = 15000; // 15 seconds between bites
  String sounds[] = {"nom nom", "munch munch", "slurp!", "nyam nyam", "crunch crunch"};

  static unsigned long lastEatingTime = 0;
  unsigned long currentMillis = millis();

  // Only simulate eating if the bowl has kibble
  if (feeder.bowlLevel > 0 && currentMillis - lastEatingTime >= eatingInterval)
  {
    feeder.bowlLevel -= eatingRate;

    if (feeder.bowlLevel < 0)
    {
      feeder.bowlLevel = 0; // Don’t go negative
    }

    lastEatingTime = currentMillis;
    Serial.println("🐱 " + sounds[random(0, 5)]);
    Serial.println("Bowl now has " + String(feeder.bowlLevel) + "g\n");
  }
  if (feeder.bowlLevel <= 0 && feeder.bowlEmptyTime == 0)
  {
    feeder.bowlEmptyTime = millis();
  }
}

void logFeedingEvent()
{
  String timestamp = formatTime(millis()); // or your RTC time
  String modeStr = feeder.Mode == MANUAL ? "MANUAL" : "SCHEDULED";
  int quantity = feeder.portionSize;

  Serial.println("Feeding: " + timestamp + ", " + modeStr + ", " + String(quantity) + "g\n");
}

void checkEatingTrendAndAlert()
{
  int minDailyIntake = feeder.portionSize / 2;

  if (feeder.feedEventsToday == 0 || feeder.bowlLevel < minDailyIntake)
  {
    displayAlert();
  }
}
