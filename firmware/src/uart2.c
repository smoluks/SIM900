/*
 * uart2.c
 *
 *  Created on: 4 февр. 2018 г.
 *      Author: Администратор
 */
#include "stm32f1xx.h"
#include <stdbool.h>
#include "call.h"
#include "errors.h"
#include "modem.h"
#include "media.h"


void uart2_rx(char data);
void uart2_endpacket();
void uart2_custompackethandlers();
void uart2_tx();

bool transmit = false; //признак передачи
char* nexttxcharptr; //ссылка на следюущий символ для передачи

char rxbuffer[64]; //буфер для принимаемых команд
char* nextrxcharptr = rxbuffer; //указатель на место для следующего символа

extern int tick;
extern errorcode error;

void USART2_IRQHandler()
{
	if (USART2->SR & USART_SR_ORE)
	{
		//данные приходят быстрее, чем мы успеваем их обрабатывать
		error = ERR_UARTOVERRUN;
	}
	if (USART2->SR & USART_SR_IDLE)
	{
		//не было новых данных в течение времени одного байта
	}
	if (USART2->SR & USART_SR_RXNE)
	{
		//принят байт
		uart2_rx(USART2->DR);
	}
	if (transmit && (USART2->SR & USART_SR_TXE))
	{
		//готовность к передаче следующего байта
		uart2_tx();
	}
}

uint8_t waitpacketcount = 0; //сколько пакетов ожидается в ответ на команду
bool packetreceived = false; //флаг окончания приема пакета
char lastdata; //предыдущий принятый байт

//обработчик принятого байта
void uart2_rx(char data)
{
	*nextrxcharptr++ = data; //записываем в буффер
	//TODO: добавить проверку на переполнение буффера
	if(data == 0x0A && lastdata== 0x0D)
	{
		//конец пакета
		*nextrxcharptr = 0; //помечаем конец строки
		uart2_endpacket();
		nextrxcharptr = rxbuffer;
	}
	else lastdata = data;
}

//обработчик приема пакета
void uart2_endpacket()
{
	if(waitpacketcount)
	{
		//если мы ждем ответ
		GPIOA->BSRR = 0x0002; //выставляем RTS, чтобы модем не сыпал нам новые данные
		USART2->CR1 &= ~USART_CR1_RXNEIE; //отключаем обработку
		packetreceived = true; //ставим флаг готовности пакета
	}
	else
		//отдаем обработчикам команд
		uart2_custompackethandlers();
}

//обработчики команд не ответов
void uart2_custompackethandlers()
{
	if (!sms_ready(rxbuffer))
		if (!callcommands(rxbuffer))
			if (!mediacommands(rxbuffer))
				smscommands(rxbuffer);
}

//обработчик передачи байта
void uart2_tx()
{
	char c = *nexttxcharptr++;
	if(!c)
	{
		//посылка кончилась
		USART2->CR1 &= ~USART_CR1_TXEIE;
		transmit = false;
	}
	else
	{
		while(GPIOA->IDR & 0x01); //если модем выставил cts, ждем, пока он его не сбросит
		USART2->DR = c;
	}
}


//----------обмен по прерываниям---------

//отправить пакет
//answercount - количество ожидаемых посылок в ответ
void modem_sendpacket(char* data, int answercount)
{
	USART2->DR = *data++;
	nexttxcharptr = data;
	transmit = true;
	USART2->CR1 |= USART_CR1_TXEIE;
	waitpacketcount = answercount;
}

//считать принятый пакет, если есть
//если нет, вернется null
char* modem_trygetpacket()
{
	if(!packetreceived)
		return 0;
	else
	{
		packetreceived = false;
		return rxbuffer;
	}
}

//сбросить количество ожидаемых посылок в ответ
void modem_dropanswercount()
{
	if(waitpacketcount)
		waitpacketcount = 0;
}

//подтвердить, что мы ждем именно этот пакет
//должно вызываться после modem_trygetpacket, если ответ не null
void modem_ack(bool neededpacket)
{
	if(neededpacket)
	{
		//это ожидаемый пакет
		if(waitpacketcount)
			waitpacketcount--;
	}
	else
		//отдаем обработчикам команд
		uart2_custompackethandlers();

	USART2->CR1 |= USART_CR1_RXNEIE; //включаем обработку
	GPIOA->BSRR = 0x00020000; //сбрасываем RTS
}

//----------обмен по флагам, не по прерываниям---------
//отправить данные
void send_uart2(char* data)
{
	do
	{
		char c = *data++;
		if(!c)
			break;
		while(!(USART2->SR & USART_SR_TXE));
		USART2->DR = c;
	}
	while(1);

	USART2->SR &= ~USART_SR_RXNE;
}

//принять данные
char* receive_uart2(int count, int timeout)
{
	char* pointer = rxbuffer;
	int t=tick+timeout;

	while(count--)
	{
		while(!(USART2->SR & USART_SR_RXNE) && (t - tick)>0);
		if(USART2->SR & USART_SR_RXNE)
			*pointer++ = USART2->DR;
		else
			return 0;
	}

	*pointer = 0;

	return rxbuffer;
}




