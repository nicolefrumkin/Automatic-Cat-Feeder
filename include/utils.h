#include <Arduino.h>

// Utility and helper functions
String formatTimestamp(unsigned long time);
String formatWeight(float weight);
int mapPotToGrams(int potValue);
void playFeedingSound();
void delay_ms(int milliseconds);
bool isWithinRange(float value, float min, float max);
void printPeriodicStatus();
