extern unsigned long lastSensorUpdate;

// Weight sensor management
void initializeWeightSensors();
float readBowlWeight();
float readTankWeight();
void calibrateWeightSensors();
bool isBowlFull();
bool isBowlEmpty();
bool isTankLow();
float calculateFoodConsumed();

// Bowl monitoring functions
void monitorBowlStatus();
unsigned long getBowlEmptyDuration();
void trackEatingDuration();
void detectOverfeeding();
void preventDispenseIfFull();

// Sensor update and simulation functions
void updateSensorReadings();
void simulateEating();
void printSensorStatus();

// Helper functions for testing
void addFoodToBowl(float amount);
void refillTank();