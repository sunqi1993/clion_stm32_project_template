

#define en_uartRx_irq 0

#include <pid.h>
#include "sys.h"
#include "delay.h"
#include "led.h"
#include "beep.h"
#include "key.h"
#include "usart.h"
#include "wdg.h"
#include "lcd.h"
#include "adc.h"
#include "pid.h"

#include "mcp41050.h"
/************************************************
ALIENTEK战舰STM32开发板实验2
蜂鸣器实验
技术支持：www.openedv.com
淘宝店铺：http://eboard.taobao.com
关注微信公众平台微信号："正点原子"，免费获取STM32资料。
广州市星翼电子科技有限公司
作者：正点原子 @ALIENTEK
************************************************/
static int led0 = 0;
static int led1 = 0;


//进行蜂鸣器的初始化
void uart_init1(u32 bound)
{
    //PA9 PA10对应UART的 RX TX

    //定义对应的数据结构

    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    //首先使能对应的模块的串口时钟

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);

    //对对应的串口进行复位操作
    USART_DeInit(USART1);

    //GPIO端口模式设置 对应于stm32中文手册的 相关设置

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; //TX推挽输出  PA9
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;    //RX浮空输入
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

#if en_uartRx_irq
    //中断设置
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
#endif // en_uartRx_irq

    //USART结构体的初始化
    USART_InitStructure.USART_BaudRate = bound;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);

    //使能串口
    USART_Cmd(USART1, ENABLE);
}

void test_uart()
{
    u8 t;
    u32 length;
    u32 times=0;
    delay_init();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //设置中断分组的优先级为2
    LED_Init();
    KEY_Init();
    uart_init(115200);

    while (1)
    {
        if (USART_RX_STA & 0x8000)
        {
            length = USART_RX_STA & 0x3ff;
            printf("\r\n您发送的消息为：\r\n");
            for (t = 0; t < length; t++)
            {
                //在uart init里面使能了中断函数将收到的数据存储在BUFF里面 接收完成标志置位
                USART_SendData(USART1, USART_RX_BUF[t]);
                while (USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET);//直到单个数据发完，才跳出阻塞
            }
            printf("\r\n\r\n");
            USART_RX_STA = 0;
        }
        else
        {
            times++;
            if (times % 5000 == 0)
            {
                printf("\r\n请输入回车\r\n");
                LED0 = !LED0;
                delay_ms(100);
            }
        }


    }
}

//测试按键以及蜂鸣器
void test_beep_key()
{
    int  key;
    delay_init();     //初始化delay()系统函数的变量 使用delay函数一定要对变量进行初始化
    LED_Init();
    KEY_Init();
    BEEP_Init();
    LED0 = 0;
    int i = 1;
    i = !i;

    while (1)
    {
        key = KEY_Scan(0);
        led0 = LED0;
        led1 = LED1;
        if (key)
        {

            switch (key)
            {
                case KEY0_PRES:
                    LED0 = !LED0; break;
                case KEY1_PRES:
                    LED1 = !LED1; break;
                case KEY2_PRES:
                    LED1 = !LED1;
                    LED0 = !LED0; break;
                case WKUP_PRES:
                    BEEP = !BEEP; break;
            }
        }
        else
        {
            delay_ms(10);

        }
    }
}

//外部中断2的初始化函数
void irq_line2_init()
{
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    delay_init();
    KEY_Init();  //按键的初始化 E 234都是上拉输入

    /*为了读写这三个寄存器 我们才需要开启AFIO时钟
     *1.AFIO_EVCR  AFIO事件控制寄存器
     *2.AFIO_MAPR  AFIO重定向寄存器
     *3.AFIO_EXTICRX 外部中断寄存器
     **/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);   //开启AFIO时钟 因为我们要修改外部中断寄存器

    //GPIO中断线 以及中断初始化 触发方式
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource2);
    EXTI_InitStructure.EXTI_Line = EXTI_Line2;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_Init(&EXTI_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void EXTI2_IRQHandler()
{
    delay_ms(10);
    if (KEY2 == 0)
    {
        LED0 = !LED0;
        EXTI_ClearITPendingBit(EXTI_Line2);
    }
}

/*使用key2 来测试外部中断的可用性*/
void test_iqr()
{
    LED_Init();
    KEY_Init(); //按键对应端口初始化
    irq_line2_init();  //初始化对应的中断函数
    uart_init(115200); //串口初始化
    while(1)
    {
        printf("ok\r\n");
        delay_ms(1000);
    }
}

/*测试看门狗的可用性*/
void test_watchDog()
{
    /*如果没有喂狗 那么灯应该是0-0.5s 灯不亮 0.5s-1.5s灯亮 一直重复这个过程 如果不断喂狗那么灯在0.5s后应该是一直亮的*/
    delay_init();
    KEY_Init();
    LED_Init();
    delay_ms(500);     //延时 营造一种led0先灭后亮的闪烁感
    IWDG_Init(4,625);  //看门狗定时时间大致为1s
    LED0 = 0;
    while (1)
    {
        if (KEY_Scan(0)==WKUP_PRES)
        {
            IWDG_Feed();
        }
        delay_ms(10);

    }

}

/*定时器测试函数*/
void timer3_init(u32 arr, u32 psc)
{

    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    //定时器3的初始化
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period = arr;
    TIM_TimeBaseStructure.TIM_Prescaler = psc;

    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);   //允许更新中断

    //NVIC中断优先级设置
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    TIM_Cmd(TIM3, ENABLE);

}
//定时器3的中断函数
//void TIM3_IRQHandler()
//{
//    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
//    {
//        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
//        LED1 = !LED1;
//
//    }
//}

void test_timer3()
{
    u8  ii=0;
    delay_init();
    LED_Init();
    timer3_init(4999, 7199);
    while (1)
    {
        LED0 = !LED0;
        delay_ms(500);
        for(int i=0;i<100;i++)
        {
            ii+=2;
            if(ii==20) ii=0;
        }
    }
}

/*测试采样程序 使用模式是单次采样 启动方式：软件启动，（非触发模式）*/
void test_adc()
{
    //adc_get_average函数中用到了一个delay_ms 所以需要初始化
    delay_init();
}

void test_lcd(void)
{
    u8 x=0;
    u8 lcd_id[12];			//存放LCD ID字符串
    delay_init();	    	 //延时函数初始化
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	 //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
    uart_init(115200);	 	//串口初始化为115200
    LED_Init();			     //LED端口初始化
    LCD_Init();
    POINT_COLOR=RED;
    sprintf((char*)lcd_id,"LCD ID:%04X",lcddev.id);//将LCD ID打印到lcd_id数组。
    while(1)
    {
        switch(x)
        {
            case 0:LCD_Clear(WHITE);break;
            case 1:LCD_Clear(BLACK);break;
            case 2:LCD_Clear(BLUE);break;
            case 3:LCD_Clear(RED);break;
            case 4:LCD_Clear(MAGENTA);break;
            case 5:LCD_Clear(GREEN);break;
            case 6:LCD_Clear(CYAN);break;

            case 7:LCD_Clear(YELLOW);break;
            case 8:LCD_Clear(BRRED);break;
            case 9:LCD_Clear(GRAY);break;
            case 10:LCD_Clear(LGRAY);break;
            case 11:LCD_Clear(BROWN);break;
        }
        POINT_COLOR=RED;
        LCD_ShowString(30,40,410,24,24,"WarShip STM32 ^_^ hello world heheheheh");
        LCD_ShowString(30,70,200,16,16,"TFTLCD TEST");
        LCD_ShowString(30,90,200,16,16,"ATOM@ALIENTEK");
        LCD_ShowString(30,110,200,16,16,lcd_id);		//显示LCD ID
        LCD_ShowString(30,130,200,12,12,"2014/5/4");
        x++;
        if(x==12)x=0;
        LED0=!LED0;
        delay_ms(1000);

    }
}

void show_adc_on_lcd()
{
    u8 x=0;
    u16 adc_result;
    u8 adc_str[16];
    u8 lcd_id[12];			//存放LCD ID字符串
    delay_init();	    	 //延时函数初始化

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	 //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
    uart_init(115200);	 	//串口初始化为115200
    LED_Init();			     //LED端口初始化
    LCD_Init();
    Adc_Init();

    POINT_COLOR=RED;
    sprintf((char*)lcd_id,"LCD ID:%04X",lcddev.id);//将LCD ID打印到lcd_id数组。
    LCD_Clear(LGRAY);
    while(1)
    {
        LCD_ShowString(30,40,410,24,24,"WarShip STM32 ^_^ hello world heheheheh");
        LCD_ShowString(30,70,200,16,16,"TFTLCD TEST");
        LCD_ShowString(30,90,200,16,16,"ATOM@ALIENTEK");
        LCD_ShowString(30,110,200,16,16,lcd_id);		//显示LCD ID
        LCD_ShowString(30,130,200,12,12,"2014/5/4");

        float temp=(float)Get_Adc(ADC_Channel_1)*(3.3/4096);
        u16 integer=(int)temp;
        u16 decimal=(int)((temp-integer)*1000);

        sprintf(adc_str,"ADC input:%d.%3d",integer,decimal);
        LCD_ShowString(30,150,200,16,16,adc_str);
        delay_ms(1000);
    }


}

void test_mcp41050()
{

    MCP_SPI_Init();
    mcp41050_set_Res(28);
    delay_us(2000);
    while(1);
}

//void add_int(int &x,int &y)
//{
//    x=2;
//    y=3;
//}

int  main(void)
{

//    test_timer3();

    //test_lcd();
    //show_adc_on_lcd();
//    test_mcp41050();
    int a,b;
    pos_pid_init(1,1,1,10,10);
//    add_int(a,b);
    return 1;


}

