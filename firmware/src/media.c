/*
 * media.c
 *
 *	работа с аудиофайлами в модеме
 *  Created on: 4 февр. 2018 г.
 *      Author: Администратор
 */


#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "stringext.h"
#include "modem.h"
#include "media.h"

bool isplaying = false; //идет ли воспроизведение файла

//обработчик команд от модема
bool mediacommands(char* packet)
{
	//конец файла
	if(strpartcmp(packet, "+CREC: 0"))
	{
		isplaying = false;
		return true;
	}
	return false;
}

//воспроизведение файла
void start_play(char* filename, bool repeat)
{
	stop_play();

	char play[64];
	if(!repeat)
		snprintf(play, sizeof(play), "%s%s%s", "AT+CREC=4,\"C:\\User\\", filename, "\",1,80\r\n");
	else
		snprintf(play, sizeof(play), "%s%s%s", "AT+CREC=4,\"C:\\User\\", filename, "\",1,80,1\r\n");
	if(!sendcommand(play, 5000))
		isplaying = true;
}

//стоп
void stop_play()
{
	char buffer[11];
	if(isplaying)
	{
		sendcommandwithanswer("AT+CREC=5\r\n", buffer, 11, 5000);
		isplaying = false;
	}
}

