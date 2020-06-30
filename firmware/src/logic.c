/*
 * logic.c
 *
 *  Created on: 30 июн. 2020 г.
 *      Author: Администратор
 */

#include "stm32f1xx.h"
#include "gpio.h"
#include "systick.h"
#include "atCommands.h"

const uint32_t LOCK_TIMEOUT = 20 * 60 * 1000u;
const uint32_t UNLOCK_TIMEOUT = 2 * 60 * 1000u;


void alg(uint16_t duration)
{
	//stage 1
	LedGreenOn();

 	uint32_t timestamp = getSystime();
	while(!IsLocked() && !checkDelay(timestamp, LOCK_TIMEOUT));

	if(!IsLocked())
	{
		OutputDisable();
		AllLedOff();
		return;
	}

	//stage 2
	LedOrangeOn(); //orange on

	timestamp = getSystime();
	uint32_t timestamp2 = getSystime();

	while(!checkDelay(timestamp, duration*60u*1000u))
	{
		IsLocked() ? OutputEnable() : OutputDisable();


		if(IsLocked())
		{
			LedRedOn();
			timestamp2 = getSystime();
		}
		else
		{
			if(checkDelay(timestamp2, UNLOCK_TIMEOUT))
				break;
		}
	}

	//end
	OutputDisable();
	AllLedOff();

	sendcommand(callCommand, 20000);
	delay(5000);
	sendcommand(stopCallCommand, 5000);
}
