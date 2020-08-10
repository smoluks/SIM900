#pragma once

#include <stdbool.h>

#define REQUEST_TIMEOUT 60000

struct httpAnswer_s {
	bool received;
	uint16_t code;
	uint32_t answerLength;
};
