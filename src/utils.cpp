#include <Arduino.h>
#include "config.h"
#include "hardware.h"
#include "state_machine.h"
#include "sensors.h"
#include "data_manager.h"

String formatTimestamp(unsigned long time) {
    unsigned long seconds = time / 1000;
    unsigned long minutes = seconds / 60;
    unsigned long hours = minutes / 60;
    
    seconds = seconds % 60;
    minutes = minutes % 60;
    hours = hours % 24;
    
    String result = "";
    if (hours < 10) result += "0";
    result += String(hours) + ":";
    if (minutes < 10) result += "0";
    result += String(minutes) + ":";
    if (seconds < 10) result += "0";
    result += String(seconds);
    
    return result;
}

String formatWeight(float weight) {
    return String(weight, 1) + "g";
}

int mapPotToGrams(int potValue) {
    return map(potValue, 0, 1023, MIN_PORTION, MAX_PORTION);
}

void playFeedingSound() {
    // Simple tone using LED blinks instead of actual sound
    Serial.println("♪ Playing feeding sound (LED blinks)");
    blinkStatusLED(3, 100, 100);
}

void delay_ms(int milliseconds) {
    delay(milliseconds);
}

bool isWithinRange(float value, float min, float max) {
    return (value >= min && value <= max);
}

void printPeriodicStatus() {
    static unsigned long lastStatusPrint = 0;
    
    if (millis() - lastStatusPrint > 30000) { // Every 30 seconds
        Serial.println("\n=== SYSTEM STATUS UPDATE ===");
        Serial.print("Uptime: ");
        Serial.print(millis() / 1000);
        Serial.println(" seconds");
        
        Serial.print("Mode: ");
        Serial.println(getCurrentMode() == MANUAL ? "MANUAL" : "SCHEDULED");
        
        Serial.print("Bowl weight: ");
        Serial.print(readBowlWeight(), 1);
        Serial.println("g");
        
        Serial.print("Tank weight: ");
        Serial.print(readTankWeight(), 1);
        Serial.println("g");
        
        Serial.print("Portion setting: ");
        Serial.print(readPotentiometer());
        Serial.println("g");
        
        Serial.print("Feeds today: ");
        Serial.println(getFeedingCount24h());
        
        Serial.println("============================\n");
        
        lastStatusPrint = millis();
    }
}