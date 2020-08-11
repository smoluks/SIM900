#include "stdio.h"
#include "config.h"
#include "stm32f1xx.h"

char callCommand[22];
char* getCallCommand()
{
	snprintf(callCommand, sizeof(callCommand), "ATD%s;\r\n", getMasterPhone());
	return callCommand;
}

char smsCommand[28] ;
char* getSmsCommand()
{
	snprintf(smsCommand, sizeof(smsCommand), "AT+CMGS=\"%s\"\r\n", getMasterPhone());
	return smsCommand;
}
