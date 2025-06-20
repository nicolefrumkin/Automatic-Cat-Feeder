// Serial communication
void printSystemStatus();
void printFeedingLog();
void handleSerialCommands();
void debugOutput();

// MQTT functions
void initializeMQTT();
void connectMQTT();
void publishFeedingEvent();
void publishSystemStatus();
void publishAlert();
void publishWeightData();
void handleMQTTReconnect();