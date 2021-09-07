#include <stdlib.h>
#include <rtthread.h>
#include "timer_hal.h"

static TIM_HandleTypeDef TIM_Handler;         //定时器句柄
static TIM_OC_InitTypeDef TIM_CHHandler;     //定时器通道句柄

//TIM3 PWM部分初始化
//PWM输出初始化
//arr：自动重装值
//psc：时钟预分频数
void TIM3_PWM_Init(u16 arr, u16 psc,u32 compare)
{
	TIM_Handler.Instance = TIM3;          //定时器3
	TIM_Handler.Init.Prescaler = psc;     //定时器分频
	TIM_Handler.Init.CounterMode = TIM_COUNTERMODE_UP; //向上计数模式
	TIM_Handler.Init.Period = arr;        //自动重装载值
	TIM_Handler.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	HAL_TIM_PWM_Init(&TIM_Handler);       //初始化PWM

	TIM_CHHandler.OCMode = TIM_OCMODE_PWM1; //模式选择PWM1
	TIM_CHHandler.Pulse = compare;        //设置比较值,此值用来确定占空比，
	//默认比较值为自动重装载值的一半,即占空比为50%
	TIM_CHHandler.OCPolarity = TIM_OCPOLARITY_LOW; //输出比较极性为低
	HAL_TIM_PWM_ConfigChannel(&TIM_Handler, &TIM_CHHandler, TIM_CHANNEL_4); //配置TIM3通道4
	HAL_TIM_PWM_Start(&TIM_Handler, TIM_CHANNEL_4); //开启PWM通道4

	TIM_Handler.Instance = TIM1;          //定时器1
	HAL_TIM_PWM_Init(&TIM_Handler); 	  //初始化PWM

	HAL_TIM_PWM_ConfigChannel(&TIM_Handler, &TIM_CHHandler, TIM_CHANNEL_1); //配置TIM1通道1
	HAL_TIM_PWM_Start(&TIM_Handler, TIM_CHANNEL_1); //开启PWM通道1
}

static void tim3_pwm_set(int argc,char **argv)
{
	if(argc < 4)
	{
		rt_kprintf("tim3 ch4 PB1 set:arr psc compare \r\n");
		return;
	}
	TIM3_PWM_Init(atoi(argv[1]), atoi(argv[2]),atoi(argv[3]));
	printf("tim3 ch4 PB1 set:%d %d %d \r\n",atoi(argv[1]), atoi(argv[2]),atoi(argv[3]));
	printf("tim3 ch4 PB1 fre=%.2fHz pwm=%.4f%% \r\n",(float)(108 / (atoi(argv[2]) + 1) * 1000000 / (atoi(argv[1]) + 1)) ,(float)(100 * atoi(argv[3])/atoi(argv[1])));
}
MSH_CMD_EXPORT(tim3_pwm_set, tim3 pwm set);


//定时器底层驱动，时钟使能，引脚配置
//此函数会被HAL_TIM_PWM_Init()调用
//htim:定时器句柄
void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim)
{
	GPIO_InitTypeDef GPIO_Initure;
	if(htim->Instance == TIM3)
	{
		__HAL_RCC_TIM3_CLK_ENABLE();            //使能定时器3
		__HAL_RCC_GPIOB_CLK_ENABLE();           //开启GPIOB时钟

		GPIO_Initure.Pin = GPIO_PIN_1;          //PB1
		GPIO_Initure.Mode = GPIO_MODE_AF_PP;    //复用推完输出
		GPIO_Initure.Pull = GPIO_PULLUP;        //上拉
		GPIO_Initure.Speed = GPIO_SPEED_HIGH;   //高速
		GPIO_Initure.Alternate = GPIO_AF2_TIM3; //PB1复用为TIM3_CH4
		HAL_GPIO_Init(GPIOB, &GPIO_Initure);
	}
	else if(htim->Instance == TIM1)
	{
		__HAL_RCC_TIM1_CLK_ENABLE();            //使能定时器1
		__HAL_RCC_GPIOA_CLK_ENABLE();           //开启GPIOB时钟

		GPIO_Initure.Pin = GPIO_PIN_8;          //PB1
		GPIO_Initure.Mode = GPIO_MODE_AF_PP;    //复用推完输出
		GPIO_Initure.Pull = GPIO_PULLUP;        //上拉
		GPIO_Initure.Speed = GPIO_SPEED_HIGH;   //高速
		GPIO_Initure.Alternate = GPIO_AF1_TIM1; //PA8复用为TIM1_CH1
		HAL_GPIO_Init(GPIOA, &GPIO_Initure);
	}
}

//设置TIM通道4的占空比
//compare:比较值
void TIM_SetTIM3Compare4(u32 compare)
{
	TIM3->CCR4 = compare;
}
