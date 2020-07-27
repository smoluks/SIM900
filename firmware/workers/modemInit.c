#include "stm32f1xx.h"
#include "string.h"
#include "stdint.h"
#include "stdbool.h"
#include "modem.h"
#include "atCommands.h"
#include "gpio.h"
#include "systick.h"
#include "uart2.h"
#include "stringext.h"

static uint8_t cpin = 0;
static bool callReadyFlag = false;
static bool smsReadyFlag = false;

//Network registration handler
bool modemInitCommands(char *packet) {
	if (!strcmp(packet, "RDY\r\n")) {
		//readyflag = true;
		return true;
	}
	//sim ready
	else if (!strcmp(packet, "+CPIN: READY\r\n")) {
		return true;
	}
	//sim not ready
	else if (!strcmp(packet, "+CPIN: NOT READY\r\n")) {
		cpin = 1;
		return true;
	}
	//sim need pin
	else if (!strcmp(packet, "+CPIN: SIM PIN\r\n")) {
		cpin = 2;
		sendcommand(pinCommand, 5000);
		return true;
	} else if (!strcmp(packet, "Call Ready\r\n")) {
		callReadyFlag = true;
		return true;
	} else if (!strcmp(packet, "SMS Ready\r\n")) {
		smsReadyFlag = true;
		return true;
	} else
		return false;
}

bool modem_init() {
	commanderror error;

	NVIC_EnableIRQ(USART2_IRQn);
	uart2Clear();

	while (!IsModemPowerUp()) {
		WDT_RESET();

		delay(200);
		LedOrangeOff();
		delay(200);
		LedOrangeOn();
	}

	//---POWER UP---
	error = sendcommand(pingCommand, 1000);
	if (error == C_TIMEOUT) {
		ModemPowerUp();
		delay(1200);
		ModemPowerUpOff();
		delay(2200);

		error = sendcommand(pingCommand, 1000);
		if (error == C_TIMEOUT) {
			ModemPowerUp();
			delay(1200);
			ModemPowerUpOff();
			delay(2200);

			error = sendcommand(pingCommand, 1000);
			if (error == C_TIMEOUT) {
				//reset
				ModemReset();
				delay(200);
				ModemResetOff();
				delay(2700);

				error = sendcommand(pingCommand, 1000);
				if (error == C_TIMEOUT)
					return false;
			}
		}
	}

	//---UART params---
	char buffer[16];
	error = sendcommandwithanswer(getUartSpeedCommand, buffer, 16, 2200);
	if (error != C_OK || !strpartcmp(buffer, "+IPR: 115200\r\n")) {

		error = sendcommand(echoOffCommand, 2200);
		if (error == C_OK)

			error = sendcommand(shortResponceCommand, 2200);
		if (error == C_TIMEOUT)
			return false;

		error = sendcommand(uartSpeedCommand, 2200);
		if (error == C_TIMEOUT)
			return false;

		error = sendcommand(uartModeCommand, 2200);
		if (error == C_TIMEOUT)
			return false;

		error = sendcommand(saveCommand, 2200);
		if (error == C_TIMEOUT)
			return false;
		delay(200);
	}

	//---Registration---
	callReadyFlag = false;
	smsReadyFlag = false;

	//reset
	ModemReset();
	delay(200);
	ModemResetOff();
	delay(100);

	do {
		WDT_RESET();
		error = sendcommand(pingCommand, 100);
	} while (error == C_TIMEOUT);

	LedOrangeOff();

	error = sendcommand(getRegistrationStatusCommand, 5000);
	if (error == C_TIMEOUT)
		return false;

	uint32_t timestamp = getSystime();
	while (!callReadyFlag || !smsReadyFlag) {
		WDT_RESET();

		if (cpin == 1) {
			//sim not ready
			LedOrangeOn();
			LedRedOff();
			cpin = 0;
		}

		if (cpin == 2) {
			//need pin
			error = sendcommand(pinCommand, 5000);
			cpin = 0;
		}

		if (checkDelay(timestamp, 60000u))
			return false;
	}

	//Enable DTMF
	error = sendcommand("AT+DDET=1,0,0\r\n", 5000);
	if (error != C_OK && error != C_NOCODE)
		return false;

	//Broadcast SMS are not accepted
	error = sendcommand("AT+CSCB=1\r\n", 5000);
	if (error)
		return false;

	//Select SMS format
	error = sendcommand("AT+CMGF=1\r\n", 5000);
	if (error)
		return false;

	//delete all sms
	error = sendcommand("AT+CMGDA=\"DEL ALL\"\r\n", 5000);
	if (error)
		return false;

	LedRedOff();

	return true;
}
