#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "led.h"
#include "dht11.h"
#include "mq2.h"        
#include "app_network.h"
#include "app_sensor.h"
#include "oled.h"

static const char *TAG = "APP_SENSOR";

static void sensor_upload_task(void *pvParameters) {
    uint8_t temperature = 0;
    uint8_t humidity = 0;
    float gas_ppm = 0.0f;    
    char payload[128]; 
    char display_buf[32];

    while (1) {
        bool dht11_success = false;
        for (int retry = 0; retry < 3; retry++) {
            if (dht11_read_data(&temperature, &humidity) == 0) { dht11_success = true; break; }
            vTaskDelay(pdMS_TO_TICKS(100)); 
        }

        mq2_read_ppm(&gas_ppm);

        if (dht11_success) {
            // 打印时带上 1 位小数
            ESP_LOGI(TAG, "local- Temp: %d C, Humi: %d %%, Gas: %.1f ppm", temperature, humidity, gas_ppm);
            //OLED_Clear();

            gpio_set_level(LED_GPIO_PIN, 0); 
            vTaskDelay(pdMS_TO_TICKS(100));  
            gpio_set_level(LED_GPIO_PIN, 1); 

            sprintf(display_buf, "Temp: %2d C    ", temperature);
            OLED_ShowString(0, 0, display_buf, 16);
            
            sprintf(display_buf, "Humi: %2d %%    ", humidity);
            OLED_ShowString(0, 2, display_buf, 16);

            sprintf(display_buf, "Gas : %.1f ppm  ", gas_ppm);
            OLED_ShowString(0, 4, display_buf, 16);

            if (app_network_is_mqtt_ready()) {
                // 打包 JSON：上传 ppm 值
                sprintf(payload, "{\"temp\": %d, \"humi\": %d, \"gas_ppm\": %.1f}", temperature, humidity, gas_ppm);
                app_network_mqtt_publish(payload);
            }
        } else {
            ESP_LOGE(TAG, "DHT11 read error");
        }

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void app_sensor_start_task(void) {
    xTaskCreate(sensor_upload_task, "sensor_task", 4096, NULL, 5, NULL);
    ESP_LOGI(TAG, "sensor task started！");
}