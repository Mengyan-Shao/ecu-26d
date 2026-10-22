#include "TIME.h"
#include "stm32f10x.h"

void TIME_Config(void)
{		
	//1M分频，每10ms进一次中断
	TIM_TimeBaseInitTypeDef 	TIM_TimeBaseInitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6,ENABLE);           //使能TIM时钟
	//------------------------------时基机构体初始化------------------------------/
	TIM_TimeBaseInitStructure.TIM_Period = 10000-1; 	          //自动重装载值
	TIM_TimeBaseInitStructure.TIM_Prescaler=72-1;                 //定时器分频
	TIM_ClearITPendingBit(TIM6,TIM_IT_Update);
	TIM_TimeBaseInit(TIM6,&TIM_TimeBaseInitStructure);            //初始化TIM
	TIM_ITConfig(TIM6,TIM_IT_Update,ENABLE);                      //允许定时器更新中断
	TIM_Cmd(TIM6,ENABLE);  										  //使能定时器
	//----------------------定时中断----------------------/	
	NVIC_InitTypeDef 			NVIC_InitStructure;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
	NVIC_InitStructure.NVIC_IRQChannel=TIM6_IRQn;                 //定时器5中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x00;    //抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x00;           //子优先级5
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}
