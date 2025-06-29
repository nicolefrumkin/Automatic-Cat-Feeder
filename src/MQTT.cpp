#include "header.h"

void setupWiFi() {
  Serial.print("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected!");
}

void setupMQTT() {
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);

  while (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT...");
    if (mqttClient.connect("ESP32CatFeeder")) {
      Serial.println(" connected!\n");
      mqttClient.publish(MQTT_TOPIC_STATUS, "Smart Cat Feeder initialized.");
    } else {
      Serial.print(" failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5s");
      delay(5000);
    }
  }
}

void printMQTTInstructions() {
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
  Serial.println("3. Open a new terminal window and run:");
  Serial.println("   mosquitto_sub -h broker.hivemq.com -t \"catfeeder/#\" -v");
  Serial.println();
  Serial.println("You will now see live feeding and alert messages from your feeder.");
  Serial.println("=====================================\n");
}

