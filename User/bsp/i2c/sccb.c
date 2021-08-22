#include <rtthread.h>
#include "sccb.h"
#include "delay.h"
#include "pcf8574.h"

void SCCB_Delay(void)
{
    delay_us(5);
}

//初始化SCCB接口
void SCCB_Init(void)
{
    GPIO_InitTypeDef GPIO_Initure;
    __HAL_RCC_GPIOB_CLK_ENABLE();           //使能GPIOB时钟

    //PB3.4初始化设置
    GPIO_Initure.Pin = GPIO_PIN_3 | GPIO_PIN_4;
    GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP; //推挽输出
    GPIO_Initure.Pull = GPIO_PULLUP;        //上拉
    GPIO_Initure.Speed = GPIO_SPEED_FAST;   //快速
    HAL_GPIO_Init(GPIOB, &GPIO_Initure);
}

//SCCB起始信号
//当时钟为高的时候,数据线的高到低,为SCCB起始信号
//在激活状态下,SDA和SCL均为低电平
void SCCB_Start(void)
{
    SCCB_SDA(1);     //数据线高电平
    SCCB_SCL(1);        //在时钟线高的时候数据线由高至低
    SCCB_Delay();
    SCCB_SDA(0);
    SCCB_Delay();
    SCCB_SCL(0);        //数据线恢复低电平，单操作函数必要
}

//SCCB停止信号
//当时钟为高的时候,数据线的低到高,为SCCB停止信号
//空闲状况下,SDA,SCL均为高电平
void SCCB_Stop(void)
{
    SCCB_SDA(0);
    SCCB_Delay();
    SCCB_SCL(1);
    SCCB_Delay();
    SCCB_SDA(1);
    SCCB_Delay();
}
//产生NA信号
void SCCB_No_Ack(void)
{
    SCCB_Delay();
    SCCB_SDA(1);
    SCCB_SCL(1);
    SCCB_Delay();
    SCCB_SCL(0);
    SCCB_Delay();
    SCCB_SDA(0);
    SCCB_Delay();
}
//SCCB,写入一个字节
//返回值:0,成功;1,失败.
uint8_t SCCB_WR_Byte(uint8_t dat)
{
    uint8_t j, res;
    for (j = 0; j < 8; j++) //循环8次发送数据
    {
        if (dat & 0x80)SCCB_SDA(1);
        else SCCB_SDA(0);
        dat <<= 1;
        SCCB_Delay();
        SCCB_SCL(1);
        SCCB_Delay();
        SCCB_SCL(0);
    }
    SCCB_SDA_IN();      //设置SDA为输入
    SCCB_Delay();
    SCCB_SCL(1);            //接收第九位,以判断是否发送成功
    SCCB_Delay();
    if (SCCB_READ_SDA)res = 1; //SDA=1发送失败，返回1
    else res = 0;       //SDA=0发送成功，返回0
    SCCB_SCL(0);
    SCCB_SDA_OUT();     //设置SDA为输出
    return res;
}
//SCCB 读取一个字节
//在SCL的上升沿,数据锁存
//返回值:读到的数据
uint8_t SCCB_RD_Byte(void)
{
    uint8_t temp = 0, j;
    SCCB_SDA_IN();      //设置SDA为输入
    for (j = 8; j > 0; j--) //循环8次接收数据
    {
        SCCB_Delay();
        SCCB_SCL(1);
        temp = temp << 1;
        if (SCCB_READ_SDA)temp++;
        SCCB_Delay();
        SCCB_SCL(0);
    }
    SCCB_SDA_OUT();     //设置SDA为输出
    return temp;
}
////写寄存器
////返回值:0,成功;1,失败.
//uint8_t SCCB_WR_Reg(uint8_t reg,uint8_t data)
//{
//  uint8_t res=0;
//  SCCB_Start();                   //启动SCCB传输
//  if(SCCB_WR_Byte(SCCB_ID))res=1; //写器件ID
//  delay_us(100);
//      if(SCCB_WR_Byte(reg))res=1;     //写寄存器地址
//  delay_us(100);
//      if(SCCB_WR_Byte(data))res=1;    //写数据
//      SCCB_Stop();
//      return  res;
//}
////读寄存器
////返回值:读到的寄存器值
//uint8_t SCCB_RD_Reg(uint8_t reg)
//{
//  uint8_t val=0;
//  SCCB_Start();               //启动SCCB传输
//  SCCB_WR_Byte(SCCB_ID);      //写器件ID
//  delay_us(100);
//      SCCB_WR_Byte(reg);          //写寄存器地址
//  delay_us(100);
//  SCCB_Stop();
//  delay_us(100);
//  //设置寄存器地址后，才是读
//  SCCB_Start();
//  SCCB_WR_Byte(SCCB_ID|0X01); //发送读命令
//  delay_us(100);
//      val=SCCB_RD_Byte();         //读取数据
//      SCCB_No_Ack();
//      SCCB_Stop();
//      return val;
//}

//检测指定地址是否存在设备
uint8_t SCCB_AddrCheck(uint8_t addr)
{
    uint8_t state;
    SCCB_Start();

    state = SCCB_WR_Byte(addr);
    SCCB_Stop();

    return state;
}
//扫描i2c从机设备
void sccbscan(void)
{
    uint8_t u8I2cAddr;

    PCF8574_WriteBit(DCMI_PWDN_IO, 0); //开启DCMI电源
    rt_kprintf("scanning sccb.... \r\n");
    for (u8I2cAddr = 1; u8I2cAddr < 127; u8I2cAddr++)
    {
        if (SCCB_AddrCheck(u8I2cAddr << 1) == 0)
        {
            rt_kprintf("sccb addr: 0x%x alive \r\n", u8I2cAddr);
            //          vDebug("alive \r\n");
        }
        else
        {
            //          vDebug_Error("nofind \r\n");
        }
        SCCB_Stop();
    }
}

MSH_CMD_EXPORT(sccbscan, scan sccb device);
