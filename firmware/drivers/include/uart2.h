#pragma once

#include "modem.h"

void modemSendPacket(char* data);
char* modemGetPacket(uint32_t timeout);

void send_uart2(char* data);
char* receive_uart2(uint8_t count, uint32_t timeout);


