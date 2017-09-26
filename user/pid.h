//
// Created by sunqi on 17-9-22.
//

#ifndef MCU_PID_H
#define MCU_PID_H

#include <arm_math.h>
#include "pid.h"
#include "adc.h"
#include "mcp41050.h"

typedef struct
{
    float set_Value;        //定义的设定值
    float actual_Value;     //定义的实际值
    float err;              //当前误差
    float err_last;         //定义上一次的偏差值
    float kp,ki,kd;         //pid 三个参数
    float control_value;    //执行器的控制变量
    float intergral_value;  //积分的数值
    float sample_time;     //采样时间 ms
}pos_pid;

typedef struct
{
    float set_Value;        //定义的设定值
    float actual_Value;     //定义的实际值
    float err;              //当前误差
    float err_next;         //定义上一次的偏差值
    float err_last;         //定义倒数第二次的偏差值
    float kp,ki,kd;         //pid 三个参数
    float sample_time;     //采样时间
}add_pos;
//通过传感器获得当前的状态数据

pos_pid pos_pid_init(float kp, float ki, float kd, float set_value,int sample_time_ms);



#ifdef __cplusplus
extern "C"
{
#endif



#ifdef __cplusplus
}
#endif

#endif //MCU_PID_H
