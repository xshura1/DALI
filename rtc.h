#ifndef RTC_H_
#define RTC_H_

#include <stdint.h>

#define RTC_ADDR 0x68

uint8_t RTC_get_seconds(void);
uint8_t RTC_get_minutes(void);
uint8_t RTC_get_hours(void);
uint8_t RTC_get_weekday(void);
uint8_t RTC_get_day(void);
uint8_t RTC_get_month(void);
uint16_t RTC_get_year(void);
uint16_t RTC_get_temp(void);

void RTC_set_seconds(uint8_t val);
void RTC_set_minutes(uint8_t val);
void RTC_set_hours(uint8_t val);
void RTC_set_weekday(uint8_t val);
void RTC_set_day(uint8_t val);
void RTC_set_month(uint8_t val);
void RTC_set_year(uint16_t val);

#endif /* RTC_H_ */
