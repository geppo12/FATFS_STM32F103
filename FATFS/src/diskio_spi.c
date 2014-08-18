/*
 * spi.c
 *
 *  Created on: 17/ago/2014
 *      Author: Geppo
 */


#include <stdint.h>
#include <USER/interface/spi.h>

void xmit_spi(uint8_t dato) {
	SPIx_ReadWriteByte(dato);
}

uint8_t rcvr_spi(void) {
	return SPIx_ReadWriteByte(0xFF);
}

void rcvr_spi_buf(uint8_t *buff, uint32_t size) {
	while (size > 0) {
		*buff++ = SPIx_ReadWriteByte(0xFF);
	}
}

// alias
void delay(uint32_t ms) {

}

void release_spi(void) {

}

void power_on(void) {

}

void power_off(void) {

}

int chk_power(void) {

}
