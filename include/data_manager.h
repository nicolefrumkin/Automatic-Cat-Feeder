// Data logging and history management
void initializeDataManager();
void logFeedingEvent(String mode, int quantity);
void updateLastFeedingEvent(float bowlWeightAfter, float consumed, unsigned long eatingDuration);
void printFeedingHistory();

int getFeedingCount24h();
float getTotalFoodConsumed24h();

// Settings management
SystemSettings* getSettings();
void updateSetting(String setting, float value);