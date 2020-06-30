/*
 * systick.h
 *
 *  Created on: Jun 30, 2020
 *      Author: Администратор
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

uint32_t getSystime();
bool checkDelay(uint32_t timestamp, uint32_t delay);
void delay(uint32_t delay);
