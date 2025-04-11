//启用lvgl_port_display
#if 1
#ifndef LVGL_PORT_DISPLAY_H
#define LVGL_PORT_DISPLAY_H

//头文件
#include"lvgl/lvgl.h"
#include"ili9488_driver.h"

//LVGL移植显示初始化
uint8_t lvgl_port_display_init();

#endif//#ifndef LVGL_PORT_DISPLAY_H
#endif//#if 1
