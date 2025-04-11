//启用ili9488_driver
#if 1
#ifndef ILI9488_DRIVER_H
#define ILI9488_DRIVER_H

//头文件
#include<stdio.h>
#include<stdint.h>
#include"driver/gpio.h"
#include"driver/spi_master.h"
#include"freertos/FreeRTOS.h"

//ili9488初始化
void ili9488_init();
//ili9488刷新
void ili9488_flash(int16_t x1,int16_t y1,int16_t x2,int16_t y2,uint16_t*color);

#endif//#ifndef ILI9488_DRIVER_H
#endif//#if 1
