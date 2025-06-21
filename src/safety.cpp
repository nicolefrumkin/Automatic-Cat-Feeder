#include <Arduino.h>
#include "config.h"
#include "hardware.h"
#include "sensors.h"
#include "feeding.h"
#include "display.h"

bool performSafetyChecks() {
    Serial.println("=== PERFORMING SAFETY CHECKS ===");
    
    bool safe = true;
    
    // Check 1: System not in emergency mode
    if (isSystemInEmergencyMode()) {
        Serial.println("SAFETY FAIL: System in emergency mode");
        safe = false;
    }
    
    // Check 2: Servo is ready
    if (!isServoReady()) {
        Serial.println("SAFETY FAIL: Servo not ready");
        safe = false;
    }
    
    // Check 3: Bowl not overfull
    if (isBowlFull()) {
        Serial.println("SAFETY WARNING: Bowl is full");
        // Not a complete failure, but worth noting
    }
    
    // Check 4: Tank has food
    if (isTankLow()) {
        Serial.println("SAFETY WARNING: Tank is low");
        // Not a complete failure, but worth noting
    }
    
    // Check 5: Sensor readings are valid
    if (!validateSensorReadings()) {
        Serial.println("SAFETY FAIL: Invalid sensor readings");
        safe = false;
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