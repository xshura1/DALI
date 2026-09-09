#ifndef I2C_H_
#define I2C_H_

#include <stdint.h>

void I2C2_init(void);

uint8_t I2C2_read_reg(uint8_t dev_addr, uint8_t reg_addr);
void I2C2_write_reg(uint8_t dev_addr, uint8_t reg_addr, uint8_t data);

#endif /* I2C_H_ */
