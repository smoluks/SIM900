/*
 * init.c
 *
 *  Created on: Jun 30, 2020
 *      Author: Администратор
 */

#include "stm32f1xx.h"

void __attribute__ ((weak)) _init(void)
{
}

void SystemCoreClockUpdate();

void SystemInit(void)
{
	RCC->APB1ENR = RCC_APB1ENR_PWREN | RCC_APB1ENR_USART2EN;
	RCC->APB2ENR = RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN
			| RCC_APB2ENR_AFIOEN;
	//SWD
	AFIO->MAPR = AFIO_MAPR_SPI1_REMAP | AFIO_MAPR_SWJ_CFG_1;
	//porta
	GPIOA->CRH = 0x88800000;
	GPIOA->CRL = 0x00008B38;
	GPIOA->ODR = 0x0000A008;
	//portb
	GPIOB->CRH = 0x00000033;
	GPIOB->CRL = 0x38003700;
	GPIOB->ODR = 0x00000384;
	//portc
	GPIOC->CRH = 0x70000000;
	GPIOC->CRL = 0x00000000;
	GPIOC->ODR = 0x00008000;
	//UART2 - Modem
	USART2->BRR = 313;
	USART2->CR2 = 0x00000000;
	USART2->CR3 = 0x00000000;
	USART2->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE;
	//NVIC_EnableIRQ(USART2_IRQn);
	NVIC_SetPriority(USART2_IRQn, 1);
	//Flash
	FLASH->KEYR = FLASH_KEY1;
	FLASH->KEYR = FLASH_KEY2;
	//systick
	SysTick->LOAD = 72000000UL / 1000 - 1;
	SysTick->VAL = 72000000UL / 1000 - 1;
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk
			| SysTick_CTRL_ENABLE_Msk;
	//iwdg
	// Для IWDG_PR=7 Tmin=6,4мс RLR=Tмс*40/256
	IWDG->KR=0x5555;
	IWDG->PR=7;
	IWDG->RLR=1000*40/256;
	IWDG->KR=0xAAAA;
	IWDG->KR=0xCCCC;

	SystemCoreClockUpdate();

	__enable_irq();
}

void SystemCoreClockUpdate()
{
	RCC->CR = RCC_CR_HSEON;
	while (!(RCC->CR & RCC_CR_HSERDY))

	FLASH->ACR = FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY_1;
	RCC->CFGR = RCC_CFGR_PLLMULL9 | RCC_CFGR_PLLSRC | RCC_CFGR_ADCPRE_DIV6
			| RCC_CFGR_PPRE2_DIV1 | RCC_CFGR_PPRE1_DIV2 | RCC_CFGR_HPRE_DIV1;// | RCC_CFGR_PLLXTPRE;

	RCC->CR |= RCC_CR_PLLON;
	while (!(RCC->CR & RCC_CR_PLLRDY));

	RCC->CFGR = RCC->CFGR | RCC_CFGR_SW_PLL;
	while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
}

