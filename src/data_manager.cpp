#include <Arduino.h>
#include "config.h"
#include "hardware.h"
#include "sensors.h"
#include <EEPROM.h>

// EEPROM Memory Layout
#define EEPROM_SIZE 512
#define EEPROM_MAGIC_NUMBER 0xCAFE
#define EEPROM_VERSION 1

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
void saveFeedingData();
void loadFeedingData();
void saveToEEPROM();
void loadFromEEPROM();

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

void saveFeedingData() {
    Serial.println("Saving feeding data to EEPROM...");
    
    // Save feeding event count
    EEPROM.writeUShort(ADDR_FEEDING_COUNT, feedingEventCount);
    
    // Save feeding events (only store essential data to save space)
    int addr = ADDR_FEEDING_EVENTS;
    for (int i = 0; i < feedingEventCount && i < 50; i++) {
        FeedingEvent* event = &feedingLog[i];
        
        // Store timestamp (4 bytes)
        EEPROM.writeULong(addr, event->timestamp);
        addr += 4;
        
        // Store mode code (1 byte)
        EEPROM.writeByte(addr, event->modeCode);
        addr += 1;
        
        // Store quantity (2 bytes)
        EEPROM.writeUShort(addr, event->quantity);
        addr += 2;
        
        // Store consumed amount as int (2 bytes, multiply by 10 for decimal precision)
        EEPROM.writeUShort(addr, (int)(event->consumed * 10));
        addr += 2;
        
        // Total: 9 bytes per event, max 50 events = 450 bytes (fits in allocated space)
    }
    
    EEPROM.commit();
    lastSaveTime = millis();
    
    Serial.print("Saved ");
    Serial.print(feedingEventCount);
    Serial.println(" feeding events to EEPROM");
}

void loadFeedingData() {
    Serial.println("Loading feeding data from EEPROM...");
    
    // Load feeding event count
    feedingEventCount = EEPROM.readUShort(ADDR_FEEDING_COUNT);
    
    if (feedingEventCount > 50) {
        Serial.println("Invalid feeding count in EEPROM, resetting...");
        feedingEventCount = 0;
        return;
    }
    
    // Load feeding events
    int addr = ADDR_FEEDING_EVENTS;
    for (int i = 0; i < feedingEventCount; i++) {
        FeedingEvent* event = &feedingLog[i];
        
        // Load timestamp
        event->timestamp = EEPROM.readULong(addr);
        addr += 4;
        
        // Load mode code
        event->modeCode = EEPROM.readByte(addr);
        addr += 1;
        
        // Load quantity
        event->quantity = EEPROM.readUShort(addr);
        addr += 2;
        
        // Load consumed amount
        event->consumed = EEPROM.readUShort(addr) / 10.0;
        addr += 2;
        
        // Reconstruct mode string
        event->mode = modeCodeToString(event->modeCode);
        
        // Set default values for fields not stored
        event->bowlWeightBefore = 0.0;
        event->bowlWeightAfter = 0.0;
        event->eatingDuration = 0;
    }
    
    Serial.print("Loaded ");
    Serial.print(feedingEventCount);
    Serial.println(" feeding events from EEPROM");
}

void saveToEEPROM() {
    Serial.println("=== SAVING ALL DATA TO EEPROM ===");
    
    // Write magic number and version
    EEPROM.writeUShort(ADDR_MAGIC, EEPROM_MAGIC_NUMBER);
    EEPROM.writeByte(ADDR_VERSION, EEPROM_VERSION);
    
    // Calculate and save settings checksum
    currentSettings.checksum = calculateChecksum(&currentSettings);
    
    // Save settings
    int addr = ADDR_SETTINGS;
    EEPROM.writeInt(addr, currentSettings.defaultPortion);
    addr += 4;
    EEPROM.writeULong(addr, currentSettings.feedingInterval);
    addr += 4;
    EEPROM.writeFloat(addr, currentSettings.bowlFullThreshold);
    addr += 4;
    EEPROM.writeFloat(addr, currentSettings.bowlEmptyThreshold);
    addr += 4;
    EEPROM.writeFloat(addr, currentSettings.tankLowThreshold);
    addr += 4;
    EEPROM.writeByte(addr, currentSettings.adaptiveFeedingEnabled ? 1 : 0);
    addr += 1;
    EEPROM.writeByte(addr, currentSettings.checksum);
    
    // Save feeding data
    saveFeedingData();
    
    EEPROM.commit();
    Serial.println("All data saved to EEPROM successfully!");
}

void loadFromEEPROM() {
    Serial.println("=== LOADING DATA FROM EEPROM ===");
    
    // Check magic number
    uint16_t magic = EEPROM.readUShort(ADDR_MAGIC);
    if (magic != EEPROM_MAGIC_NUMBER) {
        Serial.println("Invalid magic number, EEPROM data not valid");
        dataLoaded = false;
        return;
    }
    
    // Check version
    uint8_t version = EEPROM.readByte(ADDR_VERSION);
    if (version != EEPROM_VERSION) {
        Serial.print("Version mismatch: expected ");
        Serial.print(EEPROM_VERSION);
        Serial.print(", found ");
        Serial.println(version);
        dataLoaded = false;
        return;
    }
    
    // Load settings
    int addr = ADDR_SETTINGS;
    currentSettings.defaultPortion = EEPROM.readInt(addr);
    addr += 4;
    currentSettings.feedingInterval = EEPROM.readULong(addr);
    addr += 4;
    currentSettings.bowlFullThreshold = EEPROM.readFloat(addr);
    addr += 4;
    currentSettings.bowlEmptyThreshold = EEPROM.readFloat(addr);
    addr += 4;
    currentSettings.tankLowThreshold = EEPROM.readFloat(addr);
    addr += 4;
    currentSettings.adaptiveFeedingEnabled = EEPROM.readByte(addr) == 1;
    addr += 1;
    currentSettings.checksum = EEPROM.readByte(addr);
    
    // Validate settings
    if (!validateSettings(&currentSettings)) {
        Serial.println("Settings validation failed, using defaults");
        initializeDefaultSettings();
        dataLoaded = false;
        return;
    }
    
    // Load feeding data
    loadFeedingData();
    
    dataLoaded = true;
    Serial.println("Data loaded from EEPROM successfully!");
    
    // Print loaded settings
    Serial.println("Loaded settings:");
    Serial.print("  Default portion: ");
    Serial.print(currentSettings.defaultPortion);
    Serial.println("g");
    Serial.print("  Feeding interval: ");
    Serial.print(currentSettings.feedingInterval / 1000);
    Serial.println("s");
    Serial.print("  Adaptive feeding: ");
    Serial.println(currentSettings.adaptiveFeedingEnabled ? "ENABLED" : "DISABLED");
}

// Main interface functions
void initializeDataManager() {
    Serial.println("=== INITIALIZING DATA MANAGER ===");
    
    // Initialize EEPROM
    EEPROM.begin(EEPROM_SIZE);
    
    // Try to load existing data
    loadFromEEPROM();
    
    if (!dataLoaded) {
        Serial.println("No valid data found, initializing defaults...");
        initializeDefaultSettings();
        saveToEEPROM();
    }
    
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
    
    // Auto-save if it's been a while
    if (millis() - lastSaveTime > AUTO_SAVE_INTERVAL) {
        saveFeedingData();
    }
    
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

void clearMemory() {
    Serial.println("=== CLEARING ALL MEMORY ===");
    
    // Clear EEPROM
    for (int i = 0; i < EEPROM_SIZE; i++) {
        EEPROM.writeByte(i, 0xFF);
    }
    EEPROM.commit();
    
    // Clear feeding log
    feedingEventCount = 0;
    
    // Reset to default settings
    initializeDefaultSettings();
    
    Serial.println("All memory cleared and reset to defaults!");
}

void backupSettings() {
    Serial.println("=== BACKING UP SETTINGS ===");
    
    Serial.println("Current settings backup:");
    Serial.print("defaultPortion=");
    Serial.println(currentSettings.defaultPortion);
    Serial.print("feedingInterval=");
    Serial.println(currentSettings.feedingInterval);
    Serial.print("bowlFullThreshold=");
    Serial.println(currentSettings.bowlFullThreshold);
    Serial.print("bowlEmptyThreshold=");
    Serial.println(currentSettings.bowlEmptyThreshold);
    Serial.print("tankLowThreshold=");
    Serial.println(currentSettings.tankLowThreshold);
    Serial.print("adaptiveFeedingEnabled=");
    Serial.println(currentSettings.adaptiveFeedingEnabled);
    
    Serial.println("Settings backed up to serial output!");
    Serial.println("Copy the above values to restore settings later");
}

void loadSettings() {
    Serial.println("=== LOADING SETTINGS ===");
    Serial.println("Current settings:");
    Serial.print("  Default portion: ");
    Serial.print(currentSettings.defaultPortion);
    Serial.println("g");
    Serial.print("  Feeding interval: ");
    Serial.print(currentSettings.feedingInterval / 1000);
    Serial.println("s");
    Serial.print("  Bowl full threshold: ");
    Serial.print(currentSettings.bowlFullThreshold);
    Serial.println("g");
    Serial.print("  Bowl empty threshold: ");
    Serial.print(currentSettings.bowlEmptyThreshold);
    Serial.println("g");
    Serial.print("  Tank low threshold: ");
    Serial.print(currentSettings.tankLowThreshold * 100);
    Serial.println("%");
    Serial.print("  Adaptive feeding: ");
    Serial.println(currentSettings.adaptiveFeedingEnabled ? "ENABLED" : "DISABLED");
}

// Public accessor functions for settings
SystemSettings* getSettings() {
    return &currentSettings;
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
    
    // Auto-save after setting update
    saveToEEPROM();
    
    Serial.print("Updated setting: ");
    Serial.print(setting);
    Serial.print(" = ");
    Serial.println(value);
}