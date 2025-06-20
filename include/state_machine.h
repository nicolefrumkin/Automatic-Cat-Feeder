// System state management
void setFeedingMode(FeedingMode mode);
void setSystemState(SystemState state);
FeedingMode getCurrentMode();
SystemState getCurrentState();
void handleStateTransition();
void updateSystemState();
void checkInputs();
void handleModeSwitch();

// Main workflow controllers
void runScheduledMode();
void runManualMode();
void performDailyMaintenance();
void handleCriticalAlerts();