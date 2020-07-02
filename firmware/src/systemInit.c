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
	RCC->APB2ENR = RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN | RCC_APB2ENR_AFIOEN;
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
	//UART2
	USART2->BRR = 625;
	USART2->CR2 = 0x00000000;
	USART2->CR3 = 0x00000000;
	USART2->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE;
	//NVIC_EnableIRQ(USART2_IRQn);
	NVIC_SetPriority(USART2_IRQn, 1);

	//systick
	SysTick->LOAD = 72000000UL / 1000 - 1;
	SysTick->VAL = 72000000UL / 1000 - 1;
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;


	SystemCoreClockUpdate();

	__enable_irq();
}

void SystemCoreClockUpdate()
{
	//�������� ������� �����
	RCC->CR = RCC_CR_HSEON;
	while (!(RCC->CR & RCC_CR_HSERDY)); //���� ���������� �������� ������

	FLASH->ACR = FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY_1; //�������� ������� ��� FLASH ������
	RCC->CFGR = RCC_CFGR_PLLMULL9 | RCC_CFGR_PLLSRC
			| RCC_CFGR_ADCPRE_DIV6 | RCC_CFGR_PPRE2_DIV1 | RCC_CFGR_PPRE1_DIV2
			| RCC_CFGR_HPRE_DIV1;

	RCC->CR |= RCC_CR_PLLON; //�������� PLL
	while (!(RCC->CR & RCC_CR_PLLRDY)); //���� ���������� PLL

	RCC->CFGR = RCC->CFGR | RCC_CFGR_SW_PLL; //������������� �� pll
	while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL); //���� ������������ �� pll
}


