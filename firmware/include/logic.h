void startProgram(uint16_t dur);
void stopProgram();
bool isProgramWorking();
uint16_t getEstimateTime();

void processLogic();

enum algstage_e {
	waitStart, waitlock, waitUnock, call
};
