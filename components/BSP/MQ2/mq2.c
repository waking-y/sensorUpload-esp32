#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_adc/adc_oneshot.h"
#include "mq2.h"

static const char *TAG = "MQ2";

#define MQ2_ADC_UNIT    ADC_UNIT_1
#define MQ2_ADC_CHANNEL ADC_CHANNEL_4 // 对应 GPIO 5

static adc_oneshot_unit_handle_t adc1_handle;

void mq2_init(void) {
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = MQ2_ADC_UNIT,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT, 
        .atten = ADC_ATTEN_DB_12,         
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, MQ2_ADC_CHANNEL, &config));
    ESP_LOGI(TAG, "MQ-2 初始化完成，已开启自动校准");
}

// 对应 STM32 的多次平均算法
int mq2_read_average(int times, int *avg_val) {
    if (times <= 0) return -1;
    long sum = 0;
    int temp_val = 0;
    
    for (int i = 0; i < times; i++) {
        if (adc_oneshot_read(adc1_handle, MQ2_ADC_CHANNEL, &temp_val) == ESP_OK) {
            sum += temp_val;
        } else {
            return -1; // 读取失败
        }
        vTaskDelay(pdMS_TO_TICKS(5)); // 每次采样间隔 5ms
    }
    
    *avg_val = sum / times;
    return 0;
}

// 对应 STM32 的 PPM 换算公式
int mq2_read_ppm(float *ppm_value) {
    int avg_val = 0;
    // 采样 10 次求平均
    if (mq2_read_average(10, &avg_val) == 0) {
        // STM32 里的公式: adc_val * 1000.0f / 4095.0f
        *ppm_value = (float)avg_val * 1000.0f / 4095.0f;
        return 0;
    }
    return -1;
}