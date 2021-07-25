#include <stdlib.h>
#include "stm32f7xx.h"
#include <rtthread.h>

#pragma  diag_suppress 870 //忽略对汉字打印的警告

//使能GPIO对应的时钟
static void vGpio_SetClock(GPIO_TypeDef * GPIOx)
{
	if (GPIOx == GPIOA)
		__GPIOA_CLK_ENABLE();
	else if (GPIOx == GPIOB)
		__GPIOB_CLK_ENABLE();
	else if (GPIOx == GPIOC)
		__GPIOC_CLK_ENABLE();
	else if (GPIOx == GPIOD)
		__GPIOD_CLK_ENABLE();
	else if (GPIOx == GPIOE)
		__GPIOE_CLK_ENABLE();
	else if (GPIOx == GPIOF)
		__GPIOF_CLK_ENABLE();
#if defined (STM32F769xx) || defined (STM32F779xx)
	else if (GPIOx == GPIOG)
		__GPIOG_CLK_ENABLE();
	else if (GPIOx == GPIOH)
		__GPIOH_CLK_ENABLE();
	else if (GPIOx == GPIOI)
		__GPIOI_CLK_ENABLE();
	else if (GPIOx == GPIOJ)
		__GPIOJ_CLK_ENABLE();
	else if (GPIOx == GPIOK)
		__GPIOK_CLK_ENABLE();
#endif
	else
		rt_kprintf("%s,%d fail \n",__FUNCTION__,__LINE__);
}

//满足一般gpio初始化的函数封装
void gpio_init(GPIO_TypeDef * GPIOx, uint16_t Pinx,uint32_t Mode,uint32_t Pull)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	if(GPIOx == NULL | Pinx == NULL | Mode == NULL | Pull == NULL)
	{
		rt_kprintf("%s fail! param null \n",__FUNCTION__);
		return;
	}

	rt_kprintf("0x%x,0x%x,0x%x,0x%x \n",(uint32_t)GPIOx,Pinx,Mode,Pull);
	vGpio_SetClock(GPIOx);
	GPIO_InitStruct.Pin = Pinx;
	GPIO_InitStruct.Mode = Mode;
	GPIO_InitStruct.Pull  = Pull;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);

	if(GPIO_MODE_OUTPUT_PP == Mode)
	{
		HAL_GPIO_WritePin(GPIOx,Pinx,(GPIO_PinState)(Pull % 2));
	}
}

static void gpio_init_cmd(int argc, char**argv)
{
	GPIO_TypeDef * GPIOx;
	uint32_t Pinx;
	uint32_t Mode;
	uint32_t Pull = GPIO_PULLUP;

	if(argc < 3)
	{
		rt_kprintf("please input:GPIOx<a~k|A~K> Pinx<0~15> mode<i/I|o/U|f/F> pull<H/L>\n");
		return;
	}

	//获取编组
	if(((argv[1][0]) >= 'a') && ((argv[1][0]) <= 'z'))
	{
		GPIOx = (GPIO_TypeDef *)(GPIOA_BASE + (argv[1][0] - 'a') * 1024);
	}
	else if(((argv[1][0]) >= 'A') && ((argv[1][0]) <= 'Z'))
	{
		GPIOx = (GPIO_TypeDef *)(GPIOA_BASE + (argv[1][0] - 'A') * 1024);
	}
	else
	{
		rt_kprintf("input GPIO%c not find \n",argv[1][0]);
		return;
	}

	//获取引脚
	Pinx = atoi(argv[2]);
	if(Pinx > 15)
	{
		rt_kprintf("input Pin%d not find \n",Pinx);
		return;
	}
	Pinx = 1 << Pinx;

	if(3 == argc)
	{
		rt_kprintf("GPIO%c.%d = %d \n",argv[1][0],atoi(argv[2]),HAL_GPIO_ReadPin(GPIOx,Pinx));
		return;
	}

	//获取模式
	if(('i' == argv[3][0]) || ('I' == argv[3][0]))
		Mode = GPIO_MODE_INPUT;
	else if(('o' == argv[3][0]) || ('O' == argv[3][0]))
		Mode = GPIO_MODE_OUTPUT_PP;
	else if(('f' == argv[3][0]) || ('F' == argv[3][0]))
		Mode = GPIO_MODE_AF_PP;
	else
	{
		rt_kprintf("input mode:%s error \n",argv[3]);
		return;
	}

	if(4 == argc)
	{
		gpio_init(GPIOx,Pinx,Mode,Pull);
		rt_kprintf("GPIO%c.%d set mode:%s pullup \n",argv[1][0],atoi(argv[2]),argv[3]);
		return;
	}

	//获取上下拉方式
	if(('h' == argv[4][0]) || ('H' == argv[4][0]))
		Pull = GPIO_PULLUP;
	else if(('l' == argv[4][0]) || ('L' == argv[4][0]))
		Pull = GPIO_PULLDOWN;
	else
	{
		rt_kprintf("input pull mode:%s error \n",argv[4]);
		return;
	}
	
	if(5 == argc)
	{
		gpio_init(GPIOx,Pinx,Mode,Pull);
		rt_kprintf("GPIO%c.%d set mode:%s pull:%s \n",argv[1][0],atoi(argv[2]),argv[3],argv[4]);
		return;
	}

}
MSH_CMD_EXPORT(gpio_init_cmd, gpio control:GPIOx<a~k|A~K> Pinx<0~15> mode<i/I|o/U|f/F> pull<h/H|l/L>);

static void gpio_out_set_cmd(int argc,char**argv)
{
	GPIO_TypeDef * GPIOx;
	GPIO_PinState PinState;
	uint32_t Pinx;
	
	if(argc < 4)
	{
		rt_kprintf("please input:GPIOx<a~k|A~K> Pinx<0~15> state<h/l|H/L|1/0>\n");
		return;
	}

	//获取编组
	if(((argv[1][0]) >= 'a') && ((argv[1][0]) <= 'z'))
	{
		GPIOx = (GPIO_TypeDef *)(GPIOA_BASE + (argv[1][0] - 'a') * 1024);
	}
	else if(((argv[1][0]) >= 'A') && ((argv[1][0]) <= 'Z'))
	{
		GPIOx = (GPIO_TypeDef *)(GPIOA_BASE + (argv[1][0] - 'A') * 1024);
	}
	else
	{
		rt_kprintf("input GPIO%c not find \n",argv[1][0]);
		return;
	}

	//获取引脚
	Pinx = atoi(argv[2]);
	if(Pinx > 15)
	{
		rt_kprintf("input Pin%d not find \n",Pinx);
		return;
	}
	Pinx = 1 << Pinx;

	//获取设置电平
	if(((argv[3][0]) == 'h') || ((argv[3][0]) == 'H') || ((argv[3][0]) == '1'))
	{
		PinState = GPIO_PIN_SET;
	}
	else if(((argv[3][0]) == 'l') || ((argv[3][0]) == 'L') || ((argv[3][0]) == '0'))
	{
		PinState = GPIO_PIN_RESET;
	}
	else
	{
		rt_kprintf("input PinState:%s error \n",argv[3][0]);
		return;
	}

	HAL_GPIO_WritePin(GPIOx,Pinx,PinState);
}
MSH_CMD_EXPORT(gpio_out_set_cmd, gpio out set:GPIOx<a~k|A~K> Pinx<0~15> state<h/l|H/L|1/0>);

static void gpio_state_get_cmd(int argc,char**argv)
{
	GPIO_TypeDef * GPIOx;
	uint32_t Pinx;
	
	if(argc < 3)
	{
		rt_kprintf("please input:GPIOx<a~k|A~K> Pinx<0~15> state<h/l|H/L|1/0> \n");
		return;
	}

	//获取编组
	if(((argv[1][0]) >= 'a') && ((argv[1][0]) <= 'z'))
	{
		GPIOx = (GPIO_TypeDef *)(GPIOA_BASE + (argv[1][0] - 'a') * 1024);
	}
	else if(((argv[1][0]) >= 'A') && ((argv[1][0]) <= 'Z'))
	{
		GPIOx = (GPIO_TypeDef *)(GPIOA_BASE + (argv[1][0] - 'A') * 1024);
	}
	else
	{
		rt_kprintf("input GPIO%c not find \n",argv[1][0]);
		return;
	}

	//获取引脚
	Pinx = atoi(argv[2]);
	if(Pinx > 15)
	{
		rt_kprintf("input Pin%d not find \n",Pinx);
		return;
	}
	Pinx = 1 << Pinx;

	rt_kprintf("GPIO%c.%d = %d \n",argv[1][0],atoi(argv[2]),HAL_GPIO_ReadPin(GPIOx,Pinx));
}
MSH_CMD_EXPORT(gpio_state_get_cmd, gpio state get:GPIOx<a~k|A~K> Pinx<0~15> );



