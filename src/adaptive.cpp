#include <Arduino.h>
#include "config.h"
#include "sensors.h"
#include "data_manager.h"
#include "feeding.h"
#include "display.h"

void analyzeFeedingPatterns() {
    Serial.println("=== ANALYZING FEEDING PATTERNS ===");
    
    int feedCount = getFeedingCount24h();
    float totalConsumed = getTotalFoodConsumed24h();
    
    Serial.print("Feeds in last 24h: ");
    Serial.println(feedCount);
    Serial.print("Total consumed: ");
    Serial.print(totalConsumed);
    Serial.println("g");
    
    if (feedCount > 0) {
        float avgPerFeed = totalConsumed / feedCount;
        Serial.print("Average per feed: ");
        Serial.print(avgPerFeed);
        Serial.println("g");
    }
    
    Serial.println("Pattern analysis complete");
}

bool detectUnusualBehavior() {
    float consumed24h = getTotalFoodConsumed24h();
    
    // Check for very low consumption (less than 0.5 portions)
    if (consumed24h < 25.0) { // Less than half of minimum portion
        Serial.println("ALERT: Unusual behavior - very low food consumption");
        return true;
    }
    
    // Check for excessive consumption
    if (consumed24h > 200.0) { // More than ~4 max portions
        Serial.println("ALERT: Unusual behavior - excessive food consumption");
        return true;
    }
    
    return false;
}

void calculateConsumptionTrends() {
    Serial.println("=== CONSUMPTION TRENDS ===");
    
    float consumed = getTotalFoodConsumed24h();
    int feeds = getFeedingCount24h();
    
    Serial.print("24h consumption: ");
    Serial.print(consumed);
    Serial.println("g");
    
    Serial.print("Feed frequency: ");
    Serial.print(feeds);
    Serial.println(" times");
    
    if (feeds > 0) {
        Serial.print("Consumption rate: ");
        Serial.print((consumed / feeds), 1);
        Serial.println("g per feed");
    }
    
    Serial.println("Trends calculated");
}

void generateHealthAlerts() {
    if (detectUnusualBehavior()) {
        Serial.println("🚨 HEALTH ALERT GENERATED 🚨");
        displayHealthAlert();
    }
}

void analyzeEatingBehavior() {
    Serial.println("Analyzing eating behavior...");
    monitorBowlStatus();
}

void adjustNextFeeding() {
    Serial.println("Adjusting next feeding based on consumption...");
    // Implementation would adjust timing or portion
}

void checkBowlStatusEndOfDay() {
    if (isBowlFull()) {
        Serial.println("Bowl still full at end of day - consider reducing portion");
        adjustPortionSize(-1); // Reduce by 1g
    }
}

void checkRapidConsumption() {
    unsigned long emptyDuration = getBowlEmptyDuration();
    
    // If bowl emptied within 5 seconds of filling
    if (emptyDuration > 0 && emptyDuration < 5000) {
        Serial.println("Rapid consumption detected - consider increasing portion");
        adjustPortionSize(1); // Increase by 1g
    }
}

void implementAdaptiveChanges() {
    Serial.println("Implementing adaptive feeding changes...");
    
    // Check end of day status
    checkBowlStatusEndOfDay();
    
    // Check for rapid consumption
    checkRapidConsumption();
    
    // Generate health alerts if needed
    generateHealthAlerts();
}

void processAdaptiveBehavior() {
    Serial.println("=== PROCESSING ADAPTIVE BEHAVIOR ===");
    
    analyzeFeedingPatterns();
    analyzeEatingBehavior();
    calculateConsumptionTrends();
    implementAdaptiveChanges();
    
    Serial.println("Adaptive behavior processing complete");
}