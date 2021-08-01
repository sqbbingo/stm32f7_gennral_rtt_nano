#ifndef __RTC_H
#define __RTC_H
#include "stm32f7xx.h"

uint8_t RTC_Init(void);              //RTC初始化
void RTC_Get_Time(RTC_TimeTypeDef *sTime, uint32_t Format);
HAL_StatusTypeDef RTC_Set_Time(uint8_t hour, uint8_t min, uint8_t sec, uint8_t ampm);   //RTC时间设置
void RTC_Get_Date(RTC_DateTypeDef *sDate, uint32_t Format);
HAL_StatusTypeDef RTC_Set_Date(uint8_t year, uint8_t month, uint8_t date, uint8_t week); //RTC日期设置
void RTC_Set_AlarmA(uint8_t week, uint8_t hour, uint8_t min, uint8_t sec); //设置闹钟时间(按星期闹铃,24小时制)
void RTC_Set_WakeUp(uint32_t wksel, uint16_t cnt);            //周期性唤醒定时器设置
#endif
