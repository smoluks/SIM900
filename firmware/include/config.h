#pragma once

#include "stm32f1xx.h"

#define baseaddr 0x0801FC00

#pragma pack(push,1)
struct config_s {
	char masterphone[16];
};
#pragma pack(pop)

void readConfig();
void writeConfig();
char* getMasterPhone();
void setMasterPhone(char* phone);
