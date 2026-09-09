#ifndef DALI_CMD_H_
#define DALI_CMD_H_

#include <stdint.h>

#include "dali.h"

enum DALI_CMD_FEATURE
{
	DCF_SINGLE,
	DCF_SET_ADDRESES,
	DCF_CHECK_ADDRESES,
	DCF_SEND_PACKET
};

struct DALI_CMD
{
	uint8_t bus_no;
	enum DALI_CMD_FEATURE feature;
	uint8_t arg1;
	uint8_t arg2;
};

enum DALI_FRAME_FORMAT
{
	DFF_16,
	DFF_24
};

// struct DALI_CMD_RESPONSE dali_cmd_exec(struct DALI_CMD *cmd);
void set_address(uint8_t bus_no, uint8_t start_short_addr, uint8_t show_progress, uint8_t onadr, enum DALI_FRAME_FORMAT fmt);
void set_address2(uint8_t bus_no, uint8_t start_short_addr, uint8_t show_progress, uint8_t onadr, enum DALI_FRAME_FORMAT fmt);
void test_short_address(uint8_t bus_no, uint8_t is_show);
void replace_short_addr(uint8_t bus_no, uint8_t old_addr, uint8_t new_addr);

#endif /* DALI_CMD_H_ */
