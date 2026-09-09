#ifndef CMD_FUNC_H_
#define CMD_FUNC_H_

#include <stdint.h>
#include "cmd.h"

enum CMD_RESULT cmd_help(struct CMD_ARGUMENT *args, uint8_t args_num);
enum CMD_RESULT cmd_reset(struct CMD_ARGUMENT *args, uint8_t args_num);
enum CMD_RESULT cmd_net_info(struct CMD_ARGUMENT *args, uint8_t args_num);
enum CMD_RESULT cmd_dali(struct CMD_ARGUMENT *args, uint8_t args_num);
enum CMD_RESULT cmd_dali2(struct CMD_ARGUMENT *args, uint8_t args_num);
enum CMD_RESULT cmd_set_short_addr(struct CMD_ARGUMENT *args, uint8_t args_num);
enum CMD_RESULT cmd_test_short_addr(struct CMD_ARGUMENT *args, uint8_t args_num);
enum CMD_RESULT cmd_replace_addr(struct CMD_ARGUMENT *args, uint8_t args_num);
enum CMD_RESULT cmd_get_rtc(struct CMD_ARGUMENT *args, uint8_t args_num);
enum CMD_RESULT cmd_set_rtc(struct CMD_ARGUMENT *args, uint8_t args_num);
enum CMD_RESULT cmd_temp(struct CMD_ARGUMENT *args, uint8_t args_num);
enum CMD_RESULT cmd_ver(struct CMD_ARGUMENT *args, uint8_t args_num);

#endif /* CMD_FUNC_H_ */
