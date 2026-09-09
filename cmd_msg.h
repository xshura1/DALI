#ifndef CMD_MSG_H_
#define CMD_MSG_H_

const static char MSG_CLRF[] = "\r\n";
const static char MSG_CRF_OK[] = "%s - OK";
const static char MSG_CRF_ERROR[] = "%s - Error: ";

const static char MSG_IP[] = "IP: %d.%d.%d.%d";
const static char MSG_PORT[] = "Port: %d";
const static char MSG_MAC[] = "MAC: %02X:%02X:%02X:%02X:%02X:%02X";

const static char MSG_RESPONSE[] = "Response: %d";
const static char MSG_DALI_RESPONSE[] = "Response: %02X";

const static char MSG_RTC_RESPONSE_F[] = "Response: %.2d.%.2d.%d %.2d:%.2d:%.2d";

const static char MSG_VERSION[] = "ver: %s date: %s";

#endif /* CMD_MSG_H_ */
