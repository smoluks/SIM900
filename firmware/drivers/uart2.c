#include "stm32f1xx.h"
#include <stdbool.h>
#include <string.h>
#include "call.h"
#include "modem.h"
#include "media.h"
#include "sms.h"
#include "systick.h"
#include "modemInit.h"

void uart2Rx(char data);
void uart2ProcessPacket();
void uart2Tx();

bool transmit = false;

void USART2_IRQHandler() {
	if (USART2->SR & USART_SR_ORE) {
		//overrun - read as fast as can
		uart2Rx(USART2->DR);
	}
	if (USART2->SR & USART_SR_IDLE) {
		//use as packet end?
	}
	if (USART2->SR & USART_SR_RXNE) {
		//read
		uart2Rx(USART2->DR);
	}
	if (transmit && (USART2->SR & USART_SR_TXE)) {
		//write
		uart2Tx();
	}
}

static char rxbuffer[64];
static char *nextrxcharptr = rxbuffer;
static char lastdata;

bool packetreceived = false;
static char answer[64];

//read uart
void uart2Rx(char data) {
	*nextrxcharptr++ = data;

	//EOP?
	if (data == 0x0A && lastdata == 0x0D) {
		GPIOA->BSRR = 0x0002; //set RTS
		USART2->CR1 &= ~USART_CR1_RXNEIE;

		*nextrxcharptr = 0;
		uart2ProcessPacket();

		nextrxcharptr = rxbuffer;

		USART2->CR1 |= USART_CR1_RXNEIE;
		GPIOA->BSRR = 0x00020000; //clr RTS
	} else
		lastdata = data;
}

void uart2ProcessPacket() {
	if (modemInitCommands(rxbuffer)) {
		return;
	}

	if (callcommands(rxbuffer)) {
		return;
	}

	if (mediacommands(rxbuffer)) {
		return;
	}

	if (smsHandler(rxbuffer)) {
		return;
	}

	uint32_t timestamp = getSystime();
	while (packetreceived && !checkDelay(timestamp, 5000u))
	{
		WDT_RESET();
	}

	if(packetreceived)
	{
		timestamp = getSystime();
	}

	strncpy(answer, rxbuffer, 63);
	packetreceived = true;
}

char *nexttxcharptr;

void uart2Tx() {
	char c = *nexttxcharptr++;
	if (!c) {
		USART2->CR1 &= ~USART_CR1_TXEIE;
		transmit = false;
	} else {
		while (GPIOA->IDR & 0x01)
			; //CTS
		USART2->DR = c;
	}
}

void uart2Clear()
{
	nextrxcharptr = rxbuffer;
	packetreceived = false;

	USART2->CR1 |= USART_CR1_RXNEIE;
	GPIOA->BSRR = 0x00020000; //clr RTS
}

//----------packet mode - new ---------
//
void modemSendPacket(char *data) {
	USART2->DR = *data++;
	nexttxcharptr = data;
	transmit = true;
	USART2->CR1 |= USART_CR1_TXEIE;

	packetreceived = false;
}

//
char* modemGetPacket(uint32_t timeout) {
	uint32_t timestamp = getSystime();
	while (!packetreceived && !checkDelay(timestamp, timeout))
	{
		WDT_RESET();
	}

	if (!packetreceived)
		return 0;
	else {
		packetreceived = false;
		return answer;
	}
}

//----------direct mode - old---------
//send data
void send_uart2(char *data) {
	do {
		char c = *data++;
		if (!c)
			break;

		while (!(USART2->SR & USART_SR_TXE)) {
			WDT_RESET();
		}
		USART2->DR = c;
	} while (1);

	USART2->SR &= ~USART_SR_RXNE;
}

//receive data
char* receive_uart2(uint8_t count, uint32_t timeout) {
	char *pointer = rxbuffer;
	uint32_t timestamp = getSystime();

	while (count--) {
		while (!(USART2->SR & USART_SR_RXNE) && !checkDelay(timestamp, timeout)) {
			WDT_RESET();
		}

		if (USART2->SR & USART_SR_RXNE)
			*pointer++ = USART2->DR;
		else
			return 0;
	}

	*pointer = 0;

	return rxbuffer;
}

