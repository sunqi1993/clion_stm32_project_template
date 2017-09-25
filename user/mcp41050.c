//
// Created by sunqi on 17-9-23.
//

#include <stm32f10x_conf.h>
#include "mcp41050.h"
#include "delay.h"

void MCP_SPI_Init()
{
    once_delayinit();

    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef  SPI_InitStructure;

    RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB, ENABLE );//PORTB时钟使能
    RCC_APB1PeriphClockCmd(	RCC_APB1Periph_SPI2,  ENABLE );//SPI2时钟使能

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //PB13/14/15复用推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIOB

    GPIO_InitStructure.GPIO_Pin=CS_Pin;
    GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB,&GPIO_InitStructure);

    GPIO_SetBits(GPIOB,CS_Pin);  // CS上拉
    GPIO_ResetBits(GPIOB,GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);   //clk si 下拉

    SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;  //设置SPI单向或者双向的数据模式:SPI设置为只能发送
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		//设置SPI工作模式:设置为主SPI
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_16b;		//设置SPI的数据大小:SPI发送接收16位帧结构
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;		//串行同步时钟的空闲状态为低电平
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;	//串行同步时钟的第1个跳变沿（上升或下降）数据被采样  MCP从第一个跳边岩就开始计
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		//NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;		//定义波特率预分频的值:波特率预分频值为256
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
    SPI_InitStructure.SPI_CRCPolynomial = 7;	//CRC值计算的多项式
    SPI_Init(SPI2, &SPI_InitStructure);  //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器

    mcp41050_spi_setSpeed(SPI_BaudRatePrescaler_256);  //设置SPI2的波特率为36M的256分频

    SPI_Cmd(SPI2, ENABLE); //使能SPI外设

}

//SPI 速度设置函数
//SpeedSet:
//SPI_BaudRatePrescaler_2   2分频
//SPI_BaudRatePrescaler_8   8分频
//SPI_BaudRatePrescaler_16  16分频
//SPI_BaudRatePrescaler_256 256分频

void mcp41050_spi_setSpeed(u8 SPI_BaudRatePrescaler)
{
    assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));
    SPI2->CR1&=0XFFC7;   //清除寄存器上的波特率选择的三个位
    SPI2->CR1|=SPI_BaudRatePrescaler;	//设置SPI2速度

}


void mcp41050_writeWord(u16 TxData)
{
    u16 retry=0;
    //因为cs拉低期间 MCP41050会计算输入SCLK上升沿的个数 必须为16的倍数 否则数据会被丢弃
    CS_ON;  //开启片选写入数据
    SPI_I2S_SendData(SPI2, TxData); //通过外设SPIx发送一个数据
    //检查是否发送完毕
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET); //检查指定的SPI标志位设置与否:发送缓存空标志位

    delay_us(120);  //使用示波器观察到的间隙 即使数据从16位的缓存寄存器发出去了 数据其实也还没有到MSI数据线上 这样造成CS在数据发送前就关闭了
    CS_OFF; //关闭片选
}

u16 dataCombine(u8 cmd,u8 data)
{
    u16 temp;
    if(cmd==WriteData_CMD)
    {
        temp=((u16)cmd<<8)+data;
    }
    else if(cmd==NOP_CMD)
    {
        temp=cmd<<8;

    }
    return temp;
}

void mcp41050_set_Res(u8 data)
{
    u16 TxData=dataCombine(WriteData_CMD,data);
    mcp41050_writeWord(TxData);
}