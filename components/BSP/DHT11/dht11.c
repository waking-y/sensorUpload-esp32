#include "dht11.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "rom/ets_sys.h" // 提供精确的微秒级延时 ets_delay_us()
#include "esp_log.h"

static const char *TAG = "DHT11";

// 辅助函数：带超时的等待电平变化，防止程序死循环卡死
static int wait_for_state(int target_state, int timeout_us) {
    int time_waited = 0;
    while (gpio_get_level(DHT11_PIN) != target_state) {
        if (time_waited > timeout_us) return -1; // 超时返回错误
        ets_delay_us(1); // 延时 1 微秒
        time_waited++;
    }
    return time_waited; // 返回实际等待的时间
}

void dht11_init(void) {
    gpio_reset_pin(DHT11_PIN);
    gpio_set_direction(DHT11_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(DHT11_PIN, 1); // 默认拉高总线
    ESP_LOGI(TAG, "DHT11 init success, pin: %d", DHT11_PIN);
}

int dht11_read_data(uint8_t *temp, uint8_t *humi) {
    uint8_t data[5] = {0};

    // ========== 1. 主机发送起始信号 ==========
    gpio_set_direction(DHT11_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(DHT11_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(20)); // 主机拉低总线至少 18ms
    gpio_set_level(DHT11_PIN, 1);
    ets_delay_us(30);              // 主机拉高 20~40us，准备读取
    gpio_set_direction(DHT11_PIN, GPIO_MODE_INPUT); // 切换为输入模式

    // ========== 2. 等待 DHT11 响应 ==========
    // DHT11 会拉低 80us，然后拉高 80us
    if (wait_for_state(0, 80) == -1) return -1; // 等待低电平
    if (wait_for_state(1, 80) == -1) return -1; // 等待高电平
    if (wait_for_state(0, 80) == -1) return -1; // 等待拉低，准备传数据

    // ========== 3. 读取 40 bit 数据 ==========
    for (int i = 0; i < 40; i++) {
        // 等待每 bit 开始的 50us 低电平结束，变为高电平
        if (wait_for_state(1, 80) == -1) return -1; 
        
        // 测量高电平持续的时间
        int high_time = wait_for_state(0, 80);      
        if (high_time == -1) return -1;

        // 根据高电平时间判断是 '0' 还是 '1'
        // '0' 大概 26-28us，'1' 大概 70us。我们取中间值 40us 作为分界线
        int byte_idx = i / 8;
        data[byte_idx] <<= 1; // 左移一位腾出空间
        if (high_time > 40) {
            data[byte_idx] |= 1; // 如果时间长，这一位写 1
        }
    }

    // ========== 4. 校验数据 ==========
    // data[0]湿度整数, data[1]湿度小数, data[2]温度整数, data[3]温度小数, data[4]校验和
    if (data[0] + data[1] + data[2] + data[3] == data[4]) {
        *humi = data[0];
        *temp = data[2];
        return 0; // 读取成功
    }
    
    ESP_LOGE(TAG, "check sum error!");
    return -1; // 校验失败
}