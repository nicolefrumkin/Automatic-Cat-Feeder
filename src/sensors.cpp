#include "HX711.h"
#include "header.h"

HX711 bowlScale;
HX711 tankScale;

void initHX711() {
  bowlScale.begin(HX711_BOWL_DT, HX711_BOWL_SCK);
  tankScale.begin(HX711_TANK_DT, HX711_TANK_SCK);
}

int getPortionFromPot() {
  int potValue = analogRead(POT_PIN);
  int portion = map(potValue, 0, 4095, MIN_PORTION, MAX_PORTION);
  return portion;
}

