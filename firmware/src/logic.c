/*
 * logic.c
 *
 *  Created on: 30 июн. 2020 г.
 *      Author: Администратор
 */

#include "stm32f1xx.h"
#include "gpio.h"
#include "systick.h"
#include "modem.h"
#include "atCommands.h"

const uint32_t LOCK_TIMEOUT = 20 * 60 * 1000u;
const uint32_t UNLOCK_TIMEOUT = 2 * 60 * 1000u;
const uint32_t HANGUP_TIMEOUT = 20 * 1000u;

enum algstage_e {
	waitStart, waitlock, waitUnock, call
} algstage = waitStart;

bool _startFlag = false;
uint16_t _duration;
bool _stopFlag = false;

volatile uint32_t logic_timestamp;
volatile uint32_t logic_timeout_timestamp;

extern bool isPlaying;

void startProgram(uint16_t dur)
{
	_duration = dur;
	_startFlag = true;
}

void stopProgram()
{
	_stopFlag = true;
}

bool isProgramWorking()
{
	return _startFlag;
}

uint16_t getEstimateTime()
{
	uint32_t left = _duration * 60 * 1000u - (getSystime() - logic_timestamp);

	return left / 60000u;
}

void processLogic() {
	switch (algstage) {
	case waitStart:
		if (!_startFlag)
			return;

		_stopFlag = false;
		LedGreenOn();
		algstage = waitlock;
		logic_timestamp = getSystime();
	case waitlock:
		if (_stopFlag || checkDelay(logic_timestamp, LOCK_TIMEOUT)) {
			OutputDisable();
			AllLedOff();
			algstage = waitStart;
			_startFlag = false;
			_stopFlag = false;
			return;
		}
		if (!IsLocked())
			return;

		algstage = waitUnock;
		LedOrangeOn();
		logic_timestamp = getSystime();
		logic_timeout_timestamp = getSystime();
	case waitUnock:
		if (_stopFlag) {
			OutputDisable();
			AllLedOff();
			algstage = waitStart;
			_startFlag = false;
			_stopFlag = false;
			return;
		}

		if (IsLocked()) {
			OutputEnable();
			logic_timeout_timestamp = getSystime();
			LedRedOn();
		} else
			OutputDisable();

		if (!checkDelay(logic_timestamp, _duration * 60u * 1000u)
				&& !checkDelay(logic_timeout_timestamp, UNLOCK_TIMEOUT))
			return;

		OutputDisable();
		AllLedOff();

		sendcommand(getCallCommand(), 20000);
		logic_timestamp = getSystime();
		algstage = call;

		break;

	case call:
		if (!checkDelay(logic_timestamp, HANGUP_TIMEOUT))
			return;

		sendcommand(stopCallCommand, 5000);
		_startFlag = false;
		algstage = waitStart;
		break;
	}
}
