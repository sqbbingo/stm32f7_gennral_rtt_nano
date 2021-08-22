#ifndef __SCCB_H
#define __SCCB_H
#include "stm32f7xx.h"

//IO方向设置
#define SCCB_SDA_IN()  {GPIOB->MODER&=~(3<<(3*2));GPIOB->MODER|=0<<3*2;}    //PB3输入模式
#define SCCB_SDA_OUT() {GPIOB->MODER&=~(3<<(3*2));GPIOB->MODER|=1<<3*2;}    //PB3输出模式
//IO操作
#define SCCB_SCL(n)  (n?HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,GPIO_PIN_RESET)) //SCL
#define SCCB_SDA(n)  (n?HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_RESET)) //SDA

#define SCCB_READ_SDA    HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_3)     //输入SDA
#define SCCB_ID         0X60                                    //OV2640的ID

///////////////////////////////////////////
void SCCB_Init(void);
void SCCB_Start(void);
void SCCB_Stop(void);
void SCCB_No_Ack(void);
uint8_t SCCB_WR_Byte(uint8_t dat);
uint8_t SCCB_RD_Byte(void);
//uint8_t SCCB_WR_Reg(uint8_t reg,uint8_t data);
//uint8_t SCCB_RD_Reg(uint8_t reg);
#endif

