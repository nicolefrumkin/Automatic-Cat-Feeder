#include <Arduino.h>
#include "config.h"
#include "hardware.h"

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
    return map(potValue, 0, 4095, MIN_PORTION, MAX_PORTION);
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