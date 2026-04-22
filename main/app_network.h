#ifndef __APP_NETWORK_H__
#define __APP_NETWORK_H__

#include <stdbool.h>

// 初始化网络事件监听
void app_network_init(void);

// 检查 MQTT 是否准备就绪
bool app_network_is_mqtt_ready(void);

// 向阿里云发送数据
void app_network_mqtt_publish(const char *payload);

#endif