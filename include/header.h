#pragma once // the compiler ensures that the contents of that file are included only once per compilation unit, even if it's included multiple timesץ

#include <ESP32Servo.h>
#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Project configuration and constants
#define OLED_SDA 21
#define OLED_SCL 22
#define SERVO_PIN 13
#define BUTTON_PIN 14
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
#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""
#define MQTT_SERVER "broker.hivemq.com"
#define MQTT_PORT 1883 // default TCP port for MQTT

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

struct SystemSettings
{
    int Portion;            // Default feeding portion
    // FeedingMode Mode;      // Current feeding mode
    int Mode; // 0 = SCHEDULED, 1 = MANUAL
};

extern Servo feederServo;
extern SystemSettings settings;

// Constants for sensor thresholds
const float BOWL_FULL_THRESHOLD = 80.0;   // 80g = full bowl
const float BOWL_EMPTY_THRESHOLD = 5.0;   // 5g = empty bowl
const float TANK_EMPTY_THRESHOLD = 100.0; // 100g = nearly empty tank

extern WiFiClient espClient;
extern PubSubClient mqttClient;

// MQTT functions
void setupWiFi();
void setupMQTT();

// init functions
void initOLED();
void initHX711();

// read values
// FeedingMode getFeedingMode();
int getFeedingMode();
int getPortionFromPot();
void handleSerialCommands();
void displayWelcomeScreen();
void detectButtonPress();

