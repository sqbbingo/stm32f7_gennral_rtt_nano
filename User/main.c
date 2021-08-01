/*
* @Author: s
* @Date:   2021-07-25 11:48:54
* @Last Modified by:   s
* @Last Modified time: 2021-07-25 11:49:26
*/

/*
*************************************************************************
*                             包含的头文件
*************************************************************************
*/
#include "board.h"
#include "rtthread.h"
#include "lcd.h"

/*
*************************************************************************
*                               变量
*************************************************************************
*/
/* 定义线程控制块 */
static rt_thread_t led1_thread = RT_NULL;
static rt_thread_t lcd_thread = RT_NULL;

/*
*************************************************************************
*                             函数声明
*************************************************************************
*/
static void led1_thread_entry(void* parameter);
static void lcd_thread_entry(void* parameter);

/*
*************************************************************************
*                             main 函数
*************************************************************************
*/
/**
  * @brief  主函数
  * @param  无
  * @retval 无
  */
int main(void)
{
	/*
	* 开发板硬件初始化，RTT系统初始化已经在main函数之前完成，
	* 即在component.c文件中的rtthread_startup()函数中完成了。
	* 所以在main函数中，只需要创建线程和启动线程即可。
	*/

	led1_thread =                          /* 线程控制块指针 */
	    rt_thread_create( "led1",              /* 线程名字 */
	                      led1_thread_entry,   /* 线程入口函数 */
	                      RT_NULL,             /* 线程入口函数参数 */
	                      512,                 /* 线程栈大小 */
	                      3,                   /* 线程的优先级 */
	                      20);                 /* 线程时间片 */

	/* 启动线程，开启调度 */
	if (led1_thread != RT_NULL)
		rt_thread_startup(led1_thread);
	else
		return -1;

	lcd_thread =                          /* 线程控制块指针 */
	    rt_thread_create( "lcd",              /* 线程名字 */
	                      lcd_thread_entry,   /* 线程入口函数 */
	                      RT_NULL,             /* 线程入口函数参数 */
	                      512,                 /* 线程栈大小 */
	                      3,                   /* 线程的优先级 */
	                      20);                 /* 线程时间片 */

	/* 启动线程，开启调度 */
	if (lcd_thread != RT_NULL)
		rt_thread_startup(lcd_thread);
	else
	{
		rt_kprintf("lcd thread create fail! \r\n");
		return -1;
	}

}

/*
*************************************************************************
*                             线程定义
*************************************************************************
*/

static void led1_thread_entry(void* parameter)
{
	while (1)
	{
		LED1_ON;
		rt_thread_delay(500);   /* 延时500个tick */
//        rt_kprintf("led1_thread running,LED1_ON\r\n");

		LED1_OFF;
		rt_thread_delay(500);   /* 延时500个tick */
//        rt_kprintf("led1_thread running,LED1_OFF\r\n");
	}
}

static void lcd_thread_entry(void* parameter)
{
    uint8_t x=0;
  	uint8_t lcd_id[12];

	POINT_COLOR=RED; 
	sprintf((char*)lcd_id,"LCD ID:%04X",lcddev.id);//将LCD ID打印到lcd_id数组
    while(1)
    {
        switch(x)
		{
			case 0:LCD_Clear(WHITE);break;
			case 1:LCD_Clear(BLACK);break;
			case 2:LCD_Clear(BLUE);break;
			case 3:LCD_Clear(RED);break;
			case 4:LCD_Clear(MAGENTA);break;
			case 5:LCD_Clear(GREEN);break;
			case 6:LCD_Clear(CYAN);break; 
			case 7:LCD_Clear(YELLOW);break;
			case 8:LCD_Clear(BRRED);break;
			case 9:LCD_Clear(GRAY);break;
			case 10:LCD_Clear(LGRAY);break;
			case 11:LCD_Clear(BROWN);break;
		}
		POINT_COLOR=RED;	  
		LCD_ShowString(10,40,260,32,32,"Apollo STM32F4/F7"); 	
		LCD_ShowString(10,80,240,24,24,"LTDC TEST");
		LCD_ShowString(10,110,240,16,16,"ATOM@ALIENTEK");
 		LCD_ShowString(10,130,240,16,16,lcd_id);		//显示LCD ID	      					 
		LCD_ShowString(10,150,240,12,12,"2016/7/12");	      					 
	    x++;
		if(x==12)x=0;      

		rt_thread_mdelay(1000);	
	}

}


/********************************END OF FILE****************************/
