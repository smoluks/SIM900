#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum e_commanderror {
	C_OK = 0,
	C_CONNECT,
	C_RING,
	C_NOCARRIER,
	C_ERROR,
	C_NODIALTONE,
	C_BUSY,
	C_NOANSWER,
	C_PROCEEDING,
	C_NOCODE,
	C_TIMEOUT
} commanderror;

commanderror sendcommand(char* command, uint32_t timeout);
commanderror sendcommandwithanswer(char* command, char* buffer, int buffersize, uint32_t timeout);
commanderror sendcommandwith2answer(char* command, char* buffer, int buffersize, char* buffer2, int buffer2size, uint32_t timeout);

commanderror sendCommandWithData(char *command, uint32_t timeout);
commanderror sendCommandWithDownload(char *command, uint32_t timeout);
void sendTextData(char *data);
void sendBinaryData(uint8_t *data, uint8_t count);
commanderror getDataAnswer(char *buffer, int buffersize, uint32_t timeout);
commanderror getDownloadAnswer(uint32_t timeout);
commanderror getTcpData(uint8_t *buffer, uint32_t count, uint32_t timeout);
