/**
 ******************************************************************************
 * @file           :
 * @author         : Xiang Guo
 * @brief          : brief
 * @date           : 2023/04/13
 ******************************************************************************
 * @attention
 *
 ******************************************************************************
 */
#ifndef __BDC_MOTOR_H
#define __BDC_MOTOR_H

/* ------------------------------ Includes ------------------------------ */

#include "stm32f4xx_hal.h"
#include "pid.h"

/* ------------------------------ Defines ------------------------------ */

/* ------------------------------ Variable Declarations ------------------------------ */

/* ------------------------------ Typedef ------------------------------ */

/* ------------------------------ Class ------------------------------ */

namespace bdc_motor
{
  class Encoder
  {
  public:
    TIM_HandleTypeDef *p_htim;

    /* 编码器timer捕获脉冲计数值，用来等时间间隔法计算速度（高速） */
    uint16_t count_last;
    uint16_t count_now;

    /* 编码器timer捕获脉冲变化时间戳，用来等脉冲间隔法计算速度（低速） */
    bool last_count_change_flag;
    uint32_t time_count;
    uint32_t last_count_change_time;

    /* 编码器timer重装载值 */
    uint16_t update_arr;

    Encoder(TIM_HandleTypeDef *p_htim, uint16_t update_arr);
    ~Encoder();
    void Update(uint16_t count_now);
  };

  typedef enum
  {
    MOTOR_MODE_VOL,
    MOTOR_MODE_VEL,
    MOTOR_MODE_POS,
  } MotorModeTypeDef;

  typedef enum
  {
    MOTOR_IDLE,
    MOTOR_RUNNING,
    MOTOR_ERROR,
  } MotorStatusTypeDef;

  typedef struct
  {
    /* enable pin */
    GPIO_TypeDef *p_enable_port;
    uint16_t enable_pin;

    /* pwm generation timer */
    TIM_HandleTypeDef *p_pwm_htim;
    uint32_t pwm_channel_positive;
    uint32_t pwm_channel_negative;
    uint32_t pwm_arr;
    float full_vol;

    /* encoder */
    TIM_HandleTypeDef *p_encoder_htim;
    uint16_t encoder_update_arr;
    float pulse_to_rad_ratio;

    /* motor status */
    MotorModeTypeDef mode;
    
    /* pid controllers */
    struct
    {
      float kp, ki, kd;
      float output_limit;
    } vel_pid, pos_pid;

    /* control loop time period */
    float control_time_period_s;
  } MotorInitTypeDef;

  class Motor
  {
  public:
    /* enable pin */
    GPIO_TypeDef *p_enable_port;
    uint16_t enable_pin;

    /* pwm generation timer */
    TIM_HandleTypeDef *p_pwm_htim;
    uint32_t pwm_channel_positive;
    uint32_t pwm_channel_negative;
    uint32_t pwm_arr;

    /* equivalent voltage when pwm is full */
    float full_vol;

    /* encoder */
    Encoder encoder;
    float pulse_to_rad_ratio;

    /* motor status */
    MotorModeTypeDef mode;
    MotorStatusTypeDef status;
    float vel;
    float pos;

    /* pid controllers */
    pid::Pid vel_pid;
    pid::Pid pos_pid;
    
    /* control loop time period */
    float control_time_period_s;
    
    Motor(MotorInitTypeDef *p_motor_init);
    ~Motor();
    void Enable();
    void Disable();
    void SetVelocity(float vel);
    void SetPosition(float pos);
    void SetPosZero();
    void SetVoltage(float vol);
    void SetMode(MotorModeTypeDef mode);
    void ControlLoop();
  };
} // namespace bdc_motor



#endif
