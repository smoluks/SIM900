#include <stdint.h>
#include <stdbool.h>
#include "systick.h"

volatile uint32_t systime = 0;

//SysTick Interrupt
void SysTick_Handler(void)
{
	systime++;
}

uint32_t getSystime()
{
	return systime;
}

bool checkDelay(uint32_t timestamp, uint32_t delay)
{
	uint32_t delta = systime - timestamp;
	if(delta & 0x80000000)
		return false;

	return delta >= delay;
}

void delay(uint32_t delay)
{
	volatile uint32_t timestamp = systime;

	while(!checkDelay(timestamp, delay));
}
