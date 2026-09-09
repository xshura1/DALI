#include <stdint.h>

#include "rtc.h"
#include "i2c.h"
#include "utils.h"

uint8_t RTC_get_seconds(void)
{
	return bcd_to_dec(I2C2_read_reg(RTC_ADDR, 0x00));
}

uint8_t RTC_get_minutes(void)
{
	return bcd_to_dec(I2C2_read_reg(RTC_ADDR, 0x01));
}

uint8_t RTC_get_hours(void)
{
	return bcd_to_dec(I2C2_read_reg(RTC_ADDR, 0x02) & 0x3F);
}

uint8_t RTC_get_weekday(void)
{
	return bcd_to_dec(I2C2_read_reg(RTC_ADDR, 0x03));
}

uint8_t RTC_get_day(void)
{
	return bcd_to_dec(I2C2_read_reg(RTC_ADDR, 0x04));
}

uint8_t RTC_get_month(void)
{
	return bcd_to_dec(I2C2_read_reg(RTC_ADDR, 0x05));
}

uint16_t RTC_get_year(void)
{
	return bcd_to_dec(I2C2_read_reg(RTC_ADDR, 0x06)) + 2000;
}

uint16_t RTC_get_temp(void)
{
	uint8_t fl = I2C2_read_reg(RTC_ADDR, 0x0F) & 0x04;
	while (fl)
	{
		fl = I2C2_read_reg(RTC_ADDR, 0x0F) & 0x04;
	}
	fl = I2C2_read_reg(RTC_ADDR, 0x0E);
	I2C2_write_reg(RTC_ADDR, 0x0E, fl | 0x20);
	fl = I2C2_read_reg(RTC_ADDR, 0x0E);
	while (fl & 0x20)
	{
		fl = I2C2_read_reg(RTC_ADDR, 0x0E);
	}
	uint8_t ub = I2C2_read_reg(RTC_ADDR, 0x11);
	uint8_t lb = I2C2_read_reg(RTC_ADDR, 0x12) >> 6;
	uint16_t res = (ub & 0x7F) * 100 + (lb * 25);
	return res;
}

void RTC_set_seconds(uint8_t val)
{
	I2C2_write_reg(RTC_ADDR, 0x00, dec_to_bcd(val));
}

void RTC_set_minutes(uint8_t val)
{
	I2C2_write_reg(RTC_ADDR, 0x01, dec_to_bcd(val));
}

void RTC_set_hours(uint8_t val)
{
	I2C2_write_reg(RTC_ADDR, 0x02, dec_to_bcd(val));
}

void RTC_set_weekday(uint8_t val)
{
	I2C2_write_reg(RTC_ADDR, 0x03, val);
}

void RTC_set_day(uint8_t val)
{
	I2C2_write_reg(RTC_ADDR, 0x04, dec_to_bcd(val));
}

void RTC_set_month(uint8_t val)
{
	I2C2_write_reg(RTC_ADDR, 0x05, dec_to_bcd(val));
}

void RTC_set_year(uint16_t val)
{
	if (val < 2000)
		return;
	I2C2_write_reg(RTC_ADDR, 0x06, dec_to_bcd(val - 2000));
}
