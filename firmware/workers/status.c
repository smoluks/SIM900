#include "stm32f1xx.h"
#include <stdbool.h>
#include "modem.h"

static uint32_t timestamp;
void processStatus(){
	if(!checkDelay(timestamp, 60000))
		return;

	sendStatus();
	timestamp = getSystime();
}

void sendStatus()
{
	char buffer[20];
	commanderror result = sendcommandwithanswer("AT+CSQ\r\n", buffer, sizeof(buffer), 5000);
	if (result != C_OK)
		return;

	uint8_t payload[1];
	payload[0] =  atoi(buffer + 6);

	sendPost("status", payload, 1, 0, 0);
}
