#pragma once
// Project configuration and constants
#define OLED_SDA 21
#define OLED_SCL 22
#define SERVO_PIN 18
#define BUTTON_PIN 25
#define SWITCH_PIN 26
#define POT_PIN 32
#define LED_PIN 27
#define HX711_BOWL_DT 16
#define HX711_BOWL_SCK 17
#define HX711_TANK_DT 4
#define HX711_TANK_SCK 2

#define MIN_PORTION 30
#define MAX_PORTION 75
#define DAY_CYCLE_MS 120000  // 2 minutes = 1 day
#define TANK_LOW_THRESHOLD 0.25

// MQTT settings
#define MQTT_SERVER "your_mqtt_server"
#define MQTT_PORT 1883

// Global enums and structs
enum FeedingMode { SCHEDULED, MANUAL };
enum SystemState { IDLE, FEEDING, MONITORING, ERROR };

struct FeedingEvent {
    unsigned long timestamp;
    String mode;
    int quantity;
    float bowlWeightBefore;
    float bowlWeightAfter;
};