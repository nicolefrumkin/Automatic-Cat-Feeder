#include "header.h"

void handleSerialInput(String command) {
  Serial.print("Processing command: ");
  Serial.println(command);

  if (command == "status") {
    // printSystemStatus();
  }
}