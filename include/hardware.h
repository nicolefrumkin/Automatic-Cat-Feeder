#pragma once

#include <Arduino.h>
#include "config.h"

// Existing declarations (e.g., initializeHardware, etc.)
void clearEmergencyMode();
void initializeHardware();
void initializeOLED();
void initializeServo();
bool isServoReady();
int getCurrentServoPosition();
void moveServoToPosition(int targetPosition);
void setStatusLED(bool state);
bool getLEDState();
void toggleStatusLED();
void blinkStatusLED(int times);
void blinkStatusLED(int times, int onTime, int offTime);
void indicateSystemStatus(SystemState state);
void setLEDBlinkInterval(int intervalMs);
void updateBlinkingLED();
void indicateFeedingSuccess();
void indicateFeedingError();
void indicateLowFood();
void indicateWiFiConnecting();
void indicateWiFiConnected();
void clearEmergencyMode();
bool hasSensorTimeout();
int readPotentiometer();
bool readFeedButton();
bool readModeSwitch();
String readSerialInput();
String getSerialCommand(); 
void emergencyStop();      
void performHealthCheck();
void printInputStates();
