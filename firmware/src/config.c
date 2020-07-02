/*
 * config.c
 *
 *  Created on: 3 июл. 2020 г.
 *      Author: Администратор
 */
#include <string.h>
#include "config.h"
#include "eeprom.h"
#include "stm32f1xx.h"

struct config_s config;

void readConfig()
{
	__IO uint32_t *source = (uint32_t *)baseaddr;
	uint32_t *dest = (uint32_t*) &config;

	for (uint8_t i = 0; i < sizeof(config) / 4; i++)
	{
		*dest++ = *source++;
	}

	if(config.masterphone[0] == 0xFF)
		setMasterPhone("+79372389951");
}

void writeConfig()
{
	flashErasePage(baseaddr);
	flashWrite(baseaddr, (uint8_t*)&config, sizeof(config));
}

char* getMasterPhone()
{
	return config.masterphone;
}

void setMasterPhone(char *phone)
{
	stpncpy(config.masterphone, phone, 15);
	writeConfig();
}
