#include <Arduino.h>
#include "config.h"
#include "hardware.h"
#include "sensors.h"

// Memory addresses
#define ADDR_MAGIC 0                    // 2 bytes - Magic number for validation
#define ADDR_VERSION 2                  // 1 byte - Data format version
#define ADDR_SETTINGS 3                 // 20 bytes - System settings
#define ADDR_FEEDING_COUNT 23           // 2 bytes - Total number of feeding events
#define ADDR_FEEDING_EVENTS 25          // 400 bytes - Feeding event log (50 events max)

// Global variables
static SystemSettings currentSettings;
static FeedingEvent feedingLog[50];    // Store up to 50 feeding events
static int feedingEventCount = 0;
static bool dataLoaded = false;
static unsigned long lastSaveTime = 0;
static const unsigned long AUTO_SAVE_INTERVAL = 300000; // Auto-save every 5 minutes

// Forward declarations for functions defined later
void initializeDefaultSettings();
uint8_t calculateChecksum(SystemSettings* settings);
bool validateSettings(SystemSettings* settings);
void compactFeedingLog();
String modeCodeToString(uint8_t code);
uint8_t stringToModeCode(String mode);

// Helper functions first
void initializeDefaultSettings() {
    currentSettings.defaultPortion = 50;              // 50g default portion
    currentSettings.feedingInterval = 30000;          // 30 seconds between feeds
    currentSettings.bowlFullThreshold = 80.0;         // 80g = full bowl
    currentSettings.bowlEmptyThreshold = 5.0;         // 5g = empty bowl
    currentSettings.tankLowThreshold = 0.25;          // 25% = low tank
    currentSettings.adaptiveFeedingEnabled = true;    // Enable adaptive feeding
    currentSettings.dayStartTime = 0;                 // Start of day cycle
    currentSettings.checksum = calculateChecksum(&currentSettings);
    
    Serial.println("Default settings initialized");
}

uint8_t calculateChecksum(SystemSettings* settings) {
    uint8_t checksum = 0;
    uint8_t* ptr = (uint8_t*)settings;
    for (int i = 0; i < sizeof(SystemSettings) - 1; i++) { // -1 to exclude checksum field
        checksum ^= ptr[i];
    }
    return checksum;
}

bool validateSettings(SystemSettings* settings) {
    // Check ranges
    if (settings->defaultPortion < 10 || settings->defaultPortion > 200) return false;
    if (settings->feedingInterval < 1000 || settings->feedingInterval > 3600000) return false;
    if (settings->bowlFullThreshold < 10 || settings->bowlFullThreshold > 500) return false;
    if (settings->bowlEmptyThreshold < 0 || settings->bowlEmptyThreshold > 50) return false;
    if (settings->tankLowThreshold < 0.05 || settings->tankLowThreshold > 0.9) return false;
    
    // Check checksum
    uint8_t calculatedChecksum = calculateChecksum(settings);
    return (calculatedChecksum == settings->checksum);
}

void compactFeedingLog() {
    Serial.println("Compacting feeding log (removing oldest entries)...");
    
    // Keep the most recent 40 events, remove the oldest 10
    for (int i = 0; i < 40; i++) {
        feedingLog[i] = feedingLog[i + 10];
    }
    
    feedingEventCount = 40;
    Serial.println("Feeding log compacted");
}

String modeCodeToString(uint8_t code) {
    switch (code) {
        case 0: return "MANUAL";
        case 1: return "SCHEDULED";
        case 2: return "ADAPTIVE";
        default: return "UNKNOWN";
    }
}

uint8_t stringToModeCode(String mode) {
    if (mode == "MANUAL") return 0;
    if (mode == "SCHEDULED") return 1;
    if (mode == "ADAPTIVE") return 2;
    return 0; // Default to manual
}

int getFeedingCount24h() {
    unsigned long currentTime = millis();
    unsigned long dayAgo = currentTime - (24UL * 60UL * 60UL * 1000UL); // 24 hours ago
    
    int count = 0;
    for (int i = 0; i < feedingEventCount; i++) {
        if (feedingLog[i].timestamp >= dayAgo) {
            count++;
        }
    }
    
    return count;
}

float getTotalFoodConsumed24h() {
    unsigned long currentTime = millis();
    unsigned long dayAgo = currentTime - (24UL * 60UL * 60UL * 1000UL); // 24 hours ago
    
    float total = 0.0;
    for (int i = 0; i < feedingEventCount; i++) {
        if (feedingLog[i].timestamp >= dayAgo && feedingLog[i].consumed > 0) {
            total += feedingLog[i].consumed;
        }
    }
    
    return total;
}

// Main interface functions
void initializeDataManager() {
    Serial.println("=== INITIALIZING DATA MANAGER ===");
    
    initializeDefaultSettings();

    Serial.println("Data manager initialized successfully!");
    Serial.print("Feeding events in log: ");
    Serial.println(feedingEventCount);
}

void logFeedingEvent(String mode, int quantity) {
    Serial.println("=== LOGGING FEEDING EVENT ===");
    
    // Ensure we have space in the log
    if (feedingEventCount >= 50) {
        compactFeedingLog(); // Remove oldest entries
    }
    
    // Create new feeding event
    FeedingEvent* event = &feedingLog[feedingEventCount];
    event->timestamp = millis();
    event->mode = mode;
    event->modeCode = stringToModeCode(mode);
    event->quantity = quantity;
    event->bowlWeightBefore = readBowlWeight(); // Get current bowl weight
    event->bowlWeightAfter = 0.0; // Will be updated later
    event->consumed = 0.0; // Will be calculated later
    event->eatingDuration = 0; // Will be tracked separately
    
    feedingEventCount++;
    
    Serial.print("Logged feeding event #");
    Serial.println(feedingEventCount);
    Serial.print("Mode: ");
    Serial.println(mode);
    Serial.print("Quantity: ");
    Serial.print(quantity);
    Serial.println("g");
    Serial.print("Bowl weight before: ");
    Serial.print(event->bowlWeightBefore);
    Serial.println("g");
    
    Serial.println("Event logged successfully!");
}

void updateLastFeedingEvent(float bowlWeightAfter, float consumed, unsigned long eatingDuration) {
    if (feedingEventCount > 0) {
        FeedingEvent* lastEvent = &feedingLog[feedingEventCount - 1];
        lastEvent->bowlWeightAfter = bowlWeightAfter;
        lastEvent->consumed = consumed;
        lastEvent->eatingDuration = eatingDuration;
        
        Serial.println("Updated last feeding event with consumption data");
        Serial.print("Bowl weight after: ");
        Serial.print(bowlWeightAfter);
        Serial.println("g");
        Serial.print("Consumed: ");
        Serial.print(consumed);
        Serial.println("g");
        Serial.print("Eating duration: ");
        Serial.print(eatingDuration / 1000);
        Serial.println("s");
    }
}

void printFeedingHistory() {
    Serial.println("╔══════════════════════════════════════════════════════════╗");
    Serial.println("║                    FEEDING HISTORY                       ║");
    Serial.println("╚══════════════════════════════════════════════════════════╝");
    
    if (feedingEventCount == 0) {
        Serial.println("No feeding events recorded yet.");
        return;
    }
    
    Serial.println("Recent feeding events (newest first):");
    Serial.println("Time     | Mode      | Qty | Before | After | Consumed | Duration");
    Serial.println("---------+-----------+-----+--------+-------+----------+---------");
    
    // Display events in reverse order (newest first)
    for (int i = feedingEventCount - 1; i >= 0 && i >= feedingEventCount - 10; i--) {
        FeedingEvent* event = &feedingLog[i];
        
        // Format timestamp (show time since event)
        unsigned long timeSince = millis() - event->timestamp;
        String timeStr;
        if (timeSince < 60000) {
            timeStr = String(timeSince / 1000) + "s ago";
        } else if (timeSince < 3600000) {
            timeStr = String(timeSince / 60000) + "m ago";
        } else {
            timeStr = String(timeSince / 3600000) + "h ago";
        }
        
        // Pad time string to 8 characters
        while (timeStr.length() < 8) timeStr += " ";
        
        Serial.print(timeStr);
        Serial.print(" | ");
        
        // Mode (9 characters)
        String modeStr = modeCodeToString(event->modeCode);
        while (modeStr.length() < 9) modeStr += " ";
        Serial.print(modeStr);
        Serial.print(" | ");
        
        // Quantity (3 characters)
        String qtyStr = String(event->quantity) + "g";
        while (qtyStr.length() < 3) qtyStr = " " + qtyStr;
        Serial.print(qtyStr);
        Serial.print(" | ");
        
        // Before weight (6 characters)
        String beforeStr = String(event->bowlWeightBefore, 1) + "g";
        while (beforeStr.length() < 6) beforeStr = " " + beforeStr;
        Serial.print(beforeStr);
        Serial.print(" | ");
        
        // After weight (5 characters)
        String afterStr = String(event->bowlWeightAfter, 1) + "g";
        while (afterStr.length() < 5) afterStr = " " + afterStr;
        Serial.print(afterStr);
        Serial.print(" | ");
        
        // Consumed (8 characters)
        String consumedStr = String(event->consumed, 1) + "g";
        while (consumedStr.length() < 8) consumedStr = " " + consumedStr;
        Serial.print(consumedStr);
        Serial.print(" | ");
        
        // Duration (8 characters)
        String durationStr = String(event->eatingDuration / 1000) + "s";
        while (durationStr.length() < 8) durationStr = " " + durationStr;
        Serial.println(durationStr);
    }
    
    Serial.println();
    
    // Summary statistics
    Serial.println("=== FEEDING STATISTICS ===");
    Serial.print("Total events: ");
    Serial.println(feedingEventCount);
    
    Serial.print("Events in last 24h: ");
    Serial.println(getFeedingCount24h());
    
    Serial.print("Food consumed (24h): ");
    Serial.print(getTotalFoodConsumed24h());
    Serial.println("g");
    
    // Calculate average values
    if (feedingEventCount > 0) {
        float avgPortion = 0;
        float avgConsumed = 0;
        int validConsumptionEvents = 0;
        
        for (int i = 0; i < feedingEventCount; i++) {
            avgPortion += feedingLog[i].quantity;
            if (feedingLog[i].consumed > 0) {
                avgConsumed += feedingLog[i].consumed;
                validConsumptionEvents++;
            }
        }
        
        avgPortion /= feedingEventCount;
        if (validConsumptionEvents > 0) {
            avgConsumed /= validConsumptionEvents;
        }
        
        Serial.print("Average portion size: ");
        Serial.print(avgPortion, 1);
        Serial.println("g");
        
        Serial.print("Average consumption: ");
        Serial.print(avgConsumed, 1);
        Serial.println("g");
        
        if (validConsumptionEvents > 0) {
            float consumptionRate = (avgConsumed / avgPortion) * 100;
            Serial.print("Consumption rate: ");
            Serial.print(consumptionRate, 1);
            Serial.println("%");
        }
    }
}



void updateSetting(String setting, float value) {
    if (setting == "defaultPortion") {
        currentSettings.defaultPortion = (int)value;
    } else if (setting == "feedingInterval") {
        currentSettings.feedingInterval = (unsigned long)value;
    } else if (setting == "bowlFullThreshold") {
        currentSettings.bowlFullThreshold = value;
    } else if (setting == "bowlEmptyThreshold") {
        currentSettings.bowlEmptyThreshold = value;
    } else if (setting == "tankLowThreshold") {
        currentSettings.tankLowThreshold = value;
    } else if (setting == "adaptiveFeedingEnabled") {
        currentSettings.adaptiveFeedingEnabled = (value != 0);
    }
    
    Serial.print("Updated setting: ");
    Serial.print(setting);
    Serial.print(" = ");
    Serial.println(value);
}