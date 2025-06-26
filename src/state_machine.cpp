#include <Arduino.h>
#include "config.h"
#include "hardware.h"
#include "sensors.h"
#include "data_manager.h"
#include "adaptive.h"
#include "display.h"
#include "feeding.h"
#include "utils.h"
#include "communication.h"
#include "safety.h"
#include "timing.h"

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

void runManualMode() {
    // Only print this occasionally
    static unsigned long lastModeMsg = 0;
    if (millis() - lastModeMsg > 10000) { // Every 10 seconds
        Serial.println("System running in MANUAL mode - press button to feed");
        lastModeMsg = millis();
    }
}

void runScheduledMode() {
    // Only print this occasionally  
    static unsigned long lastModeMsg = 0;
    if (millis() - lastModeMsg > 10000) { // Every 10 seconds
        Serial.println("System running in SCHEDULED mode");
        lastModeMsg = millis();
    }
    
    // Check if it's time for scheduled feeding
    if (isScheduledFeedTime()) {
        executeScheduledFeed();
    }
}

void performDailyMaintenance() {
    Serial.println("=== DAILY MAINTENANCE ===");
    
    // Check bowl status at end of day
    checkBowlStatusEndOfDay();
    
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

void checkInputs() {
    static bool lastButtonState = false;
    static unsigned long lastManualFeedTime = 0;
    const unsigned long debounceDelay = 200;       
    const unsigned long manualCooldown = 5000;     

    // --- Update feeding mode based on switch ---
    bool switchState = readModeSwitch();
    FeedingMode selectedMode = switchState ? MANUAL : SCHEDULED;
    if (selectedMode != currentMode) {
        setFeedingMode(selectedMode);
    }

    // --- Potentiometer adjustment (LESS FREQUENT) ---
    if (currentMode == MANUAL) {
        static unsigned long lastPotCheck = 0;
        if (millis() - lastPotCheck > 1000) { // Only check every second
            int potValue = readPotentiometer();
            // Don't call adjustPortionSize every time
            static int lastPortion = -1;
            if (abs(potValue - lastPortion) > 3) { // Larger threshold
                adjustPortionSize(potValue);
                lastPortion = potValue;
            }
            lastPotCheck = millis();
        }
    }

    // --- Manual feed button ---
    bool currentButtonState = readFeedButton();
    unsigned long now = millis();

    if (currentButtonState && !lastButtonState) {
        if (now - lastManualFeedTime > manualCooldown) {
            if (currentMode == MANUAL && canDispenseFood()) {
                executeManualFeed();
                lastManualFeedTime = now;
            } else {
                Serial.println("Feeding blocked - check safety conditions");
            }
        }
    }
    lastButtonState = currentButtonState;

    // --- Tank low alert (LESS FREQUENT) ---
    static unsigned long lastTankCheck = 0;
    if (now - lastTankCheck > 30000) { // Check every 30 seconds
        if (isTankLow()) {
            indicateLowFood();
            publishAlert();
        }
        lastTankCheck = now;
    }

    // Sensor safety check 
    static unsigned long lastSensorCheck = 0;
    static int consecutiveSensorFails = 0;
    
    if (now - lastSensorCheck > 30000) { // Only check every 30 seconds!
        consecutiveSensorFails = 0; // Reset on success
        lastSensorCheck = now;
    }
}
