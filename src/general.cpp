#include "header.h"

void handleSerialCommands()
{
  if (Serial.available() > 0)
  {
    String command = Serial.readString();
    command.trim();

    Serial.print("Processing command: ");
    Serial.println(command);

    if (command == "help")
    {
      Serial.println("Available commands:");
      Serial.println("1. help - Show this help message");
      Serial.println("2. fill tank - Top the tank with 2KG of kibble");
      Serial.println("3. increase - Increaing Feeding rate by half an hour ");
      Serial.println("4. decrease - Decarsing feeding rate, by half an hour ");
      Serial.println("5. default - Setting default feeding rate to 6 times a day");
      Serial.println("6. MQTT - MQTT setting instructions");
    }

    if (command == "fill tank")
    {
      feeder.tankLevel = FULL_TANK; // Reset tank level to 2KG
    }
    if (command == "increase")
    { // reducing interval - increases feeding rate
      if (feeder.feedInterval - FEED_RATE_CHANGE < MAX_FEED_RATE)
      {
        Serial.println("Cannot increase feeding rate, already at maximum");
        return;
      }
      else
      {
        feeder.feedInterval -= FEED_RATE_CHANGE;
        Serial.println("Increasing feeding rate by 30 minutes");
      }
    }
    if (command == "decrease")
    {
      if (feeder.feedInterval + FEED_RATE_CHANGE > MIN_FEED_RATE)
      {
        Serial.println("Cannot decrease feeding rate, already at minimum");
        return;
      }
      else
      {
        feeder.feedInterval += FEED_RATE_CHANGE;
        Serial.println("Decreasing feeding rate by 30 minutes");
      }
    }
    if (command == "default")
    {
      feeder.feedInterval = DEFAULT_FEED_RATE; // Reset to default feeding rate
      Serial.println("Feeding rate reset to default (6 times per day)");
    }
    if (command == "MQTT")
    {
      Serial.println("=== MQTT SETUP INSTRUCTIONS ===");
      Serial.println("To view MQTT messages from the Smart Cat Feeder:");
      Serial.println("1. Download Mosquitto from:");
      Serial.println("   https://mosquitto.org/download/");
      Serial.println();
      Serial.println("2. After installation, go to:");
      Serial.println("   System Properties > Environment Variables > Path");
      Serial.println("   Add the Mosquitto installation folder (e.g.):");
      Serial.println("   C:\\Program Files\\mosquitto");
      Serial.println();
      Serial.println("3. Open a new terminal window, cd to project dirrctory and run:");
      Serial.println("   mosquitto_sub -h broker.hivemq.com -t \"catfeeder/#\" -v");
      Serial.println();
      Serial.println("You will now see live feeding and alert messages from your feeder.");
      Serial.println("=====================================\n");
    }
  }
}

bool detectButtonPress()
{
  static bool lastState = HIGH;
  bool currentState = digitalRead(BUTTON_PIN);
  bool is_pressed = false;

  if (currentState == HIGH && lastState == LOW)
  {
    is_pressed = true;
  }

  lastState = currentState;
  return is_pressed;
}

void feederInit()
{
  feeder.Mode = getFeedingMode();
  feeder.portionSize = getPortionFromPot();
  feeder.bowlLevel = 0;                    // Initialize bowl level
  feeder.tankLevel = FULL_TANK;            // Initialize tank level 2KG
  feeder.feedEventsToday = 0;              // Reset feed events count
  feeder.bowlIsFull = false;               // Assume bowl is not full initially
  feeder.tankLow = false;                  // Assume tank is not low initially
  feeder.dayCycle = 0;                     // Initialize day cycle (0-23)
  feeder.lastFeedTime = millis();          // Set last feed time to current time
  feeder.feedInterval = DEFAULT_FEED_RATE; // Set default feed interval - 6 times per day
}

void functionsUpdate()
{
  FeedingMode nextMode = getFeedingMode();
  if (nextMode == SCHEDULED && feeder.Mode == MANUAL)
  {
    feeder.lastFeedTime = millis(); // Reset last feed time when switching to scheduled mode
  }
  feeder.Mode = nextMode;
  feeder.portionSize = getPortionFromPot();
  displayFunctionScreen();
  handleSerialCommands(); // checking serial every second
}