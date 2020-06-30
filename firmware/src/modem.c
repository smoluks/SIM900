/*
 * modem.c
 *
 *  Created on: 4 февр. 2018 г.
 *      Author: Администратор
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

//обработчик команд от модема
bool sms_ready(char* packet)
{
	//завершение инициализации ОС модема - работает только при заданной скорости обмена
	if(!readyflag && !strcmp(packet, "RDY\r\n"))
	{
		readyflag = true;
		return true;
	}
	//регистрация завершена - работает только при заданной скорости обмена
	else if(!cpin && !strcmp(packet, "+CPIN: READY\r\n"))
	{
		cpin = 1;
		return true;
	}
	//сим-карта треубет пин - работает только при заданной скорости обмена
	else if(!cpin && !strcmp(packet, "+CPIN: SIM PIN\r\n"))
	{
		cpin = 2;
		return true;
	}
	//модем готов принимать вызовы
	else if(!callreadyflag && !strcmp(packet, "Call Ready\r\n"))
	{
		callreadyflag = true;
		return true;
	}
	//модем готов принимать СМС
	else if(!smsreadyflag && !strcmp(packet, "SMS Ready\r\n"))
	{
		smsreadyflag = true;
		return true;
	}
	else return false;
}

//инициализация модема
errorcode modem_init()
{
	char* result;

	//reset
	delay(100*1000);
	GPIOC->BSRR = 0x00008000;

	//power up
	delay(1200*1000);
	GPIOB->BSRR = 0x00000004;

	//-----команды, обрабатываемые нативно-----

	result = receive_uart2(3,2200); //RDY или таймаут

	//отключение эха
	//У SIM800 эхо заканчивается /r, у SIM900 /r/n
	uint8_t count = 3;
	do
	{
		send_uart2( "ATE0\r\n");
		result = receive_uart2(11,5000);
	}
	while(!result && --count);
	if(!count)
		return ERR_NOMODEM;

	//ошибки цифрой
	count = 3;
	do
	{
		send_uart2("ATV0\r\n");
		result = receive_uart2(3, 1500);
	}
	while(!result && --count);
	if(!count)
		return ERR_NOMODEM;

	//скорость порта - без этого не будет RDY - нужно сохранять
/*	do
	{
		send_uart2("AT+IPR=57600\r\n");
		result = receive_uart2(3, 1500);
	}
	while(!result);*/

	//программный контроль передачи
	do
	{
		send_uart2("AT+IFC=2,2\r\n");
		result = receive_uart2(3, 1500);
	}
	while(!result);

	//-----дальше команды, обрабатываемые на подсистеме команд-----

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
		//отправка пин кода
		sendcommand(pincommand, 5000);
	}

	//ожидание регистрации в сети
	while(!smsreadyflag);

	//инициализация DTMF декодера
	error = sendcommand("AT+DDET=1,0,0\r\n", 5000);
	if(error != C_OK && error != C_NOCODE)
		return ERR_DTMFINIT;

	//смс в текстовом виде
	error = sendcommand("AT+CMGF=1\r\n", 5000);

	//запрет широковещательных сообщений
	error = sendcommand("AT+CSCB=1\r\n", 5000);

	return 0;
}

//отправка команды на модем
commanderror sendcommand(char* command, uint32_t timeout)
{
	volatile uint32_t timestamp;
	char* result;

	//---отправка---
	modem_sendpacket(command, 1);

	//---ответ---
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
				//пришел не код ошибки, а какая-то фигня - ждем дальше
				modem_ack(false);
			else if (errorcode == 2 || errorcode == 3)
				//это коды звонилки, оставить ей на обработку - ждем дальше
				modem_ack(false);
			else
			{
				modem_ack(true);
				return errorcode;
			}
		}
	} while (1);
}

//отправка команды на модем с текстовым ответом
commanderror sendcommandwithanswer(char* command, char* buffer, int buffersize,
		uint32_t timeout)
{
	volatile uint32_t timestamp;
	char* result;

	//---отправка---
	modem_sendpacket(command, 2);

	//---ответ---
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
			//пришел код ошибки, а не ответ
			if (errorcode == 2 || errorcode == 3)
				//это коды звонилки, оставить ей на обработку
				modem_ack(false);
			else
			{
				modem_ack(true);
				modem_dropanswercount();
				return errorcode;
			}
		}

		//TODO: добавить проверку, что пришел именно ответ на эту команду
		stpncpy(buffer, result, buffersize - 1); //копируем ответ себе
		buffer[buffersize - 1] = 0; //признак конца строки на  всякий случай

		modem_ack(true);
		//---код ошибки---
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
				//пришел не код ошибки, а какая-то фигня - ждем дальше
				modem_ack(false);
			else if (errorcode == 2 || errorcode == 3)
				//это коды звонилки, оставить ей на обработку - ждем дальше
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

	//---отправка---
	modem_sendpacket(command, 3);

	//---ответ 1---
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
			//пришел код ошибки, а не ответ
			if (errorcode == 2 || errorcode == 3)
				//это коды звонилки, оставить ей на обработку
				modem_ack(false);
			else
			{
				modem_ack(true);
				modem_dropanswercount();
				return errorcode;
			}
		}

		//TODO: добавить проверку, что пришел именно ответ на эту команду
		stpncpy(buffer, result, buffersize - 1); //копируем ответ себе
		buffer[buffersize - 1] = 0; //признак конца строки на  всякий случай

		modem_ack(true);
		//---ответ2---
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
				//пришел код ошибки, а не ответ
				if (errorcode == 2 || errorcode == 3)
					//это коды звонилки, оставить ей на обработку
					modem_ack(false);
				else
				{
					modem_ack(true);
					modem_dropanswercount();
					return errorcode;
				}
			}

			//TODO: добавить проверку, что пришел именно ответ на эту команду
			stpncpy(buffer2, result, buffer2size - 1); //копируем ответ себе
			buffer2[buffer2size - 1] = 0; //признак конца строки на  всякий случай

			modem_ack(true);

			//---код ошибки---
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
					//пришел не код ошибки, а какая-то фигня - ждем дальше
					modem_ack(false);
				else if (errorcode == 2 || errorcode == 3)
					//это коды звонилки, оставить ей на обработку - ждем дальше
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


//преобразует ответ с кодом в цифру
int8_t geterrorcode(char* data)
{
	if(*data >='0' && *data <= '9' && *(data+1)== 0x0D && *(data+2)== 0x0A)
		return *data-0x30;
	else
		return -1;
}

