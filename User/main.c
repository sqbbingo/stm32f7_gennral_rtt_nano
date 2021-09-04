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
#include "rthw.h"
#include "rtthread.h"
#include "lcd.h"
#include "lwip_comm.h"

#include "usbd_msc_core.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "usb_conf.h"
#include "usbd_msc_bot.h"

/*
*************************************************************************
*                               变量
*************************************************************************
*/
/* 定义线程控制块 */
static rt_thread_t led1_thread = RT_NULL;
static rt_thread_t lcd_thread = RT_NULL;
static rt_thread_t lwip_thread = RT_NULL;
static rt_thread_t usb_msc_thread = RT_NULL;


USB_OTG_CORE_HANDLE USB_OTG_dev;
extern vu8 USB_STATUS_REG;		//USB状态
extern vu8 bDeviceState;		//USB连接 情况

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
  	u32 total,free;
  	short temp;

	POINT_COLOR=RED; 
	sprintf((char*)lcd_id,"LCD ID:%04X",lcddev.id);//将LCD ID打印到lcd_id数组  
	LCD_ShowString(10,40,260,32,32,"bigo test");
	exf_getfree("1:", &total, &free);
	LCD_ShowString(10,80,240,5,16,"Flash total:  MB free:  MB");
	LCD_ShowNum(10 + 8 * 12, 80, total >> 10, 2, 16);	//显示flash总容量 MB
	LCD_ShowNum(10 + 8 * 22, 80, free >> 10, 2, 16); //显示flash剩余容量 MB

	exf_getfree("2:", &total, &free);
	LCD_ShowString(10,100,240,5,16,"Nand total:   MB free:   MB");
	LCD_ShowNum(10 + 8 * 11, 100, total >> 10, 3, 16);	//显示nand总容量 MB
	LCD_ShowNum(10 + 8 * 22, 100, free >> 10, 3, 16); //显示nand剩余容量 MB

	LCD_ShowString(10,130,240,16,16,lcd_id);		//显示LCD ID				

    while(1)
    {
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

static void lwip_thread_entry(void* parameter)
{
	while(lwip_comm_init()) 	    //lwip初始化
	{
		LCD_ShowString(10,230,200,20,16,"Lwip Init failed!"); 	//lwip初始化失败
		rt_thread_mdelay(500);
		LCD_Fill(10,230,230,150,WHITE);
		rt_thread_mdelay(500);
	}

}

static void usb_msc_thread_entry(void* parameter)
{
	u8 offline_cnt=0;
	u8 tct=0;
	u8 USB_STA;
	u8 Divece_STA;
//	rt_ubase_t level;


	MSC_BOT_Data=mymalloc(SRAMIN,MSC_MEDIA_PACKET); 		//申请内存

//	level = rt_hw_interrupt_disable();
	USBD_Init(&USB_OTG_dev,USB_OTG_FS_CORE_ID,&USR_desc,&USBD_MSC_cb,&USR_cb);
//	rt_hw_interrupt_enable(level);
	rt_thread_mdelay(1800);

	while(1)
	{
		rt_thread_mdelay(1);		  
		if(USB_STA!=USB_STATUS_REG)//状态改变了 
		{							   
			LCD_Fill(30,210,240,210+16,WHITE);//清除显示				   
			if(USB_STATUS_REG&0x01)//正在写		  
			{
				LCD_ShowString(30,250,200,16,16,"USB Writing...");//提示USB正在写入数据  
			}
			if(USB_STATUS_REG&0x02)//正在读
			{
				LCD_ShowString(30,250,200,16,16,"USB Reading...");//提示USB正在读出数据 		 
			}											  
			if(USB_STATUS_REG&0x04)LCD_ShowString(30,270,200,16,16,"USB Write Err ");//提示写入错误
			else LCD_Fill(30,270,240,230+16,WHITE);//清除显示	  
			if(USB_STATUS_REG&0x08)LCD_ShowString(30,290,200,16,16,"USB Read  Err ");//提示读出错误
			else LCD_Fill(30,290,240,250+16,WHITE);//清除显示	 
			USB_STA=USB_STATUS_REG;//记录最后的状态
		}
		if(Divece_STA!=bDeviceState) 
		{
			if(bDeviceState==1)LCD_ShowString(30,230,200,16,16,"USB Connected	 ");//提示USB连接已经建立
			else LCD_ShowString(30,230,200,16,16,"USB DisConnected ");//提示USB被拔出了
			Divece_STA=bDeviceState;
		}
		tct++;
		if(tct==200)
		{
			tct=0;
			if(USB_STATUS_REG&0x10)
			{
				offline_cnt=0;//USB连接了,则清除offline计数器
				bDeviceState=1;
			}else//没有得到轮询 
			{
				offline_cnt++;	
				if(offline_cnt>10)bDeviceState=0;//2s内没收到在线标记,代表USB被拔出了
			}
			USB_STATUS_REG=0;
		} 
	}		
}

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
#if 0
	lwip_thread = rt_thread_create("lwip", 
								   lwip_thread_entry,
								   RT_NULL,
								   2 * 1024,
								   3,
								   30);
	/* 启动线程，开启调度 */
	if (lwip_thread != RT_NULL)
		rt_thread_startup(lwip_thread);
	else
	{
		rt_kprintf("lwip thread create fail! \r\n");
		return -1;
	}
#endif
}

int lwip_sample(void)
{
	lwip_thread = rt_thread_create("lwip", 
								   lwip_thread_entry,
								   RT_NULL,
								   2 * 1024,
								   3,
								   30);
	/* 启动线程，开启调度 */
	if (lwip_thread != RT_NULL)
		rt_thread_startup(lwip_thread);
	else
	{
		rt_kprintf("lwip thread create fail! \r\n");
		return -1;
	}
	
	return 0;
}

MSH_CMD_EXPORT(lwip_sample, lwip sample);


int usb_msc(void)
{
	usb_msc_thread = rt_thread_create("usb_msc", 
								   usb_msc_thread_entry,
								   RT_NULL,
								   4 * 1024,
								   1,
								   10);
	/* 启动线程，开启调度 */
	if (usb_msc_thread != RT_NULL)
		rt_thread_startup(usb_msc_thread);
	else
	{
		rt_kprintf("usb_msc thread create fail! \r\n");
		return -1;
	}
	
	return 0;
}

MSH_CMD_EXPORT(usb_msc, usb_msc_sample);

/********************************END OF FILE****************************/
