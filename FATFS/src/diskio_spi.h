/*
 * spi.h
 *
 *  Created on: 17/ago/2014
 *      Author: Geppo
 *  spi interface to uC driver
 */


#ifndef _SPI_H
#define _SPI_H

void xmit_spi(uint8_t dato);
uint8_t rcvr_spi(void);
void rcvr_spi_buf(uint8_t *buff, uint32_t size);

// alias
void delay(uint32_t ms);
void release_spi(void);
void power_on(void);
void power_off(void);
int chk_power(void);

#define CardSelect(onOff)	do {} while(0)

#endif
