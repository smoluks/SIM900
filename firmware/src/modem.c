/*
 * modem.c
 *
 *  Created on: 4 ����. 2018 �.
 *      Author: �������������
 */

#include "stm32f1xx.h"
#include <stdbool.h>
#include <string.h>
#include "uart2.h"
#include "delay.h"
#include "errors.h"
#include "modem.h"

#define pin "1234"
#define pincommand "AT+CPIN="pin"\r\n"

int8_t geterrorcode(char* data);

extern uint32_t tick;

bool readyflag = false;
uint8_t cpin = 0;
bool callreadyflag = false;
bool smsreadyflag = false;

//���������� ������ �� ������
bool sms_ready(char* packet)
{
	//���������� ������������� �� ������ - �������� ������ ��� �������� �������� ������
	if(!readyflag && !strcmp(packet, "RDY\r\n"))
	{
		readyflag = true;
		return true;
	}
	//����������� ��������� - �������� ������ ��� �������� �������� ������
	else if(!cpin && !strcmp(packet, "+CPIN: READY\r\n"))
	{
		cpin = 1;
		return true;
	}
	//���-����� ������� ��� - �������� ������ ��� �������� �������� ������
	else if(!cpin && !strcmp(packet, "+CPIN: SIM PIN\r\n"))
	{
		cpin = 2;
		return true;
	}
	//����� ����� ��������� ������
	else if(!callreadyflag && !strcmp(packet, "Call Ready\r\n"))
	{
		callreadyflag = true;
		return true;
	}
	//����� ����� ��������� ���
	else if(!smsreadyflag && !strcmp(packet, "SMS Ready\r\n"))
	{
		smsreadyflag = true;
		return true;
	}
	else return false;
}

//������������� ������
errorcode modem_init()
{
	char* result;

	//reset
	delay(100*1000);
	GPIOC->BSRR = 0x00008000;

	//power up
	delay(1200*1000);
	GPIOB->BSRR = 0x00000004;

	//-----�������, �������������� �������-----

	result = receive_uart2(3,2200); //RDY ��� �������

	//���������� ���
	//� SIM800 ��� ������������� /r, � SIM900 /r/n
	uint8_t count = 3;
	do
	{
		send_uart2( "ATE0\r\n");
		result = receive_uart2(11,5000);
	}
	while(!result && --count);
	if(!count)
		return ERR_NOMODEM;

	//������ ������
	count = 3;
	do
	{
		send_uart2("ATV0\r\n");
		result = receive_uart2(3, 1500);
	}
	while(!result && --count);
	if(!count)
		return ERR_NOMODEM;

	//�������� ����� - ��� ����� �� ����� RDY - ����� ���������
/*	do
	{
		send_uart2("AT+IPR=57600\r\n");
		result = receive_uart2(3, 1500);
	}
	while(!result);*/

	//����������� �������� ��������
	do
	{
		send_uart2("AT+IFC=2,2\r\n");
		result = receive_uart2(3, 1500);
	}
	while(!result);

	//-----������ �������, �������������� �� ���������� ������-----

	USART2->SR &= ~USART_SR_RXNE;
	NVIC_EnableIRQ(USART2_IRQn);
	delay(100*1000);

	//cpin
	commanderror error;
	char answerbuffer[64];
	do
	{
		error = sendcommandwithanswer("AT+CPIN?\r\n", answerbuffer, sizeof(answerbuffer), 5000);
		if(error == C_OK)
			break;
		delay(1000*1000);
	}
	while(1);
	if(!strcmp(answerbuffer, "+CPIN: SIM PIN\r\n"))
	{
		//�������� ��� ����
		sendcommand(pincommand, 5000);
	}

	//�������� ����������� � ����
	while(!smsreadyflag);

	//������������� DTMF ��������
	error = sendcommand("AT+DDET=1,0,0\r\n", 5000);
	if(error != C_OK && error != C_NOCODE)
		return ERR_DTMFINIT;

	//��� � ��������� ����
	error = sendcommand("AT+CMGF=1\r\n", 5000);

	//������ ����������������� ���������
	error = sendcommand("AT+CSCB=1\r\n", 5000);

	return 0;
}

//�������� ������� �� �����
commanderror sendcommand(char* command, uint32_t timeout)
{
	volatile uint32_t timestamp;
	char* result;

	//---��������---
	modem_sendpacket(command, 1);

	//---�����---
	do
	{
		timestamp = tick;
		do
		{
			result = modem_trygetpacket();
		} while (!result && ((tick - timestamp) < timeout));

		if (!result)
		{
			modem_dropanswercount();
			return C_TIMEOUT;
		}
		else
		{
			int8_t errorcode = geterrorcode(result);

			if (errorcode == -1)
				//������ �� ��� ������, � �����-�� ����� - ���� ������
				modem_ack(false);
			else if (errorcode == 2 || errorcode == 3)
				//��� ���� ��������, �������� �� �� ��������� - ���� ������
				modem_ack(false);
			else
			{
				modem_ack(true);
				return errorcode;
			}
		}
	} while (1);
}

//�������� ������� �� ����� � ��������� �������
commanderror sendcommandwithanswer(char* command, char* buffer, int buffersize,
		uint32_t timeout)
{
	volatile uint32_t timestamp;
	char* result;

	//---��������---
	modem_sendpacket(command, 2);

	//---�����---
	do
	{
		timestamp = tick;

		do
		{
			result = modem_trygetpacket();
		}
		while (!result && ((tick - timestamp) < timeout));

		if (!result)
		{
			modem_dropanswercount();
			return C_TIMEOUT;
		}

		int8_t errorcode = geterrorcode(result);
		if (errorcode != -1)
		{
			//������ ��� ������, � �� �����
			if (errorcode == 2 || errorcode == 3)
				//��� ���� ��������, �������� �� �� ���������
				modem_ack(false);
			else
			{
				modem_ack(true);
				modem_dropanswercount();
				return errorcode;
			}
		}

		//TODO: �������� ��������, ��� ������ ������ ����� �� ��� �������
		stpncpy(buffer, result, buffersize - 1); //�������� ����� ����
		buffer[buffersize - 1] = 0; //������� ����� ������ ��  ������ ������

		modem_ack(true);
		//---��� ������---
		do
		{
			timestamp = tick;

			do
			{
				result = modem_trygetpacket();
			}
			while (!result && ((tick - timestamp) < timeout));

			if (!result)
			{
				modem_dropanswercount();
				return C_NOCODE;
			}

			errorcode = geterrorcode(result);

			if (errorcode == -1)
				//������ �� ��� ������, � �����-�� ����� - ���� ������
				modem_ack(false);
			else if (errorcode == 2 || errorcode == 3)
				//��� ���� ��������, �������� �� �� ��������� - ���� ������
				modem_ack(false);
			else
			{
				modem_ack(true);
				return errorcode;
			}
		} while (1);

	} while (1);
}

commanderror sendcommandwith2answer(char* command, char* buffer, int buffersize,
		char* buffer2, int buffer2size, uint32_t timeout)
{
	volatile uint32_t timestamp;
	char* result;

	//---��������---
	modem_sendpacket(command, 3);

	//---����� 1---
	do
	{
		timestamp = tick;

		do
		{
			result = modem_trygetpacket();
		} while (!result && ((tick - timestamp) < timeout));

		if (!result)
		{
			modem_dropanswercount();
			return C_TIMEOUT;
		}

		int8_t errorcode = geterrorcode(result);
		if (errorcode != -1)
		{
			//������ ��� ������, � �� �����
			if (errorcode == 2 || errorcode == 3)
				//��� ���� ��������, �������� �� �� ���������
				modem_ack(false);
			else
			{
				modem_ack(true);
				modem_dropanswercount();
				return errorcode;
			}
		}

		//TODO: �������� ��������, ��� ������ ������ ����� �� ��� �������
		stpncpy(buffer, result, buffersize - 1); //�������� ����� ����
		buffer[buffersize - 1] = 0; //������� ����� ������ ��  ������ ������

		modem_ack(true);
		//---�����2---
		do
		{
			timestamp = tick;

			do
			{
				result = modem_trygetpacket();
			} while (!result && ((tick - timestamp) < timeout));

			if (!result)
			{
				modem_dropanswercount();
				return C_TIMEOUT;
			}

			int8_t errorcode = geterrorcode(result);
			if (errorcode != -1)
			{
				//������ ��� ������, � �� �����
				if (errorcode == 2 || errorcode == 3)
					//��� ���� ��������, �������� �� �� ���������
					modem_ack(false);
				else
				{
					modem_ack(true);
					modem_dropanswercount();
					return errorcode;
				}
			}

			//TODO: �������� ��������, ��� ������ ������ ����� �� ��� �������
			stpncpy(buffer2, result, buffer2size - 1); //�������� ����� ����
			buffer2[buffer2size - 1] = 0; //������� ����� ������ ��  ������ ������

			modem_ack(true);

			//---��� ������---
			do
			{
				timestamp = tick;

				do
				{
					result = modem_trygetpacket();
				} while (!result && ((tick - timestamp) < timeout));

				if (!result)
				{
					modem_dropanswercount();
					return C_NOCODE;
				}

				errorcode = geterrorcode(result);

				if (errorcode == -1)
					//������ �� ��� ������, � �����-�� ����� - ���� ������
					modem_ack(false);
				else if (errorcode == 2 || errorcode == 3)
					//��� ���� ��������, �������� �� �� ��������� - ���� ������
					modem_ack(false);
				else
				{
					modem_ack(true);
					return errorcode;
				}

			} while (1);
		} while (1);
	} while (1);
}


//����������� ����� � ����� � �����
int8_t geterrorcode(char* data)
{
	if(*data >='0' && *data <= '9' && *(data+1)== 0x0D && *(data+2)== 0x0A)
		return *data-0x30;
	else
		return -1;
}

