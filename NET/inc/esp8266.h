#ifndef _ESP8266_H_
#define _ESP8266_H_

#define REV_OK		0
#define REV_WAIT	1

/* WiFi配置 */
#define ESP8266_WIFI_INFO		"AT+CWJAP=\"Tenda_mqtt\",\"12345678\"\r\n"

/* EMQX服务器配置 */
#define ESP8266_EMQX_INFO		"AT+CIPSTART=\"TCP\",\"broker.emqx.io\",1883\r\n"

extern unsigned char esp8266_buf[512];
extern unsigned short esp8266_cnt;

/* 调试打印函数 - 在main.c中实现 */
extern void DebugPrint(const char *str);

void ESP8266_Init(void);
void ESP8266_Clear(void);
_Bool ESP8266_SendCmd(char *cmd, char *res);
void ESP8266_SendData(unsigned char *data, unsigned short len);
unsigned char *ESP8266_GetIPD(unsigned short timeOut);

#endif