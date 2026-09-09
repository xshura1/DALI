#include "stm32f10x.h"

#include "timers.h"

void SPI2_init(void)
{
	// Включаем тактирование SPI1 и GPIOA
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN;
	RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;

	/**********************************************************/
	/*** Настройка выводов GPIOA на работу совместно с SPI1 ***/
	/**********************************************************/
	// PA8 	- INT
	// PB15	- MOSI
	// PB14 	- MISO
	// PB13 	- SCK

	// Для начала сбрасываем все конфигурационные биты в нули
	//  INT
	GPIOA->CRH &= ~GPIO_CRH_CNF8;
	GPIOA->CRH &= ~GPIO_CRH_MODE8;
	// CS
	GPIOB->CRH &= ~GPIO_CRH_CNF12;
	GPIOB->CRH &= ~GPIO_CRH_MODE12;
	// SCK
	GPIOB->CRH &= ~GPIO_CRH_CNF13;
	GPIOB->CRH &= ~GPIO_CRH_MODE13;
	// MISO
	GPIOB->CRH &= ~GPIO_CRH_CNF14;
	GPIOB->CRH &= ~GPIO_CRH_MODE14;
	// MOSI
	GPIOB->CRH &= ~GPIO_CRH_CNF15;
	GPIOB->CRH &= ~GPIO_CRH_MODE15;

	// Настраиваем
	//  INT
	GPIOA->CRH |= GPIO_CRH_CNF8_0;
	GPIOA->BSRR = GPIO_BSRR_BS8;
	AFIO->EXTICR[2] |= AFIO_EXTICR3_EXTI8_PA;
	EXTI->FTSR |= EXTI_FTSR_TR8; // прерывание по спаду
	// EXTI->RTSR |= EXTI_RTSR_TR8;

	// CS
	GPIOB->CRH |= GPIO_CRH_MODE12;
	GPIOB->BSRR |= GPIO_BSRR_BS12;
	// SCK: MODE13 = 0x03 (11b); CNF13 = 0x02 (10b)
	GPIOB->CRH |= GPIO_CRH_CNF13_1;
	GPIOB->CRH |= GPIO_CRH_MODE13;
	// MISO: MODE14 = 0x00 (00b); CNF14 = 0x01 (01b)
	GPIOB->CRH |= GPIO_CRH_CNF14_0;
	GPIOB->BSRR = GPIO_BSRR_BS14;
	// MOSI: MODE15 = 0x03 (11b); CNF15 = 0x02 (10b)
	GPIOB->CRH |= GPIO_CRH_CNF15_1;
	GPIOB->CRH |= GPIO_CRH_MODE15;

	/**********************/
	/*** Настройка SPI1 ***/
	/**********************/
	SPI2->CR1 &= ~SPI_CR1_DFF;		// Размер кадра 8 бит
	SPI2->CR1 &= ~SPI_CR1_LSBFIRST; // MSB first
	SPI2->CR1 |= SPI_CR1_SSM;		// Программное управление SS
	// SPI1->CR1 &= ~SPI_CR1_SSM;					//Программное управление SS
	SPI2->CR1 |= SPI_CR1_SSI; // SS в высоком состоянии
	SPI2->CR1 |= SPI_CR1_BR_0;
	SPI2->CR1 &= ~SPI_CR1_BR_1;
	SPI2->CR1 &= ~SPI_CR1_BR_2; // Скорость передачи: F_PCLK/4
	SPI2->CR1 |= SPI_CR1_MSTR;	// Режим Master (ведущий)
	SPI2->CR1 &= ~SPI_CR1_CPOL; // Режим работы SPI: 0
	SPI2->CR1 &= ~SPI_CR1_CPHA; // Режим работы SPI: 0

	SPI2->CR1 |= SPI_CR1_SPE; // Включаем SPI

	EXTI->IMR |= EXTI_IMR_MR8; // разрешаем прерывания
}

uint8_t SPI2_write_read(uint8_t data)
{
	// uint16_t time = 0xFF;
	while (!(SPI2->SR & SPI_SR_TXE))
	{
	}
	// заполняем буфер передатчика
	SPI2->DR = data;

	while (!(SPI2->SR & SPI_SR_RXNE))
	{
		//	time++;
		//	if (!time) break;
	}

	// if (!time) return 0xFF;
	// возвращаем значение буфера приемника
	return SPI2->DR;
}

void SPI2_write(uint8_t data)
{
	SPI2_write_read(data);
	// while(!(SPI2->SR & SPI_SR_TXE)) {}
	// заполняем буфер передатчика
	// SPI2->DR = data;
}

uint8_t SPI2_read(void)
{
	return SPI2_write_read(0x00);
	// return b;
	// SPI2->DR = 0;
	// while(!(SPI2->SR & SPI_SR_RXNE)) {}
	// возвращаем значение буфера приемника
	// return SPI2->DR;
}

void SPI2_write_buf(uint8_t *buf, uint16_t len)
{
	for (uint8_t i = 0; i < len; i++)
	{
		SPI2_write(buf[i]);
	}
}

void SPI2_read_buf(uint8_t *buf, uint16_t len)
{
	for (uint16_t i = 0; i < len; i++)
	{
		buf[i] = SPI2_read();
	}
}

void SS_SELECT(void)
{
	// дожидаемся отправки последнего пакета, и потом начинаем обмен
	while (SPI2->SR & SPI_SR_BSY)
	{
	}
	GPIOB->BSRR |= GPIO_BSRR_BR12;
}

void SS_DESELECT(void)
{
	// while(SPI2->SR & SPI_SR_BSY) {
	// asm("NOP");
	//}
	GPIOB->BSRR |= GPIO_BSRR_BS12;
}
