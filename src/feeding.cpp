#include <Arduino.h>
#include "config.h"
#include "hardware.h"
#include "sensors.h"
#include "data_manager.h"

// Forward declarations for functions used before they're defined
bool canDispenseFood();
int calculatePortion();

// Global variables for feeding logic
static int lastPortionSize = 50;  // Default portion size
static unsigned long lastFeedTime = 0;
static const unsigned long MIN_FEED_INTERVAL = 30000;  // 30 seconds minimum between feeds

// Helper function implementations
int calculatePortion() {
    // Get portion from potentiometer
    int portion = readPotentiometer();
    
    // Ensure it's within valid range
    portion = constrain(portion, MIN_PORTION, MAX_PORTION);
    
    return portion;
}

bool canDispenseFood() {
    // Safety check 1: Minimum time between feeds
    if (millis() - lastFeedTime < MIN_FEED_INTERVAL) {
        Serial.println("SAFETY: Too soon since last feed");
        return false;
    }
    
    // Safety check 2: Servo must be ready
    if (!isServoReady()) {
        Serial.println("SAFETY: Servo not ready");
        return false;
    }
    
    // Safety check 3: System not in emergency mode
    if (isSystemInEmergencyMode()) {
        Serial.println("SAFETY: System in emergency mode");
        return false;
    }
    
    // Safety check 4: Bowl not full
    if (isBowlFull()) {
        Serial.println("SAFETY: Bowl is full - prevent overfeeding");
        preventDispenseIfFull();
        return false;
    }
    
    // Safety check 5: Tank not empty
    if (isTankLow()) {
        Serial.println("SAFETY: Food tank is low - refill needed");
        return false;
    }
    
    // Safety check 6: Valid portion size
    int portion = calculatePortion();
    if (portion < MIN_PORTION || portion > MAX_PORTION) {
        Serial.println("SAFETY: Invalid portion size");
        return false;
    }
    
    return true;
}

// Core feeding operations
void dispenseFood(int amount) {
    Serial.println("=== DISPENSING FOOD ===");
    Serial.print("Amount: ");
    Serial.print(amount);
    Serial.println("g");
    
    if (!canDispenseFood()) {
        Serial.println("ERROR: Cannot dispense food - safety check failed");
        return;
    }
    
    // Get bowl weight before feeding for logging
    float bowlWeightBefore = readBowlWeight();
    
    // Calculate servo open angle based on amount (30g = 30°, 75g = 75°)
    int servoAngle = map(amount, MIN_PORTION, MAX_PORTION, 30, 75);
    servoAngle = constrain(servoAngle, 30, 75);
    
    Serial.print("Opening feeder gate to ");
    Serial.print(servoAngle);
    Serial.println(" degrees");
    
    // Open gate
    moveServoToPosition(servoAngle);
    delay(2000);  // Dispense time (2 seconds)
    
    // Close gate
    moveServoToPosition(0);
    
    // Update tracking variables
    lastFeedTime = millis();
    lastPortionSize = amount;
    
    // Get bowl weight after feeding
    delay(1000); // Wait for food to settle
    float bowlWeightAfter = readBowlWeight();
    
    Serial.println("Food dispensing complete!");
    indicateFeedingSuccess();
    
    Serial.print("Bowl weight before: ");
    Serial.print(bowlWeightBefore);
    Serial.println("g");
    Serial.print("Bowl weight after: ");
    Serial.print(bowlWeightAfter);
    Serial.println("g");
    
    // Log the feeding event
    logFeedingEvent("DISPENSE", amount);
}

void executeManualFeed() {
    Serial.println("=== MANUAL FEED REQUESTED ===");
    
    int portion = calculatePortion();
    Serial.print("Manual portion: ");
    Serial.print(portion);
    Serial.println("g");
    
    dispenseFood(portion);
    
    // Log the feeding event
    logFeedingEvent("MANUAL", portion);
}

void executeScheduledFeed() {
    Serial.println("=== SCHEDULED FEED ===");
    
    // For now, use a fixed scheduled portion (can be made adaptive later)
    int scheduledPortion = 50;  // 50g default
    
    Serial.print("Scheduled portion: ");
    Serial.print(scheduledPortion);
    Serial.println("g");
    
    dispenseFood(scheduledPortion);
    
    // Log the feeding event
    logFeedingEvent("SCHEDULED", scheduledPortion);
}

void validateFeedingConditions() {
    Serial.println("=== FEEDING VALIDATION ===");
    
    Serial.print("Time since last feed: ");
    Serial.print((millis() - lastFeedTime) / 1000);
    Serial.println(" seconds");
    
    Serial.print("Servo ready: ");
    Serial.println(isServoReady() ? "YES" : "NO");
    
    Serial.print("Emergency mode: ");
    Serial.println(isSystemInEmergencyMode() ? "YES" : "NO");
    
    Serial.print("Current portion setting: ");
    Serial.print(calculatePortion());
    Serial.println("g");
    
    Serial.print("Can dispense: ");
    Serial.println(canDispenseFood() ? "YES" : "NO");
    
    // Additional sensor checks
    Serial.print("Bowl full: ");
    Serial.println(isBowlFull() ? "YES" : "NO");
    
    Serial.print("Tank low: ");
    Serial.println(isTankLow() ? "YES" : "NO");
    
    Serial.print("Bowl weight: ");
    Serial.print(readBowlWeight());
    Serial.println("g");
    
    Serial.print("Tank weight: ");
    Serial.print(readTankWeight());
    Serial.println("g");
}

void processFeedingRequest() {
    // Determine feeding mode and execute appropriate feeding
    FeedingMode mode = readModeSwitch() ? MANUAL : SCHEDULED;
    
    if (mode == MANUAL) {
        executeManualFeed();
    } else {
        // Only feed if it's the scheduled time (implement timing logic later)
        executeScheduledFeed();
    }
}

// Portion control functions
int getMinPortion() {
    return MIN_PORTION;
}

int getMaxPortion() {
    return MAX_PORTION;
}

void adjustPortionSize(int change) {
    // This could be used for adaptive feeding later
    Serial.print("Adjusting portion size by ");
    Serial.print(change);
    Serial.println("g");
    
    // For now, just log the change
    // In adaptive implementation, this would modify feeding amounts
}

// Helper functions
void openFeederGate(int degrees) {
    degrees = constrain(degrees, 0, 90);
    moveServoToPosition(degrees);
}

void closeFeederGate() {
    moveServoToPosition(0);
}

void calibrateServo() {
    Serial.println("=== SERVO CALIBRATION ===");
    
    Serial.println("Testing servo range...");
    
    // Test closed position
    Serial.println("Moving to closed position (0°)");
    moveServoToPosition(0);
    delay(1000);
    
    // Test small opening
    Serial.println("Moving to small opening (30°)");
    moveServoToPosition(30);
    delay(1000);
    
    // Test medium opening
    Serial.println("Moving to medium opening (45°)");
    moveServoToPosition(45);
    delay(1000);
    
    // Test full opening
    Serial.println("Moving to full opening (75°)");
    moveServoToPosition(75);
    delay(1000);
    
    // Return to closed
    Serial.println("Returning to closed position");
    moveServoToPosition(0);
    
    Serial.println("Servo calibration complete!");
}