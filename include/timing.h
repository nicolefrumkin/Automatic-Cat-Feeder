// Time management and scheduling
void initializeRTC();
String getCurrentTime();
String getNextFeedTime();
void setFeedingSchedule(String time);
bool isScheduledFeedTime();
void updateDayCycle();
unsigned long getTimestamp();
bool isFeedingTime();