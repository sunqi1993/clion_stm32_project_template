//
// Created by sunqi on 17-8-28.
//

#ifndef MCU_ADC_H
#define MCU_ADC_H

#include "stm32f10x_adc.h"
#include "stm32f10x_gpio.h"

extern void delay_ms(u16 nms);


void adc_init();

u16 get_adc(u8 ch);

u16 get_adc_average(u8 ch,u8 times);

#endif //MCU_ADC_H
