#include "string.h"
#include "stdio.h"
#include "stm32f1xx.h"
#include "modem.h"
#include "stringext.h"
#include "uart2.h"
#include "atCommands.h"
#include "config.h"

void sendstatussms();
void setphone(char sms[]);

static bool sms = false;
static char messagenumber[5];

bool smsHandler(char *packet) {
	if (strpartcmp(packet, "+CMTI: \"SM\"")) {

		//copy input number
		stpncpy(messagenumber, packet + 12, sizeof(messagenumber) - 1);
		for (uint8_t i = 0; i < 5; i++) {
			if (messagenumber[i] == '\r' || messagenumber[i] == '\n')
				messagenumber[i] = 0;
		}

		sms = true;
		return true;
	}
	return false;
}

static char phonenumber[13];
static char command[100];
static char smstext[164];
void processSms() {
	if(!sms)
		return;

	sms = false;

	//read sms
	snprintf(command, sizeof(command), "%s%s%s", "AT+CMGR=", messagenumber,
			",0\r\n");

	if(sendcommandwith2answer(command, command, sizeof(command), smstext, sizeof(smstext), 5000))
		return;

	stpncpy(phonenumber, command + 21, sizeof(phonenumber) - 1);

	//Deleting SMS Messages from Message Storage
	if(sendcommand("AT+CMGDA=\"DEL ALL\"\r\n", 5000))
		return;

	if (strpartcmp(smstext, "get status"))
		sendstatussms();

	if (strpartcmp(smstext, "set phone"))
		setphone(smstext);
}

void sendstatussms() {
	char buffer[20];

	commanderror result = sendcommandwithanswer("AT+CSQ\r\n", buffer,
			sizeof(buffer), 5000);
	if (result != C_OK)
		return;

	char buffer2[50];
	result = sendcommandwithanswer("AT+COPS?\r\n", buffer2, sizeof(buffer2),
			5000);
	if (result != C_OK)
		return;

	NVIC_DisableIRQ(USART2_IRQn);

	char command[100];

	snprintf(command, sizeof(command), "%s%s%s", "AT+CMGS=\"", phonenumber,
			"\"\r\n");
	send_uart2(command);

	char *answer;
	do {
		answer = receive_uart2(3, 2200);
	} while (answer[2] != '>');

	send_uart2("provider: ");
	send_uart2(buffer2 + 7);
	send_uart2("level: ");
	send_uart2(buffer + 6);
	send_uart2("master phone: ");
	send_uart2(getMasterPhone());

	char end[] = { 0x1A, 0x00 };
	send_uart2(end);

	USART2->SR &= ~USART_SR_RXNE;
	NVIC_EnableIRQ(USART2_IRQn);

	getDataAnswer(0, 0, 10000);
}

void setphone(char sms[]) {
	if (strlen(sms) < 22)
		return;

	char phone[16];
	stpncpy(phone, sms + 10, sizeof(phone) - 1);

	setMasterPhone(phone);

	NVIC_DisableIRQ(USART2_IRQn);

	char command[100];

	snprintf(command, sizeof(command), "%s%s%s", "AT+CMGS=\"", phonenumber,
			"\"\r\n");
	send_uart2(command);

	char *answer;
	do {
		answer = receive_uart2(1, 5000);
	} while (answer[0] != '>');

	send_uart2("set master phone ");
	send_uart2(phone);
	send_uart2("ok");

	char end[] = { 0x1A, 0x00 };
	send_uart2(end);

	USART2->SR &= ~USART_SR_RXNE;
	NVIC_EnableIRQ(USART2_IRQn);

	getDataAnswer(0, 0, 10000);
}

