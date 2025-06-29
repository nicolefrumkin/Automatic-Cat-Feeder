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

// MQTT settings
#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""
#define MQTT_SERVER "broker.hivemq.com"
#define MQTT_PORT 1883 // default TCP port for MQTT
#define MQTT_TOPIC_FEED_EVENT   "catfeeder/feed"
#define MQTT_TOPIC_ALERT        "catfeeder/alert"
#define MQTT_TOPIC_STATUS       "catfeeder/status"

// Feeding settings
#define FULL_TANK 2000 // grams = 2KG tank
#define MIN_PORTION 30
#define MAX_PORTION 75
#define DAY_CYCLE_MS 120000 // 2 minutes = 1 day
#define DEFAULT_FEED_RATE 20000 // 
#define FEED_RATE_CHANGE 2500 // equivalent to 30 minutes
#define MIN_FEED_RATE 60000 // equivalent to every 12 hours
#define MAX_FEED_RATE 10000 // equivalent to every 2 hours 
#define TANK_LOW_THRESHOLD 0.25
#define BOWL_FULL_THRESHOLD 250 // grams
const int BOWL_EMPTY_THRESHOLD = 5;   // 5g = empty bowl
const int TANK_EMPTY_THRESHOLD = FULL_TANK * TANK_LOW_THRESHOLD; 

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

struct FeederStatus {
  FeedingMode Mode;     // Current feeding mode
  int portionSize;      // grams per portion (e.g., 30g–75g)
  int bowlLevel;        // grams currently in bowl (from load cell)
  int tankLevel;        // grams remaining in main reservoir
  int feedEventsToday;  // count of feedings today
  bool bowlIsFull;      // flag: bowl is full or not
  bool tankLow;         // flag: tank low alert (<=25%)
  int dayCycle;       // current day cycle (0-23)
  unsigned long lastFeedTime; // timestamp of last feeding (ms since boot or RTC)
  unsigned long feedInterval;
  unsigned long bowlEmptyTime; 
};

extern FeederStatus feeder;
extern Servo feederServo;

extern WiFiClient espClient;
extern PubSubClient mqttClient;

// MQTT functions
void setupWiFi();
void setupMQTT();

// init functions
void initOLED();
void initHX711();
void feederInit();

// read values
FeedingMode getFeedingMode();
int getPortionFromPot();
void handleSerialCommands();
void displayWelcomeScreen();
bool detectButtonPress();
void functionsUpdate();

//display functions
void displayFunctionScreen();   
void displayWelcomeScreen();
String formatTime(unsigned long milliseconds);

// feeding functions
void addFoodToBowl();
void simulateEating();
void resetFeederForNextDay();
void logFeedingEvent();
void checkEatingTrendAndAlert();
void displayAlert();
void printMQTTInstructions();

