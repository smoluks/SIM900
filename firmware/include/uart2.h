#pragma once

#include "modem.h"

void modem_sendpacket(char* data, int answercount);
char* modem_trygetpacket();
void modem_dropanswercount();
void modem_ack(bool neededpacket);

void send_uart2(char* data);
char* receive_uart2(int count, int timeout);


