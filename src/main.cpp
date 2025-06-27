#include "header.h"

WiFiClient espClient;
PubSubClient mqttClient(espClient);

Servo feederServo;
FeederStatus feeder; // external - available in all cpp files

// timing constants
unsigned long lastReadingUpdate = 0;
unsigned long lastDisplayUpdate = 0;
const unsigned long interval1 = 1000;  // 1 second
const unsigned long interval2 = 500;  // 0.5 second

void setup()
{
  Serial.begin(115200);
  Serial.println("\n\n=== SMART CAT FEEDER STARTING ===\n");
  
  setupWiFi();
  setupMQTT();
  initOLED();  // Initialize OLED display
                        
  pinMode(SWITCH_PIN, INPUT_PULLUP); // Initialize slide switch

  pinMode(BUTTON_PIN, INPUT_PULLUP); // Initialize button
  pinMode(POT_PIN, INPUT);           // Initialize potentiometer
  feederServo.attach(SERVO_PIN, 500, 2400);     // Initialize servo
  initHX711();
  // initialize OLED display
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); // Turn off initially

  feeder.Mode = getFeedingMode();
  feeder.portionSize = getPortionFromPot();
  Serial.println("mode: " + String(feeder.Mode == MANUAL ? "MANUAL" : "SCHEDULED"));
  Serial.println("portion: " + String(feeder.portionSize) + "g\n");

  displayWelcomeScreen();
  feederInit();
  Serial.println("=== INITIALIZATION COMPLETE ===\n");
  delay(2000); // Wait for a second before starting the loop
}

void loop()
{

  if (feeder.Mode == MANUAL)
  {
    if(detectButtonPress()) {
      addFoodToBowl();
    }
    // Do something for manual mode
  }
  else // scheduled mode
  {
    unsigned long currentMillis = millis();
    if (currentMillis - feeder.lastFeedTime >= feeder.feedInterval) {
      // Time to feed
      feeder.lastFeedTime = currentMillis; // Update last feed time
      addFoodToBowl();
      //Serial.println("Feeding event occurred. Total today: " + String(feeder.feedEventsToday));
    }
  }
  
  // timing logic for display and reading updates
  unsigned long currentMillis = millis();

  if (currentMillis - lastDisplayUpdate >= interval1) {
    lastDisplayUpdate = currentMillis;
    // update function per interval1
    functionsUpdate();
  }
  // next day cycle update
  if ((currentMillis - feeder.dayCycle * DAY_CYCLE_MS) >= DAY_CYCLE_MS) {
    resetFeederForNextDay();
  }
}
