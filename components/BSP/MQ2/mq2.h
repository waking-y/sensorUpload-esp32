#ifndef __MQ2_H__
#define __MQ2_H__

void mq2_init(void);

// 对应 STM32 的 MQ2_Read_Average：连续读取多次求平均值
int mq2_read_average(int times, int *avg_val);

// 对应 STM32 的 MQ2_Read_PPM：计算 PPM 浓度 (0~1000)
int mq2_read_ppm(float *ppm_value);

#endif