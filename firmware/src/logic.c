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

enum algstage_e {
	waitCall, waitlock, waitUnock
} algstage = waitCall;

bool _startFlag = false;
uint16_t _duration;
bool _stopFlag = false;

uint32_t timestamp;
uint32_t timestamp2;

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
	//сколько прошло минут
	uint16_t estimate = (getSystime() - timestamp) / 60000;

	if(!((getSystime() - timestamp) % 60000))
		estimate++;

	return _duration >=  estimate ? _duration - estimate : 0;
}

void processLogic() {
	switch (algstage) {
	case waitCall:
		if (!_startFlag)
			return;

		_stopFlag = false;
		LedGreenOn();
		algstage = waitlock;
		timestamp = getSystime();
	case waitlock:
		if (_stopFlag || checkDelay(timestamp, LOCK_TIMEOUT)) {
			OutputDisable();
			AllLedOff();
			algstage = waitCall;
			_startFlag = false;
			_stopFlag = false;
			return;
		}
		if (!IsLocked())
			return;

		algstage = waitUnock;
		LedOrangeOn();
		timestamp = getSystime();
		timestamp2 = getSystime();
	case waitUnock:
		if (_stopFlag) {
			OutputDisable();
			AllLedOff();
			algstage = waitCall;
			_startFlag = false;
			_stopFlag = false;
			return;
		}

		if (IsLocked()) {
			OutputEnable();
			timestamp2 = getSystime();
			LedRedOn();
		} else
			OutputDisable();

		if (!checkDelay(timestamp, _duration * 60u * 1000u)
				&& !checkDelay(timestamp2, UNLOCK_TIMEOUT))
			return;

		algstage = waitCall;
		OutputDisable();
		AllLedOff();

		sendcommand(getCallCommand(), 20000);
		delay(20000);
		sendcommand(stopCallCommand, 5000);

		_startFlag = false;
	}
}
