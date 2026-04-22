#include "oled.h"
#include "oledfont.h" // 注意这里是你修改过的一维数组字库文件
#include "driver/i2c.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define I2C_MASTER_SCL_IO           18      // SCL引脚
#define I2C_MASTER_SDA_IO           17      // SDA引脚
#define I2C_MASTER_NUM              I2C_NUM_0
#define I2C_MASTER_FREQ_HZ          400000  // I2C时钟 400kHz
#define OLED_ADDR                   0x3C    // OLED I2C地址

static void OLED_WriteCmd(uint8_t cmd) {
    uint8_t data[2] = {0x00, cmd};
    i2c_master_write_to_device(I2C_MASTER_NUM, OLED_ADDR, data, 2, pdMS_TO_TICKS(1000));
}

static void OLED_WriteDat(uint8_t dat) {
    uint8_t data[2] = {0x40, dat};
    i2c_master_write_to_device(I2C_MASTER_NUM, OLED_ADDR, data, 2, pdMS_TO_TICKS(1000));
}

static void OLED_SetPos(uint8_t x, uint8_t y) {
    OLED_WriteCmd(0xb0 + y);
    OLED_WriteCmd(((x & 0xf0) >> 4) | 0x10);
    OLED_WriteCmd((x & 0x0f));
}

void OLED_Clear(void) {
    for(uint8_t i = 0; i < 8; i++) {
        OLED_WriteCmd(0xb0 + i);
        OLED_WriteCmd(0x00);
        OLED_WriteCmd(0x10);
        for(uint8_t n = 0; n < 128; n++) OLED_WriteDat(0);
    }
}

void OLED_ShowChar(uint8_t x, uint8_t y, uint8_t chr, uint8_t size) {
    uint8_t c = chr - ' '; 
    
    if (size == 8) { // 使用 6x8 小字体
        if(x > 122) { x = 0; y += 1; } // 换行
        OLED_SetPos(x, y);
        for(uint8_t i = 0; i < 6; i++) {
            OLED_WriteDat(F6x8[c * 6 + i]);
        }
    } 
    else if (size == 16) { // 使用 8x16 大字体
        if(x > 120) { x = 0; y += 2; } // 换行 (16字体占2页)
        OLED_SetPos(x, y);
        for(uint8_t i = 0; i < 8; i++) {
            OLED_WriteDat(F8X16[c * 16 + i]);
        }
        OLED_SetPos(x, y + 1);
        for(uint8_t i = 0; i < 8; i++) {
            OLED_WriteDat(F8X16[c * 16 + i + 8]);
        }
    }
}

void OLED_ShowString(uint8_t x, uint8_t y, char *chr, uint8_t size) {
    uint8_t j = 0;
    while(chr[j] != '\0') {
        OLED_ShowChar(x, y, chr[j], size);
        if (size == 8) {
            x += 6; // 6x8 字体横向加 6
            if(x > 122) { x = 0; y += 1; }
        } else {
            x += 8; // 8x16 字体横向加 8
            if(x > 120) { x = 0; y += 2; }
        }
        j++;
    }
}

void OLED_Init(void) {
    // 1. 初始化 I2C
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);

    vTaskDelay(pdMS_TO_TICKS(200));

    // 2. 初始化 SSD1306
    OLED_WriteCmd(0xAE); // display off
    OLED_WriteCmd(0x20); // Set Memory Addressing Mode
    OLED_WriteCmd(0x10); // 00,Horizontal; 01,Vertical; 10,Page
    OLED_WriteCmd(0xb0); // Set Page Start Address
    OLED_WriteCmd(0xc8); // Set COM Output Scan Direction
    OLED_WriteCmd(0x00); // set low column address
    OLED_WriteCmd(0x10); // set high column address
    OLED_WriteCmd(0x40); // set start line address
    OLED_WriteCmd(0x81); // set contrast control register
    OLED_WriteCmd(0xff); // 亮度调节 0x00~0xff
    OLED_WriteCmd(0xa1); // set segment re-map
    OLED_WriteCmd(0xa6); // set normal display
    OLED_WriteCmd(0xa8); // set multiplex ratio
    OLED_WriteCmd(0x3F); 
    OLED_WriteCmd(0xa4); // 0xa4,Output follows RAM content
    OLED_WriteCmd(0xd3); // set display offset
    OLED_WriteCmd(0x00); 
    OLED_WriteCmd(0xd5); // set osc division
    OLED_WriteCmd(0xf0); 
    OLED_WriteCmd(0xd9); // set pre-charge period
    OLED_WriteCmd(0x22); 
    OLED_WriteCmd(0xda); // set com pins hardware config
    OLED_WriteCmd(0x12); 
    OLED_WriteCmd(0xdb); // set vcomh
    OLED_WriteCmd(0x20); 
    OLED_WriteCmd(0x8d); // set DC-DC enable
    OLED_WriteCmd(0x14); 
    OLED_WriteCmd(0xaf); // display on
    OLED_Clear();
}