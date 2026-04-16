/**
  ******************************************************************************
  * @file           : screen.c
  * @author         : Mark Xiang
  * @brief          : 
  ******************************************************************************
  * @attention
  *             
  *
  ******************************************************************************
  */

/* ------------------------------ Includes ------------------------------ */

#include "screen.h"
#include "main.h"
#include "spi.h"

/* ------------------------------ Defines ------------------------------ */

/* ------------------------------ Private Function Declarations ------------------------------ */

static void prvWriteCmd(uint8_t cmd);
static void prvWriteData(uint8_t data);
static void prvWriteData16Bit(uint16_t data);
static void prvWriteReg(uint8_t index, uint16_t data);
static void prvReset(void);
static void prvSetXY(uint16_t x, uint16_t y);

/* ------------------------------ Public Function Declarations ------------------------------ */

void Lcd_TurnLed(void);
void Lcd_Init(void);
void Lcd_DrawPoint(uint16_t x, uint16_t y, uint16_t Data);

/* ------------------------------ Variable Declarations ------------------------------ */

static uint8_t xmit_buffer[2];

/* ------------------------------ Private ------------------------------ */

static void prvWriteCmd(uint8_t cmd)
{
  xmit_buffer[0] = cmd;
  HAL_GPIO_WritePin(SCR_nCS_GPIO_Port, SCR_nCS_Pin, GPIO_PIN_RESET);    // reset nCS pin
  HAL_GPIO_WritePin(SRC_DC_GPIO_Port, SRC_DC_Pin, GPIO_PIN_RESET);      // reset Data/Cmd pin to write cmd
  HAL_SPI_Transmit(&hspi1, xmit_buffer, 1, 10);
  HAL_GPIO_WritePin(SCR_nCS_GPIO_Port, SCR_nCS_Pin, GPIO_PIN_SET);      // set nCS pin
}

static void prvWriteData(uint8_t data)
{
  xmit_buffer[0] = data;
  HAL_GPIO_WritePin(SCR_nCS_GPIO_Port, SCR_nCS_Pin, GPIO_PIN_RESET);    // reset nCS pin
  HAL_GPIO_WritePin(SRC_DC_GPIO_Port, SRC_DC_Pin, GPIO_PIN_SET);        // set Data/Cmd pin to write data
  HAL_SPI_Transmit(&hspi1, xmit_buffer, 1, 10);
  HAL_GPIO_WritePin(SCR_nCS_GPIO_Port, SCR_nCS_Pin, GPIO_PIN_SET);      // set nCS pin
}

static void prvWriteData16Bit(uint16_t data)
{
  xmit_buffer[0] = data >> 8;
  xmit_buffer[1] = data & 0xFF;
  HAL_GPIO_WritePin(SCR_nCS_GPIO_Port, SCR_nCS_Pin, GPIO_PIN_RESET);    // reset nCS pin
  HAL_GPIO_WritePin(SRC_DC_GPIO_Port, SRC_DC_Pin, GPIO_PIN_SET);        // set Data/Cmd pin to write data
  HAL_SPI_Transmit(&hspi1, xmit_buffer, 2, 10);
  HAL_GPIO_WritePin(SCR_nCS_GPIO_Port, SCR_nCS_Pin, GPIO_PIN_SET);      // set nCS pin
}

static void prvWriteReg(uint8_t index, uint16_t data)
{
  prvWriteCmd(index);
  prvWriteData16Bit(data);
}

static void prvReset(void)
{
  HAL_GPIO_WritePin(SCR_RST_GPIO_Port, SCR_RST_Pin, GPIO_PIN_RESET);    // reset nRST pin
  HAL_Delay(100);
  HAL_GPIO_WritePin(SCR_RST_GPIO_Port, SCR_RST_Pin, GPIO_PIN_SET);      // set nRST pin
  HAL_Delay(50);
}

static void prvSetXY(uint16_t x, uint16_t y)
{
#if USE_HORIZONTAL            // if use horizontal mode
	prvWriteCmd(0x21);
	prvWriteData(x);
	prvWriteData(y + 2);	
#else                         // if use vertical mode
	prvWriteCmd(0x21);
	prvWriteData(y + 2);
	prvWriteData(x + 2);	
#endif
	prvWriteCmd(0x22);
}

/* ------------------------------ Public ------------------------------ */

void Lcd_Init(void)
{
  prvReset(); //Reset before LCD Init.
	HAL_GPIO_WritePin(SRC_LED_GPIO_Port, SRC_LED_Pin, GPIO_PIN_SET); //点亮背光
	prvWriteReg(0x10, 0x2F8E); /* power control 1 */
	prvWriteReg(0x11, 0x000C); /* power control 2 */
  prvWriteReg(0x07, 0x0021); /* display control */
  prvWriteReg(0x28, 0x0006); /* vcom OTP */
  prvWriteReg(0x28, 0x0005);
  prvWriteReg(0x27, 0x057F); /* further bias current setting */
  prvWriteReg(0x29, 0x89A1); /* vcom OTP */
  prvWriteReg(0x00, 0x0001); /* OSC en */
  HAL_Delay(100);
  prvWriteReg(0x29, 0x80B0); /* vcom OTP */
  HAL_Delay(30);
  prvWriteReg(0x29, 0xFFFE); /* vcom OTP */
  prvWriteReg(0x07, 0x0023); /* display control */
  HAL_Delay(30);
  prvWriteReg(0x07, 0x0033); /* display control */
#if USE_HORIZONTAL
  prvWriteReg(0x01, 0x2283); /* driver output control, REV, TB=0, RL=1, RGB */
  prvWriteReg(0x03, 0x6838); /* entry mode, 65K, ram, ID3 */
#else
  prvWriteReg(0x01, 0x2183); /* driver output control, REV, TB, RGB */
  prvWriteReg(0x03, 0x6830); /* entry mode, 65K, ram, ID0 */
#endif
	prvWriteReg(0x2F, 0xFFFF); /* 2A ~ 2F, test */
  prvWriteReg(0x2C, 0x8000);
	prvWriteReg(0x27, 0x0570);
  prvWriteReg(0x02, 0x0300); /* driving wave form control */
  prvWriteReg(0x0B, 0x580C); /* frame cycle control */
  prvWriteReg(0x12, 0x0609); /* power control 3 */
  prvWriteReg(0x13, 0x3100); /* power control 4 */
}

void Lcd_SetRegion(uint8_t xStar, uint8_t yStar, uint8_t xEnd, uint8_t yEnd)
{
	
#if USE_HORIZONTAL
	prvWriteCmd(0x45);
	prvWriteData(xEnd);
	prvWriteData(xStar);	
	
	prvWriteCmd(0x44);
	prvWriteData(yEnd+2);
	prvWriteData(yStar+2);	
#else
	prvWriteCmd(0x44);
	prvWriteData(xEnd+2);
	prvWriteData(xStar+2);	
	
	prvWriteCmd(0x45);
	prvWriteData(yEnd+2);
	prvWriteData(yStar+2);	
#endif
	prvSetXY(xStar, yStar);
}

void Lcd_DrawPoint(uint16_t x, uint16_t y, uint16_t data)
{
  prvSetXY(x, y);
  prvWriteData16Bit(data);
}

void Lcd_Clear(uint16_t Color)               
{	
   unsigned int i,m;
   Lcd_SetRegion(0,0,X_MAX_PIXEL-1,Y_MAX_PIXEL-1);
   for(i=0;i<X_MAX_PIXEL;i++)
    for(m=0;m<Y_MAX_PIXEL;m++)
    {	
	  	prvWriteData16Bit(Color);
    }
}

void Lcd_WriteData16Bit(uint16_t data)
{
  xmit_buffer[0] = data >> 8;
  xmit_buffer[1] = data & 0xFF;
  HAL_GPIO_WritePin(SCR_nCS_GPIO_Port, SCR_nCS_Pin, GPIO_PIN_RESET);    // reset nCS pin
  HAL_GPIO_WritePin(SRC_DC_GPIO_Port, SRC_DC_Pin, GPIO_PIN_SET);        // set Data/Cmd pin to write data
  HAL_SPI_Transmit(&hspi1, xmit_buffer, 2, 10);
  HAL_GPIO_WritePin(SCR_nCS_GPIO_Port, SCR_nCS_Pin, GPIO_PIN_SET);      // set nCS pin
}

/* ------------------------------ Manager Declaration ------------------------------ */
