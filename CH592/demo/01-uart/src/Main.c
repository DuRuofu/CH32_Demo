/********************************** (C) COPYRIGHT *******************************
 * File Name          : Main.c
 * Description        : UART1 serial receive/transmit test
 *******************************************************************************/

#include "CH59x_common.h"

static void DebugInit(void)
{
    GPIOA_SetBits(GPIO_Pin_9);
    GPIOA_ModeCfg(GPIO_Pin_8, GPIO_ModeIN_PU);      /* UART1 RX */
    GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA); /* UART1 TX */
    UART1_DefInit();                                 /* 115200 8N1 */
}

int main(void)
{
    uint8_t rx[64];
    uint16_t len;

    SetSysClock(CLK_SOURCE_PLL_60MHz);
    DebugInit();

    PRINT("\r\n=== DEMO UART1 ===\r\n");
    PRINT("PA8=RX PA9=TX 115200 8N1\r\n");
    PRINT("Send text to test echo.\r\n");

    while(1)
    {
        len = UART1_RecvString(rx);
        if(len)
        {
            UART1_SendString(rx, len);
        }
    }
}
