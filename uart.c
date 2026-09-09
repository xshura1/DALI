#include <stm32f10x.h>
#include <stdio.h>
#include <stddef.h>

#include "timers.h"
#include "uart.h"

#define USART_BAUDRATE (72000000 + 4800) / 9600

// USART Receiver buffer
#define RX_BUFFER_SIZE 256
#define TX_BUFFER_SIZE 256

struct FIFO_BUFFER
{
	uint8_t *buffer;
	uint16_t wr_index;
	uint16_t rd_index;
	uint16_t counter;
	uint8_t overflow;
};

volatile struct FIFO_BUFFER rx_buf;
volatile struct FIFO_BUFFER tx_buf;

uint8_t rx_buffer[RX_BUFFER_SIZE];
uint8_t tx_buffer[TX_BUFFER_SIZE];

enum USART_CMD_STATE
{
	UCS_PREFIX,
	UCS_ARG1,
	UCS_ARG2
};

struct USART_CMD usart_cmd;
enum USART_CMD_STATE usart_cmd_state = UCS_PREFIX;

/*volatile uint8_t  rx_buffer[RX_BUFFER_SIZE];
volatile uint16_t rx_wr_index=0,rx_rd_index=0;
volatile uint16_t rx_counter=0;
volatile uint8_t rx_buffer_overflow=0;

// USART Transmitter buffer

volatile uint8_t tx_buffer[TX_BUFFER_SIZE];
volatile uint16_t tx_wr_index=0,tx_rd_index=0;
volatile uint16_t tx_counter=0;
*/

void USART1_IRQHandler(void)
{
	uint16_t tmp = 0;

	if (USART1->SR & USART_SR_RXNE)
	{

		if ((USART1->SR & (USART_SR_NE | USART_SR_FE | USART_SR_PE | USART_SR_ORE)) == 0) // проверяем нет ли ошибок
		{
			rx_buf.buffer[rx_buf.wr_index++] = (uint8_t)(USART1->DR & 0xFF); // считываем данные в буфер, инкрементируя хвост буфера
			if (rx_buf.wr_index == RX_BUFFER_SIZE)
				rx_buf.wr_index = 0;				// идем по кругу
			if (++rx_buf.counter == RX_BUFFER_SIZE) // переполнение буфера
			{
				rx_buf.counter = 0;	 // начинаем сначала (удаляем все данные)
				rx_buf.overflow = 1; // сообщаем о переполнении
			}
		}
		else
			tmp = USART1->DR; // в идеале пишем здесь обработчик ошибок, в данном случае просто пропускаем ошибочный байт.
	}

	if (USART1->SR & USART_SR_TXE)
	{
		if (tx_buf.counter)
		{					  // если есть что передать
			--tx_buf.counter; // уменьшаем количество не переданных данных
			USART1->DR = tx_buf.buffer[tx_buf.rd_index++];
			if (tx_buf.rd_index == TX_BUFFER_SIZE)
				tx_buf.rd_index = 0; // идем по кругу
		}
		else
		{ // если нечего передать, запрещаем прерывание по передачи
			USART1->CR1 &= ~USART_SR_TXE;
		}
	}
}

// FILE f_uart1 = fopencookie( NULL, "rw+", (cookie_io_functions_t){ uart1_read, uart1_write, NULL, NULL } );

void USART1_init(void)
{

	rx_buf.buffer = &rx_buffer[0];
	tx_buf.buffer = &tx_buffer[0];

	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;										  // инициализируем шину для USART1
	USART1->BRR = USART_BAUDRATE;												  // бодрэйт
	USART1->CR1 |= USART_CR1_UE | USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE; // | USART_CR1_TXEIE; // USART1 ON, TX ON, RX ON, RX, TX interrupt ON
	USART1->CR1 |= USART_CR1_TXEIE;

	// настройка ножки TX (PA9)
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN; // GPIOA Clock ON. Alter function clock ON
	GPIOA->CRH &= ~GPIO_CRH_CNF9;							 // Clear CNF bit 9
	GPIOA->CRH |= GPIO_CRH_CNF9_1;							 // Set CNF bit 9 to 10 - AFIO Push-Pull
	GPIOA->CRH |= GPIO_CRH_MODE9_0;							 // Set MODE bit 9 to Mode 01 = 10MHz

	// настройка ножки RX (PA10)
	GPIOA->CRH &= ~GPIO_CRH_CNF10;	// Clear CNF bit 10
	GPIOA->CRH |= GPIO_CRH_CNF10_0; // Set CNF bit 10 to 01 = HiZ
	GPIOA->CRH &= ~GPIO_CRH_MODE10; // Set MODE bit 10 to Mode 01 = 10MHz

	NVIC_EnableIRQ(USART1_IRQn);

	__enable_irq();
}

void USART1_send_char(char c)
{
	while (!(USART1->SR & USART_SR_TC))
	{
	}; // Сравниваем регистр статуса с разрядом установки флага об окончании прерывания
	USART1->DR = c;
}

void USART1_send_str(char *str)
{
	uint8_t i = 0;
	while (str[i] != 0)
		USART1_send_char(str[i++]);
}

void put_char(uint8_t c)
{
	while (tx_buf.counter == TX_BUFFER_SIZE)
		;
	// USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
	USART1->CR1 &= ~USART_SR_TXE;
	if (tx_buf.counter || (!(USART1->SR & USART_SR_TXE)))
	{
		tx_buf.buffer[tx_buf.wr_index++] = c;
		if (tx_buf.wr_index == TX_BUFFER_SIZE)
			tx_buf.wr_index = 0;
		++tx_buf.counter;
		USART1->CR1 |= USART_SR_TXE;
	}
	else
		USART1->DR = (c & (uint16_t)0x01FF);
}

void USART1_put_str(uint8_t *s)
{
	while (*s != 0)
		put_char(*s++);
}

uint8_t USART1_get_char(void)
{
	uint8_t data;
	while (rx_buf.counter == 0)
	{
	};
	data = rx_buf.buffer[rx_buf.rd_index++];
	if (rx_buf.rd_index == RX_BUFFER_SIZE)
		rx_buf.rd_index = 0;
	USART1->CR1 &= ~USART_SR_RXNE;
	--rx_buf.counter;
	USART1->CR1 |= USART_SR_RXNE;
	return data;
}

struct USART_CMD *get_usart_cmd(void)
{
	uint8_t data = 0;
	uint8_t i = 0;
	if (rx_buf.counter < 3)
		return NULL;

	while (rx_buf.counter != 0)
	{
		data = rx_buf.buffer[rx_buf.rd_index++];
		if (rx_buf.rd_index == RX_BUFFER_SIZE)
			rx_buf.rd_index = 0;
		USART1->CR1 &= ~USART_SR_RXNE;
		--rx_buf.counter;
		USART1->CR1 |= USART_SR_RXNE;
		if (((data == PREFIX_CMD) || (data == PREFIX_SEARCH) || (data == PREFIX_TEST_SHORT_ADDR) || (data == PREFIX_SEND_PACKET)) && (i == 0))
		{
			usart_cmd.prefix = data;
		}
		else
		{
			if (i == 1)
				usart_cmd.arg1 = data;
			else if (i == 2)
			{
				usart_cmd.arg2 = data;
				break;
			}
		}
		i++;
	}
	if (i == 2)
		return &usart_cmd;
	else
		return NULL;
}

void hex2str(uint8_t *str, unsigned char d)
{

	str[2] = 0;

	if ((d & 0x0F) >= 10)
		str[1] = 'A' + (d & 0x0F) - 10;
	else
		str[1] = '0' + (d & 0x0F);
	d >>= 4;

	if ((d & 0x0F) >= 10)
		str[0] = 'A' + (d & 0x0F) - 10;
	else
		str[0] = '0' + (d & 0x0F);
}
