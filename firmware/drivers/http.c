#include <stdbool.h>
#include "modem.h"
#include "systick.h"
#include "stringext.h"
#include "http.h"

struct httpAnswer_s postResponce;
struct httpAnswer_s getResponce;

#define BASE_PATH "http://www.smoluks.ru:5001/1/"

bool httpHandler(char *packet) {
	if (strpartcmp(packet, "+HTTPACTION: 1")) {
		if (!postResponce.received) {
			postResponce.received = true;
			postResponce.code = atoi(packet + 15);
			postResponce.answerLength = atoi(packet + 19);
		}
		return true;
	} else if (strpartcmp(packet, "+HTTPACTION: 0")) {
		if (!getResponce.received) {
			getResponce.received = true;
			getResponce.code = atoi(packet + 15);
			getResponce.answerLength = atoi(packet + 19);
		}
		return true;
	} else
		return false;
}

char command[128];
void sendGet(char *path, uint8_t *result, uint8_t resultLength) {
	getResponce.received = false;

	//make path
	snprintf(command, sizeof(command), "AT+HTTPPARA=\"URL\",\"%s%s\"\r\n",
	BASE_PATH, path);
	commanderror error = sendcommand(command, 2200);
	if (error)
		return error;

	//start query
	error = sendcommand("AT+HTTPACTION=0\r\n", 2200);
	if (error)
		return error;

	//wait answer
	uint32_t timestamp = getSystime();
	while (!getResponce.received && !checkDelay(timestamp, REQUEST_TIMEOUT)) {
		WDT_RESET();
	}
	if (checkDelay(timestamp, REQUEST_TIMEOUT))
		return C_TIMEOUT;
	if (postResponce.code != 200 || postResponce.answerLength != resultLength)
		return C_ERROR;

	//get answer
	uint8_t buffer[64];
	error = getTcpData(result, resultLength, 2200);
	if (error)
		return error;

	return C_OK;
}

commanderror sendPost(char *path, uint8_t *data, uint8_t length,
		uint8_t *result, uint8_t resultLength) {
	postResponce.received = false;

	//make path
	snprintf(command, sizeof(command), "AT+HTTPPARA=\"URL\",\"%s%s\"\r\n",
	BASE_PATH, path);
	commanderror error = sendcommand(command, 2200);
	if (error)
		return error;

	//make payload
	char snum[5];
	itoa(length, snum, 10);
	snprintf(command, sizeof(command), "AT+HTTPDATA=%s,5000\r\n", snum);
	error = sendCommandWithDownload(command, 2200);
	if (error)
		return error;

	sendBinaryData(data, length);

	error = getDownloadAnswer(2200);
	if (error)
		return error;

	//start query
	error = sendcommand("AT+HTTPACTION=1\r\n", 2200);
	if (error)
		return error;

	//wait answer
	uint32_t timestamp = getSystime();
	while (!postResponce.received && !checkDelay(timestamp, REQUEST_TIMEOUT)) {
		WDT_RESET();
	}
	if (checkDelay(timestamp, REQUEST_TIMEOUT))
		return C_TIMEOUT;
	if (postResponce.code != 200 || postResponce.answerLength != resultLength)
		return C_ERROR;

	//get answer
	if(resultLength)
	{
		error = getTcpData(result, resultLength, 2200);
		if (error)
			return error;
	}

	return C_OK;
}
