#include "header.h"

FeedingMode getFeedingMode() {
  return (digitalRead(SWITCH_PIN) == LOW) ? MANUAL : SCHEDULED;
}

void addFoodToBowl() {
  // implement
  Serial.println("Food added to bowl!");
}
