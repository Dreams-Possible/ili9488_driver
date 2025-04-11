//头文件
#include<stdio.h>
#include"freertos/FreeRTOS.h"
#include"freertos/task.h"

#include"spi_driver.h"
#include"ili9488_driver.h"

#include"lvgl/lvgl.h"
#include"lvgl_port_display.h"
#include"ui.h"

// //简单测试配置
// #define HOR 240
// #define VER 320
// #define SIZE HOR*VER

void app_main(void)
{
    //SPI初始化
    spi_init();

    //屏幕初始化
    ili9488_init();

    // //简单测试
    // uint16_t*data=(uint16_t*)malloc(SIZE*2);
    // if(!data)
    // {
    //     while(1)
    //     {
    //         printf("no mem");
    //         vTaskDelay(pdMS_TO_TICKS(1000));
    //     }
    // }

    // uint16_t color=0x07bf;
    // for(uint32_t i=0;i<SIZE;++i)
    // {
    //     data[i]=color;
    // }

    // //ili9488 flash color
    // ili9488_flash(0,0,239,79,data);
    // vTaskDelay(pdMS_TO_TICKS(100));

    // ili9488_flash(0,80,239,159,data);
    // vTaskDelay(pdMS_TO_TICKS(100));

    // ili9488_flash(0,160,239,239,data);
    // vTaskDelay(pdMS_TO_TICKS(100));

    // ili9488_flash(0,240,239,319,data);
    // vTaskDelay(pdMS_TO_TICKS(100));

    // if(data)
    // {
    //     free(data);
    // }else
    // {
    //     printf("mem err\n");
    // }

    //LVGL初始化
    lv_init();

    //LVGL移植显示初始化
    lvgl_port_display_init();

    //LVGLUI初始化
    ui_init();

    //LVGL运行
    while(1)
    {
        //提供时基
        lv_tick_inc(10);
        //LVGL运行
        lv_task_handler();
        //让出资源
        vTaskDelay(pdMS_TO_TICKS(10));
    }

}
