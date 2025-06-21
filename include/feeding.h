#pragma once

#include <Arduino.h>

// Core feeding operations
void dispenseFood(int amount);
void openFeederGate(int degrees);
void closeFeederGate();
void calibrateServo();
bool isServoReady();

// Feeding logic
void executeScheduledFeed();
void executeManualFeed();
bool canDispenseFood();
void processFeedingRequest();
void validateFeedingConditions();

// Portion control
int calculatePortion();
int getMinPortion();
int getMaxPortion();
void adjustPortionSize(int change);