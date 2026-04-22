#ifndef __OLED_H
#define __OLED_H

#include <stdint.h>

// 初始化 OLED (I2C引脚: SDA=17, SCL=18)
void OLED_Init(void);

// 清屏
void OLED_Clear(void);

// 显示单个字符
// size: 8 (6x8小字体) 或 16 (8x16大字体)
void OLED_ShowChar(uint8_t x, uint8_t y, uint8_t chr, uint8_t size);

// 显示英文字符串
// size: 8 (6x8小字体) 或 16 (8x16大字体)
void OLED_ShowString(uint8_t x, uint8_t y, char *chr, uint8_t size);

#endif