#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "header.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void initOLED()
{
  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println(F("SSD1306 allocation failed"));
    while (true)
      ;
  }
  display.clearDisplay();
  display.display();
}

void displayWelcomeScreen()
{
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(20, 0);
  display.println("Welcome");

  display.setTextSize(1);
  display.setCursor(25, 30);
  display.print("Mode: ");
  display.println(feeder.Mode ? "MANUAL" : "SCHEDULED");

  display.setCursor(25, 40);
  display.print("Portion: ");
  display.print(feeder.portionSize);
  display.println("g");

  display.display();
}

void displayFunctionScreen()
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Feedes today: " + String(feeder.feedEventsToday));

  // Mode
  display.setCursor(0, 9);
  display.print("Mode: ");
  display.println(feeder.Mode ? "MANUAL" : "SCHEDULED");

  // Portion size
  display.setCursor(0, 18);
  display.print("Portion:");
  display.print(feeder.portionSize);
  display.println("g");

  // Bowl weight
  display.setCursor(0, 27);
  display.print("Bowl:");
  display.print(feeder.bowlLevel);
  display.print("g");

  // Tank amount
  display.print(" Tank:");
  display.print(feeder.tankLevel);
  display.println("g");

  // Current time and day
  display.setCursor(0, 36);
  display.print("Time:");
  display.print(formatTime(millis() - feeder.dayCycle * DAY_CYCLE_MS)); // Example: "12:34"
  display.print(" Day:");
  display.println(feeder.dayCycle); // Example: "12:34"

  // Next feed time (only for SCHEDULED mode)
  if (feeder.Mode == SCHEDULED)
  {
    display.setCursor(0, 45);
    display.print("Next:");
    display.println(formatTime(feeder.lastFeedTime + feeder.feedInterval)); // Implement this or format feeder.nextFeedTime
    display.setCursor(0, 54);
    display.print("Interval:");
    display.println(formatTime(feeder.feedInterval)); // Implement this or format feeder.nextFeedTime
  }

  display.display();
}

// Helper function to format time
String formatTime(unsigned long milliseconds)
{
  unsigned long seconds = milliseconds / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;

  seconds = seconds % 60;
  minutes = minutes % 60;
  hours = hours % 24;

  String timeStr = "";
  if (minutes < 10)
    timeStr += "0";
  timeStr += String(minutes);
  timeStr += ":";
  if (seconds < 10)
    timeStr += "0";
  timeStr += String(seconds);

  return timeStr;
}

void displayAlert()
{
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("ALERT");
  display.setTextSize(1);
  display.setCursor(0, 20);
  display.println("Low intake today.");
  Serial.println("⚠️  ALERT:");
  Serial.println("Low intake today.\n");
  display.display();
  mqttClient.publish(MQTT_TOPIC_ALERT, "Alert: Low intake today.\n");
}
