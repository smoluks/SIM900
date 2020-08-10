#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "stringext.h"
#include "modem.h"
#include "media.h"

void processNextInternal();
void playNextInternal();

bool isPlaying = false;
static bool end = false;

bool mediacommands(char *packet)
{
	if (strpartcmp(packet, "+CREC: 0"))
	{
		end = true;
		return true;
	}
	return false;
}

void processMedia()
{
	if(end)
	{
		playNextInternal();
		end = false;
	}
}

static char filesToPlay[16][32];
uint8_t filesCount;

void play(char *filename)
{
	stop();

	strncpy(filesToPlay[0], filename, 31);
	filesCount = 1;

	playNextInternal();
}

void playNext(char *filename)
{
	if(filesCount == 16)
		return;

	strncpy(filesToPlay[filesCount], filename, 15);
	filesCount++;

	if(!isPlaying)
		playNextInternal();
}

void stop()
{
	if (isPlaying)
	{
		sendcommand("AT+CREC=5\r\n", 5000);
		end = false;
		isPlaying = false;
	}
}

void playNextInternal()
{
	if(!filesCount)
	{
		isPlaying = false;
		return;
	}

	filesCount--;

	char* filename = filesToPlay[0];

	char play[64];
	snprintf(play, sizeof(play), "%s%s%s", "AT+CREC=4,\"C:\\User\\", filename, "\",0,100\r\n");

	commanderror error = sendcommand(play, 5000);
	if (!error)
		isPlaying = true;
	else
		playNextInternal();
}

