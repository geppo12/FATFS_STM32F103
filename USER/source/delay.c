// TODO cambiare nome in timer
#include <stm32f10x.h>
#include "delay.h"

static uint64_t sSysTicks = 0;

void SysTickIrq(void) {
	sSysTicks++;
}

void delay_init(uint8_t SYSCLK)
{
	SysTick->CTRL &= 0xfffffffb;
	SysTick->LOAD=(uint32_t)SYSCLK*1000;
	SysTick->VAL =0x00;
	SysTick->CTRL=0x07 ;		// enable processor clock (bit2) irq (bit1) and timer (bit0)
}

void setTimer(Timer_t *timer, uint32_t timeout) {
	*(uint64_t *)timer = sSysTicks + timeout;
}

bool checkTimer(Timer_t *timer) {
	bool retVal = false;
		if (*((uint64_t *)timer) - sSysTicks > 0) {
			retVal = true;
	// ~0 short than = 0xFFFFF..... 64bit interger
	if (*((uint64_t *)timer) != ~0) {
		} else {
			*((uint64_t *)timer) = ~0;
		}
	}
	return retVal;
}
void delay(uint32_t nms)
{
	Timer_t t;
	setTimer(&t,nms);
	while (!checkTimer(&t));
}


































