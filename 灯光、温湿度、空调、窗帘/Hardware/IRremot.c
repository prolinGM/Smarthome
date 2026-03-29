#include "IRremot.h"
#include "Delay.h"

// Constants
//空调的不同模式
const uint8_t kGreeAuto  = 0;    //自动模式
const uint8_t kGreeCool  = 1;    //制冷模式
const uint8_t kGreeDry   = 2;    //除湿模式
const uint8_t kGreeFan   = 3;    //送风模式
const uint8_t kGreeHeat  = 4;    //制热模式
const uint8_t kGreeEcono = 5;    //经济模式

//空调的开关状态
const uint8_t kGreeon  = 1;      //开
const uint8_t kGreeoff = 0;      //关

//风扇的不同速度
const uint8_t kGreeFanAuto = 0;  //自动
const uint8_t kGreeFanMin  = 1;  //最小
const uint8_t kGreeFanMed  = 2;  //中等
const uint8_t kGreeFanMax  = 3;  //最大

//空调的温度范围，分别以摄氏度和华氏度表示
const uint8_t kGreeMinTempC = 16;  // Celsius
const uint8_t kGreeMaxTempC = 30;  // Celsius
const uint8_t kGreeMinTempF = 61;  // Fahrenheit
const uint8_t kGreeMaxTempF = 86;  // Fahrenheit

//定时器的最大时间，单位为分钟
const uint16_t kGreeTimerMax = 24 * 60;

//空调的垂直扫风模式
const uint8_t kGreeSwingLastPos    = 0 ;  // 0B0000
const uint8_t kGreeSwingAuto       = 1 ;  // 0B0001
const uint8_t kGreeSwingUp         = 2 ;  // 0B0010
const uint8_t kGreeSwingMiddleUp   = 3 ;  // 0B0011
const uint8_t kGreeSwingMiddle     = 4 ;  // 0B0100
const uint8_t kGreeSwingMiddleDown = 5 ;  // 0B0101
const uint8_t kGreeSwingDown       = 6 ;  // 0B0110
const uint8_t kGreeSwingDownAuto   = 7 ;  // 0B0111
const uint8_t kGreeSwingMiddleAuto = 9 ;  // 0B01001
const uint8_t kGreeSwingUpAuto     = 11;  // 0B01011

//空调的水平扫风模式
const uint8_t kGreeSwingHOff        = 0;  // 0B000
const uint8_t kGreeSwingHAuto       = 1;  // 0B001
const uint8_t kGreeSwingHMaxLeft    = 2;  // 0B010
const uint8_t kGreeSwingHLeft       = 3;  // 0B011
const uint8_t kGreeSwingHMiddle     = 4;  // 0B0100
const uint8_t kGreeSwingHRight      = 5;  // 0B0101
const uint8_t kGreeSwingHMaxRight   = 6;  // 0B0110

//空调显示屏上显示的温度类型
const uint8_t kGreeDisplayTempOff     = 0;  // 0B00
const uint8_t kGreeDisplayTempSet     = 1;  // 0B01
const uint8_t kGreeDisplayTempInside  = 2;  //0B010
const uint8_t kGreeDisplayTempOutside = 3;  // 0B011

// 定义红外发送器协议参数，引导码的时长、高低电平的时长等
const uint16_t GreeHdrMark = 9000;
const uint16_t GreeHdrSpace = 4500; 
const uint16_t GreeBitMark = 620;
const uint16_t GreeOneSpace = 1620;
const uint16_t GreeZeroSpace = 540;
const uint16_t GreeMsgSpace = 20000; 
const uint16_t GreeRepSpace = 40000; 
const uint8_t GreeBlockFooter = 2;//0b010
const uint8_t GreeBlockFooterBits = 3;
//系统内默认控制码
Protocol ptcl;

//逆序
u8 getCheckoutCode(u8 data){
     u8 _data=0;
	for(int i=0;i<=3;i++){
		_data=_data*2;
	  _data=_data+data%2;
		data=data/2;
	}
   return _data;
}



//PWM频率：		Freq = CK_PSC(72000000) / (PSC + 1) / (ARR + 1)
//PWM占空比：	Duty = CCR / (ARR + 1)
//PWM分辨率：	Reso = 1 / (ARR + 1)
void IR_init(void){
	
	//38kHz初始化
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);	//打开定时器4的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);	//打开GPIOB的时钟			/*注意：只有部分端口才有PWM功能*/

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	TIM_InternalClockConfig(TIM4);	//设置定时器4的时钟源为内部时钟源
	
	TIM_TimeBaseInitTypeDef TimBaseInitStructure;		//设置时基单元
	TimBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;		
	TimBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; 	//设置计数模式为向上计数
	TimBaseInitStructure.TIM_Period = 101-1;		//ARR(自动重装器，设置计数峰值)
	TimBaseInitStructure.TIM_Prescaler = 19-1;		//PSC(预分频器，设置计时频率)
	TimBaseInitStructure.TIM_RepetitionCounter = 0;	//设置计数重装值
	TIM_TimeBaseInit(TIM4, &TimBaseInitStructure);
	
	TIM_OCInitTypeDef TIM_OCInitStructure;		//设置输出比较单元
	TIM_OCStructInit(&TIM_OCInitStructure);		//先对TIM_OCInitStructure所有成员赋值，之后只会对部分成员重新赋值，防止出现部分成员未赋值情况
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;	//设置输出比较模式为PWM1模式
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;	//输出比较极性（设置REF，有效电平为高电平）
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;	//输出状态使能
	TIM_OCInitStructure.TIM_Pulse = 0;		//CRR(输出比较值)
	TIM_OC1Init(TIM4,&TIM_OCInitStructure);	//使用输出比较通道3
	TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);  // 添加通道预装载配置

	TIM_Cmd(TIM4,ENABLE);	//定时器4使能
	
	//0x7C062050112000F0 
	//Protocol ptcl;
	//红外码初始化
	 // Byte 0
   /* uint8_t Mode    :3;
    uint8_t Power     :1;
    uint8_t Fan       :2;
    uint8_t SwingAuto :1;
    uint8_t Sleep     :1;
    // Byte 1
    uint8_t Temp        :4;
    uint8_t TimerHalfHr :1;
    uint8_t TimerTensHr :2;
    uint8_t TimerEnabled:1;
    // Byte 2
    uint8_t TimerHours:4;
    uint8_t Turbo     :1;
    uint8_t Light     :1;
    uint8_t ModelA    :1;  // model==YAW1F
    uint8_t Xfan      :1;
    // Byte 3
    uint8_t unknown0        :2;//00
    uint8_t TempExtraDegreeF:1;
    uint8_t UseFahrenheit   :1;
    uint8_t unknown1        :4;  // value=0b0101
    // Byte 4
    uint8_t SwingV      :4;
    uint8_t SwingH      :3;
    uint8_t unknown2     :1; //0
    // Byte 5
    uint8_t DisplayTemp :2;
    uint8_t IFeel       :1;
    uint8_t unknown3    :3;  // value = 0b100 
    uint8_t WiFi        :1;  //0
    uint8_t unknown4    :1;    //0
    // Byte 6
    uint8_t unknown5   :8;  //00000000
    // Byte 7
    uint8_t unknown6    :2; //00
    uint8_t Econo       :1;
    uint8_t unknown7    :1;//0
    uint8_t Sum         :4;*/
	ptcl.Power=kGreeon;
	ptcl.Mode = kGreeCool;  // 初始模式设为有效值
    ptcl.Fan = kGreeFanAuto;
    ptcl.Temp = 24 - kGreeMinTempC;
	ptcl.SwingAuto=kGreeSwingAuto;   
	ptcl.Sleep=0;
	ptcl.TimerHalfHr=0;
	ptcl.TimerTensHr=0;
	ptcl.TimerEnabled=0;
	ptcl.TimerHours=0;
	ptcl.Turbo=0;
	ptcl.Light=0;
	ptcl.anion=0;
	ptcl.Powersv=0;
	ptcl.TempExtraDegreeF=0;
	ptcl.UseFahrenheit=0;
	ptcl.unknown1=5;//0b0101
	ptcl.SwingV=0;
	ptcl.SwingH=0;
	ptcl.DisplayTemp=1;
	ptcl.IFeel=0;
	ptcl.unknown2=0;
	ptcl.WiFi=0;
	ptcl.Econo=0;
	ptcl.unknown3=4;//0b100
	ptcl.unknown4=0;
	ptcl.unknown5=0;
	ptcl.unknown6=0;
	ptcl.unknown7=0;
	ptcl.Sum=(ptcl.Mode-1)+ptcl.Temp+5+ptcl.SwingH+0+ptcl.Econo;// 1.ptcl.Temp-18+ptcl.TimerHours+ptcl.Power*8
                                                             // 2.(ptcl.Mode-1)+ptcl.Temp+5+ptcl.SwingH+0+ptcl.Econo
}

void IR_Send38kHz(uint16_t time,FunctionalState NewState){
	if(NewState == 1){
		TIM_SetCompare1(TIM4,0);
	  Delay_us(time);}
	else
	{
		TIM_SetCompare1(TIM4,50);
	  Delay_us(time);
	}
};

void IR_SendGreeH(void){
    TIM_SetCompare1(TIM4,50);
	  Delay_us(GreeBitMark);
	  TIM_SetCompare1(TIM4,0);
	  Delay_us(GreeOneSpace);
};

void IR_SendGreeL(void){
    TIM_SetCompare1(TIM4,50);
	  Delay_us(GreeBitMark);
	  TIM_SetCompare1(TIM4,0);
	  Delay_us(GreeZeroSpace);
};

void IR_SendMsg(uint8_t cnt,uint8_t data){
     while(cnt--){
	   if(data&0x01){
     IR_SendGreeH();
		 data=data>>1;}
		 else{
		 IR_SendGreeL();
		 data=data>>1;
		 }
	 }
};

void IR_SendGree(Protocol ptcl,uint16_t repeat){
	for (uint16_t r = 0; r <= repeat; r++) {
    //引导码
		IR_Send38kHz(GreeHdrMark,DISABLE);
		IR_Send38kHz(GreeHdrSpace,ENABLE);
		//数据
		IR_SendMsg(3,ptcl.Mode);
		IR_SendMsg(1,ptcl.Power);
		IR_SendMsg(2,ptcl.Fan);
		IR_SendMsg(1,ptcl.SwingAuto);
		IR_SendMsg(1,ptcl.Sleep);
		
		IR_SendMsg(4,ptcl.Temp);
		IR_SendMsg(1,ptcl.TimerHalfHr);
		IR_SendMsg(2,ptcl.TimerTensHr);
		IR_SendMsg(1,ptcl.TimerEnabled);
		
		IR_SendMsg(4,ptcl.TimerHours);
		IR_SendMsg(1,ptcl.Turbo);
		IR_SendMsg(1,ptcl.Light);
		IR_SendMsg(1,ptcl.anion);
		IR_SendMsg(1,ptcl.Powersv);
		
		IR_SendMsg(2,ptcl.unknown0);
		IR_SendMsg(1,ptcl.TempExtraDegreeF);
		IR_SendMsg(1,ptcl.UseFahrenheit);
		IR_SendMsg(4,ptcl.unknown1);
////0x7C062050112000F0 
//      IR_SendMsg(8,0x7c);
//			IR_SendMsg(8,0x06);
//			IR_SendMsg(8,0x20);
//			IR_SendMsg(8,0x50);
			
		//footer
		IR_SendMsg(GreeBlockFooterBits,GreeBlockFooter);
		//连接码
		IR_Send38kHz(GreeBitMark,DISABLE);
		IR_Send38kHz(GreeMsgSpace,ENABLE);
		//后段数据
		IR_SendMsg(4,ptcl.SwingV);
		IR_SendMsg(3,ptcl.SwingH);
		IR_SendMsg(1,ptcl.unknown2);
		
		IR_SendMsg(2,ptcl.DisplayTemp);
		IR_SendMsg(1,ptcl.IFeel);
		IR_SendMsg(3,ptcl.unknown3);
		IR_SendMsg(1,ptcl.WiFi);
		IR_SendMsg(1,ptcl.unknown4);
		
		IR_SendMsg(8,ptcl.unknown5);
		
		IR_SendMsg(2,ptcl.unknown6);
		IR_SendMsg(1,ptcl.Econo);
		IR_SendMsg(1,ptcl.unknown7);
		IR_SendMsg(4,ptcl.Sum);
//      IR_SendMsg(8,0x11);
//			IR_SendMsg(8,0x20);
//			IR_SendMsg(8,0x00);
//			IR_SendMsg(8,0xF0);
		IR_Send38kHz(GreeBitMark,DISABLE);
		IR_Send38kHz(GreeRepSpace,ENABLE);
		//多次发送延迟
	
	}
}

// 修改Gree_SetPower函数
void Gree_SetPower(uint8_t state) {
    ptcl.Power = (state != 0) ? kGreeon : kGreeoff;
    // 关机时需要特殊处理
    if (state == kGreeoff) {
        ptcl.Mode = kGreeCool;    // 必须设置为有效模式（不能是Auto）
        ptcl.Fan = kGreeFanAuto;
        ptcl.Temp = 25 - kGreeMinTempC; // 设置默认温度
        ptcl.SwingAuto = 0;        // 关闭自动扫风
        ptcl.SwingV = kGreeSwingAuto; // 设置默认扫风模式
		ptcl.unknown1 = 0x05;         // 必须的协议固定值
    }
}


// 工作模式设置
void Gree_SetMode(uint8_t mode) {
    if (mode <= kGreeEcono) {
        ptcl.Mode = mode;
        // 温度范围保护
        if (mode == kGreeHeat && ptcl.Temp < 16) 
            Gree_SetTemperature(16);
        if (mode == kGreeCool && ptcl.Temp > 30)
            Gree_SetTemperature(30);
    }
}

// 风速设置
void Gree_SetFanSpeed(uint8_t speed) {
    if (speed <= kGreeFanMax) {
        ptcl.Fan = speed;
        // 自动模式关联处理
        if (speed == kGreeFanAuto)
            ptcl.SwingAuto = 1;
    }
}

// 温度设置（摄氏度）
void Gree_SetTemperature(uint8_t temp) {
    if (temp >= kGreeMinTempC && temp <= kGreeMaxTempC) {
        ptcl.Temp = temp - kGreeMinTempC;
        ptcl.UseFahrenheit = 0;  // 强制使用摄氏度
    }
}

// 修改Gree_UpdateIRSignal函数
void Gree_UpdateIRSignal(void) {
    // 格力协议特殊校验算法
    if (ptcl.Power == kGreeon) {
        ptcl.Sum = (ptcl.Mode - 1) + ptcl.Temp + 5 + ptcl.SwingH + ptcl.Econo;
    } else {
        // 关机时的特殊校验和
        ptcl.Sum = 0x0F & (0x0D + ptcl.Temp);
    }
    IR_SendGree(ptcl, 1);
}
