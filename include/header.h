#pragma once // the compiler ensures that the contents of that file are included only once per compilation unit, even if it's included multiple timesץ

#include <ESP32Servo.h>
#include <Arduino.h>
#include <Wire.h>

// Project configuration and constants
#define OLED_SDA 21
#define OLED_SCL 22
#define SERVO_PIN 13
#define BUTTON_PIN 15
#define SWITCH_PIN 26
#define POT_PIN 32
#define LED_PIN 27
#define HX711_BOWL_DT 16
#define HX711_BOWL_SCK 17
#define HX711_TANK_DT 4
#define HX711_TANK_SCK 2

#define MIN_PORTION 30
#define MAX_PORTION 75
#define DAY_CYCLE_MS 120000 // 2 minutes = 1 day
#define TANK_LOW_THRESHOLD 0.25

// MQTT settings
#define MQTT_SERVER "your_mqtt_server"
#define MQTT_PORT 1883

// Global enums and structs
enum FeedingMode
{
    SCHEDULED,
    MANUAL
};
enum SystemState
{
    IDLE,
    FEEDING,
    MONITORING,
    ERROR
};

struct FeedingEvent
{
    unsigned long timestamp;
    String mode;
    int quantity;
    float bowlWeightBefore;
    float bowlWeightAfter;
    float consumed;               // Amount consumed
    unsigned long eatingDuration; // How long cat took to eat
    uint8_t modeCode;             // 0=MANUAL, 1=SCHEDULED
};

struct SystemSettings
{
    int Portion;            // Default feeding portion
    FeedingMode Mode;      // Current feeding mode
    unsigned long feedingInterval; // Minimum time between feeds
    float bowlFullThreshold;       // Bowl full threshold in grams
    float bowlEmptyThreshold;      // Bowl empty threshold in grams
    float tankLowThreshold;        // Tank low threshold percentage
    bool adaptiveFeedingEnabled;   // Enable adaptive feeding
    unsigned long dayStartTime;    // Time when day cycle starts
};

extern Servo feederServo;
extern SystemSettings settings;

// Constants for sensor thresholds
const float BOWL_FULL_THRESHOLD = 80.0;   // 80g = full bowl
const float BOWL_EMPTY_THRESHOLD = 5.0;   // 5g = empty bowl
const float TANK_EMPTY_THRESHOLD = 100.0; // 100g = nearly empty tank

// init functions
void initOLED();
void initSlideSwitch();
void initButton();
void initServo();
void initHX711();
void initLED();

// read values
FeedingMode getFeedingMode();
int getPortionFromPot();
void handleSerialInput(String command);
void displayWelcomeScreen();

