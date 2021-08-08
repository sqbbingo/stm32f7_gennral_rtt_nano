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
	                      2*1024,                 /* 线程栈大小 */
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
	                      2*1024,                 /* 线程栈大小 */
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
/*
	int state = 1;
	while(state)
	{
		state = mpu_dmp_init();
		rt_kprintf("mpu_dmp_init fail state:%d \r\n",state);
		rt_thread_mdelay(400);
	}
	rt_kprintf("mpu_dmp_init successful \r\n");
*/
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
    RTC_TimeTypeDef RTC_TimeStruct;
    RTC_DateTypeDef RTC_DateStruct;

  	uint8_t lcd_id[12];
  	uint8_t tbuf[40];
  	short temp;

	POINT_COLOR=RED; 
	sprintf((char*)lcd_id,"LCD ID:%04X",lcddev.id);//将LCD ID打印到lcd_id数组
    while(1)
    {
		POINT_COLOR=RED;	  
		LCD_ShowString(10,40,260,32,32,"Apollo STM32F4/F7"); 	
		LCD_ShowString(10,80,240,24,24,"LTDC TEST");
		LCD_ShowString(10,110,240,16,16,"ATOM@ALIENTEK");
 		LCD_ShowString(10,130,240,16,16,lcd_id);		//显示LCD ID	      					 

		RTC_Get_Time(&RTC_TimeStruct,RTC_FORMAT_BIN);
		sprintf((char*)tbuf,"Time:%02d:%02d:%02d",RTC_TimeStruct.Hours,RTC_TimeStruct.Minutes,RTC_TimeStruct.Seconds); 
		LCD_ShowString(10,150,210,16,16,tbuf);	
		RTC_Get_Date(&RTC_DateStruct,RTC_FORMAT_BIN);
		sprintf((char*)tbuf,"Date:20%02d-%02d-%02d",RTC_DateStruct.Year,RTC_DateStruct.Month,RTC_DateStruct.Date); 
		LCD_ShowString(10,170,210,16,16,tbuf);	
		sprintf((char*)tbuf,"Week:%d",RTC_DateStruct.WeekDay); 
		LCD_ShowString(10,190,210,16,16,tbuf);   

		LCD_ShowString(10,210,200,16,16,"TEMPERATE: 00.00C");
		temp=Get_Temprate(); //得到温度值
		if(temp<0)
		{
			temp=-temp;
			LCD_ShowString(10+10*8,210,16,16,16,"-"); //显示负号
		}
		else 
			LCD_ShowString(10+10*8,210,16,16,16," "); //无符号
		LCD_ShowxNum(10+11*8,210,temp/100,2,16,0); //显示整数部分
		LCD_ShowxNum(10+14*8,210,temp%100,2,16,0); //显示小数部分


		rt_thread_mdelay(1000);	
	}

}


/********************************END OF FILE****************************/
