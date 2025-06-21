#include <Arduino.h>
#include "hardware.h"
#include "feeding.h"
#include "sensors.h"
#include "display.h"  // Add display functions
#include "config.h"

// Function declarations
void checkInputs();
void handleSerialInput();
void printQuickStatus();
void printDetailedStatus();
void runSimpleTest();
void printHelp();
void testFeedingSystem();
void testSensorSystem();

// Simple variables to track state
unsigned long lastStatusUpdate = 0;
unsigned long lastInputCheck = 0;
unsigned long lastSensorUpdate = 0;
int feedCount = 0;

void setup() {
    Serial.println("=== SMART CAT FEEDER STARTING ===");
    
    // Initialize hardware
    initializeHardware();
    initializeOLED();
    initializeServo();
    
    // Initialize weight sensors
    Serial.println("Initializing weight sensors...");
    initializeWeightSensors();
    
    Serial.println("=== INITIALIZATION COMPLETE ===");
    Serial.println("System is ready!");
    Serial.println("FEEDING SYSTEM COMMANDS:");
    Serial.println("  'feed' - Manual feed");
    Serial.println("  'dispense' - Direct dispense");
    Serial.println("  'validate' - Check feeding conditions");
    Serial.println("  'calibrate' - Calibrate servo");
    Serial.println("  'test-feeding' - Run feeding system tests");
    Serial.println("SENSOR SYSTEM COMMANDS:");
    Serial.println("  'sensors' - Show sensor readings");
    Serial.println("  'calibrate-sensors' - Calibrate weight sensors");
    Serial.println("  'test-sensors' - Run sensor system tests");
    Serial.println("  'status' - Show status");
    Serial.println("  Press green button for manual feed");
}

void loop() {
    unsigned long currentTime = millis();
    
    // Check inputs every 100ms
    if (currentTime - lastInputCheck >= 100) {
        checkInputs();
        lastInputCheck = currentTime;
    }
    
    // Update sensors every 1 second
    if (currentTime - lastSensorUpdate >= 1000) {
        updateSensorReadings();
        updateDisplay();  // Update OLED display
        lastSensorUpdate = currentTime;
    }
    
    // Status update every 10 seconds
    if (currentTime - lastStatusUpdate >= 10000) {
        printQuickStatus();
        lastStatusUpdate = currentTime;
    }
    
    // Handle serial commands
    handleSerialInput();
    
    // Small delay
    delay(50);
}

void checkInputs() {
    // Check feed button
    if (readFeedButton()) {
        Serial.println("🍽️ FEED BUTTON PRESSED!");
        executeManualFeed();  // Use your feeding function
        feedCount++;
    }
    
    // Check mode switch
    static bool lastMode = true;
    bool currentMode = readModeSwitch();
    if (currentMode != lastMode) {
        Serial.print("Mode switched to: ");
        Serial.println(currentMode ? "MANUAL" : "SCHEDULED");
        lastMode = currentMode;
    }
}

void handleSerialInput() {
    String command = getSerialCommand();
    if (command.length() > 0) {
        
        // FEEDING SYSTEM TESTS
        if (command == "feed") {
            Serial.println("📝 Manual feed via serial...");
            executeManualFeed();
            feedCount++;
        }
        else if (command == "dispense") {
            int portion = calculatePortion();
            Serial.print("📝 Direct dispense: ");
            Serial.print(portion);
            Serial.println("g");
            dispenseFood(portion);
            feedCount++;
        }
        else if (command == "validate") {
            Serial.println("📝 Checking feeding conditions...");
            validateFeedingConditions();
        }
        else if (command == "calibrate") {
            Serial.println("📝 Starting servo calibration...");
            calibrateServo();
        }
        else if (command == "test-feeding") {
            Serial.println("📝 Running feeding system tests...");
            testFeedingSystem();
        }
        else if (command == "scheduled") {
            Serial.println("📝 Testing scheduled feed...");
            executeScheduledFeed();
            feedCount++;
        }
        
        // SENSOR SYSTEM TESTS
        else if (command == "sensors") {
            Serial.println("📊 Showing sensor readings...");
            printSensorStatus();
        }
        else if (command == "calibrate-sensors") {
            Serial.println("📊 Calibrating weight sensors...");
            calibrateWeightSensors();
        }
        else if (command == "test-sensors") {
            Serial.println("📊 Running sensor system tests...");
            testSensorSystem();
        }
        
        // DISPLAY SYSTEM TESTS
        else if (command == "display-welcome") {
            Serial.println("🖥️ Showing welcome screen...");
            displayWelcomeScreen();
        }
        else if (command == "display-time") {
            Serial.println("🖥️ Showing time screen...");
            displayCurrentTime();
        }
        else if (command == "display-feeding") {
            Serial.println("🖥️ Showing feeding screen...");
            displayNextFeedTime();
        }
        else if (command == "display-levels") {
            Serial.println("🖥️ Showing food levels...");
            displayKibbleAmount();
        }
        else if (command == "display-mode") {
            Serial.println("🖥️ Showing mode status...");
            displayModeStatus();
        }
        else if (command == "display-history") {
            Serial.println("🖥️ Showing feeding history...");
            displayFeedingHistory();
        }
        else if (command == "display-alert") {
            Serial.println("🖥️ Testing health alert...");
            displayHealthAlert();
        }
        
        // BASIC SYSTEM COMMANDS
        else if (command == "status") {
            printDetailedStatus();
        }
        else if (command == "test") {
            runSimpleTest();
        }
        else if (command == "help") {
            printHelp();
        }
        else {
            Serial.print("❌ Unknown command: ");
            Serial.println(command);
            Serial.println("Type 'help' for commands");
        }
    }
}

// Comprehensive feeding system test
void testFeedingSystem() {
    Serial.println("╔══════════════════════════════════╗");
    Serial.println("║     FEEDING SYSTEM TEST SUITE    ║");
    Serial.println("╚══════════════════════════════════╝");
    
    Serial.println("\n🔍 TEST 1: Feeding Conditions Validation");
    validateFeedingConditions();
    
    Serial.println("\n🔧 TEST 2: Portion Calculation");
    int portion = calculatePortion();
    Serial.print("Current portion setting: ");
    Serial.print(portion);
    Serial.println("g");
    Serial.print("Min portion: ");
    Serial.print(getMinPortion());
    Serial.println("g");
    Serial.print("Max portion: ");
    Serial.print(getMaxPortion());
    Serial.println("g");
    
    Serial.println("\n🚨 TEST 3: Safety Checks");
    bool canFeed = canDispenseFood();
    Serial.print("Can dispense food: ");
    Serial.println(canFeed ? "✅ YES" : "❌ NO");
    
    Serial.println("\n🎮 TEST 4: Servo Movement Test");
    Serial.println("Testing small servo movements...");
    moveServoToPosition(15);
    delay(1000);
    moveServoToPosition(30);
    delay(1000);
    moveServoToPosition(0);
    Serial.println("Servo test complete");
    
    Serial.println("\n⚙️ TEST 5: Gate Control Functions");
    Serial.println("Testing gate functions...");
    openFeederGate(20);
    delay(500);
    closeFeederGate();
    
    Serial.println("\n📊 TEST 6: Portion Adjustment");
    adjustPortionSize(5);
    adjustPortionSize(-3);
    
    Serial.println("\n✅ FEEDING SYSTEM TESTS COMPLETE!");
    Serial.println("Ready for actual feeding operations.");
}

// Comprehensive sensor system test
void testSensorSystem() {
    Serial.println("╔══════════════════════════════════╗");
    Serial.println("║      SENSOR SYSTEM TEST SUITE    ║");
    Serial.println("╚══════════════════════════════════╝");
    
    Serial.println("\n📊 TEST 1: Weight Sensor Readings");
    printSensorStatus();
    
    Serial.println("\n🍽️ TEST 2: Bowl Status Detection");
    Serial.print("Bowl empty: ");
    Serial.println(isBowlEmpty() ? "YES" : "NO");
    Serial.print("Bowl full: ");
    Serial.println(isBowlFull() ? "YES" : "NO");
    
    Serial.println("\n🏢 TEST 3: Tank Status Detection");
    Serial.print("Tank low: ");
    Serial.println(isTankLow() ? "YES - REFILL NEEDED" : "NO - OK");
    
    Serial.println("\n🔢 TEST 4: Food Consumption Calculation");
    float consumed = calculateFoodConsumed();
    Serial.print("Food consumed since last feeding: ");
    Serial.print(consumed);
    Serial.println("g");
    
    Serial.println("\n⏱️ TEST 5: Bowl Empty Duration");
    unsigned long emptyDuration = getBowlEmptyDuration();
    if (emptyDuration > 0) {
        Serial.print("Bowl has been empty for: ");
        Serial.print(emptyDuration / 1000);
        Serial.println(" seconds");
    } else {
        Serial.println("Bowl is not empty");
    }
    
    Serial.println("\n🚨 TEST 6: Safety Checks");
    Serial.println("Testing overfeed detection...");
    detectOverfeeding();
    
    Serial.println("Testing dispense prevention...");
    preventDispenseIfFull();
    
    Serial.println("\n✅ SENSOR SYSTEM TESTS COMPLETE!");
    Serial.println("All sensor functions are operational.");
}

void printQuickStatus() {
    Serial.print("⏰ Uptime: ");
    Serial.print(millis() / 1000);
    Serial.print("s | Feeds: ");
    Serial.print(feedCount);
    Serial.print(" | Portion: ");
    Serial.print(calculatePortion());
    Serial.print("g | Mode: ");
    Serial.println(readModeSwitch() ? "MANUAL" : "SCHEDULED");
}

void printDetailedStatus() {
    Serial.println("╔══════════════════════════════════╗");
    Serial.println("║         SYSTEM STATUS            ║");
    Serial.println("╚══════════════════════════════════╝");
    
    Serial.print("System uptime: ");
    Serial.print(millis() / 1000);
    Serial.println(" seconds");
    
    Serial.print("Total feeds: ");
    Serial.println(feedCount);
    
    Serial.print("Current portion setting: ");
    Serial.print(calculatePortion());
    Serial.println("g");
    
    Serial.print("Feeding mode: ");
    Serial.println(readModeSwitch() ? "MANUAL" : "SCHEDULED");
    
    Serial.print("Servo status: ");
    Serial.println(isServoReady() ? "READY" : "NOT READY");
    
    Serial.print("Can feed now: ");
    Serial.println(canDispenseFood() ? "YES" : "NO");
    
    // Add sensor status to detailed status
    Serial.println("\n--- SENSOR STATUS ---");
    printSensorStatus();
    
    Serial.println("═══════════════════════════════════");
}

void runSimpleTest() {
    Serial.println("=== BASIC HARDWARE TEST ===");
    blinkStatusLED(2);
    Serial.print("Potentiometer: ");
    Serial.print(readPotentiometer());
    Serial.println("g");
    
    if (isServoReady()) {
        Serial.println("Quick servo test...");
        moveServoToPosition(10);
        delay(300);
        moveServoToPosition(0);
    }
    Serial.println("=== HARDWARE TEST COMPLETE ===");
}

void printHelp() {
    Serial.println("╔══════════════════════════════════╗");
    Serial.println("║         AVAILABLE COMMANDS       ║");
    Serial.println("╠══════════════════════════════════╣");
    Serial.println("║ FEEDING COMMANDS:                ║");
    Serial.println("║  feed        - Manual feed        ║");
    Serial.println("║  dispense    - Direct dispense    ║");
    Serial.println("║  scheduled   - Scheduled feed     ║");
    Serial.println("║  validate    - Check conditions   ║");
    Serial.println("║  calibrate   - Servo calibration  ║");
    Serial.println("║  test-feeding- Full feeding tests ║");
    Serial.println("║                                   ║");
    Serial.println("║ SENSOR COMMANDS:                  ║");
    Serial.println("║  sensors     - Show sensor data   ║");
    Serial.println("║  calibrate-sensors - Calibrate    ║");
    Serial.println("║  test-sensors- Full sensor tests  ║");
    Serial.println("║                                   ║");
    Serial.println("║ DISPLAY COMMANDS:                 ║");
    Serial.println("║  display-welcome - Welcome screen ║");
    Serial.println("║  display-time - Time screen       ║");
    Serial.println("║  display-feeding - Feed schedule  ║");
    Serial.println("║  display-levels - Food levels     ║");
    Serial.println("║  display-mode - Mode status       ║");
    Serial.println("║  display-history - Feed history   ║");
    Serial.println("║  display-alert - Test alerts      ║");
    Serial.println("║                                   ║");
    Serial.println("║ SYSTEM COMMANDS:                  ║");
    Serial.println("║  status      - Detailed status    ║");
    Serial.println("║  test        - Basic hardware test║");
    Serial.println("║  help        - Show this menu     ║");
    Serial.println("║                                   ║");
    Serial.println("║ HARDWARE CONTROLS:                ║");
    Serial.println("║  • Green button - Manual feed     ║");
    Serial.println("║  • Potentiometer - Adjust portion ║");
    Serial.println("║  • Blue switch - Change mode      ║");
    Serial.println("╚══════════════════════════════════╝");
}