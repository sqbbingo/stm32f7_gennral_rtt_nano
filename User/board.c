/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-05-24                  the first version
 */

#include <rthw.h>
#include <rtthread.h>
#include "board.h"
#include "sdram.h"
#include "lcd.h"

#if defined(RT_USING_USER_MAIN) && defined(RT_USING_HEAP)
/*
 * Please modify RT_HEAP_SIZE if you enable RT_USING_HEAP
 * the RT_HEAP_SIZE max value = (sram size - ZI size), 1024 means 1024 bytes
 */
#define RT_HEAP_SIZE (10*1024)
static rt_uint8_t rt_heap[RT_HEAP_SIZE];
//#define RT_HEAP_SIZE (32*1024*1024)
//static rt_uint8_t rt_heap[RT_HEAP_SIZE] __attribute__((at(0XC0000000)));

RT_WEAK void *rt_heap_begin_get(void)
{
	return rt_heap;
}

RT_WEAK void *rt_heap_end_get(void)
{
	return rt_heap + RT_HEAP_SIZE;
}
#endif

void SysTick_Handler(void)
{
	rt_interrupt_enter();

	rt_tick_increase();

	rt_interrupt_leave();
}

/**
	* @brief  System Clock 配置
	*         system Clock 配置如下 :
	*            System Clock source            = PLL (HSE)
	*            SYSCLK(Hz)                     = 216000000
	*            HCLK(Hz)                       = 216000000
	*            AHB Prescaler                  = 1
	*            APB1 Prescaler                 = 4
	*            APB2 Prescaler                 = 2
	*            HSE Frequency(Hz)              = 25000000
	*            PLL_M                          = 25
	*            PLL_N                          = 432
	*            PLL_P                          = 2
	*            PLL_Q                          = 9
	*            VDD(V)                         = 3.3
	*            Main regulator output voltage  = Scale1 mode
	*            Flash Latency(WS)              = 7
	* @param  无
	* @retval 无
	*/
static void SystemClock_Config(void)
{
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_OscInitTypeDef RCC_OscInitStruct;
	HAL_StatusTypeDef ret = HAL_OK;

	/* 使能HSE，配置HSE为PLL的时钟源，配置PLL的各种分频因子M N P Q
	 * PLLCLK = HSE/M*N/P = 25M / 25 *432 / 2 = 216M
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 25;
	RCC_OscInitStruct.PLL.PLLN = 432;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 9;

	ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
	if (ret != HAL_OK)
	{
		while (1) { ; }
	}

	/* 激活 OverDrive 模式以达到216M频率  */
	ret = HAL_PWREx_EnableOverDrive();
	if (ret != HAL_OK)
	{
		while (1) { ; }
	}

	/* 选择PLLCLK作为SYSCLK，并配置 HCLK, PCLK1 and PCLK2 的时钟分频因子
	 * SYSCLK = PLLCLK     = 216M
	 * HCLK   = SYSCLK / 1 = 216M
	 * PCLK2  = SYSCLK / 2 = 108M
	 * PCLK1  = SYSCLK / 4 = 54M
	 */
	RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	/* 在HAL_RCC_ClockConfig函数里面同时初始化好了系统定时器systick，配置为1ms中断一次 */
	ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7);
	if (ret != HAL_OK)
	{
		while (1) { ; }
	}
}


/**
 * This function will initial your board.
 */
void rt_hw_board_init(void)
{
	SystemClock_Config();

	/* 初始化SysTick */
	HAL_SYSTICK_Config( HAL_RCC_GetSysClockFreq() / RT_TICK_PER_SECOND );

	/* 硬件BSP初始化统统放在这里，比如LED，串口，LCD等 */
#ifdef RT_USING_CONSOLE
	DEBUG_USART_Config();
#endif
	HAL_Init();
	delay_init(216);                //延时函数初始化
	SDRAM_Init();
	/* LED 端口初始化 */
	LED_GPIO_Config();
	LCD_Init();
	RTC_Init();
	RTC_Set_WakeUp(RTC_WAKEUPCLOCK_CK_SPRE_16BITS, 0);
	adc_init();
	IIC_Init();
	PCF8574_Init();
	MPU9250_Init();

	/*
	 * TODO 1: OS Tick Configuration
	 * Enable the hardware timer and call the rt_os_tick_callback function
	 * periodically with the frequency RT_TICK_PER_SECOND.
	 */

	/* Call components board initial (use INIT_BOARD_EXPORT()) */
#ifdef RT_USING_COMPONENTS_INIT
	rt_components_board_init();
#endif

#if defined(RT_USING_USER_MAIN) && defined(RT_USING_HEAP)
	rt_system_heap_init(rt_heap_begin_get(), rt_heap_end_get());
#endif
}


