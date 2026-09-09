#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "utils.h"
#include "cmd.h"
#include "cmd_func.h"
#include "cmd_token.h"
#include "cmd_msg.h"

// #include "tcp.h"

struct CMD_ARGUMENT arg_buf[CMD_MAX_ARGS];
char _cmd_tx_buf[256];
char *cmd_tx_buf = _cmd_tx_buf;

char *(*_get_str_handler)(void) = NULL;
void (*_set_str_handler)(const char *str) = NULL;
void (*_cmd_end_handler)(void) = NULL;

static const void *const sys_func[] = {
	cmd_help,
	cmd_reset,
	cmd_net_info,
	cmd_dali,
	cmd_dali2,
	cmd_set_short_addr,
	cmd_test_short_addr,
	cmd_replace_addr,
	cmd_get_rtc,
	cmd_set_rtc,
	cmd_temp,
	cmd_ver};

void cmd_init_handlers(void *get_str_handler, void *set_str_handler, void *cmd_end_handler)
{
	_get_str_handler = get_str_handler;
	_set_str_handler = set_str_handler;
	_cmd_end_handler = cmd_end_handler;
}

char *get_str(void)
{
	if (_get_str_handler == NULL)
		return NULL;
	return _get_str_handler();
}

void set_str(const char *str)
{
	if (_set_str_handler != NULL)
	{
		_set_str_handler(str);
	}
}

static void cmd_end(void)
{
	if (_cmd_end_handler != NULL)
	{
		_cmd_end_handler();
	}
}

void cmd_send_result(enum CMD_RESULT result, char *command, struct CMD_ARGUMENT *args, uint8_t args_num);
enum CMD_RESULT cmd_parse_str(char *str, uint8_t is_parse_arg, char **command, int8_t *index_command, struct CMD_ARGUMENT *args, uint8_t *args_num);
void cmd_parse_arg(struct CMD_ARGUMENT *arg);
void cmd_parse_val(struct CMD_ARGUMENT *arg);
enum CMD_RESULT cmd_exec_func(int8_t func_no, struct CMD_ARGUMENT *args, uint8_t args_num);

void cmd_check(void)
{
	// FILE *				file			= uart_s();
	char *command = NULL;
	int8_t command_index = -1;
	struct CMD_ARGUMENT *args = (struct CMD_ARGUMENT *)&arg_buf;
	uint8_t args_num = 0;
	enum CMD_RESULT result = CRF_OK;

	char *str = get_str(); // get_tcp_data();
	if (str == NULL)
		return;

	result = cmd_parse_str(str,
						   1,
						   &command,
						   &command_index,
						   args,
						   &args_num);

	if (result != CRF_OK)
		cmd_send_result(result, command, args, args_num);
	else
	{
		result = cmd_exec_func(command_index, args, args_num);
		// if (result != CRF_EMPTY)
		cmd_send_result(result, command, args, args_num);
	}
	cmd_end();
}

enum CMD_RESULT cmd_exec_func(int8_t func_no, struct CMD_ARGUMENT *args, uint8_t args_num)
{
	if (func_no < 0)
		return CRF_COMMAND_NOT_FOUND;
	enum CMD_RESULT result = CRF_OK;
	enum CMD_RESULT (*handler)(struct CMD_ARGUMENT *args, uint8_t args_num) = NULL;
	handler = (void *)sys_func[func_no];
	if (handler != NULL)
		result = handler(args, args_num);
	else
		result = CRF_COMMAND_NOT_FOUND;
	return result;
}

void cmd_send_result(enum CMD_RESULT result, char *command, struct CMD_ARGUMENT *args, uint8_t args_num)
{
	// FILE * file = uart_s();
	uint8_t is_clrf = 0;

	if (result == CRF_OK)
	{
		sprintf(cmd_tx_buf, MSG_CRF_OK, command);
		// send_tcp_data(cmd_tx_buf);
		set_str(cmd_tx_buf);
	}
	else if ((result == CRF_INVALID_ARG) || (result == CRF_INVALID_VALUE))
	{
		for (uint8_t i = 0; i < args_num; i++)
		{
			is_clrf = 0;
			if (args[i].val_type == CTV_ERROR_ARG)
			{
				sprintf(cmd_tx_buf, MSG_CRF_ERROR, command);
				// send_tcp_data(cmd_tx_buf);
				set_str(cmd_tx_buf);
				// fprintf_P(file, MSG_CRF_ERROR, command);
				sprintf(cmd_tx_buf, MSG_CRF_INVALID_ARG, args[i].parse_arg);
				// send_tcp_data(cmd_tx_buf);
				set_str(cmd_tx_buf);
				// fprintf_P(file, MSG_CRF_INVALID_ARG, args[i].parse_arg);
				is_clrf = 1;
			}
			else if (args[i].val_type == CTV_ERROR_VAL)
			{
				sprintf(cmd_tx_buf, MSG_CRF_ERROR, command);
				// send_tcp_data(cmd_tx_buf);
				set_str(cmd_tx_buf);
				// fprintf_P(file, MSG_CRF_ERROR, command);
				sprintf(cmd_tx_buf, MSG_CRF_INVALID_VALUE, args[i].parse_arg, args[i].parse_val);
				// send_tcp_data(cmd_tx_buf);
				set_str(cmd_tx_buf);
				// fprintf_P(file, MSG_CRF_INVALID_VALUE, args[i].parse_arg, args[i].parse_val);
				is_clrf = 1;
			}

			if (is_clrf)
			{
				sprintf(cmd_tx_buf, MSG_CLRF);
				// send_tcp_data(cmd_tx_buf);
				set_str(cmd_tx_buf);
				// fprintf_P(file, MSG_CLRF);
			}
		}
	}
	else
	{
		sprintf(cmd_tx_buf, MSG_CRF_ERROR, command);
		// send_tcp_data(cmd_tx_buf);
		set_str(cmd_tx_buf);
		sprintf(cmd_tx_buf, cmd_messages[result - 1]);
		set_str(cmd_tx_buf);
		// fprintf_P(file, MSG_CRF_ERROR, command);
		// fprintf_P(file, (char*)pgm_read_word(&(cmd_messages[result - 1])));
	}

	if (!is_clrf)
	{
		sprintf(cmd_tx_buf, MSG_CLRF);
		set_str(cmd_tx_buf);
		// send_tcp_data(cmd_tx_buf);
		// fprintf_P(file, MSG_CLRF);
	}
}

enum CMD_RESULT cmd_parse_str(char *str, uint8_t is_parse_arg, char **command, int8_t *index_command, struct CMD_ARGUMENT *args, uint8_t *args_num)
{
	enum STATE
	{
		find_command,
		parse_command,
		find_slash,
		find_arg,
		parse_arg,
		find_val,
		parse_val
	};

	enum CMD_RESULT result = CRF_COMMAND_NOT_FOUND;
	enum STATE state = find_command;
	uint8_t is_quotes = 0;

	*command = NULL;
	*index_command = -1;
	*args_num = 0;

	if (is_parse_arg)
		for (uint8_t i = 0; i < CMD_MAX_ARGS; i++)
		{
			args[i].parse_arg = NULL;
			args[i].parse_val = NULL;
			args[i].val_t.value = 0;
			args[i].val_t.dp_pos = 0;
		}

	while (*str != 0)
	{
		if ((state != find_val) && (state != parse_val)) // значения аргументов к нижнему регистру не приводим
			*str = tolower(*str);
		if (*str < 32)
			*str = ' ';

		switch (state)
		{
		case find_command:
			if (*str != ' ')
			{
				*command = str;
				state = parse_command;
			}
			break;

		case parse_command:
			if (*str == ' ')
			{
				*str = 0;
				if (!is_parse_arg)
					goto BREAK_LOOP;
				state = find_slash;
			}
			break;

		case find_slash:
			if (*str == '/')
				state = find_arg;

			break;

		case find_arg:
			if (*str != ' ')
			{
				if (*args_num == CMD_MAX_ARGS)
					return CRF_TOO_MANY_ARGUMENTS;
				(*args_num)++;

				args[(*args_num) - 1].parse_arg = str;
				state = parse_arg;
			}

			break;

		case parse_arg:
			if (*str == ' ')
			{
				*str = 0;
				state = find_val;
				is_quotes = 0;
			}
			break;

		case find_val:
			if (*str != ' ')
			{
				args[(*args_num) - 1].parse_val = str;
				state = parse_val;
				is_quotes = ((*str == 34) || (*str == 39));
			}

			break;

		case parse_val:
			if (is_quotes)
			{
				if ((*str == 34) || (*str == 39))
					is_quotes = 0;
			}
			else if (*str == ' ')
			{
				*str = 0;
				state = find_slash;
			}
			break;
		}

		str++;
	}

BREAK_LOOP:

	if (*command != NULL)
		result = CRF_OK;

	if (result == CRF_OK)
	{

		for (uint8_t i = 0; i < TOKENS_COMMAND_SIZE; i++)
			if (strcmp(*command, tokens_command[i]) == 0)
			{
				*index_command = i;
				break;
			}

		if (*index_command < 0)
			result = CRF_COMMAND_NOT_FOUND;

		if (result == CRF_OK)
			for (uint8_t i = 0; i < (*args_num); i++)
			{
				cmd_parse_arg(&args[i]);
				if (args[i].arg != T_ARG_NULL)
				{
					cmd_parse_val(&args[i]);

					if (args[i].val_type == CTV_ERROR_VAL)
						result = CRF_INVALID_VALUE;
				}
				else
				{
					args[i].val_type = CTV_ERROR_ARG;
					result = CRF_INVALID_ARG;
				}
			}
	}

	return result;
}

void cmd_parse_arg(struct CMD_ARGUMENT *arg)
{
	arg->arg = T_ARG_NULL;
	for (uint8_t i = 0; i < TOKENS_ARG_SIZE; i++)
		if (strcmp(arg->parse_arg, args_map[i].str) == 0)
		{
			arg->arg = args_map[i].arg_e;
			break;
		}
}

void cmd_parse_val(struct CMD_ARGUMENT *arg)
{
	arg->val_type = CTV_UNDEFINED;
	uint8_t is_expected_enum = 0;

	for (uint8_t i = 0; i < TOKENS_VAL_SIZE; i++)
		if (arg->arg == vals_map[i].arg_e)
		{
			is_expected_enum = 1;
			if (strcmp(arg->parse_val, vals_map[i].str) == 0)
			{
				arg->val_type = CTV_ENUM;
				arg->val_enum = vals_map[i].val;
				break;
			}
		}

	if (is_expected_enum && (arg->val_type != CTV_ENUM))
		arg->val_type = CTV_ERROR_VAL;

	if (arg->val_type == CTV_UNDEFINED)
	{
		if (str_is_empty(arg->parse_val))
			arg->val_type = CTV_EMPTY;
		else if (str_is_quotes(arg->parse_val, &arg->val_str))
			arg->val_type = CTV_STR;
		else if (str_is_bits_enum(arg->parse_val, &arg->val_bit))
			arg->val_type = CTV_BIT;
		else if (str_to_int32_t(arg->parse_val, &arg->val_t.value, &arg->val_t.dp_pos))
		{
			arg->val_type = CTV_VALUE_T;
		}
		else
			arg->val_type = CTV_ERROR_VAL;
	}
}
