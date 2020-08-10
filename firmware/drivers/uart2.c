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
		(void) USART2->DR;
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

bool packetreceived = true;
bool dataMarkerReceived = true;
bool downloadMarkerReceived = true;
static char answer[64];

//read uart
void uart2Rx(char data) {
	*nextrxcharptr++ = data;

	if (!dataMarkerReceived && data == '>') {
		dataMarkerReceived = true;
		nextrxcharptr = rxbuffer;
	}
	if (!downloadMarkerReceived && nextrxcharptr == rxbuffer + 10) {
		if (rxbuffer[0] == 'D'
				&& rxbuffer[1] == 'O'
				&& rxbuffer[2] == 'W'
				&& rxbuffer[3] == 'N'
				&& rxbuffer[4] == 'L'
				&& rxbuffer[5] == 'O'
				&& rxbuffer[6] == 'A'
				&& rxbuffer[7] == 'D')
			downloadMarkerReceived = true;
		nextrxcharptr = rxbuffer;
	}

	else if (data == 0x0A && lastdata == 0x0D) {
		//EOP
		GPIOA->BSRR = 0x0002; //set RTS

		*nextrxcharptr = 0;
		uart2ProcessPacket();

		nextrxcharptr = rxbuffer;

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

	if (httpHandler(rxbuffer)) {
		return;
	}

	if (modemPowerCommandsHandler(rxbuffer)) {
			return;
	}

	if (!packetreceived) {
		strncpy(answer, rxbuffer, 63);
		packetreceived = true;
	}
}

static char txBuffer[256];
volatile char *txBufferWritePtr = txBuffer;
volatile char *txBufferReadPtr = txBuffer;

void uart2Tx() {
	if (txBufferWritePtr == txBufferReadPtr) {
		//end
		USART2->CR1 &= ~USART_CR1_TXEIE;
		transmit = false;
		return;
	}

	transmit = true;
	char c = *txBufferReadPtr++;
	while (GPIOA->IDR & 0x01) //CTS
	{
		WDT_RESET();
	}
	USART2->DR = c;
	USART2->CR1 |= USART_CR1_TXEIE;

	if (txBufferReadPtr == txBuffer + 256)
		txBufferReadPtr = txBuffer;
}

//----------packet mode - new ---------
//
void modemSendPacket(char *data) {
	do {
		char current = *data++;
		if (!current)
			break;

		*txBufferWritePtr++ = current;
		if (txBufferWritePtr == txBuffer + 256)
			txBufferWritePtr = txBuffer;
	} while (true);

	if (!transmit)
		uart2Tx();

	packetreceived = false;
}

void modemSendData(char *data) {
	do {
		char current = *data++;
		if (!current)
			break;

		*txBufferWritePtr++ = current;
		if (txBufferWritePtr == txBuffer + 256)
			txBufferWritePtr = txBuffer;
	} while (true);

	if (!transmit)
		uart2Tx();
}

void modemSendBinaryData(char *data, uint8_t count) {
	do {
		if (!count--)
			break;

		char current = *data++;

		*txBufferWritePtr++ = current;
		if (txBufferWritePtr == txBuffer + 256)
			txBufferWritePtr = txBuffer;
	} while (true);

	if (!transmit)
		uart2Tx();
}

commanderror waitDataMarker(char *data, uint32_t timeout) {
	dataMarkerReceived = false;

	uint32_t timestamp = getSystime();
	while (!dataMarkerReceived && !packetreceived
			&& !checkDelay(timestamp, timeout)) {
		WDT_RESET();
	}
	if (dataMarkerReceived)
		return C_OK;
	else if (packetreceived) {
		packetreceived = false;
		return C_ERROR;
	} else
		return C_TIMEOUT;
}

commanderror waitDownloadMarker(char *data, uint32_t timeout) {
	downloadMarkerReceived = false;

	uint32_t timestamp = getSystime();
	while (!downloadMarkerReceived && !packetreceived
			&& !checkDelay(timestamp, timeout)) {
		WDT_RESET();
	}
	if (downloadMarkerReceived)
		return C_OK;
	else if (packetreceived) {
		packetreceived = false;
		return C_ERROR;
	} else
		return C_TIMEOUT;
}

//
char* modemGetPacket(uint32_t timeout, bool lastPacket) {
	uint32_t timestamp = getSystime();
	while (!packetreceived && !checkDelay(timestamp, timeout)) {
		WDT_RESET();
	}

	if (!packetreceived) {
		packetreceived = true;
		return 0;
	} else {
		packetreceived = lastPacket;
		return answer;
	}
}

//----------direct mode - old---------
void uart2Clear() {
	nextrxcharptr = rxbuffer;
	packetreceived = true;
	dataMarkerReceived = true;
	downloadMarkerReceived = true;
	USART2->SR &= ~USART_SR_RXNE;
	GPIOA->BSRR = 0x00020000; //clr RTS
}

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

