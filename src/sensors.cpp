#include <Arduino.h>
#include "config.h"
#include "hardware.h"
#include "sensors.h"

// Global variables for weight sensors
static float bowlWeight = 0.0;
static float tankWeight = 1000.0;  // Start with full tank
static float lastBowlWeight = 0.0;
static unsigned long lastBowlEmptyTime = 0;
static unsigned long bowlEmptyStartTime = 0;
static bool bowlWasEmpty = false;
static unsigned long lastEatingStartTime = 0;
static bool isEating = false;
unsigned long lastSensorUpdate = 0;  // Time of last successful sensor update

// Calibration values (would be set during calibration)
static float bowlCalibrationFactor = 1.0;
static float tankCalibrationFactor = 1.0;
static float bowlZeroOffset = 0.0;
static float tankZeroOffset = 0.0;

void initializeWeightSensors() {
  Serial.println("=== INITIALIZING WEIGHT SENSORS ===");

  // Initialize HX711 pins
  pinMode(HX711_BOWL_DT, INPUT);
  pinMode(HX711_BOWL_SCK, OUTPUT);
  pinMode(HX711_TANK_DT, INPUT);
  pinMode(HX711_TANK_SCK, OUTPUT);

  // Set clock pins low initially
  digitalWrite(HX711_BOWL_SCK, LOW);
  digitalWrite(HX711_TANK_SCK, LOW);

  // Simulate initialization delay
  delay(1000);

  // Set initial values
  bowlWeight = 10.0;  // Start with small amount in bowl
  tankWeight = 1500.0; // Start with full tank
  lastBowlWeight = bowlWeight;

  Serial.println("Weight sensors initialized!");
  Serial.print("Initial bowl weight: ");
  Serial.print(bowlWeight);
  Serial.println("g");
  Serial.print("Initial tank weight: ");
  Serial.print(tankWeight);
  Serial.println("g");

  Serial.println("Weight sensor initialization complete!\n");
}

float readBowlWeight() {
  // Simulate HX711 reading with some noise and variation
  // In real implementation, this would read from HX711

  float reading = bowlWeight;

  // Ensure reading is never negative - fix the max() issue
  if (reading < 0.0) {
    reading = 0.0;
  }

  return reading;
}

float readTankWeight() {
  // Simulate tank weight reading
  // In real implementation, this would read from second HX711

  float reading = tankWeight;

  // Ensure reading is never negative - fix the max() issue
  if (reading < 0.0) {
    reading = 0.0;
  }

  return reading;
}

void calibrateWeightSensors() {
  Serial.println("=== WEIGHT SENSOR CALIBRATION ===");

  Serial.println("Starting calibration process...");
  Serial.println("1. Remove all items from bowl and tank areas");
  Serial.println("2. Press any key to continue...");

  // Zero calibration
  Serial.println("Calibrating zero points...");

  // Simulate zero calibration
  bowlZeroOffset = readBowlWeight();
  tankZeroOffset = readTankWeight();

  Serial.print("Bowl zero offset: ");
  Serial.print(bowlZeroOffset);
  Serial.println("g");
  Serial.print("Tank zero offset: ");
  Serial.print(tankZeroOffset);
  Serial.println("g");

  // Known weight calibration
  Serial.println("Place known weight (100g) on bowl scale...");
  delay(3000); // Simulate waiting

  float knownWeight = 100.0;
  float bowlReading = readBowlWeight() - bowlZeroOffset;
  if (bowlReading != 0) {
    bowlCalibrationFactor = knownWeight / bowlReading;
  }

  Serial.print("Bowl calibration factor: ");
  Serial.println(bowlCalibrationFactor);

  // Repeat for tank
  Serial.println("Place known weight (500g) on tank scale...");
  delay(3000);

  float tankKnownWeight = 500.0;
  float tankReading = readTankWeight() - tankZeroOffset;
  if (tankReading != 0) {
    tankCalibrationFactor = tankKnownWeight / tankReading;
  }

  Serial.print("Tank calibration factor: ");
  Serial.println(tankCalibrationFactor);

  Serial.println("Calibration complete!");
  Serial.println("Remove all calibration weights");
  delay(2000);

  // Test calibrated readings
  Serial.println("Testing calibrated readings:");
  for (int i = 0; i < 5; i++) {
    Serial.print("Bowl: ");
    Serial.print(readBowlWeight());
    Serial.print("g, Tank: ");
    Serial.print(readTankWeight());
    Serial.println("g");
    delay(1000);
  }
}

bool isBowlFull() {
  float weight = readBowlWeight();
  return weight >= BOWL_FULL_THRESHOLD;
}

bool isBowlEmpty() {
  float weight = readBowlWeight();
  return weight <= BOWL_EMPTY_THRESHOLD;
}

bool isTankLow() {
  float weight = readTankWeight();
  return weight <= TANK_EMPTY_THRESHOLD;
}

float calculateFoodConsumed() {
  // Calculate food consumed since last measurement
  float currentWeight = readBowlWeight();
  float consumed = lastBowlWeight - currentWeight;

  // Only count positive consumption (food eaten, not added)
  if (consumed > 0) {
    lastBowlWeight = currentWeight;
    return consumed;
  }

  return 0.0;
}

// Bowl monitoring functions
void monitorBowlStatus() {
  float currentWeight = readBowlWeight();
  bool currentlyEmpty = (currentWeight <= BOWL_EMPTY_THRESHOLD);

  // Track empty duration
  if (currentlyEmpty && !bowlWasEmpty) {
    // Bowl just became empty
    bowlEmptyStartTime = millis();
    bowlWasEmpty = true;
    Serial.println("Bowl became empty");
  } else if (!currentlyEmpty && bowlWasEmpty) {
    // Bowl is no longer empty
    bowlWasEmpty = false;
    Serial.println("Bowl refilled");
  }

  // Check for eating behavior
  if (!currentlyEmpty && !isEating && (lastBowlWeight - currentWeight) > 1.0) {
    // Eating started (weight decreased by more than 1g)
    isEating = true;
    lastEatingStartTime = millis();
    Serial.println("Cat started eating");
  } else if (isEating && abs(lastBowlWeight - currentWeight) < 0.5) {
    // Eating stopped (weight stabilized)
    isEating = false;
    unsigned long eatingDuration = millis() - lastEatingStartTime;
    Serial.print("Cat finished eating. Duration: ");
    Serial.print(eatingDuration / 1000);
    Serial.println(" seconds");
  }

  lastBowlWeight = currentWeight;
}

unsigned long getBowlEmptyDuration() {
  if (bowlWasEmpty) {
    return millis() - bowlEmptyStartTime;
  }
  return 0;
}

void trackEatingDuration() {
  if (isEating) {
    unsigned long currentDuration = millis() - lastEatingStartTime;
    Serial.print("Currently eating for: ");
    Serial.print(currentDuration / 1000);
    Serial.println(" seconds");
  }
}

void detectOverfeeding() {
  float currentWeight = readBowlWeight();

  if (currentWeight > BOWL_FULL_THRESHOLD * 1.2) { // 20% over threshold
    Serial.println("WARNING: Overfeeding detected!");
    Serial.print("Bowl weight: ");
    Serial.print(currentWeight);
    Serial.print("g (threshold: ");
    Serial.print(BOWL_FULL_THRESHOLD);
    Serial.println("g)");

    // Could trigger emergency stop here
    // emergencyStop();
  }
}

void preventDispenseIfFull() {
  if (isBowlFull()) {
    Serial.println("PREVENTION: Bowl is full - dispensing blocked");
    Serial.print("Current bowl weight: ");
    Serial.print(readBowlWeight());
    Serial.print("g (full threshold: ");
    Serial.print(BOWL_FULL_THRESHOLD);
    Serial.println("g)");

    // Could flash LED or show message on display
    blinkStatusLED(5);

    // Log the prevention event
    Serial.println("Overfeeding prevention activated");
  }
}

// Update sensor readings and simulate eating
void updateSensorReadings() {
  // Simulate gradual eating
  simulateEating();

  // Monitor bowl status
  monitorBowlStatus();

  // Check for tank level
  if (isTankLow()) {
    static unsigned long lastLowTankWarning = 0;
    if (millis() - lastLowTankWarning > 30000) { // Warn every 30 seconds
      Serial.println("WARNING: Food tank is low! Please refill.");
      lastLowTankWarning = millis();
    }
  }
  lastSensorUpdate = millis();

}

// Simulate eating behavior for testing
void simulateEating() {
  static unsigned long lastEatingSimulation = 0;
  static bool simulatingEating = false;
  static unsigned long eatingStartTime = 0;

  // Start eating simulation randomly
  if (!simulatingEating && millis() - lastEatingSimulation > 30000 && random(100) < 5) { // 5% chance every check
    if (bowlWeight > BOWL_EMPTY_THRESHOLD) {
      simulatingEating = true;
      eatingStartTime = millis();
      Serial.println("🐱 Cat started eating (simulation)");
    }
  }

  // Continue eating simulation
  if (simulatingEating) {
    unsigned long eatingDuration = millis() - eatingStartTime;

    // Eat for 10-30 seconds
    if (eatingDuration < 30000 && bowlWeight > BOWL_EMPTY_THRESHOLD) {
      // Reduce bowl weight gradually (0.1g per second)
      bowlWeight -= 0.1;
      if (bowlWeight < 0) bowlWeight = 0;
    } else {
      // Stop eating
      simulatingEating = false;
      lastEatingSimulation = millis();
      Serial.println("🐱 Cat finished eating (simulation)");
    }
  }

  // Simulate tank depletion when food is dispensed
  // This would be handled in the feeding functions in real implementation
}

// Print current sensor status
void printSensorStatus() {
  Serial.println("=== SENSOR STATUS ===");

  float bowlWt = readBowlWeight();
  float tankWt = readTankWeight();

  Serial.print("Bowl weight: ");
  Serial.print(bowlWt);
  Serial.println("g");

  Serial.print("Tank weight: ");
  Serial.print(tankWt);
  Serial.println("g");

  Serial.print("Bowl status: ");
  if (isBowlEmpty()) {
    Serial.println("EMPTY");
  } else if (isBowlFull()) {
    Serial.println("FULL");
  } else {
    Serial.println("NORMAL");
  }

  Serial.print("Tank status: ");
  if (isTankLow()) {
    Serial.println("LOW - REFILL NEEDED");
  } else {
    Serial.println("OK");
  }

  if (bowlWasEmpty) {
    Serial.print("Bowl empty for: ");
    Serial.print(getBowlEmptyDuration() / 1000);
    Serial.println(" seconds");
  }

  if (isEating) {
    Serial.println("Cat is currently eating");
  }

  Serial.println("=====================");
}

// Helper function to add food to bowl (for testing)
void addFoodToBowl(float amount) {
  bowlWeight += amount;
  tankWeight -= amount; // Reduce tank weight
  if (tankWeight < 0) tankWeight = 0;

  Serial.print("Added ");
  Serial.print(amount);
  Serial.println("g to bowl");
  Serial.print("New bowl weight: ");
  Serial.print(bowlWeight);
  Serial.println("g");
  Serial.print("New tank weight: ");
  Serial.print(tankWeight);
  Serial.println("g");
}

// Helper function to refill tank (for testing)
void refillTank() {
  tankWeight = 1500.0; // Full tank
  Serial.println("Tank refilled to 1500g");
}