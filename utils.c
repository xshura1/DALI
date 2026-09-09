
#include <stdint.h>
#include <stdlib.h>

uint32_t str2hex(char *str, char **end)
{
	uint32_t res = 0;
	uint8_t val = 0;
	uint8_t i = 0;

	while (str[i] != 0)
	{
		if (((str[i] >= '0') && (str[i] <= '9')) || ((str[i] >= 65) && (str[i] <= 70)) || ((str[i] >= 97) && (str[i] <= 102)))
		{
			if (((str[i] >= '0') && (str[i] <= '9')))
				val = str[i] - 48;
			else if ((str[i] >= 65) && (str[i] <= 70))
				val = str[i] - 55;
			else if (((str[i] >= 97) && (str[i] <= 102)))
				val = str[i] - 87;
			if (i > 0)
				res = (res << 4);
			res |= val;
			i++;
		}
		else
			break;
	}

	*end = &str[i];

	return res;
}

uint8_t str2hex8bit(char *str, char **end)
{
	uint32_t res = 0;
	uint8_t val = 0;
	uint8_t i = 0;

	while (str[i] != 0)
	{
		if (((str[i] >= '0') && (str[i] <= '9')) || ((str[i] >= 65) && (str[i] <= 70)) || ((str[i] >= 97) && (str[i] <= 102)))
		{
			if (((str[i] >= '0') && (str[i] <= '9')))
				val = str[i] - 48;
			else if ((str[i] >= 65) && (str[i] <= 70))
				val = str[i] - 55;
			else if (((str[i] >= 97) && (str[i] <= 102)))
				val = str[i] - 87;
			if (i > 0)
				res = (res << 4);
			res |= val;
			i++;
			if (i > 1)
				break;
		}
		else
			break;
	}

	*end = &str[i];

	return res;
}

uint8_t str_is_empty(char *str)
{
	uint8_t result = 1;
	while (*str != 0)
	{
		if (*str > ' ')
		{
			result = 0;
			break;
		}
		str++;
	}
	return result;
}

uint8_t str_is_quotes(char *str, char **out_str)
{

	enum STATE
	{
		find_quote_begin,
		find_str,
		find_quote_end
	};

	uint8_t result = 0;
	enum STATE state = find_quote_begin;

	while (*str != 0)
	{
		switch (state)
		{
		case find_quote_begin:
			if ((*str == 34) || (*str == 39))
				state = find_str;
			break;

		case find_str:
			if (*str >= ' ')
			{
				*out_str = str;
				state = find_quote_end;
			}

			break;

		case find_quote_end:
			if ((*str == 34) || (*str == 39))
			{
				*str = 0;
				result = 1;
			}
			break;
		}

		if (result)
			break;

		str++;
	}

	return result;
}

uint8_t str_is_bits_enum(char *str, uint16_t *out_bits)
{
	uint8_t result = 0;
	int16_t bit = 0;

	*out_bits = 0;

	while (*str != 0)
	{
		if ((*str >= '0') && (*str <= '9'))
		{
			if (bit < 0)
				bit = 0;
			bit += (*str - '0');
			bit *= 10;
			result = 0;
		}
		else if (*str == ';')
		{
			bit /= 10;

			if (bit > 15)
				return 0;

			*out_bits |= (1 << bit);
			bit = 0;
			result = 1;
		}
		else if (*str != ' ')
		{
			result = 0;
			break;
		}
		str++;
	}

	return result;
}

uint8_t str_to_int32_t(char *str, int32_t *number, uint8_t *dp_pos)
{
	uint8_t result = 0;
	uint8_t is_negative = 0;
	int8_t dp = -1;
	int32_t tmp = 0;
	char *unit_str = NULL;

	if (str == NULL)
		return result;

	dp = -1;
	*dp_pos = 0;

	// TODO добавить проверку на 234ввв например - результат должен быть отрицательным
	while (*str != 0)
	{
		if ((*str >= '0') && (*str <= '9'))
		{
			tmp += (*str - '0');
			tmp *= 10;
			if (dp > -1)
				dp++;
			result = 1;
		}
		else
		{
			if (*str == '-')
			{
				if (is_negative)
				{
					result = 0;
					break;
				}
				is_negative = 1;
			}
			else if ((*str == '.') || (*str == ','))
			{
				if (dp > -1)
				{
					result = 0;
					break;
				}
				dp++;
			}
			else if (*str > ' ')
			{
				if (unit_str == NULL)
					unit_str = str;
			}
			else
			{
				*str = 0;
				break;
			}
		}

		str++;
	}

	if (result)
	{
		tmp /= 10;
		if (is_negative)
			tmp = -tmp;
		*number = tmp;
		if (dp > 0)
			*dp_pos = dp;
	}

	return result;
}

int random(int min, int max)
{
	return min + (rand() % (max + 1 - min));
}

uint8_t bcd_to_dec(uint8_t c)
{
	uint8_t ch = ((c >> 4) * 10 + (0x0F & c));
	return ch;
}

uint8_t dec_to_bcd(uint8_t c)
{
	uint8_t ch = ((c / 10) << 4) | (c % 10);
	return ch;
}
