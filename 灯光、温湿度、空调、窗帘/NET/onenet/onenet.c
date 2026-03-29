//单片机头文件
#include "stm32f10x.h"

//网络设备
#include "esp8266.h"

//协议文件
#include "onenet.h"

#define PROID			"admin"

#define ACCESS_KEY		"13161543463@Lgm"

#define DEVICE_NAME		"emqx_MjIwNT"

#define EEPROM_ADDR 0x08080000  // STM32F103C8T6用户数据区地址

char devid[16];

char key[48];

/*
************************************************************
*	函数名称：	OTA_UrlEncode
*
*	函数功能：	sign需要进行URL编码
*
*	入口参数：	sign：加密结果
*
*	返回参数：	0-成功	其他-失败
*
*	说明：		+			%2B
*				空格		%20
*				/			%2F
*				?			%3F
*				%			%25
*				#			%23
*				&			%26
*				=			%3D
************************************************************
*/
static unsigned char OTA_UrlEncode(char *sign)
{

	char sign_t[40];
	unsigned char i = 0, j = 0;
	unsigned char sign_len = strlen(sign);
	
	if(sign == (void *)0 || sign_len < 28)
		return 1;
	
	for(; i < sign_len; i++)
	{
		sign_t[i] = sign[i];
		sign[i] = 0;
	}
	sign_t[i] = 0;
	
	for(i = 0, j = 0; i < sign_len; i++)
	{
		switch(sign_t[i])
		{
			case '+':
				strcat(sign + j, "%2B");j += 3;
			break;
			
			case ' ':
				strcat(sign + j, "%20");j += 3;
			break;
			
			case '/':
				strcat(sign + j, "%2F");j += 3;
			break;
			
			case '?':
				strcat(sign + j, "%3F");j += 3;
			break;
			
			case '%':
				strcat(sign + j, "%25");j += 3;
			break;
			
			case '#':
				strcat(sign + j, "%23");j += 3;
			break;
			
			case '&':
				strcat(sign + j, "%26");j += 3;
			break;
			
			case '=':
				strcat(sign + j, "%3D");j += 3;
			break;
			
			default:
				sign[j] = sign_t[i];j++;
			break;
		}
	}
	
	sign[j] = 0;
	
	return 0;

}

/*
************************************************************
*	函数名称：	OTA_Authorization
*
*	函数功能：	计算Authorization
*
*	入口参数：	ver：参数组版本号，日期格式，目前仅支持格式"2018-10-31"
*				res：产品id
*				et：过期时间，UTC秒值
*				access_key：访问密钥
*				dev_name：设备名
*				authorization_buf：缓存token的指针
*				authorization_buf_len：缓存区长度(字节)
*
*	返回参数：	0-成功	其他-失败
*
*	说明：		当前仅支持sha1
************************************************************
*/
#define METHOD		"sha1"
static unsigned char OneNET_Authorization(char *ver, char *res, unsigned int et, char *access_key, char *dev_name,
											char *authorization_buf, unsigned short authorization_buf_len, _Bool flag)
{
	
	size_t olen = 0;
	
	char sign_buf[64];								//保存签名的Base64编码结果 和 URL编码结果
	char hmac_sha1_buf[64];							//保存签名
	char access_key_base64[64];						//保存access_key的Base64编码结合
	char string_for_signature[72];					//保存string_for_signature，这个是加密的key

//----------------------------------------------------参数合法性--------------------------------------------------------------------
	if(ver == (void *)0 || res == (void *)0 || et < 1564562581 || access_key == (void *)0
		|| authorization_buf == (void *)0 || authorization_buf_len < 120)
		return 1;
	
//----------------------------------------------------将access_key进行Base64解码----------------------------------------------------
	memset(access_key_base64, 0, sizeof(access_key_base64));
	BASE64_Decode((unsigned char *)access_key_base64, sizeof(access_key_base64), &olen, (unsigned char *)access_key, strlen(access_key));
//	UsartPrintf(USART_DEBUG, "access_key_base64: %s\r\n", access_key_base64);
	
//----------------------------------------------------计算string_for_signature-----------------------------------------------------
	memset(string_for_signature, 0, sizeof(string_for_signature));
	if(flag)
		snprintf(string_for_signature, sizeof(string_for_signature), "%d\n%s\nproducts/%s\n%s", et, METHOD, res, ver);
	else
		snprintf(string_for_signature, sizeof(string_for_signature), "%d\n%s\nproducts/%s/devices/%s\n%s", et, METHOD, res, dev_name, ver);
//	UsartPrintf(USART_DEBUG, "string_for_signature: %s\r\n", string_for_signature);
	
//----------------------------------------------------加密-------------------------------------------------------------------------
	memset(hmac_sha1_buf, 0, sizeof(hmac_sha1_buf));
	
	hmac_sha1((unsigned char *)access_key_base64, strlen(access_key_base64),
				(unsigned char *)string_for_signature, strlen(string_for_signature),
				(unsigned char *)hmac_sha1_buf);
	
//	UsartPrintf(USART_DEBUG, "hmac_sha1_buf: %s\r\n", hmac_sha1_buf);
	
//----------------------------------------------------将加密结果进行Base64编码------------------------------------------------------
	olen = 0;
	memset(sign_buf, 0, sizeof(sign_buf));
	BASE64_Encode((unsigned char *)sign_buf, sizeof(sign_buf), &olen, (unsigned char *)hmac_sha1_buf, strlen(hmac_sha1_buf));

//----------------------------------------------------将Base64编码结果进行URL编码---------------------------------------------------
	OTA_UrlEncode(sign_buf);
//	UsartPrintf(USART_DEBUG, "sign_buf: %s\r\n", sign_buf);
	
//----------------------------------------------------计算Token--------------------------------------------------------------------
	if(flag)
		snprintf(authorization_buf, authorization_buf_len, "version=%s&res=products%%2F%s&et=%d&method=%s&sign=%s", ver, res, et, METHOD, sign_buf);
	else
		snprintf(authorization_buf, authorization_buf_len, "version=%s&res=products%%2F%s%%2Fdevices%%2F%s&et=%d&method=%s&sign=%s", ver, res, dev_name, et, METHOD, sign_buf);
//	UsartPrintf(USART_DEBUG, "Token: %s\r\n", authorization_buf);
	
	return 0;

}

//==========================================================
//	函数名称：	OneNET_RegisterDevice
//
//	函数功能：	在产品中注册一个设备
//
//	入口参数：	access_key：访问密钥
//				pro_id：产品ID
//				serial：唯一设备号
//				devid：保存返回的devid
//				key：保存返回的key
//
//	返回参数：	0-成功		1-失败
//
//	说明：		
//==========================================================
_Bool OneNET_RegisterDevice(void)
{

	_Bool result = 1;
	unsigned short send_len = 11 + strlen(DEVICE_NAME);
	char *send_ptr = NULL, *data_ptr = NULL;
	
	char authorization_buf[144];													//加密的key
	
	send_ptr = malloc(send_len + 240);
	if(send_ptr == NULL)
		return result;
	
	while(ESP8266_SendCmd("AT+CIPSTART=\"TCP\",\"183.230.40.33\",80\r\n", "CONNECT"))
		Delay_ms(500);
	
	OneNET_Authorization("2018-10-31", PROID, 1956499200, ACCESS_KEY, NULL,
							authorization_buf, sizeof(authorization_buf), 1);
	
	snprintf(send_ptr, 240 + send_len, "POST /mqtt/v1/devices/reg HTTP/1.1\r\n"
					"Authorization:%s\r\n"
					"Host:ota.heclouds.com\r\n"
					"Content-Type:application/json\r\n"
					"Content-Length:%d\r\n\r\n"
					"{\"name\":\"%s\"}",
	
					authorization_buf, 11 + strlen(DEVICE_NAME), DEVICE_NAME);
	
	ESP8266_SendData((unsigned char *)send_ptr, strlen(send_ptr));
	
	/*
	{
	  "request_id" : "f55a5a37-36e4-43a6-905c-cc8f958437b0",
	  "code" : "onenet_common_success",
	  "code_no" : "000000",
	  "message" : null,
	  "data" : {
		"device_id" : "589804481",
		"name" : "mcu_id_43057127",
		
	"pid" : 282932,
		"key" : "indu/peTFlsgQGL060Gp7GhJOn9DnuRecadrybv9/XY="
	  }
	}
	*/
	
	data_ptr = (char *)ESP8266_GetIPD(250);							//等待平台响应
	
	if(data_ptr)
	{
		data_ptr = strstr(data_ptr, "device_id");
	}
	
	if(data_ptr)
	{
		char name[16];
		int pid = 0;
		
		if(sscanf(data_ptr, "device_id\" : \"%[^\"]\",\r\n\"name\" : \"%[^\"]\",\r\n\r\n\"pid\" : %d,\r\n\"key\" : \"%[^\"]\"", devid, name, &pid, key) == 4)
		{
			UsartPrintf(USART_DEBUG, "create device: %s, %s, %d, %s\r\n", devid, name, pid, key);
			result = 0;
		}
	}
	
	free(send_ptr);
	ESP8266_SendCmd("AT+CIPCLOSE\r\n", "OK");
	
	return result;

}

//==========================================================
//	函数名称：	OneNet_DevLink
//
//	函数功能：	与onenet创建连接
//
//	入口参数：	无
//
//	返回参数：	1-成功	0-失败
//
//	说明：		与onenet平台建立连接
//==========================================================
_Bool OneNet_DevLink(void)
{
	
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};					//协议包

	unsigned char *dataPtr;
	
	char authorization_buf[160];
	
	_Bool status = 1;
	
	OneNET_Authorization("2018-10-31", PROID, 1956499200, ACCESS_KEY, DEVICE_NAME,
								authorization_buf, sizeof(authorization_buf), 0);
	
	UsartPrintf(USART_DEBUG, "OneNET_DevLink\r\n"
							"NAME: %s,	PROID: %s,	KEY:%s\r\n"
                        , DEVICE_NAME, PROID, authorization_buf);
	
	if(MQTT_PacketConnect(PROID, authorization_buf, DEVICE_NAME, 256, 1, MQTT_QOS_LEVEL0, NULL, NULL, 0, &mqttPacket) == 0)
	{
		ESP8266_SendData(mqttPacket._data, mqttPacket._len);			//上传平台
		
		dataPtr = ESP8266_GetIPD(250);									//等待平台响应
		if(dataPtr != NULL)
		{
			if(MQTT_UnPacketRecv(dataPtr) == MQTT_PKT_CONNACK)
			{
				switch(MQTT_UnPacketConnectAck(dataPtr))
				{
					case 0:UsartPrintf(USART_DEBUG, "Tips:	连接成功\r\n");status = 0;break;
					
					case 1:UsartPrintf(USART_DEBUG, "WARN:	连接失败：协议错误\r\n");break;
					case 2:UsartPrintf(USART_DEBUG, "WARN:	连接失败：非法的clientid\r\n");break;
					case 3:UsartPrintf(USART_DEBUG, "WARN:	连接失败：服务器失败\r\n");break;
					case 4:UsartPrintf(USART_DEBUG, "WARN:	连接失败：用户名或密码错误\r\n");break;
					case 5:UsartPrintf(USART_DEBUG, "WARN:	连接失败：非法链接(比如token非法)\r\n");break;
					
					default:UsartPrintf(USART_DEBUG, "ERR:	连接失败：未知错误\r\n");break;
				}
			}
		}
		
		MQTT_DeleteBuffer(&mqttPacket);								//删包
	}
	else
		UsartPrintf(USART_DEBUG, "WARN:	MQTT_PacketConnect Failed\r\n");
	
	return status;
	
}


//==========================================================
//	函数名称：	OneNET_Publish
//
//	函数功能：	发布消息
//
//	入口参数：	topic：发布的主题
//				msg：消息内容
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void OneNET_Publish(const char *topic, const char *msg) {
    MQTT_PACKET_STRUCTURE mqtt_packet = {NULL, 0, 0, 0};
    char payload[512]; // 增大缓冲区

    snprintf(payload, sizeof(payload), "%s", msg); // 使用安全格式化
	
	UsartPrintf(USART_DEBUG, "MQTT PUBLISH: Topic=\"%s\" Payload=\"%s\"\r", topic, payload);
	
    if (MQTT_PacketPublish(MQTT_PUBLISH_ID, topic, payload, strlen(payload), 
                         MQTT_QOS_LEVEL0, 1, 1, &mqtt_packet) == 0) {
        ESP8266_SendData(mqtt_packet._data, mqtt_packet._len);
        MQTT_DeleteBuffer(&mqtt_packet); // 释放缓冲区
							 
    } else {
        UsartPrintf(USART_DEBUG, "MQTT_Publish Failed\r\n");
    }
}
//==========================================================
//	函数名称：	OneNet_Subscribe
//
//	函数功能：	订阅
//
//	入口参数：	topics：订阅的topic
//				topic_cnt：topic个数
//
//	返回参数：	SEND_TYPE_OK-成功	SEND_TYPE_SUBSCRIBE-需要重发
//
//	说明：		
//==========================================================
//void OneNet_Subscribe(const char *topics[], unsigned char topic_cnt)
//{
//	
//	unsigned char i = 0;
//	
//	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};							//协议包
//	
//	for(; i < topic_cnt; i++)
//		UsartPrintf(USART_DEBUG, "Subscribe Topic: %s\r\n", topics[i]);
//	
//	if(MQTT_PacketSubscribe(MQTT_SUBSCRIBE_ID, MQTT_QOS_LEVEL0, topics, topic_cnt, &mqttPacket) == 0)
//	{
//		ESP8266_SendData(mqttPacket._data, mqttPacket._len);					//向平台发送订阅请求
//		
//		MQTT_DeleteBuffer(&mqttPacket);											//删包
//	}
//	
//}

void OneNet_Subscribe(const char *topics[], unsigned char topic_cnt) {
    unsigned char i = 0;
    MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};                          

    if(MQTT_PacketSubscribe(MQTT_SUBSCRIBE_ID, MQTT_QOS_LEVEL0, topics, topic_cnt, &mqttPacket) == 0) {
        ESP8266_SendData(mqttPacket._data, mqttPacket._len);
        MQTT_DeleteBuffer(&mqttPacket);
        UsartPrintf(USART_DEBUG, "Subscribed to all topics successfully.\r");
        
        // 添加延时确保订阅完成
        Delay_ms(200); 

        // 发送 INIT 时增加间隔
        for(i = 0; i < topic_cnt; i++) {
            UsartPrintf(USART_DEBUG, "Sending INIT to topic: %s\r", topics[i]);
            OneNET_Publish(topics[i], "init");
            Delay_ms(200);  // 增加 200ms 延时
        }
    } else {
        UsartPrintf(USART_DEBUG, "MQTT_Subscribe Failed\r");
    }
}


//==========================================================
//	函数名称：	OneNet_RevPro
//
//	函数功能：	平台返回数据检测
//
//	入口参数：	dataPtr：平台返回的数据
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void OneNet_RevPro(unsigned char *cmd)
{
	
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};								//协议包
	
	char *req_payload = NULL;
	char *cmdid_topic = NULL;
	
	unsigned short topic_len = 0;
	unsigned short req_len = 0;
	
	unsigned char type = 0;
	unsigned char qos = 0;
	static unsigned short pkt_id = 0;
	
	short result = 0;

	char *dataPtr = NULL;
	char numBuf[10];
	int num = 0;
	cJSON *json;
	type = MQTT_UnPacketRecv(cmd);
	switch(type)
	{
		case MQTT_PKT_CMD:															//命令下发
			
			result = MQTT_UnPacketCmd(cmd, &cmdid_topic, &req_payload, &req_len);	//解出topic和消息体
			if(result == 0)
			{
				UsartPrintf(USART_DEBUG, "cmdid: %s, req: %s, req_len: %d\r\n", cmdid_topic, req_payload, req_len);
				
				//if(MQTT_PacketCmdResp(cmdid_topic, req_payload, &mqttPacket) == 0)	//命令回复组包
				//{
					//UsartPrintf(USART_DEBUG, "Tips:	Send CmdResp\r\n");
					
					//ESP8266_SendData(mqttPacket._data, mqttPacket._len);			//回复命令

				MQTT_DeleteBuffer(&mqttPacket);									//删包
				//}
			}
		
		break;
			
		case MQTT_PKT_PUBLISH:														//接收的Publish消息
		
			result = MQTT_UnPacketPublish(cmd, &cmdid_topic, &topic_len, &req_payload, &req_len, &qos, &pkt_id);
			if(result == 0)
			{
				UsartPrintf(USART_DEBUG, "Received on topic: %s\n", cmdid_topic);
				json=cJSON_Parse(req_payload);
				if (!json) {UsartPrintf(USART_DEBUG, "JSON parse error\r\n");}
				else
				{
					// 根据不同主题处理数据
					if (strcmp(cmdid_topic, MQTT_TOPIC1) == 0) { // livingroom LED
						handleLivingRoomLED(json);} 
					else if (strcmp(cmdid_topic, MQTT_TOPIC2) == 0) { // bedroom LED
						handleBedroomLED(json);} 
					else if (strcmp(cmdid_topic, MQTT_TOPIC3) == 0) { // studyroom LED
						handleStudyroomLED(json);} 
					else if (strcmp(cmdid_topic, MQTT_TOPIC4) == 0) { // kitchen LED
						handleKitchenLED(json);} 
					else if (strcmp(cmdid_topic, MQTT_TOPIC5) == 0) { // bathroom LED
						handleBathroomLED(json);} 
					else if (strcmp(cmdid_topic, MQTT_TOPIC6) == 0) { // bedroom curtain
							// 直接传递原始数值（无需JSON解析）
							int raw_cmd = atoi((char*)req_payload);
							handleBedroomCurtainRaw(raw_cmd);}
					else if (strcmp(cmdid_topic, MQTT_TOPIC7) == 0) { // ac
						handleAC(json);}
				cJSON_Delete(json);
				}
			}
		
		break;
			
		case MQTT_PKT_PUBACK:														//发送Publish消息，平台回复的Ack
		
			if(MQTT_UnPacketPublishAck(cmd) == 0)
				UsartPrintf(USART_DEBUG, "Tips:	MQTT Publish Send OK\r\n");
			
		break;
			
		case MQTT_PKT_PUBREC:														//发送Publish消息，平台回复的Rec，设备需回复Rel消息
		
			if(MQTT_UnPacketPublishRec(cmd) == 0)
			{
				UsartPrintf(USART_DEBUG, "Tips:	Rev PublishRec\r\n");
				if(MQTT_PacketPublishRel(MQTT_PUBLISH_ID, &mqttPacket) == 0)
				{
					UsartPrintf(USART_DEBUG, "Tips:	Send PublishRel\r\n");
					ESP8266_SendData(mqttPacket._data, mqttPacket._len);
					MQTT_DeleteBuffer(&mqttPacket);
				}
			}
		
		break;
			
		case MQTT_PKT_PUBREL:														//收到Publish消息，设备回复Rec后，平台回复的Rel，设备需再回复Comp
			
			if(MQTT_UnPacketPublishRel(cmd, pkt_id) == 0)
			{
				UsartPrintf(USART_DEBUG, "Tips:	Rev PublishRel\r\n");
				if(MQTT_PacketPublishComp(MQTT_PUBLISH_ID, &mqttPacket) == 0)
				{
					UsartPrintf(USART_DEBUG, "Tips:	Send PublishComp\r\n");
					ESP8266_SendData(mqttPacket._data, mqttPacket._len);
					MQTT_DeleteBuffer(&mqttPacket);
				}
			}
		
		break;
		
		case MQTT_PKT_PUBCOMP:														//发送Publish消息，平台返回Rec，设备回复Rel，平台再返回的Comp
		
			if(MQTT_UnPacketPublishComp(cmd) == 0)
			{
				UsartPrintf(USART_DEBUG, "Tips:	Rev PublishComp\r\n");
			}
		
		break;
			
		case MQTT_PKT_SUBACK:														//发送Subscribe消息的Ack
		
			if(MQTT_UnPacketSubscribe(cmd) == 0)
			{
				UsartPrintf(USART_DEBUG, "Tips:	MQTT Subscribe OK\r\n");	
			}
			else
				UsartPrintf(USART_DEBUG, "Tips:	MQTT Subscribe Err\r\n");
		
		break;
			
		case MQTT_PKT_UNSUBACK:														//发送UnSubscribe消息的Ack
		
			if(MQTT_UnPacketUnSubscribe(cmd) == 0)
				UsartPrintf(USART_DEBUG, "Tips:	MQTT UnSubscribe OK\r\n");
			else
				UsartPrintf(USART_DEBUG, "Tips:	MQTT UnSubscribe Err\r\n");
		
		break;
		
		default:
			result = -1;
		break;
	}
	
	ESP8266_Clear();									//清空缓存
	
	if(result == -1)
		return;
	
	dataPtr = strchr(req_payload, '}');					//搜索'}'

	if(dataPtr != NULL && result != -1)					//如果找到了
	{
		dataPtr++;
		
		while(*dataPtr >= '0' && *dataPtr <= '9')		//判断是否是下发的命令控制数据
		{
			numBuf[num++] = *dataPtr++;
		}
		
		num = atoi((const char *)numBuf);				//转为数值形式
		
	}

	if(type == MQTT_PKT_CMD || type == MQTT_PKT_PUBLISH)
	{
		MQTT_FreeBuffer(cmdid_topic);
		MQTT_FreeBuffer(req_payload);
	}

}

//LivingRoomLED处理函数
void handleLivingRoomLED(cJSON *json) {
    static int current_brightness = 100;
    static int current_temp = 4000;
    static int is_light_on = 0;
	
	if (!json) return;
	
	// 解析Auto指令
	cJSON *auto_json = cJSON_GetObjectItem(json, "Auto");
	if (auto_json && auto_json->type == cJSON_Number) {
		living_auto = auto_json->valueint;  // 更新全局模式
		if (living_auto) {
			// 进入自动模式时关闭灯光
			is_light_on = 0;
			WS2812B_SetBrightness_liv(0);
			WS2812B_SetTemperature_liv(current_temp);
			UsartPrintf(USART_DEBUG, "System Auto Mode: %d", living_auto);
		}else {
			UsartPrintf(USART_DEBUG, "Auto Mode Disabled");
		}
	}
	
    // 处理手动开关指令（独立处理）
    if (!living_auto && json->type == cJSON_Number) {
        int raw_cmd = json->valueint;
        if (raw_cmd == 1) {
            is_light_on = 1;
			Delay_ms(50);
            WS2812B_SetBrightness_liv(current_brightness);
			Delay_ms(50);
            WS2812B_SetTemperature_liv(current_temp);
            UsartPrintf(USART_DEBUG, "Livingroom LED ON");
        } else {
            is_light_on = 0;
            WS2812B_SetBrightness_liv(0);
            UsartPrintf(USART_DEBUG, "Livingroom LED OFF");
        }
    }

    // 处理色温调节指令（全模式生效）
    if (1) {  // 全局生效的色温处理
        cJSON *temp_json = cJSON_GetObjectItem(json, "Temperature");
        if (temp_json && temp_json->type == cJSON_Number) {
            int new_temp = temp_json->valueint;
            if (new_temp >= 2000 && new_temp <= 6000) {
                current_temp = new_temp;
                if (is_light_on) {
                    WS2812B_SetTemperature_liv(current_temp);
                }
                if (living_auto) {
                    WS2812B_SetTemperature_liv(current_temp);
                }
                UsartPrintf(USART_DEBUG, "Temperature updated to %dk", new_temp);
            }
        }
    }

    // 处理亮度调节指令（手动模式+灯亮时生效）
    if (!living_auto && is_light_on) {  // 手动模式且灯亮时处理亮度
        cJSON *brightness_json = cJSON_GetObjectItem(json, "Brightness");
        if (brightness_json && brightness_json->type == cJSON_Number) {
            int new_brightness = brightness_json->valueint;
            if (new_brightness >= 0 && new_brightness <= 100) {
                current_brightness = new_brightness;
                WS2812B_SetBrightness_liv(current_brightness);
                UsartPrintf(USART_DEBUG, "Brightness adjusted to %d%%", new_brightness);
            }
        }
    }
}

//BedroomLED处理函数
void handleBedroomLED(cJSON *json) {
    static int current_brightness = 100;
    static int current_temp = 4000;
    static int is_light_on = 0;
	
	if (!json) return;
	
	// 解析Auto指令
	cJSON *auto_json = cJSON_GetObjectItem(json, "Auto");
	if (auto_json && auto_json->type == cJSON_Number) {
		bed_auto = auto_json->valueint;  // 更新全局模式
		if (bed_auto) {
			// 进入自动模式时关闭灯光
			is_light_on = 0;
			WS2812B_SetBrightness_bed(0);
			WS2812B_SetTemperature_bed(current_temp);
			UsartPrintf(USART_DEBUG, "System Auto Mode: %d", bed_auto);
		}else {
			UsartPrintf(USART_DEBUG, "Auto Mode Disabled");
		}
	}
	
    // 处理手动开关指令（独立处理）
    if (!bed_auto && json->type == cJSON_Number) {
        int raw_cmd = json->valueint;
        if (raw_cmd == 1) {
            is_light_on = 1;
			Delay_ms(50);
            WS2812B_SetBrightness_bed(current_brightness);
			Delay_ms(50);
            WS2812B_SetTemperature_bed(current_temp);
            UsartPrintf(USART_DEBUG, "Bedroom LED ON");
        } else {
            is_light_on = 0;
            WS2812B_SetBrightness_bed(0);
            UsartPrintf(USART_DEBUG, "Bedroom LED OFF");
        }
    }

    // 处理色温调节指令（全模式生效）
    if (1) {  // 全局生效的色温处理
        cJSON *temp_json = cJSON_GetObjectItem(json, "Temperature");
        if (temp_json && temp_json->type == cJSON_Number) {
            int new_temp = temp_json->valueint;
            if (new_temp >= 2000 && new_temp <= 6000) {
                current_temp = new_temp;
                if (is_light_on) {
                    WS2812B_SetTemperature_bed(current_temp);
                }
                if (bed_auto) {
                    WS2812B_SetTemperature_bed(current_temp);
                }
                UsartPrintf(USART_DEBUG, "Temperature updated to %dk", new_temp);
            }
        }
    }

    // 处理亮度调节指令（手动模式+灯亮时生效）
    if (!bed_auto && is_light_on) {  // 手动模式且灯亮时处理亮度
        cJSON *brightness_json = cJSON_GetObjectItem(json, "Brightness");
        if (brightness_json && brightness_json->type == cJSON_Number) {
            int new_brightness = brightness_json->valueint;
            if (new_brightness >= 0 && new_brightness <= 100) {
                current_brightness = new_brightness;
                WS2812B_SetBrightness_bed(current_brightness);
                UsartPrintf(USART_DEBUG, "Brightness adjusted to %d%%", new_brightness);
            }
        }
    }
}


//StudyroomLED处理函数
void handleStudyroomLED(cJSON *json) {
    static int current_brightness = 100;
    static int current_temp = 4000;
    static int is_light_on = 0;
	
	if (!json) return;
	
	// 解析Auto指令
	cJSON *auto_json = cJSON_GetObjectItem(json, "Auto");
	if (auto_json && auto_json->type == cJSON_Number) {
		study_auto = auto_json->valueint;  // 更新全局模式
		if (study_auto) {
			// 进入自动模式时关闭灯光
			is_light_on = 0;
			WS2812B_SetBrightness_stu(0);
			WS2812B_SetTemperature_stu(current_temp);
			UsartPrintf(USART_DEBUG, "System Auto Mode: %d", study_auto);
		}else {
			UsartPrintf(USART_DEBUG, "Auto Mode Disabled");
		}
	}
	
    // 处理手动开关指令（独立处理）
    if (!study_auto && json->type == cJSON_Number) {
        int raw_cmd = json->valueint;
        if (raw_cmd == 1) {
            is_light_on = 1;
            WS2812B_SetBrightness_stu(current_brightness);
			Delay_ms(50);
            WS2812B_SetTemperature_stu(current_temp);
			Delay_ms(50);
            UsartPrintf(USART_DEBUG, "Studyroom LED ON");
        } else {
            is_light_on = 0;
            WS2812B_SetBrightness_stu(0);
            UsartPrintf(USART_DEBUG, "Studyroom LED OFF");
        }
    }

    // 处理色温调节指令（全模式生效）
    if (1) {  // 全局生效的色温处理
        cJSON *temp_json = cJSON_GetObjectItem(json, "Temperature");
        if (temp_json && temp_json->type == cJSON_Number) {
            int new_temp = temp_json->valueint;
            if (new_temp >= 2000 && new_temp <= 6000) {
                current_temp = new_temp;
                if (is_light_on) {
                    WS2812B_SetTemperature_stu(current_temp);
                }
                if (study_auto) {
                    WS2812B_SetTemperature_stu(current_temp);
                }
                UsartPrintf(USART_DEBUG, "Temperature updated to %dk", new_temp);
            }
        }
    }

    // 处理亮度调节指令（手动模式+灯亮时生效）
    if (!study_auto && is_light_on) {  // 手动模式且灯亮时处理亮度
        cJSON *brightness_json = cJSON_GetObjectItem(json, "Brightness");
        if (brightness_json && brightness_json->type == cJSON_Number) {
            int new_brightness = brightness_json->valueint;
            if (new_brightness >= 0 && new_brightness <= 100) {
                current_brightness = new_brightness;
                WS2812B_SetBrightness_stu(current_brightness);
                UsartPrintf(USART_DEBUG, "Brightness adjusted to %d%%", new_brightness);
            }
        }
    }
}

//KitchenLED处理函数
void handleKitchenLED(cJSON *json) {
    static int current_brightness = 100;
    static int current_temp = 4000;
    static int is_light_on = 0;
	
	if (!json) return;
	
	// 解析Auto指令
	cJSON *auto_json = cJSON_GetObjectItem(json, "Auto");
	if (auto_json && auto_json->type == cJSON_Number) {
		kit_auto = auto_json->valueint;  // 更新全局模式
		if (kit_auto) {
			// 进入自动模式时关闭灯光
			is_light_on = 0;
			WS2812B_SetBrightness_kit(0);
			WS2812B_SetTemperature_kit(current_temp);
			UsartPrintf(USART_DEBUG, "System Auto Mode: %d", kit_auto);
		}else {
			UsartPrintf(USART_DEBUG, "Auto Mode Disabled");
		}
	}
	
    // 处理手动开关指令（独立处理）
    if (!kit_auto && json->type == cJSON_Number) {
        int raw_cmd = json->valueint;
        if (raw_cmd == 1) {
            is_light_on = 1;
			Delay_ms(50);
            WS2812B_SetBrightness_kit(current_brightness);
			Delay_ms(50);
            WS2812B_SetTemperature_kit(current_temp);
            UsartPrintf(USART_DEBUG, "Kitchen LED ON");
        } else {
            is_light_on = 0;
            WS2812B_SetBrightness_kit(0);
            UsartPrintf(USART_DEBUG, "Kitchen LED OFF");
        }
    }

    // 处理色温调节指令（全模式生效）
    if (1) {  // 全局生效的色温处理
        cJSON *temp_json = cJSON_GetObjectItem(json, "Temperature");
        if (temp_json && temp_json->type == cJSON_Number) {
            int new_temp = temp_json->valueint;
            if (new_temp >= 2000 && new_temp <= 6000) {
                current_temp = new_temp;
                if (is_light_on) {
                    WS2812B_SetTemperature_kit(current_temp);
                }
                if (kit_auto) {
                    WS2812B_SetTemperature_kit(current_temp);
                }
                UsartPrintf(USART_DEBUG, "Temperature updated to %dk", new_temp);
            }
        }
    }

    // 处理亮度调节指令（手动模式+灯亮时生效）
    if (!kit_auto && is_light_on) {  // 手动模式且灯亮时处理亮度
        cJSON *brightness_json = cJSON_GetObjectItem(json, "Brightness");
        if (brightness_json && brightness_json->type == cJSON_Number) {
            int new_brightness = brightness_json->valueint;
            if (new_brightness >= 0 && new_brightness <= 100) {
                current_brightness = new_brightness;
                WS2812B_SetBrightness_kit(current_brightness);
                UsartPrintf(USART_DEBUG, "Brightness adjusted to %d%%", new_brightness);
            }
        }
    }
}

//BathroomLED处理函数
void handleBathroomLED(cJSON *json) {
    static int current_brightness = 100;
    static int current_temp = 4000;
    static int is_light_on = 0;
	
	if (!json) return;
	
	// 解析Auto指令
	cJSON *auto_json = cJSON_GetObjectItem(json, "Auto");
	if (auto_json && auto_json->type == cJSON_Number) {
		bat_auto = auto_json->valueint;  // 更新全局模式
		if (bat_auto) {
			// 进入自动模式时关闭灯光
			is_light_on = 0;
			WS2812B_SetBrightness_bat(0);
			WS2812B_SetTemperature_bat(current_temp);
			UsartPrintf(USART_DEBUG, "System Auto Mode: %d", bat_auto);
		}else {
			UsartPrintf(USART_DEBUG, "Auto Mode Disabled");
		}
	}
	
    // 处理手动开关指令（独立处理）
    if (!bat_auto && json->type == cJSON_Number) {
        int raw_cmd = json->valueint;
        if (raw_cmd == 1) {
            is_light_on = 1;
            WS2812B_SetBrightness_bat(current_brightness);
			Delay_ms(50);
            WS2812B_SetTemperature_bat(current_temp);
			Delay_ms(50);
            UsartPrintf(USART_DEBUG, "Bathroom LED ON");
        } else {
            is_light_on = 0;
            WS2812B_SetBrightness_bat(0);
            UsartPrintf(USART_DEBUG, "Bathroom LED OFF");
        }
    }

    // 处理色温调节指令（全模式生效）
    if (1) {  // 全局生效的色温处理
        cJSON *temp_json = cJSON_GetObjectItem(json, "Temperature");
        if (temp_json && temp_json->type == cJSON_Number) {
            int new_temp = temp_json->valueint;
            if (new_temp >= 2000 && new_temp <= 6000) {
                current_temp = new_temp;
                if (is_light_on) {
                    WS2812B_SetTemperature_bat(current_temp);
                }
                if (bat_auto) {
                    WS2812B_SetTemperature_bat(current_temp);
                }
                UsartPrintf(USART_DEBUG, "Temperature updated to %dk", new_temp);
            }
        }
    }

    // 处理亮度调节指令（手动模式+灯亮时生效）
    if (!bat_auto && is_light_on) {  // 手动模式且灯亮时处理亮度
        cJSON *brightness_json = cJSON_GetObjectItem(json, "Brightness");
        if (brightness_json && brightness_json->type == cJSON_Number) {
            int new_brightness = brightness_json->valueint;
            if (new_brightness >= 0 && new_brightness <= 100) {
                current_brightness = new_brightness;
                WS2812B_SetBrightness_bat(current_brightness);
                UsartPrintf(USART_DEBUG, "Brightness adjusted to %d%%", new_brightness);
            }
        }
    }
}

// 窗帘处理函数
void handleBedroomCurtainRaw(uint8_t raw_cmd)
{
    static uint8_t curtain_state = 0;
    
    if(raw_cmd == 1 && !curtain_state) {
        MOTOR_Direction_Angle(0, 0, 3600, 1);
        curtain_state = 1;
        UsartPrintf(USART_DEBUG, "Bedroom Curtain: RAW_OPENED");
    }
    else if(raw_cmd == 0 && curtain_state) {
        MOTOR_Direction_Angle(1, 0, 3600, 1);
        curtain_state = 0;
        UsartPrintf(USART_DEBUG, "Bedroom Curtain: RAW_CLOSED");
    }
}

// 空调处理函数
void handleAC(cJSON *json)
{
	static uint8_t current_power;  //开关
    static uint8_t current_mode;  //模式
    static uint8_t current_temp;       // 默认温度
    static uint8_t current_fan; // 默认风速

    // 解析JSON参数
    cJSON *mode_item = cJSON_GetObjectItem(json, "mode");
    cJSON *temp_item = cJSON_GetObjectItem(json, "temp");
    cJSON *fan_item = cJSON_GetObjectItem(json, "fan");
    cJSON *power_item = cJSON_GetObjectItem(json, "power");

    if (json->type == cJSON_Number) {
		int raw_power = json->valueint;
		// 处理电源状态
		if(raw_power == 1) {
			current_power = kGreeon;
			ptcl.Power = kGreeon;
			ptcl.Mode = current_mode;
			ptcl.Temp = current_temp;
			ptcl.Fan = current_fan;
		} else {
			current_power = kGreeoff;
			ptcl.Temp = 25 - kGreeMinTempC;
			ptcl.Mode = kGreeCool;
			ptcl.Power = kGreeoff;}
    }

    // 处理模式设置（仅在设备开启时有效）
    if(current_power == kGreeon && mode_item && mode_item->type == cJSON_Number) {
        uint8_t new_mode = mode_item->valueint;
        if(new_mode <= kGreeEcono) {  // 仅处理有效模式
            current_mode = new_mode;
            ptcl.Mode = new_mode;
        }
    }

    // 处理温度设置（限制在16-30℃）
    if(temp_item && temp_item->type == cJSON_Number) {
        int new_temp = temp_item->valueint;
        if(new_temp < kGreeMinTempC) new_temp = kGreeMinTempC;
        if(new_temp > kGreeMaxTempC) new_temp = kGreeMaxTempC;
        current_temp = new_temp;
        ptcl.Temp = new_temp - kGreeMinTempC;
    }

    // 处理风速设置
    if(fan_item && fan_item->type == cJSON_Number) {
        uint8_t new_fan = fan_item->valueint;
        if(new_fan <= kGreeFanMax) {
            current_fan = new_fan;
            ptcl.Fan = new_fan;
        }
    }

    // 更新红外信号
    if(current_power == kGreeon || current_power == kGreeoff) {
        Gree_UpdateIRSignal();  // 发送完整控制指令
    }
    
    UsartPrintf(USART_DEBUG, "AC Control - Mode:%d, Temp:%d℃, Fan:%d",
               current_mode, current_temp, current_fan);
}

//LivingRoomLED自动化
void HandleLivingLEDAuto(void) {	
    static uint8_t prev_human = 0;
    static uint32_t delay_counter = 0;    
    static int delay_flag = 0;      

	if (global_light <= 200) {
		int pwm_value = 100 - (global_light * 100) / 200;
		
		if (living_human == 0) {
			WS2812B_SetTemperature_liv(3500);
			WS2812B_SetBrightness_liv(pwm_value);
			delay_flag = 0;
			delay_counter = 0;
		} else {
			if (prev_human == 0) {
				delay_flag = 1;
				delay_counter = 0;
			}
			
			if (delay_flag) {
				if (++delay_counter >= 6) {
					WS2812B_SetBrightness_liv(0);
					delay_flag = 0;
				} else {
					WS2812B_SetBrightness_liv(pwm_value);
				}
			} else {
				WS2812B_SetBrightness_liv(0);
			}
		}
	} else {		// 光照充足时无论是否有人都关灯
		WS2812B_SetBrightness_liv(0);
		delay_flag = 0;    // 强制清除延时
		delay_counter = 0; // 重置计时器
	}
	prev_human = living_human;
}

//BedRoomLED自动化
void HandleBedLEDAuto(void) {	
    static uint8_t prev_human = 0;
    static uint32_t delay_counter = 0;    
    static int delay_flag = 0;      

	if (global_light <= 200) {
		int pwm_value = 100 - (global_light * 100) / 200;
		
		if (bed_human == 0) {
			WS2812B_SetTemperature_bed(3000);
			WS2812B_SetBrightness_bed(pwm_value);
			delay_flag = 0;
			delay_counter = 0;
		} else {
			if (prev_human == 0) {
				delay_flag = 1;
				delay_counter = 0;
			}
			
			if (delay_flag) {
				if (++delay_counter >= 6) {
					WS2812B_SetBrightness_bed(0);
					delay_flag = 0;
				} else {
					WS2812B_SetBrightness_bed(pwm_value);
				}
			} else {
				WS2812B_SetBrightness_bed(0);
			}
		}
	} else {		// 光照充足时无论是否有人都关灯
		WS2812B_SetBrightness_bed(0);
		delay_flag = 0;    // 强制清除延时
		delay_counter = 0; // 重置计时器
	}
	prev_human = bed_human;
}

//StudyRoomLED自动化
void HandleStudyLEDAuto(void) {	
    static uint8_t prev_human = 0;
    static uint32_t delay_counter = 0;    
    static int delay_flag = 0;      

	if (global_light <= 200) {
		int pwm_value = 100 - (global_light * 100) / 200;
		
		if (study_human == 0) {
			WS2812B_SetTemperature_stu(4500);
			WS2812B_SetBrightness_stu(pwm_value);
			delay_flag = 0;
			delay_counter = 0;
		} else {
			if (prev_human == 0) {
				delay_flag = 1;
				delay_counter = 0;
			}
			
			if (delay_flag) {
				if (++delay_counter >= 6) {
					WS2812B_SetBrightness_stu(0);
					delay_flag = 0;
				} else {
					WS2812B_SetBrightness_stu(pwm_value);
				}
			} else {
				WS2812B_SetBrightness_stu(0);
			}
		}
	} else {		// 光照充足时无论是否有人都关灯
		WS2812B_SetBrightness_stu(0);
		delay_flag = 0;    // 强制清除延时
		delay_counter = 0; // 重置计时器
	}
	prev_human = study_human;
}

//KitchenLED自动化
void HandleKitLEDAuto(void) {	
    static uint8_t prev_human = 0;
    static uint32_t delay_counter = 0;    
    static int delay_flag = 0;      

	if (global_light <= 200) {
		int pwm_value = 100 - (global_light * 100) / 200;
		
		if (kit_human == 0) {
			WS2812B_SetTemperature_kit(4500);
			WS2812B_SetBrightness_kit(pwm_value);
			delay_flag = 0;
			delay_counter = 0;
		} else {
			if (prev_human == 0) {
				delay_flag = 1;
				delay_counter = 0;
			}
			
			if (delay_flag) {
				if (++delay_counter >= 6) {
					WS2812B_SetBrightness_kit(0);
					delay_flag = 0;
				} else {
					WS2812B_SetBrightness_kit(pwm_value);
				}
			} else {
				WS2812B_SetBrightness_kit(0);
			}
		}
	} else {		// 光照充足时无论是否有人都关灯
		WS2812B_SetBrightness_kit(0);
		delay_flag = 0;    // 强制清除延时
		delay_counter = 0; // 重置计时器
	}
	prev_human = kit_human;
}

//BathRoomLED自动化
void HandleBathLEDAuto(void) {	
    static uint8_t prev_human = 0;
    static uint32_t delay_counter = 0;    
    static int delay_flag = 0;      

	if (global_light <= 200) {
		int pwm_value = 100 - (global_light * 100) / 200;
		
		if (bat_human == 0) {
			WS2812B_SetTemperature_bat(4000);
			WS2812B_SetBrightness_bat(pwm_value);
			delay_flag = 0;
			delay_counter = 0;
		} else {
			if (prev_human == 0) {
				delay_flag = 1;
				delay_counter = 0;
			}
			
			if (delay_flag) {
				if (++delay_counter >= 6) {
					WS2812B_SetBrightness_bat(0);
					delay_flag = 0;
				} else {
					WS2812B_SetBrightness_bat(pwm_value);
				}
			} else {
				WS2812B_SetBrightness_bat(0);
			}
		}
	} else {		// 光照充足时无论是否有人都关灯
		WS2812B_SetBrightness_bat(0);
		delay_flag = 0;    // 强制清除延时
		delay_counter = 0; // 重置计时器
	}
	prev_human = bat_human;
}
