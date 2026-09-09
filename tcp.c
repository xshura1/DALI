#include <stdio.h>
#include <string.h>

#include "tcp.h"
#include "spi.h"
#include "wizchip_conf.h"
#include "socket.h"
#include "uart.h"
#include "timers.h"

uint8_t gDATABUF[DATA_BUF_SIZE];
uint8_t tcp_tx_buf[DATA_BUF_SIZE];
uint16_t tx_size = 0;
uint8_t tcp_rx_buf[DATA_BUF_SIZE];
uint16_t rx_size = 0;

int32_t tcp_server(void)
{
	int32_t ret;
	uint16_t size = 0, sentsize = 0;
	// дожидаемся обнуления регистра Sn_CR только после этого можно подавать команды для сокета
	// while (getSn_CR(SOCK_TCP) != 0){
	// return 1;
	//}

	switch (getSn_SR(SOCK_TCP))
	{
	case SOCK_ESTABLISHED:
		if (getSn_IR(SOCK_TCP) & Sn_IR_CON)
		{
			// USART1_put_str((uint8_t *)"\r\nConnected");
			setSn_IR(SOCK_TCP, Sn_IR_CON);
		}
		/*
		if((rx_size = getSn_RX_RSR(SOCK_TCP)) > 0){
			   if(rx_size > DATA_BUF_SIZE) rx_size = DATA_BUF_SIZE;
			   ret = recv(SOCK_TCP,tcp_rx_buf,rx_size);
			   if (ret <=0){
				   rx_size = 0;
			   }
	   }

			//if(ret <= 0) return ret;
	   if (tx_size){
			   sentsize = 0;
			   while(tx_size != sentsize)
			   {
					ret = send(SOCK_TCP,tcp_tx_buf+sentsize,tx_size-sentsize);
					if(ret < 0)
					{
						   close(SOCK_TCP);
						   return ret;
					}
					sentsize += ret; // Don't care SOCKERR_BUSY, because it is zero.
			   }

		}
	  tx_size = 0;*/
		break;

	case SOCK_LISTEN:
		// USART1_put_str((uint8_t *)"\r\nListen!");
		break;

	case SOCK_CLOSE_WAIT:
		// USART1_put_str((uint8_t *)"\r\nCloseWait");
		disconnect(SOCK_TCP);
		break;
	case SOCK_INIT:
		listen(SOCK_TCP);

		break;
	case SOCK_CLOSED:
		if (socket(SOCK_TCP, Sn_MR_TCP, LOCAL_PORT_TCP, 0x00) == SOCK_TCP)
		{
			// USART1_put_str((uint8_t *)"\r\nSocket open");
		}
		break;
	default:
		break;
	}
	return 1;
}

char *get_tcp_data(void)
{
	// tcp_server();
	// if (getSn_SR(SOCK_TCP) != SOCK_ESTABLISHED) return NULL;
	int32_t ret;
	if ((rx_size = getSn_RX_RSR(SOCK_TCP)) > 0)
	{
		if (rx_size > DATA_BUF_SIZE)
			rx_size = DATA_BUF_SIZE;
		ret = recv(SOCK_TCP, tcp_rx_buf, rx_size);
		if (ret <= 0)
		{
			rx_size = 0;
		}
	}
	/*if (getSn_IR(SOCK_TCP) & Sn_IR_RECV){
		setSn_IR(SOCK_TCP, Sn_IR_RECV);
	}*/
	if (!rx_size)
		return NULL;
	return (char *)tcp_rx_buf;
}

void clear_tcp_data(void)
{
	rx_size = 0;
}

void send_tcp_data(const char *str)
{
	uint16_t sentsize = 0;
	int32_t ret;
	// return;
	tx_size = strlen((char *)str);

	if (tx_size)
	{
		// delay_10us(50); // задержка влияет на вывод строк
		sentsize = 0;
		while (tx_size != sentsize)
		{
			ret = send(SOCK_TCP, str + sentsize, tx_size - sentsize);
			if (ret < 0)
			{
				close(SOCK_TCP);
				return;
			}
			sentsize += ret; // Don't care SOCKERR_BUSY, because it is zero.
		}
	}
	/*if (getSn_IR(SOCK_TCP) & Sn_IR_SENDOK){
		setSn_IR(SOCK_TCP, Sn_IR_SENDOK);
	}*/
}

void net_init(void)
{
	reg_wizchip_cs_cbfunc(SS_SELECT, SS_DESELECT);
	reg_wizchip_spi_cbfunc(SPI2_read, SPI2_write);
	reg_wizchip_spiburst_cbfunc(SPI2_read_buf, SPI2_write_buf);
	wizchip_sw_reset();
	delay_ms(10);

	uint8_t buf_size[] = {2, 2, 2, 2, 2, 2, 2, 2};

	if (wizchip_init(buf_size, buf_size) != 0)
	{
		asm("NOP");
	}

	wiz_NetInfo netInfo = {
		.mac = MAC_ADDR,	// MAC адрес
		.ip = IP_ADDR,		// IP адрес
		.sn = NET_MASK,		// маска сети
		.gw = GATEWAY_ADDR, // адрес шлюза
		.dns = {0, 0, 0, 0},
		.dhcp = NETINFO_STATIC};

	wizchip_setnetinfo(&netInfo);
	wizchip_getnetinfo(&netInfo);
	setSn_IMR(SOCK_TCP, 0x04);
	// setSn_IMR(SOCK_TCP, 0x17);
	setSIMR(0x08); // аппаратное прерывание
}
