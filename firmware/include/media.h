#pragma once

bool mediacommands(char* packet);
void processMedia();

void play(char *filename);
void playSome(char *files[], uint8_t count);
void stop();
