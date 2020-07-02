/*
 * eeprom.c
 *
 *  Created on: 3 июл. 2020 г.
 *      Author: Администратор
 */

#include "stm32f1xx.h"
#include "stdbool.h"

void flashErasePage(uint32_t address)
{
	while (FLASH->SR & FLASH_SR_BSY)
		;

	if (FLASH->SR & FLASH_SR_EOP)
	{
		FLASH->SR = FLASH_SR_EOP;
	}

	FLASH->CR |= FLASH_CR_PER;
	FLASH->AR = address;
	FLASH->CR |= FLASH_CR_STRT;
	while (!(FLASH->SR & FLASH_SR_EOP))
		;
	FLASH->SR = FLASH_SR_EOP;
	FLASH->CR &= ~FLASH_CR_PER;
}

void flashWrite(uint32_t address, uint8_t *data, uint32_t count)
{
	uint32_t i;

	while (FLASH->SR & FLASH_SR_BSY)
		;
	if (FLASH->SR & FLASH_SR_EOP)
	{
		FLASH->SR = FLASH_SR_EOP;
	}

	FLASH->CR |= FLASH_CR_PG;

	for (i = 0; i < count; i += 2)
	{
		*(volatile unsigned short*) (address + i) = (((unsigned short) data[i
				+ 1]) << 8) + data[i];
		while (!(FLASH->SR & FLASH_SR_EOP))
			;
		FLASH->SR = FLASH_SR_EOP;
	}

	FLASH->CR &= ~(FLASH_CR_PG);
}

uint32_t flashRead(uint32_t address)
{
	return (*(__IO uint32_t*) address);
}
