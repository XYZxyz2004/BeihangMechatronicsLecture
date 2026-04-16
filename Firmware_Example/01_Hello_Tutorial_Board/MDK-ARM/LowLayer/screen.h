/**
  ******************************************************************************
  * @file           : screen.h
  * @author         : Mark Xiang
  * @brief          : brief
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */
#ifndef __SCREEN_H
#define __SCREEN_H

/* ------------------------------ Includes ------------------------------ */

#include "stm32f4xx_hal.h"
#include "stdint.h"

/* ------------------------------ Defines ------------------------------ */

#define RGB888_2_RGB565(R,G,B)		 (uint16_t)(((R & 0xF8) << 8) | ((G & 0xFC) << 3) | (B & 0xF8) >> 3)
#define LCDGUI_ShowImage(x, y, Width, Height, p) {Lcd_Clear(WHITE);HAL_Delay(1000);\
    for(uint32_t i = 0;i < Width;i++)for(uint32_t j = 0;j < Height;j++)Lcd_DrawPoint(j + x, i + y, RGB888_2_RGB565(p[i][j][0], p[i][j][1], p[i][j][2]));} 

#define LCD_X_SIZE	        130
#define LCD_Y_SIZE	        130

#define USE_HORIZONTAL  		0

#if USE_HORIZONTAL
#define X_MAX_PIXEL	        LCD_Y_SIZE
#define Y_MAX_PIXEL	        LCD_X_SIZE
#else
#define X_MAX_PIXEL	        LCD_X_SIZE
#define Y_MAX_PIXEL	        LCD_Y_SIZE
#endif

//--------------------颜色定义---------------------------------//
#define RED  		0xf800    //红色
#define GREEN		0x07e0    //绿色
#define BLUE 		0x001f    //蓝色
#define SBLUE 	    0x251F    //淡蓝色
#define WHITE		0xffff    //白色
#define BLACK		0x0000    //黑色
#define YELLOW      0xFFE0    //黄色
#define GRAY0       0xEF7D    //灰色0 1110 1111 0111 1100 
#define GRAY1       0x8410    //灰色1 1000 0100 0001 0000
#define GRAY2       0x4208    //灰色2 0100 0010 0000 1000

/* ------------------------------ Enum Typedef ------------------------------ */

/* ------------------------------ Struct Typedef ------------------------------ */

/* ------------------------------ Variable Declarations ------------------------------ */

/* ------------------------------ Manager Typedef ------------------------------ */

/* ------------------------------ Manager Extern ------------------------------ */
void Lcd_Init(void);
void Lcd_SetRegion(uint8_t xStar, uint8_t yStar, uint8_t xEnd, uint8_t yEnd);
void Lcd_DrawPoint(uint16_t x, uint16_t y, uint16_t data);
void Lcd_Clear(uint16_t Color);
void Lcd_WriteData16Bit(uint16_t data);

#endif

