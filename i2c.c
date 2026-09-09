#include "stm32f10x.h"
#include <stdio.h>

#include "timers.h"
#include "uart.h"

static void I2C2_start(void)
{
	I2C2->CR1 |= I2C_CR1_START;
	// ждем окончания формирования сигнала "Старт"
	while (!(I2C2->SR1 & I2C_SR1_SB))
	{
		// asm("NOP");
	}
	(void)I2C2->SR1;
}

static void I2C2_stop(void)
{
	I2C2->CR1 |= I2C_CR1_STOP;
}

static void I2C2_set_addr(uint8_t addr)
{
	I2C2->DR = addr;
	while (!(I2C2->SR1 & I2C_SR1_ADDR))
	{
		// asm("NOP");
	}
	(void)I2C2->SR1;
	(void)I2C2->SR2;
}

uint8_t I2C2_read_reg(uint8_t dev_addr, uint8_t reg_addr)
{
	uint8_t res = 0;
	I2C2_start();
	I2C2_set_addr((dev_addr << 1) | 0);
	I2C2->DR = reg_addr;
	while (!(I2C2->SR1 & I2C_SR1_TXE))
	{
	}
	I2C2_stop();
	I2C2_start();
	I2C2_set_addr((dev_addr << 1) | 1);
	// res = I2C2_read(0);
	I2C2->CR1 &= ~I2C_CR1_ACK;
	while (!(I2C2->SR1 & I2C_SR1_RXNE))
	{
	}
	res = I2C2->DR;
	I2C2_stop();
	return res;
}

void I2C2_write_reg(uint8_t dev_addr, uint8_t reg_addr, uint8_t data)
{
	uint8_t res = 0;
	I2C2_start();
	I2C2_set_addr((dev_addr << 1) | 0);
	I2C2->DR = reg_addr;
	while (!(I2C2->SR1 & I2C_SR1_TXE))
	{
	}
	I2C2->DR = data;
	while (!(I2C2->SR1 & I2C_SR1_BTF))
	{
	}
	I2C2_stop();
}

void I2C2_init(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN;
	RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;

	GPIOB->CRH &= ~(GPIO_CRH_MODE10_1 | GPIO_CRH_MODE11_1 | GPIO_CRH_CNF10 | GPIO_CRH_CNF11);
	GPIOB->CRH |= GPIO_CRH_MODE10_1 | GPIO_CRH_MODE11_1 | GPIO_CRH_CNF10_0 | GPIO_CRH_CNF11_0;
	I2C1->CR1 &= ~I2C_CR1_PE;

	GPIOB->ODR |= GPIO_ODR_ODR10 | GPIO_ODR_ODR11;
	while (!(GPIOB->IDR & (GPIO_IDR_IDR10 | GPIO_IDR_IDR11)))
		;

	GPIOB->ODR &= ~(GPIO_ODR_ODR10);
	while (GPIOB->IDR & GPIO_IDR_IDR10)
		;
	GPIOB->ODR &= ~(GPIO_ODR_ODR11);
	while (GPIOB->IDR & GPIO_IDR_IDR11)
		;

	GPIOB->ODR |= GPIO_ODR_ODR10;
	while (!(GPIOB->IDR & GPIO_IDR_IDR10))
		;
	GPIOB->ODR |= GPIO_ODR_ODR11;
	while (!(GPIOB->IDR & GPIO_IDR_IDR11))
		;

	GPIOB->CRH &= ~(GPIO_CRH_MODE10 | GPIO_CRH_MODE11 | GPIO_CRH_CNF10 | GPIO_CRH_CNF11);
	GPIOB->CRH |= GPIO_CRH_MODE10 | GPIO_CRH_MODE11 | GPIO_CRH_CNF10 | GPIO_CRH_CNF11;
	I2C2->CR1 |= I2C_CR1_SWRST;
	I2C2->CR1 &= ~I2C_CR1_SWRST;
	I2C2->CR2 &= ~I2C_CR2_FREQ;
	I2C2->CR2 |= 36;
	I2C2->CCR &= ~I2C_CCR_CCR;
	I2C2->CCR = 180;
	I2C2->TRISE = 37;
	I2C2->CR1 |= I2C_CR1_ACK;
	I2C2->CR1 |= I2C_CR1_PE;

	//------------------------------------------------------------------------------------//
	/*
	while(1){
		volatile uint8_t s, m, h, dd, mm, yyyy;

		s = I2C2_write_read_reg(0x68, 0x00);

		I2C2_start();
		I2C2_write(0xD0, 0x00);
		I2C2_start();
	  s = I2C2_read(0xD1);
		I2C2_stop();

		I2C2_start();
		I2C2_write(0xD0, 0x01);
		I2C2_start();
	  m = I2C2_read(0xD1);
		I2C2_stop();

		I2C2_start();
		I2C2_write(0xD0, 0x02);
		I2C2_start();
	  h = I2C2_read(0xD1);
		I2C2_stop();

		I2C2_start();
		I2C2_write(0xD0, 0x04);
		I2C2_start();
	  dd = I2C2_read(0xD1);
		I2C2_stop();

		I2C2_start();
		I2C2_write(0xD0, 0x05);
		I2C2_start();
	  mm = I2C2_read(0xD1);
		I2C2_stop();

		I2C2_start();
		I2C2_write(0xD0, 0x06);
		I2C2_start();
	  yyyy = I2C2_read(0xD1);
		I2C2_stop();

		s = RTC_ConvertFromDec(s);
		m = RTC_ConvertFromDec(m);
		h = RTC_ConvertFromDec(h);
		dd = RTC_ConvertFromDec(dd);
		mm = RTC_ConvertFromDec(mm);
		yyyy = RTC_ConvertFromDec(yyyy);


		char buf[30];
		//sprintf(buf, "%d.%d.%d %d:%d:%d\r\n", dd,mm,yyyy,h,m,s);
		sprintf(buf, "%d.%d.%d %d:%d:%d\r\n", dd,mm,yyyy,h,m,s);
		//USART1_send_str("\r\n");
		//hex2str(buf, data);
		USART1_send_str(buf);
		delay_ms(1000);
	}*/
}
