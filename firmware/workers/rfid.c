#include "stm32f1xx.h"
#include <stdbool.h>
#include "pt.h"
#include "modem.h"
#include "gpio.h"
#include "http.h"
#include "systick.h"
#include "media.h"
#include "logic.h"

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

	uint8_t result[2];
	commanderror error = sendPost("card", rfidCode, 5, result, 2);
	AccessBeeperOff();
	if (error) {
		rfidFlag = false;
		PT_RESTART(&rfid_pt);
	}

	uint16_t minutes = (result[1] << 8) + result[0];
	if (!minutes) {
		play("accessdenied.amr", true);
		AccessLedRedOn();

		//wait
		timestamp = getSystime();
		PT_WAIT_UNTIL(&rfid_pt, checkDelay(timestamp, 5000));

		AccessLedRedOff();
	} else {
		AccessLedGreenOn();

		startProgram(minutes, true);

		PT_WAIT_WHILE(&rfid_pt, isProgramWorking());
	}

	rfidFlag = false;

	PT_END(&rfid_pt);
}

void sendWaste(uint16_t duration){
	uint8_t data[7];
	data[0] = rfidCode[0];
	data[1] = rfidCode[1];
	data[2] = rfidCode[2];
	data[3] = rfidCode[3];
	data[4] = rfidCode[4];
	data[5] = duration & 0xFF;
	data[6] = duration >> 8;

	commanderror error;
	do {
		error = sendPost("card/writeoff", data, 7, 0, 0);
		if(error)
		{
			delay(10000);
		}
	}
	while(error);
}
