#include <stm32f10x.h>

#include "socket.h"

#include "dali.h"
#include "cmd.h"
#include "tcp.h"

volatile unsigned int int_socket = 0;
volatile uint8_t int_recv = 0;
volatile uint8_t int_con = 0;
volatile uint8_t int_discon = 0;
volatile uint8_t int_send = 0;

void get_int_socket(void)
{
	/*if (getSn_IR(SOCK_TCP) & Sn_IR_RECV){
		cmd_check();
	}*/

	if (int_recv)
	{
		// if (getSn_IR(SOCK_TCP) & Sn_IR_RECV){
		// setSIR(0x00);
		// setSn_IR(SOCK_TCP, 0xFF);

		cmd_check();
		setSn_IR(SOCK_TCP, Sn_IR_RECV);
		// setIR(0x00);
		//	setSn_IR(SOCK_TCP, 0x1F);
		// setSn_IR(SOCK_TCP, 0x1F);
		//}
		int_recv = 0;

		// wizchip_clrinterrupt(IK_SOCK_ALL);
	}

	/*
	volatile uint8_t snir = getSn_IR(SOCK_TCP);
	volatile uint8_t snimr = getSn_IMR(SOCK_TCP);
	volatile uint8_t simr = getSIMR();
	volatile uint8_t sir = getSIR();

	//setSn_IR(SOCK_TCP, 0xFF);
	//wizchip_clrinterrupt(IK_SOCK_ALL);	*/
}

void TIM3_IRQHandler()
{

	if (TIM3->SR & TIM_SR_UIF)
	{							// Проверяем, что это нас именно переполнение вызывало. Т.к. у таймера несколько видов событий и переполнение одно из
		TIM3->SR = ~TIM_SR_UIF; // сбросить флаг.
		// tx_bus_handler();
	}
}

void EXTI0_IRQHandler()
{
	bus_handler(get_bus(0), 1);
	// Сбрасываем флаг прерывания
	EXTI->PR |= EXTI_PR_PR0;
}

void EXTI1_IRQHandler()
{
	bus_handler(get_bus(1), 1);
	// Сбрасываем флаг прерывания
	EXTI->PR |= EXTI_PR_PR1;
}

void EXTI2_IRQHandler()
{
	bus_handler(get_bus(2), 1);
	// Сбрасываем флаг прерывания
	EXTI->PR |= EXTI_PR_PR2;
}

void EXTI3_IRQHandler()
{
	bus_handler(get_bus(3), 1);
	// Сбрасываем флаг прерывания
	EXTI->PR |= EXTI_PR_PR3;
}

void EXTI4_IRQHandler()
{
	bus_handler(get_bus(4), 1);
	// Сбрасываем флаг прерывания
	EXTI->PR |= EXTI_PR_PR4;
}

void EXTI9_5_IRQHandler()
{

	for (uint8_t i = 5; i < 10; i++)
	{
		if (EXTI->PR & (1 << i))
		{
			if (i == 8)
			{
				int_recv = 1;
				// setSn_IR(SOCK_TCP, Sn_IR_RECV);
			}
			else
			{
				bus_handler(get_bus(i), 1);
			}
			EXTI->PR |= (1 << i);
			return;
		}
	}
	/*
	if (EXTI->PR & (1<<5)){
		rx_bus_handler(get_bus_no(5));
		EXTI->PR |= (1<<5);
  }
	if (EXTI->PR & (1<<6)){
		rx_bus_handler(get_bus_no(6));
		EXTI->PR |= (1<<6);
  }
	if (EXTI->PR & (1<<7)){
		rx_bus_handler(get_bus_no(7));
		EXTI->PR |= (1<<7);
  }
	if (EXTI->PR & (1<<8)){
		net_pool();
		EXTI->PR |= (1<<8);
  }
	if (EXTI->PR & (1<<9)){
		rx_bus_handler(get_bus_no(9));
		EXTI->PR |= (1<<9);
  }
	*/
}

void EXTI15_10_IRQHandler()
{
	if (EXTI->PR & (1 << 14))
	{
		bus_handler(get_bus(14), 1);
		EXTI->PR |= (1 << 14);
	}
	/*
	if (EXTI->PR & (1<<11)){

		EXTI->PR |= (1<<11);
  }
	if (EXTI->PR & (1<<12)){

		EXTI->PR |= (1<<12);
  }
	if (EXTI->PR & (1<<13)){

		EXTI->PR |= (1<<13);
  }
	if (EXTI->PR & (1<<15)){

		EXTI->PR |= (1<<15);
  }
	*/
}
