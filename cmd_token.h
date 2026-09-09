#ifndef CMD_TOKEN_H_
#define CMD_TOKEN_H_

#include <stdint.h>

// команды---------------------------------------------------------------------------------------------------
#define TOKENS_COMMAND_SIZE 12

const static char TOKEN_COMMAND_HELP[] = "help";
const static char TOKEN_COMMAND_RESET[] = "reset";
const static char TOKEN_COMMAND_NET_INFO[] = "netinfo";
const static char TOKEN_COMMAND_DALI[] = "dali";
const static char TOKEN_COMMAND_DALI2[] = "dali2";
const static char TOKEN_COMMAND_DALI_SET_SHORT_ADR[] = "dsadr";
const static char TOKEN_COMMAND_DALI_TEST_SHORT_ADR[] = "tsadr";
const static char TOKEN_COMMAND_DALI_REPLACE_ADR[] = "repladr";
const static char TOKEN_COMMAND_GET_RTC[] = "grtc";
const static char TOKEN_COMMAND_SET_RTC[] = "srtc";
const static char TOKEN_COMMAND_TEMP[] = "temp";
const static char TOKEN_COMMAND_VER[] = "ver";

const static char *const tokens_command[TOKENS_COMMAND_SIZE] =
	{
		TOKEN_COMMAND_HELP,				   //	cmd_help
		TOKEN_COMMAND_RESET,			   //	cmd_reset
		TOKEN_COMMAND_NET_INFO,			   //	cmd_net_info
		TOKEN_COMMAND_DALI,				   //	cmd_dali
		TOKEN_COMMAND_DALI2,			   //	cmd_dali2
		TOKEN_COMMAND_DALI_SET_SHORT_ADR,  //	cmd_set_short_addr
		TOKEN_COMMAND_DALI_TEST_SHORT_ADR, //	cmd_test_short_addr
		TOKEN_COMMAND_DALI_REPLACE_ADR,	   //	cmd_replace_addr
		TOKEN_COMMAND_GET_RTC,			   //	cmd_get_trc
		TOKEN_COMMAND_SET_RTC,			   //	cmd_set_trc
		TOKEN_COMMAND_TEMP,				   //	cmd_temp
		TOKEN_COMMAND_VER				   //	cmd_ver

};
//-----------------------------------------------------------------------------------------------------------

// аргументы-------------------------------------------------------------------------------------------------

const static char TOKEN_ARG_ADR[] = "adr";
const static char TOKEN_ARG_CMD[] = "cmd";
const static char TOKEN_ARG_OP[] = "op";
const static char TOKEN_ARG_INST[] = "inst";
const static char TOKEN_ARG_BUS[] = "bus";
const static char TOKEN_ARG_SADR[] = "sadr";
const static char TOKEN_ARG_ONADR[] = "onadr";
const static char TOKEN_ARG_LOG[] = "log";
const static char TOKEN_ARG_OADR[] = "oadr";
const static char TOKEN_ARG_NADR[] = "nadr";
const static char TOKEN_ARG_S[] = "s";
const static char TOKEN_ARG_M[] = "m";
const static char TOKEN_ARG_H[] = "h";
const static char TOKEN_ARG_W[] = "w";
const static char TOKEN_ARG_DD[] = "dd";
const static char TOKEN_ARG_MM[] = "mm";
const static char TOKEN_ARG_YY[] = "yy";
const static char TOKEN_ARG_TWICE[] = "twice";

#define TOKENS_ARG_SIZE 18

enum TOKEN_ARG
{
	T_ARG_NULL = 0,
	T_ARG_ADR = 1,
	T_ARG_CMD = 2,
	T_ARG_OP = 3,
	T_ARG_INST = 4,
	T_ARG_BUS = 5,
	T_ARG_SADR = 6,
	T_ARG_LOG = 7,
	T_ARG_OADR = 8,
	T_ARG_ONADR = 9,
	T_ARG_NADR = 10,
	T_ARG_S = 11,
	T_ARG_M = 12,
	T_ARG_H = 13,
	T_ARG_W = 14,
	T_ARG_DD = 15,
	T_ARG_MM = 16,
	T_ARG_YY = 17,
	T_ARG_TWICE = 18
};

const static struct
{
	const char *str;
	enum TOKEN_ARG arg_e;
} args_map[TOKENS_ARG_SIZE] =
	{
		{TOKEN_ARG_ADR, T_ARG_ADR},
		{TOKEN_ARG_CMD, T_ARG_CMD},
		{TOKEN_ARG_OP, T_ARG_OP},
		{TOKEN_ARG_INST, T_ARG_INST},
		{TOKEN_ARG_BUS, T_ARG_BUS},
		{TOKEN_ARG_SADR, T_ARG_SADR},
		{TOKEN_ARG_LOG, T_ARG_LOG},
		{TOKEN_ARG_OADR, T_ARG_OADR},
		{TOKEN_ARG_ONADR, T_ARG_ONADR},
		{TOKEN_ARG_NADR, T_ARG_NADR},
		{TOKEN_ARG_S, T_ARG_S},
		{TOKEN_ARG_M, T_ARG_M},
		{TOKEN_ARG_H, T_ARG_H},
		{TOKEN_ARG_W, T_ARG_W},
		{TOKEN_ARG_DD, T_ARG_DD},
		{TOKEN_ARG_MM, T_ARG_MM},
		{TOKEN_ARG_YY, T_ARG_YY},
		{TOKEN_ARG_TWICE, T_ARG_TWICE}

};

//-----------------------------------------------------------------------------------------------------------

// значения--------------------------------------------------------------------------------------------------
#define TOKENS_VAL_SIZE 0
/*
const static char TOKEN_VAL_01[]				 = "01";		// прерывания
const static char TOKEN_VAL_10[]				 = "10";
const static char TOKEN_VAL_0_1[]				 = "0|1";

const static char TOKEN_VAL_24BIT[]				 = "24bit";	// протоколы
const static char TOKEN_VAL_21BIT[]				 = "21bit";
const static char TOKEN_VAL_BCD7[]				 = "bcd7";
const static char TOKEN_VAL_BIN6[]				 = "bin6";
const static char TOKEN_VAL_QUAD[]				 = "quad";
const static char TOKEN_VAL_PULSE[]				 = "pulse";

const static char TOKEN_VAL_X[]					 = "x";		// назначение устройства
const static char TOKEN_VAL_Y[]					 = "y";
const static char TOKEN_VAL_Z[]					 = "z";
const static char TOKEN_VAL_ANGLE[]				 = "angle";
const static char TOKEN_VAL_TACHO[]				 = "tacho";
const static char TOKEN_VAL_EDGE[]				 = "edge";
const static char TOKEN_VAL_OTHER[]				 = "other";



const static char TOKEN_VAL_UNDEFINED[]			 = "undef";	// тип канала
const static char TOKEN_VAL_CLK_IN[]			 = "clkin";
const static char TOKEN_VAL_CLK_OUT[]			 = "clkout";
const static char TOKEN_VAL_DATA_IN[]			 = "datain";
const static char TOKEN_VAL_A_IN[]				 = "ain";
const static char TOKEN_VAL_B_IN[]				 = "bin";
const static char TOKEN_VAL_Z_IN[]				 = "zin";
const static char TOKEN_VAL_ZERO_OUT[]			 = "zeroout";
const static char TOKEN_VAL_CHANGE_UNIT_OUT[]	 = "unitout";
const static char TOKEN_VAL_FASTMODE_OUT[]		 = "fastout";
*/

const static struct
{
	const char *str;
	uint16_t val;
	enum TOKEN_ARG arg_e;
} vals_map[TOKENS_VAL_SIZE] =
	{
		/*
		// прерывания
		{TOKEN_VAL_01,				INT_LEVEL_01,			T_ARG_INTERRUPT_LEVEL},
		{TOKEN_VAL_10,				INT_LEVEL_10,			T_ARG_INTERRUPT_LEVEL},
		{TOKEN_VAL_0_1,				INT_LEVEL_ALL,			T_ARG_INTERRUPT_LEVEL},
		// протоколы
		{TOKEN_VAL_24BIT,			BIT24_PROTOCOL,			T_ARG_PROTOCOL},
		{TOKEN_VAL_21BIT,			BIT21_PROTOCOL,			T_ARG_PROTOCOL},
		{TOKEN_VAL_BCD7,			BCD7_PROTOCOL,			T_ARG_PROTOCOL},
		{TOKEN_VAL_BIN6,			BIN6_PROTOCOL,			T_ARG_PROTOCOL},
		{TOKEN_VAL_QUAD,			QUAD_PROTOCOL,			T_ARG_PROTOCOL},
		{TOKEN_VAL_PULSE,			PULSE_PROTOCOL,			T_ARG_PROTOCOL},
		// назначение
		{TOKEN_VAL_X,				DF_AXIS_X,				T_ARG_FUNC},
		{TOKEN_VAL_Y,				DF_AXIS_Y,				T_ARG_FUNC},
		{TOKEN_VAL_Z,				DF_AXIS_Z,				T_ARG_FUNC},
		{TOKEN_VAL_ANGLE,			DF_ANGLE_SENSOR,		T_ARG_FUNC},
		{TOKEN_VAL_TACHO,			DF_TACHOMETER,			T_ARG_FUNC},
		{TOKEN_VAL_EDGE,			DF_EDGE_FINDER,			T_ARG_FUNC},
		{TOKEN_VAL_OTHER,			DF_OTHER,				T_ARG_FUNC},
		// единицы измерения
		{TOKEN_VAL_INCH,			U_INCH,					T_ARG_UNIT},
		{TOKEN_VAL_MM,				U_MM,					T_ARG_UNIT},
		// тип канала
		{TOKEN_VAL_UNDEFINED,		CHT_UNDEFINED,			T_ARG_TYPE},
		{TOKEN_VAL_CLK_IN,			CHT_CLK_IN,				T_ARG_TYPE},
		{TOKEN_VAL_CLK_OUT,			CHT_CLK_OUT,			T_ARG_TYPE},
		{TOKEN_VAL_DATA_IN,			CHT_DATA_IN,			T_ARG_TYPE},
		{TOKEN_VAL_A_IN,			CHT_A_IN,				T_ARG_TYPE},
		{TOKEN_VAL_B_IN,			CHT_B_IN,				T_ARG_TYPE},
		{TOKEN_VAL_Z_IN,			CHT_Z_IN,				T_ARG_TYPE},
		{TOKEN_VAL_ZERO_OUT,		CHT_SET_ZERO_OUT,		T_ARG_TYPE},
		{TOKEN_VAL_CHANGE_UNIT_OUT,	CHT_CHANGE_UNIT_OUT,	T_ARG_TYPE},
		{TOKEN_VAL_FASTMODE_OUT,	CHT_SET_FAST_MODE_OUT,	T_ARG_TYPE}*/

};

//-----------------------------------------------------------------------------------------------------------

#endif /* CMD_TOKEN_H_ */
