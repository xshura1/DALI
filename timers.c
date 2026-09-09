#include "stm32f10x.h"

#include "dali.h"
#include "config.h"

// делители таймера протокола, период 417us
#define TIMER_FRAME_PRESCALER 72
#define TIMER_FRAME_HALF_ARR 20 // 417 //417
#define TIMER_FRAME_075_ARR 625

// делители таймера, период 1ms
#define TIMER_MAIN_PRESCALER 7200
#define TIMER_MAIN_ARR 10

// делители таймера, период 25us
#define TIMER_10US_PRESCALER 72
#define TIMER_10US_ARR 10 // 25

static volatile uint32_t time_s = 0;
static volatile uint32_t time_ms = 0;
static volatile uint32_t time_10_us = 0;

void TIM2_IRQHandler()
{
	static uint32_t curr_time_s;
	if (TIM2->SR & TIM_SR_UIF)
	{							// Проверяем, что это нас именно переполнение вызывало. Т.к. у таймера несколько видов событий и переполнение одно из
		TIM2->SR = ~TIM_SR_UIF; // сбросить флаг.

		time_ms++;
		curr_time_s++;
		if (curr_time_s == 1000)
		{
			time_s++;
			curr_time_s = 0;
		}
	}
}

void TIM4_IRQHandler()
{
	// static uint32_t curr_time_10us;
	if (TIM4->SR & TIM_SR_UIF)
	{
		TIM4->SR = ~TIM_SR_UIF;

#ifdef TEST_TIMER
		GPIOB->ODR ^= GPIO_ODR_ODR0;
#endif
		time_10_us++;
		/*curr_time_10us++;
		if (curr_time_10us == 5){
			check_buses();
			curr_time_10us = 0;

		}*/
		// tx_bus_handler();
	}
}

/*
void set_timer_dali_half_frame(void){
	TIM3->CR1 &= ~TIM_CR1_CEN; // останавливаем таймер
	TIM3->ARR = TIMER_FRAME_HALF_ARR - 1;
	TIM3->CNT = 0;
	TIM3->CR1 |= TIM_CR1_CEN; // запускаем таймер
}

void set_timer_dali_075_frame(void){
	TIM3->CR1 &= ~TIM_CR1_CEN; // останавливаем таймер
	TIM3->ARR = TIMER_FRAME_075_ARR - 1;
	TIM3->CNT = 0;//TIMER_FRAME_075_ARR - 1;
	TIM3->CR1 |= TIM_CR1_CEN; // запускаем таймер
}
*/

void timers_init(void)
{
	//	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;    												// Подаем тактирование на таймер от шины APB1
	//  TIM3->PSC = TIMER_FRAME_PRESCALER - 1;												// Частота шины 72 мегагерц. Так что в предделитель записываем 72-1, получим 1мГц
	//  TIM3->ARR = TIMER_FRAME_HALF_ARR - 1;													// Потолком счета таймера укажем 417-1. Получим деление на 417 в частоте вызова прерываний. Интервал 417us
	//  TIM3->CR1   |= TIM_CR1_ARPE | TIM_CR1_URS | TIM_CR1_CEN;  		// ARPE=1 - буфферизируем регистр предзагрузки таймера опциональная вещь.
	// URS=1	- разрешаем из событий таймера только события от переполнения
	// CEN=1 - запускаем таймер
	//	TIM3->DIER  |= TIM_DIER_UIE;																	// UIE=1 - Разрешаем прерывание от переполнения

	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	TIM2->PSC = TIMER_MAIN_PRESCALER - 1;
	TIM2->ARR = TIMER_MAIN_ARR - 1;
	TIM2->CR1 |= TIM_CR1_ARPE | TIM_CR1_URS | TIM_CR1_CEN;
	TIM2->DIER |= TIM_DIER_UIE;

	RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
	TIM4->PSC = TIMER_10US_PRESCALER - 1;
	TIM4->ARR = TIMER_10US_ARR - 1;
	TIM4->CR1 |= TIM_CR1_ARPE | TIM_CR1_URS | TIM_CR1_CEN;
	TIM4->DIER |= TIM_DIER_UIE;

	NVIC_SetPriority(TIM2_IRQn, 4); // 14
	// NVIC_SetPriority(TIM3_IRQn,5);	//13
	NVIC_SetPriority(TIM4_IRQn, 3);

	NVIC_EnableIRQ(TIM2_IRQn);
	// NVIC_EnableIRQ(TIM3_IRQn);
	NVIC_EnableIRQ(TIM4_IRQn);
}

uint32_t get_time_s(void)
{
	return time_s;
}

uint32_t get_diff_time_s(uint32_t old_time)
{
	if (old_time <= time_s)
		return time_s - old_time;
	else
		return (UINT32_MAX - old_time) + time_s;
}

uint32_t get_time_ms(void)
{
	return time_ms;
}

uint32_t get_diff_time_ms(uint32_t old_time)
{
	if (old_time <= time_ms)
		return time_ms - old_time;
	else
		return (UINT32_MAX - old_time) + time_ms;
}

uint32_t get_time_10_us(void)
{
	return time_10_us;
}

uint32_t get_diff_time_10_us(uint32_t old_time)
{
	if (old_time <= time_10_us)
		return time_10_us - old_time;
	else
		return (UINT32_MAX - old_time) + time_10_us;
}

void delay_ms(uint32_t time)
{
	uint32_t curr_time_ms = time_ms;
	while (get_diff_time_ms(curr_time_ms) < time)
	{
		asm("NOP");
	}
}

void delay_10us(uint32_t time)
{
	uint32_t curr_time_10_us = time_10_us;
	while (get_diff_time_10_us(curr_time_10_us) < time)
	{
		asm("NOP");
	}
}

void hard_delay(void)
{
	for (int i = 0; i < 5000000; i++)
	{
	}
}
