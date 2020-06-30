/*
 * systick.h
 *
 *  Created on: Jun 30, 2020
 *      Author: Администратор
 */

#pragma once

uint32_t getSystime(void);
bool checkDelay(uint32_t timestamp, uint32_t delay);
