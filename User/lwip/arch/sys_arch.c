/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
/*  Porting by Michael Vysotsky <michaelvy@hotmail.com> August 2011   */

#define SYS_ARCH_GLOBALS

/* lwIP includes. */
#include "lwip/debug.h"
#include "lwip/def.h"
#include "lwip/lwip_sys.h"
#include "lwip/mem.h"
#include "delay.h"
#include "arch/sys_arch.h"
#include "malloc.h"
#include <string.h>

static struct rt_thread tcp_ip_thread;

//当消息指针为空时,指向一个常量pvNullPointer所指向的值.
//在UCOS中如果OSQPost()中的msg==NULL会返回一条OS_ERR_POST_NULL
//错误,而在lwip中会调用sys_mbox_post(mbox,NULL)发送一条空消息,我们
//在本函数中把NULL变成一个常量指针0Xffffffff
const void * const pvNullPointer = (mem_ptr_t*)0xffffffff;


//创建一个消息邮箱
//*mbox:消息邮箱
//size:邮箱大小
//返回值:ERR_OK,创建成功
//         其他,创建失败
err_t sys_mbox_new(sys_mbox_t *mbox, int size)
{
	if (size > MAX_QUEUE_ENTRIES)
		size = MAX_QUEUE_ENTRIES;		//消息队列最多容纳MAX_QUEUE_ENTRIES消息数目
		
	mbox->pQ = rt_mq_create("lwip_mq",size,MAX_QUEUES,RT_IPC_FLAG_PRIO); //使用rtthread创建一个消息队列
	LWIP_ASSERT("OSQCreate", mbox->pQ != NULL);
	if (mbox->pQ != NULL)
		return ERR_OK; //返回ERR_OK,表示消息队列创建成功 ERR_OK=0
	else 
		return ERR_MEM;  				//消息队列创建错误
}

//释放并删除一个消息邮箱
//*mbox:要删除的消息邮箱
void sys_mbox_free(sys_mbox_t *mbox)
{
	u8_t ucErr;
	rt_mq_delete(mbox->pQ);
	LWIP_ASSERT( "OSQDel ", ucErr == RT_EOK );
	mbox = NULL;
}

//向消息邮箱中发送一条消息(必须发送成功)
//*mbox:消息邮箱
//*msg:要发送的消息
void sys_mbox_post(sys_mbox_t *mbox, void *msg)
{
	if (msg == NULL)
		msg = (void*)&pvNullPointer; //当msg为空时 msg等于pvNullPointer指向的值
	while (rt_mq_send(mbox->pQ, &msg,4) != RT_EOK); //死循环等待消息发送成功
}

//尝试向一个消息邮箱发送消息
//此函数相对于sys_mbox_post函数只发送一次消息，
//发送失败后不会尝试第二次发送
//*mbox:消息邮箱
//*msg:要发送的消息
//返回值:ERR_OK,发送OK
// 	     ERR_MEM,发送失败
err_t sys_mbox_trypost(sys_mbox_t *mbox, void *msg)
{
	if (msg == NULL)
		msg = (void*)&pvNullPointer; //当msg为空时 msg等于pvNullPointer指向的值
	if ((rt_mq_send(mbox->pQ, &msg,4)) != RT_EOK)
		return ERR_MEM;
	return ERR_OK;
}

//等待邮箱中的消息
//*mbox:消息邮箱
//*msg:消息
//timeout:超时时间，如果timeout为0的话,就一直等待
//返回值:当timeout不为0时如果成功的话就返回等待的时间，
//		失败的话就返回超时SYS_ARCH_TIMEOUT
u32_t sys_arch_mbox_fetch(sys_mbox_t *mbox, void **msg, u32_t timeout)
{
	rt_err_t ucErr;
	u32_t rtos_timeout, timeout_new;
	void *temp;
	if (timeout != 0)
	{
		rtos_timeout = (timeout * RT_TICK_PER_SECOND) / 1000; //转换为节拍数,因为rtos延时使用的是节拍数,而LWIP是用ms
		if (rtos_timeout < 1)
		{
			rtos_timeout = 1; //至少1个节拍
		}
	} 
	else 
	{
		rtos_timeout = (u32_t)RT_WAITING_FOREVER;
	}
	
	timeout = rt_tick_get_millisecond(); //获取系统时间
	ucErr = rt_mq_recv(mbox->pQ,&temp,4,rtos_timeout);//请求消息队列,等待时限为rtos_timeout
	if (msg != NULL)
	{
		if (temp == (void*)&pvNullPointer)
			*msg = NULL;   	//因为lwip发送空消息的时候我们使用了pvNullPointer指针,所以判断pvNullPointer指向的值
		else 
			*msg = temp;									//就可知道请求到的消息是否有效
	}
	if (ucErr == -RT_ETIMEOUT )
		timeout = SYS_ARCH_TIMEOUT; //请求超时
	else
	{
		LWIP_ASSERT("OSQPend ", ucErr == RT_EOK);
		timeout_new = rt_tick_get_millisecond();
		if (timeout_new > timeout) 
			timeout_new = timeout_new - timeout; //算出请求消息或使用的时间
		else 
			timeout_new = 0xffffffff - timeout + timeout_new;
		timeout = timeout_new + 1;
	}
	return timeout;
}

//尝试获取消息
//*mbox:消息邮箱
//*msg:消息
//返回值:等待消息所用的时间/SYS_ARCH_TIMEOUT
u32_t sys_arch_mbox_tryfetch(sys_mbox_t *mbox, void **msg)
{
	return sys_arch_mbox_fetch(mbox, msg, 10); //尝试获取一个消息
}

//检查一个消息邮箱是否有效
//*mbox:消息邮箱
//返回值:1,有效.
//      0,无效
int sys_mbox_valid(sys_mbox_t *mbox)
{
	return ((mbox->pQ != NULL) ? 1 : 0);
}

//设置一个消息邮箱为无效
//*mbox:消息邮箱
void sys_mbox_set_invalid(sys_mbox_t *mbox)
{
	mbox = NULL;
}

//创建一个信号量
//*sem:创建的信号量
//count:信号量值
//返回值:ERR_OK,创建OK
// 	     ERR_MEM,创建失败
err_t sys_sem_new(sys_sem_t * sem, u8_t count)
{
	*sem = rt_sem_create("lwip sem", (rt_uint32_t) count, RT_IPC_FLAG_PRIO);
	
	LWIP_ASSERT("OSSemCreate ", *sem != NULL );
	return ERR_OK;
}

//等待一个信号量
//*sem:要等待的信号量
//timeout:超时时间
//返回值:当timeout不为0时如果成功的话就返回等待的时间，
//		失败的话就返回超时SYS_ARCH_TIMEOUT
u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout)
{
	u8_t ucErr;
	u32_t rtos_timeout, timeout_new;
	if (timeout != 0)
	{
		rtos_timeout = rt_tick_from_millisecond(timeout);//转换为节拍数,因为rtos延时使用的是节拍数,而LWIP是用ms
		if (rtos_timeout < 1)
			rtos_timeout = 1;
	} 
	else 
		rtos_timeout = 0;
	timeout = rt_tick_get();
	ucErr = rt_sem_take(*sem,(rt_int32_t)rtos_timeout);
	if (ucErr == RT_ETIMEOUT)
		timeout = SYS_ARCH_TIMEOUT; //请求超时
	else
	{
		timeout_new = rt_tick_get();
		if (timeout_new >= timeout) 
			timeout_new = timeout_new - timeout;
		else 
			timeout_new = 0xffffffff - timeout + timeout_new;
		timeout = rt_tick_to_millisecond(timeout_new) + 1;//算出请求消息或使用的时间(ms)
	}
	return timeout;
}

//发送一个信号量
//sem:信号量指针
void sys_sem_signal(sys_sem_t *sem)
{
	rt_sem_release(*sem);
}

//释放并删除一个信号量
//sem:信号量指针
void sys_sem_free(sys_sem_t *sem)
{
	u8_t ucErr;
	ucErr = rt_sem_delete(*sem);
	if (ucErr != RT_EOK)
		LWIP_ASSERT("OSSemDel ", ucErr == RT_EOK);
	*sem = NULL;
}

//查询一个信号量的状态,无效或有效
//sem:信号量指针
//返回值:1,有效.
//      0,无效
int sys_sem_valid(sys_sem_t *sem)
{	
	return (*sem != NULL ) ? 1 : 0;
}

//设置一个信号量无效
//sem:信号量指针
void sys_sem_set_invalid(sys_sem_t *sem)
{
	*sem = NULL;
}

//arch初始化
void sys_init(void)
{
	//这里,我们在该函数,不做任何事情
}

extern char * TCPIP_THREAD_TASK_STK;//TCP IP内核任务堆栈,在lwip_comm函数定义
//创建一个新进程
//*name:进程名称
//thred:进程任务函数
//*arg:进程任务函数的参数
//stacksize:进程任务的堆栈大小
//prio:进程任务的优先级
sys_thread_t sys_thread_new(const char *name, lwip_thread_fn thread, void *arg, int stacksize, int prio)
{
	rt_err_t error;

	if (strcmp(name, TCPIP_THREAD_NAME) == 0) //创建TCP IP内核任务
	{
		error = rt_thread_init(&tcp_ip_thread, name, thread, arg, TCPIP_THREAD_TASK_STK, stacksize, prio, 10);//创建TCP IP内核任务
		/* 启动线程，开启调度 */
		if (error == RT_EOK)
		{
			rt_thread_startup(&tcp_ip_thread);
			return &tcp_ip_thread;
		}
		else
		{
			printf("%s %s create fail \r\n",__FUNCTION__,name);
		}
	}
	return RT_NULL;
}

//lwip延时函数
//ms:要延时的ms数
void sys_msleep(u32_t ms)
{
	delay_ms(ms);
}

//获取系统时间,LWIP1.4.1增加的函数
//返回值:当前系统时间(单位:毫秒)
u32_t sys_now(void)
{
	return rt_tick_get_millisecond(); 		//返回lwip_time;
}



