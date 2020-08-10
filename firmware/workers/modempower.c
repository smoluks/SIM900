#include "stm32f1xx.h"
#include <stdint.h>
#include <stdbool.h>
#include "gpio.h"

bool modemPowerCommandsHandler(char *packet) {
	if (!strcmp(packet, "UNDER-VOLTAGE")) {
		while (true) {
			delay(200);
			LedRedOff();
			delay(200);
			LedRedOn();
		}
	} else if (!strcmp(packet, "OVER-VOLTAGE")) {
		while (true) {
			delay(200);
			LedRedOff();
			delay(400);
			LedRedOn();
		}
	}
	return false;
}
