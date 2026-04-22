#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "led.h"
#include "dht11.h"
#include "app_network.h"
#include "app_sensor.h"

static const char *TAG = "APP_SENSOR";

static void dht11_upload_task(void *pvParameters) {
    uint8_t temperature = 0;
    uint8_t humidity = 0;
    char payload[128]; 

    while (1) {
        bool read_success = false;
        
        // 容错 3 次读取
        for (int retry = 0; retry < 3; retry++) {
            if (dht11_read_data(&temperature, &humidity) == 0) {
                read_success = true;
                break; 
            }
            vTaskDelay(pdMS_TO_TICKS(100)); 
        }

        if (read_success) {
            ESP_LOGI(TAG, "local- temp: %d C, humi: %d %%", temperature, humidity);
            
            // 状态指示灯
            gpio_set_level(LED_GPIO_PIN, 0); 
            vTaskDelay(pdMS_TO_TICKS(100));  
            gpio_set_level(LED_GPIO_PIN, 1); 

            // 如果网络准备好了，就让网络模块帮忙发出去
            if (app_network_is_mqtt_ready()) {
                sprintf(payload, "{\"temp\": %d, \"humi\": %d}", temperature, humidity);
                app_network_mqtt_publish(payload);
            }
        } else {
            ESP_LOGE(TAG, "DHT11 read failed!");
        }

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void app_sensor_start_task(void) {
    // 启动 FreeRTOS 任务
    xTaskCreate(dht11_upload_task, "dht11_task", 4096, NULL, 5, NULL);
    ESP_LOGI(TAG, "The sensor collection task has been started！");
}