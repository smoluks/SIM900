#include "stm32f1xx.h"
#include <stdbool.h>

uint8_t ConvertASCIIToHex(uint8_t data);

enum rfidReceiveState_e {
	rfidWaitStartToken,
	rfidReceiveDataFirst,
	rfidReceiveDataSecond,
	rfidWaitStopToken
} rfidReceiveState = rfidWaitStartToken;

uint8_t rfidCode[5];
static uint8_t rfidCodeHandle;

static uint8_t temp;
static uint8_t crc;
bool rfidFlag = false;
void USART1_IRQHandler() {
	if (USART1->SR & USART_SR_RXNE || USART1->SR & USART_SR_ORE) {
		uint8_t data = USART1->DR;

		if(rfidFlag)
			return;

		switch(rfidReceiveState)
		{
		case rfidWaitStartToken:
			if(data != 0x02)
				break;

			rfidReceiveState = rfidReceiveDataFirst;
			rfidCodeHandle = 0;
			crc = 0;
			break;
		case rfidReceiveDataFirst:
			temp = ConvertASCIIToHex(data) << 4;
			rfidReceiveState = rfidReceiveDataSecond;
			break;
		case rfidReceiveDataSecond:
			temp |= ConvertASCIIToHex(data);

			if(rfidCodeHandle == 5)
			{
				if(temp == crc)
					rfidFlag = true;

				rfidReceiveState = rfidWaitStopToken;
			}
			else
			{
				rfidCode[rfidCodeHandle++] = temp;
				crc ^= temp;
				rfidReceiveState = rfidReceiveDataFirst;
			}

			break;
		case rfidWaitStopToken:
			rfidReceiveState = rfidWaitStartToken;
			break;
		}
	}

	if (USART1->SR & USART_SR_IDLE)
	{
		(void) USART1->DR;
		rfidReceiveState = rfidWaitStartToken;
	}
}

uint8_t ConvertASCIIToHex(uint8_t data)
{
	if(data >= 0x30 && data <= 0x39)
		return data - 0x30;

	if(data >= 0x41 && data <= 0x46)
			return data - 55;

	return 0;
}
