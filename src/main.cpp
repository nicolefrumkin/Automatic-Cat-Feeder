#include <Arduino.h>
#include "config.h"
#include "hardware.h"
#include "sensors.h"
#include "feeding.h"
#include "display.h"
#include "communication.h"
#include "data_manager.h"
#include "adaptive.h"
#include "state_machine.h"
#include "timing.h"
#include "simulation.h"
#include "safety.h"
#include "utils.h"

void handleSerialInput(String command);

void setup() {
  Serial.begin(115200);

  Serial.println("=== SMART CAT FEEDER STARTING ===");
  initializeHardware();
  initializeServo(); 
  initializeWeightSensors();
  initializeOLED();
  initializeDataManager();
  initializeRTC();
  initializeMQTT();
  displayBootScreen();
  delay(2000);

  printSystemStatus();
  displayWelcomeScreen();

  // Load settings and set initial states

  setSystemState(IDLE);
  setFeedingMode(SCHEDULED);  // default mode

  Serial.println("=== INITIALIZATION COMPLETE ===");
  Serial.println("System is ready for operation!");

  delay(1000);
}

// REPLACE your entire loop() function in main.cpp with this:

void loop() {
    
  if (Serial.available() > 0) {         // Check if data is available
    String input = Serial.readString(); // Read full string until timeout or newline
    input.trim();                       // Remove any leading/trailing whitespace
    handleSerialInput(input);           // Process the command
    }

  // Basic heartbeat (fast)
  updateBlinkingLED();
  
  // Update sensor readings (includes eating simulation) - LESS FREQUENT
  static unsigned long lastSensorUpdate = 0;
  if (millis() - lastSensorUpdate > 2000) { // Every 2 seconds
      updateSensorReadings();
      lastSensorUpdate = millis();
  }
  
  // Check inputs - LESS FREQUENT
  static unsigned long lastInputCheck = 0;
  if (millis() - lastInputCheck > 500) { // Every 500ms
      checkInputs();
      lastInputCheck = millis();
  }
  
  // Update system state - LESS FREQUENT  
  static unsigned long lastStateUpdate = 0;
  if (millis() - lastStateUpdate > 1000) { // Every 1 second
      updateSystemState();
      lastStateUpdate = millis();
  }
  
  // Day cycle updates - LESS FREQUENT
  static unsigned long lastDayUpdate = 0;
  if (millis() - lastDayUpdate > 5000) { // Every 5 seconds
      updateDayCycle();
      lastDayUpdate = millis();
  }

  // Safety checks - MUCH LESS FREQUENT
  static unsigned long lastSafetyCheck = 0;
  if (millis() - lastSafetyCheck > 30000) { // Every 30 seconds
      if (!performSafetyChecks()) {
          Serial.println("Safety checks failed");
          handleFeedingError();
      }
      lastSafetyCheck = millis();
  }

  // Mode-specific logic - LESS FREQUENT
  static unsigned long lastModeCheck = 0;
  if (millis() - lastModeCheck > 10000) { // Every 10 seconds (REDUCED FREQUENCY)
      switch (getCurrentMode()) {
          case SCHEDULED:
              runScheduledMode();
              break;
          case MANUAL:
              runManualMode();
              break;
      }
      lastModeCheck = millis();
  }

  // Adaptive behavior checks - MUCH LESS FREQUENT
  static unsigned long lastAdaptiveCheck = 0;
  if (millis() - lastAdaptiveCheck > 120000) { // Every 2 MINUTES
      processAdaptiveBehavior();
      lastAdaptiveCheck = millis();
  }

  // Update display - LESS FREQUENT
  static unsigned long lastDisplayUpdate = 0;
  if (millis() - lastDisplayUpdate > 5000) { // Every 5 seconds
      updateDisplay();
      lastDisplayUpdate = millis();
  }
  
  // MQTT updates - LESS FREQUENT
  static unsigned long lastMQTTUpdate = 0;
  if (millis() - lastMQTTUpdate > 15000) { // Every 15 seconds
      publishSystemStatus();
      handleMQTTReconnect();
      lastMQTTUpdate = millis();
  }

  // Simulation for cat eating - LESS FREQUENT
  static unsigned long lastEatingSimulation = 0;
  if (millis() - lastEatingSimulation > 3000) { // Every 3 seconds
      randomizeEatingBehavior();
      lastEatingSimulation = millis();
  }
  
  // ADD THE PERIODIC STATUS HERE (replaces spam)
  printPeriodicStatus();
  
  // Add a small delay to prevent overwhelming the system
  delay(100); // 100ms delay between loop iterations
}

// Handle serial input commands for debugging and control
void handleSerialInput(String command) {
  Serial.print("Processing command: ");
  Serial.println(command);

  if (command == "status") {
    printSystemStatus();
  }
  else if (command == "feed") {
    if (getCurrentMode() == MANUAL) {
      executeManualFeed();
    } else {
      executeScheduledFeed();
    }
  }
  else if (command == "sensors") {
    printSensorStatus();
  }
  else if (command == "history") {
    printFeedingHistory();
  }
  else if (command == "calibrate") {
    calibrateWeightSensors();
  }
  else if (command == "test") {
    // Comprehensive system test
    Serial.println("=== RUNNING SYSTEM TEST ===");
    validateFeedingConditions();
    printSensorStatus();
    Serial.println("System test complete");
  }
  else if (command.startsWith("set portion ")) {
    int portion = command.substring(12).toInt();
    if (portion >= MIN_PORTION && portion <= MAX_PORTION) {
      updateSetting("defaultPortion", portion);
      Serial.print("Portion size set to: ");
      Serial.print(portion);
      Serial.println("g");
    } else {
      Serial.println("Invalid portion size. Range: 30-75g");
    }
  }
  else if (command == "help") {
    Serial.println("=== AVAILABLE COMMANDS ===");
    Serial.println("status          - Show system status");
    Serial.println("feed            - Trigger feeding");
    Serial.println("test servo      - Test servo movement");
    Serial.println("reset           - Reset system");
    Serial.println("sensors         - Show sensor readings");
    Serial.println("history         - Show feeding history");
    Serial.println("settings        - Show current settings");
    Serial.println("calibrate       - Calibrate weight sensors");
    Serial.println("test            - Run system test");
    Serial.println("set portion X   - Set portion size (30-75g)");
    Serial.println("help            - Show this help");
    Serial.println("========================");
  }
  else {
    Serial.println("Unknown command. Type 'help' for available commands.");
  }
}