#include "stm32f1xx_hal.h"
#include "esp8266.h"
#include "onenet.h"
#include "mqttkit.h"
#include "cJSON.h"
#include <string.h>
#include <stdio.h>

/* Device Config */
#define DEVICE_NAME		"bmm150_device"
#define MQTT_USER		"bmm150"
#define MQTT_PASSWORD	"bmm150123"

extern unsigned char esp8266_buf[512];

/* External Variables - defined in main.c */
extern int16_t bmm_x;
extern int16_t bmm_y;
extern int16_t bmm_z;

_Bool OneNet_DevLink(void)
{
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};
	unsigned char *dataPtr;
	_Bool status = 1;

	DebugPrint("MQTT Connect...\r\n");

	if(MQTT_PacketConnect(MQTT_USER, MQTT_PASSWORD, DEVICE_NAME, 256, 1, MQTT_QOS_LEVEL0, NULL, NULL, 0, &mqttPacket) == 0)
	{
		ESP8266_SendData(mqttPacket._data, mqttPacket._len);

		dataPtr = ESP8266_GetIPD(250);
		if(dataPtr != NULL)
		{
			if(MQTT_UnPacketRecv(dataPtr) == MQTT_PKT_CONNACK)
			{
				switch(MQTT_UnPacketConnectAck(dataPtr))
				{
					case 0: DebugPrint("   MQTT Connected OK\r\n"); status = 0; break;
					case 1: DebugPrint("   WARN: Protocol Error\r\n"); break;
					case 2: DebugPrint("   WARN: Invalid ClientID\r\n"); break;
					case 3: DebugPrint("   WARN: Server Failed\r\n"); break;
					case 4: DebugPrint("   WARN: User/Password Error\r\n"); break;
					case 5: DebugPrint("   WARN: Invalid Token\r\n"); break;
					default: DebugPrint("   ERR: Unknown Error\r\n"); break;
				}
			}
		}
		MQTT_DeleteBuffer(&mqttPacket);
	}
	else
		DebugPrint("   WARN: MQTT_PacketConnect Failed\r\n");

	return status;
}

/* Build JSON data using strcat */
unsigned char OneNet_FillBuf(char *buf)
{
	char text[100];
	int32_t mag_value;
	memset(text, 0, sizeof(text));
	strcpy(buf, "{");

	/* x axis */
	memset(text, 0, sizeof(text));
	sprintf(text, "\"x\":%d,", bmm_x);
	strcat(buf, text);

	/* y axis */
	memset(text, 0, sizeof(text));
	sprintf(text, "\"y\":%d,", bmm_y);
	strcat(buf, text);

	/* z axis */
	memset(text, 0, sizeof(text));
	sprintf(text, "\"z\":%d,", bmm_z);
	strcat(buf, text);

	/* mag = sqrt(x² + y² + z²) - integer calculation */
	mag_value = (int32_t)(bmm_x * bmm_x) + (int32_t)(bmm_y * bmm_y) + (int32_t)(bmm_z * bmm_z);
	/* Simple integer sqrt approximation */
	int32_t sqrt_val = 0;
	int32_t temp = mag_value;
	while(temp > 0)
	{
		temp -= (sqrt_val * 2 + 1);
		sqrt_val++;
	}
	/* Adjust for better approximation */
	if(sqrt_val > 0 && (sqrt_val * sqrt_val) > mag_value)
		sqrt_val--;

	memset(text, 0, sizeof(text));
	sprintf(text, "\"mag\":%ld", sqrt_val);
	strcat(buf, text);

	strcat(buf, "}");

	return strlen(buf);
}

/* Upload data to MQTT */
void OneNet_SendData(void)
{
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};
	char buf[256];
	short body_len = 0, i = 0;

	memset(buf, 0, sizeof(buf));

	body_len = OneNet_FillBuf(buf);

	DebugPrint("Upload: ");
	DebugPrint(buf);
	DebugPrint("\r\n");

	if(body_len)
	{
		if(MQTT_PacketSaveData(NULL, DEVICE_NAME, body_len, NULL, &mqttPacket) == 0)
		{
			for(; i < body_len; i++)
				mqttPacket._data[mqttPacket._len++] = buf[i];

			ESP8266_SendData(mqttPacket._data, mqttPacket._len);

			MQTT_DeleteBuffer(&mqttPacket);
		}
		else
			DebugPrint("   WARN: MQTT_PacketSaveData Failed\r\n");
	}
}

void OneNET_Subscribe(void)
{
	MQTT_PACKET_STRUCTURE mqtt_packet = {NULL, 0, 0, 0};
	char topic_buf[56];
	const char *topic = topic_buf;

	snprintf(topic_buf, sizeof(topic_buf), "sl_sub");
	DebugPrint("Subscribe Topic: sl_sub\r\n");

	if(MQTT_PacketSubscribe(MQTT_SUBSCRIBE_ID, MQTT_QOS_LEVEL0, &topic, 1, &mqtt_packet) == 0)
	{
		ESP8266_SendData(mqtt_packet._data, mqtt_packet._len);
		MQTT_DeleteBuffer(&mqtt_packet);
	}
}

void OneNet_RevPro(unsigned char *cmd)
{
	char *req_payload = NULL;
	char *cmdid_topic = NULL;
	unsigned short topic_len = 0;
	unsigned short req_len = 0;
	unsigned char qos = 0;
	static unsigned short pkt_id = 0;
	unsigned char type = 0;
	short result = 0;
	cJSON *raw_json;
	cJSON *led_item;
	char msg_buf[100];

	type = MQTT_UnPacketRecv(cmd);
	switch(type)
	{
		case MQTT_PKT_PUBLISH:
			result = MQTT_UnPacketPublish(cmd, &cmdid_topic, &topic_len, &req_payload, &req_len, &qos, &pkt_id);
			if(result == 0)
			{
				DebugPrint("Received: ");
				DebugPrint(req_payload);
				DebugPrint("\r\n");

				/* Parse JSON */
				raw_json = cJSON_Parse(req_payload);
				if(raw_json != NULL)
				{
					/* Check led field */
					led_item = cJSON_GetObjectItem(raw_json, "led");
					if(led_item != NULL)
					{
						int led_value = led_item->valueint;
						sprintf(msg_buf, "LED Control: %d\r\n", led_value);
						DebugPrint(msg_buf);

						/* led=1 ON, led=0 OFF */
						if(led_value == 1)
						{
							HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
							DebugPrint("LED ON\r\n");
						}
						else if(led_value == 0)
						{
							HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
							DebugPrint("LED OFF\r\n");
						}
					}
					else
					{
						DebugPrint("No 'led' field found\r\n");
					}

					cJSON_Delete(raw_json);
				}
				else
				{
					DebugPrint("JSON Parse Failed\r\n");
				}
			}
		break;

		case MQTT_PKT_PUBACK:
			DebugPrint("MQTT Publish OK\r\n");
		break;

		case MQTT_PKT_SUBACK:
			if(MQTT_UnPacketSubscribe(cmd) == 0)
				DebugPrint("MQTT Subscribe OK\r\n");
			else
				DebugPrint("MQTT Subscribe Failed\r\n");
		break;

		default:
			result = -1;
		break;
	}

	ESP8266_Clear();

	if(result == -1)
		return;

	if(type == MQTT_PKT_CMD || type == MQTT_PKT_PUBLISH)
	{
		MQTT_FreeBuffer(cmdid_topic);
		MQTT_FreeBuffer(req_payload);
	}
}