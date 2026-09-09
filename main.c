#include "stm32f10x.h"
#include "stddef.h"
#include "time.h"
#include "stdlib.h"

#include "config.h"
#include "timers.h"
#include "uart.h"
#include "dali.h"
#include "dali_cmd.h"
#include "spi.h"

#include "utils.h"
#include "cmd.h"
#include "tcp.h"
#include "socket.h"
#include "i2c.h"
#include "rtc.h"

// volatile char *buf;
// char tmp[64];

int main(void)
{
	SystemInit();

	// srand(time(NULL));

	RCC->APB2ENR |= RCC_APB2ENR_IOPCEN | RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN; // включаем тактирование портов
	GPIOC->CRH |= GPIO_CRH_MODE13_1;											  // частота шины 2 мГц
	GPIOC->CRH &= ~GPIO_CRH_CNF13;												  // выход push-pull
	GPIOC->BSRR |= GPIO_BSRR_BS13;

	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN; // включаем тактирование порта B

	// TODO перенести в настройку SPI шины
	//  SPI RST
	GPIOA->CRH |= GPIO_CRH_MODE11_1; // частота шины 2 мГц
	GPIOA->CRH &= ~GPIO_CRH_CNF11;	 // выход push-pull

	cmd_init_handlers(get_tcp_data, send_tcp_data, clear_tcp_data);

	timers_init();
	USART1_init();
	I2C2_init();
	SPI2_init();

	__enable_irq();

	// TODO перенести в настройку SPI шины
	// Hard Reset
	GPIOA->BSRR |= GPIO_BSRR_BR11;
	delay_ms(100);
	GPIOA->BSRR |= GPIO_BSRR_BS11;
	delay_ms(100);

	dali_init();

	NVIC_SetPriority(EXTI3_IRQn, 10);
	NVIC_EnableIRQ(EXTI9_5_IRQn);

	hard_delay();
#ifndef LINAR
	net_init();
#endif
	volatile uint8_t ver = getVERSIONR();
	USART1_put_str((uint8_t *)"\r\nBoard init");
	delay_ms(5000);
	// GPIOC->BSRR |= GPIO_BSRR_BR13;
	GPIOC->BSRR &= ~GPIO_BSRR_BR13;

	uint32_t curr_time_ms = get_time_ms();
	uint8_t is_bus = 0;
	volatile uint8_t sec = 0;
	// tcp_server();
	while (1)
	{

		//	sec = RTC_get_minutes();
		// RTC_set_minutes(14);

// delay_ms(1);
#ifdef TEST_TIME
		GPIOB->BSRR |= GPIO_BSRR_BS0;
#endif

		is_bus = check_buses();
		get_int_socket();

#ifdef TEST_TIME
		GPIOB->BSRR |= GPIO_BSRR_BR0;
#endif

		// if (get_diff_time_ms(curr_time_ms) > 100){
		// get_int_socket();
		tcp_server();
		// curr_time_ms = get_time_ms();
		//}
	}
}
