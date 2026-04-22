#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_pm.h"     

#include "led.h"
#include "dht11.h"
#include "mq2.h"       
#include "webconfig.h"
#include "app_network.h"
#include "app_sensor.h"

static const char *TAG = "APP_MAIN";

void app_main(void) {
    // ============ 【配置自动低功耗机制】 ============
#if CONFIG_PM_ENABLE
    esp_pm_config_t pm_config = {
        .max_freq_mhz = 160,       // 满载时 CPU 跑 160MHz
        .min_freq_mhz = 10,        // 闲置时 CPU 降频至 10MHz
        .light_sleep_enable = true // 允许进入轻度休眠
    };
    ESP_ERROR_CHECK(esp_pm_configure(&pm_config));
    ESP_LOGI(TAG, "自动轻度休眠与动态调频已开启！");
#endif
    // =================================================

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
    mq2_init();  

    // 3. 启动网页配网环境
    webconfig_init_ap_and_server();

    // 4. 初始化应用层模块
    app_network_init();       
    app_sensor_start_task();  
    
    ESP_LOGI(TAG, "=============================================");
    ESP_LOGI(TAG, "system initialized, waiting for network connection...");
    ESP_LOGI(TAG, "=============================================");
}