#ifndef __DELAY_H
#define __DELAY_H 			   
#include <stm32f10x.h>
#include <stdint.h>
#include <stdbool.h>

typedef uint64_t Timer_t;

void delay_init(uint8_t SYSCLK);
void setTimer(Timer_t *timer, uint32_t timeout);
// when expired return false
// TODO meglio invertire i valori di ritorno ?
bool checkTimer(Timer_t *timer);

#endif





























