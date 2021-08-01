#include "stm32f7xx.h"
#include "adc.h"
#include "delay.h"

ADC_HandleTypeDef ADC1_Handler;//ADC句柄

//初始化ADC
//ch: ADC_channels
//通道值 0~16取值范围为：ADC_CHANNEL_0~ADC_CHANNEL_16
void adc_init(void)
{
	ADC1_Handler.Instance = ADC1;
	ADC1_Handler.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4; //4分频，ADCCLK=PCLK2/4=108/4=27MHZ
	ADC1_Handler.Init.Resolution = ADC_RESOLUTION_12B;          //12位模式
	ADC1_Handler.Init.DataAlign = ADC_DATAALIGN_RIGHT;          //右对齐
	ADC1_Handler.Init.ScanConvMode = DISABLE;                   //非扫描模式
	ADC1_Handler.Init.EOCSelection = DISABLE;                   //关闭EOC中断
	ADC1_Handler.Init.ContinuousConvMode = DISABLE;             //关闭连续转换
	ADC1_Handler.Init.NbrOfConversion = 1;                      //1个转换在规则序列中 也就是只转换规则序列1
	ADC1_Handler.Init.DiscontinuousConvMode = DISABLE;          //禁止不连续采样模式
	ADC1_Handler.Init.NbrOfDiscConversion = 0;                  //不连续采样通道数为0
	ADC1_Handler.Init.ExternalTrigConv = ADC_SOFTWARE_START;    //软件触发
	ADC1_Handler.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE; //使用软件触发
	ADC1_Handler.Init.DMAContinuousRequests = DISABLE;          //关闭DMA请求
	HAL_ADC_Init(&ADC1_Handler);                                //初始化
}

//ADC底层驱动，引脚配置，时钟使能
//此函数会被HAL_ADC_Init()调用
//hadc:ADC句柄
void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
{
	GPIO_InitTypeDef GPIO_Initure;
	__HAL_RCC_ADC1_CLK_ENABLE();            //使能ADC1时钟
	__HAL_RCC_GPIOA_CLK_ENABLE();           //开启GPIOA时钟

	GPIO_Initure.Pin = GPIO_PIN_5;          //PA5
	GPIO_Initure.Mode = GPIO_MODE_ANALOG;   //模拟
	GPIO_Initure.Pull = GPIO_NOPULL;        //不带上下拉
	HAL_GPIO_Init(GPIOA, &GPIO_Initure);
}

//获得ADC值
//ch: 通道值 0~16，取值范围为：ADC_CHANNEL_0~ADC_CHANNEL_16
//返回值:转换结果
uint16_t Get_Adc(uint32_t ch)
{
	ADC_ChannelConfTypeDef ADC1_ChanConf;

	ADC1_ChanConf.Channel = ch;                                 //通道
	ADC1_ChanConf.Rank = 1;                                     //1个序列
	ADC1_ChanConf.SamplingTime = ADC_SAMPLETIME_480CYCLES;      //采样时间
	ADC1_ChanConf.Offset = 0;
	HAL_ADC_ConfigChannel(&ADC1_Handler, &ADC1_ChanConf);       //通道配置

	HAL_ADC_Start(&ADC1_Handler);                               //开启ADC

	HAL_ADC_PollForConversion(&ADC1_Handler, 10);               //轮询转换

	return (uint16_t)HAL_ADC_GetValue(&ADC1_Handler);                //返回最近一次ADC1规则组的转换结果
}

//获取指定通道的转换值，取times次,然后平均
//times:获取次数
//返回值:通道ch的times次转换结果平均值
uint16_t Get_Adc_Average(uint32_t ch, uint8_t times)
{
	uint32_t temp_val = 0;
	uint8_t t;
	for (t = 0; t < times; t++)
	{
		temp_val += Get_Adc(ch);
		delay_ms(5);
	}
	return temp_val / times;
}

//得到温度值
//返回值:温度值(扩大了100倍,单位:℃.)
short Get_Temprate(void)
{
	uint32_t adcx;
	short result;
	double temperate;
	adcx = Get_Adc_Average(ADC_CHANNEL_TEMPSENSOR, 10); //读取内部温度传感器通道,10次取平均
	temperate = (float)adcx * (3.3 / 4096); //电压值
	temperate = (temperate - 0.76) / 0.0025 + 25; //转换为温度值
	result = temperate *= 100;              //扩大100倍.
	return result;
}
