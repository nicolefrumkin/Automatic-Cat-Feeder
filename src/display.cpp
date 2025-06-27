#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "header.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void initOLED() {
  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true);
  }
  display.clearDisplay();
  display.display();
}

void displayWelcomeScreen() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Smart Cat Feeder");
    
    display.setTextSize(1);
    display.setCursor(0, 10);
    display.print("Mode: ");
    display.println(settings.Mode ? "MANUAL" : "SCHEDULED");
    
    display.setCursor(0, 20);
    display.print("Portion: ");
    display.print(settings.Portion);
    display.println("g");
    
    display.display();
}