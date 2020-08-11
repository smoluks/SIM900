#include "stm32f1xx.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "modem.h"
#include "atCommands.h"
#include "gpio.h"
#include "systick.h"
#include "uart2.h"
#include "stringext.h"
#include "modemInit.h"

static uint8_t cpin = 0;
static bool callReadyFlag = false;
static bool smsReadyFlag = false;


struct providerSettings_s Tele2ProviderSettings = {
		"internet.tele2.ru",
		"tele2",
		"tele2"
};

struct providerSettings_s MegafonProviderSettings = {
		"internet",
		"gdata",
		"gdata"
};


//Network registration handler
bool modemInitCommands(uint8_t *packet) {
	if (!strcmp((char*)packet, "RDY\r\n")) {
		//readyflag = true;
		return true;
	}
	//sim ready
	else if (!strcmp((char*)packet, "+CPIN: READY\r\n")) {
		return true;
	}
	//sim not ready
	else if (!strcmp((char*)packet, "+CPIN: NOT READY\r\n")) {
		cpin = 1;
		return true;
	}
	//sim need pin
	else if (!strcmp((char*)packet, "+CPIN: SIM PIN\r\n")) {
		cpin = 2;
		sendcommand(pinCommand, 5000);
		return true;
	} else if (!strcmp((char*)packet, "Call Ready\r\n")) {
		callReadyFlag = true;
		return true;
	} else if (!strcmp((char*)packet, "SMS Ready\r\n")) {
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
	bool needSave = false;
	char buffer[16];
	error = sendcommandwithanswer(getUartSpeedCommand, buffer, 16, 2200);
	if (error != C_OK || !strpartcmp(buffer, "+IPR: 115200\r\n"))
		needSave = true;

	sendcommand(echoOffCommand, 2200);

	error = sendcommand(shortResponceCommand, 2200);
	if (error == C_TIMEOUT)
		return false;

	error = sendcommand(uartSpeedCommand, 2200);
	if (error == C_TIMEOUT)
		return false;

	error = sendcommand(uartModeCommand, 2200);
	if (error == C_TIMEOUT)
		return false;

	if (needSave) {
		error = sendcommand(saveCommand, 2200);
		if (error == C_TIMEOUT)
			return false;
	}

	delay(200);

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
	error = sendcommand("AT+DDET=1,0,0\r\n", 15000);
	if (error != C_OK && error != C_NOCODE)
		return false;

	//Broadcast SMS are not accepted
	error = sendcommand("AT+CSCB=1\r\n", 15000);
	if (error)
		return false;

	//Select SMS format
	error = sendcommand("AT+CMGF=1\r\n", 15000);
	if (error)
		return false;

	//delete all sms
	error = sendcommand("AT+CMGDA=\"DEL ALL\"\r\n", 15000);
	if (error)
		return false;

	//Attach from GPRS Service
	do {
		error = sendcommand("AT+CGATT=1\r\n", 15000);
		if (!error)
			break;

		delay(100);
	} while (true);

	//Enable TCP normal mode
	error = sendcommand("AT+CIPMODE=0\r\n", 15000);
	if (error)
		return false;

	//Monosocket
	error = sendcommand("AT+CIPMUX=0\r\n", 15000);
	if (error)
		return false;

	error = sendcommand("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r\n", 15000);
	if (error)
		return false;

	char command[64];
	snprintf(command, sizeof(command), "%s%s%s", "AT+SAPBR=3,1,\"APN\",\"", MegafonProviderSettings.apn, "\"\r\n");
	error = sendcommand(command, 15000);
	if (error)
		return false;

	snprintf(command, sizeof(command), "%s%s%s", "AT+SAPBR=3,1,\"USER\",\"", MegafonProviderSettings.login, "\"\r\n");
	error = sendcommand(command, 15000);
	if (error)
		return false;

	snprintf(command, sizeof(command), "%s%s%s", "AT+SAPBR=3,1,\"PWD\",\"", MegafonProviderSettings.password, "\"\r\n");
	error = sendcommand(command, 15000);
	if (error)
		return false;

	error = sendcommand("AT+SAPBR=1,1\r\n", 15000);
	if (error)
		return false;

	error = sendcommand("AT+HTTPINIT\r\n", 15000);
	if (error)
		return false;

	error = sendcommand("AT+HTTPPARA=\"CID\",1\r\n", 15000);
	if (error)
		return false;

	LedRedOff();

	return true;
}
