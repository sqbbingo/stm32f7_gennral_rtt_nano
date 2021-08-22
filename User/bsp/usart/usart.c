/**
	******************************************************************************
	* @file    bsp_debug_usart.c
	* @author  bingo
	* @version V1.0
	* @date    2021-07-25
	* @brief   使用串口1，重定向c库printf函数到usart端口，中断接收模式
	******************************************************************************
	*/

#include <rtthread.h>
#include "usart.h"
#include "ringbuffer.h"

#define DEBUG_UART_RX_BUF_LEN 16

UART_HandleTypeDef DebugUartHandle;
rt_uint8_t debug_uart_rx_buf[DEBUG_UART_RX_BUF_LEN] = {0};
struct rt_ringbuffer  debug_uart_rxcb;         /* 定义一个 ringbuffer cb */
static struct rt_semaphore shell_rx_sem; /* 定义一个静态信号量 */
extern uint8_t ucTemp;
/**
* @brief  DEBUG_USART GPIO 配置,工作模式配置。115200 8-N-1
* @param  无
* @retval 无
*/
void DEBUG_USART_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	RCC_PeriphCLKInitTypeDef RCC_PeriphClkInit;

	/* 初始化串口接收 ringbuffer  */
	rt_ringbuffer_init(&debug_uart_rxcb, debug_uart_rx_buf, DEBUG_UART_RX_BUF_LEN);

	/* 初始化串口接收数据的信号量 */
	rt_sem_init(&(shell_rx_sem), "shell_rx", 0, 0);

	/* 配置串口1时钟源*/
	RCC_PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_UARTx;
	RCC_PeriphClkInit.Usart1ClockSelection = RCC_UARTxCLKSOURCE_SYSCLK;
	HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphClkInit);
	/* 使能 UART 时钟 */
	DEBUG_USART_CLK_ENABLE();

	DEBUG_USART_RX_GPIO_CLK_ENABLE();
	DEBUG_USART_TX_GPIO_CLK_ENABLE();

	/**USART1 GPIO Configuration
	PA9     ------> USART1_TX
	PA10    ------> USART1_RX
	*/
	/* 配置Tx引脚为复用功能  */
	GPIO_InitStruct.Pin = DEBUG_USART_TX_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = DEBUG_USART_TX_AF;
	HAL_GPIO_Init(DEBUG_USART_TX_GPIO_PORT, &GPIO_InitStruct);

	/* 配置Rx引脚为复用功能 */
	GPIO_InitStruct.Pin = DEBUG_USART_RX_PIN;
	GPIO_InitStruct.Alternate = DEBUG_USART_RX_AF;
	HAL_GPIO_Init(DEBUG_USART_RX_GPIO_PORT, &GPIO_InitStruct);

	/* 配置串DEBUG_USART 模式 */
	DebugUartHandle.Instance = DEBUG_USART;
	DebugUartHandle.Init.BaudRate = 115200;
	DebugUartHandle.Init.WordLength = UART_WORDLENGTH_8B;
	DebugUartHandle.Init.StopBits = UART_STOPBITS_1;
	DebugUartHandle.Init.Parity = UART_PARITY_NONE;
	DebugUartHandle.Init.Mode = UART_MODE_TX_RX;
	DebugUartHandle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	DebugUartHandle.Init.OverSampling = UART_OVERSAMPLING_16;
	DebugUartHandle.Init.OneBitSampling = UART_ONEBIT_SAMPLING_DISABLED;
	DebugUartHandle.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	HAL_UART_Init(&DebugUartHandle);

	/*串口1中断初始化 */
	HAL_NVIC_SetPriority(DEBUG_USART_IRQ, 3, 3);
	HAL_NVIC_EnableIRQ(DEBUG_USART_IRQ);
	/*配置串口接收中断 */
	__HAL_UART_ENABLE_IT(&DebugUartHandle, UART_IT_RXNE);
}


/*****************  发送字符串 **********************/
void Usart_SendString(uint8_t *str)
{
	unsigned int k = 0;
	do
	{
		HAL_UART_Transmit(&DebugUartHandle, (uint8_t *)(str + k) , 1, 1000);
		k++;
	} while (*(str + k) != '\0');

}
//加入以下代码,支持printf函数,而不需要选择use MicroLIB
//#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#if 1
#pragma import(__use_no_semihosting)
//标准库需要的支持函数
struct __FILE
{
	int handle;
};

FILE __stdout;
//定义_sys_exit()以避免使用半主机模式
void _sys_exit(int x)
{
	x = x;
}
//重定义fputc函数
int fputc(int ch, FILE *f)
{
	while ((USART1->ISR & 0X40) == 0); //循环发送,直到发送完毕
	USART1->TDR = (uint8_t)ch;
	return ch;
}
#else
///重定向c库函数printf到串口DEBUG_USART，重定向后可使用printf函数
int fputc(int ch, FILE *f)
{
	/* 发送一个字节数据到串口DEBUG_USART */
	HAL_UART_Transmit(&DebugUartHandle, (uint8_t *)&ch, 1, 1000);

	return (ch);
}

///重定向c库函数scanf到串口DEBUG_USART，重写向后可使用scanf、getchar等函数
int fgetc(FILE *f)
{

	int ch;
	HAL_UART_Receive(&DebugUartHandle, (uint8_t *)&ch, 1, 1000);
	return (ch);
}
#endif

void rt_hw_console_output(const char *str)
{
	/* 进入临界段 */
	rt_enter_critical();

	/* 直到字符串结束 */
	while (*str != '\0')
	{
		/* 换行 */
		if (*str == '\n')
		{
			//			HAL_UART_Transmit( &DebugUartHandle,(uint8_t *)'\r',1,1000);
		}
		HAL_UART_Transmit( &DebugUartHandle, (uint8_t *)(str++), 1, 1000);
	}

	/* 退出临界段 */
	rt_exit_critical();

}

/* 移植 FinSH，实现命令行交互, 需要添加 FinSH 源码，然后再对接 rt_hw_console_getchar */
/* 中断方式 */
char rt_hw_console_getchar(void)
{
	char ch = 0;

	/* 从 ringbuffer 中拿出数据 */
	while (rt_ringbuffer_getchar(&debug_uart_rxcb, (rt_uint8_t *)&ch) != 1)
	{
		rt_sem_take(&shell_rx_sem, RT_WAITING_FOREVER);
	}
	return ch;
}

/* uart 中断 */
void DEBUG_USART_IRQHandler(void)
{
	int ch = -1;
	/* enter interrupt */
	rt_interrupt_enter();          //在中断中一定要调用这对函数，进入中断

	if ((__HAL_UART_GET_FLAG(&(DebugUartHandle), UART_FLAG_RXNE) != RESET) &&
	        (__HAL_UART_GET_IT_SOURCE(&(DebugUartHandle), UART_IT_RXNE) != RESET))
	{
		while (1)
		{
			ch = -1;
			if (__HAL_UART_GET_FLAG(&(DebugUartHandle), UART_FLAG_RXNE) != RESET)
			{
				ch =  DebugUartHandle.Instance->RDR & 0xff;
			}
			if (ch == -1)
			{
				break;
			}
			/* 读取到数据，将数据存入 ringbuffer */
			rt_ringbuffer_putchar(&debug_uart_rxcb, ch);
		}
		rt_sem_release(&shell_rx_sem);
	}

	/* leave interrupt */
	rt_interrupt_leave();    //在中断中一定要调用这对函数，离开中断
}
/*********************************************END OF FILE**********************/
