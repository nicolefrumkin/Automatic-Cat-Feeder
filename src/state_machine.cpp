#include <Arduino.h>
#include "config.h"
#include "hardware.h"
#include "sensors.h"
#include "data_manager.h"
#include "adaptive.h"
#include "display.h"

// System state management
static FeedingMode currentMode = MANUAL;
static SystemState currentState = IDLE;

void setFeedingMode(FeedingMode mode) {
    currentMode = mode;
    Serial.print("Feeding mode set to: ");
    Serial.println(mode == MANUAL ? "MANUAL" : "SCHEDULED");
}

void setSystemState(SystemState state) {
    currentState = state;
    Serial.print("System state changed to: ");
    switch(state) {
        case IDLE: Serial.println("IDLE"); break;
        case FEEDING: Serial.println("FEEDING"); break;
        case MONITORING: Serial.println("MONITORING"); break;
        case ERROR: Serial.println("ERROR"); break;
    }
    
    // Update LED indication based on state
    indicateSystemStatus(state);
}

FeedingMode getCurrentMode() {
    return currentMode;
}

SystemState getCurrentState() {
    return currentState;
}

void handleStateTransition() {
    // Handle state transitions based on current conditions
    switch(currentState) {
        case IDLE:
            if (isSystemInEmergencyMode()) {
                setSystemState(ERROR);
            } else if (readFeedButton()) {
                setSystemState(FEEDING);
            }
            break;
            
        case FEEDING:
            // Would transition back to IDLE after feeding complete
            break;
            
        case MONITORING:
            // Would transition to IDLE after monitoring period
            break;
            
        case ERROR:
            // Would require manual intervention to clear
            break;
    }
}

void updateSystemState() {
    // Update mode based on switch position
    bool switchState = readModeSwitch();
    FeedingMode newMode = switchState ? MANUAL : SCHEDULED;
    
    if (newMode != currentMode) {
        setFeedingMode(newMode);
    }
    
    // Handle state transitions
    handleStateTransition();
}

void handleModeSwitch() {
    bool switchState = readModeSwitch();
    FeedingMode newMode = switchState ? MANUAL : SCHEDULED;
    setFeedingMode(newMode);
}

void runScheduledMode() {
    Serial.println("Running in scheduled mode");
    // Implementation would check time and feed if scheduled
}

void runManualMode() {
    Serial.println("Running in manual mode");
    // Implementation would respond to button presses
}

void performDailyMaintenance() {
    Serial.println("=== DAILY MAINTENANCE ===");
    
    // Check bowl status at end of day
    checkBowlStatusEndOfDay();
    
    // Save data
    saveToEEPROM();
    
    // Analyze patterns
    analyzeFeedingPatterns();
    
    Serial.println("Daily maintenance complete");
}

void handleCriticalAlerts() {
    if (isTankLow()) {
        Serial.println("CRITICAL: Tank is low!");
        displayLowFoodAlert();
    }
    
    if (detectUnusualBehavior()) {
        Serial.println("CRITICAL: Unusual eating behavior detected!");
        displayHealthAlert();
    }
}