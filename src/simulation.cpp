// ============ simulation.cpp ============
#include <Arduino.h>
#include "config.h"
#include "sensors.h"

static bool eatingInProgress = false;
static unsigned long eatingStartTime = 0;
static float foodToEat = 0;

void startEatingCycle() {
    if (!eatingInProgress && readBowlWeight() > 5.0) {
        eatingInProgress = true;
        eatingStartTime = millis();
        foodToEat = random(10, 30); // Eat 10-30g
        
        Serial.println("🐱 Starting eating simulation");
        Serial.print("Will eat approximately ");
        Serial.print(foodToEat);
        Serial.println("g");
    }
}

void updateEatingProgress() {
    if (eatingInProgress) {
        unsigned long elapsed = millis() - eatingStartTime;
        
        // Eat for 15-45 seconds
        if (elapsed < 45000 && readBowlWeight() > 1.0) {
            // Reduce bowl weight gradually
            // This would be handled by the sensor simulation
        } else {
            eatingInProgress = false;
            Serial.println("🐱 Finished eating simulation");
        }
    }
}

bool isEatingComplete() {
    return !eatingInProgress;
}

void randomizeEatingBehavior() {
    // Randomly start eating cycles
    if (!eatingInProgress && random(1000) < 2) { // 0.2% chance per call
        startEatingCycle();
    }
    
    updateEatingProgress();
}

