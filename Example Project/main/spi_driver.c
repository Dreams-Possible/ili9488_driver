#include"spi_driver.h"
//启用spi_driver
#ifdef SPI_DRIVER_H

//配置
#define COM SPI2_HOST
#define SCLK 12
#define MOSI 11
#define MISO 13

//SPI初始化
void spi_init();

//SPI初始化
void spi_init()
{
    spi_bus_config_t config={0};
    config.mosi_io_num=MOSI;
    config.miso_io_num=MISO;
    config.sclk_io_num=SCLK;
    config.quadwp_io_num=-1;
    config.quadhd_io_num=-1;
    spi_bus_initialize(COM,&config,SPI_DMA_CH_AUTO);
}

#endif//#ifdef SPI_DRIVER_H
