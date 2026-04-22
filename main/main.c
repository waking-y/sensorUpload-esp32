#include "nvs_flash.h"
#include "esp_log.h"
#include "led.h"
#include "dht11.h"
#include "webconfig.h"
#include "app_network.h"
#include "app_sensor.h"

static const char *TAG = "APP_MAIN";

void app_main(void) {
    // 1. 初始化基础存储
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 2. 初始化底层硬件 (BSP)
    led_init();
    dht11_init();

    // 3. 启动网页配网环境
    webconfig_init_ap_and_server();

    // 4. 初始化应用层模块
    app_network_init();       // 注册网络监听
    app_sensor_start_task();  // 开启采集任务
    
    ESP_LOGI(TAG, "====================================================");
    ESP_LOGI(TAG, "system initialized, waiting for network connection...");
    ESP_LOGI(TAG, "====================================================");
}