#include "stm32f1xx.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "stringext.h"
#include "modem.h"
#include "media.h"
#include "main.h"
#include "call.h"
#include "systick.h"
#include "atCommands.h"
#include "logic.h"
#include "pt.h"

static bool callFlag = false;
static bool hangUpFlag = false;
static uint8_t dtmf = 0;

bool callcommands(char *packet) {
	if (!strcmp(packet, "2\r\n")) {
		//Ring
		callFlag = true;
		hangUpFlag = false;
		return true;
	} else if (!strcmp(packet, "3\r\n")) {
		callFlag = false;
		return true;
	} else if (strpartcmp(packet, "+DTMF:")) {
		dtmf = packet[7];
		return true;
	}
	return false;
}

static uint32_t timestamp;
static uint16_t passwd = 0;
static uint8_t passSymbolCount = 0;
static uint8_t wrongpasscount;
static uint16_t duration;
static struct pt call_pt;

extern bool isPlaying;

void initCall() {
	PT_INIT(&call_pt);
}

int processCall() {
	PT_BEGIN(&call_pt)
	;

	//wait call
	PT_WAIT_UNTIL(&call_pt, callFlag);
	dtmf = 0;

	if (sendcommand(pickUpCommand, 5000)) {
		PT_RESTART(&call_pt);
	}

	play("main.amr");

	passwd = 0;
	passSymbolCount = 0;
	wrongpasscount = 3;
	duration = 0;
	timestamp = getSystime();

	do {
		//wait dtmf
		PT_WAIT_WHILE(&call_pt,
				callFlag && !dtmf && !checkDelay(timestamp, 10000));

		stop();

		if (!callFlag) {
			PT_RESTART(&call_pt);
		}
		if (checkDelay(timestamp, 10000)) {
			stop();
			sendcommand(hangUpCommand, 5000);
			callFlag = false;
			PT_RESTART(&call_pt);
		}

		timestamp = getSystime();

		if (dtmf == '*') {
			passwd = 0;
			passSymbolCount = 0;
			dtmf = 0;
			continue;
		} else if (dtmf >= '0' && dtmf <= '9') {
			passwd = passwd * 10 + (dtmf - '0');
			passSymbolCount++;
		}

		if (passSymbolCount == sizeof(PASSWORD)) {
			if (passwd != PASSWORD) {
				if (!--wrongpasscount) {
					stop();
					sendcommand(hangUpCommand, 5000);
					callFlag = false;
					PT_RESTART(&call_pt);
				} else {
					play("badpassword.amr");
					passSymbolCount = 0;
					passwd = 0;
					dtmf = 0;
					continue;
				}
			} else {
				dtmf = 0;
				break;
			}
		}

		dtmf = 0;
	} while (true);

	if (!isProgramWorking()) {
		//-----start-----
		play("delay.amr");
		timestamp = getSystime();

		do {
			PT_WAIT_WHILE(&call_pt,
					callFlag && !dtmf && !checkDelay(timestamp, 5000));
			stop();

			if (!callFlag) {
				PT_RESTART(&call_pt);
			}
			if (checkDelay(timestamp, 5000)) {
				stop();
				sendcommand(hangUpCommand, 5000);
				callFlag = false;
				PT_RESTART(&call_pt);
			}

			timestamp = getSystime();

			if (dtmf == '*') {
				duration = 0;
				continue;
			} else if (dtmf == '#') {
				break;
			} else if (dtmf >= '0' && dtmf <= '9') {
				duration = duration * 10 + (dtmf - '0');
				if (duration > 999) {
					stop();
					sendcommand(hangUpCommand, 5000);
					callFlag = false;
					PT_RESTART(&call_pt);
				}
			}

			dtmf = 0;
		} while (true);

		startProgram(duration);
		play("start.amr");

		PT_WAIT_WHILE(&call_pt, callFlag && isPlaying);

		if (callFlag) {
			stop();
			sendcommand(hangUpCommand, 5000);
			callFlag = false;
		}

	} else {
		//-----stop-----
		play("current.amr");

		uint16_t estimateTime = getEstimateTime();
		char *file = "0.amr";

		uint8_t n = estimateTime % 1000 / 100;
		if (n) {
			file[0] = n + '0';
			playNext(file);
		}

		uint8_t n2 = estimateTime % 100 / 10;
		if (n || n2) {
			file[0] = n2 + '0';
			playNext(file);
		}

		n = estimateTime % 10;
		file[0] = n + '0';
		playNext(file);

		playNext("stop.amr");

		timestamp = getSystime();
		PT_WAIT_WHILE(&call_pt,
				callFlag && !dtmf && !checkDelay(timestamp, 15000));
		stop();

		if (dtmf == '#') {
			stopProgram();
			play("stopsuccessful.amr");
			PT_WAIT_WHILE(&call_pt, callFlag && isPlaying);
			if (callFlag) {
				sendcommand(hangUpCommand, 5000);
				callFlag = false;
			}
		}

		if (callFlag) {
			stop();
			sendcommand(hangUpCommand, 5000);
			callFlag = false;
		}
	}

PT_END(&call_pt);
}
