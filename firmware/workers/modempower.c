#include "stm32f1xx.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "gpio.h"
#include "systick.h"

bool modemPowerCommandsHandler(uint8_t *packet) {
	if (!strcmp((char*)packet, "UNDER-VOLTAGE")) {
		while (true) {
			delay(200);
			LedRedOff();
			delay(200);
			LedRedOn();
		}
	} else if (!strcmp((char*)packet, "OVER-VOLTAGE")) {
		while (true) {
			delay(200);
			LedRedOff();
			delay(400);
			LedRedOn();
		}
	}
	return false;
}
