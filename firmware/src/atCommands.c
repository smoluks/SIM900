#include "stdio.h"
#include "config.h"
#include "stm32f1xx.h"

char callCommand[22];
char* getCallCommand()
{
	snprintf(callCommand, sizeof(callCommand), "%s%s%s", "ATD", getMasterPhone(), ";\r\n");
	return callCommand;
}

char smsCommand[28] ;
char* getSmsCommand()
{
	snprintf(smsCommand, sizeof(smsCommand), "%s%s%s", "AT+CMGS=\"", getMasterPhone(), "\"\r\n");
	return smsCommand;
}
