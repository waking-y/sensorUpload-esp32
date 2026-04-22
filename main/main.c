#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_pm.h"     
#include "oled.h"
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

    OLED_Init();
    OLED_Clear();
    OLED_ShowString(0, 0, "Please enter in the browser:", 8);
    OLED_ShowString(0, 2, "http://", 8);
    OLED_ShowString(0, 4, "192.168.4.1", 16);
    vTaskDelay(pdMS_TO_TICKS(5000));

    // 3. 启动网页配网环境
    webconfig_init_ap_and_server();
    
    OLED_ShowString(0, 2, "AP: ESP32_Config", 8);
    OLED_ShowString(0, 4, "Wait Web Config.", 8);
    // 4. 初始化应用层模块
    app_network_init();       
    app_sensor_start_task();  
    
    // ESP_LOGI(TAG, "=============================================");
    // ESP_LOGI(TAG, "system initialized, waiting for network connection...");
    // ESP_LOGI(TAG, "=============================================");
}