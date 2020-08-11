#pragma once

#define REQUEST_TIMEOUT 60000

struct httpAnswer_s {
	bool received;
	uint16_t code;
	uint32_t answerLength;
};

bool httpHandler(uint8_t *packet);
commanderror sendGet(char *path, uint8_t *result, uint8_t resultLength);
commanderror sendPost(char *path, uint8_t *data, uint8_t length, uint8_t *result, uint8_t resultLength);
