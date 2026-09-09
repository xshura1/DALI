#ifndef NET_H_
#define NET_H_

#include <stdint.h>

#define LOCAL_PORT_TCP 789
#define IP_ADDR {192, 168, 1, 19}
#define MAC_ADDR {0x00, 0x15, 0x42, 0xBF, 0xF0, 0x51}
#define MAC_BROADCAST {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}
#define MAC_NULL {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}

#define NET_MASK {255, 255, 255, 0}   // маска сети
#define GATEWAY_ADDR {192, 168, 1, 1} // адрес шлюза

#define SOCK_TCP 3

#define DATA_BUF_SIZE 2048

void net_init(void);
char *get_tcp_data(void);
void clear_tcp_data(void);
void send_tcp_data(const char *str);
int8_t sock_tcp_init(void);
uint8_t is_sock_closed(void);
void get_int_socket(void);

int32_t tcp_server(void);

#endif /* NET_H_ */
