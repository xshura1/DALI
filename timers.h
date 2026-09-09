#ifndef TIMERS_H_
#define TIMERS_H_

#include "stdint.h"

void timers_init(void);
// void set_timer_dali_half_frame(void);
// void set_timer_dali_075_frame(void);

uint32_t get_time_s(void);
uint32_t get_diff_time_s(uint32_t old_time);

uint32_t get_time_ms(void);
uint32_t get_diff_time_ms(uint32_t old_time);

uint32_t get_time_10_us(void);
uint32_t get_diff_time_10_us(uint32_t old_time);

void delay_ms(uint32_t time);
void delay_10us(uint32_t time);
void hard_delay(void);

#endif /*TIMERS_H_*/
