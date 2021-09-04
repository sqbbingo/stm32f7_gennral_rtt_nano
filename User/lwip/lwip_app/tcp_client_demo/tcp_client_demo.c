#include "rtthread.h"
#include "tcp_client_demo.h"
#include "lwip/opt.h"
#include "lwip_comm.h"
#include "delay.h"
#include "lwip/lwip_sys.h"
#include "lwip/api.h"
#include "string.h"

struct netconn *tcp_clientconn = NULL;					//TCP CLIENT网络连接结构体
u8 tcp_client_recvbuf[TCP_CLIENT_RX_BUFSIZE];	//TCP客户端接收数据缓冲区
u8 *tcp_client_sendbuf = "Apollo STM32F4/F7 NETCONN TCP Client send data\r\n";	//TCP客户端发送数据缓冲区
u8 tcp_client_flag;		//TCP客户端数据发送标志位
static rt_thread_t tcp_client_demo_thread = RT_NULL;

//TCP客户端任务
#define TCPCLIENT_PRIO		6
//任务堆栈大小
#define TCPCLIENT_STK_SIZE	300

//tcp客户端任务函数
static void tcp_client_thread(void *arg)
{
	u32 data_len = 0;
	struct pbuf *q;
	err_t err, recv_err;
	static ip_addr_t server_ipaddr, loca_ipaddr;
	static u16_t 		 server_port, loca_port;
	static rt_base_t level;

	LWIP_UNUSED_ARG(arg);
	server_port = REMOTE_PORT;
	IP4_ADDR(&server_ipaddr, lwipdev.remoteip[0], lwipdev.remoteip[1], lwipdev.remoteip[2], lwipdev.remoteip[3]);

	while (1)
	{
		tcp_clientconn = netconn_new(NETCONN_TCP); //创建一个TCP链接
		err = netconn_connect(tcp_clientconn, &server_ipaddr, server_port); //连接服务器
		if (err != ERR_OK)  netconn_delete(tcp_clientconn); //返回值不等于ERR_OK,删除tcp_clientconn连接
		else if (err == ERR_OK)    //处理新连接的数据
		{
			struct netbuf *recvbuf;
			tcp_clientconn->recv_timeout = 10;
			netconn_getaddr(tcp_clientconn, &loca_ipaddr, &loca_port, 1); //获取本地IP主机IP地址和端口号
			printf("连接上服务器%d.%d.%d.%d,本机端口号为:%d\r\n", lwipdev.remoteip[0], lwipdev.remoteip[1], lwipdev.remoteip[2], lwipdev.remoteip[3], loca_port);
			while (1)
			{
				if ((tcp_client_flag & LWIP_SEND_DATA) == LWIP_SEND_DATA) //有数据要发送
				{
					err = netconn_write(tcp_clientconn , tcp_client_sendbuf, strlen((char*)tcp_client_sendbuf), NETCONN_COPY); //发送tcp_server_sentbuf中的数据
					if (err != ERR_OK)
					{
						printf("发送失败\r\n");
					}
					tcp_client_flag &= ~LWIP_SEND_DATA;
				}

				if ((recv_err = netconn_recv(tcp_clientconn, &recvbuf)) == ERR_OK) //接收到数据
				{
					level = rt_hw_interrupt_disable(); //关中断
					memset(tcp_client_recvbuf, 0, TCP_CLIENT_RX_BUFSIZE); //数据接收缓冲区清零
					for (q = recvbuf->p; q != NULL; q = q->next) //遍历完整个pbuf链表
					{
						//判断要拷贝到TCP_CLIENT_RX_BUFSIZE中的数据是否大于TCP_CLIENT_RX_BUFSIZE的剩余空间，如果大于
						//的话就只拷贝TCP_CLIENT_RX_BUFSIZE中剩余长度的数据，否则的话就拷贝所有的数据
						if (q->len > (TCP_CLIENT_RX_BUFSIZE - data_len)) memcpy(tcp_client_recvbuf + data_len, q->payload, (TCP_CLIENT_RX_BUFSIZE - data_len)); //拷贝数据
						else memcpy(tcp_client_recvbuf + data_len, q->payload, q->len);
						data_len += q->len;
						if (data_len > TCP_CLIENT_RX_BUFSIZE) break; //超出TCP客户端接收数组,跳出
					}
					rt_hw_interrupt_enable(level);  //开中断
					data_len = 0; //复制完成后data_len要清零。
					printf("%s\r\n", tcp_client_recvbuf);
					netbuf_delete(recvbuf);
				} else if (recv_err == ERR_CLSD) //关闭连接
				{
					netconn_close(tcp_clientconn);
					netconn_delete(tcp_clientconn);
					printf("服务器%d.%d.%d.%d断开连接\r\n", lwipdev.remoteip[0], lwipdev.remoteip[1], lwipdev.remoteip[2], lwipdev.remoteip[3]);
					break;
				}
			}
		}
	}
}

//创建TCP客户端线程
//返回值:0 TCP客户端创建成功
//		其他 TCP客户端创建失败
int tcp_client_demo(void)
{
	tcp_client_demo_thread = rt_thread_create("tcp_client_demo", tcp_client_thread, RT_NULL, TCPCLIENT_STK_SIZE, TCPCLIENT_PRIO, 10);

	if(tcp_client_demo_thread != RT_NULL)
	{
		rt_thread_startup(tcp_client_demo_thread);
	}
	else
	{
		rt_kprintf("tcp client demo thread create fail \r\n");
		return -1;
	}
	return 0;
}

MSH_CMD_EXPORT(tcp_client_demo, tcp client test);

int tcp_client_send(void)
{
	tcp_client_flag = LWIP_SEND_DATA;
	
	return 0;
}

MSH_CMD_EXPORT(tcp_client_send, tcp client send);


