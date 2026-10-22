#include "stm32f10x.h"
#include "stm32f10x_it.h"
#include "GPIO.h"
#include "CAN.h"
#include "TIME.h"
#include "Flag.h"


int main(void)
{
    GPIO_Config();
    CAN_Config();
    TIME_Config();
    while(1) 
	{	
		if(Tick_10ms)
		{
			Tick_10ms=0;
			ECU_Task_10ms();
		}
	}
}
