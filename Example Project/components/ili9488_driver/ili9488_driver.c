#include"ili9488_driver.h"
//启用ili9488_driver
#ifdef ILI9488_DRIVER_H

//配置
#define DC 21
#define RST 47//-1表示不启用
#define LED 14//-1表示不启用
#define HOR 320//水平像素
#define HOR_OFF 0//水平像素偏移
#define VER 480//垂直像素
#define VER_OFF 0//垂直像素偏移
#define DIR 0//旋转方向
#define BRG 1//0表示RGB顺序，1表示BGR顺序
#define INV 0//是否启用反色
#define SPI SPI2_HOST
#define RATE 40*1000*1000
#define CS 10

//某OEM ili9488横屏示例配置
// //配置
// #define DC 21
// #define RST 47//-1表示不启用
// #define LED 14//-1表示不启用
// #define HOR 320//水平像素
// #define HOR_OFF 0//水平像素偏移
// #define VER 240//垂直像素
// #define VER_OFF 0//垂直像素偏移
// #define DIR 0//旋转方向
// #define BRG 1//0表示RGB顺序，1表示BGR顺序
// #define INV 0//是否启用反色

//某OEM ili9488竖屏示例配置
// //配置
// #define DC 21
// #define RST 47//-1表示不启用
// #define LED 14//-1表示不启用
// #define HOR 240//水平像素
// #define HOR_OFF 0//水平像素偏移
// #define VER 320//垂直像素
// #define VER_OFF 0//垂直像素偏移
// #define DIR 2//旋转方向
// #define BRG 1//0表示RGB顺序，1表示BGR顺序
// #define INV 0//是否启用反色

//定义
#define SWRESET 0x01
#define SLPOUT 0x11
#define INVON 0x21
#define INVOFF 0x20
#define COLMOD 0x3a
#define MADCTL 0x36
#define DISPON 0x29
#define GMCTRP1 0xe0
#define GMCTRN1 0xe1

//本地数据
typedef struct local_data_t
{
    spi_device_handle_t spi;
}local_data_t;
static local_data_t local_data={0};

//引脚设置
static void io(uint16_t pin,uint8_t level);
//毫秒级延迟
static void delay(uint32_t ms);
//发送命令
static void send_cmd(uint8_t cmd);
//发送数据
static void send_data(uint8_t data);
//发送颜色
static void send_color(uint16_t*data,uint32_t length);
//初始化引脚
static void init_gpio();
//初始化SPI
static void init_spi();
//初始化软件
static void init_soft();
//ili9488初始化
void ili9488_init();
//ili9488刷新
void ili9488_flash(int16_t x1,int16_t y1,int16_t x2,int16_t y2,uint16_t*color);

//引脚设置
static void io(uint16_t pin,uint8_t level)
{
    gpio_set_level(pin,level);
    return;
}

//毫秒级延迟
static void delay(uint32_t ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
    return;
}

//发送命令
static void send_cmd(uint8_t cmd)
{
    io(DC,0);
    spi_transaction_t trans={0};
    trans.length=8;
    trans.tx_buffer=&cmd;
    spi_device_transmit(local_data.spi,&trans);
    return;
}

//发送数据
static void send_data(uint8_t data)
{
    io(DC,1);
    spi_transaction_t trans={0};
    trans.length=8;
    trans.tx_buffer=&data;
    spi_device_transmit(local_data.spi,&trans);
    return;
}

//发送颜色
static void send_color(uint16_t*data,uint32_t length)
{
    io(DC,1);
    if(data==NULL||length==0)
    {
        return;
    }
    //申请内存
    uint8_t*data_ts=malloc(length*3);
    if(!data_ts)
    {
        return;
    }
    uint32_t offset=0;
    //ili9488要求按照rgb888格式接收（但实际上按照rgb666处理）
    for(uint32_t f=0;f<length;++f)
    {
        data_ts[offset]=(uint8_t)(((data[f]&0xf800)>>8)|((data[f]&0x8000)>>13));
        ++offset;
        data_ts[offset]=(uint8_t)((data[f]&0x07e0)>>3);
        ++offset;
        data_ts[offset]=(uint8_t)(((data[f]&0x001f)<<3)|((data[f]&0x0010)>>2));
        ++offset;
    }
    spi_transaction_t transaction={0};
    offset=0;
    uint32_t len=length*3;
    while(len>0)
    {
        if(len>=512)
        {
            transaction.length=512*8;
            transaction.tx_buffer=&data_ts[offset];
            transaction.rxlength=0;
            spi_device_transmit(local_data.spi,&transaction);
            offset+=512;
            len-=512;
        }else
        {
            transaction.length=len*8;
            transaction.tx_buffer=&data_ts[offset];
            transaction.rxlength=0;
            spi_device_transmit(local_data.spi,&transaction);
            len=0;
        }
    }
    free(data_ts);
}

//初始化引脚
static void init_gpio()
{
    gpio_config_t config={0};
    //DC
    config.intr_type=GPIO_INTR_DISABLE;
    config.mode=GPIO_MODE_OUTPUT;
    config.pin_bit_mask=(1ULL<<DC);
    config.pull_down_en=0;
    config.pull_up_en=0;
    gpio_config(&config);
    #if RST!=-1
        //RST
        config.intr_type=GPIO_INTR_DISABLE;
        config.mode=GPIO_MODE_OUTPUT;
        config.pin_bit_mask=(1ULL<<RST);
        config.pull_down_en=0;
        config.pull_up_en=0;
        gpio_config(&config);
    #endif//#if RST!=-1
    #if LED!=-1
        //LED
        config.intr_type=GPIO_INTR_DISABLE;
        config.mode=GPIO_MODE_OUTPUT;
        config.pin_bit_mask=(1ULL<<LED);
        config.pull_down_en=0;
        config.pull_up_en=0;
        gpio_config(&config);
    #endif//#if LED!=-1
    return;
}

//初始化SPI
static void init_spi()
{
    //添加SPI设备
    spi_device_interface_config_t config={0};
    config.mode=0;
    config.spics_io_num=CS;
    config.queue_size=1;
    config.clock_speed_hz=RATE;
    spi_bus_add_device(SPI,&config,&local_data.spi);
}

//初始化软件
static void init_soft()
{
    //禁用背光
    if(LED!=-1)
    {
        io(LED,0);
    }
    //硬件复位
    if(RST!=-1)
    {
        io(RST,1);
        io(RST,0);
        io(RST,1);
        delay(200);
    }
    //发送软件初始化代码（已经为最小必须设置）
    send_cmd(SWRESET);//软复位
    delay(200);
    send_cmd(SLPOUT);//退出睡眠模式
    delay(200);
    if(INV)
    {
        send_cmd(INVON);//开启反色
    }else
    {
        send_cmd(INVOFF);//关闭反色
    }
    send_cmd(COLMOD);//设置颜色模式为18bit（rgb666）
    send_data(0x66);
    send_cmd(MADCTL);//设置内存访问控制（用来控制横竖屏，本质上是在设置交换xy轴和xy轴镜像；还有BRG和RGB）
    uint8_t bgr=0;
    if(BRG)
    {
        bgr=0x08;
    }
    switch(DIR)
    {
        case 0:
            send_data(0x80|bgr);
        break;
        case 1:
            send_data(0x40|bgr);
        break;
        case 2:
            send_data(0xe0|bgr);
        break;
        case 3:
            send_data(0x20|bgr);
        break;
        default:
            send_data(0x80|bgr);
        break;
    }
    send_cmd(GMCTRP1);//设置正极伽马矫正（非必须，不设置颜色会有点奇怪）
    send_data(0x01);
    send_data(0x13);
    send_data(0x1e);
    send_data(0x00);
    send_data(0x0d);
    send_data(0x03);
    send_data(0x3d);
    send_data(0x55);
    send_data(0x4f);
    send_data(0x06);
    send_data(0x10);
    send_data(0x0b);
    send_data(0x2c);
    send_data(0x32);
    send_data(0x0f);
    send_cmd(GMCTRN1);//设置负极伽马矫正（非必须，不设置颜色会有点奇怪）
    send_data(0x08);
    send_data(0x10);
    send_data(0x15);
    send_data(0x03);
    send_data(0x0e);
    send_data(0x03);
    send_data(0x32);
    send_data(0x34);
    send_data(0x44);
    send_data(0x07);
    send_data(0x10);
    send_data(0x0e);
    send_data(0x23);
    send_data(0x2e);
    send_data(0x0f);
    send_cmd(DISPON);
    delay(200);
    //启用背光
    if(LED!=-1)
    {
        io(LED,1);
    }
}

//ili9488初始化
void ili9488_init()
{
    init_gpio();
    init_spi();
    init_soft();
    return;
}

//ili9488刷新
void ili9488_flash(int16_t x1,int16_t y1,int16_t x2,int16_t y2,uint16_t*color)
{
    //检查参数
    if(x2<x1||y2<y1||color==NULL)
    {
        return;
    }
    if(x1<0)
    {
        return;
    }
    if(y1<0)
    {
        return;
    }
    if(x2>=HOR)
    {
        return;
    }
    if(y2>=VER)
    {
        return;
    }
    uint8_t data=0;
    //设置x轴起点终点
    send_cmd(0x2a);
    data=(uint8_t)(((uint16_t)x1+HOR_OFF)>>8);
    send_data(data);
    data=(uint8_t)((uint16_t)x1+HOR_OFF);
    send_data(data);
    data=(uint8_t)(((uint16_t)x2+HOR_OFF)>>8);
    send_data(data);
    data=(uint8_t)((uint16_t)x2+HOR_OFF);
    send_data(data);
    //设置y轴起点终点
    send_cmd(0x2b);
    data=(uint8_t)(((uint16_t)y1+VER_OFF)>>8);
    send_data(data);
    data=(uint8_t)((uint16_t)y1+VER_OFF);
    send_data(data);
    data=(uint8_t)(((uint16_t)y2+VER_OFF)>>8);
    send_data(data);
    data=(uint8_t)((uint16_t)y2+VER_OFF);
    send_data(data);
    //发送颜色
    send_cmd(0x2c);
    uint32_t length=(x2-x1+1)*(y2-y1+1);
    send_color(color,length);
}

#endif//#ifdef ILI9488_DRIVER_H
