#ifndef _TIMER_HAL_H
#define _TIMER_HAL_H
#include "sys.h"

extern TIM_HandleTypeDef TIM3_Handler;      //定时器3PWM句柄
extern TIM_OC_InitTypeDef TIM3_CH4Handler;  //定时器3通道4句柄

void TIM3_Init(u16 arr, u16 psc);
void TIM3_PWM_Init(u16 arr, u16 psc,u32 compare);
void TIM_SetTIM3Compare4(u32 compare);
#endif

