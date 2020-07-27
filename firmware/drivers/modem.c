#include "stm32f1xx.h"
#include "uart2.h"
#include "modem.h"
#include "systick.h"
#include "gpio.h"
#include "string.h"
#include "pt-sem.h"

int8_t geterrorcode(char *data);

commanderror sendcommand(char *command, uint32_t timeout) {
	modemSendPacket(command);

	char *result = modemGetPacket(timeout, true);
	if (!result)
		return C_TIMEOUT;

	int8_t errorcode = geterrorcode(result);
	return errorcode;
}

commanderror sendcommandwithanswer(char *command, char *buffer, int buffersize,
		uint32_t timeout) {
	modemSendPacket(command);

	char *result = modemGetPacket(timeout, false);
	if (!result)
		return C_TIMEOUT;

	int8_t errorcode = geterrorcode(result);
	if (errorcode != -1)
		return errorcode;

	stpncpy(buffer, result, buffersize - 1);
	buffer[buffersize - 1] = 0;

	result = modemGetPacket(timeout, true);
	if (!result)
		return C_TIMEOUT;

	errorcode = geterrorcode(result);
	return errorcode;

}

commanderror sendcommandwith2answer(char *command, char *buffer, int buffersize,
		char *buffer2, int buffer2size, uint32_t timeout) {

	modemSendPacket(command);

	char *result = modemGetPacket(timeout, false);
	if (!result)
		return C_TIMEOUT;

	int8_t errorcode = geterrorcode(result);
	if (errorcode != -1)
		return errorcode;

	stpncpy(buffer, result, buffersize - 1);
	buffer[buffersize - 1] = 0;

	result = modemGetPacket(timeout, false);
	if (!result)
		return C_TIMEOUT;

	errorcode = geterrorcode(result);
	if (errorcode != -1)
		return errorcode;

	stpncpy(buffer2, result, buffer2size - 1);
	buffer2[buffer2size - 1] = 0;

	result = modemGetPacket(timeout, true);
	if (!result)
		return C_TIMEOUT;

	errorcode = geterrorcode(result);
	return errorcode;

	//return 0;
}

//------------data commands----------------------
commanderror sendCommandWithData(char *command, uint32_t timeout) {
	modemSendPacket(command);

	bool result = waitDataMarker(command, timeout);
	if (!result)
		return C_TIMEOUT;

	return C_OK;
}

void sendData(char *data) {
	modemSendData(data);
}

commanderror getDataAnswer(char *buffer, int buffersize, uint32_t timeout) {
	char *result = modemGetPacket(timeout, false);
	if (!result)
		return C_TIMEOUT;

	int8_t errorcode = geterrorcode(result);
	if (errorcode != -1)
		return errorcode;

	if(buffer){
		stpncpy(buffer, result, buffersize - 1);
		buffer[buffersize - 1] = 0;
	}

	result = modemGetPacket(timeout, true);
	if (!result)
		return C_TIMEOUT;

	errorcode = geterrorcode(result);
	return errorcode;
}


int8_t geterrorcode(char *data) {
	if (*data >= '0' && *data <= '9' && *(data + 1) == 0x0D
			&& *(data + 2) == 0x0A)
		return *data - 0x30;
	else
		return -1;
}

