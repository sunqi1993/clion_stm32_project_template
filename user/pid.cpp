//
// Created by sunqi on 17-9-22.
//

#include "pid.h"
#include "adc.h"

/*
 * 由于参数中采用了引用的方式这是c++才支持的特性 所以我吧文件后缀改成了*.cpp
 * 同时在.h的文件中使用了宏 来判断该调用者是c还是c++进行不同的编译过程，使得文件能够见人c/c++
 */

pos_pid pos_pid_init(float kp, float ki, float kd, float set_value) {
    Adc_Init();    //开启ADC1的初始化 并且指定了GPIOA_1  方便后面函数调用adc采集信号

    pos_pid pid;
    pid.set_Value = set_value;
    pid.actual_Value = 0;
    pid.err = 0;
    pid.err_last = set_value;  //默认上一次的误差就是与设定值的差值
    pid.control_value = 0;
    pid.intergral_value = 0;
    pid.kp = kp;
    pid.ki = ki;
    pid.kd = kd;

    pos_pid_param_update(&pid);

    return pid;
}

//通过传感器获得当前的状态数据
void get_pos_pid_actual(pos_pid &pid) {
    //这个部分的代码需要进行ADC单次采集电路修改初始化的代码建议单独放在一个地方，这个地方只负责采集
    //   pid.actual_Value
    float temp;
    temp = (float) Get_Adc(ADC_Channel_1) * (3.3 / 4096);   //stm32 adc的精度为12
    pid.actual_Value = temp;

}

//对于t-1时刻额数据，在t时刻进行数据的更新
void pos_pid_param_update(pos_pid &pid) {
//    pid.err_last=pid.err;
//    get_pos_pid_actual(pid);
//    pid.err = pid.set_Value - pid.actual_Value;
//    pid.intergral_value += pid.err;
//
//    pid.control_value = pid.kp*pid.err +  pid.ki*pid.intergral_value  +  pid.kd*(pid.err-pid.err_last);

    pid.err_last = pid.err;   //更新上一次的误差为当前的
    get_pos_pid_actual(pid);    //通过传感器获得当前的控制状态，并修改该数值 pid.actual_value
    pid.err = pid.set_Value - pid.actual_Value;     //更新误差
    pid.intergral_value += pid.err;       //更新积分部分数值

    //更新控制部分的数值
    pid.control_value = pid.kp * pid.err + pid.ki * pid.intergral_value + pid.kd * (pid.err - pid.err_last);
}

//在这个函数里设置一个定时器，定时对进行控制
void pos_pid_control(pos_pid &pid) {
    //这里需要设置一个定时器，使用中断的方式定时进行控制输出的更新

}

