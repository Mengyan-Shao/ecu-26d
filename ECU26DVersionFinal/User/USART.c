#include "USART.h"                  // Device header

uint8_t Serial_RxData;		//定义串口接收的数据变量
uint8_t Serial_RxFlag;		//定义串口接收的标志位变量

volatile uint8_t Serial_SBUSFrame[25];	//SBUS完整帧缓冲区（0x0F开头，0x00结尾，共25字节）
volatile uint8_t Serial_SBUSFrameIndex;	//SBUS帧接收索引
volatile uint8_t Serial_SBUSFrameFlag;	//SBUS完整帧标志位

#include "stm32f10x.h"

void Serial_Init(void)
{
	// 1. 开启对应时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);   // PB10 PB11时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);  // USART3在APB1总线

	// 2. GPIO初始化 PB11 = USART3_RX 复用输入
	GPIO_InitTypeDef GPIO_InitStructure;
	// PB11 RX
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  // 串口RX复用输入
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// 3. USART3 SBUS参数配置（你原有参数保留，外设改为USART3）
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 100000;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx;        // 仅接收
	USART_InitStructure.USART_Parity = USART_Parity_Even;  // 偶校验
	USART_InitStructure.USART_StopBits = USART_StopBits_2;  // 2停止位
	USART_InitStructure.USART_WordLength = USART_WordLength_9b; //9位数据
	USART_Init(USART3, &USART_InitStructure);

	// 4. 开启USART3接收中断
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);

	// 5. NVIC配置 USART3中断通道（与CAN/TIME保持Group_0）
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;      // 关键：USART3中断向量
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStructure);

	// 6. 使能USART3外设
	USART_Cmd(USART3, ENABLE);
}

/**
  * 函    数：获取一帧完整的SBUS数据
  * 参    数：Frame 存放25字节SBUS帧的数组首地址
  * 返 回 值：1=已获取到新的一帧，0=暂无新帧
  */
uint8_t Serial_GetSBUSFrame(uint8_t *Frame)
{
	if (Serial_SBUSFrameFlag == 1)		//如果标志位为1
	{
		USART_ITConfig(USART3, USART_IT_RXNE, DISABLE);	//拷贝期间锁住接收缓冲
		Serial_SBUSFrameFlag = 0;
		uint8_t i;
		for(i = 0; i < 25; i++)
		{
			Frame[i] = Serial_SBUSFrame[i];
		}
		USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
		return 1;						//返回1，并自动清零标志位
	}
	return 0;							//如果标志位为0，则返回0
}


/**
  * 函    数：USART1中断函数
  * 参    数：无
  * 返 回 值：无
  * 注意事项：此函数为中断函数，无需调用，中断触发后自动执行
  *           函数名为预留的指定名称，可以从启动文件复制
  *           请确保函数名正确，不能有任何差异，否则中断函数将不能进入
  */
void USART3_IRQHandler(void)
{
	if (USART_GetITStatus(USART3, USART_IT_RXNE) == SET)		//判断是否是USART1的接收事件触发的中断
	{
		uint8_t Data = USART_ReceiveData(USART3);				//读取数据寄存器
		Serial_RxData = Data;									//存放在接收的数据变量
		Serial_RxFlag = 1;										//置接收标志位变量为1

		/*SBUS帧组装：帧头0x0F，帧尾0x00，共25字节*/
		if (Serial_SBUSFrameIndex == 0)							//等待帧头
		{
			if (Data == 0x0F)									//是帧头则开始收帧
			{
				Serial_SBUSFrame[0] = Data;
				Serial_SBUSFrameIndex = 1;
			}
		}
		else if (Serial_SBUSFrameIndex < 25)					//收帧中
		{
			Serial_SBUSFrame[Serial_SBUSFrameIndex] = Data;
			Serial_SBUSFrameIndex++;
			if (Serial_SBUSFrameIndex == 25)					//收满25字节
			{
				if (Data == 0x00)								//校验帧尾
				{
					Serial_SBUSFrameFlag = 1;					//置完整帧标志位
				}
				Serial_SBUSFrameIndex = 0;						//重新开始找帧头
			}
		}
		USART_ClearITPendingBit(USART3, USART_IT_RXNE);			//清除USART1的RXNE标志位
																//读取数据寄存器会自动清除此标志位
																//如果已经读取了数据寄存器，也可以不执行此代码
	}
}


/* ============================ 配置 ============================ */
// 6C-MINI（右手油门模式）：
//   左摇杆左右 = 通道4 -> 方向 -25~+25 度
//   右摇杆上下 = 通道3 -> 速度 0~3 m/s
#define SBUS_CH_DIRECTION	3	// channels[3] = 通道4
#define SBUS_CH_SPEED		2	// channels[2] = 通道3

// SBUS 通道原始值范围（见 MC7RE-V2 说明书）
#define SBUS_RAW_MIN		200
#define SBUS_RAW_MAX		1800
#define SBUS_RAW_CENTER		1000

// 输出范围
#define STEERING_MAX_DEG	7	// 方向：-7 ~ +7 度
#define SPEED_MAX_CM_S		3	// 速度：0 ~ 3 m/s

// 摇杆中位死区（原始值），防止手抖导致输出抖动
#define DEADZONE			30

// 信号丢失：约1s（主循环10ms一次）收不到完整帧
#define LOST_TIMEOUT		100

// 停车模式参数（SBUS每帧约14ms）
#define PARK_ENTER_FRAMES	70	// 推到底保持约1秒进入停车
#define PARK_EXIT_FRAMES	20	// 推到高位保持约0.3秒解除停车
#define PARK_BOTTOM_OFFSET	100	// 与行程最低点的差值，小于此值视为推到底

// 校准完成判定参数（原始值，中位=1000）
#define CALIB_MIN_SIDE		150	// 中位每侧至少要推到这么远
#define CALIB_MIN_DEVIATION	200	// 行程跨度达到该值才认为行程足够

/* ===================== 摇杆行程自动校准 ===================== */
// 开机后把两个摇杆向两侧各推一次，再回到中位，校准即完成。
// 校准期间方向/速度都输出0，避免第一次推杆就跳到峰值。
// 映射固定以中位1000为0点，回中始终输出0，不会因左右行程不对称而偏置。
typedef struct
{
	uint16_t min_raw;
	uint16_t max_raw;
} RawCalib_t;

static RawCalib_t steer_calib = {SBUS_RAW_CENTER, SBUS_RAW_CENTER};
static RawCalib_t speed_calib = {SBUS_RAW_CENTER, SBUS_RAW_CENTER};
static uint8_t steer_ready = 0;
static uint8_t speed_ready = 0;

// 用当前原始值扩展校准范围
static void Calib_Update(RawCalib_t *calib, uint16_t raw)
{
	if (raw < calib->min_raw) { calib->min_raw = raw; }
	if (raw > calib->max_raw) { calib->max_raw = raw; }
}

// 中位左侧/右侧行程
static int32_t Calib_LeftDev(const RawCalib_t *calib)
{
	return SBUS_RAW_CENTER - calib->min_raw;
}

static int32_t Calib_RightDev(const RawCalib_t *calib)
{
	return calib->max_raw - SBUS_RAW_CENTER;
}

// 有效半行程：取左右两侧中较大者
static int32_t Calib_HalfSpan(const RawCalib_t *calib)
{
	int32_t left = Calib_LeftDev(calib);
	int32_t right = Calib_RightDev(calib);
	return left > right ? left : right;
}

// 两侧是否都扫过足够行程
static uint8_t Calib_SweepDone(const RawCalib_t *calib)
{
	return Calib_LeftDev(calib) >= CALIB_MIN_SIDE &&
	       Calib_RightDev(calib) >= CALIB_MIN_SIDE &&
	       Calib_HalfSpan(calib) >= CALIB_MIN_DEVIATION;
}

// 摇杆是否在中位死区内
static uint8_t InCenter(uint16_t raw)
{
	return raw > SBUS_RAW_CENTER - DEADZONE && raw < SBUS_RAW_CENTER + DEADZONE;
}

// 原始值以中位1000为0点，按有效半行程映射为 -max~+max（用于方向）
static int16_t Calib_ToSigned(uint16_t raw, const RawCalib_t *calib, int16_t max)
{
	int32_t half = Calib_HalfSpan(calib);
	if (half <= 0) { return 0; }
	return (int16_t)(((int32_t)raw - SBUS_RAW_CENTER) * max / half);
}

// 原始值以中位1000为0点，向上推时映射为0~max（用于速度）
static uint16_t Calib_ToSpeed(uint16_t raw, const RawCalib_t *calib, uint16_t max)
{
	int32_t half = Calib_HalfSpan(calib);
	int32_t value = 0;
	if (half > 0 && raw > SBUS_RAW_CENTER + DEADZONE)
	{
		value = ((int32_t)raw - SBUS_RAW_CENTER) * max / half;
		if (value > max) { value = max; }
	}
	return (uint16_t)value;
}

/* ========================= 数据解析 ========================= */

// 将原始值限幅到合法范围
static uint16_t ClampRaw(uint16_t raw)
{
	if (raw < SBUS_RAW_MIN) { raw = SBUS_RAW_MIN; }
	if (raw > SBUS_RAW_MAX) { raw = SBUS_RAW_MAX; }
	return raw;
}

// 从25字节SBUS帧中按位解析16个通道，每个通道11bit
static void SBUS_ParseChannels(uint8_t *buf, uint16_t *channels)
{
	uint8_t *data = &buf[1];
	int bit_pos = 0;

	for (int ch = 0; ch < 16; ch++)
	{
		uint16_t val = 0;
		for (int b = 0; b < 11; b++)
		{
			if (data[bit_pos / 8] & (1 << (bit_pos % 8)))
			{
				val |= (1 << b);
			}
			bit_pos++;
		}
		channels[ch] = val;
	}
}

/* ======================= 主循环调用入口 ======================= */
// 每帧流程：取帧 -> 解析/校准 -> 丢信号与停车处理 -> 更新角度和速度
// 停车、信号丢失等状态都在函数内部处理，外部只需要读取角度和速度。
void SBUS_Process(CarControl_t *ctrl)
{
	uint8_t frame[25];
	uint16_t channels[16];
	uint16_t raw_steer;
	uint16_t raw_speed;
	uint8_t throttle_bottom;
	uint8_t throttle_active;
	static CarControl_t current;	// 最近一次有效控制量
	static uint8_t lost = 1;		// 信号丢失状态
	static uint8_t parked = 0;		// 停车锁定状态
	static uint8_t rearm = 0;		// 停车解除后需回中才能重新加速
	static uint16_t timeout = 0;	// 未收到帧的累计计数
	static uint16_t park_enter = 0;	// 推到底累计帧数
	static uint16_t park_exit = 0;	// 推高累计帧数

	// 1. 取帧：没收到帧时累计超时，超过约100ms判定信号丢失
	if (!Serial_GetSBUSFrame(frame))
	{
		timeout++;
		if (timeout > LOST_TIMEOUT && !lost)
		{
			lost = 1;			// 信号丢失：方向归中、速度归零
			current.steering_deg = 0;
			current.speed_m_s = 0;
		}
		current.lost = lost;
		current.parked = parked;
		current.driving = (speed_ready && !lost && !parked && !rearm) ? 1 : 0;
		*ctrl = current;
		return;
	}
	timeout = 0;

	// 2. 解析通道并更新摇杆行程校准
	SBUS_ParseChannels(frame, channels);
	raw_steer = ClampRaw(channels[SBUS_CH_DIRECTION]);
	raw_speed = ClampRaw(channels[SBUS_CH_SPEED]);
	Calib_Update(&steer_calib, raw_steer);
	Calib_Update(&speed_calib, raw_speed);

	// 3. 校准锁存：行程扫够且摇杆回到中位后生效，之后一直保持
	if (!steer_ready && Calib_SweepDone(&steer_calib) && InCenter(raw_steer))
	{
		steer_ready = 1;
	}
	if (!speed_ready && Calib_SweepDone(&speed_calib) && InCenter(raw_speed))
	{
		speed_ready = 1;
	}

	// 4. 帧标志：0x10=信号丢失，0x20=失控保护，触发后停车归中
	if (frame[23] & 0x30)
	{
		lost = 1;
		current.steering_deg = 0;
		current.speed_m_s = 0;
		current.lost = lost;
		current.parked = parked;
		current.driving = 0;
		*ctrl = current;
		return;
	}
	lost = 0;
	current.lost = 0;
	current.parked = parked;
	current.driving = (speed_ready && !parked && !rearm) ? 1 : 0;

	// 5. 映射方向（校准完成前输出0；左正右负）
	current.steering_deg = steer_ready && !InCenter(raw_steer)
		? -Calib_ToSigned(raw_steer, &steer_calib, STEERING_MAX_DEG)
		: 0;

	// 6. 停车模式：推到底保持约1秒进入；推过70%行程保持约0.3秒解除
	throttle_bottom = speed_ready && (raw_speed <= speed_calib.min_raw + PARK_BOTTOM_OFFSET);
	throttle_active = speed_ready && (raw_speed >= SBUS_RAW_CENTER + Calib_HalfSpan(&speed_calib) * 7 / 10);
	if (throttle_bottom)
	{
		if (park_enter < PARK_ENTER_FRAMES) { park_enter++; }
		if (park_enter >= PARK_ENTER_FRAMES)
		{
			parked = 1;
			rearm = 1;
		}
		park_exit = 0;
	}
	else if (throttle_active && parked)
	{
		if (park_exit < PARK_EXIT_FRAMES) { park_exit++; }
		if (park_exit >= PARK_EXIT_FRAMES)
		{
			parked = 0;
			rearm = 1;	// 解除停车后必须回中才能重新加速
			park_enter = 0;
			park_exit = 0;
		}
	}
	else
	{
		// 摇杆在中位：保持当前停车状态，仅清零计时
		park_enter = 0;
		park_exit = 0;
	}

	// 回中后解除加速锁定
	if (rearm && InCenter(raw_speed))
	{
		rearm = 0;
	}

	// 7. 速度：停车中或停车解除后未回中时输出0，回中后按行程线性加速
	current.speed_m_s = (speed_ready && !parked && !rearm)
		? Calib_ToSpeed(raw_speed, &speed_calib, SPEED_MAX_CM_S)
		: 0;

	// 8. 输出最新角度和速度
	current.lost = lost;
	current.parked = parked;
	current.driving = (speed_ready && !lost && !parked && !rearm) ? 1 : 0;
	*ctrl = current;
}


