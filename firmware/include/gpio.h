/*
 * gpio.h
 *
 *  Created on: Jun 30, 2020
 *      Author: Администратор
 */

#pragma once

#define IsLocked() ((GPIOB->IDR & 0x0040) == 0x0040)

#define LedGreenOn() (GPIOB->BSRR = 0x00000080)
#define LedOrangeOn() (GPIOB->BSRR = 0x00000100)
#define LedRedOn() (GPIOB->BSRR = 0x00000200)
#define LedGreenOff() (GPIOB->BSRR = 0x00800000)
#define LedOrangeOff() (GPIOB->BSRR = 0x01000000)
#define LedRedOff() (GPIOB->BSRR = 0x02000000)
#define AllLedOff() (GPIOB->BSRR = 0x03800000)

#define OutputEnable() (GPIOB->BSRR = 0x00000008)
#define OutputDisable() (GPIOB->BSRR = 0x00080000)

#define ModemReset() (GPIOC->BSRR = 0x80000000)
#define ModemResetOff() (GPIOC->BSRR = 0x00008000)

#define IsModemPowerUp() (GPIOB->IDR & 0x00000004)
#define ModemPowerUp() (GPIOB->BSRR = 0x00040000)
#define ModemPowerUpOff() (GPIOB->BSRR = 0x00000004)
