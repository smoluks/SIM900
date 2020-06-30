#pragma once

#include <stdint.h>

#define nop() asm("NOP")

extern void SystemInit(void);
extern uint8_t SystemCoreClockUpdate(void);

