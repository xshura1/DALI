#ifndef SPI_H_
#define SPI_H_

#include "stdint.h"

void SPI2_init(void);
void SPI2_write(uint8_t data);

void SPI2_write_buf(uint8_t *buf, uint16_t len);
void SPI2_read_buf(uint8_t *buf, uint16_t len);

uint8_t SPI2_read(void);
void SS_SELECT(void);
void SS_DESELECT(void);

#endif /*SPI_H_*/
