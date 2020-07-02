/*
 * sms.c
 *
 *  Created on: 19 ����. 2018 �.
 *      Author: �������������
 */

#include "stm32f1xx.h"
#include "modem.h"
#include "string.h"
#include "stdio.h"
#include "stringext.h"
#include "uart2.h"
#include "atCommands.h"

void sendstatussms();

bool sms = false;
char messagenumber[5];
//���������� ������ �� ������
bool smscommands(char* packet)
{
	if (strpartcmp(packet, "+CMTI: \"SM\""))
	{
		stpncpy(messagenumber, packet+12, sizeof(messagenumber) - 1);
		sms = true;
		return true;
	}
	return false;
}

void processSms()
{
	if(sms)
	{
		sms = false;

		char play[100];
		char sms[164];
		//read sms
		snprintf(play, sizeof(play), "%s%s%s", "AT+CMGR=", messagenumber, ",0\r\n");
		commanderror result = sendcommandwith2answer(play, play, sizeof(play), sms, sizeof(sms), 5000);
		if(result != C_OK)
				return;

		if(strpartcmp(sms, "get status"))
			sendstatussms();

		snprintf(play, sizeof(play), "%s%s%s", "AT+CMGD=", messagenumber, ",0\r\n");
		sendcommand(play, 5000);
	}
}

void sendstatussms()
{
	char buffer[20];

	commanderror result = sendcommandwithanswer("AT+CSQ\r\n", buffer, sizeof(buffer), 5000);
	if(result != C_OK)
		return;

	char buffer2[50];
	result = sendcommandwithanswer("AT+COPS?\r\n", buffer2, sizeof(buffer2), 5000);
	if(result != C_OK)
		return;

	NVIC_DisableIRQ(USART2_IRQn);

	send_uart2(smscommand);

	char* answer;
	do
	{
		answer = receive_uart2(3,2200);
	}
	while(answer[2] != '>');

	send_uart2("provider: ");
	send_uart2(buffer2+7);
	send_uart2("level: ");
	send_uart2(buffer+6);

	char end[] = {0x1A, 0x00};
	send_uart2(end);

	USART2->SR &= ~USART_SR_RXNE;
	NVIC_EnableIRQ(USART2_IRQn);
}


