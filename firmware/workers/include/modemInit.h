#pragma once

bool modemInitCommands(uint8_t *packet);
bool modem_init();

struct providerSettings_s
{
	char* apn;
	char* login;
	char* password;
};
