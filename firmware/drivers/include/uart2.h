#pragma once

#include "modem.h"

void modemSendPacket(char* data);
commanderror modemSendDataPacket(char *data, uint32_t timeout);
void modemSendData(char *data);
void modemSendBinaryData(uint8_t *data, uint8_t count);
commanderror waitDataMarker(char *data, uint32_t timeout);
commanderror waitDownloadMarker(char *data, uint32_t timeout);
char* modemGetPacket(uint32_t timeout, bool lastPacket);

void uart2Clear();
void send_uart2(char* data);
char* receive_uart2(uint8_t count, uint32_t timeout);


