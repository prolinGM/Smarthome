#ifndef _ONENET_H_
#define _ONENET_H_

#define MQTT_TOPIC1 "smarthome/medicine"  

#include "stm32f10x.h"
#include "stm32f10x_flash.h"
#include "cJSON.h"
#include "mqttkit.h"
//Ëã·¨
#include "base64.h"
#include "hmac_sha1.h"
//Ó²¼þÇý¶¯
#include "usart.h"
#include "delay.h"
//C¿â
#include <string.h>
#include <stdio.h>

_Bool OneNET_RegisterDevice(void);

_Bool OneNet_DevLink(void);

void OneNet_Subscribe(const char *topics[], unsigned char topic_cnt);

void OneNet_Publish(const char *topic, const char *msg);

void OneNet_RevPro(unsigned char *cmd);

void handleLivingRoomLED(cJSON *json); 
void handleBedroomLED(cJSON *json);
void handleStudyroomLED(cJSON *json);
void handleKitchenLED(cJSON *json);
void handleBathroomLED(cJSON *json);

void handleBedroomCurtainRaw(uint8_t raw_cmd);
void handleAC(cJSON *json);

void HandleLivingLEDAuto(void);
void HandleBedLEDAuto(void);
void HandleStudyLEDAuto(void);
void HandleKitLEDAuto(void);
void HandleBathLEDAuto(void);

#endif
