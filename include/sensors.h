#pragma once

#include <Arduino.h>

// Weight sensor management functions
void initializeWeightSensors();
float readBowlWeight();
float readTankWeight();
void calibrateWeightSensors();
bool isBowlFull();
bool isBowlEmpty();
bool isTankLow();
float calculateFoodConsumed();

// Bowl monitoring functions
void monitorBowlStatus();
unsigned long getBowlEmptyDuration();
void trackEatingDuration();
void detectOverfeeding();
void preventDispenseIfFull();

// Additional helper functions for testing and status
void printSensorStatus();
void updateSensorReadings();

// Constants and thresholds (can be accessed from other files if needed)
extern const float BOWL_FULL_THRESHOLD;
extern const float BOWL_EMPTY_THRESHOLD;
extern const float OVERFEED_THRESHOLD;
extern const float TANK_CAPACITY;
extern const float TANK_LOW_THRESHOLD_GRAMS;