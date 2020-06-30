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

#define password 4321

bool menu_password(); //���� ����� ������
uint16_t menu_delay(); //���� ������� ������

bool ringflag = false; //���� ����������� ������
bool nocarrierflag = false; //���� ��������� ������
char dtmfcode = 0; //��� ����������� DTMF

//���������� ������ �� ������
bool callcommands(char* packet)
{
	//�������� �����
	if(!strcmp(packet, "2\r\n"))
	{
		ringflag = true;
		nocarrierflag = false;
		return true;
	//������ ��������
	}else if(!strcmp(packet, "3\r\n"))
	{
		nocarrierflag = true;
		return true;
	//������� DTMF
	}else if(strpartcmp(packet, "+DTMF:"))
	{
		dtmfcode = packet[7];
		return true;
	}
	return false;
}

//���������� �������� �������
void process_call()
{
	if(ringflag)
	{
		ringflag = false;
		sendcommand("ATA\r\n", 5000); //������� ������

		if(!menu_password())
			return;

		uint16_t delay = menu_delay();
		if(!delay)
			return;

		start_play("start.amr", false);

		alg(delay);
	}
}

//��������� ���� ����� ������
bool menu_password()
{
	start_play("main.amr", false);

	uint8_t wrongpasscount = 3; //���������� ������� ��������� �����
	uint8_t passsymbolcount; //���������� �������� ��������
	uint16_t passwd; //��������� ������

	volatile uint32_t timestamp = getSystime(); //������� ����� ��� ��������

	do
	{
		passsymbolcount = 0;
		passwd = 0;

		do
		{
			if (nocarrierflag)
			{
				//������ ��������
				nocarrierflag = false;
				return false;
			}
			if (dtmfcode)
			{
				if (dtmfcode == '*')
				{
					//����� ������
					passwd = 0;
					passsymbolcount = 0;
				} else if (dtmfcode >= '0' && dtmfcode <= '9')
				{
					passwd = passwd * 10 + (dtmfcode - '0');
					passsymbolcount++;
				}
				dtmfcode = 0;
			}
			//����-��� 3 ������
			if(checkDelay(timestamp, 3*60*1000u))
				return false;

		} while (passsymbolcount < 4);

		if(passwd == password)
		{
			return true;
		}

		start_play("badpassword.amr", false);

	} while (--wrongpasscount);

	sendcommand("ATH\r\n", 5000); //�������� ������

	return false;
}

//��������� ���� ����� ������� ������
uint16_t menu_delay()
{
	volatile uint32_t timestamp = getSystime(); //������� ����� ��� ��������

	int delay = 0;

	start_play("delay.amr", false);

	do
	{
		if (nocarrierflag)
		{
			//������ ��������
			nocarrierflag = false;
			return 0;
		}
		if (dtmfcode)
		{
			if (dtmfcode == '*')
			{
				//����� ������
				delay = 0;
			}
			else if (dtmfcode == '#')
			{
				//enter
				return delay;
			}
			else if (dtmfcode >= '0' && dtmfcode <= '9')
			{
				delay = delay * 10 + (dtmfcode - '0');
				if(delay>65535)
				{
					//���� ���� �����, ����������� ���������
					sendcommand("ATH\r\n", 5000); //�������� ������
					return 0;
				}
			}
			dtmfcode = 0;
		}
		//����-��� 3 ������
		if(checkDelay(timestamp, 3*60*1000u))
			return false;
	} while (1);

	stop_play();

	return 0;
}




