#include "GPIO.h"
#include "stm32f10x.h"

void LED_Config()
{
	GPIO_InitTypeDef GPIO_InitStruct;
	RCC_APB2PeriphClockCmd(LED1_GPIO_CLK,ENABLE);
	RCC_APB2PeriphClockCmd(LED2_GPIO_CLK,ENABLE);
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Pin=LED1_Pin|LED2_Pin;
	GPIO_Init(LED1_Port, &GPIO_InitStruct);
	GPIO_Init(LED2_Port, &GPIO_InitStruct);
	GPIO_ResetBits(LED1_Port,LED1_Pin);
	GPIO_ResetBits(LED2_Port,LED2_Pin);
}

void ASSI_Config()
{
	GPIO_InitTypeDef GPIO_InitStruct;
	RCC_APB2PeriphClockCmd(ASSI_1_GPIO_CLK,ENABLE);
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Pin=ASSI_1_Pin|ASSI_2_Pin;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(ASSI_1_Port,&GPIO_InitStruct);
	GPIO_Init(ASSI_2_Port,&GPIO_InitStruct);
	GPIO_ResetBits(ASSI_1_Port,ASSI_1_Pin);
	GPIO_ResetBits(ASSI_2_Port,ASSI_2_Pin);
}

void ENABLE_LED_Config()
{
	GPIO_InitTypeDef GPIO_InitStruct;
	RCC_APB2PeriphClockCmd(ENABLELED_GPIO_CLK,ENABLE);
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Pin=ENABLELED_Pin;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(ENABLELED_Port,&GPIO_InitStruct);
	GPIO_ResetBits(ENABLELED_Port,ENABLELED_Pin);
}

void BEEP_Config()
{
	GPIO_InitTypeDef GPIO_InitStruct;
	RCC_APB2PeriphClockCmd(BEEP_GPIO_CLK,ENABLE);
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Pin=BEEP_Pin;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(BEEP_Port,&GPIO_InitStruct);
	GPIO_ResetBits(BEEP_Port,BEEP_Pin);
}

void KEY_ENABLE_Config()
{
	GPIO_InitTypeDef GPIO_InitStruct;
	RCC_APB2PeriphClockCmd(KEY_ENABLE_GPIO_CLK,ENABLE);
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IN_FLOATING;
	GPIO_InitStruct.GPIO_Pin=KEY_ENABLE_Pin;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(KEY_ENABLE_Port,&GPIO_InitStruct);
}

void SC_OUT_Config()
{
	GPIO_InitTypeDef GPIO_InitStruct;
	RCC_APB2PeriphClockCmd(SC_OUT_GPIO_CLK,ENABLE);
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IN_FLOATING;
	GPIO_InitStruct.GPIO_Pin=SC_OUT_Pin;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(SC_OUT_Port,&GPIO_InitStruct);
}

void ASMS_Config()
{
	GPIO_InitTypeDef GPIO_InitStruct;
	RCC_APB2PeriphClockCmd(ASMS_GPIO_CLK,ENABLE);
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IN_FLOATING;
	GPIO_InitStruct.GPIO_Pin=ASMS_Pin;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(ASMS_Port,&GPIO_InitStruct);
}

void GPIO_Config()
{
	LED_Config();
	ASMS_Config();
	ASSI_Config();
	BEEP_Config();
	KEY_ENABLE_Config();
	ENABLE_LED_Config();
	SC_OUT_Config();
}
