//
// Created by sunqi on 17-8-28.
//

#include "adc.h"

/*配置ADC的端口 以及各个模块的初始化配置*/
void adc_init()
{
    ADC_InitTypeDef ADC_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_ADC1,ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div6); //总线时钟72M ADC时钟不能超过14M所以div6

    //初始化adc的输入io口的方式
    GPIO_InitStructure.GPIO_Pin=GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AIN; //模拟输入
    GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
    GPIO_Init(GPIOA,&GPIO_InitStructure);
    //复位ADC1
    ADC_DeInit(ADC1);
    ADC_InitStructure.ADC_Mode=ADC_Mode_Independent;
    ADC_InitStructure.ADC_ContinuousConvMode=DISABLE;
    ADC_InitStructure.ADC_DataAlign=ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel=1;   //顺序进行规则转换的ADC通道的数目
    ADC_InitStructure.ADC_ExternalTrigConv=ADC_ExternalTrigConv_None; //软件触发转化 而不是别的方式 单次模式
    ADC_InitStructure.ADC_ScanConvMode=DISABLE;
    ADC_Init(ADC1,&ADC_InitStructure);
    ADC_Cmd(ADC1,ENABLE);    //ADC1进行使能

    ADC_ResetCalibration(ADC1); //开启复位校准
    while (ADC_GetResetCalibrationStatus(ADC1));  //等待校准结束
    ADC_StartCalibration(ADC1); //开启AD校准
    while (ADC_GetCalibrationStatus(ADC1));      //等待校准结束

}

u16 get_adc(u8 ch)
{
    ADC_RegularChannelConfig(ADC1,ch,1,ADC_SampleTime_239Cycles5);
    ADC_SoftwareStartInjectedConvCmd(ADC1,ENABLE);
    while (!ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC));
    return ADC_GetConversionValue(ADC1);
}


u16 get_adc_average(u8 ch,u8 times)
{
    u32 time_val=0;
    u8 t;
    for(t=0;t<times;t++)
    {
        time_val+=get_adc(ch);
        //这个地方用到了初始化
        delay_ms(5);

    }
    return time_val/times;
}