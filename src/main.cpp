#include "header.h"

Servo feederServo;
SystemSettings settings;
WiFiClient espClient;
PubSubClient mqttClient(espClient);

void setup()
{
  Serial.begin(115200);
  Serial.println("\n\n=== SMART CAT FEEDER STARTING ===\n");
  // setupWiFi();
  // setupMQTT();

  initOLED();                        // Initialize OLED display
  pinMode(SWITCH_PIN, INPUT_PULLUP); // Initialize slide switch
  pinMode(BUTTON_PIN, INPUT_PULLUP); // Initialize button
  pinMode(POT_PIN, INPUT);           // Initialize potentiometer
  feederServo.attach(SERVO_PIN);     // Initialize servo
  initHX711();
  // initialize OLED display
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); // Turn off initially

  settings.Mode = getFeedingMode();
  settings.Portion = getPortionFromPot();
  Serial.println("mode: " + String(settings.Mode == MANUAL ? "MANUAL" : "SCHEDULED"));
  Serial.println("portion: " + String(settings.Portion) + "g\n");

  displayWelcomeScreen();

  Serial.println("=== INITIALIZATION COMPLETE ===\n");
}

void loop()
{
  settings.Mode = getFeedingMode();
  settings.Portion = getPortionFromPot();

  // if (!mqttClient.connected())
  // {
  //   setupMQTT(); // Reconnect if needed
  // }
  // mqttClient.loop(); // Handle incoming messages

  handleSerialCommands();

  if (settings.Mode == MANUAL)
  {
    detectButtonPress();
    // Do something for manual mode
  }
  else
  {
    // Do something for scheduled mode
  }
}
