
#include <stdint.h>
#include <stddef.h>

#include "dali_cmd.h"
#include "dali.h"
#include "uart.h"
#include "timers.h"
#include "tcp.h"

#define MAX_SHORT_ADDRESS 64

#define BROADCAST_ADDR 0b11111111

#define RESET 0b00100000

#define DOWN 0b00000010
#define STEP_DOWN 0b00000100
#define UP 0b00000001
#define OFF 0b00000000 // yes
#define ON_AND_STEP_UP 0b00000101
#define QUERY_VERSION_NUMBER 0b10010111 // yes
#define QUERY_BALLAST 0b10010001		// no
#define QUERY_ACTUAL_LEVEL 0b10100000	// yes
#define QUERY_LAMP_FAILURE 0b10010010	// no
#define QUERY_LAMP_POWER_ON 0b10010011	// при выключенной - не отвечает, при включенной - отвечает 0xFF
#define RECALL_MAX_LEVEL 0b00000101		// включает на полную (MAX_LEVEL)
#define QUERY_MAX_LEVEL 0b10100001		// yes
#define QUERY_MIN_LEVEL 0b10100010		// yes

#define QUERY_STATUS 0b10010000
/*
Standard ECGs
Bit 0: status of ballast; 0 = OK
Bit 1: lamp failure; 0 = OK
Bit 2: lamp arc power on; 0 = OFF
Bit 3: limit error; 0 = "last requested power was OFF or was between MIN..MAX LEVEL"
Bit 4: fade ready; 0 = ready, 1 = running
Bit 5: reset state? 0 = NO
Bit 6: missing short address? 0 = NO
Bit 7: power failure? 0 = "RESET" or an arc power control command has been received since the last power-on
*/

#define QUERY_SYSTEM_FAILURE_LEVEL 0b10100100
#define INITIALISE 0xA5
#define RANDOMISE 0xA7
#define SEARCHADDRH 0xB1
#define SEARCHADDRM 0xB3
#define SEARCHADDRL 0xB5
#define PRG_SHORT_ADDR 0xB7
#define COMPARE 0xA9
#define WITHDRAW 0xAB
#define TERMINATE 0xA1
#define QUERY_SHORT_ADDRESS 0b10111011

#define TWO_PACKET_DELAY 10 // ms

void print_hex_search_addr(uint32_t search_addr)
{
	uint8_t buf[3];
	USART1_put_str((uint8_t *)"0x");
	send_tcp_data("0x");
	hex2str((uint8_t *)&buf, (search_addr >> 16));
	USART1_put_str((uint8_t *)&buf);
	send_tcp_data((uint8_t *)&buf);
	hex2str((uint8_t *)&buf, (search_addr >> 8));
	USART1_put_str((uint8_t *)&buf);
	send_tcp_data((uint8_t *)&buf);
	hex2str((uint8_t *)&buf, search_addr);
	USART1_put_str((uint8_t *)&buf);
	send_tcp_data((uint8_t *)&buf);
}

void test_short_address(uint8_t bus_no, uint8_t is_show)
{
	uint8_t addr = 0;
	uint8_t addr_c = 0;
	uint8_t buf[3];
	struct DALI_CMD_RESPONSE res;

	if (is_show)
	{
		USART1_put_str((uint8_t *)"\r\nStart test short address");
		send_tcp_data((uint8_t *)"\r\nStart test short address");
	}
	res = __dali_send_command(bus_no, BROADCAST_ADDR, OFF);

	while (addr < 64)
	{
		addr_c = (1 + (addr << 1));
		res = __dali_send_command(bus_no, addr_c, QUERY_VERSION_NUMBER);
		if (res.result == DCR_RESPONSE)
		{
			if (is_show)
			{
				USART1_put_str((uint8_t *)"\r\nFound short address: 0x");
				send_tcp_data((uint8_t *)"\r\nFound short address: 0x");
				hex2str((uint8_t *)&buf, addr);
				USART1_put_str((uint8_t *)&buf);
				send_tcp_data(&buf);
			}
			res = __dali_send_command(bus_no, addr_c, ON_AND_STEP_UP);
			delay_ms(1000);
			res = __dali_send_command(bus_no, addr_c, OFF);
			delay_ms(1000);
		}
		addr++;
	}
	USART1_put_str((uint8_t *)"\r\nEnd test short address");
	send_tcp_data((uint8_t *)"\r\nEnd test short address");
}

uint8_t search_and_compare(uint8_t bus_no, uint32_t address)
{
	// передаем случайный адрес 24 бита тремя пакетами три раза //TODO почему три раза????
	for (uint8_t i = 0; i < 3; i++)
	{
		__dali_send_command(bus_no, SEARCHADDRH, address >> 16);
		__dali_send_command(bus_no, SEARCHADDRM, address >> 8);
		__dali_send_command(bus_no, SEARCHADDRL, address);
	}

	struct DALI_CMD_RESPONSE res = __dali_send_command(bus_no, COMPARE, 0x00);

	if (res.result == DCR_ERROR)
		return 2;
	else if (res.result == DCR_RESPONSE)
		return 1;
	else
		return 0;
}

uint8_t search_and_compare2(uint8_t bus_no, uint32_t address)
{
	// передаем случайный адрес 24 бита тремя пакетами три раза
	// for (uint8_t i = 0; i < 3; i++){
	__dali2_send_command(bus_no, 0xC1, 0x05, address >> 16);
	__dali2_send_command(bus_no, 0xC1, 0x06, address >> 8);
	__dali2_send_command(bus_no, 0xC1, 0x07, address);

	//}

	struct DALI_CMD_RESPONSE res = __dali2_send_command(bus_no, 0xC1, 0x03, 0x00);

	if (res.result == DCR_ERROR)
		return 2;
	else if (res.result == DCR_RESPONSE)
		return 1;
	else
		return 0;
}

void send_progress_set_addr(char *str, uint32_t search_addr)
{
	USART1_put_str((uint8_t *)str);
	send_tcp_data(str);
	print_hex_search_addr(search_addr);
}

void set_address(uint8_t bus_no, uint8_t start_short_addr, uint8_t show_progress, uint8_t onadr, enum DALI_FRAME_FORMAT fmt)
{

	uint32_t low_addr = 0;
	uint32_t high_addr = 0xFFFFFF;
	uint32_t search_addr = (uint32_t)((low_addr + high_addr) / 2); // 0xFFFFFF;
	uint8_t initialise_arg = (onadr == 1 ? 0xFF : 0x00);

	uint8_t response = 0;
	uint8_t short_addr = start_short_addr;
	uint8_t buf[3];

	USART1_put_str((uint8_t *)"\r\nStart set address");

	// дважды подаем команду RESET
	__dali_send_command(bus_no, BROADCAST_ADDR, RESET);
	delay_ms(2 * TWO_PACKET_DELAY);
	__dali_send_command(bus_no, BROADCAST_ADDR, RESET);
	delay_ms(100);
	// выключаем все устройства
	__dali_send_command(bus_no, BROADCAST_ADDR, OFF);

	// дважды подаем команду INITIALISE
	__dali_send_command(bus_no, INITIALISE, initialise_arg);
	__dali_send_command(bus_no, INITIALISE, initialise_arg);

	// дважды подаем команду RANDOMISE
	__dali_send_command(bus_no, RANDOMISE, 0x00);
	__dali_send_command(bus_no, RANDOMISE, 0x00);

	while ((search_addr <= (0xFFFFFF - 2)) && (short_addr < MAX_SHORT_ADDRESS))
	{
		while ((high_addr - low_addr) > 1)
		{

			if (show_progress == 0xF3)
			{
				send_progress_set_addr("\r\n", search_addr);
				// USART1_put_str((uint8_t *)"\r\n");
				// print_hex_search_addr(search_addr);
			}

			response = search_and_compare(bus_no, search_addr);
			if (response != 2)
			{
				if (show_progress == 0x01)
				{
					send_progress_set_addr("\r\n", search_addr);
					// USART1_put_str((uint8_t *)"\r\n");
					// print_hex_search_addr(search_addr);
				}
				if (response == 1)
				{
					if ((search_addr == 0) || (!search_and_compare(bus_no, search_addr - 1)))
					{
						break;
					}

					high_addr = search_addr;
				}
				else
				{
					low_addr = search_addr;
				}

				search_addr = (uint32_t)((low_addr + high_addr) / 2);
			}
		}

		if (high_addr != 0xFFFFFF)
		{
			search_and_compare(bus_no, search_addr);
			__dali_send_command(bus_no, PRG_SHORT_ADDR, (1 + (short_addr << 1)));
			__dali_send_command(bus_no, WITHDRAW, 0x00);

			send_progress_set_addr("\r\n24-bit address found: ", search_addr);
			// USART1_put_str((uint8_t *)"\r\n24-bit address found: ");
			// print_hex_search_addr(search_addr);

			USART1_put_str((uint8_t *)"	Short address = 0x");
			hex2str((uint8_t *)&buf, short_addr);
			USART1_put_str((uint8_t *)&buf);
			send_tcp_data("Short address = 0x");
			send_tcp_data(&buf);

			short_addr++;
			high_addr = 0xFFFFFF;
			search_addr = (uint32_t)((low_addr + high_addr) / 2);
		}
	}

	__dali_send_command(bus_no, TERMINATE, 0x00);
	USART1_put_str((uint8_t *)"\r\nEnd set address");
	send_tcp_data("\r\nEnd set address");
}

void set_address2(uint8_t bus_no, uint8_t start_short_addr, uint8_t show_progress, uint8_t onadr, enum DALI_FRAME_FORMAT fmt)
{
	uint32_t low_addr = 0;
	uint32_t high_addr = 0xFFFFFF;
	uint32_t search_addr = (uint32_t)((low_addr + high_addr) / 2); // 0xFFFFFF;
	uint8_t initialise_arg = (onadr == 1 ? 0xFF : 0x00);

	uint8_t response = 0;
	uint8_t short_addr = start_short_addr;
	uint8_t buf[3];

	USART1_put_str((uint8_t *)"\r\nStart set address");

	// дважды подаем команду RESET
	//	__dali2_send_command(bus_no, BROADCAST_ADDR, 0xFE, 0x10);
	//	delay_ms(2*TWO_PACKET_DELAY);
	//	__dali2_send_command(bus_no, BROADCAST_ADDR, 0xFE, 0x10);
	//	delay_ms(100);
	// выключаем все устройства
	//__dali_send_command(bus_no, BROADCAST_ADDR, OFF);

	// дважды подаем команду INITIALISE
	__dali2_send_command(bus_no, 0xC1, 0x01, initialise_arg);
	__dali2_send_command(bus_no, 0xC1, 0x01, initialise_arg);

	// дважды подаем команду RANDOMISE
	__dali2_send_command(bus_no, 0xC1, 0x02, 0x00);
	__dali2_send_command(bus_no, 0xC1, 0x02, 0x00);

	while ((search_addr <= (0xFFFFFF - 2)) && (short_addr < MAX_SHORT_ADDRESS))
	{
		while ((high_addr - low_addr) > 1)
		{

			if (show_progress == 0xF3)
			{
				send_progress_set_addr("\r\n", search_addr);
				// USART1_put_str((uint8_t *)"\r\n");
				// print_hex_search_addr(search_addr);
			}

			response = search_and_compare2(bus_no, search_addr);
			if (response != 2)
			{
				if (show_progress == 0x01)
				{
					send_progress_set_addr("\r\n", search_addr);
					// USART1_put_str((uint8_t *)"\r\n");
					// print_hex_search_addr(search_addr);
				}
				if (response == 1)
				{
					if ((search_addr == 0) || (!search_and_compare2(bus_no, search_addr - 1)))
					{
						break;
					}

					high_addr = search_addr;
				}
				else
				{
					low_addr = search_addr;
				}

				search_addr = (uint32_t)((low_addr + high_addr) / 2);
			}
			else
			{
				return; // TODO заглушка
			}
		}

		if (high_addr != 0xFFFFFF)
		{
			search_and_compare2(bus_no, search_addr);
			__dali2_send_command(bus_no, 0xC1, 0x08, short_addr);
			__dali2_send_command(bus_no, 0xC1, 0x04, 0x00);

			send_progress_set_addr("\r\n24-bit address found: ", search_addr);
			// USART1_put_str((uint8_t *)"\r\n24-bit address found: ");
			// print_hex_search_addr(search_addr);

			USART1_put_str((uint8_t *)"	Short address = 0x");
			hex2str((uint8_t *)&buf, short_addr);
			USART1_put_str((uint8_t *)&buf);
			send_tcp_data("Short address = 0x");
			send_tcp_data(&buf);

			short_addr++;
			high_addr = 0xFFFFFF;
			search_addr = (uint32_t)((low_addr + high_addr) / 2);
		}
	}

	__dali2_send_command(bus_no, 0xC1, 0x00, 0x00);
	USART1_put_str((uint8_t *)"\r\nEnd set address");
	send_tcp_data("\r\nEnd set address");
}

void replace_short_addr(uint8_t bus_no, uint8_t old_addr, uint8_t new_addr)
{
	__dali_send_command(bus_no, 0xA3, old_addr);
	__dali_send_command(bus_no, new_addr, 0x80);
	__dali_send_command(bus_no, new_addr, 0x80);
}
