#include "modem.h"
#include "call.h"
#include "sms.h"
#include "gpio.h"
#include "stm32f1xx.h"
#include "atCommands.h"
#include "logic.h"
#include "media.h"

int main(void)
{
	LedGreenOff();

	bool isInit = false;
	do
	{
		isInit = modem_init();
	} while(!isInit);

	while(1)
	{
		process_call();
		processSms();
		processLogic();
		processMedia();
	}
}
