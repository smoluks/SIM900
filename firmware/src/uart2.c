/*
 * uart2.c
 *
 *  Created on: 4 ����. 2018 �.
 *      Author: �������������
 */
#include "stm32f1xx.h"
#include <stdbool.h>
#include "call.h"
#include "modem.h"
#include "media.h"


void uart2_rx(char data);
void uart2_endpacket();
void uart2_custompackethandlers();
void uart2_tx();

bool transmit = false; //������� ��������
char* nexttxcharptr; //������ �� ��������� ������ ��� ��������

char rxbuffer[64]; //����� ��� ����������� ������
char* nextrxcharptr = rxbuffer; //��������� �� ����� ��� ���������� �������

extern int tick;
void USART2_IRQHandler()
{
	if (USART2->SR & USART_SR_ORE)
	{
		//������ �������� �������, ��� �� �������� �� ������������
	}
	if (USART2->SR & USART_SR_IDLE)
	{
		//�� ���� ����� ������ � ������� ������� ������ �����
	}
	if (USART2->SR & USART_SR_RXNE)
	{
		//������ ����
		uart2_rx(USART2->DR);
	}
	if (transmit && (USART2->SR & USART_SR_TXE))
	{
		//���������� � �������� ���������� �����
		uart2_tx();
	}
}

uint8_t waitpacketcount = 0; //������� ������� ��������� � ����� �� �������
bool packetreceived = false; //���� ��������� ������ ������
char lastdata; //���������� �������� ����

//���������� ��������� �����
void uart2_rx(char data)
{
	*nextrxcharptr++ = data; //���������� � ������
	//TODO: �������� �������� �� ������������ �������
	if(data == 0x0A && lastdata== 0x0D)
	{
		//����� ������
		*nextrxcharptr = 0; //�������� ����� ������
		uart2_endpacket();
		nextrxcharptr = rxbuffer;
	}
	else lastdata = data;
}

//���������� ������ ������
void uart2_endpacket()
{
	if(waitpacketcount)
	{
		//���� �� ���� �����
		GPIOA->BSRR = 0x0002; //���������� RTS, ����� ����� �� ����� ��� ����� ������
		USART2->CR1 &= ~USART_CR1_RXNEIE; //��������� ���������
		packetreceived = true; //������ ���� ���������� ������
	}
	else
		//������ ������������ ������
		uart2_custompackethandlers();
}

//����������� ������ �� �������
void uart2_custompackethandlers()
{
	if (!sms_ready(rxbuffer))
		if (!callcommands(rxbuffer))
			if (!mediacommands(rxbuffer))
				smscommands(rxbuffer);
}

//���������� �������� �����
void uart2_tx()
{
	char c = *nexttxcharptr++;
	if(!c)
	{
		//������� ���������
		USART2->CR1 &= ~USART_CR1_TXEIE;
		transmit = false;
	}
	else
	{
		while(GPIOA->IDR & 0x01); //���� ����� �������� cts, ����, ���� �� ��� �� �������
		USART2->DR = c;
	}
}


//----------����� �� �����������---------

//��������� �����
//answercount - ���������� ��������� ������� � �����
void modem_sendpacket(char* data, int answercount)
{
	USART2->DR = *data++;
	nexttxcharptr = data;
	transmit = true;
	USART2->CR1 |= USART_CR1_TXEIE;
	waitpacketcount = answercount;
}

//������� �������� �����, ���� ����
//���� ���, �������� null
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

//�������� ���������� ��������� ������� � �����
void modem_dropanswercount()
{
	if(waitpacketcount)
		waitpacketcount = 0;
}

//�����������, ��� �� ���� ������ ���� �����
//������ ���������� ����� modem_trygetpacket, ���� ����� �� null
void modem_ack(bool neededpacket)
{
	if(neededpacket)
	{
		//��� ��������� �����
		if(waitpacketcount)
			waitpacketcount--;
	}
	else
		//������ ������������ ������
		uart2_custompackethandlers();

	USART2->CR1 |= USART_CR1_RXNEIE; //�������� ���������
	GPIOA->BSRR = 0x00020000; //���������� RTS
}

//----------����� �� ������, �� �� �����������---------
//��������� ������
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

//������� ������
char* receive_uart2(int count, int timeout)
{
	char* pointer = rxbuffer;
	uint32_t timestamp = getSystime();

	while(count--)
	{
		while(!(USART2->SR & USART_SR_RXNE) && !checkDelay(timestamp, timeout));
		if(USART2->SR & USART_SR_RXNE)
			*pointer++ = USART2->DR;
		else
			return 0;
	}

	*pointer = 0;

	return rxbuffer;
}




