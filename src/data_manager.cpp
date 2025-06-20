// implement these:
// Data logging and history management
void logFeedingEvent(String mode, int quantity);
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