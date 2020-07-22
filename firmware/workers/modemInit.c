#include "stm32f1xx.h"
#include "string.h"
#include "stdint.h"
#include "stdbool.h"
#include "modem.h"
#include "atCommands.h"
#include "gpio.h"
#include "systick.h"
#include "uart2.h"

static uint8_t cpin = 0;
static bool callReadyFlag = false;
static bool smsReadyFlag = false;

//Network registration handler
bool modemInitCommands(char *packet)
{
	if (!strcmp(packet, "RDY\r\n"))
	{
		//readyflag = true;
		return true;
	}
	//sim ready
	else if (!strcmp(packet, "+CPIN: READY\r\n"))
	{
		return true;
	}
	//sim not ready
	else if (!strcmp(packet, "+CPIN: NOT READY\r\n"))
	{
		cpin = 1;
		return true;
	}
	//sim need pin
	else if (!strcmp(packet, "+CPIN: SIM PIN\r\n"))
	{
		cpin = 2;
		sendcommand(pinCommand, 5000);
		return true;
	}
	else if (!strcmp(packet, "Call Ready\r\n"))
	{
		callReadyFlag = true;
		return true;
	}
	else if (!strcmp(packet, "SMS Ready\r\n"))
	{
		smsReadyFlag = true;
		return true;
	}
	else
		return false;
}

bool modem_init()
{
	NVIC_DisableIRQ(USART2_IRQn);
	uart2Clear();

	while (!IsModemPowerUp())
		;
	delay(200);

	//power up
	ModemPowerUp();
	delay(1200);
	ModemPowerUpOff();

	//reset
	ModemReset();
	delay(200);
	ModemResetOff();

	send_uart2("AT\r\n");
	delay(500);

	//PING
	uint8_t count = 3;
	char *result;
	do
	{
		WDT_RESET();

		send_uart2("ATE0\r\n");
		result = receive_uart2(11, 5000);
	} while (!result && --count);
	if (!count)
		return false;

	//Short responce
	count = 3;
	do
	{
		WDT_RESET();

		send_uart2("ATV0\r\n");
		result = receive_uart2(3, 1500);
	} while (!result && --count);
	if (!count)
		return false;

	//Uart speed
	/*	do
	 {
	 send_uart2("AT+IPR=57600\r\n");
	 result = receive_uart2(3, 1500);
	 }
	 while(!result);*/

	//use RTS CTS
	count = 3;
	do
	{
		WDT_RESET();

		send_uart2("AT+IFC=2,2\r\n");
		result = receive_uart2(3, 1500);
	} while (!result && --count);
	if (!count)
		return false;

	//---Registration---
	USART2->SR &= ~USART_SR_RXNE;
	NVIC_EnableIRQ(USART2_IRQn);
	commanderror error;

	LedOrangeOff();
	delay(200);

	uint32_t timestamp = getSystime();
	error = sendcommand("AT+CPIN?\r\n", 5000);

	while (!callReadyFlag || !smsReadyFlag)
	{
		WDT_RESET();

		if (cpin == 1)
		{
			//error = sendcommand("AT+CPIN?\r\n", 5000);
			cpin = 0;
		}

		if (cpin == 2)
		{
			error = sendcommand(pinCommand, 5000);
			cpin = 0;
		}

		if(checkDelay(timestamp, 30000u))
			return false;
	}

	//Enable DTMF
	error = sendcommand("AT+DDET=1,0,0\r\n", 5000);
	if (error != C_OK && error != C_NOCODE)
		return false;

	//Broadcast SMS are not accepted
	error = sendcommand("AT+CSCB=1\r\n", 5000);
	if (error)
		return false;

	//Select SMS format
	error = sendcommand("AT+CMGF=1\r\n", 5000);
	if (error)
		return false;

	//delete all sms
	error = sendcommand("AT+CMGDA=\"DEL ALL\"\r\n", 5000);
	if (error)
		return false;

	LedRedOff();

	return true;
}
