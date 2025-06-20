// Hardware initialization and basic I/O
void initializeHardware();
void initializeOLED();
void initializeServo();
void setStatusLED(bool state);
void blinkStatusLED(int times);

// Input reading functions
int readPotentiometer();
bool readFeedButton();
bool readModeSwitch();
String readSerialInput();

// Hardware utility functions
void resetSystem();
void handleHardwareFailure();
bool validateSensorReadings();
void emergencyStop();