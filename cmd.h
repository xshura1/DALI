#ifndef CMD_H_
#define CMD_H_

#include <stdint.h>

#include "cmd_token.h"

#define CMD_MAX_ARGS 10

enum CMD_RESULT
{
	CRF_OK = 0,
	CRF_INVALID_ARG = 1,
	CRF_INVALID_VALUE = 2,
	CRF_COMMAND_NOT_FOUND = 3,
	CRF_REQUIRED_ARG_NOT_FOUND = 4,
	CRF_TOO_MANY_ARGUMENTS = 5,
	CRF_DALI_ERROR = 6
};

#define CMD_MESSAGES_SIZE 6

const static char MSG_CRF_INVALID_ARG[] = "invalid argument: %s";
const static char MSG_CRF_INVALID_VALUE[] = "invalid value for argument '%s': %s";
const static char MSG_CRF_COMMAND_NOT_FOUND[] = "command not found";
const static char MSG_CRF_REQUIRED_ARG_NOT_FOUND[] = "required argument not found";
const static char MSG_CRF_TOO_MANY_ARGUMENTS[] = "too many arguments";
const static char MSG_CRF_DALI_ERROR[] = "dali error";

const static char *const cmd_messages[CMD_MESSAGES_SIZE] =
	{
		MSG_CRF_INVALID_ARG,
		MSG_CRF_INVALID_VALUE,
		MSG_CRF_COMMAND_NOT_FOUND,
		MSG_CRF_REQUIRED_ARG_NOT_FOUND,
		MSG_CRF_TOO_MANY_ARGUMENTS,
		MSG_CRF_DALI_ERROR};

enum CMD_TYPE_VAL
{
	CTV_UNDEFINED,
	CTV_EMPTY,
	CTV_ENUM,
	CTV_VALUE_T,
	CTV_STR,
	CTV_BIT,
	CTV_ERROR_ARG,
	CTV_ERROR_VAL
};

// целочисленное знаковое значение
struct S32_VALUE
{
	int32_t value;
	uint8_t dp_pos;
};

struct CRON_VALUE
{
	uint32_t mins_0_31;
	unsigned int days : 31;
	unsigned int mins_32_59 : 28;
	unsigned int hours : 24;
	unsigned int months : 12;
	unsigned int weeks : 7; // 134
	unsigned int reserved_ : 5;
	unsigned int mins_ : 1;
	unsigned int hours_ : 1;
	unsigned int days_ : 1;
	unsigned int months_ : 1;
	unsigned int weeks_ : 1;
};

struct CMD_ARGUMENT
{
	union
	{
		struct S32_VALUE val_t;
		uint16_t val_bit;
		char *val_str;
		uint16_t val_enum;
	};
	char *parse_arg;
	char *parse_val;
	enum TOKEN_ARG arg;
	enum CMD_TYPE_VAL val_type : 4;
	// enum UNIT						unit	 : 4;
};

extern char *cmd_tx_buf;
void cmd_check(void);
void cmd_init_handlers(void *get_str_handler, void *set_str_handler, void *cmd_end_handler);
char *get_str(void);
void set_str(const char *str);

#endif /* CMD_H_ */
