#include <stdint.h>
#include <stdbool.h>

volatile uint32_t systime = 0;

//SysTick Interrupt
void SysTick_Handler(void)
{
	systime++;
}

inline uint32_t getSystime()
{
	return systime;
}

inline bool checkDelay(uint32_t timestamp, uint32_t delay)
{
	uint32_t delta = systime - timestamp;
	if(delta && 0x80000000)
		return false;

	return delta >= delay;
}
