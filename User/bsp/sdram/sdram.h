#ifndef _SDRAM_H
#define _SDRAM_H

#include "stm32f7xx.h"

extern SDRAM_HandleTypeDef SDRAM_Handler;//SDRAM句柄
#define Bank5_SDRAM_ADDR    ((rt_uint32_t)(0XC0000000)) //SDRAM开始地址

//SDRAM配置参数
#define SDRAM_MODEREG_BURST_LENGTH_1             ((rt_uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2             ((rt_uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4             ((rt_uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8             ((rt_uint16_t)0x0004)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((rt_uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ((rt_uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_2              ((rt_uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3              ((rt_uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    ((rt_uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((rt_uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((rt_uint16_t)0x0200)

void SDRAM_Init(void);
void SDRAM_MPU_Config(void);
rt_uint8_t SDRAM_Send_Cmd(rt_uint8_t bankx, rt_uint8_t cmd, rt_uint8_t refresh, rt_uint16_t regval);
void FMC_SDRAM_WriteBuffer(rt_uint8_t *pBuffer, rt_uint32_t WriteAddr, rt_uint32_t n);
void FMC_SDRAM_ReadBuffer(rt_uint8_t *pBuffer, rt_uint32_t ReadAddr, rt_uint32_t n);
void SDRAM_Initialization_Sequence(SDRAM_HandleTypeDef *hsdram);
#endif
