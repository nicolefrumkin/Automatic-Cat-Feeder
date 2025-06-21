#include <Arduino.h>
#include "config.h"
#include "hardware.h"

// Forward declarations for functions used before they're defined
float readBowlWeight();
float readTankWeight();
void detectOverfeeding();
unsigned long getBowlEmptyDuration();

// Global variables for weight sensor management
static float bowlTareWeight = 0.0;
static float tankTareWeight = 0.0;
static float bowlCalibrationFactor = 2000.0;
static float tankCalibrationFactor = 1000.0;

// Bowl monitoring variables
static float lastBowlWeight = 0.0;
static float bowlWeightBeforeFeeding = 0.0;
static unsigned long bowlEmptyStartTime = 0;
static unsigned long eatingStartTime = 0;
static bool bowlWasEmpty = true;
static bool currentlyEating = false;

// Tank monitoring variables
static float lastTankWeight = 1000.0;
static const float TANK_CAPACITY = 2000.0;
static const float TANK_LOW_THRESHOLD_GRAMS = TANK_CAPACITY * TANK_LOW_THRESHOLD;

// Bowl thresholds
static const float BOWL_FULL_THRESHOLD = 80.0;
static const float BOWL_EMPTY_THRESHOLD = 5.0;
static const float OVERFEED_THRESHOLD = 100.0;

// Simulated HX711 functions
long readHX711(int dt_pin, int sck_pin) {
    static long bowlSimValue = 50000;
    static long tankSimValue = 500000;
    
    long noise = random(-100, 100);
    
    if (dt_pin == HX711_BOWL_DT) {
        if (millis() % 60000 < 5000) {
            bowlSimValue -= random(50, 200);
            bowlSimValue = bowlSimValue < 10000L ? 10000L : bowlSimValue;
        }
        return bowlSimValue + noise;
    } else if (dt_pin == HX711_TANK_DT) {
        return tankSimValue + noise;
    }
    
    return 0;
}

float readBowlWeight() {
    long rawValue = readHX711(HX711_BOWL_DT, HX711_BOWL_SCK);
    float weight = (rawValue / bowlCalibrationFactor) - bowlTareWeight;
    
    if (weight < 0.0f) {
        weight = 0.0f;
    }
    
    static float smoothedWeight = weight;
    smoothedWeight = (smoothedWeight * 0.8f) + (weight * 0.2f);
    
    return smoothedWeight;
}

float readTankWeight() {
    long rawValue = readHX711(HX711_TANK_DT, HX711_TANK_SCK);
    float weight = (rawValue / tankCalibrationFactor) - tankTareWeight;
    
    if (weight < 0.0f) {
        weight = 0.0f;
    }
    if (weight > TANK_CAPACITY) {
        weight = TANK_CAPACITY;
    }
    
    static float smoothedWeight = weight;
    smoothedWeight = (smoothedWeight * 0.9f) + (weight * 0.1f);
    
    return smoothedWeight;
}

unsigned long getBowlEmptyDuration() {
    if (bowlWasEmpty) {
        return millis() - bowlEmptyStartTime;
    }
    return 0;
}

void detectOverfeeding() {
    float currentWeight = readBowlWeight();
    
    if (currentWeight >= OVERFEED_THRESHOLD) {
        Serial.println("🚨 OVERFEEDING DETECTED!");
        Serial.print("Bowl weight: ");
        Serial.print(currentWeight);
        Serial.print("g (threshold: ");
        Serial.print(OVERFEED_THRESHOLD);
        Serial.println("g)");
        
        Serial.println("Recommendations:");
        Serial.println("- Reduce portion size");
        Serial.println("- Check feeding schedule");
        Serial.println("- Monitor cat's eating behavior");
    }
}

void initializeWeightSensors() {
    Serial.println("=== INITIALIZING WEIGHT SENSORS ===");
    
    pinMode(HX711_BOWL_DT, INPUT);
    pinMode(HX711_BOWL_SCK, OUTPUT);
    pinMode(HX711_TANK_DT, INPUT);
    pinMode(HX711_TANK_SCK, OUTPUT);
    
    digitalWrite(HX711_BOWL_SCK, LOW);
    digitalWrite(HX711_TANK_SCK, LOW);
    
    Serial.println("HX711 pins configured");
    Serial.print("Bowl sensor - DT: ");
    Serial.print(HX711_BOWL_DT);
    Serial.print(", SCK: ");
    Serial.println(HX711_BOWL_SCK);
    
    Serial.print("Tank sensor - DT: ");
    Serial.print(HX711_TANK_DT);
    Serial.print(", SCK: ");
    Serial.println(HX711_TANK_SCK);
    
    delay(1000);
    
    Serial.println("Reading initial sensor values...");
    
    float initialBowlWeight = readBowlWeight();
    float initialTankWeight = readTankWeight();
    
    Serial.print("Initial bowl weight: ");
    Serial.print(initialBowlWeight);
    Serial.println("g");
    
    Serial.print("Initial tank weight: ");
    Serial.print(initialTankWeight);
    Serial.println("g");
    
    lastBowlWeight = initialBowlWeight;
    lastTankWeight = initialTankWeight;
    bowlWasEmpty = (initialBowlWeight <= BOWL_EMPTY_THRESHOLD);
    
    if (bowlWasEmpty) {
        bowlEmptyStartTime = millis();
    }
    
    Serial.println("Weight sensors initialized successfully!");
}

void calibrateWeightSensors() {
    Serial.println("=== WEIGHT SENSOR CALIBRATION ===");
    Serial.println("Please ensure both bowl and tank are empty");
    Serial.println("Calibration will start in 5 seconds...");
    
    for (int i = 5; i > 0; i--) {
        Serial.print("Calibrating in ");
        Serial.print(i);
        Serial.println(" seconds...");
        delay(1000);
    }
    
    Serial.println("Calibrating bowl sensor (tare)...");
    
    float bowlTareSum = 0;
    for (int i = 0; i < 10; i++) {
        long rawValue = readHX711(HX711_BOWL_DT, HX711_BOWL_SCK);
        bowlTareSum += (rawValue / bowlCalibrationFactor);
        delay(100);
    }
    bowlTareWeight = bowlTareSum / 10.0f;
    
    Serial.print("Bowl tare weight set to: ");
    Serial.print(bowlTareWeight);
    Serial.println("g");
    
    Serial.println("Calibrating tank sensor (tare)...");
    
    float tankTareSum = 0;
    for (int i = 0; i < 10; i++) {
        long rawValue = readHX711(HX711_TANK_DT, HX711_TANK_SCK);
        tankTareSum += (rawValue / tankCalibrationFactor);
        delay(100);
    }
    tankTareWeight = tankTareSum / 10.0f;
    
    Serial.print("Tank tare weight set to: ");
    Serial.print(tankTareWeight);
    Serial.println("g");
    
    Serial.println("=== CALIBRATION COMPLETE ===");
    
    delay(1000);
    Serial.print("Current bowl reading: ");
    Serial.print(readBowlWeight());
    Serial.println("g (should be ~0g)");
    
    Serial.print("Current tank reading: ");
    Serial.print(readTankWeight());
    Serial.println("g (should be ~0g)");
}

bool isBowlFull() {
    float currentWeight = readBowlWeight();
    return (currentWeight >= BOWL_FULL_THRESHOLD);
}

bool isBowlEmpty() {
    float currentWeight = readBowlWeight();
    return (currentWeight <= BOWL_EMPTY_THRESHOLD);
}

bool isTankLow() {
    float currentWeight = readTankWeight();
    return (currentWeight <= TANK_LOW_THRESHOLD_GRAMS);
}

float calculateFoodConsumed() {
    float consumed = bowlWeightBeforeFeeding - readBowlWeight();
    
    if (consumed < 0.0f) {
        consumed = 0.0f;
    }
    
    return consumed;
}

void monitorBowlStatus() {
    float currentBowlWeight = readBowlWeight();
    bool currentlyEmpty = (currentBowlWeight <= BOWL_EMPTY_THRESHOLD);
    bool currentlyFull = (currentBowlWeight >= BOWL_FULL_THRESHOLD);
    
    // Track bowl empty/full transitions
    if (currentlyEmpty && !bowlWasEmpty) {
        // Bowl just became empty
        bowlEmptyStartTime = millis();
        bowlWasEmpty = true;
        
        if (currentlyEating) {
            // Cat finished eating
            unsigned long eatingDuration = millis() - eatingStartTime;
            Serial.print("Eating session complete. Duration: ");
            Serial.print(eatingDuration / 1000);
            Serial.println(" seconds");
            
            float consumed = calculateFoodConsumed();
            Serial.print("Food consumed: ");
            Serial.print(consumed);
            Serial.println("g");
            
            currentlyEating = false;
        }
        
        Serial.println("🍽️ Bowl is now EMPTY");
    } else if (!currentlyEmpty && bowlWasEmpty) {
        // Bowl just got food
        bowlWasEmpty = false;
        bowlWeightBeforeFeeding = currentBowlWeight;
        Serial.println("🍽️ Bowl has food");
    }
    
    // Detect eating behavior
    if (!currentlyEmpty && !currentlyEating) {
        // Check if weight is decreasing (eating detected)
        if (currentBowlWeight < (lastBowlWeight - 2.0f)) {  // 2g decrease threshold
            eatingStartTime = millis();
            currentlyEating = true;
            Serial.println("🐱 Eating detected!");
        }
    }
    
    // Check for overfeeding
    if (currentlyFull) {
        detectOverfeeding();
    }
    
    // Update last weight
    lastBowlWeight = currentBowlWeight;
    
    // Print status periodically
    static unsigned long lastStatusPrint = 0;
    if (millis() - lastStatusPrint >= 30000) {  // Every 30 seconds
        Serial.println("=== BOWL STATUS ===");
        Serial.print("Current weight: ");
        Serial.print(currentBowlWeight);
        Serial.println("g");
        
        Serial.print("Status: ");
        if (currentlyEmpty) {
            Serial.println("EMPTY");
        } else if (currentlyFull) {
            Serial.println("FULL");
        } else {
            Serial.println("NORMAL");
        }
        
        if (currentlyEating) {
            Serial.println("Cat is currently eating");
        }
        
        unsigned long emptyDuration = getBowlEmptyDuration();
        if (emptyDuration > 0) {
            Serial.print("Empty for: ");
            Serial.print(emptyDuration / 1000);
            Serial.println(" seconds");
        }
        
        lastStatusPrint = millis();
    }
}

void trackEatingDuration() {
    if (currentlyEating) {
        unsigned long currentDuration = millis() - eatingStartTime;
        
        // Log eating progress every 10 seconds
        static unsigned long lastEatingLog = 0;
        if (millis() - lastEatingLog >= 10000) {
            Serial.print("🐱 Eating for ");
            Serial.print(currentDuration / 1000);
            Serial.print(" seconds. Consumed: ");
            Serial.print(calculateFoodConsumed());
            Serial.println("g");
            lastEatingLog = millis();
        }
        
        // Check for very long eating sessions (possible issue)
        if (currentDuration > 300000) {  // 5 minutes
            Serial.println("⚠️ WARNING: Very long eating session detected");
            Serial.println("Check if cat is having difficulty eating");
        }
    }
}

void preventDispenseIfFull() {
    if (isBowlFull()) {
        Serial.println("🚫 FEEDING PREVENTED - Bowl is full");
        Serial.print("Current bowl weight: ");
        Serial.print(readBowlWeight());
        Serial.print("g (full threshold: ");
        Serial.print(BOWL_FULL_THRESHOLD);
        Serial.println("g)");
        
        Serial.println("Waiting for cat to eat before next feeding...");
    }
}

void printSensorStatus() {
    Serial.println("=== WEIGHT SENSOR STATUS ===");
    
    float bowlWeight = readBowlWeight();
    float tankWeight = readTankWeight();
    
    Serial.print("Bowl: ");
    Serial.print(bowlWeight);
    Serial.print("g ");
    
    if (isBowlEmpty()) {
        Serial.print("(EMPTY)");
    } else if (isBowlFull()) {
        Serial.print("(FULL)");
    } else {
        Serial.print("(NORMAL)");
    }
    Serial.println();
    
    Serial.print("Tank: ");
    Serial.print(tankWeight);
    Serial.print("g ");
    
    if (isTankLow()) {
        Serial.print("(LOW - REFILL NEEDED)");
    } else {
        Serial.print("(OK)");
    }
    Serial.println();
    
    // Tank level percentage
    float tankPercentage = (tankWeight / TANK_CAPACITY) * 100.0f;
    Serial.print("Tank level: ");
    Serial.print(tankPercentage);
    Serial.println("%");
    
    if (currentlyEating) {
        Serial.println("🐱 Cat is currently eating");
        Serial.print("Eating duration: ");
        Serial.print((millis() - eatingStartTime) / 1000);
        Serial.println(" seconds");
    }
    
    unsigned long emptyDuration = getBowlEmptyDuration();
    if (emptyDuration > 0) {
        Serial.print("Bowl empty for: ");
        Serial.print(emptyDuration / 1000);
        Serial.println(" seconds");
    }
    
    Serial.println("============================");
}

void updateSensorReadings() {
    // Call this function regularly in your main loop
    monitorBowlStatus();
    trackEatingDuration();
    
    // Check tank level periodically
    static unsigned long lastTankCheck = 0;
    if (millis() - lastTankCheck >= 60000) {  // Check every minute
        if (isTankLow()) {
            Serial.println("🚨 LOW FOOD TANK - Please refill!");
            Serial.print("Current tank weight: ");
            Serial.print(readTankWeight());
            Serial.println("g");
        }
        lastTankCheck = millis();
    }
}