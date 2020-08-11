#include "stm32f1xx.h"
#include "gpio.h"
#include "systick.h"
#include "modem.h"
#include "atCommands.h"
#include "logic.h"
#include "pt.h"
#include "rfid.h"

bool workCycle();

static bool _startFlag = false;
static uint16_t _duration;
static bool _stopFlag = false;
static bool _rfid = false;
static struct pt logic_pt;
static volatile uint32_t logic_timestamp;
static volatile uint32_t logic_timeout_timestamp;

extern bool isPlaying;

void startProgram(uint16_t dur, bool rfid) {
	_duration = dur;
	_startFlag = true;
	_rfid = rfid;
}

void stopProgram() {
	_stopFlag = true;
}

bool isProgramWorking() {
	return _startFlag;
}

uint16_t getEstimateTime() {
	uint32_t left = _duration * 60 * 1000u - (getSystime() - logic_timestamp);

	return left / 60000u;
}

void initLogic() {
	PT_INIT(&logic_pt);
}

int processLogic() {
	PT_BEGIN(&logic_pt)
	;

	//wait start
	PT_WAIT_UNTIL(&logic_pt, _startFlag);

	_stopFlag = false;
	LedGreenOn();
	logic_timestamp = getSystime();

	//wait lock
	PT_WAIT_UNTIL(&logic_pt,
			IsLocked() || _stopFlag || checkDelay(logic_timestamp, LOCK_TIMEOUT));
	if (_stopFlag || checkDelay(logic_timestamp, LOCK_TIMEOUT)) {
		OutputDisable();
		AllLedOff();
		_startFlag = false;
		_stopFlag = false;
		PT_RESTART(&logic_pt);
	}

	LedOrangeOn();
	logic_timestamp = getSystime();
	logic_timeout_timestamp = getSystime();

	//wait unlock
	PT_WAIT_WHILE(&logic_pt, workCycle());

	OutputDisable();
	AllLedOff();

	if (_stopFlag) {
		_startFlag = false;
		_stopFlag = false;
		PT_RESTART(&logic_pt);
	}

	if (_rfid) {
		sendWaste((getSystime() - logic_timestamp - UNLOCK_TIMEOUT) / 60000);
	} else {
		if (sendcommand(getCallCommand(), 20000)) {
			_startFlag = false;
			_stopFlag = false;
			PT_RESTART(&logic_pt);
		}

		logic_timestamp = getSystime();

		PT_WAIT_UNTIL(&logic_pt, checkDelay(logic_timestamp, HANGUP_TIMEOUT));

		sendcommand(stopCallCommand, 5000);
	}
	_startFlag = false;
	_stopFlag = false;

PT_END(&logic_pt);
}

bool workCycle() {
if (_stopFlag)
	return false;

if (IsLocked()) {
	OutputEnable();
	logic_timeout_timestamp = getSystime();
	LedRedOn();
} else
	OutputDisable();

return (!checkDelay(logic_timestamp, _duration * 60u * 1000u)
		&& !checkDelay(logic_timeout_timestamp, UNLOCK_TIMEOUT));
}
