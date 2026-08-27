/********************************** (C) COPYRIGHT *******************************
 * File Name          : Main.c
 * Description        : GPIO input/output test
 *******************************************************************************/

#include "CH59x_common.h"

#define KEY_PIN     GPIO_Pin_4       /* PB4, active low */
#define TEST_PIN    GPIO_Pin_13      /* PA13, output for LED/scope */

static void DebugInit(void)
{
    GPIOA_SetBits(GPIO_Pin_9);
    GPIOA_ModeCfg(GPIO_Pin_8, GPIO_ModeIN_PU);
    GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA);
    UART1_DefInit();
}

int main(void)
{
    uint8_t previous = 1;
    uint8_t current;

    SetSysClock(CLK_SOURCE_PLL_60MHz);
    DebugInit();

    GPIOB_ModeCfg(KEY_PIN, GPIO_ModeIN_PU);
    GPIOA_ResetBits(TEST_PIN);
    GPIOA_ModeCfg(TEST_PIN, GPIO_ModeOut_PP_5mA);

    PRINT("\r\n=== DEMO GPIO ===\r\n");
    PRINT("PB4 input (active low), PA13 output\r\n");

    while(1)
    {
        current = (GPIOB_ReadPortPin(KEY_PIN) != 0);
        if(current != previous)
        {
            if(current)
            {
                GPIOA_ResetBits(TEST_PIN);
                PRINT("PB4 released, PA13=0\r\n");
            }
            else
            {
                GPIOA_SetBits(TEST_PIN);
                PRINT("PB4 pressed, PA13=1\r\n");
            }
            previous = current;
        }
        DelayMs(20);
    }
}
