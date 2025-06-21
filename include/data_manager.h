// Data logging and history management
void initializeDataManager();
void logFeedingEvent(String mode, int quantity);
void updateLastFeedingEvent(float bowlWeightAfter, float consumed, unsigned long eatingDuration);
void printFeedingHistory();
void saveFeedingData();
void loadFeedingData();
int getFeedingCount24h();
float getTotalFoodConsumed24h();

// Memory management
void saveToEEPROM();
void loadFromEEPROM();
void clearMemory();
void backupSettings();
void loadSettings();

// Settings management
SystemSettings* getSettings();
void updateSetting(String setting, float value);