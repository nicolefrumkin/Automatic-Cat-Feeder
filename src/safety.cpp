#include <Arduino.h>
#include "config.h"
#include "hardware.h"
#include "sensors.h"
#include "feeding.h"
#include "display.h"

bool performSafetyChecks() {
    Serial.println("=== PERFORMING SAFETY CHECKS ===");
    
    bool safe = true;
    
    // Check : Servo is ready
    if (!isServoReady()) {
        Serial.println("SAFETY FAIL: Servo not ready");
        safe = false;
    }
    
    // Check : Bowl not overfull
    if (isBowlFull()) {
        Serial.println("SAFETY WARNING: Bowl is full");
        // Not a complete failure, but worth noting
    }
    
    // Check : Tank has food
    if (isTankLow()) {
        Serial.println("SAFETY WARNING: Tank is low");
        // Not a complete failure, but worth noting
    }
    
    
    Serial.print("Safety check result: ");
    Serial.println(safe ? "PASSED" : "FAILED");
    
    return safe;
}

void handleFeedingError() {
    Serial.println("=== HANDLING FEEDING ERROR ===");
    
    // Stop any ongoing feeding operations
    closeFeederGate();
    
    // Flash error indication
    indicateFeedingError();
    
    // Display error message
    displayEmergencyMessage("Feeding Error");
    
    // Log the error
    Serial.println("Feeding error handled - system safe");
}