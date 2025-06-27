#include "header.h"

FeedingMode getFeedingMode() {
  int switchState = digitalRead(SWITCH_PIN);
  return (switchState == HIGH) ? MANUAL : SCHEDULED;
}