#ifndef __ONENET_H
#define __ONENET_H

/* 全局变量声明 - BMM150数据 */
extern int16_t bmm_x;
extern int16_t bmm_y;
extern int16_t bmm_z;

_Bool OneNet_DevLink(void);
void OneNet_SendData(void);
void OneNET_Subscribe(void);
void OneNet_RevPro(unsigned char *cmd);

#endif