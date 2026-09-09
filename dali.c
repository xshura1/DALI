#include <stm32f10x.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#include "config.h"
#include "dali.h"
#include "dali_config.h"
#include "timers.h"
#include "uart.h"

#define SLICE_TIME 10
// все значения ниже, выражены в мкс*10 ----------------------------------------
#define TX_TIME_SEND_HALF_BIT 42		// время отправки полубита IEC 62386-101:2014 table 18
#define TIME_STOP_CONDITION 240			// время формирования окончания отправки/приема пакета  IEC 62386-101:2014 table 18,19
#define RX_MAX_TIME_BACKWARD_FRAME 1050 // время ожидания ответного пакета

#define RX_MIN_TIME_BEGIN_VIOLATION 75 - SLICE_TIME // минимальное время, после которого полубит считается ошибочным (при передаче начального полубита)  IEC 62386-101:2014 table 18
#define RX_MIN_TIME_END_VIOLATION 120 - SLICE_TIME	// минимальное время, после которого полубит считается ошибочным (при окончании передачи конечного полубита)  IEC 62386-101:2014 table 19
#define RX_MAX_TIME_VIOLATION 140 + SLICE_TIME		// максимальное время, после которого полубит считается ошибочным  IEC 62386-101:2014 table 18,19
#define RX_MAX_TIME_END_2_HALF_BIT 100 + SLICE_TIME // принято два полубита IEC 62386-101:2014 table 19
#define RX_MIN_TIME_HALF_BIT 33 - SLICE_TIME		// минимальное время принятия одного полубита IEC 62386-101:2014 table 18,19
#define RX_MAX_TIME_HALF_BIT 50 + SLICE_TIME		// максимальное время принятия одного полубита IEC 62386-101:2014 table 18,19

//-------------------------------IEC 62386-101:2014 Table 18-------------------------------------
//						Minimum												Maximum										Description
//
//			RX_MIN_TIME_HALF_BIT						RX_MAX_TIME_HALF_BIT						Half bit
//			RX_MIN_TIME_BEGIN_VIOLATION			RX_MAX_TIME_VIOLATION						Bit timing violation
//			TIME_STOP_CONDITION																							Stop condition
//-----------------------------------------------------------------------------------------------

//-------------------------------IEC 62386-101:2014 Table 19-------------------------------------
//						Minimum												Maximum										Description
//
//			RX_MIN_TIME_HALF_BIT						RX_MAX_TIME_HALF_BIT						Half bit
//																			RX_MAX_TIME_END_2_HALF_BIT			2 half bits
//			RX_MIN_TIME_END_VIOLATION				RX_MAX_TIME_VIOLATION						Bit timing violation
//			TIME_STOP_CONDITION																							Stop condition
//-----------------------------------------------------------------------------------------------

volatile struct BUS gBuses[BUS_COUNT];
volatile struct DEVICE gDevices[ALL_DEVICE_COUNT];
void tx_port_init(GPIO_TypeDef *port, uint8_t no);
void rx_port_init(GPIO_TypeDef *port, uint8_t no);
void ena_port_init(GPIO_TypeDef *port, uint8_t port_no);

void check_rx(void);

static void send_level(volatile struct BUS *bus, uint8_t level)
{
	if (bus == NULL)
		return;
	// старая схема (моя)
	if (level)
		// if (!level)
		bus->tx_port->ODR |= (1 << bus->tx_port_no);
	else
		bus->tx_port->ODR &= ~(1 << bus->tx_port_no);
}

volatile uint16_t tmp_cnt_int = 0;

void bus_handler(volatile struct BUS *bus, uint8_t is_int)
{
	if (bus == NULL)
		return;
	uint8_t bus_level = ((bus->rx_port->IDR >> bus->rx_port_no) & 0x01);

	if ((bus->state == BS_END_STOP_CONDITION) && is_int)
	{
		bus->state = BS_RX;
	}

	if ((bus->state == BS_READY) && is_int)
	{
		bus->state = BS_RX;
	}

	uint8_t bit_val = 0;

	uint32_t diff_time = 0;
	char str_buf[255];

	switch (bus->state)
	{
	case BS_READY: // если на шине происходит передача от другого устройства, то шину выставляем в статус занято
		if (is_int && !bus_level)
		{
			bus->settling_10us = get_time_10_us();
			bus->state = BS_BUSY;
		}
		break;

	case BS_BUSY:
		if (get_diff_time_10_us(bus->settling_10us) < 135)
			return; // TODO пересмотреть приоритет, и выставить нужный
		bus->state = BS_READY;
		break;

	case BS_TX:
		// время выполнения 1.5us
		if (get_diff_time_10_us(bus->time_10us) < TX_TIME_SEND_HALF_BIT)
			return;

#ifdef TEST_BS_TX
		if (bus->tx_bit_cnt_send == 0)
		{
			//	GPIOB->BSRR |= GPIO_BSRR_BS0;
		}
#endif

		bit_val = ((bus->tx_buf >> (bus->tx_bit_cnt - bus->tx_bit_cnt_send - 1)) & 1);
		send_level(bus, (bit_val == bus->is_half_bit));

		if (bus->is_half_bit)
		{
			bus->tx_bit_cnt_send++;
			if (bus->tx_bit_cnt == bus->tx_bit_cnt_send)
			{
				if (!bit_val) // если последний бит 0, то поднимем шину через ~420us
					bus->state = BS_END_TX;
				else
					bus->state = BS_BEGIN_STOP_CONDITION; // если последний бит был 1, то переходим к процедуре формирования остановки передачи
			}
		}
		bus->is_half_bit = ~bus->is_half_bit;
		bus->time_10us = get_time_10_us();
		break;

	case BS_END_TX:
		if (get_diff_time_10_us(bus->time_10us) < TX_TIME_SEND_HALF_BIT)
			return;

		send_level(bus, 1);
		bus->time_10us = get_time_10_us();
		bus->state = BS_BEGIN_STOP_CONDITION;
		break;

	case BS_BEGIN_STOP_CONDITION:
		if (get_diff_time_10_us(bus->time_10us) < TIME_STOP_CONDITION)
			return; // если время меньше 2.4ms то уходим
					// если время >=2.4ms то сформировали stop condition
		bus->is_half_bit = 0;
		bus->rx_bit_cnt = 0;
		bus->rx_buf = 0;
		bus->tx_bit_cnt = 0;
		bus->is_rx = 0;
		bus->is_error_rx = 0;
		tmp_cnt_int = 0;
		bus->state = BS_END_STOP_CONDITION;

		break;

	case BS_END_STOP_CONDITION:
		if (get_diff_time_10_us(bus->time_10us) < RX_MAX_TIME_BACKWARD_FRAME)
			return;			   // если время меньше 10.5ms (время считается от начала формирования stop condition), то уходим. В этом промежутке может прийти ответ
		bus->state = BS_READY; // ответ не пришел, но может прийти другой входящий пакет
// bus->state = BS_WAIT_HANDLING;
#ifdef TEST_BS_TX
							   // GPIOB->BSRR |= GPIO_BSRR_BR0;
#endif
		break;

	case BS_RX:
#ifdef TEST_BS_TX
		if (bus_level)
			GPIOB->BSRR |= GPIO_BSRR_BR0;
		else
			GPIOB->BSRR |= GPIO_BSRR_BS0;
// return;
#endif

		if (!bus->is_half_bit && !bus->rx_bit_cnt && is_int)
		{									   // приняли стартовый полубит ответа
			bus->time_10us = get_time_10_us(); // синхронизируем время
			bus->is_half_bit = 1;
			return;
		}

		diff_time = get_diff_time_10_us(bus->time_10us);

		// окончание принятия полубита
		if (bus->is_half_bit)
		{
			if (is_int)
			{

				// считываем значение бита
				bus->rx_buf = bus->rx_buf << 1;

				if (!bus_level)
					bus->rx_buf |= 1;
				else
					bus->rx_buf &= ~1;

				bus->rx_bit_cnt++;
				bus->is_half_bit = 0;
			}
			else
			{
				if ((diff_time > RX_MIN_TIME_BEGIN_VIOLATION) && (diff_time < RX_MAX_TIME_VIOLATION) && is_int)
				{
					// ошибка принятия бита
					bus->is_error_rx = 1;
					// bus->state = BS_WAIT_HANDLING;
				}
				else
				{
					if (diff_time < TIME_STOP_CONDITION)
						return; // игнорируем бит, если он пришел ДО отработки STOP CONDITION
					// stop condition
					bus->is_half_bit = 0;
					bus->state = BS_READY;
					// bus->state = BS_WAIT_HANDLING;
					bus->is_rx = 1;
					// sprintf(str_buf, "\r\n val1 = %d", bus->rx_buf);
					// USART1_put_str((uint8_t *)str_buf);
				}
			}
		}
		else
		{
			if (is_int)
			{
				// начало принятия нового бита
				if ((diff_time > RX_MIN_TIME_HALF_BIT) && (diff_time < RX_MAX_TIME_HALF_BIT))
				{
					bus->is_half_bit = 1;
				}
				else if (diff_time < RX_MAX_TIME_END_2_HALF_BIT)
				{
					// двойной half бит, считываем значение бита
					bus->rx_buf = bus->rx_buf << 1;

					if (!bus_level)
						bus->rx_buf |= 1;
					else
						bus->rx_buf &= ~1;

					bus->rx_bit_cnt++;
				}
			}
			else
			{
				if ((diff_time > RX_MIN_TIME_END_VIOLATION) && (diff_time < RX_MAX_TIME_VIOLATION) && is_int)
				{
					// ошибка принятия бита
					bus->is_error_rx = 1;
				}
				else
				{
					if (diff_time < TIME_STOP_CONDITION)
						return;
					// stop condition
					bus->is_half_bit = 0;
					bus->state = BS_READY;
					// bus->state = BS_WAIT_HANDLING;
					bus->is_rx = 1;
					// sprintf(str_buf, "\r\n val2 = %d", bus->rx_buf);
					// USART1_put_str((uint8_t *)str_buf);
				}
			}
		}

		if (is_int)
		{
			bus->time_10us = get_time_10_us();
			tmp_cnt_int++;
		}
		break;

		/*case BS_WAIT_HANDLING:

		break;	*/

	default:
		break;
	}
}

void rx_bus_handler(uint8_t bus_no)
{
	/*	struct BUS *bus = (struct BUS *)&gBuses[bus_no];
		if (bus != curr_bus_tx) return; //TODO пока заглушка

		uint8_t rx_bit = ((bus->rx_port->IDR >> bus->rx_port_no) & 0x01);


		if (bus->state == BS_BEGIN_RX){									// после передачи, если пришла единичка, значит пришел стартовый полубит ответа
			if (rx_bit){
				bus->is_half_bit = 1;
			}else{
				if (bus->is_half_bit){											// отработали стартовый бит
					bus->is_half_bit = 0;
					bus->rx_bit_cnt = 0;
					bus->rx_buf = 0;
					bus->is_rx = 0;
					bus->is_error_rx = 0;
					bus->is_075_slice_end = 0;
					bus->sync_slice_cnt = get_time_25_us();
					bus->state = BS_RX;
				}else{
					bus->is_error_rx = 1;
					bus->state = BS_END_RX;
				}
			}
		}else{
			if (bus->is_075_slice_end && (bus->state == BS_RX)){										// проверяем интервал длительностью 75% от фрейма приема (833.3us)
				bus->sync_slice_cnt = get_time_25_us();			// если отработано, то опять выставляем выдержку
				bus->is_075_slice_end = 0;
			}
		}	*/
}

void check_rx(void)
{
	/*if (curr_bus_tx->state != BS_RX) return;
	if (get_diff_time_25_us(curr_bus_tx->sync_slice_cnt) < SYNC_SLICE_CNT_END) return;

	uint8_t rx_bit = ((curr_bus_tx->rx_port->IDR >> curr_bus_tx->rx_port_no) & 0x01);

	if (rx_bit)
		curr_bus_tx->rx_buf |= (1 << ( 7 - curr_bus_tx->rx_bit_cnt));
	else
		curr_bus_tx->rx_buf &= ~(1 << ( 7 - curr_bus_tx->rx_bit_cnt));

	curr_bus_tx->rx_bit_cnt++;
	if (curr_bus_tx->rx_bit_cnt == 8){
		curr_bus_tx->is_rx = 1;
		curr_bus_tx->slice_half_bit_cnt = 0;
		curr_bus_tx->state = BS_END_RX;
	}
	curr_bus_tx->sync_slice_cnt = get_time_25_us();
	curr_bus_tx->is_075_slice_end = 1;
	*/
}

volatile struct BUS *get_bus(uint8_t rx_no)
{
	volatile struct BUS *res = NULL;

	for (uint8_t i = 0; i < BUS_COUNT; i++)
	{
		if (BUS_MAP[i].rx_port_no == rx_no)
		{
			res = &gBuses[i];
			break;
		}
	}
	return res;
}

void dali_init(void)
{

	for (uint8_t i = 0; i < BUS_COUNT; i++)
	{
		gBuses[i].tx_port = BUS_MAP[i].tx_port;
		gBuses[i].tx_port_no = BUS_MAP[i].tx_port_no;
		gBuses[i].rx_port = BUS_MAP[i].rx_port;
		gBuses[i].rx_port_no = BUS_MAP[i].rx_port_no;
		gBuses[i].ena_port = BUS_MAP[i].ena_port;
		gBuses[i].ena_port_no = BUS_MAP[i].ena_port_no;
	}

	for (uint8_t i = 0; i < BUS_COUNT; i++)
	{
		gBuses[i].begin_device = (struct DEVICE *)&gDevices[i * MAX_BUS_DEVICES_COUNT * sizeof(struct DEVICE)];
		gBuses[i].devices_cnt = 0;
		tx_port_init(gBuses[i].tx_port, gBuses[i].tx_port_no);
		rx_port_init(gBuses[i].rx_port, gBuses[i].rx_port_no);
		ena_port_init(gBuses[i].ena_port, gBuses[i].ena_port_no);
	}

	for (uint8_t i = 0; i < BUS_COUNT; i++)
	{
		gBuses[i].state = BS_READY;
	}

	bus_ena(&gBuses[0], 1);
	// bus_ena(&gBuses[1], 1);
}

// конфигурация порта на выход
void tx_port_init(GPIO_TypeDef *port, uint8_t port_no)
{
	if (port == NULL)
		return;
	if (port_no > 7)
	{
		port->CRH &= ~(1 << ((port_no - 8) * 4)); // MODE = 0x10 Output mode, max speed 2 MHz.
		port->CRH |= (1 << ((port_no - 8) * 4 + 1));
		port->CRH &= ~(1 << ((port_no - 8) * 4 + 2)); // CNF = 0x00 General purpose output push-pull
		port->CRH &= ~(1 << ((port_no - 8) * 4 + 3));
	}
	else
	{
		port->CRL &= ~(1 << (port_no * 4)); // MODE = 0x10 Output mode, max speed 2 MHz.
		port->CRL |= (1 << (port_no * 4 + 1));
		port->CRL &= ~(1 << (port_no * 4 + 2)); // CNF = 0x00 General purpose output push-pull
		port->CRL &= ~(1 << (port_no * 4 + 3));
	}
	// старая схема (моя)
	port->ODR |= (1 << port_no); // выставляем единичку

	// новая схема
	// port->ODR &= ~(1 << port_no);								// выставляем ноль
}

// конфигурация порта на вход
void rx_port_init(GPIO_TypeDef *port, uint8_t port_no)
{
	if (port == NULL)
		return;

	uint8_t afio_exti = 0;

	if (port_no > 7)
	{
		port->CRH &= ~(1 << ((port_no - 8) * 4)); // MODE = 0x00 Input mode (reset state)
		port->CRH &= ~(1 << ((port_no - 8) * 4 + 1));
		port->CRH &= ~(1 << ((port_no - 8) * 4 + 2)); // CNF = 0x10 Input with pull-up / pull-down
		port->CRH |= (1 << ((port_no - 8) * 4 + 3));
	}
	else
	{
		port->CRL &= ~(1 << (port_no * 4)); // MODE = 0x00 Input mode (reset state)
		port->CRL &= ~(1 << (port_no * 4 + 1));
		port->CRL &= ~(1 << (port_no * 4 + 2)); // CNF = 0x10 Input with pull-up / pull-down
		port->CRL |= (1 << (port_no * 4 + 3));
	}
	port->ODR |= (1 << port_no); // выставляем единичку

	// настройка прерывания для порта
	if (port == GPIOA)
		afio_exti = 0x00;
	else if (port == GPIOB)
		afio_exti = 0x01;
	else if (port == GPIOC)
		afio_exti = 0x02;

	AFIO->EXTICR[port_no / 4] |= (afio_exti << ((port_no % 4) * 4));
	EXTI->FTSR |= (1 << port_no);
	EXTI->RTSR |= (1 << port_no);
	EXTI->IMR |= (1 << port_no); // разрешаем прерывания

	switch (port_no)
	{
	case 0:
		NVIC_EnableIRQ(EXTI0_IRQn);
		break;
	case 1:
		NVIC_EnableIRQ(EXTI1_IRQn);
		break;
	case 2:
		NVIC_EnableIRQ(EXTI2_IRQn);
		break;
	case 3:
		NVIC_EnableIRQ(EXTI3_IRQn);
		break;
	case 4:
		NVIC_EnableIRQ(EXTI4_IRQn);
		break;
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
		NVIC_EnableIRQ(EXTI9_5_IRQn);
		break;
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
	case 15:
		NVIC_EnableIRQ(EXTI15_10_IRQn);
		break;
	}
}

void ena_port_init(GPIO_TypeDef *port, uint8_t port_no)
{
	if (port == NULL)
		return;
	if (port_no > 7)
	{
		port->CRH &= ~(1 << ((port_no - 8) * 4)); // MODE = 0x10 Output mode, max speed 2 MHz.
		port->CRH |= (1 << ((port_no - 8) * 4 + 1));
		port->CRH &= ~(1 << ((port_no - 8) * 4 + 2)); // CNF = 0x00 General purpose output push-pull
		port->CRH &= ~(1 << ((port_no - 8) * 4 + 3));
	}
	else
	{
		port->CRL &= ~(1 << (port_no * 4)); // MODE = 0x10 Output mode, max speed 2 MHz.
		port->CRL |= (1 << (port_no * 4 + 1));
		port->CRL &= ~(1 << (port_no * 4 + 2)); // CNF = 0x00 General purpose output push-pull
		port->CRL &= ~(1 << (port_no * 4 + 3));
	}
	// старая схема (моя)
	port->ODR |= (1 << port_no); // выставляем единичку, изначально питание на шину не подаем
}

void bus_ena(struct BUS *bus, uint8_t is_ena)
{
	if (bus == NULL)
		return;
	if (is_ena)
	{
		bus->ena_port->ODR &= ~(1 << bus->ena_port_no);
	}
	else
	{
		bus->ena_port->ODR |= (1 << bus->ena_port_no);
	}
}

struct DALI_CMD_RESPONSE __dali_send_command(uint8_t bus_no, uint8_t arg1, uint8_t arg2)
{
	struct DALI_CMD_RESPONSE res;
	res.result = DCR_OK;
	volatile struct BUS *tbus = &gBuses[bus_no];

	//	while (tbus->state != BS_READY){
	//		asm("NOP");
	//	}

	if (tbus->state != BS_READY)
	{
		res.result = DCR_ERROR;
		return res;
	}

	// tbus = &gBuses[bus_no];
	// curr_bus_tx->slice_half_bit_cnt = 0;
	tbus->is_half_bit = 0;
	tbus->tx_buf = 0;
	tbus->tx_buf = (1 << 16) | (arg1 << 8) | arg2;
	tbus->tx_bit_cnt = 17;
	tbus->tx_bit_cnt_send = 0;
	tbus->is_rx = 0;
	tbus->is_error_rx = 0;
	// curr_bus_tx->is_rx = 0;
	// curr_bus_tx->is_error_rx = 0;
	tbus->rx_buf = 0;
	tbus->time_10us = TX_TIME_SEND_HALF_BIT;
	tbus->state = BS_TX;

	while (tbus->state != BS_READY)
	{
		check_buses();
		if (tbus->state == BS_READY)
			break;
	}
	if (tbus->is_error_rx)
	{
		res.result = DCR_ERROR;
	}
	else if (tbus->is_rx)
	{
		res.result = DCR_RESPONSE;
		res.response = (uint8_t)tbus->rx_buf;
	}
	else
		res.result = DCR_OK;

	return res;
}

struct DALI_CMD_RESPONSE __dali2_send_command(uint8_t bus_no, uint8_t addr, uint8_t inst, uint8_t op)
{
	struct DALI_CMD_RESPONSE res;
	res.result = DCR_OK;
	volatile struct BUS *tbus = &gBuses[bus_no];

	//	while (tbus->state != BS_READY){
	//		asm("NOP");
	//	}

	if (tbus->state != BS_READY)
	{
		res.result = DCR_ERROR;
		return res;
	}

	// tbus = &gBuses[bus_no];
	// curr_bus_tx->slice_half_bit_cnt = 0;
	tbus->is_half_bit = 0;
	tbus->tx_buf = 0;
	tbus->tx_buf = (1 << 24) | (addr << 16) | (inst << 8) | op;
	tbus->tx_bit_cnt = 25;
	tbus->tx_bit_cnt_send = 0;
	tbus->is_rx = 0;
	tbus->is_error_rx = 0;
	// curr_bus_tx->is_rx = 0;
	// curr_bus_tx->is_error_rx = 0;
	tbus->rx_buf = 0;
	tbus->time_10us = TX_TIME_SEND_HALF_BIT;
	tbus->state = BS_TX;

	while (tbus->state != BS_READY)
	{
		check_buses();
		if (tbus->state == BS_READY)
			break;
	}
	if (tbus->is_error_rx)
	{
		res.result = DCR_ERROR;
	}
	else if (tbus->is_rx)
	{
		res.result = DCR_RESPONSE;
		res.response = (uint8_t)tbus->rx_buf;
	}
	else
		res.result = DCR_OK;

	return res;
}

uint8_t check_buses(void)
{
	uint8_t res = 1;
	for (uint8_t i = 0; i < BUS_COUNT; i++)
	{
		bus_handler(&gBuses[i], 0);
	}
	for (uint8_t i = 0; i < BUS_COUNT; i++)
	{
		if (gBuses[i].state != BS_READY)
		{
			res = 0;
			break;
		} /*else{
			 if (gBuses[i].is_rx){
				 gBuses[i].is_rx = 0;
				 gBuses[i].rx_bit_cnt = 0;
				 gBuses[i].rx_buf = 0;
			 }
		 }*/
	}
	return res;
}
