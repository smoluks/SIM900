#pragma once

bool mediacommands(uint8_t* packet);
void processMedia();

void play(char *filename, bool external);
void playNext(char *filename, bool external);
void stop();
