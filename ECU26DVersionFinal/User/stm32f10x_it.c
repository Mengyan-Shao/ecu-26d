#include "stm32f10x_it.h" 
#include "GPIO.h"
#include "CAN.h"
#include "TIME.h"
#include "Flag.h"

//---------------For CAN3---------------/EBS,RES,ROS，AMI，华海
extern		 CanTxMsg	 HRT_CTRL;
extern		 CanRxMsg	 CAN_RxMessage;

void      	 AS_ERROR_Test(void);
void      	 ASSI_SHOW(void);
void		 AS_BEEP(void);
void		 Man_BEEP(void);
void		 Man_ERROR_TEST(void);
void		 Motor_State(void);
void		 Motor_State_Disable(void);
void		 Emergency_Flag_Config(void);
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
中断函数
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
//-----------------------------定时中断TIM6：100 HZ == 10ms--------------------------//---只打节拍---//
void TIM6_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM6,TIM_IT_Update)==SET)
	{
		TIM_ClearITPendingBit(TIM6,TIM_IT_Update);
		Tick_10ms=1;
	}
}
/*----------------------------10ms任务：主循环调用----------------------------*/
void ECU_Task_10ms(void)
{
	//------------------------------------有人模式---------------------------------------//
	if(Match_Mode==0 || MODE_LOCK_Flag==0)
		{
			//制动检测 EBS油压
			if(EBS_Oil>=1000)	
			{				
				Brake_Flag=1;
			}
			else
			{
				Brake_Flag=0;
			}
			
			//制动踏板时间检测
			if(Brake_Flag==1)
			{
				Brake_Time++;
				if(Brake_Time>3000)
					Brake_Time=3000;
			}
			else
			{
				Brake_Time=0;
			}
			
			//使能按钮检测
			if(KEY_ENABLE)  
			{
				Key_Time++;
				KeyFlag=1;
				if(Key_Time>=3000)
				{
					Key_Time=3000;
				}
			}
			else
			{
				Key_Time=0;
			}
		
			//有人模式状态机
			switch(Man_State)
			{
				case 0: //低压
					Statu_Flag=0;
					AMI_State=0;

					Motor_State_Disable();
				
					if(SC_OUT)
					{
						Man_State=1;
					}
					
					if(KeyFlag==1 && ModeFlag==0 && !KEY_ENABLE && MODE_LOCK_Flag==0)
					{
						ModeFlag=1;
						Match_Mode++;		
						KeyFlag=0;
						if(Match_Mode>11)
							Match_Mode-=12;
					}
					
					if(KeyFlag==0&&ModeFlag==1)
					{
						ModeFlag=0;
					}
					
					if(Key_Time >=50)
					{
						MODE_LOCK_Flag=1;
					}

					ToDrive_LED_OFF;      
					break;
					
				case 1: //高压 
					Statu_Flag=1;
					AMI_State=0;
					if(MODE_LOCK_Flag==0)
					{
						Match_Mode=0;
					}
				
					Motor_State_Disable();
				
					if(!SC_OUT)
					{
						Man_State=0;
					}
					
					if((Key_Time>100)&&(Brake_Time>100)&&(EBS_Air<100)) //判断使能按键 制动踏板 EBS释放
					{
						Man_State=2;
					}						
					
					ToDrive_LED_OFF;
					break;
					
				case 2: //使能 Manual Driving
					Statu_Flag=1;
					if(MODE_LOCK_Flag==0)
					{
						Match_Mode=0;
					}
					AMI_State=5;
				
					if(!SC_OUT)
					{
						Man_State=0;
					}
					
					if((Key_Time>200)&&(Brake_Time<100))//手动下使能
					{
						Man_State=1;
					}
					
					if(Motor_EN_State)//使能灯条件
					{
						ToDrive_LED_ON;
					}
					
					Motor_State(); //控制器上使能
					break;
			}	
			
			Man_BEEP();          
			ASSI_State=0;
			Man_ERROR_TEST();
		}
//------------------------------------无人模式---------------------------------------//
		else if(Match_Mode!=0 && MODE_LOCK_Flag==1)//模式锁定
		{
			if (Statu_Flag == 1)
			{
				Statu_Flag = 0;   // 把旧有人状态清成低压
				Man_State = 0;
			}
			switch(Statu_Flag)		
			{
				case 0: //低压   AS OFF
					AMI_State=0;
					RES_GO=0;
				
					//低压->无人高压
					if(ASMS&&SC_OUT)
					{
						Statu_Flag=2;
						ASSI_State=0;
					}
					
					ASSI_State=0;
					Motor_State_Disable();
					break;
					
				case 2: //无人高压  AS OFF
					AMI_State=0;
				
					//无人高压->准备
					if(Error_Flag==0&&ROS_Statu_Flag==3&&RES_State==4&&EBS_Ready==1)
					{
						Motor_State();
						Time_Ready++;
						if(Time_Ready >= 100 && Motor_EN_State==1)
						{
							Statu_Flag=3;
							ASSI_State=1;
						}
					}
					
					if(!SC_OUT)
					{
						Statu_Flag=0;
					}
					break;
					
				case 3: //准备 AS Ready
					Motor_State();
					AMI_State=1;
					ReadyTime++;
				
					//准备->行驶
					if(RES_State==5)//接收段判断GO（已经5S延时过）
					{
						ASSI_State=2;
						Beep_ros=0;//启动蜂鸣
						Statu_Flag=4;
					}

					//准备->制动
					//掉高压,ROS掉线,转向电机掉线
					if(!SC_OUT || ROS_LOSE_Flag==255 || Steering_LOST_Flag==255)		
					{
						Emergency_Flag_Config();
						Statu_Flag=6;
						ASSI_State=4;
						Beep_ros=1;
					}
					break;
					
				case 4://行驶  AS DRIVING
					Motor_State();
					AMI_State=2;
				
					if(Beep_flag)
						{
							Beep_ros=3;	//蜂鸣完成
							RES_GO=1;
							Beep_flag=0;
						}
						
					//行驶->制动
					//掉高压，ROS掉线，转向电机报错，转向电机掉线
					if(!SC_OUT || ROS_LOSE_Flag==255 || Steering_ERROR || Steering_LOST_Flag==255)
					{
						Emergency_Flag_Config();
						Statu_Flag=6;
						ASSI_State=4;
						Beep_ros=1;
					}	
					
					//行驶->完成
					if(ROS_Statu_Flag==5&&ASMS)
					{
						if(Motor_Speed<1)
						{
							Statu_Flag=5;
							ASSI_State=3;
						}
					}
					
					break;
					
				case 5://完成 AS FINISHED
					Motor_State_Disable();
					AMI_State=3;
				
					//完成->制动
					if(RES_State==0)//RES EME
					{
						Statu_Flag=6;	
						ASSI_State=4;
						Beep_ros=1;
					}
					
					//完成->低压
					if(!ASMS&&EBS_Air<=100)
					{
						ASSI_State=0;
						BEEP_OFF;
						Beep_ros=3;
						Statu_Flag=0;
					}
					break;
					
				case 6://制动 AS EMERGENCY
					Motor_State();
					AMI_State=4;
				
					if(!ASMS && Beep_flag==1 && EBS_Air<=100)
					{
						ASSI_State=0;
						BEEP_OFF;
						Beep_ros=3;
						Statu_Flag=0;
					}

					break;
			}	
			
			//掉线时间计数
			if(Emergency_Flag && Error_Flag && Lose_Time<255)
			{
				Lose_Time++;
			}			
			
			//使能灯状态
			if(Motor_EN_State==1)    
			{			
				ToDrive_LED_ON;
			}
			else
			{
				ToDrive_LED_OFF;
			}
			
			AS_ERROR_Test();
			ASSI_SHOW();     
			AS_BEEP();//无人蜂鸣
		}
//------------------------------------有人&无人--------------------------------------//	 	
	//-------------CAN1发送-----------//		
		if(time0%2==1)
		{
			HRT_CTRL.Data[0]=(uint8_t)(Statu_Flag|ASMS<<3|Man_State<<4);						//0(ASMS) 000(Statu_Flag)
			if(MODE_LOCK_Flag==1)
			{
				HRT_CTRL.Data[1]=(uint8_t)Match_Mode;
			}
			HRT_CTRL.Data[2]=(uint8_t)(Error_Flag);			
			HRT_CTRL.Data[3]=(uint8_t)(Match_Mode|MODE_LOCK_Flag<<4);				//0 0000
			HRT_CTRL.Data[4]=(uint8_t)((LB_State)|(LF_State<<2)|(RB_State<<4)|(RF_State<<6));	//00 00 00 00
			HRT_CTRL.Data[5]=(uint8_t)((Emergency_Flag)| (SBUS.steering_deg <<4));		
			HRT_CTRL.Data[6]=(uint8_t)Lose_Time;
			HRT_CTRL.Data[7]=(uint8_t)((Motor_EN_State)|(SC_OUT<<1)|(RES_GO<<4)|((SBUS.speed_m_s * 2) << 5));	
			if(CAN_Send(HRT_CTRL))//检测是否发送成功
			{
				CAN_TX_Toggle;
			}
		}

		//-------------时间复位----------//
		if(time0==100)
		{
			time0=0;
		}
		time0++;	
}
/*----------------------------定时中断以上----------------------------*/

//-------------------CAN1接收中断--------------------//
void CAN1_RX0_IRQHandler(void)
{
	CAN_RX_Toggle;  //CAN3接收指示灯闪烁
	
	CAN_Receive(CAN1, CAN_FIFO0, &CAN_RxMessage);
	
	switch(CAN_RxMessage.StdId)
	{
		case 0x055:			//选择模式报文
			Match_Mode=CAN_RxMessage.Data[0];
			MODE_LOCK_Flag=1;
			break;

		case 0x003:			//RES
			switch(CAN_RxMessage.Data[0])
			{
				case 7://GO信号
					RES_State=5;
					break;
				case 5://准备
					RES_State=4;
					break;
				case 0://紧急制动
					RES_State=0;
					break;
			}
			RES_LOSE_Flag=0;
			break;

		case 0x002:		//EBS状态和EBS气压（只判断左前）
			EBS_LOSE_Flag=0;
			EBS_Ready=(CAN_RxMessage.Data[4]&0x10)>>4;
			EBS_State=CAN_RxMessage.Data[5]&0x0F;	
		    EBS_AirH = (uint16_t)CAN_RxMessage.Data[1];
			EBS_AirL = (uint16_t)CAN_RxMessage.Data[0];
			EBS_Air =  (EBS_AirH<<8) |  EBS_AirL;
		    break;	
		
		case 0x402:		//EBS油压（只判断左前）
			EBS_Oilh =  (uint16_t)CAN_RxMessage.Data[1];
			EBS_Oill =  (uint16_t)CAN_RxMessage.Data[0];	 
			EBS_Oil  =  (EBS_Oilh<<8) | EBS_Oill; 
			break;
		
		case 0x020:		//工作站报文
			ROS_LOSE_Flag=0;
			if(Statu_Flag==2||Statu_Flag==3||Statu_Flag==4||Statu_Flag==5||Statu_Flag==6)				//无人状态
			{
				ROS_Statu_Flag=CAN_RxMessage.Data[0] & 0x0F;
			}
			break;
			
		case 0X232:    //华海发的四电机目标速度 用于判断华海在线
			HUAHAI_LOSE_Flag = 0;
			break;
		
		case 0x157:     //电机状态统一报文 第一个字节是四个电机（左后左前右后右前）的实际状态 第二个字节是整车速度（0：速度小于1m/s 1:速度大于1m/s）
            Motor_LB_State = (CAN_RxMessage.Data[0]) & 0x03;
            Motor_LF_State = (CAN_RxMessage.Data[0] >> 2) & 0x03;
            Motor_RB_State = (CAN_RxMessage.Data[0] >> 4) & 0x03;
            Motor_RF_State = (CAN_RxMessage.Data[0] >> 6) & 0x03;

            Motor_Speed = CAN_RxMessage.Data[1] & 0x01;
                
            Motor_LOSE_Flag = 0;
            break;
		
		case 0x381:    //转向电机报错检测
			Steering_ERROR = (CAN_RxMessage.Data[0]) &0x08; //转向电机报错检测
			Steering_LOST_Flag = 0; //转向电机掉线检测
			break;
		
	}
	if(Motor_LF_State==3||Motor_LB_State==3||Motor_RF_State==3||Motor_RB_State==3)
	{
		Motor_EN_State=1;
	}
	else
	{
		Motor_EN_State=0;
	}
	CAN_RxMessage.StdId=0x00; //回归初始状态
}

//-------------------有人错误检测--------------------//
void Man_ERROR_TEST(void)
{
	Motor_LOSE_Flag++;
	if(Motor_LOSE_Flag>200)
	{
		Motor_LOSE_Flag=200;
	}

}

//-------------------无人状态错误检测----------------//
void AS_ERROR_Test(void)
{
	//---------掉线检测--------//	
		EBS_LOSE_Flag++;
		ROS_LOSE_Flag++;
		RES_LOSE_Flag++;
		HUAHAI_LOSE_Flag++;
		if(Statu_Flag==3||Statu_Flag==4)
		{
			Steering_LOST_Flag++;
		}
		
		if(ROS_LOSE_Flag>255)// 0000 1000
		{
			ROS_LOSE_Flag=255;
			Error_Flag|=0x08;
		}
		else
		{
			Error_Flag&=0xF7;
		}
		
		if(HUAHAI_LOSE_Flag>255)// 1000 0000
		{
			HUAHAI_LOSE_Flag=80;
			Error_Flag|=0x80;
		}
		else
		{
			Error_Flag&=0x7F;
		}
		
		if(EBS_LOSE_Flag>500)
		{
			EBS_LOSE_Flag=500;
			Error_Flag|=0x20;//0010 0000
		}
		else
		{
			Error_Flag&=0xDF;//1101 1111
		}
		
		if(Steering_LOST_Flag>255)
		{
			Steering_LOST_Flag=255;
//			Error_Flag|=0x01;//0000 0001
		}
//		else
//		{
//			Error_Flag&=0xFE;//1111 1110
//		}
		
		if(RES_LOSE_Flag>500)// 0001 0000
		{
			RES_LOSE_Flag=500;
			Error_Flag|=0x10;
		}
		else
		{
			Error_Flag&=0xEF;
		}
}

//-------------------ASSI状态显示--------------------//
void ASSI_SHOW(void)
{
	switch(ASSI_State)
	{
		case 0: 
			NoneLight;
			break;
		case 1:
			YellowLight;
			break;
		case 2:
			if(Yellowcount<=20)
			{
				Yellowcount++;
				if(Yellowcount%10==0)
				{
					Yellow_Toggle;
				}
			}
			else
			{
				Yellowcount=0;
			}
			break;
		case 3:
			BlueLight;
			break;
		case 4:
			if(Bluecount<=20)
			{
				Bluecount++;
				if(Bluecount%10==0)
				{
					Blue_Toggle;
				}
			}
			else
			{
				Bluecount=0;
			}
			break;
		case 5:
			BothLight;
			break;
	}
}	
//-------------------无人蜂鸣器----------------------//
void AS_BEEP(void)
{
	//蜂鸣器--无人启动与紧急制动鸣笛//
	switch(Beep_ros)
	{
		case 0://启动鸣笛
			if(Beep_ros_count<150)
			{
				Beep_ros_count++;
				if(Beep_ros_count%10==0)
				{
					BEEP_ON;
				}
			}
			else
			{
				BEEP_OFF;
				Beep_ros_count=0;
				Beep_ros=3;
				Beep_flag=1;
			}
			break;
		case 1://紧急制动鸣笛   10s+5Hz+0.5占空比
			if(Beep_ros_count<1000)
			{
				Beep_ros_count++;
				if(Beep_ros_count%10==0)
				{
					BEEP_Toggle;
				}
			}
			else
			{
				BEEP_OFF;
				Beep_ros_count=0;
				Beep_ros=3;
				Beep_flag=1;
			}
			break;
	}
}	
//-------------------有人蜂鸣器----------------------//
void Man_BEEP(void)
{
		if(Man_State==2)//待驶状态执行
		{
			if(Beep_Once==0&&Motor_EN_State==1)//蜂鸣器未执行且控制器上使能
			{
				ToDriveFlag=1;//进入待驶蜂鸣
				Beep_Once=1;
			}
		}
		else
		{
			Beep_Once=0;
		}
		
		/*蜂鸣器--进入有人待驶模式*/
		if(ToDriveFlag&&Beep_man<=150)
		{
			Beep_man++;
			BEEP_ON;
		}
		else
		{
			Beep_man=0;
			BEEP_OFF;
			ToDriveFlag=0;//待驶蜂鸣完成
		}
}


//------------------控制器上使能-----------------------//
void Motor_State(void)
{
	/*
	以左前为例：
	LF_Enable_State：
	①0： 高压-->LF_State：0   请求上使能-->LF_State: 1   成功使能-->LF_State：3
	②1： 报错-->LF_State：0   复位成功(高压状态)-->LF_State: 1   成功使能-->LF_State：3
	*/
	//左前
	switch(LF_Enable_State)
	{
		case 0:						//默认手动上使能
			LF_State=1;				//请求上使能

			if(Motor_LF_State==3)	//接收成功上使能
			{
				LF_State=3;			//成功使能
			}
			
			if(Motor_LF_State==2)	//接收报错
			{
				LF_Enable_State=1;	//意外上使能
			}
			break;
			
		case 1:
			LF_State=0;				//下使能
			if(Motor_LF_State==1)	//控制器上高压后请求上使能
			{
				LF_State=1;			
			}
			if(Motor_LF_State==3)	//接收成功上使能
			{
				LF_State=3;			//成功使能
			}
			break;
	}
	//左后
	switch(LB_Enable_State)
	{
		case 0:
			LB_State=1;
			
			if(Motor_LB_State==3)
			{
				LB_State=3;
			}
		
			if(Motor_LB_State==2)
			{
				LB_Enable_State=1;
			}
			break;
			
		case 1:
			LB_State=0;
			if(Motor_LB_State==1)
			{
				LB_State=1;
			}
			if(Motor_LB_State==3)
			{
				LB_State=3;
			}
			break;
	}
	//右前
	switch(RF_Enable_State)
	{
		case 0:
			RF_State=1;
			
			if(Motor_RF_State==3)
			{
				RF_State=3;
			}
			
			if(Motor_RF_State==2)
			{
				RF_Enable_State=1;
			}
			break;
			
		case 1:
			RF_State=0;
			if(Motor_RF_State==1)
			{
				RF_State=1;
			}
			if(Motor_RF_State==3)
			{
				RF_State=3;
			}
			break;
	}
	//右后
	switch(RB_Enable_State)
	{
		case 0:
			RB_State=1;
			
			if(Motor_RB_State==3)
			{
				RB_State=3;
			}
			
			if(Motor_RB_State==2)
			{
				RB_Enable_State=1;
			}
			break;
			
		case 1:
			RB_State=0;
			if(Motor_RB_State==1)
			{
				RB_State=1;
			}
			if(Motor_RB_State==3)
			{
				RB_State=3;
			}
			break;
	}
}
//-------------------控制器下使能--------------------//
void Motor_State_Disable(void)
{
	LF_State=0;
	LB_State=0;
	RF_State=0;
	RB_State=0;
	LF_Enable_State=0;
	LB_Enable_State=0;
	RF_Enable_State=0;
	RB_Enable_State=0;
}

//--------------------紧急制动检测-------------------//
void Emergency_Flag_Config(void)
{
	if(Emergency_Flag==0)
	{
		//掉线判断		
		if(Error_Flag&0x08)//ROS 0000 1000
		{
			Emergency_Flag = 1;
		}
		if(Error_Flag&0x10)//RES 0001 0000
		{
			Emergency_Flag = 2;
		}
		if(Error_Flag&0x20)//EBS 0010 0000
		{
			Emergency_Flag = 3;
		}
		if(Error_Flag&0x40)//HUAHAI 0100 0000
		{
			Emergency_Flag = 4;
		}
		//制动检测
		if(ROS_Statu_Flag == 6 )
		{
			Emergency_Flag = 5;
		}
		if(RES_State == 0)
		{
			Emergency_Flag = 6;
		}
		if(EBS_State == 6)
		{
			Emergency_Flag = 7;
		}
		if(!ASMS)
		{
			Emergency_Flag = 8;
		}
	}
}
