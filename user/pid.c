//
// Created by sunqi on 17-9-22.
//

#include "pid.h"

/*
 * 由于参数中采用了引用的方式这是c++才支持的特性 所以我吧文件后缀改成了*.cpp
 * 同时在.h的文件中使用了宏 来判断该调用者是c还是c++进行不同的编译过程，使得文件能够见人c/c++
 */

float AD_Scalar_Param=1;   //进行AD采样的时候 电压的放缩系数，使得电压输入范围在0-3.3

//这个部分是电压放大的部分mcp41050
float voltage=10;
float R1=10;
float R2=1;
float voltage_bound=50;   //输出电压的边界数值 因为输出电压是有 边界的不能无限大

pos_pid pid;   //首先定义一个
int pos_pid_init_flag=0;  //是否进行过结构体初始化的标志位

pos_pid pos_pid_init(float kp, float ki, float kd, float set_value,int sample_time_ms) {


    pid.set_Value = set_value;
    pid.actual_Value = 0;
    pid.err = 0;
    pid.err_last = set_value;  //默认上一次的误差就是与设定值的差值
    pid.control_value = 0;
    pid.intergral_value = 0;
    pid.kp = kp;
    pid.ki = ki;
    pid.kd = kd;
    pid.sample_time=sample_time_ms;   //默认采样时间10ms
    pos_pid_param_update();

    Adc_Init();    //开启ADC1的初始化 并且指定了GPIOA_1  方便后面函数调用adc采集信号
    pid_timer_init(sample_time_ms,7199);   //定时器的定时为10ms

    return pid;
}



//通过传感器获得当前的状态数据
void get_pos_pid_actual() {
    //这个部分的代码需要进行ADC单次采集电路修改初始化的代码建议单独放在一个地方，这个地方只负责采集
    //   pid.actual_Value
    float temp;
    temp = (float) Get_Adc(ADC_Channel_1) * (3.3 / 4096*AD_Scalar_Param);   //stm32 adc的精度为12
    pid.actual_Value = temp;

}



//对于t-1时刻额数据，在t时刻进行数据的更新
void pos_pid_param_update() {

    pid.err_last = pid.err;   //更新上一次的误差为当前的
    get_pos_pid_actual();    //通过传感器获得当前的控制状态，并修改该数值 pid.actual_value
    pid.err = pid.set_Value - pid.actual_Value;     //更新误差
    pid.intergral_value += pid.err*(pid.sample_time/1000);       //更新积分部分数值

    //更新控制部分的数值
    pid.control_value = pid.kp * pid.err + pid.ki * pid.intergral_value + pid.kd * (pid.err - pid.err_last);
}
/*
 * v0 *(1+R2/R1)= control_value
 * R2=50K * (set_value)/255
 * 已知control_value  求set_res
 */
int  output_transform(float control_value)
{
    float r2=(control_value/voltage-1)*R1;
    int set_value=r2 * 255/50;
    return set_value;
}

//在这个函数里设置一个定时器，定时对进行控制
void pos_pid_control() {
    //这里需要设置一个定时器，使用中断的方式定时进行控制输出的更新
    int writedata=output_transform(pid.control_value);

    if(writedata>255){
        mcp41050_set_Res(255);
    }
    else{
        mcp41050_set_Res(writedata);
    }

}

void pid_timer_init(int arr, int psc)
{
    //f_out=72M/((arr+1)*(psc+1))  psc=7199的时候单个arr时间0.1ms
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

void TIM3_IRQHandler(){
//    if(pos_pid_init_flag==0)
//    {
//        pos_pid_init(1,1,1,10,10);
//        pos_pid_init_flag=1;
//    }
    pos_pid_param_update();   //更新数值
    pos_pid_control();        //输出新的控制变量
}


