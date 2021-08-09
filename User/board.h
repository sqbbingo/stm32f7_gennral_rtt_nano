#ifndef __BOARD_H__
#define __BOARD_H__

/*
*************************************************************************
*                             包含的头文件
*************************************************************************
*/
/* STM32 固件库头文件 */
#include "stm32f7xx.h"
#include "main.h"
/* 开发板硬件bsp头文件 */
#include "./led/bsp_led.h"
#include "./usart/bsp_debug_usart.h"
#include "delay.h"
#include "rtc.h"
#include "adc.h"
#include "myiic.h"
#include "pcf8574.h"
#include "mpu.h"
#include "mpu9250.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h" 
#include "malloc.h"
#include "w25qxx.h"
#include "nand.h"
#include "sdmmc_sdcard.h"

/*
*************************************************************************
*                               函数声明
*************************************************************************
*/
void rt_hw_board_init(void);
void SysTick_Handler(void);

#endif /* __BOARD_H__ */
