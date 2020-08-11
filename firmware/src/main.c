#include <stdbool.h>
#include <stdint.h>
#include "modemInit.h"
#include "call.h"
#include "sms.h"
#include "gpio.h"
#include "stm32f1xx.h"
#include "atCommands.h"
#include "logic.h"
#include "media.h"
#include "config.h"
#include "systick.h"
#include "rfid.h"
#include "status.h"

int main(void)
{
	LedGreenOff();

	readConfig();

	while(!modem_init());

	initLogic();
	initCall();
	initRfid();

	NVIC_EnableIRQ(USART1_IRQn);
	//sendStatus();
	while(1)
	{
		WDT_RESET();

		processCall();
		processSms();
		processLogic();
		processMedia();
		processRfid();
		processStatus();
	}
}
