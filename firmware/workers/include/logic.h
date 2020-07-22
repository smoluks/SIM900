#define LOCK_TIMEOUT 20 * 60 * 1000u
#define UNLOCK_TIMEOUT 2 * 60 * 1000u
#define HANGUP_TIMEOUT 20 * 1000u

void startProgram(uint16_t dur);
void stopProgram();
bool isProgramWorking();
uint16_t getEstimateTime();

void initLogic();
int processLogic();
