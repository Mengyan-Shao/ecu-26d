#include "Flag.h"

/*-------------总状态位----------------*/
uint8_t		 Statu_Flag=0;			//状态标志位：0低压 1有人高压 2无人高压 3无人准备(AS READY) 4无人行驶(AS DRIVING) 5无人完成(AS FINISHED) 6无人制动(AS EMERGENCY)		
uint8_t		 Match_Mode=0;			//比赛模式:		0有人  1直线加速A  	2转向标定   3八字绕环  	 4高速循迹A    5高速循迹B     6制动测试    7无人检查_感知  8调试模式  9无人检查_电气 10有人录包 11无人遥控
uint8_t		 Error_Flag=0;			//错误源(1错误)								
uint8_t		 MODE_LOCK_Flag=0;			//选模式确认位	0：可以更换模式 	1:锁定模式（不能修改）
uint32_t 	 time0=0;				//计时time0  中断系统时间
volatile uint8_t Tick_10ms=0;		//10ms节拍标志
uint8_t		 HUAHAI_LOSE_Flag=0;   //华海错误标志位
uint8_t		 HUAHAI_Enable=0;		//华海使能 	1有人使能	2无人使能
uint8_t		 AMI_State=0;           //AMI标志位:0:AS Off,1:AS Rrady,2:AS Driving,3:AS Finished,4:AS Emergency,5:Manual Driving
uint16_t	 ReadyTime=0;			//准备状态时间

uint8_t		 Motor_EN_State=0;		//电机使能状态位：0：电机没有使能	    1:电机不使能
uint8_t		 Motor_LOSE_Flag=0;		//电机控制器错误标志位

uint8_t 	 Motor_LF_State=0;		//左前电机实际状态   0：低压 1：高压 2：报错 3：使能
uint8_t 	 Motor_LB_State=0;		//左后电机实际状态   0：低压 1：高压 2：报错 3：使能
uint8_t 	 Motor_RF_State=0;		//右前电机实际状态   0：低压 1：高压 2：报错 3：使能
uint8_t 	 Motor_RB_State=0;		//右后电机实际状态   0：低压 1：高压 2：报错 3：使能

uint8_t		 LF_Enable_State=0;		//左前使能情况	 0：手动上使能  1：意外上使能
uint8_t		 LF_State=0;			//左前控制上使能状态位      0：下使能 1：请求上使能 3：成功上使能
uint8_t		 LB_Enable_State=0;		//左后使能情况	 0：手动上使能  1：意外上使能
uint8_t		 LB_State=0;			//左后控制上使能状态位
uint8_t		 RF_Enable_State=0;		//右前使能情况	 0：手动上使能  1：意外上使能
uint8_t		 RF_State=0;			//右前控制上使能状态位
uint8_t		 RB_Enable_State=0;		//右后使能情况	 0：手动上使能  1：意外上使能
uint8_t		 RB_State=0;			//右后控制上使能状态位
//-------------有人状态位------------/
uint8_t		 Man_State=0;			//有人模式状态	0：低压 1：有人高压 2：有人待驶
uint8_t 	 ToDriveFlag=0;		    //待驶模式标志位 1:进入待驶模式,电机可对加速踏板做出响应	
uint16_t	 Beep_man=0;			//有人待驶蜂鸣器计数
uint8_t 	 Beep_Once=0;			//只使能一次标志位
uint8_t		 Brake_Flag=0;          //0：油压小于1500 1：油压大于等于1500	
uint8_t      KeyFlag =0;            //使能按键确认位 1:使能按键已被按下 0：未按下或已更新比赛模式
uint16_t 	 Key_Time=0;			//使能按钮检测
uint8_t		 ModeFlag =0;		    //0：允许检测按键 1：正在处理按键，防止重复触发
uint16_t	 Brake_Time=0;		    //踩踏板检测
//-------------无人状态位------------/
uint8_t      EBS_Ready=0;           //EBS准备：0 未准备好 1 准备好
uint8_t		 EBS_State=0;			//EBS状态：6紧急制动
uint16_t	 EBS_LOSE_Flag=40;	    //EBS错误标志位
uint8_t		 RES_State=8;			//RES状态：0急停 4准备 5GO
uint16_t	 RES_LOSE_Flag=10;	    //RES错误标志位
uint8_t		 Beep_ros=3;			//启动鸣笛为0、制动鸣笛为1，默认值为3
uint16_t	 Beep_ros_count=0;		//蜂鸣时间计数
uint16_t	 ROS_LOSE_Flag=40;		//ROS错误位
uint16_t	 Yellowcount=0;			//YellowFlash计时
uint16_t	 Bluecount=0;			//BlueFlash计时
uint8_t		 ASSI_State=0;          //0关闭,1黄灯常亮,2黄灯闪烁,3蓝灯常亮,4蓝灯闪烁
uint8_t		 ROS_Statu_Flag=0;		//接收工作站状态位  从 1 开始
uint8_t      Motor_Speed=0;		    //整车速度(以左后电机作为代表) 0为速度小于1m/s（停车） 1为速度大于1m/s（行驶）
uint8_t		 Emergency_Flag=0;		//锁定最先进入紧急制动的状态 0：没掉线 1：ROS掉线 2：RES掉线 3：EBS掉线 
uint8_t      Lose_Time=0;           //设备掉线时间  Emergency_Flag!=0时，Lose_Time开始计时，最多至255(2.55s)
uint16_t     Time_Ready=0;		    //工作站完成计数
uint8_t      RES_GO=0;				//进入AS Driving蜂鸣完成后此标志位记1
uint8_t      Beep_flag=0;			//蜂鸣器标志位  0：工作中 1: 已完成蜂鸣
uint8_t		 RES_LOST =0;
uint8_t      Steering_ERROR = 0;     //转向电机报错位 0正常 1报错
uint16_t      Steering_LOST_Flag =0;  //转向电机掉线位

//---------------EBS油压（左前）----------------/
uint16_t   			EBS_Oilh;   //油压数据是两个字节 但CAN一次只能发一个字节 拆成两个高和低两个字节再组合在一起
uint16_t    		EBS_Oill;
uint16_t 			EBS_Oil;

//---------------EBS气压（左前）----------------/
uint16_t 			EBS_AirH;	//气压数据是两个字节 但CAN一次只能发一个字节 拆成两个高和低两个字节再组合在一起
uint16_t 			EBS_AirL;
uint16_t 			EBS_Air;

//遥控器
CarControl_t SBUS;
