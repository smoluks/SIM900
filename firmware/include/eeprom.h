#pragma once

void flashErasePage(uint32_t address);
void flashWrite(uint32_t address, uint8_t* data, uint32_t count);
uint32_t flashRead(uint32_t address);
