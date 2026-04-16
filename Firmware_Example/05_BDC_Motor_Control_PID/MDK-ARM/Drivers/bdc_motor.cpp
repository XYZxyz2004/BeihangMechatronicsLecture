/**
 ******************************************************************************
 * @file           :
 * @author         : Xiang Guo
 * @brief          : 
 * @date					 : 2023/04/13
 ******************************************************************************
 * @attention
 *
 *
 ******************************************************************************
 */

/* ------------------------------ Includes ------------------------------ */

#include "bdc_motor.h"

/* ------------------------------ Defines ------------------------------ */

/* ------------------------------ Variables ------------------------------ */

/* ------------------------------ Functions ------------------------------ */

namespace bdc_motor
{
  Encoder::Encoder(TIM_HandleTypeDef *p_htim, uint16_t update_arr)
  {
    this->p_htim = p_htim;
    this->update_arr = update_arr;
    this->count_last = 0;
    this->count_now = 0;
    this->last_count_change_flag = 1;
    this->time_count = 0;
    this->last_count_change_time = 0;
  }

  Encoder::~Encoder()
  {
  }

  void Encoder::Update(uint16_t count_now)
  {
    this->count_last = this->count_now;
    this->count_now = count_now;
    if (this->count_now != this->count_last)
    {
      this->last_count_change_flag = 1;
      this->last_count_change_time = this->time_count;
    }
    else
    {
      this->last_count_change_flag = 0;
    }
    this->time_count++;
  }

  Motor::Motor(MotorInitTypeDef *p_motor_init) :
  encoder(p_motor_init->p_encoder_htim, p_motor_init->encoder_update_arr), pos_pid(p_motor_init->pos_pid.kp, p_motor_init->pos_pid.ki, p_motor_init->pos_pid.kd, p_motor_init->pos_pid.output_limit, p_motor_init->control_time_period_s), vel_pid(p_motor_init->vel_pid.kp, p_motor_init->vel_pid.ki, p_motor_init->vel_pid.kd, p_motor_init->vel_pid.output_limit, p_motor_init->control_time_period_s)
  {
    this->p_enable_port = p_motor_init->p_enable_port;
    this->enable_pin = p_motor_init->enable_pin;

    this->p_pwm_htim = p_motor_init->p_pwm_htim;
    this->pwm_channel_positive = p_motor_init->pwm_channel_positive;
    this->pwm_channel_negative = p_motor_init->pwm_channel_negative;
    this->pwm_arr = p_motor_init->pwm_arr;
    this->full_vol = p_motor_init->full_vol;

    this->pulse_to_rad_ratio = p_motor_init->pulse_to_rad_ratio;

    this->mode = p_motor_init->mode;
    this->status = MOTOR_IDLE;
    this->pos = 0;
    this->vel = 0;

    this->control_time_period_s = p_motor_init->control_time_period_s;
  }

  Motor::~Motor()
  {
  }

  void Motor::Enable(void)
  {
    HAL_GPIO_WritePin(this->p_enable_port, this->enable_pin, GPIO_PIN_SET);
    this->status = MOTOR_RUNNING;
  }

  void Motor::Disable(void)
  {
    HAL_GPIO_WritePin(this->p_enable_port, this->enable_pin, GPIO_PIN_RESET);
    HAL_TIM_PWM_Stop(this->p_pwm_htim, this->pwm_channel_positive);
    HAL_TIM_PWM_Stop(this->p_pwm_htim, this->pwm_channel_negative);
    this->status = MOTOR_IDLE;
  }

  void Motor::SetMode(MotorModeTypeDef mode)
  {
    this->mode = mode;
  }

  void Motor::SetVelocity(float vel)
  {
    this->vel_pid.target = vel;
  }

  void Motor::SetPosition(float pos)
  {
    this->pos_pid.target = pos;
  }

  void Motor::SetPosZero(void)
  {
    this->pos = 0;
  }

  void Motor::SetVoltage(float vol)
  {
    if (vol > this->full_vol)
    {
      vol = this->full_vol;
    }
    else if (vol < -this->full_vol)
    {
      vol = -this->full_vol;
    }
    if (vol > 0)
    {
      HAL_TIM_PWM_Stop(this->p_pwm_htim, this->pwm_channel_negative);
      __HAL_TIM_SET_COMPARE(this->p_pwm_htim, this->pwm_channel_positive, (uint32_t)(vol / this->full_vol * this->pwm_arr));
      HAL_TIM_PWM_Start(this->p_pwm_htim, this->pwm_channel_positive);
    }
    else if (vol < 0)
    {
      HAL_TIM_PWM_Stop(this->p_pwm_htim, this->pwm_channel_positive);
      __HAL_TIM_SET_COMPARE(this->p_pwm_htim, this->pwm_channel_negative, (uint32_t)(-vol / this->full_vol * this->pwm_arr));
      HAL_TIM_PWM_Start(this->p_pwm_htim, this->pwm_channel_negative);
    }
    else
    {
      HAL_TIM_PWM_Stop(this->p_pwm_htim, this->pwm_channel_positive);
      HAL_TIM_PWM_Stop(this->p_pwm_htim, this->pwm_channel_negative);
    }
  }

  void Motor::ControlLoop(void)
  {
    /* update time */
    this->encoder.time_count++;

    /* update pos */
    this->encoder.Update(__HAL_TIM_GET_COUNTER(this->encoder.p_htim));
    float overflow = this->encoder.count_now - this->encoder.count_last < -this->encoder.update_arr / 2 ? 1 : (this->encoder.count_now - this->encoder.count_last > this->encoder.update_arr / 2 ? -1 : 0);
    // count to rad
    float pos_change = (this->encoder.count_now - this->encoder.count_last + overflow * this->encoder.update_arr) * this->pulse_to_rad_ratio;
    this->pos += pos_change;

    /* update vel */
    // 如果计数器发生变化，则按照等时间间隔法计算速度
    if (this->encoder.count_now != this->encoder.count_last)
    {
      if (this->encoder.last_count_change_flag)
      {
        this->vel = pos_change / this->control_time_period_s;
      }
      else
      {
        this->vel = 1.0f * this->pulse_to_rad_ratio / ((this->encoder.time_count - this->encoder.last_count_change_time) * this->control_time_period_s);
        this->encoder.last_count_change_flag = 1;
      }
      this->encoder.last_count_change_time = this->encoder.time_count;
    }
    // 如果计数器没有发生变化，则按照等脉冲间隔法计算速度
    else
    {
      this->encoder.last_count_change_flag = 0;
      this->vel = 1.0f * this->pulse_to_rad_ratio / ((this->encoder.time_count - this->encoder.last_count_change_time) * this->control_time_period_s);
    }

    /* update pid */
    if (this->status == MOTOR_RUNNING)
    {
      if (this->mode == MOTOR_MODE_VEL)
      {
        this->SetVoltage(this->vel_pid.Calc(this->vel));
      }
      else if (this->mode == MOTOR_MODE_POS)
      {
        this->vel_pid.SetTarget(this->pos_pid.Calc(this->pos));
        this->SetVoltage(this->vel_pid.Calc(this->vel));
      }
    }
  }
} // namespace bdc_motor
