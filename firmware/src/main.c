#include "modem.h"
#include "call.h"
#include "sms.h"
#include "gpio.h"
#include "stm32f1xx.h"
#include "atCommands.h"
#include "logic.h"
#include "media.h"
#include "config.h"
#include "systick.h"

int main(void)
{
	LedGreenOff();

	readConfig();

	bool isInit = false;
	do
	{
		isInit = modem_init();
	} while(!isInit);

	initLogic();
	initCall();

	while(1)
	{
		WDT_RESET();

		process_call();
		processSms();
		processLogic();
		processMedia();
	}
}
