#include "esp8266.h"
#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdio.h>

extern unsigned char esp8266_buf[512];
extern unsigned short esp8266_cnt;
static unsigned short esp8266_cntPre = 0;

extern UART_HandleTypeDef huart2;

void ESP8266_Clear(void)
{
	memset((void*)esp8266_buf, 0, sizeof(esp8266_buf));
	esp8266_cnt = 0;
}

_Bool ESP8266_WaitRecive(void)
{
	if(esp8266_cnt == 0)
		return REV_WAIT;

	if(esp8266_cnt == esp8266_cntPre)
	{
		esp8266_cnt = 0;
		return REV_OK;
	}

	esp8266_cntPre = esp8266_cnt;
	return REV_WAIT;
}

_Bool ESP8266_SendCmd(char *cmd, char *res)
{
	unsigned char timeOut = 200;

	HAL_UART_Transmit(&huart2, (uint8_t*)cmd, strlen(cmd), 1000);

	while(timeOut--)
	{
		if(ESP8266_WaitRecive() == REV_OK)
		{
			if(strstr((const char *)esp8266_buf, res) != NULL)
			{
				ESP8266_Clear();
				return 0;
			}
		}
		HAL_Delay(10);
	}
	return 1;
}

void ESP8266_SendData(unsigned char *data, unsigned short len)
{
	char cmdBuf[32];

	ESP8266_Clear();
	sprintf(cmdBuf, "AT+CIPSEND=%d\r\n", len);
	if(!ESP8266_SendCmd(cmdBuf, ">"))
	{
		HAL_UART_Transmit(&huart2, data, len, 1000);
	}
}

unsigned char *ESP8266_GetIPD(unsigned short timeOut)
{
	char *ptrIPD = NULL;

	do
	{
		if(ESP8266_WaitRecive() == REV_OK)
		{
			ptrIPD = strstr((char *)esp8266_buf, "IPD,");
			if(ptrIPD == NULL)
			{

			}
			else
			{
				ptrIPD = strchr(ptrIPD, ':');
				if(ptrIPD != NULL)
				{
					ptrIPD++;
					return (unsigned char *)(ptrIPD);
				}
				else
					return NULL;
			}
		}
		HAL_Delay(5);
	} while(timeOut--);

	return NULL;
}

void ESP8266_Init(void)
{
	ESP8266_Clear();

	DebugPrint("1. AT Test\r\n");
	while(ESP8266_SendCmd("AT\r\n", "OK"))
		HAL_Delay(200);
	DebugPrint("   AT OK\r\n");

	DebugPrint("2. Set Station Mode\r\n");
	while(ESP8266_SendCmd("AT+CWMODE=1\r\n", "OK"))
		HAL_Delay(200);
	DebugPrint("   CWMODE OK\r\n");

	DebugPrint("3. Enable DHCP\r\n");
	while(ESP8266_SendCmd("AT+CWDHCP=1,1\r\n", "OK"))
		HAL_Delay(200);
	DebugPrint("   CWDHCP OK\r\n");

	DebugPrint("4. Connect WiFi: Tenda_mqtt\r\n");
	while(ESP8266_SendCmd(ESP8266_WIFI_INFO, "GOT IP"))
		HAL_Delay(200);
	DebugPrint("   WiFi Connected\r\n");

	DebugPrint("5. ESP8266 Init OK\r\n");
	HAL_Delay(200);

	DebugPrint("6. Connect MQTT Server\r\n");
	while(ESP8266_SendCmd(ESP8266_EMQX_INFO, "CONNECT"))
		HAL_Delay(200);
	DebugPrint("   MQTT Server Connected\r\n");
}