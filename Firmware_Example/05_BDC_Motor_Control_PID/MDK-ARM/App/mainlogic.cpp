/**
  ******************************************************************************
  * @file           : 
  * @author         : Xiang Guo
  * @brief          : 
	* @date           : 2023/04/14
  ******************************************************************************
  * @attention
  *             
  *
  ******************************************************************************
  */

/* ------------------------------ Includes ------------------------------ */

#include "main.h"
#include "tim.h"
#include "bdc_motor.h"

/* ------------------------------ Defines ------------------------------ */

#define PI 3.1415926f

/* ------------------------------ Variables ------------------------------ */

bdc_motor::MotorInitTypeDef motor_init = {
  MOTOR_EN_GPIO_Port,
  MOTOR_EN_Pin,

  &htim2,
  TIM_CHANNEL_3,
  TIM_CHANNEL_4,
  PWM_ARR,
  12.0f,

  &htim1,
  ENCODER_ARR,
  2.0f * PI / (500.0f * 34.0f * 4),

  bdc_motor::MOTOR_MODE_VEL,

  {6.0f, 100.00f, 0.005f, 12.0f}, // vel_pid
  {50.0f, 0.4f, 0.2f, 20.0f}, // pos_pid

  0.001f, // control_time_period_s
};

bdc_motor::Motor motor(&motor_init);

float vol = 0;
float vel = 0;

/* ------------------------------ Functions ------------------------------ */

#ifdef __cplusplus
extern "C" {
#endif

void Setup(void);
void Loop(void);

#ifdef __cplusplus
}
#endif

void Setup(void)
{
  HAL_TIM_Base_Start_IT(&htim4);
  HAL_TIM_Encoder_Start(motor.encoder.p_htim, TIM_CHANNEL_ALL);
}

void Loop(void)
{
	
  if (HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin) == GPIO_PIN_RESET)
  {
    motor.Enable();
		motor.SetMode(bdc_motor::MOTOR_MODE_POS);
    HAL_Delay(100);
  }
  if (HAL_GPIO_ReadPin(KEY2_GPIO_Port, KEY2_Pin) == GPIO_PIN_RESET)
  {
    motor.Disable();
    HAL_Delay(100);
  }
  if (HAL_GPIO_ReadPin(KEY3_GPIO_Port, KEY3_Pin) == GPIO_PIN_RESET)
  {
    vel += 5.0f;
    motor.SetPosition(vel);
    HAL_Delay(100);
  }
  if (HAL_GPIO_ReadPin(KEY4_GPIO_Port, KEY4_Pin) == GPIO_PIN_RESET)
  {
    vel -= 5.0f;
    motor.SetPosition(vel);
    HAL_Delay(100);
  }
  
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM4)
  {
    motor.ControlLoop();
  }
}