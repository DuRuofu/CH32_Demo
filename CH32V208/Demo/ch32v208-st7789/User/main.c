/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2021/06/06
 * Description        : Main program body.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*
 *@Note
 *USART Print debugging routine:
 *USART1_Tx(PA9).
 *This example demonstrates using USART1(PA9) as a print debug port output.
 *
 */

#include "debug.h"
#include "LCD.h"
#include "logo.h"

/* Global typedef */

/* Global define */

/* Global Variable */

static uint16_t lcd_x, lcd_y;
static uint16_t lcd_r;
static int16_t lcd_dx, lcd_dy, lcd_dist_sq;

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();
    USART_Printf_Init(115200);
    printf("SystemClk:%d\r\n", SystemCoreClock);
    printf("ChipID:%08x\r\n", DBGMCU_GetCHIPID());
    printf("ST7789 Test Program\r\n");

    LCD_Init();
    printf("LCD Init Complete\r\n");

    /* Test 1: Fill screen with WHITE */
    printf("Test 1: Fill WHITE\r\n");
    LCD_Fill(WHITE);
    Delay_Ms(500);

    /* Test 2: Fill screen with RED */
    printf("Test 2: Fill RED\r\n");
    LCD_Fill(RED);
    Delay_Ms(500);

    /* Test 3: Fill screen with GREEN */
    printf("Test 3: Fill GREEN\r\n");
    LCD_Fill(GREEN);
    Delay_Ms(500);

    /* Test 4: Fill screen with BLUE */
    printf("Test 4: Fill BLUE\r\n");
    LCD_Fill(BLUE);
    Delay_Ms(500);

    /* Test 5: Draw pixels - diagonal line */
    printf("Test 5: Draw Diagonal Line (WHITE)\r\n");
    LCD_Fill(BLACK);
    ST7789_DrawLine(0, 0, ST7789_WIDTH-1, ST7789_HEIGHT-1, WHITE);
    ST7789_DrawLine(0, ST7789_HEIGHT-1, ST7789_WIDTH-1, 0, WHITE);
    Delay_Ms(500);

    /* Test 6: Draw rectangles */
    printf("Test 6: Draw Rectangles\r\n");
    LCD_Fill(BLACK);
    ST7789_DrawRectangle(10, 10, 100, 80, RED);
    ST7789_DrawRectangle(120, 60, 200, 140, GREEN);
    ST7789_DrawRectangle(220, 100, 310, 200, BLUE);
    Delay_Ms(1000);

    /* Test 7: Draw pixels - grid pattern */
    printf("Test 7: Draw Grid Pattern\r\n");
    LCD_Fill(BLACK);
    for(lcd_x = 0; lcd_x < ST7789_WIDTH; lcd_x += 20)
    {
        ST7789_DrawLine(lcd_x, 0, lcd_x, ST7789_HEIGHT-1, DARKBLUE);
    }
    for(lcd_y = 0; lcd_y < ST7789_HEIGHT; lcd_y += 20)
    {
        ST7789_DrawLine(0, lcd_y, ST7789_WIDTH-1, lcd_y, DARKBLUE);
    }
    Delay_Ms(1000);

    /* Test 8: Draw circles (using pixels) */
    printf("Test 8: Draw Circles\r\n");
    LCD_Fill(BLACK);
    for(lcd_r = 10; lcd_r <= 50; lcd_r += 10)
    {
        for(lcd_x = 0; lcd_x < ST7789_WIDTH; lcd_x++)
        {
            for(lcd_y = 0; lcd_y < ST7789_HEIGHT; lcd_y++)
            {
                lcd_dx = lcd_x - 160;
                lcd_dy = lcd_y - 120;
                lcd_dist_sq = lcd_dx*lcd_dx + lcd_dy*lcd_dy;
                if(lcd_dist_sq >= ((lcd_r-1)*(lcd_r-1)) && lcd_dist_sq <= ((lcd_r+1)*(lcd_r+1)))
                {
                    ST7789_DrawPixel(lcd_x, lcd_y, YELLOW);
                }
            }
        }
    }
    Delay_Ms(1000);

    /* Test 9: Display logo image */
    printf("Test 9: Display Logo\r\n");
    LCD_Fill(BLACK);
    ST7789_DrawImage(0, 0, 240, 240, (const uint16_t *)gImage_logo_240x240_16bit);
    Delay_Ms(2000);

    /* Final: Display logo and hold */
    printf("Test Complete: Logo Display\r\n");
    LCD_Fill(BLACK);
    ST7789_DrawImage(0, 0, 240, 240, (const uint16_t *)gImage_logo_240x240_16bit);

    while(1)
    {
        /* 保持Logo显示 */
    }
}
