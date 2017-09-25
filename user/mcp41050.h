//
// Created by sunqi on 17-9-23.
//

#ifndef MCU_MCP41050_H
#define MCU_MCP41050_H
#include "sys.h"
/*
 *
 * 本程序是用来使用MCP41050的参数设置
 * 引脚定义：
 * CS :   PTB12
 * SCLK : PTB13
 * SI：   PTB15
 *
 * 对于这款芯片来说 CPOL=0 CPHA=0
 *
 */
#define MCP41050_CS_Port 12            //片选端口引脚
#define CS_Pin  ((u16)0x01<<MCP41050_CS_Port)  //对应的CPIO_Pin
#define CS PBout(MCP41050_CS_Port)     //片选端口输出设置寄存器
#define CS_ON (CS=0)     //开启片选端口
#define CS_OFF (CS=1)    //关闭片选



/*
 * 设置MCP41050的命令代码
 */

#define WriteData_CMD (0x13)
#define NOP_CMD (0x00)

void MCP_SPI_Init();
void mcp41050_spi_setSpeed(u8 SPI_BaudRatePrescaler);
void mcp41050_writeWord(u16 TxData);
u16 dataCombine(u8 cmd,u8 data);
void mcp41050_set_Res(u8 data);   //这个向MCP41050写入0-255的相对的分度值的函数 写入的时候直接调用就好

#endif //MCU_MCP41050
