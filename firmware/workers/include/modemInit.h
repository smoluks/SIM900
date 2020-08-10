#pragma once

bool modemInitCommands(char *packet);
bool modem_init();

struct providerSettings_s
{
	char* apn;
	char* login;
	char* password;
};
