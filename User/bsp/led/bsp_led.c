#include "./led/bsp_led.h"

/** 控制LED灯亮灭的宏，
	* LED低电平亮，设置ON=0，OFF=1
	* 若LED高电平亮，把宏设置成ON=1 ，OFF=0 即可
	*/
#define ON  GPIO_PIN_RESET
#define OFF GPIO_PIN_SET

/**
 * @brief  初始化控制LED的IO
 * @param  无
 * @retval 无
 */
void LED_GPIO_Config(void)
{

	/*定义一个GPIO_InitTypeDef类型的结构体*/
	GPIO_InitTypeDef  GPIO_InitStruct;

	/*开启LED相关的GPIO外设时钟*/
	LED1_GPIO_CLK_ENABLE();
	LED2_GPIO_CLK_ENABLE();
	LED3_GPIO_CLK_ENABLE();
	LED4_GPIO_CLK_ENABLE();

	/*选择要控制的GPIO引脚*/
	GPIO_InitStruct.Pin = LED1_PIN;

	/*设置引脚的输出类型为推挽输出*/
	GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;

	/*设置引脚为上拉模式*/
	GPIO_InitStruct.Pull  = GPIO_PULLUP;

	/*设置引脚速率为高速 */
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;

	/*调用库函数，使用上面配置的GPIO_InitStructure初始化GPIO*/
	HAL_GPIO_Init(LED1_GPIO_PORT, &GPIO_InitStruct);

	/*选择要控制的GPIO引脚*/
	GPIO_InitStruct.Pin = LED2_PIN;
	HAL_GPIO_Init(LED2_GPIO_PORT, &GPIO_InitStruct);

	/*选择要控制的GPIO引脚*/
	GPIO_InitStruct.Pin = LED3_PIN;
	HAL_GPIO_Init(LED3_GPIO_PORT, &GPIO_InitStruct);

	/*选择要控制的GPIO引脚*/
	GPIO_InitStruct.Pin = LED4_PIN;
	HAL_GPIO_Init(LED4_GPIO_PORT, &GPIO_InitStruct);

	/*关闭RGB灯*/
	LED_RGBOFF;

	/*指示灯默认开启*/
	LED4(ON);

}
/*********************************************END OF FILE**********************/
