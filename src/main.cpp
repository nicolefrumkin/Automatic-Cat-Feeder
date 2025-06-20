#include "hardware.h"
#include "display.h"
#include "feeding.h"
#include "sensors.h"
#include "communication.h"
#include "adaptive.h"
#include "utils.h"
#include "config.h"
#include "data_manager.h"
#include "safety.h"
#include "simulation.h"
#include "state_machine.h"
#include "timing.h"

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Hello, ESP32!");
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(10); // this speeds up the simulation
}
