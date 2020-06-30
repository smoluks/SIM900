#include "modem.h"
#include "call.h"
#include "sms.h"
#include "gpio.h"
#include "stm32f1xx.h"

int main(void)
{
	LedGreenOff();

	modem_init();

	while(1)
	{
		process_call();
		processSms();
	}
}
