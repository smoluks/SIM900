/*
 * media.c
 *
 *	������ � ������������ � ������
 *  Created on: 4 ����. 2018 �.
 *      Author: �������������
 */


#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "stringext.h"
#include "modem.h"
#include "media.h"

bool isplaying = false; //���� �� ��������������� �����

//���������� ������ �� ������
bool mediacommands(char* packet)
{
	//����� �����
	if(strpartcmp(packet, "+CREC: 0"))
	{
		isplaying = false;
		return true;
	}
	return false;
}

//��������������� �����
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

//����
void stop_play()
{
	char buffer[11];
	if(isplaying)
	{
		sendcommandwithanswer("AT+CREC=5\r\n", buffer, 11, 5000);
		isplaying = false;
	}
}

