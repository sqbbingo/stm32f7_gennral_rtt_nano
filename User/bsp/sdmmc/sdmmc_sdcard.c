#include "sdmmc_sdcard.h"
#include "string.h"
#include "delay.h"

SD_HandleTypeDef        SDCARD_Handler;             //SD卡句柄
HAL_SD_CardInfoTypedef  SDCardInfo;                 //SD卡信息结构体
DMA_HandleTypeDef SDTxDMAHandler, SDRxDMAHandler;   //SD卡DMA发送和接收句柄

//SD_ReadDisk/SD_WriteDisk函数专用buf,当这两个函数的数据缓存区地址不是4字节对齐的时候,
//需要用到该数组,确保数据缓存区地址是4字节对齐的.
__align(4) u8 SDIO_DATA_BUFFER[512];

//SD卡初始化
//返回值:0 初始化正确；其他值，初始化错误
u8 SD_Init(void)
{
	u8 SD_Error;
	printf("sd init begin \r\n");
	//初始化时的时钟不能大于400KHZ
	SDCARD_Handler.Instance = SDMMC1;
	SDCARD_Handler.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;            //上升沿
	SDCARD_Handler.Init.ClockBypass = SDMMC_CLOCK_BYPASS_DISABLE;       //不使用bypass模式，直接用HCLK进行分频得到SDIO_CK
	SDCARD_Handler.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE; //空闲时不关闭时钟电源
	SDCARD_Handler.Init.BusWide = SDMMC_BUS_WIDE_1B;                    //1位数据线
	SDCARD_Handler.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE; //关闭硬件流控
	SDCARD_Handler.Init.ClockDiv = SDMMC_TRANSFER_CLK_DIV;              //SD传输时钟频率最大25MHZ

	printf("sd init int1 \r\n");
	SD_Error = HAL_SD_Init(&SDCARD_Handler, &SDCardInfo);
	if (SD_Error != SD_OK) return 1;

	printf("sd init int2 \r\n");
	SD_Error = HAL_SD_WideBusOperation_Config(&SDCARD_Handler, SDMMC_BUS_WIDE_4B); //使能宽总线模式

	printf("sd init end \r\n");
	if (SD_Error != SD_OK) return 2;
	return 0;
}

//SDMMC底层驱动，时钟使能，引脚配置，DMA配置
//此函数会被HAL_SD_Init()调用
//hsd:SD卡句柄
void HAL_SD_MspInit(SD_HandleTypeDef *hsd)
{
	GPIO_InitTypeDef GPIO_Initure;
	printf("sd msp init begin \r\n");
	__HAL_RCC_SDMMC1_CLK_ENABLE();  //使能SDMMC1时钟
	__HAL_RCC_DMA2_CLK_ENABLE();    //使能DMA2时钟
	__HAL_RCC_GPIOC_CLK_ENABLE();   //使能GPIOC时钟
	__HAL_RCC_GPIOD_CLK_ENABLE();   //使能GPIOD时钟

	//PC8,9,10,11,12
	GPIO_Initure.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12;
	GPIO_Initure.Mode = GPIO_MODE_AF_PP;    //推挽复用
	GPIO_Initure.Pull = GPIO_PULLUP;        //上拉
	GPIO_Initure.Speed = GPIO_SPEED_HIGH;   //高速
	GPIO_Initure.Alternate = GPIO_AF12_SDIO; //复用为SDIO
	HAL_GPIO_Init(GPIOC, &GPIO_Initure);    //初始化

	//PD2
	GPIO_Initure.Pin = GPIO_PIN_2;
	HAL_GPIO_Init(GPIOD, &GPIO_Initure);    //初始化

#if (SD_DMA_MODE==1)                        //使用DMA模式
	HAL_NVIC_SetPriority(SDMMC1_IRQn, 2, 0); //配置SDMMC1中断，抢占优先级2，子优先级0
	HAL_NVIC_EnableIRQ(SDMMC1_IRQn);        //使能SDMMC1中断

	//配置发送DMA
	SDRxDMAHandler.Instance = DMA2_Stream3;
	SDRxDMAHandler.Init.Channel = DMA_CHANNEL_4;
	SDRxDMAHandler.Init.Direction = DMA_PERIPH_TO_MEMORY;
	SDRxDMAHandler.Init.PeriphInc = DMA_PINC_DISABLE;
	SDRxDMAHandler.Init.MemInc = DMA_MINC_ENABLE;
	SDRxDMAHandler.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
	SDRxDMAHandler.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
	SDRxDMAHandler.Init.Mode = DMA_PFCTRL;
	SDRxDMAHandler.Init.Priority = DMA_PRIORITY_VERY_HIGH;
	SDRxDMAHandler.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
	SDRxDMAHandler.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
	SDRxDMAHandler.Init.MemBurst = DMA_MBURST_INC4;
	SDRxDMAHandler.Init.PeriphBurst = DMA_PBURST_INC4;

	__HAL_LINKDMA(hsd, hdmarx, SDRxDMAHandler); //将接收DMA和SD卡的发送DMA连接起来
	HAL_DMA_DeInit(&SDRxDMAHandler);
	HAL_DMA_Init(&SDRxDMAHandler);              //初始化接收DMA

	//配置接收DMA
	SDTxDMAHandler.Instance = DMA2_Stream6;
	SDTxDMAHandler.Init.Channel = DMA_CHANNEL_4;
	SDTxDMAHandler.Init.Direction = DMA_MEMORY_TO_PERIPH;
	SDTxDMAHandler.Init.PeriphInc = DMA_PINC_DISABLE;
	SDTxDMAHandler.Init.MemInc = DMA_MINC_ENABLE;
	SDTxDMAHandler.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
	SDTxDMAHandler.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
	SDTxDMAHandler.Init.Mode = DMA_PFCTRL;
	SDTxDMAHandler.Init.Priority = DMA_PRIORITY_VERY_HIGH;
	SDTxDMAHandler.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
	SDTxDMAHandler.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
	SDTxDMAHandler.Init.MemBurst = DMA_MBURST_INC4;
	SDTxDMAHandler.Init.PeriphBurst = DMA_PBURST_INC4;

	__HAL_LINKDMA(hsd, hdmatx, SDTxDMAHandler);//将发送DMA和SD卡的发送DMA连接起来
	HAL_DMA_DeInit(&SDTxDMAHandler);
	HAL_DMA_Init(&SDTxDMAHandler);              //初始化发送DMA


	HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, 3, 0);  //接收DMA中断优先级
	HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);
	HAL_NVIC_SetPriority(DMA2_Stream6_IRQn, 3, 0);  //发送DMA中断优先级
	HAL_NVIC_EnableIRQ(DMA2_Stream6_IRQn);
#endif
	printf("sd msp init end \r\n");
}

void show_sdcard_info(void)
{
	switch(SDCardInfo.CardType)
	{
		case STD_CAPACITY_SD_CARD_V1_1:printf("Card Type:SDSC V1.1\r\n");break;
		case STD_CAPACITY_SD_CARD_V2_0:printf("Card Type:SDSC V2.0\r\n");break;
		case HIGH_CAPACITY_SD_CARD:printf("Card Type:SDHC V2.0\r\n");break;
		case MULTIMEDIA_CARD:printf("Card Type:MMC Card\r\n");break;
	}	
  	printf("Card ManufacturerID:%d\r\n",SDCardInfo.SD_cid.ManufacturerID);	//ÖÆÔìÉÌID
 	printf("Card RCA:%d\r\n",SDCardInfo.RCA);								//¿¨Ïà¶ÔµØÖ·
	printf("Card Capacity:%d MB\r\n",(u32)(SDCardInfo.CardCapacity>>20));	//ÏÔÊ¾ÈÝÁ¿
 	printf("Card BlockSize:%d\r\n\r\n",SDCardInfo.CardBlockSize);			//ÏÔÊ¾¿é´óÐ¡
}

//得到卡信息
//cardinfo:卡信息存储区
//返回值:错误状态
u8 SD_GetCardInfo(HAL_SD_CardInfoTypedef *cardinfo)
{
	u8 sta;
	sta = HAL_SD_Get_CardInfo(&SDCARD_Handler, cardinfo);
	return sta;
}

#if (SD_DMA_MODE==1)        //DMA模式

//通过DMA读取SD卡一个扇区
//buf:读数据缓存区
//sector:扇区地址
//blocksize:扇区大小(一般都是512字节)
//cnt:扇区个数
//返回值:错误状态;0,正常;其他,错误代码;
u8 SD_ReadBlocks_DMA(uint32_t *buf, uint64_t sector, uint32_t blocksize, uint32_t cnt)
{
	u8 err = 0;
	if (Get_DCahceSta())SCB_CleanInvalidateDCache(); //如果开启了D_Cache，一定要先清缓存!!!!!!
	err = HAL_SD_ReadBlocks_DMA(&SDCARD_Handler, buf, sector, blocksize, cnt); //通过DMA读取SD卡一个扇区
	if (err == 0) //读取成功
	{
		//等待读取完成
		err = HAL_SD_CheckReadOperation(&SDCARD_Handler, (uint32_t)SD_TIMEOUT);
	}
	if (Get_DCahceSta())SCB_CleanInvalidateDCache(); //DMA完成，清缓存
	return err;
}

//写SD卡
//buf:写数据缓存区
//sector:扇区地址
//blocksize:扇区大小(一般都是512字节)
//cnt:扇区个数
//返回值:错误状态;0,正常;其他,错误代码;
u8 SD_WriteBlocks_DMA(uint32_t *buf, uint64_t sector, uint32_t blocksize, uint32_t cnt)
{
	u8 err = 0;
	if (Get_DCahceSta())SCB_CleanInvalidateDCache(); //如果开启了D_Cache，一定要先清缓存!!!!!!
	err = HAL_SD_WriteBlocks_DMA(&SDCARD_Handler, buf, sector, blocksize, cnt); //通过DMA写SD卡一个扇区
	if (err == 0) //写成功
	{
		//等待读取完成/
		err = HAL_SD_CheckWriteOperation(&SDCARD_Handler, (uint32_t)SD_TIMEOUT);
	}
	if (Get_DCahceSta())SCB_CleanInvalidateDCache(); //DMA完成，清缓存
	return err;
}

//读SD卡
//buf:读数据缓存区
//sector:扇区地址
//cnt:扇区个数
//返回值:错误状态;0,正常;其他,错误代码;
u8 SD_ReadDisk(u8* buf, u32 sector, u8 cnt)
{
	u8 sta = SD_OK;
	long long lsector = sector;
	u8 n;
	if (SDCardInfo.CardType != STD_CAPACITY_SD_CARD_V1_1)lsector <<= 9;
	if ((u32)buf % 4 != 0)
	{
		for (n = 0; n < cnt; n++)
		{
			sta = SD_ReadBlocks_DMA((uint32_t*)SDIO_DATA_BUFFER, lsector + 512 * n, 512, 1);
			memcpy(buf, SDIO_DATA_BUFFER, 512);
			buf += 512;
		}
	} else
	{
		sta = SD_ReadBlocks_DMA((uint32_t*)buf, lsector, 512, cnt);
	}
	return sta;
}

//写SD卡
//buf:写数据缓存区
//sector:扇区地址
//cnt:扇区个数
//返回值:错误状态;0,正常;其他,错误代码;
u8 SD_WriteDisk(u8 *buf, u32 sector, u8 cnt)
{
	u8 sta = SD_OK;
	long long lsector = sector;
	u8 n;
	if (SDCardInfo.CardType != STD_CAPACITY_SD_CARD_V1_1)lsector <<= 9;
	if ((u32)buf % 4 != 0)
	{
		for (n = 0; n < cnt; n++)
		{
			memcpy(SDIO_DATA_BUFFER, buf, 512);
			sta = SD_WriteBlocks_DMA((uint32_t*)SDIO_DATA_BUFFER, lsector + 512 * n, 512, 1); //单个sector的写操作
			buf += 512;
		}
	} else
	{
		sta = SD_WriteBlocks_DMA((uint32_t*)buf, lsector, 512, cnt); //多个sector的写操作
	}
	return sta;
}

//SDMMC1中断服务函数
void SDMMC1_IRQHandler(void)
{
	HAL_SD_IRQHandler(&SDCARD_Handler);
}

//DMA2数据流6中断服务函数
void DMA2_Stream6_IRQHandler(void)
{
	HAL_DMA_IRQHandler(SDCARD_Handler.hdmatx);
}

//DMA2数据流3中断服务函数
void DMA2_Stream3_IRQHandler(void)
{
	HAL_DMA_IRQHandler(SDCARD_Handler.hdmarx);
}
#else                                   //轮训模式

//读SD卡
//buf:读数据缓存区
//sector:扇区地址
//cnt:扇区个数
//返回值:错误状态;0,正常;其他,错误代码;
u8 SD_ReadDisk(u8* buf, u32 sector, u8 cnt)
{
	u8 sta = SD_OK;
	long long lsector = sector;
	u8 n;
	lsector <<= 9;
	INTX_DISABLE();//关闭总中断(POLLING模式,严禁中断打断SDIO读写操作!!!)
	if ((u32)buf % 4 != 0)
	{
		for (n = 0; n < cnt; n++)
		{
			sta = HAL_SD_ReadBlocks(&SDCARD_Handler, (uint32_t*)SDIO_DATA_BUFFER, lsector + 512 * n, 512, 1); //单个sector的读操作
			memcpy(buf, SDIO_DATA_BUFFER, 512);
			buf += 512;
		}
	} else
	{
		sta = HAL_SD_ReadBlocks(&SDCARD_Handler, (uint32_t*)buf, lsector, 512, cnt); //单个sector的读操作
	}
	INTX_ENABLE();//开启总中断
	return sta;
}

//写SD卡
//buf:写数据缓存区
//sector:扇区地址
//cnt:扇区个数
//返回值:错误状态;0,正常;其他,错误代码;
u8 SD_WriteDisk(u8 *buf, u32 sector, u8 cnt)
{
	u8 sta = SD_OK;
	long long lsector = sector;
	u8 n;
	lsector <<= 9;
	INTX_DISABLE();//关闭总中断(POLLING模式,严禁中断打断SDIO读写操作!!!)
	if ((u32)buf % 4 != 0)
	{
		for (n = 0; n < cnt; n++)
		{
			memcpy(SDIO_DATA_BUFFER, buf, 512);
			sta = HAL_SD_WriteBlocks(&SDCARD_Handler, (uint32_t*)SDIO_DATA_BUFFER, lsector + 512 * n, 512, 1); //单个sector的写操作
			buf += 512;
		}
	} else
	{
		sta = HAL_SD_WriteBlocks(&SDCARD_Handler, (uint32_t*)buf, lsector, 512, cnt); //多个sector的写操作
	}
	INTX_ENABLE();//开启总中断
	return sta;
}
#endif




