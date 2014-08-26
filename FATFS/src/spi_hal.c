/*
 * spi_hal.c
 *
 *  Created on: 17/ago/2014
 *      Author: Geppo
 *
 * hardware abstraction layer for spi driver. Implement platform dependent code for diskio_spi
 */

#include <stdint.h>
#include <USER/interface/spi.h>
#include "spi_hal.h"

void xmit_spi(uint8_t dato) {
	SPIx_ReadWriteByte(dato);
}

uint8_t rcvr_spi(void) {
	return SPIx_ReadWriteByte(0xFF);
}

void rcvr_spi_buf(uint8_t *buff, uint32_t size) {
	while (size > 0) {
		*buff++ = SPIx_ReadWriteByte(0xFF);
		size--;
	}
}

void release_spi(void) {
	deselect();
}

void PowerOn(void) {
	SPIx_Init();
	deselect();
}

void PowerOff(void) {
	deselect();
}

void select(void) {
	GPIO_ResetBits(GPIOA, GPIO_Pin_4);
}

void deselect(void) {
	GPIO_SetBits(GPIOA, GPIO_Pin_4);
}

int chk_power(void) {

}
