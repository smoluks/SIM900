#include "modem.h"

bool httpHandler(char *packet) {
	if (strpartcmp(packet, "+HTTPACTION")) {
		return true;
	}
	return false;
}

char command[128];
void sendGet(char *path) {
	snprintf(command, sizeof(command),
			"AT+HTTPPARA=\"URL\",\"http://ya.ru/%s\"\r\n", path);
	commanderror error = sendcommand(command, 15000);
	if (error)
		return;

	error = sendcommand("AT+HTTPACTION=0\r\n", 15000);
	if (error)
		return;
}

void sendPost(char *path, char *data, uint8_t length) {
	snprintf(command, sizeof(command),
			"AT+HTTPPARA=\"URL\",\"http://ya.ru/%s\"\r\n", path);
	commanderror error = sendcommand(command, 15000);
	if (error)
		return;

	char snum[5];
	itoa(length, snum, 10);
	snprintf(command, sizeof(command), "AT+HTTPDATA=%s,1000\r\n", snum);
	error = sendCommandWithDownload(command, 1000);
	if (error)
		return;

	sendData(data);

	error = getDownloadAnswer(2200);
	if (error)
		return;

	error = sendcommand("AT+HTTPACTION=1\r\n", 15000);
	if (error)
		return;
}
