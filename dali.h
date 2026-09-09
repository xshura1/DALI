#ifndef DALI_H_
#define DALI_H_

#include <stm32f10x.h>
#include <stdint.h>

typedef uint8_t slice_half_bit_t;
typedef uint32_t slice_sync_t;

enum DALI_CMD_RESULT
{
	DCR_OK,
	DCR_RESPONSE,
	DCR_ERROR
};

struct DALI_CMD_RESPONSE
{
	enum DALI_CMD_RESULT result;
	uint8_t response;
};

struct DEVICE
{
	uint8_t address;
};

enum BUS_STATE
{
	// BS_NONE,						// не определено
	BS_READY, // шина свободна
	// BS_BEGIN_TX,						// начать передачу в шину
	BS_TX,					 // передача данных
	BS_END_TX,				 // окончание передачи данных
	BS_BEGIN_STOP_CONDITION, // начало формирования периода окончания фрейма - 2,4ms
	BS_END_STOP_CONDITION,	 // окончание формирования периода окончания фрейма - 2,4ms
	BS_WAIT_BACKWARD_FRAME,	 // ожидание ответа на запрос
	// BS_BEGIN_RX,					// принят стартовый бит ответа
	BS_RX, // приём ответа
	// BS_RX_BACKWARD_FRAME,		// прием ответа на запрос
	BS_END_RX,		 // окончание приема данных
	BS_BUSY,		 // на шине происходит обмен
	BS_WAIT_HANDLING // ожидается обработка результата

};

enum BUS_RX_STATE
{
	BRS_NONE,
	BRS_BACKWARD_FRAME,
	BRS_ANY_FRAME
};

struct BUS
{
	GPIO_TypeDef *tx_port;
	GPIO_TypeDef *rx_port;
	GPIO_TypeDef *ena_port;
	unsigned int tx_port_no : 4;
	unsigned int rx_port_no : 4;
	unsigned int ena_port_no : 4;

	struct DEVICE *begin_device; // указатель на начальное устройство для текущей шины
	uint8_t devices_cnt;		 // количество устройств шины

	struct DEVICE *curr_device; // текущее устройство, которым управляет шина
	// slice_sync_t				sync_slice_cnt;
	uint32_t time_10us;		// время в 10us интервалах
	uint32_t settling_10us; // время в 10us интервалах между передачей пакета и приемом первого полубита ответа
	uint32_t tx_buf;
	uint32_t rx_buf;
	uint8_t tx_bit_cnt;		 // сколько должно быть передано бит
	uint8_t rx_bit_cnt;		 // количество принятых битов
	uint8_t tx_bit_cnt_send; // количество переданных битов
	// uint8_t							rx_bit_cnt_send;			// количество принятых битов

	// slice_half_bit_t		slice_half_bit_cnt; 	// количество квантов времени (один квант времени, это передача одного полубита)
	enum BUS_STATE state; // текущий статус шины
	// enum BUS_RX_STATE		rx_state;							// статус приема
	unsigned int is_half_bit : 1;	 // 0-первая половина полубита, 1-вторая половина полубита
	unsigned int half_bit_value : 1; // значение отправленного/принятого бита
	// unsigned int				backward_can_received : 1;	// может быть получен ответ
	// unsigned int				is_start_sync					:	1;	// 1 - синхронизация по началу, 0 - синхронизация по концу
	unsigned int is_rx : 1;		  // получен ответ
	unsigned int is_error_rx : 1; // ошибка приёма
								  // unsigned int			is_075_slice_end	: 1;
};

void dali_init(void);

// отправка одиночной команды в шину
struct DALI_CMD_RESPONSE __dali_send_command(uint8_t bus_no, uint8_t arg1, uint8_t arg2);
// отправка одиночной команды в шину по протоколу DALI2
struct DALI_CMD_RESPONSE __dali2_send_command(uint8_t bus_no, uint8_t addr, uint8_t inst, uint8_t op);
void bus_handler(volatile struct BUS *bus, uint8_t is_int);
void rx_bus_handler(uint8_t bus_no);
void check_rx(void);
uint8_t check_buses(void);

// возвращает шину по номеру Rx порта
volatile struct BUS *get_bus(uint8_t rx_no);

// включение/отключение питания шины от внутреннего источника
void bus_ena(struct BUS *bus, uint8_t is_ena);

#endif /*DALI_H_*/
