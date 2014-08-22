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
void release_spi(void);
void PowerOn(void);
void PowerOff(void);
void select(void);
void deselect(void);
int chk_power(void);

#define CardSelect(onOff)	do {} while(0)

#endif
