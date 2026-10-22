#include "CAN.h"

/*-----------------------------------CAN1(实车CAN3)----------------------------------*/

/*发送结构体*/
CanTxMsg HRT_CTRL=				//整车报文
{0x156,0x156,CAN_Id_Standard,CAN_RTR_Data,8,{0,0,0,0,0,0,0,0}};
/*
[0]低三位是Statu_Flag 第四位是ASMS 五六位是Man_State
[1]Match_Mode
[2]Error_Flag
[3]低四位为Match_Mode 第五位为MODE_LOCK_Flag
[4]LB_State LF_Stat RB_State RF_State
[5]Emergency_Flag
[6]Lose_Time
[7]第一位Motor_EN_State 第二位SC_OUT 第五位RES_GO
*/

/*接收结构体*/
CanRxMsg CAN_RxMessage=
{0x002,0x002,CAN_Id_Standard,CAN_RTR_Data,8,{0,0,0,0,0,0,0,0}};
/*
0x55     选择模式报文
0x110    AMI相关的 目前未知
0x187    德赛RES
0x02     EBS（EBS_Ready EBS_State）
0x402    EBS油压（只判断左前）
0x482	 EBS气压（只判断左前）
0x020    工作站报文
0x232    华海发的四电机目标速度 用于判断华海在线
0x157    //电机状态统一报文 第一个字节是四个电机（左前右前右后左后）的实际状态 第二个字节是整车速度（0：速度小于1m/s 1:速度大于1m/s）
*/

void CAN1_Config()
{
/*----------------------------------------GPIO配置-----------------------------------------------*/
		GPIO_InitTypeDef GPIO_InitStruct;
		RCC_APB2PeriphClockCmd(CAN_GPIO_CLK,ENABLE);

		GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;
		GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AF_PP;
		GPIO_InitStruct.GPIO_Pin=CAN_TX_Pin;
		GPIO_Init(CAN_PORT,&GPIO_InitStruct);
	
		GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;
		GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IPU;
		GPIO_InitStruct.GPIO_Pin=CAN_RX_Pin;
		GPIO_Init(CAN_PORT,&GPIO_InitStruct);
/*----------------------------------------中断配置-----------------------------------------------*/	
		NVIC_InitTypeDef 			NVIC_InitStructure;
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
		NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX0_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;		  
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
/*----------------------------------------CAN配置-----------------------------------------------*/
		CAN_InitTypeDef CAN_InitStructure;
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1,ENABLE);
		CAN_DeInit(CAN1);											//寄存器初始化
		CAN_StructInit(&CAN_InitStructure);
		/*CAN 单元初始化*/
		CAN_InitStructure.CAN_TTCM=DISABLE; 	//MCR-TTCM 关闭时间触发通信模式使能
		CAN_InitStructure.CAN_ABOM=ENABLE; 		//MCR-ABOM 使能自动离线管理
		CAN_InitStructure.CAN_AWUM=ENABLE; 		//MCR-AWUM 使用自动唤醒模式
		CAN_InitStructure.CAN_NART=DISABLE; 	//MCR-NART 禁止报文自动重传		DISABLE-自动重传
		CAN_InitStructure.CAN_RFLM=DISABLE; 	//MCR-RFLM 接收 FIFO 不锁定   DISABLE-溢出时新报文会覆盖原有报文
		CAN_InitStructure.CAN_TXFP=DISABLE; 	//MCR-TXFP 发送 FIFO 优先级 	DISABLE-优先级取决于报文标示符
		CAN_InitStructure.CAN_Mode = CAN_Mode_Normal; //正常工作模式
		CAN_InitStructure.CAN_SJW=CAN_SJW_2tq; //BTR-SJW 重新同步跳跃宽度 2 个时间单元
		CAN_InitStructure.CAN_BS1=CAN_BS1_9tq; //BTR-TS1 时间段 1 占用了 9 个时间单元
		CAN_InitStructure.CAN_BS2=CAN_BS2_8tq; //BTR-TS1 时间段 2 占用了 8 个时间单元
		CAN_InitStructure.CAN_Prescaler =4;
		CAN_Init(CAN1, &CAN_InitStructure);
		//ss=1 bs9=5 bs2=8 位时间宽度为(1+9+8) 波特率即为时钟周期 tq*(1+9+8)
		//CAN Baudrate = 500 kBps (1MBps 已为 stm32 的 CAN 最高速率) (CAN 时钟频率为 APB 1 = 36 MHz)
		//BTR-BRP 波特率分频器 定义了时间单元的时间长度 36/(1+9+8)/4=500 kbps
/*----------------------------------------筛选器配置-----------------------------------------------*/
		CAN_FilterInitTypeDef  CAN_FilterInitStructure;
		CAN_FilterInitStructure.CAN_FilterNumber=0;															 //筛选器组0
		CAN_FilterInitStructure.CAN_FilterScale=CAN_FilterScale_32bit;				   //筛选器位宽为单个32位。
		
		CAN_FilterInitStructure.CAN_FilterMode=CAN_FilterMode_IdMask;					  	//工作在掩码模式
		CAN_FilterInitStructure.CAN_FilterIdHigh=0x0000;            							//要筛选的ID高位 
		CAN_FilterInitStructure.CAN_FilterIdLow=0x0000;     											//要筛选的ID低位 
		CAN_FilterInitStructure.CAN_FilterMaskIdHigh=0x0000;											//筛选器高16位每位必须匹配
		CAN_FilterInitStructure.CAN_FilterMaskIdLow=0x0000;												//筛选器低16位每位必须匹配
		
		CAN_FilterInitStructure.CAN_FilterFIFOAssignment=CAN_Filter_FIFO0 ;			 //筛选器被关联到FIFO0
		CAN_FilterInitStructure.CAN_FilterActivation=ENABLE;										 //使能筛选器
		CAN_FilterInit(&CAN_FilterInitStructure);
		/*CAN通信中断使能*/	
		CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);
		CAN1->RF0R |= CAN_RF0R_RFOM0;
}

void CAN_Config()
{
	CAN1_Config();
}

//-------------------CAN1(实车CAN3)发送函数--------------//
uint8_t CAN_Send(CanTxMsg txmsg)      
{
	int i=0;
	uint8_t mbox=CAN_Transmit(CAN1, &txmsg);								//利用函数发送报文至CAN1总线上
	while((CAN_TransmitStatus(CAN1, mbox)==CAN_TxStatus_Failed)&&(i<0XFFF)) //发送检验
	{
		i++;
	}
	if(i>=0XFFF)
	{
		return 0;															//发送失败
	}		
	return 1;																//发送成功
}
