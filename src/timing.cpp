#include <Arduino.h>
#include "config.h"
#include "state_machine.h"

static String scheduledFeedTime = "12:00";
static unsigned long dayStartTime = 0;

void initializeRTC() {
    Serial.println("RTC: Initializing (using millis() as time source)");
    dayStartTime = millis();
}

String getCurrentTime() {
    unsigned long elapsed = millis() - dayStartTime;
    unsigned long seconds = (elapsed / 1000) % 60;
    unsigned long minutes = (elapsed / 60000) % 60;
    unsigned long hours = (elapsed / 3600000) % 24;
    
    String timeStr = "";
    if (hours < 10) timeStr += "0";
    timeStr += String(hours);
    timeStr += ":";
    if (minutes < 10) timeStr += "0";
    timeStr += String(minutes);
    timeStr += ":";
    if (seconds < 10) timeStr += "0";
    timeStr += String(seconds);
    
    return timeStr;
}

String getNextFeedTime() {
    // Calculate next feeding time based on day cycle
    unsigned long cycleTime = millis() % DAY_CYCLE_MS;
    unsigned long timeToNext = DAY_CYCLE_MS - cycleTime;
    
    return String(timeToNext / 1000) + "s";
}

void setFeedingSchedule(String time) {
    scheduledFeedTime = time;
    Serial.print("Feeding schedule set to: ");
    Serial.println(time);
}

bool isScheduledFeedTime() {
    // For simulation, feed every day cycle
    unsigned long cycleTime = millis() % DAY_CYCLE_MS;
    return (cycleTime < 1000); // Feed in first second of cycle
}

void updateDayCycle() {
    static unsigned long lastCycle = 0;
    unsigned long currentCycle = millis() / DAY_CYCLE_MS;
    
    if (currentCycle > lastCycle) {
        Serial.println("=== NEW DAY CYCLE STARTED ===");
        performDailyMaintenance();
        lastCycle = currentCycle;
    }
}

unsigned long getTimestamp() {
    return millis();
}

bool isFeedingTime() {
    return isScheduledFeedTime();
}