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
  delay(1000); // Give serial time to initialize

  Serial.println("=== SMART CAT FEEDER STARTING ===");
  initializeHardware();
  initializeServo(); // This was missing in the original main.cpp!
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
  loadSettings();
  setSystemState(IDLE);
  setFeedingMode(SCHEDULED);  // default mode

  Serial.println("=== INITIALIZATION COMPLETE ===");
  Serial.println("System is ready for operation!");

  delay(1000);
}

void loop() {
  updateBlinkingLED();
  updateSensorReadings();
  checkInputs();
  String command = getSerialCommand();
  if (command.length() > 0) {
    handleSerialInput(command);
  }
  updateSystemState();
  updateDayCycle();

  if (isSystemInEmergencyMode()) {
    // Already in emergency mode — don't repeat feeding error handling
    static bool alreadyHandled = false;
    if (!alreadyHandled) {
      displayEmergencyMessage("Emergency Stop");
      emergencyStop();
      alreadyHandled = true;
    }
    return; // Skip rest of loop
  }

  // Safety checks before any feeding operations
  if (!performSafetyChecks()) {
    handleFeedingError();
    return; // Skip feeding if safety check fails
  }

  // Handle mode-specific logic
  switch (getCurrentMode()) {
    case SCHEDULED:
      runScheduledMode();
      break;
    case MANUAL:
      runManualMode();
      break;
  }

  processAdaptiveBehavior();
  updateDisplay();

  // Communication and monitoring
  publishSystemStatus();
  handleMQTTReconnect();

  // Simulate cat eating behavior for testing
  randomizeEatingBehavior();
  
  // Periodic health check
  performHealthCheck();

  // Small delay to prevent overwhelming the system
  delay(100);
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
  else if (command == "test servo") {
    calibrateServo();
  }
  else if (command == "reset") {
    resetSystem();
  }
  else if (command == "emergency") {
    emergencyStop();
  }
  else if (command == "clear emergency") {
    clearEmergencyMode();
    setSystemState(IDLE);
  }
  else if (command == "sensors") {
    printSensorStatus();
  }
  else if (command == "history") {
    printFeedingHistory();
  }
  else if (command == "settings") {
    loadSettings();
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
    Serial.println("emergency       - Activate emergency stop");
    Serial.println("clear emergency - Clear emergency mode");
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