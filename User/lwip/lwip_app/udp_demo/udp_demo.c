#include "rtthread.h"
#include "udp_demo.h"
#include "lwip_comm.h"
#include "lwip/api.h"
#include "lwip/lwip_sys.h"
#include "string.h"


//TCP客户端任务
#define UDP_PRIO		6
//任务堆栈大小
#define UDP_STK_SIZE	1000
static rt_thread_t udp_demo_thread = RT_NULL;


u8 udp_demo_recvbuf[UDP_DEMO_RX_BUFSIZE];	//UDP接收数据缓冲区
//UDP发送数据内容
const u8 *udp_demo_sendbuf = "Apollo STM32F4/F7 NETCONN UDP demo send data\r\n";
u8 udp_flag;							//UDP数据发送标志位

//udp任务函数
static void udp_thread(void *arg)
{
	err_t err;
	static struct netconn *udpconn;
	static struct netbuf  *recvbuf;
	static struct netbuf  *sentbuf;
	struct ip_addr destipaddr;
	u32 data_len = 0;
	struct pbuf *q;
	rt_base_t level;

	LWIP_UNUSED_ARG(arg);
	udpconn = netconn_new(NETCONN_UDP);  //创建一个UDP链接
	udpconn->recv_timeout = 10;

	if (udpconn != NULL) //创建UDP连接成功
	{
		err = netconn_bind(udpconn, IP_ADDR_ANY, UDP_DEMO_PORT);
		if(err != ERR_OK)
		{
			rt_kprintf("bin fail err = %d \r\n",err);
		}
		IP4_ADDR(&destipaddr, lwipdev.remoteip[0], lwipdev.remoteip[1], lwipdev.remoteip[2], lwipdev.remoteip[3]); //构造目的IP地址
		rt_kprintf("connect to %d.%d.%d.%d:%d \r\n",lwipdev.remoteip[0], lwipdev.remoteip[1], lwipdev.remoteip[2], lwipdev.remoteip[3],UDP_DEMO_PORT);
		err = netconn_connect(udpconn, &destipaddr, UDP_DEMO_PORT); 	//连接到远端主机
		rt_kprintf("netconn_connect state = %d \r\n",err);
		if (err == ERR_OK) //绑定完成
		{
			while (1)
			{
				if ((udp_flag & LWIP_SEND_DATA) == LWIP_SEND_DATA) //有数据要发送
				{
					sentbuf = netbuf_new();
					netbuf_alloc(sentbuf, strlen((char *)udp_demo_sendbuf));
					memcpy(sentbuf->p->payload, (void*)udp_demo_sendbuf, strlen((char*)udp_demo_sendbuf));
					err = netconn_send(udpconn, sentbuf);  	//将netbuf中的数据发送出去
					if (err != ERR_OK)
					{
						printf("发送失败\r\n");
						netbuf_delete(sentbuf);      //删除buf
					}
					udp_flag &= ~LWIP_SEND_DATA;	//清除数据发送标志
					netbuf_delete(sentbuf);      	//删除buf
				}

				netconn_recv(udpconn, &recvbuf); //接收数据
				if (recvbuf != NULL)         //接收到数据
				{
					level = rt_hw_interrupt_disable(); //关中断
					memset(udp_demo_recvbuf, 0, UDP_DEMO_RX_BUFSIZE); //数据接收缓冲区清零
					for (q = recvbuf->p; q != NULL; q = q->next) //遍历完整个pbuf链表
					{
						//判断要拷贝到UDP_DEMO_RX_BUFSIZE中的数据是否大于UDP_DEMO_RX_BUFSIZE的剩余空间，如果大于
						//的话就只拷贝UDP_DEMO_RX_BUFSIZE中剩余长度的数据，否则的话就拷贝所有的数据
						if (q->len > (UDP_DEMO_RX_BUFSIZE - data_len)) memcpy(udp_demo_recvbuf + data_len, q->payload, (UDP_DEMO_RX_BUFSIZE - data_len)); //拷贝数据
						else memcpy(udp_demo_recvbuf + data_len, q->payload, q->len);
						data_len += q->len;
						if (data_len > UDP_DEMO_RX_BUFSIZE) break; //超出TCP客户端接收数组,跳出
					}
					rt_hw_interrupt_enable(level);  //开中断
					data_len = 0; //复制完成后data_len要清零。
					printf("%s\r\n", udp_demo_recvbuf); //打印接收到的数据
					netbuf_delete(recvbuf);      //删除buf
				} else rt_thread_mdelay(5); //延时5ms
			}
		} else printf("UDP绑定失败\r\n");
	} else printf("UDP连接创建失败\r\n");
}


//创建UDP线程
//返回值:0 UDP创建成功
//		其他 UDP创建失败
int udp_demo(void)
{
	udp_demo_thread = rt_thread_create("udp_demo", udp_thread, RT_NULL, UDP_STK_SIZE, UDP_PRIO, 10);

	if(udp_demo_thread != RT_NULL)
	{
		rt_thread_startup(udp_demo_thread);
	}
	else
	{
		rt_kprintf("udp demo thread create fail \r\n");
		return -1;
	}
	return 0;
}

MSH_CMD_EXPORT(udp_demo, udp test);


