/*
 * gpio.h
 *
 *  Created on: Jun 30, 2020
 *      Author: Администратор
 */

#pragma once

#define IsLocked() ((GPIOB->IDR & 0x0040) == 0x0040)
