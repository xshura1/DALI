#ifndef UART_H_
#define UART_H_

#include "stdint.h"

#define PREFIX_CMD 0xAA
#define PREFIX_SEARCH 0xBB
#define PREFIX_TEST_SHORT_ADDR 0xCC
#define PREFIX_SEND_PACKET 0xDD

struct USART_CMD
{
	uint8_t prefix;
	uint8_t arg1;
	uint8_t arg2;
};

void USART1_init(void);
void USART1_send_char(char c);
void USART1_send_str(char *str);
void USART1_put_str(uint8_t *s);
void hex2str(uint8_t *str, unsigned char d);
uint8_t USART1_get_char(void);

struct USART_CMD *get_usart_cmd(void);

#endif /*UART_H_*/
