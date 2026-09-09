#include <stm32f10x.h>
#include <string.h>
#include <stdio.h>
#include "cmd.h"
#include "cmd_msg.h"

#include "dali_cmd.h"
#include "dali_config.h"
#include "config.h"
#include "tcp.h"
#include "rtc.h"

struct CMD_ARGUMENT *cmd_get_argument(enum TOKEN_ARG arg, struct CMD_ARGUMENT *args, uint8_t args_num)
{
	for (uint8_t i = 0; i < args_num; i++)
		if (args[i].arg == arg)
			return &args[i];

	return NULL;
}

enum CMD_RESULT cmd_help(struct CMD_ARGUMENT *args, uint8_t args_num)
{
	for (uint8_t i = 0; i < TOKENS_COMMAND_SIZE; i++)
	{
		sprintf(cmd_tx_buf, (char *)tokens_command[i]);
		set_str(cmd_tx_buf);
		// sprintf(cmd_tx_buf, MSG_CLRF);
		set_str(MSG_CLRF);
	}
	return CRF_OK;
}

enum CMD_RESULT cmd_reset(struct CMD_ARGUMENT *args, uint8_t args_num)
{
	// NVIC_SystemRiset();
	SCB->AIRCR = 0x05FA0004;
	return CRF_OK;
}

enum CMD_RESULT cmd_net_info(struct CMD_ARGUMENT *args, uint8_t args_num)
{
	uint8_t ip_addr[4] = IP_ADDR;
	uint8_t mac_addr[6] = MAC_ADDR;
	sprintf(cmd_tx_buf, MSG_IP, ip_addr[0], ip_addr[1], ip_addr[2], ip_addr[3]);
	set_str(cmd_tx_buf);
	set_str(MSG_CLRF);
	sprintf(cmd_tx_buf, MSG_PORT, LOCAL_PORT_TCP);
	set_str(cmd_tx_buf);
	set_str(MSG_CLRF);
	sprintf(cmd_tx_buf, MSG_MAC, mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
	set_str(cmd_tx_buf);
	set_str(MSG_CLRF);
	return CRF_OK;
}

enum CMD_RESULT cmd_dali(struct CMD_ARGUMENT *args, uint8_t args_num)
{
	struct CMD_ARGUMENT *arg_bus = cmd_get_argument(T_ARG_BUS, args, args_num);
	struct CMD_ARGUMENT *arg_adr = cmd_get_argument(T_ARG_ADR, args, args_num);
	struct CMD_ARGUMENT *arg_cmd = cmd_get_argument(T_ARG_CMD, args, args_num);

	if (arg_bus == NULL)
		return CRF_REQUIRED_ARG_NOT_FOUND;
	if (arg_bus->val_type != CTV_VALUE_T)
	{
		arg_bus->val_type = CTV_ERROR_VAL;
		return CRF_INVALID_VALUE;
	}

	if ((arg_bus->val_t.value >= BUS_COUNT) || (arg_bus->val_t.value < 0))
	{
		arg_bus->val_type = CTV_ERROR_VAL;
		return CRF_INVALID_VALUE;
	}

	if (arg_adr == NULL)
		return CRF_REQUIRED_ARG_NOT_FOUND;
	if (arg_adr->val_type != CTV_VALUE_T)
	{
		arg_adr->val_type = CTV_ERROR_VAL;
		return CRF_INVALID_VALUE;
	}

	/*if ((arg_adr->val_t.value > 63) || (arg_adr->val_t.value < 0)){
		arg_adr->val_type = CTV_ERROR_VAL;
		return CRF_INVALID_VALUE;
	}*/

	if (arg_cmd == NULL)
		return CRF_REQUIRED_ARG_NOT_FOUND;
	if (arg_cmd->val_type != CTV_VALUE_T)
	{
		arg_cmd->val_type = CTV_ERROR_VAL;
		return CRF_INVALID_VALUE;
	}

	struct DALI_CMD_RESPONSE dali_res = __dali_send_command(arg_bus->val_t.value, arg_adr->val_t.value, arg_cmd->val_t.value);
	if (dali_res.result == DCR_RESPONSE)
	{
		/*USART1_put_str((uint8_t *)"\r\n val = ");
		uint8_t buf[3];
		hex2str((uint8_t *)&buf, res.response);
		USART1_put_str((uint8_t *)&buf);*/
		sprintf(cmd_tx_buf, MSG_DALI_RESPONSE, dali_res.response);
		set_str(cmd_tx_buf);
		set_str(MSG_CLRF);
	}
	else
	{
		if (dali_res.result == DCR_ERROR)
		{
			return CRF_DALI_ERROR;
		}
	}

	return CRF_OK;
}

enum CMD_RESULT cmd_dali2(struct CMD_ARGUMENT *args, uint8_t args_num)
{
	struct CMD_ARGUMENT *arg_bus = cmd_get_argument(T_ARG_BUS, args, args_num);
	struct CMD_ARGUMENT *arg_adr = cmd_get_argument(T_ARG_ADR, args, args_num);
	struct CMD_ARGUMENT *arg_inst = cmd_get_argument(T_ARG_INST, args, args_num);
	struct CMD_ARGUMENT *arg_op = cmd_get_argument(T_ARG_OP, args, args_num);
	struct CMD_ARGUMENT *arg_twice = cmd_get_argument(T_ARG_TWICE, args, args_num);

	if (arg_bus == NULL)
		return CRF_REQUIRED_ARG_NOT_FOUND;
	if (arg_bus->val_type != CTV_VALUE_T)
	{
		arg_bus->val_type = CTV_ERROR_VAL;
		return CRF_INVALID_VALUE;
	}

	if ((arg_bus->val_t.value >= BUS_COUNT) || (arg_bus->val_t.value < 0))
	{
		arg_bus->val_type = CTV_ERROR_VAL;
		return CRF_INVALID_VALUE;
	}

	if (arg_adr == NULL)
		return CRF_REQUIRED_ARG_NOT_FOUND;
	if (arg_adr->val_type != CTV_VALUE_T)
	{
		arg_adr->val_type = CTV_ERROR_VAL;
		return CRF_INVALID_VALUE;
	}

	/*if ((arg_adr->val_t.value > 63) || (arg_adr->val_t.value < 0)){
		arg_adr->val_type = CTV_ERROR_VAL;
		return CRF_INVALID_VALUE;
	}*/

	if (arg_inst == NULL)
		return CRF_REQUIRED_ARG_NOT_FOUND;
	if (arg_inst->val_type != CTV_VALUE_T)
	{
		arg_inst->val_type = CTV_ERROR_VAL;
		return CRF_INVALID_VALUE;
	}

	if (arg_op == NULL)
		return CRF_REQUIRED_ARG_NOT_FOUND;
	if (arg_op->val_type != CTV_VALUE_T)
	{
		arg_op->val_type = CTV_ERROR_VAL;
		return CRF_INVALID_VALUE;
	}

	uint8_t is_twice = 0;
	if (arg_twice != NULL)
	{
		is_twice = arg_twice->val_t.value;
	}

	if (is_twice)
		is_twice = 2;

	do
	{
		struct DALI_CMD_RESPONSE dali_res = __dali2_send_command(arg_bus->val_t.value, arg_adr->val_t.value, arg_inst->val_t.value, arg_op->val_t.value);
		if (dali_res.result == DCR_RESPONSE)
		{
			/*USART1_put_str((uint8_t *)"\r\n val = ");
			uint8_t buf[3];
			hex2str((uint8_t *)&buf, res.response);
			USART1_put_str((uint8_t *)&buf);*/
			sprintf(cmd_tx_buf, MSG_DALI_RESPONSE, dali_res.response);
			set_str(cmd_tx_buf);
			set_str(MSG_CLRF);
		}
		else
		{
			if (dali_res.result == DCR_ERROR)
			{
				return CRF_DALI_ERROR;
			}
		}
		if (is_twice)
			is_twice--;
	} while (is_twice);
	return CRF_OK;
}

enum CMD_RESULT cmd_set_short_addr(struct CMD_ARGUMENT *args, uint8_t args_num)
{
	struct CMD_ARGUMENT *arg_bus = cmd_get_argument(T_ARG_BUS, args, args_num);
	struct CMD_ARGUMENT *arg_sadr = cmd_get_argument(T_ARG_SADR, args, args_num);
	struct CMD_ARGUMENT *arg_log = cmd_get_argument(T_ARG_LOG, args, args_num);
	struct CMD_ARGUMENT *arg_onadr = cmd_get_argument(T_ARG_ONADR, args, args_num);

	if (arg_bus == NULL)
		return CRF_REQUIRED_ARG_NOT_FOUND;
	if (arg_bus->val_type != CTV_VALUE_T)
	{
		arg_bus->val_type = CTV_ERROR_VAL;
		return CRF_INVALID_VALUE;
	}

	if ((arg_bus->val_t.value >= BUS_COUNT) || (arg_bus->val_t.value < 0))
	{
		arg_bus->val_type = CTV_ERROR_VAL;
		return CRF_INVALID_VALUE;
	}

	if (arg_sadr == NULL)
		return CRF_REQUIRED_ARG_NOT_FOUND;
	if (arg_sadr->val_type != CTV_VALUE_T)
	{
		arg_sadr->val_type = CTV_ERROR_VAL;
		return CRF_INVALID_VALUE;
	}

	uint8_t is_show = 0;
	if (arg_log != NULL)
	{
		if (arg_log->val_type != CTV_VALUE_T)
		{
			arg_log->val_type = CTV_ERROR_VAL;
			return CRF_INVALID_VALUE;
		}
		is_show = arg_log->val_t.value;
	}

	/*if ((arg_adr->val_t.value > 63) || (arg_adr->val_t.value < 0)){
		arg_adr->val_type = CTV_ERROR_VAL;
		return CRF_INVALID_VALUE;
	}*/

	uint8_t onadr = 0;
	if (arg_onadr != NULL)
	{
		if (arg_onadr->val_type != CTV_VALUE_T)
		{
			arg_onadr->val_type = CTV_ERROR_VAL;
			return CRF_INVALID_VALUE;
		}
		onadr = arg_onadr->val_t.value;
	}

	// set_address(arg_bus->val_t.value, arg_sadr->val_t.value, is_show, onadr, DFF_16);
	set_address2(arg_bus->val_t.value, arg_sadr->val_t.value, is_show, onadr, DFF_24);

	return CRF_OK;
}

enum CMD_RESULT cmd_test_short_addr(struct CMD_ARGUMENT *args, uint8_t args_num)
{
	struct CMD_ARGUMENT *arg_bus = cmd_get_argument(T_ARG_BUS, args, args_num);
	struct CMD_ARGUMENT *arg_log = cmd_get_argument(T_ARG_LOG, args, args_num);

	if (arg_bus == NULL)
		return CRF_REQUIRED_ARG_NOT_FOUND;
	if (arg_bus->val_type != CTV_VALUE_T)
	{
		arg_bus->val_type = CTV_ERROR_VAL;
		return CRF_INVALID_VALUE;
	}

	if ((arg_bus->val_t.value >= BUS_COUNT) || (arg_bus->val_t.value < 0))
	{
		arg_bus->val_type = CTV_ERROR_VAL;
		return CRF_INVALID_VALUE;
	}

	uint8_t is_show = 0;
	if (arg_log != NULL)
	{
		if (arg_log->val_type != CTV_VALUE_T)
		{
			arg_log->val_type = CTV_ERROR_VAL;
			return CRF_INVALID_VALUE;
		}
		is_show = arg_log->val_t.value;
	}

	test_short_address(arg_bus->val_t.value, is_show);

	return CRF_OK;
}

enum CMD_RESULT cmd_replace_addr(struct CMD_ARGUMENT *args, uint8_t args_num)
{
	struct CMD_ARGUMENT *arg_bus = cmd_get_argument(T_ARG_BUS, args, args_num);
	struct CMD_ARGUMENT *arg_oadr = cmd_get_argument(T_ARG_OADR, args, args_num);
	struct CMD_ARGUMENT *arg_nadr = cmd_get_argument(T_ARG_NADR, args, args_num);

	if (arg_bus == NULL)
		return CRF_REQUIRED_ARG_NOT_FOUND;
	if (arg_bus->val_type != CTV_VALUE_T)
	{
		arg_bus->val_type = CTV_ERROR_VAL;
		return CRF_INVALID_VALUE;
	}

	if ((arg_bus->val_t.value >= BUS_COUNT) || (arg_bus->val_t.value < 0))
	{
		arg_bus->val_type = CTV_ERROR_VAL;
		return CRF_INVALID_VALUE;
	}

	if (arg_oadr == NULL)
		return CRF_REQUIRED_ARG_NOT_FOUND;
	if (arg_oadr->val_type != CTV_VALUE_T)
	{
		arg_oadr->val_type = CTV_ERROR_VAL;
		return CRF_INVALID_VALUE;
	}

	if ((arg_oadr->val_t.value > 63) || (arg_oadr->val_t.value < 0))
	{
		arg_oadr->val_type = CTV_ERROR_VAL;
		return CRF_INVALID_VALUE;
	}

	if (arg_nadr == NULL)
		return CRF_REQUIRED_ARG_NOT_FOUND;
	if (arg_nadr->val_type != CTV_VALUE_T)
	{
		arg_nadr->val_type = CTV_ERROR_VAL;
		return CRF_INVALID_VALUE;
	}

	if ((arg_nadr->val_t.value > 63) || (arg_nadr->val_t.value < 0))
	{
		arg_nadr->val_type = CTV_ERROR_VAL;
		return CRF_INVALID_VALUE;
	}

	replace_short_addr(arg_bus->val_t.value, arg_oadr->val_t.value, arg_nadr->val_t.value);
	return CRF_OK;
}

enum CMD_RESULT cmd_get_rtc(struct CMD_ARGUMENT *args, uint8_t args_num)
{
	sprintf(cmd_tx_buf, MSG_RTC_RESPONSE_F, RTC_get_day(), RTC_get_month(), RTC_get_year(), RTC_get_hours(), RTC_get_minutes(), RTC_get_seconds());
	set_str(cmd_tx_buf);
	set_str(MSG_CLRF);
	return CRF_OK;
}

enum CMD_RESULT cmd_set_rtc(struct CMD_ARGUMENT *args, uint8_t args_num)
{

	struct CMD_ARGUMENT *arg_s = cmd_get_argument(T_ARG_S, args, args_num);
	struct CMD_ARGUMENT *arg_m = cmd_get_argument(T_ARG_M, args, args_num);
	struct CMD_ARGUMENT *arg_h = cmd_get_argument(T_ARG_H, args, args_num);
	struct CMD_ARGUMENT *arg_w = cmd_get_argument(T_ARG_W, args, args_num);
	struct CMD_ARGUMENT *arg_dd = cmd_get_argument(T_ARG_DD, args, args_num);
	struct CMD_ARGUMENT *arg_mm = cmd_get_argument(T_ARG_MM, args, args_num);
	struct CMD_ARGUMENT *arg_yy = cmd_get_argument(T_ARG_YY, args, args_num);

	if (arg_s != NULL)
	{
		if (arg_s->val_type != CTV_VALUE_T)
		{
			arg_s->val_type = CTV_ERROR_VAL;
			return CRF_INVALID_VALUE;

			if (arg_s->val_t.value > 60)
			{
				arg_s->val_type = CTV_ERROR_VAL;
				return CRF_INVALID_VALUE;
			}
		}
	}

	if (arg_m != NULL)
	{
		if (arg_m->val_type != CTV_VALUE_T)
		{
			arg_m->val_type = CTV_ERROR_VAL;
			return CRF_INVALID_VALUE;

			if (arg_m->val_t.value > 60)
			{
				arg_m->val_type = CTV_ERROR_VAL;
				return CRF_INVALID_VALUE;
			}
		}
	}

	if (arg_h != NULL)
	{
		if (arg_h->val_type != CTV_VALUE_T)
		{
			arg_h->val_type = CTV_ERROR_VAL;
			return CRF_INVALID_VALUE;

			if (arg_h->val_t.value > 23)
			{
				arg_h->val_type = CTV_ERROR_VAL;
				return CRF_INVALID_VALUE;
			}
		}
	}

	if (arg_w != NULL)
	{
		if (arg_w->val_type != CTV_VALUE_T)
		{
			arg_w->val_type = CTV_ERROR_VAL;
			return CRF_INVALID_VALUE;

			if (arg_w->val_t.value > 8)
			{
				arg_w->val_type = CTV_ERROR_VAL;
				return CRF_INVALID_VALUE;
			}
		}
	}

	if (arg_dd != NULL)
	{
		if (arg_dd->val_type != CTV_VALUE_T)
		{
			arg_dd->val_type = CTV_ERROR_VAL;
			return CRF_INVALID_VALUE;

			if (arg_dd->val_t.value > 32)
			{
				arg_dd->val_type = CTV_ERROR_VAL;
				return CRF_INVALID_VALUE;
			}
		}
	}

	if (arg_mm != NULL)
	{
		if (arg_mm->val_type != CTV_VALUE_T)
		{
			arg_mm->val_type = CTV_ERROR_VAL;
			return CRF_INVALID_VALUE;

			if (arg_mm->val_t.value > 13)
			{
				arg_mm->val_type = CTV_ERROR_VAL;
				return CRF_INVALID_VALUE;
			}
		}
	}

	if (arg_yy != NULL)
	{
		if (arg_yy->val_type != CTV_VALUE_T)
		{
			arg_yy->val_type = CTV_ERROR_VAL;
			return CRF_INVALID_VALUE;

			if (arg_yy->val_t.value < 2000)
			{
				arg_yy->val_type = CTV_ERROR_VAL;
				return CRF_INVALID_VALUE;
			}
		}
	}

	if (arg_s != NULL)
	{
		RTC_set_seconds(arg_s->val_t.value);
	}
	if (arg_m != NULL)
	{
		RTC_set_minutes(arg_m->val_t.value);
	}
	if (arg_h != NULL)
	{
		RTC_set_hours(arg_h->val_t.value);
	}
	if (arg_w != NULL)
	{
		RTC_set_weekday(arg_w->val_t.value);
	}
	if (arg_dd != NULL)
	{
		RTC_set_day(arg_dd->val_t.value);
	}
	if (arg_mm != NULL)
	{
		RTC_set_month(arg_mm->val_t.value);
	}
	if (arg_yy != NULL)
	{
		RTC_set_year(arg_yy->val_t.value);
	}

	return CRF_OK;
}

enum CMD_RESULT cmd_temp(struct CMD_ARGUMENT *args, uint8_t args_num)
{
	sprintf(cmd_tx_buf, MSG_RESPONSE, RTC_get_temp());
	set_str(cmd_tx_buf);
	set_str(MSG_CLRF);
	return CRF_OK;
}

enum CMD_RESULT cmd_ver(struct CMD_ARGUMENT *args, uint8_t args_num)
{
	sprintf(cmd_tx_buf, MSG_VERSION, VERSION, VERSION_DATE);
	set_str(cmd_tx_buf);
	set_str(MSG_CLRF);
	return CRF_OK;
}
