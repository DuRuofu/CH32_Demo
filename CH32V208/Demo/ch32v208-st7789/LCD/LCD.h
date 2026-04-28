#ifndef __LCD_H__
#define __LCD_H__

#include "debug.h"
#include "spi.h"

/* 颜色定义 (RGB565) */
#define WHITE       0xFFFF
#define BLACK       0x0000
#define BLUE        0x001F
#define RED         0xF800
#define MAGENTA     0xF81F
#define GREEN       0x07E0
#define CYAN        0x7FFF
#define YELLOW      0xFFE0
#define GRAY        0x8430
#define BRED        0xF81F
#define GRED        0xFFE0
#define GBLUE       0x07FF
#define BROWN       0xBC40
#define BRRED       0xFC07
#define DARKBLUE    0x01CF
#define LIGHTBLUE   0x7D7C
#define GRAYBLUE    0x5458

/* ST7789 命令 */
#define ST7789_NOP      0x00
#define ST7789_SWRESET  0x01
#define ST7789_RDDID    0x04
#define ST7789_RDDST    0x09
#define ST7789_SLPIN    0x10
#define ST7789_SLPOUT   0x11
#define ST7789_PTLON    0x12
#define ST7789_NORON    0x13
#define ST7789_INVOFF   0x20
#define ST7789_INVON    0x21
#define ST7789_DISPOFF  0x28
#define ST7789_DISPON   0x29
#define ST7789_CASET    0x2A
#define ST7789_RASET    0x2B
#define ST7789_RAMWR    0x2C
#define ST7789_RAMRD    0x2E
#define ST7789_PTLAR    0x30
#define ST7789_COLMOD   0x3A
#define ST7789_MADCTL   0x36

#define USING_240X240
#define ST7789_ROTATION 0

#ifdef USING_240X240
    #define ST7789_WIDTH   240
    #define ST7789_HEIGHT  240

    #if ST7789_ROTATION == 0
        #define X_SHIFT 0
        #define Y_SHIFT 0
    #elif ST7789_ROTATION == 1
        #define X_SHIFT 0
        #define Y_SHIFT 0
    #elif ST7789_ROTATION == 2
        #define X_SHIFT 0
        #define Y_SHIFT 0
    #elif ST7789_ROTATION == 3
        #define X_SHIFT 0
        #define Y_SHIFT 0
    #endif
#endif

/* 硬件连接 - 使用 SPI2 (参考官方示例)
 * SCK: PB13, MOSI: PB15, MISO: PB14
 * CS: PB12, DC: PB10, RST: PB11, LED: PB9
 */
#define ST7789_DC_PORT   GPIOB
#define ST7789_DC_PIN    GPIO_Pin_10

#define ST7789_CS_PORT   GPIOB
#define ST7789_CS_PIN    GPIO_Pin_12

#define ST7789_RST_PORT  GPIOB
#define ST7789_RST_PIN    GPIO_Pin_11

#define ST7789_LED_PORT  GPIOB
#define ST7789_LED_PIN   GPIO_Pin_9

/* 控制宏 */
#define ST7789_DC_Clr()     GPIO_WriteBit(ST7789_DC_PORT, ST7789_DC_PIN, 0)
#define ST7789_DC_Set()     GPIO_WriteBit(ST7789_DC_PORT, ST7789_DC_PIN, 1)

#define ST7789_CS_Clr()     GPIO_WriteBit(ST7789_CS_PORT, ST7789_CS_PIN, 0)
#define ST7789_CS_Set()     GPIO_WriteBit(ST7789_CS_PORT, ST7789_CS_PIN, 1)

#define ST7789_RST_Clr()    GPIO_WriteBit(ST7789_RST_PORT, ST7789_RST_PIN, 0)
#define ST7789_RST_Set()    GPIO_WriteBit(ST7789_RST_PORT, ST7789_RST_PIN, 1)

#define ST7789_LED_Clr()    GPIO_WriteBit(ST7789_LED_PORT, ST7789_LED_PIN, 0)
#define ST7789_LED_Set()    GPIO_WriteBit(ST7789_LED_PORT, ST7789_LED_PIN, 1)

/* 函数声明 */
void LCD_Init(void);
void LCD_Fill(uint16_t color);
void ST7789_WriteCommand(uint8_t cmd);
void ST7789_WriteData(uint8_t data);
void ST7789_WriteDataArr(uint8_t *buff, size_t buff_size);
void ST7789_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void ST7789_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void ST7789_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
void ST7789_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void ST7789_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *data);

#endif
