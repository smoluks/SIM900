#include "stm32f1xx.h"
#include <stdbool.h>
#include "pt.h"
#include "modem.h"
#include "gpio.h"

static struct pt rfid_pt;

void initRfid() {
	PT_INIT(&rfid_pt);
}

extern bool rfidFlag;
extern uint8_t rfidCode[5];

static uint32_t timestamp;
int processRfid() {
	PT_BEGIN(&rfid_pt)
	;

	//wait card
	PT_WAIT_UNTIL(&rfid_pt, rfidFlag);

	AccessBeeperOn();
	char result[1];
	commanderror error = sendPost("card", rfidCode, 5, result, 1);
	AccessBeeperOff();
	if (error) {
		rfidFlag = false;
		PT_RESTART(&rfid_pt);
	}

	uint8_t resultCode = result[0] - '0';
	if (resultCode) {
		play("accessdenied.amr");
		AccessLedRedOn();

		//wait
		timestamp = getSystime();
		PT_WAIT_UNTIL(&rfid_pt, checkDelay(timestamp, 5000));

		AccessLedRedOff();
	} else {
		AccessLedGreenOn();

		startProgram(999);

		PT_WAIT_WHILE(&rfid_pt, isProgramWorking());
	}

	rfidFlag = false;

PT_END(&rfid_pt);
}
