#ifndef __IRREMOT_H__
#define __IRREMOT_H__
#include "stm32f10x.h"

// 常量声明（外部可访问）

extern const uint8_t kGreeAuto;
extern const uint8_t kGreeCool;
extern const uint8_t kGreeDry;
extern const uint8_t kGreeFan;
extern const uint8_t kGreeHeat;
extern const uint8_t kGreeEcono;
extern const uint8_t kGreeon;
extern const uint8_t kGreeoff;
extern const uint8_t kGreeFanAuto;
extern const uint8_t kGreeFanMin;
extern const uint8_t kGreeFanMed;
extern const uint8_t kGreeFanMax;
extern const uint8_t kGreeMinTempC;
extern const uint8_t kGreeMaxTempC;
extern const uint8_t kGreeSwingLastPos;
extern const uint8_t kGreeSwingAuto;
extern const uint8_t kGreeSwingUp;
extern const uint8_t kGreeSwingMiddleUp;
extern const uint8_t kGreeSwingMiddle;
extern const uint8_t kGreeSwingMiddleDown;
extern const uint8_t kGreeSwingDown;
extern const uint8_t kGreeSwingDownAuto;
extern const uint8_t kGreeSwingMiddleAuto;
extern const uint8_t kGreeSwingUpAuto;

typedef struct{
    // Byte 0
    uint8_t Mode      :3;	 //模式
    uint8_t Power     :1;    //开关
    uint8_t Fan       :2;    //风速
    uint8_t SwingAuto :1;    //自动扫风
    uint8_t Sleep     :1;    // 睡眠
    // Byte 1
    uint8_t Temp        :4;  // 温度
    uint8_t TimerHalfHr :1;
    uint8_t TimerTensHr :2;
    uint8_t TimerEnabled:1;
    // Byte 2
    uint8_t TimerHours:4;
    uint8_t Turbo     :1;     // 强劲
    uint8_t Light     :1;     //灯光
    uint8_t anion     :1;     //model==YAW1F
    uint8_t Powersv   :1;
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
    uint8_t Sum         :4;
  }Protocol;
//红外码初始化
extern Protocol ptcl;
//// Constants
//extern const uint8_t GreeAuto  = 0;
//extern const uint8_t GreeCool  = 1;
//extern const uint8_t GreeDry   = 2;
//extern const uint8_t GreeFan   = 3;
//extern const uint8_t GreeHeat  = 4;
//extern const uint8_t GreeEcono = 5;

//extern const uint8_t GreeFanAuto = 0;
//extern const uint8_t GreeFanMin  = 1;
//extern const uint8_t GreeFanMed  = 2;
//extern const uint8_t GreeFanMax  = 3;

//extern const uint8_t GreeMinTempC = 16;  // Celsius
//extern const uint8_t GreeMaxTempC = 30;  // Celsius
//extern const uint8_t GreeMinTempF = 61;  // Fahrenheit
//extern const uint8_t GreeMaxTempF = 86;  // Fahrenheit
//extern const uint16_t GreeTimerMax = 24 * 60;//

//extern const uint8_t GreeSwingLastPos    = 0 ;  // 0B0000
//extern const uint8_t GreeSwingAuto       = 1 ;  // 0B0001
//extern const uint8_t GreeSwingUp         = 2 ;  // 0B0010
//extern const uint8_t GreeSwingMiddleUp   = 3 ;  // 0B0011
//extern const uint8_t GreeSwingMiddle     = 4 ;  // 0B0100
//extern const uint8_t GreeSwingMiddleDown = 5 ;  // 0B0101
//extern const uint8_t GreeSwingDown       = 6 ;  // 0B0110
//extern const uint8_t GreeSwingDownAuto   = 7 ;  // 0B0111
//extern const uint8_t GreeSwingMiddleAuto = 9 ;  // 0B01001
//extern const uint8_t GreeSwingUpAuto     = 11;  // 0B01011

//extern const uint8_t GreeSwingHOff        = 0;  // 0B000
//extern const uint8_t GreeSwingHAuto       = 1;  // 0B001
//extern const uint8_t GreeSwingHMaxLeft    = 2;  // 0B010
//extern const uint8_t GreeSwingHLeft       = 3;  // 0B011
//extern const uint8_t GreeSwingHMiddle     = 4;  // 0B0100
//extern const uint8_t GreeSwingHRight      = 5;  // 0B0101
//extern const uint8_t GreeSwingHMaxRight   = 6;  // 0B0110

//extern const uint8_t GreeDisplayTempOff     = 0;  // 0B00
//extern const uint8_t GreeDisplayTempSet     = 1;  // 0B01
//extern const uint8_t GreeDisplayTempInside  = 2;  //0B010
//extern const uint8_t GreeDisplayTempOutside = 3;  // 0B011

// Legacy defines.
#define GREE_AUTO   kGreeAuto
#define GREE_COOL   kGreeCool
#define GREE_DRY   kGreeDry
#define GREE_FAN   kGreeFan
#define GREE_HEAT   kGreeHeat
#define GREE_MIN_TEMP   kGreeMinTempC
#define GREE_MAX_TEMP   kGreeMaxTempC
#define GREE_FAN_MAX   kGreeFanMax
#define GREE_SWING_LAST_POS   kGreeSwingLastPos
#define GREE_SWING_AUTO    kGreeSwingAuto
#define GREE_SWING_UP   kGreeSwingUp
#define GREE_SWING_MIDDLE_UP   kGreeSwingMiddleUp
#define GREE_SWING_MIDDLE   kGreeSwingMiddle
#define GREE_SWING_MIDDLE_DOWN   kGreeSwingMiddleDown
#define GREE_SWING_DOWN     kGreeSwingDown
#define GREE_SWING_DOWN_AUTO   kGreeSwingDownAuto
#define GREE_SWING_MIDDLE_AUTO  kGreeSwingMiddleAuto
#define GREE_SWING_UP_AUTO   kGreeSwingUpAuto



void IR_init(void);
void IR_SendGree(Protocol ptcl,u16 repeat);
void IR_Send38kHz(u16 time,FunctionalState);
void IR_SendGreeH(void);
void IR_SendGreeL(void);

// 新增控制API
void Gree_SetPower(uint8_t state);
void Gree_SetMode(uint8_t mode);
void Gree_SetFanSpeed(uint8_t speed);
void Gree_SetTemperature(uint8_t temp);
void Gree_SetVerticalSwing(uint8_t mode);
void Gree_SetHorizontalSwing(uint8_t mode);
void Gree_EnableEcono(uint8_t enable);
void Gree_EnableTurbo(uint8_t enable);
void Gree_UpdateIRSignal(void);



#endif
