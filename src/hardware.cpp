// Hardware initialization and basic I/O
#include <Arduino.h>
#include "config.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>

void performHealthCheck();

// OLED display object
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1  // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  // Common I2C address for SSD1306

// Servo position constants
#define SERVO_CLOSED_POS 0       // Feeder gate closed (0 degrees)
#define SERVO_OPEN_POS 90        // Feeder gate fully open (90 degrees)
#define SERVO_MIN_PULSE 500      // Minimum pulse width in microseconds
#define SERVO_MAX_PULSE 2500     // Maximum pulse width in microseconds
#define SERVO_MOVE_DELAY 15      // Delay between position steps (ms)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Servo object and configuration
Servo feederServo;
int currentServoPosition = 0;    // Track current servo position
bool servoAttached = false;      // Track servo attachment status

// Non-blocking LED blink for use in main loop
static bool blinkState = false;
static unsigned long lastBlinkTime = 0;
static int blinkInterval = 1000;  // Default 1 second

// Button debounce variables
static unsigned long lastButtonPress = 0;
static bool lastButtonState = HIGH;
static bool currentButtonState = HIGH;
static const unsigned long DEBOUNCE_DELAY = 50;  // 50ms debounce

// Switch debounce variables
static unsigned long lastSwitchChange = 0;
static bool lastSwitchState = HIGH;
static bool currentSwitchState = HIGH;

// Serial input buffer
static String serialBuffer = "";
static bool serialInputComplete = false;

// System state tracking for error handling
static bool systemInEmergencyMode = false;
static unsigned long lastValidSensorReading = 0;
static int consecutiveFailures = 0;
static const int MAX_CONSECUTIVE_FAILURES = 5;
static const unsigned long SENSOR_TIMEOUT_MS = 10000;  // 10 seconds

void initializeHardware() {
  // Initialize Serial communication first for debugging
  Serial.begin(57600);
  while (!Serial) {
    ; // Wait for serial port to connect (needed for native USB)
  }
  Serial.println("Smart Cat Feeder - Hardware Initialization Starting...");

  // Initialize digital pins
  pinMode(LED_PIN, OUTPUT);           // Status LED
  pinMode(BUTTON_PIN, INPUT_PULLUP);  // Manual feed button (active LOW)
  pinMode(SWITCH_PIN, INPUT_PULLUP);  // Mode switch (active LOW)

  // Initialize analog pins
  pinMode(POT_PIN, INPUT);            // Potentiometer for portion control

  // Set initial states
  digitalWrite(LED_PIN, LOW);         // Turn off status LED initially

  // Brief startup LED sequence to indicate system is alive
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
    delay(200);
  }

  // Initialize servo pin (servo library will be initialized separately)
  //pinMode(SERVO_PIN, OUTPUT);

  // Initialize HX711 pins for weight sensors
  pinMode(HX711_BOWL_DT, INPUT);
  pinMode(HX711_BOWL_SCK, OUTPUT);
  pinMode(HX711_TANK_DT, INPUT);
  pinMode(HX711_TANK_SCK, OUTPUT);

  // Set HX711 clock pins LOW initially
  digitalWrite(HX711_BOWL_SCK, LOW);
  digitalWrite(HX711_TANK_SCK, LOW);

  // Small delay to let hardware settle
  delay(500);

  // Test button (should read HIGH due to pullup when not pressed)
  bool buttonState = digitalRead(BUTTON_PIN);
  Serial.print("Button pin state: ");
  Serial.println(buttonState ? "HIGH (not pressed)" : "LOW (pressed)");

  // Test mode switch
  bool switchState = digitalRead(SWITCH_PIN);
  Serial.print("Switch pin state: ");
  Serial.println(switchState ? "HIGH (position 1)" : "LOW (position 2)");

  int potValue = analogRead(POT_PIN);
  Serial.print("Potentiometer value: ");
  Serial.print(potValue);
  Serial.println(" (0-1023 range in Wokwi)");  

  // Final status LED indication
  digitalWrite(LED_PIN, HIGH);
  Serial.println("Hardware initialization complete!");
  Serial.println("Status LED: ON (system ready)");
  Serial.println("----------------------------------------\n");
}

void initializeOLED() {
  Serial.println("Initializing OLED display...");

  // Initialize I2C communication
  Wire.begin(OLED_SDA, OLED_SCL);

  // Small delay to let I2C settle
  delay(100);

  // Initialize the display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("ERROR: SSD1306 allocation failed!");
    Serial.println("Check wiring:");
    Serial.print("  SDA -> Pin ");
    Serial.println(OLED_SDA);
    Serial.print("  SCL -> Pin ");
    Serial.println(OLED_SCL);
    Serial.print("  VCC -> 3.3V");
    Serial.print("  GND -> GND");

    // Blink LED rapidly to indicate OLED error
    for (int i = 0; i < 10; i++) {
      digitalWrite(LED_PIN, HIGH);
      delay(100);
      digitalWrite(LED_PIN, LOW);
      delay(100);
    }
    return;
  }

  // Clear the display buffer
  display.clearDisplay();

  // Set text properties
  display.setTextSize(1);              // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0);             // Start at top-left corner

  // Display initialization message
  display.println("Smart Cat Feeder");
  display.println("================");
  display.println();
  display.println("OLED: OK");
  display.println("System: Starting...");
  display.println();
  display.println("Please wait...");

  // Show the display buffer on screen
  display.display();

  Serial.println("OLED display initialized successfully!");
  Serial.print("Resolution: ");
  Serial.print(SCREEN_WIDTH);
  Serial.print("x");
  Serial.println(SCREEN_HEIGHT);
  Serial.print("I2C Address: 0x");
  Serial.println(SCREEN_ADDRESS, HEX);

  // Keep the initialization message visible for 2 seconds
  delay(2000);

  // Clear display for normal operation
  display.clearDisplay();
  display.display();

  Serial.println("OLED ready for operation!\n");
}

void initializeServo() {
  Serial.println("Initializing servo motor...");

  // Set PWM frequency
  feederServo.setPeriodHertz(50);  // 50Hz is standard for servos

  // Try attaching the servo
  int attached = feederServo.attach(SERVO_PIN, SERVO_MIN_PULSE, SERVO_MAX_PULSE);
  if (attached >= 0) {
    servoAttached = true;
    Serial.print("Servo attached successfully on pin ");
    Serial.println(SERVO_PIN);
  } else {
    servoAttached = false;
    Serial.println("ERROR: Failed to attach servo!");
    return;
  }

  // Move servo to closed position initially
  Serial.println("Moving servo to closed position...");
  feederServo.write(SERVO_CLOSED_POS);
  currentServoPosition = SERVO_CLOSED_POS;
  delay(100);
  Serial.println("Servo initialization complete!\n");
}

// Helper function to check if servo is ready for operation
bool isServoReady() {
  return servoAttached;
}

// Helper function to get current servo position
int getCurrentServoPosition() {
  return currentServoPosition;
}

// Helper function for smooth servo movement
void moveServoToPosition(int targetPosition) {
  if (!servoAttached) {
    Serial.println("ERROR: Servo not attached!");
    return;
  }

  // Constrain target position to valid range
  targetPosition = constrain(targetPosition, SERVO_CLOSED_POS, SERVO_OPEN_POS);

  Serial.print("Moving servo from ");
  Serial.print(currentServoPosition);
  Serial.print(" to ");
  Serial.print(targetPosition);
  Serial.println(" degrees");

  // Move gradually for smooth operation
  if (targetPosition > currentServoPosition) {
    // Moving to higher angle
    for (int pos = currentServoPosition; pos <= targetPosition; pos++) {
      feederServo.write(pos);
      delay(SERVO_MOVE_DELAY);
    }
  } else {
    // Moving to lower angle
    for (int pos = currentServoPosition; pos >= targetPosition; pos--) {
      feederServo.write(pos);
      delay(SERVO_MOVE_DELAY);
    }
  }

  currentServoPosition = targetPosition;
  Serial.print("Servo movement complete. Position: ");
  Serial.print(currentServoPosition);
  Serial.println(" degrees");
}

// LED state tracking
bool currentLEDState = false;
unsigned long lastLEDUpdate = 0;

void setStatusLED(bool state) {
  // Update the LED state
  digitalWrite(LED_PIN, state ? HIGH : LOW);
  currentLEDState = state;
  lastLEDUpdate = millis();
}

// Helper function to get current LED state
bool getLEDState() {
  return currentLEDState;
}

// Helper function to toggle LED state
void toggleStatusLED() {
  setStatusLED(!currentLEDState);
}

// Advanced function for different blink patterns
void blinkStatusLED(int times, int onTime, int offTime) {
  if (times <= 0) return;

  bool originalState = currentLEDState;

  for (int i = 0; i < times; i++) {
    setStatusLED(true);   // Turn ON
    delay(onTime);
    setStatusLED(false);  // Turn OFF
    if (i < times - 1) {  // Don't delay after last blink
      delay(offTime);
    }
  }

  // Restore original state
  setStatusLED(originalState);
}

// System status indication using LED patterns
void indicateSystemStatus(SystemState state) {
  switch (state) {
    case IDLE:
      setStatusLED(true);  // Solid ON = system ready/idle
      break;

    case FEEDING:
      // Fast blinking during feeding
      blinkStatusLED(3, 100, 100);
      break;

    case MONITORING:
      // Slow pulse = monitoring mode
      blinkStatusLED(1, 500, 500);
      break;

    case ERROR:
      // Rapid blinking = error state
      blinkStatusLED(10, 50, 50);
      break;

    default:
      setStatusLED(false);  // OFF for unknown state
      break;
  }
}

void setLEDBlinkInterval(int intervalMs) {
  blinkInterval = intervalMs;
}

void updateBlinkingLED() {
  unsigned long currentTime = millis();

  if (currentTime - lastBlinkTime >= blinkInterval) {
    blinkState = !blinkState;
    setStatusLED(blinkState);
    lastBlinkTime = currentTime;
  }
}

// LED patterns for different events
void indicateFeedingSuccess() {
  // 3 quick blinks = feeding complete
  blinkStatusLED(3, 150, 150);
  setStatusLED(true);  // Return to solid ON
}

void indicateFeedingError() {
  // 5 fast blinks = feeding error
  blinkStatusLED(5, 100, 100);
  setStatusLED(false);  // Turn OFF to indicate error
}

void indicateLowFood() {
  // 2 long blinks = low food warning
  blinkStatusLED(2, 800, 200);
  setStatusLED(true);  // Return to normal
}

void indicateWiFiConnecting() {
  // Rapid continuous blinking while connecting
  setLEDBlinkInterval(200);  // 200ms blink interval
  // Use updateBlinkingLED() in main loop while connecting
}

void indicateWiFiConnected() {
  // 3 quick blinks then solid ON
  blinkStatusLED(3, 100, 100);
  setStatusLED(true);
  setLEDBlinkInterval(1000);  // Reset to default
}

void blinkStatusLED(int times) {
  // Input validation - must be positive number
  if (times <= 0) {
    Serial.println("WARNING: blinkStatusLED called with invalid times value");
    return;
  }

  // Store the original LED state to restore later
  bool originalState = digitalRead(LED_PIN);

  Serial.print("Blinking LED ");
  Serial.print(times);
  Serial.println(" times...");

  // Perform the blinking sequence
  for (int i = 0; i < times; i++) {
    // Turn LED ON
    digitalWrite(LED_PIN, HIGH);
    delay(200);  // ON duration: 200ms

    // Turn LED OFF
    digitalWrite(LED_PIN, LOW);
    delay(200);  // OFF duration: 200ms

    // Optional: Print blink count for debugging
    Serial.print("Blink ");
    Serial.print(i + 1);
    Serial.print("/");
    Serial.println(times);
  }

  // Restore the original LED state
  digitalWrite(LED_PIN, originalState);

  Serial.print("Blink sequence complete. LED restored to: ");
  Serial.println(originalState ? "ON" : "OFF");
}

// Input reading functions
int readPotentiometer() {
  // Read raw analog value (0-1023 in WOKWI, 0-4095 on real ESP32)
  int rawValue = analogRead(POT_PIN);

  // Map to portion range (30-75 grams) - CORRECT for WOKWI
  int portionGrams = map(rawValue, 0, 1023, MIN_PORTION, MAX_PORTION);

  // Optional: Add some smoothing to prevent jittery readings
  static int lastReading = portionGrams;
  static int smoothedValue = portionGrams;

  // Simple low-pass filter for smoother readings
  smoothedValue = (smoothedValue * 9 + portionGrams) / 10;

  // Only print if value changed significantly (reduce spam)
  if (abs(smoothedValue - lastReading) > 2) {
    Serial.print("Potentiometer: ");
    Serial.print(rawValue);
    Serial.print(" (raw) -> ");
    Serial.print(smoothedValue);
    Serial.println("g (mapped)");
    lastReading = smoothedValue;
  }

  return smoothedValue;
}

bool readFeedButton() {
  // Read current button state (active LOW due to INPUT_PULLUP)
  bool reading = digitalRead(BUTTON_PIN);

  // Check if button state changed
  if (reading != lastButtonState) {
    // Reset debounce timer
    lastButtonPress = millis();
  }

  // Check if enough time has passed since last change
  if ((millis() - lastButtonPress) > DEBOUNCE_DELAY) {
    // If the reading has been stable for debounce period
    if (reading != currentButtonState) {
      currentButtonState = reading;

      // Button is pressed when it goes from HIGH to LOW (falling edge)
      if (currentButtonState == LOW) {
        Serial.println("Feed button PRESSED");
        lastButtonState = reading;
        return true;  // Button press detected
      } else {
        Serial.println("Feed button RELEASED");
      }
    }
  }

  lastButtonState = reading;
  return false;  // No button press
}

bool readModeSwitch() {
  // Read current switch state (active LOW due to INPUT_PULLUP)
  bool reading = digitalRead(SWITCH_PIN);

  // Check if switch state changed
  if (reading != lastSwitchState) {
    // Reset debounce timer
    lastSwitchChange = millis();
  }

  // Check if enough time has passed since last change
  if ((millis() - lastSwitchChange) > DEBOUNCE_DELAY) {
    // If the reading has been stable for debounce period
    if (reading != currentSwitchState) {
      currentSwitchState = reading;

      // Print switch position change
      Serial.print("Mode switch changed to: ");
      if (currentSwitchState == LOW) {
        Serial.println("MANUAL mode");
      } else {
        Serial.println("SCHEDULED mode");
      }
    }
  }

  lastSwitchState = reading;

  // Return true for MANUAL mode (switch LOW), false for SCHEDULED mode (switch HIGH)
  return (currentSwitchState == LOW);
}

String readSerialInput() {
  // Handle incoming serial data
  while (Serial.available() > 0) {
    char incomingChar = Serial.read();

    // Check for end of line characters
    if (incomingChar == '\n' || incomingChar == '\r') {
      if (serialBuffer.length() > 0) {
        serialInputComplete = true;
        Serial.print("Serial input received: '");
        Serial.print(serialBuffer);
        Serial.println("'");
      }
    } else if (incomingChar >= 32 && incomingChar <= 126) {
      // Only add printable characters
      serialBuffer += incomingChar;

      // Prevent buffer overflow
      if (serialBuffer.length() > 50) {
        Serial.println("WARNING: Serial input too long, clearing buffer");
        serialBuffer = "";
      }
    }
  }

  // Return complete input and reset buffer
  if (serialInputComplete) {
    String result = serialBuffer;
    serialBuffer = "";
    serialInputComplete = false;
    return result;
  }

  // Return empty string if no complete input
  return "";
}

// Helper function to get current feeding mode based on switch
FeedingMode getCurrentFeedingMode() {
  return readModeSwitch() ? MANUAL : SCHEDULED;
}

// Helper function to get portion amount in grams
int getCurrentPortionGrams() {
  return readPotentiometer();
}

// Helper function to check if button was just pressed (non-blocking)
bool wasButtonJustPressed() {
  static bool lastPressState = false;
  bool currentPress = readFeedButton();

  // Return true only on rising edge (button just pressed)
  if (currentPress && !lastPressState) {
    lastPressState = currentPress;
    return true;
  }

  lastPressState = currentPress;
  return false;
}

// Helper function to get formatted serial command
String getSerialCommand() {
  String input = readSerialInput();
  if (input.length() > 0) {
    input.trim();  // Remove whitespace
    input.toLowerCase();  // Convert to lowercase for easier parsing
    return input;
  }
  return "";
}

// Helper function for parsing time input (HH:MM format)
bool parseTimeInput(String input, int &hours, int &minutes) {
  input.trim();

  // Check if input matches HH:MM format
  if (input.length() == 5 && input.charAt(2) == ':') {
    String hourStr = input.substring(0, 2);
    String minuteStr = input.substring(3, 5);

    hours = hourStr.toInt();
    minutes = minuteStr.toInt();

    // Validate time range
    if (hours >= 0 && hours <= 23 && minutes >= 0 && minutes <= 59) {
      Serial.print("Parsed time: ");
      Serial.print(hours);
      Serial.print(":");
      if (minutes < 10) Serial.print("0");
      Serial.println(minutes);
      return true;
    }
  }

  Serial.println("ERROR: Invalid time format. Use HH:MM (24-hour format)");
  return false;
}

// Test function to display all current input states
void printInputStates() {
  static unsigned long lastPrint = 0;
  unsigned long currentTime = millis();

  // Print every 2 seconds to avoid spam
  if (currentTime - lastPrint >= 2000) {
    Serial.println("=== INPUT STATES ===");
    Serial.print("Potentiometer: ");
    Serial.print(getCurrentPortionGrams());
    Serial.println("g");

    Serial.print("Feed Button: ");
    Serial.println(digitalRead(BUTTON_PIN) ? "NOT PRESSED" : "PRESSED");

    Serial.print("Mode Switch: ");
    Serial.println(getCurrentFeedingMode() == MANUAL ? "MANUAL" : "SCHEDULED");

    Serial.println("==================");
    lastPrint = currentTime;
  }
}

// Periodic health check function (call in main loop)
void performHealthCheck() {
  static unsigned long lastHealthCheck = 0;
  unsigned long currentTime = millis();

  // Perform health check every 30 seconds
  if (currentTime - lastHealthCheck >= 30000) {
    Serial.println("Performing periodic health check...");
    lastHealthCheck = currentTime;
  }
}
