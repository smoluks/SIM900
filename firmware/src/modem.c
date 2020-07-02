#include "stm32f1xx.h"
#include "uart2.h"
#include "modem.h"
#include "systick.h"
#include "gpio.h"
#include "string.h"

#define pin "0000"
#define pincommand "AT+CPIN="pin"\r\n"

int8_t geterrorcode(char *data);

//bool readyflag = false;
uint8_t cpin = 0;
bool callReadyFlag = false;
bool smsReadyFlag = false;

bool modem_init()
{
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

	//PING
	uint8_t count = 3;
	char *result;
	do
	{
		send_uart2("ATE0\r\n");
		result = receive_uart2(11, 5000);
	} while (!result && --count);
	if (!count)
		return false;

	//Short responce
	count = 3;
	do
	{
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
		if (cpin == 1)
		{
			//error = sendcommand("AT+CPIN?\r\n", 5000);
			cpin = 0;
		}

		if (cpin == 2)
		{
			error = sendcommand(pincommand, 5000);
			cpin = 0;
		}

		if(checkDelay(timestamp, 30000u))
			return false;
	}

	//Enable DTMF
	error = sendcommand("AT+DDET=1,0,0\r\n", 5000);
	if (error != C_OK && error != C_NOCODE)
		return false;

	//Select SMS format
	error = sendcommand("AT+CMGF=1\r\n", 5000);
	if (error)
		return false;

	//Broadcast SMS are not accepted
	error = sendcommand("AT+CSCB=1\r\n", 5000);
	if (error)
		return false;

	LedRedOff();

	return true;
}

//Network registration handler
bool sms_ready(char *packet)
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
		sendcommand(pincommand, 5000);
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

//�������� ������� �� �����
commanderror sendcommand(char *command, uint32_t timeout)
{
	volatile uint32_t timestamp;
	char *result;

	//---��������---
	modem_sendpacket(command, 1);

	//---�����---
	do
	{
		timestamp = getSystime();
		do
		{
			result = modem_trygetpacket();
		} while (!result && !checkDelay(timestamp, timeout));

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
commanderror sendcommandwithanswer(char *command, char *buffer, int buffersize,
		uint32_t timeout)
{
	volatile uint32_t timestamp;
	char *result;

	//---��������---
	modem_sendpacket(command, 2);

	//---�����---
	do
	{
		timestamp = getSystime();

		do
		{
			result = modem_trygetpacket();
		} while (!result && !checkDelay(timestamp, timeout));

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
			timestamp = getSystime();

			do
			{
				result = modem_trygetpacket();
			} while (!result && !checkDelay(timestamp, timeout));

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

commanderror sendcommandwith2answer(char *command, char *buffer, int buffersize,
		char *buffer2, int buffer2size, uint32_t timeout)
{
	volatile uint32_t timestamp;
	char *result;

	//---��������---
	modem_sendpacket(command, 3);

	//---����� 1---
	do
	{
		timestamp = getSystime();

		do
		{
			result = modem_trygetpacket();
		} while (!result && !checkDelay(timestamp, timeout));

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
			timestamp = getSystime();

			do
			{
				result = modem_trygetpacket();
			} while (!result && !checkDelay(timestamp, timeout));

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
				timestamp = getSystime();

				do
				{
					result = modem_trygetpacket();
				} while (!result && !checkDelay(timestamp, timeout));

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
int8_t geterrorcode(char *data)
{
	if (*data >= '0' && *data <= '9' && *(data + 1) == 0x0D
			&& *(data + 2) == 0x0A)
		return *data - 0x30;
	else
		return -1;
}

