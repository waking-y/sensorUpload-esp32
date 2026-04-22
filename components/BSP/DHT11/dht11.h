// components/BSP/DHT11/dht11.h

#ifndef __DHT11_H__
#define __DHT11_H__

#include "driver/gpio.h"

#define DHT11_PIN  4  

void dht11_init(void);
int dht11_read_data(uint8_t *temp, uint8_t *humi); 

#endif