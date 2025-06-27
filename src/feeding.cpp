#include "header.h"

FeedingMode getFeedingMode() {
  return (digitalRead(SWITCH_PIN) == LOW) ? MANUAL : SCHEDULED;
}