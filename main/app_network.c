#include <string.h>
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "mqtt_client.h"
#include "webconfig.h"
#include "app_network.h"
#include "oled.h"

static const char *TAG = "APP_NET";

// ================== 【阿里云 MQTT 配置区】 ==================
#define MQTT_BROKER_URL     "mqtt://47.103.140.214:1883" 
#define MQTT_USERNAME       "admin"              
#define MQTT_PASSWORD       "public"
#define MQTT_TOPIC          "hardware/dht11" 
// ==========================================================

static esp_mqtt_client_handle_t mqtt_client = NULL;
static bool is_mqtt_connected = false;

// 对外提供的接口：是否连接成功
bool app_network_is_mqtt_ready(void) {
    return (is_mqtt_connected && mqtt_client != NULL);
}

// 对外提供的接口：发送数据
void app_network_mqtt_publish(const char *payload) {
    if (app_network_is_mqtt_ready()) {
        int msg_id = esp_mqtt_client_publish(mqtt_client, MQTT_TOPIC, payload, 0, 1, 0);
        if (msg_id != -1) {
            ESP_LOGI(TAG, "Successfully pushed to the cloud: %s", payload);
        }
    }
}

// MQTT 事件回调
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            //ESP_LOGI(TAG, "MQTT connected successfully!");
            is_mqtt_connected = true;
            OLED_Clear();
            OLED_ShowString(0, 0, "MQTT Connected!", 8);
            OLED_ShowString(0, 2, "Start Reading...", 8);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGW(TAG, "MQTT disconnected!");
            is_mqtt_connected = false;
            break;
        default:
            break;
    }
}

// Wi-Fi 事件回调
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGW(TAG, "Disconnected from WiFi router, reconnecting...");
        esp_wifi_connect();
    } 
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        //ESP_LOGI(TAG, "Successfully obtained local IP: " IPSTR, IP2STR(&event->ip_info.ip));
        OLED_Clear();
        OLED_ShowString(0, 0, "WiFi Connected!", 8);
        OLED_ShowString(0, 2, "Connecting MQTT.", 8);

        // 清理配网服务
        webconfig_stop();
        esp_wifi_set_mode(WIFI_MODE_STA);
        ESP_LOGI(TAG, "entering pure collection mode！");
        
        // 启动 MQTT
        if (mqtt_client == NULL) {
            esp_mqtt_client_config_t mqtt_cfg = {
                .broker.address.uri = MQTT_BROKER_URL,
                .credentials.username = MQTT_USERNAME,
                .credentials.authentication.password = MQTT_PASSWORD,
            };
            mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
            esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
            esp_mqtt_client_start(mqtt_client);
        }
    }
}

// 初始化网络模块监听
void app_network_init(void) {
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL);
}