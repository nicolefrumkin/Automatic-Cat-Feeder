#include "header.h"

FeedingMode getFeedingMode() {
  return (digitalRead(SWITCH_PIN) == HIGH) ? MANUAL : SCHEDULED;
}