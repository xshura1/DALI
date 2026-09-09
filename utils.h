#ifndef UTILS_H_
#define UTILS_H_

#include <stdint.h>

uint32_t str2hex(char *str, char **end);
uint8_t str2hex8bit(char *str, char **end);
uint8_t str_is_empty(char *str);
uint8_t str_is_quotes(char *str, char **out_str);
uint8_t str_is_bits_enum(char *str, uint16_t *out_bits);
uint8_t str_to_int32_t(char *str, int32_t *number, uint8_t *dp_pos);
int random(int min, int max);
uint8_t bcd_to_dec(uint8_t c);
uint8_t dec_to_bcd(uint8_t c);

#endif /* UTILS_H_ */
