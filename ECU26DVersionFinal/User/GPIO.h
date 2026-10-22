#ifndef __GPIO_H
#define __GPIO_H

#define Toggle(p,i)  do{ (p)->ODR ^= (i); }while(0)

/*
CAN收发指示灯
LED1:PB6 C3R
LED2:PB5 C3T
*/
#define LED1_Pin 	        GPIO_Pin_6
#define LED1_Port           GPIOB
#define LED1_GPIO_CLK       RCC_APB2Periph_GPIOB
#define LED2_Pin            GPIO_Pin_5
#define LED2_Port           GPIOB
#define LED2_GPIO_CLK       RCC_APB2Periph_GPIOB
#define CAN_RX_Toggle		Toggle(LED1_Port,LED1_Pin)
#define CAN_TX_Toggle       Toggle(LED2_Port,LED2_Pin)

/*
ASSI指示灯(高亮低灭)
ASSI-1-YELLOW：PC7
ASSI-2-BLUE：PC6
*/
#define ASSI_1_Pin          GPIO_Pin_7
#define ASSI_1_Port         GPIOC
#define ASSI_1_GPIO_CLK		RCC_APB2Periph_GPIOC
#define ASSI_2_Pin          GPIO_Pin_6
#define ASSI_2_Port         GPIOC
#define ASSI_2_GPIO_CLK		RCC_APB2Periph_GPIOC

#define NoneLight		    do{GPIO_ResetBits(ASSI_1_Port,ASSI_1_Pin);GPIO_ResetBits(ASSI_2_Port,ASSI_2_Pin);}while(0)

#define YellowLight	        do{GPIO_SetBits(ASSI_1_Port,ASSI_1_Pin);GPIO_ResetBits(ASSI_2_Port,ASSI_2_Pin);}while(0)

#define BlueLight			do{GPIO_SetBits(ASSI_2_Port,ASSI_2_Pin);GPIO_ResetBits(ASSI_1_Port,ASSI_1_Pin);}while(0)

#define Yellow_Toggle       do{Toggle(ASSI_1_Port,ASSI_1_Pin);GPIO_ResetBits(ASSI_2_Port,ASSI_2_Pin);}while(0)

#define Blue_Toggle		    do{Toggle(ASSI_2_Port,ASSI_2_Pin);GPIO_ResetBits(ASSI_1_Port,ASSI_1_Pin);}while(0)

#define BothLight           do{GPIO_SetBits(ASSI_1_Port,ASSI_1_Pin);GPIO_SetBits(ASSI_2_Port,ASSI_2_Pin);}while(0)

//蜂鸣器 BEEP:PC8 高响低灭
#define BEEP_Pin 	        GPIO_Pin_8
#define BEEP_Port	        GPIOC
#define BEEP_GPIO_CLK		RCC_APB2Periph_GPIOC
#define BEEP_OFF	        do{GPIO_ResetBits(BEEP_Port,BEEP_Pin);}while(0)
#define BEEP_ON 	        do{GPIO_SetBits(BEEP_Port,BEEP_Pin);}while(0)
#define BEEP_Toggle         Toggle(BEEP_Port,BEEP_Pin)

//使能灯 ENABLELED：PC9 高亮低灭
#define ENABLELED_Pin  		GPIO_Pin_9
#define ENABLELED_Port 		GPIOC
#define ENABLELED_GPIO_CLK	RCC_APB2Periph_GPIOC
#define ToDrive_LED_ON 		do{GPIO_SetBits(ENABLELED_Port,ENABLELED_Pin);}while(0)
#define ToDrive_LED_OFF 	do{GPIO_ResetBits(ENABLELED_Port,ENABLELED_Pin);}while(0)

//安全回路高压检测 SC_OUT:PB15 引脚低电平时有效
#define SC_OUT_Pin          GPIO_Pin_15
#define SC_OUT_Port         GPIOB
#define SC_OUT_GPIO_CLK		RCC_APB2Periph_GPIOB
#define SC_OUT			    (GPIO_ReadInputDataBit(SC_OUT_Port,SC_OUT_Pin)==0)

//ASMS:PB14 引脚低电平时有效
#define ASMS_Pin 	        GPIO_Pin_14
#define ASMS_Port           GPIOB
#define ASMS_GPIO_CLK		RCC_APB2Periph_GPIOB
#define ASMS		        (GPIO_ReadInputDataBit(ASMS_Port,ASMS_Pin)==0)

//使能按键 KEY_ENABLE：PA3 引脚低电平时有效
#define KEY_ENABLE_Pin	    GPIO_Pin_3
#define KEY_ENABLE_Port     GPIOA
#define KEY_ENABLE_GPIO_CLK	RCC_APB2Periph_GPIOA
#define KEY_ENABLE			(GPIO_ReadInputDataBit(KEY_ENABLE_Port,KEY_ENABLE_Pin)==0)

void GPIO_Config(void);

#endif
