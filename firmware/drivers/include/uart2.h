#pragma once

#include "modem.h"

void modemSendPacket(char* data);
commanderror modemSendDataPacket(char *data, uint32_t timeout);
void modemSendData(char *data);
commanderror waitDataMarker(char *data, uint32_t timeout);
char* modemGetPacket(uint32_t timeout, bool lastPacket);

void uart2Clear();
void send_uart2(char* data);
char* receive_uart2(uint8_t count, uint32_t timeout);


