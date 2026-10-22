#ifndef __USART_H
#define __USART_H
#include "stm32f10x.h"

// 遥控输出：只包含角度和速度
// 停车、信号丢失等状态在 SBUS_Process 内部处理，不会出现在这里
typedef struct
{
	int8_t steering_deg;	// 方向角：-7~+7 度（中位0）需要华海再乘三 到-21~+21
	uint16_t speed_m_s;		// 速度：0~3m/s，0.5m/s步进，接收端除以2
	uint8_t parked;			// 停车标志：1=停车模式
	uint8_t lost;			// 信号丢失标志：1=信号丢失/失控保护
	uint8_t driving;		// 正常行驶标志：1=正常遥控且未停车未丢信号
} CarControl_t;


void Serial_Init(void);

uint8_t Serial_GetSBUSFrame(uint8_t *Frame);

// 更新角度和速度，后续用CAN发送
void SBUS_Process(CarControl_t *ctrl);


#endif
