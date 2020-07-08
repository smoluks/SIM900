/*
 * call.c
 *
 *	�������� ���� � ��������� � ��� ��������
 *  Created on: 4 ����. 2018 �.
 *      Author: �������������
 */
#include "stm32f1xx.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "stringext.h"
#include "modem.h"
#include "media.h"
#include "main.h"
#include "call.h"
#include "systick.h"
#include "atCommands.h"
#include "logic.h"

#define password 4321

void menu_password(uint8_t dtmfcode);
void menu_delay(uint8_t dtmfcode);
void menu_stop(uint8_t dtmfcode);

extern enum algstage_e algstage;
enum callstage_e
{
	waitCall, waitPasswd, waitDuration, waitStop, hangUp, waitStopPlayAndHangUp
} callstage;

bool callFlag = false;
bool hangUpFlag = false;
uint8_t dtmf = 0;
bool callcommands(char *packet)
{
	if (!strcmp(packet, "2\r\n"))
	{
		//Ring
		callFlag = true;
		return true;
	}
	else if (!strcmp(packet, "3\r\n"))
	{
		hangUpFlag = true;
		return true;
	}
	else if (strpartcmp(packet, "+DTMF:"))
	{
		dtmf = packet[7];
		return true;
	}
	return false;
}

bool isCall = false;
uint32_t timestamp;
uint16_t passwd = 0;
uint8_t passSymbolCount = 0;
uint8_t wrongpasscount;
uint16_t duration;

extern bool isPlaying;

void process_call()
{
	if (callFlag)
	{
		sendcommand(pickUpCommand, 5000);
		isCall = true;

		if(algstage == call)
		{
			play("stopsuccessful.amr");
			callstage = waitStopPlayAndHangUp;
		}
		else
		{
			play("main.amr");
			passwd = 0;
			passSymbolCount = 0;
			wrongpasscount = 3;
			duration = 0;
			callstage = waitPasswd;
			timestamp = getSystime();
		}

		callFlag = false;
	}

	if (hangUpFlag)
	{
		isCall = false;
		callstage = waitCall;
		hangUpFlag = false;
	}

	if (dtmf)
	{
		stop();
		switch (callstage)
		{
		case waitCall:
		case hangUp:
		case waitStopPlayAndHangUp:
			break;
		case waitPasswd:
			menu_password(dtmf);
			break;
		case waitDuration:
			menu_delay(dtmf);
			break;
		case waitStop:
			menu_stop(dtmf);
		}
		timestamp = getSystime();
		dtmf = 0;
	}

	if (callstage == waitStopPlayAndHangUp)
	{
		if (isPlaying)
			return;

		if (isCall)
		{
			sendcommand(hangUpCommand, 5000);
			isCall = false;
		}
		callstage = waitCall;
	}

	if (callstage == hangUp || (isCall && checkDelay(timestamp, 60000u)))
	{
		sendcommand(hangUpCommand, 5000);
		isCall = false;
		callstage = waitCall;
	}
}

char files[3][6] = { "0.amr", "0.amr", "0.amr" };

void menu_password(uint8_t dtmfcode)
{
	if (dtmfcode == '*')
	{
		passwd = 0;
		passSymbolCount = 0;
	}
	else if (dtmfcode >= '0' && dtmfcode <= '9')
	{
		passwd = passwd * 10 + (dtmfcode - '0');
		passSymbolCount++;
	}

	if (passSymbolCount != 4)
		return;

	if (passwd == password)
	{
		if (!isProgramWorking())
		{
			play("delay.amr");
			callstage = waitDuration;
		}
		else
		{
			uint16_t estimateTime = getEstimateTime();

			files[0][0] = (estimateTime % 1000 / 100) + '0';
			files[1][0] = (estimateTime % 100 / 10) + '0';
			files[2][0] = (estimateTime % 10) + '0';

			char *f[5] =
			{ "stop.amr", files[2], files[1], files[0], "current.amr" };
			playSome(f, 5);

			callstage = waitStop;
		}
	}
	else
	{
		if (!--wrongpasscount)
		{
			callstage = hangUp;
		}
		else
		{
			play("badpassword.amr");
			passSymbolCount = 0;
			passwd = 0;
		}
	}
}

void menu_delay(uint8_t dtmfcode)
{
	if (dtmfcode == '*')
	{
		duration = 0;
	}
	else if (dtmfcode == '#')
	{
		startProgram(duration);
		play("start.amr");
		callstage = waitStopPlayAndHangUp;

	}
	else if (dtmfcode >= '0' && dtmfcode <= '9')
	{
		duration = duration * 10 + (dtmfcode - '0');
		if (duration > 999)
		{
			callstage = hangUp;
		}
	}
}

void menu_stop(uint8_t dtmfcode)
{
	if (dtmfcode == '#')
	{
		stopProgram();

		play("stopsuccessful.amr");
		callstage = waitStopPlayAndHangUp;
	}
}

