#include "header.h"

// FeedingMode getFeedingMode() {
//   return (digitalRead(SWITCH_PIN) == HIGH) ? MANUAL : SCHEDULED;
// }
int getFeedingMode() {
  return (digitalRead(SWITCH_PIN) == HIGH) ? MANUAL : SCHEDULED;
}