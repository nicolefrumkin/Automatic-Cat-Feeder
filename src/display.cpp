#include <Arduino.h>
#include "config.h"
#include "hardware.h"
#include "sensors.h"
#include "data_manager.h"
#include "feeding.h"  // Add this include for calculatePortion()
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// External reference to display object from hardware.cpp
extern Adafruit_SSD1306 display;

// Display state tracking
static unsigned long lastDisplayUpdate = 0;
static int currentScreen = 0;
static const int DISPLAY_UPDATE_INTERVAL = 2000; // Update every 2 seconds
static const int MAX_SCREENS = 6; // Number of different screens

// Helper function to format time
String formatTime(unsigned long milliseconds) {
    unsigned long seconds = milliseconds / 1000;
    unsigned long minutes = seconds / 60;
    unsigned long hours = minutes / 60;
    
    seconds = seconds % 60;
    minutes = minutes % 60;
    hours = hours % 24;
    
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

// Helper function to format uptime
String formatUptime(unsigned long milliseconds) {
    unsigned long seconds = milliseconds / 1000;
    unsigned long minutes = seconds / 60;
    unsigned long hours = minutes / 60;
    unsigned long days = hours / 24;
    
    if (days > 0) {
        return String(days) + "d " + String(hours % 24) + "h";
    } else if (hours > 0) {
        return String(hours) + "h " + String(minutes % 60) + "m";
    } else if (minutes > 0) {
        return String(minutes) + "m " + String(seconds % 60) + "s";
    } else {
        return String(seconds) + "s";
    }
}

void displayWelcomeScreen() {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Smart Cat");
    display.println("Feeder");
    
    display.setTextSize(1);
    display.setCursor(0, 40);
    display.print("Mode: ");
    display.println(readModeSwitch() ? "MANUAL" : "SCHEDULED");
    
    display.setCursor(0, 50);
    display.print("Portion: ");
    display.print(calculatePortion());
    display.println("g");
    
    display.display();
}

void displayCurrentTime() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("CURRENT TIME");
    display.println("============");
    
    // Display uptime as "current time"
    display.setTextSize(2);
    display.setCursor(0, 20);
    String timeStr = formatTime(millis());
    display.println(timeStr);
    
    display.setTextSize(1);
    display.setCursor(0, 45);
    display.print("Uptime: ");
    display.println(formatUptime(millis()));
    
    display.setCursor(0, 55);
    display.print("Day cycle: ");
    unsigned long cycleTime = millis() % DAY_CYCLE_MS;
    display.print((cycleTime * 100) / DAY_CYCLE_MS);
    display.println("%");
    
    display.display();
}

void displayNextFeedTime() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("FEEDING SCHEDULE");
    display.println("================");
    
    // Calculate next feed time (simplified)
    unsigned long cycleTime = millis() % DAY_CYCLE_MS;
    unsigned long timeToNextFeed = DAY_CYCLE_MS - cycleTime;
    
    display.setCursor(0, 20);
    display.println("Next feed in:");
    
    display.setTextSize(2);
    display.setCursor(0, 35);
    display.print(timeToNextFeed / 1000);
    display.println("s");
    
    display.setTextSize(1);
    display.setCursor(0, 55);
    display.print("Feeds today: ");
    display.println(getFeedingCount24h());
    
    display.display();
}

void displayKibbleAmount() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("FOOD LEVELS");
    display.println("===========");
    
    float bowlWeight = readBowlWeight();
    float tankWeight = readTankWeight();
    
    // Bowl status
    display.setCursor(0, 20);
    display.print("Bowl: ");
    display.print(bowlWeight, 1);
    display.println("g");
    
    // Bowl status bar
    display.setCursor(0, 30);
    int bowlPercent = map(bowlWeight, 0, 100, 0, 100);
    bowlPercent = constrain(bowlPercent, 0, 100);
    display.print("[");
    for (int i = 0; i < 10; i++) {
        if (i < bowlPercent / 10) {
            display.print("=");
        } else {
            display.print(" ");
        }
    }
    display.print("] ");
    display.print(bowlPercent);
    display.println("%");
    
    // Tank status
    display.setCursor(0, 45);
    display.print("Tank: ");
    display.print(tankWeight, 0);
    display.println("g");
    
    // Tank status
    display.setCursor(0, 55);
    if (isTankLow()) {
        display.println("STATUS: REFILL NEEDED!");
    } else if (isBowlFull()) {
        display.println("STATUS: BOWL FULL");
    } else if (isBowlEmpty()) {
        display.println("STATUS: BOWL EMPTY");
    } else {
        display.println("STATUS: NORMAL");
    }
    
    display.display();
}

void displayModeStatus() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("SYSTEM STATUS");
    display.println("=============");
    
    // Current mode
    display.setCursor(0, 20);
    display.print("Mode: ");
    display.println(readModeSwitch() ? "MANUAL" : "SCHEDULED");
    
    // Portion setting
    display.setCursor(0, 30);
    display.print("Portion: ");
    display.print(calculatePortion());
    display.println("g");
    
    // System status
    display.setCursor(0, 40);
    display.print("Servo: ");
    display.println(isServoReady() ? "READY" : "NOT READY");

    display.display();
}

void displayFeedingHistory() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("FEEDING HISTORY");
    display.println("===============");
    
    // Get recent feeding count
    int feedCount = getFeedingCount24h();
    float totalConsumed = getTotalFoodConsumed24h();
    
    display.setCursor(0, 20);
    display.print("Feeds (24h): ");
    display.println(feedCount);
    
    display.setCursor(0, 30);
    display.print("Consumed: ");
    display.print(totalConsumed, 1);
    display.println("g");
    
    // Last feeding info
    display.setCursor(0, 40);
    display.println("Last feeding:");
    display.setCursor(0, 50);
    if (feedCount > 0) {
        display.println("MANUAL - 45g");  // Simplified for now
    } else {
        display.println("No recent feeds");
    }
    
    display.display();
}

void displayLowFoodAlert() {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("ALERT!");
    
    display.setTextSize(1);
    display.setCursor(0, 25);
    display.println("Food tank is low");
    display.println("Please refill soon");
    
    display.setCursor(0, 45);
    display.print("Tank: ");
    display.print(readTankWeight(), 0);
    display.println("g");
    
    display.setCursor(0, 55);
    display.println("Threshold: 100g");
    
    display.display();
}

void displayHealthAlert() {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("HEALTH");
    display.println("ALERT!");
    
    display.setTextSize(1);
    display.setCursor(0, 35);
    display.println("Low food consumption");
    display.println("detected. Monitor pet");
    display.println("health closely.");
    
    display.display();
}

void updateDisplay() {
    unsigned long currentTime = millis();
    
    // Auto-cycle through screens every few seconds
    if (currentTime - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL) {
        currentScreen = (currentScreen + 1) % MAX_SCREENS;
        lastDisplayUpdate = currentTime;
        
        // Display appropriate screen based on current screen number
        switch (currentScreen) {
            case 0:
                displayWelcomeScreen();
                break;
            case 1:
                displayCurrentTime();
                break;
            case 2:
                displayNextFeedTime();
                break;
            case 3:
                displayKibbleAmount();
                break;
            case 4:
                displayModeStatus();
                break;
            case 5:
                displayFeedingHistory();
                break;
            default:
                displayWelcomeScreen();
                currentScreen = 0;
                break;
        }
    }
}

void clearDisplay() {
    display.clearDisplay();
    display.display();
}

// Show specific screen on demand
void showScreen(int screenNumber) {
    currentScreen = screenNumber % MAX_SCREENS;
    lastDisplayUpdate = millis(); // Reset timer
    
    switch (currentScreen) {
        case 0:
            displayWelcomeScreen();
            break;
        case 1:
            displayCurrentTime();
            break;
        case 2:
            displayNextFeedTime();
            break;
        case 3:
            displayKibbleAmount();
            break;
        case 4:
            displayModeStatus();
            break;
        case 5:
            displayFeedingHistory();
            break;
    }
}

// Display feeding in progress
void displayFeedingInProgress(int amount) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("FEEDING");
    
    display.setTextSize(1);
    display.setCursor(0, 25);
    display.println("Dispensing food...");
    
    display.setCursor(0, 40);
    display.print("Amount: ");
    display.print(amount);
    display.println("g");
    
    display.setCursor(0, 55);
    display.println("Please wait...");
    
    display.display();
}

// Display success message
void displayFeedingSuccess(int amount) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("SUCCESS!");
    
    display.setTextSize(1);
    display.setCursor(0, 25);
    display.println("Feeding complete");
    
    display.setCursor(0, 40);
    display.print("Dispensed: ");
    display.print(amount);
    display.println("g");
    
    display.setCursor(0, 55);
    display.println("Enjoy your meal!");
    
    display.display();
}

// Display calibration screen
void displayCalibrationMode() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("CALIBRATION MODE");
    display.println("================");
    
    display.setCursor(0, 20);
    display.println("Follow instructions");
    display.println("on serial monitor");
    
    display.setCursor(0, 40);
    display.println("Step: Servo test");
    
    display.setCursor(0, 55);
    display.println("Please wait...");
    
    display.display();
}

// Additional utility functions for completeness
void displayBootScreen() {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(10, 10);
    display.println("BOOTING");
    
    display.setTextSize(1);
    display.setCursor(0, 35);
    display.println("Smart Cat Feeder v1.0");
    display.setCursor(0, 50);
    display.println("Initializing...");
    
    display.display();
}

void displayErrorScreen(String errorMessage) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("ERROR");
    
    display.setTextSize(1);
    display.setCursor(0, 25);
    display.println(errorMessage);
    
    display.setCursor(0, 45);
    display.println("System stopped");
    display.setCursor(0, 55);
    display.println("Check connections");
    
    display.display();
}

void displayMaintenanceMode() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("MAINTENANCE MODE");
    display.println("================");
    
    display.setCursor(0, 20);
    display.println("System maintenance");
    display.println("in progress...");
    
    display.setCursor(0, 40);
    display.println("- Analyzing data");
    display.println("- Saving settings");
    display.println("- Cleaning logs");
    
    display.display();
}

void displayNetworkStatus(bool connected) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("NETWORK STATUS");
    display.println("==============");
    
    display.setCursor(0, 20);
    display.print("WiFi: ");
    display.println(connected ? "CONNECTED" : "DISCONNECTED");
    
    display.setCursor(0, 35);
    display.print("MQTT: ");
    display.println(connected ? "ONLINE" : "OFFLINE");
    
    if (connected) {
        display.setCursor(0, 50);
        display.println("Remote monitoring");
        display.println("active");
    } else {
        display.setCursor(0, 50);
        display.println("Operating in");
        display.println("offline mode");
    }
    
    display.display();
}

void displayProgressBar(int percentage, String label) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println(label);
    display.println("================");
    
    // Progress bar
    display.setCursor(0, 25);
    display.print("[");
    for (int i = 0; i < 20; i++) {
        if (i < percentage / 5) {
            display.print("=");
        } else {
            display.print(" ");
        }
    }
    display.print("] ");
    display.print(percentage);
    display.println("%");
    
    display.display();
}

