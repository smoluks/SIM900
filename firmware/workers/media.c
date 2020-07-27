#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "stringext.h"
#include "modem.h"
#include "media.h"

void process_next();
void playNext();

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
		process_next();
		end = false;
	}
}

static char filesToPlay[16][16];
uint8_t filesCount;

void playSome(char* files[], uint8_t count)
{
	stop();

	if (filesCount <= 16)
		filesCount = count;
	else
		filesCount = 16;

	for (uint8_t i = 0; i < filesCount; i++)
	{
		strncpy(filesToPlay[i], files[i], 15);
	}

	playNext();
}

void play(char *filename)
{
	stop();

	strncpy(filesToPlay[0], filename, 15);
	filesCount = 1;

	playNext();
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

void process_next()
{
	if(filesCount)
		playNext();
	else
		isPlaying = false;
}

void playNext()
{
	char* filename = filesToPlay[--filesCount];

	char play[64];
	snprintf(play, sizeof(play), "%s%s%s", "AT+CREC=4,\"C:\\User\\", filename, "\",1,80\r\n");

	commanderror error = sendcommand(play, 5000);
	if (!error)
		isPlaying = true;
	else
		process_next();
}

