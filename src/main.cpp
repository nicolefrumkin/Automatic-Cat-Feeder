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
#include <Arduino.h>
#include "hardware.h"

void setup() {
    initializeHardware();
    initializeOLED();
    initializeServo();
    
    // Test basic functionality
    blinkStatusLED(3);  // 3 blinks to show system ready
}

void loop() {
    // Simple test loop
    delay(1000);
}
