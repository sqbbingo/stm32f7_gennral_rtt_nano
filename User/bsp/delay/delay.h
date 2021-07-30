#ifndef _DELAY_H
#define _DELAY_H

#include "stm32f7xx.h"

void delay_init(uint8_t SYSCLK);
void delay_ms(uint16_t nms);
void delay_us(uint32_t nus);
#ifdef OS_RT_THREAD
void rt_thread_udelay(uint32_t nus);
#endif

#endif

