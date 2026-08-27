/********************************** (C) COPYRIGHT *******************************
 * File Name          : Main.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2020/08/06
 * Description        : GPIO按键输入示例 — PB4按下串口输出
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#include "CH58x_common.h"

/*********************************************************************
 * @fn      main
 *
 * @brief   主函数 — 轮询PB4按键，按下时串口打印
 *
 * @return  none
 */
int main()
{
    uint8_t key_curr = 1;   // 当前按键状态 (1=释放)
    uint8_t key_prev = 1;   // 上次按键状态

    SetSysClock(CLK_SOURCE_PLL_60MHz);

    /* UART1 初始化：PA9=TXD, PA8=RXD, 115200 */
    GPIOA_SetBits(GPIO_Pin_9);
    GPIOA_ModeCfg(GPIO_Pin_8, GPIO_ModeIN_PU);
    GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA);
    UART1_DefInit();

    /* PB4 按键输入：内部上拉，按下为低电平 */
    GPIOB_ModeCfg(GPIO_Pin_4, GPIO_ModeIN_PU);

    UART1_SendString("GPIO Key Test Start\r\n", 22);

    while(1)
    {
        /* GPIOB_ReadPortPin 返回 pin 对应 bit 值（0x10 或 0），非 0 即高电平 */
        key_curr = (GPIOB_ReadPortPin(GPIO_Pin_4) != 0);

        /* 下降沿检测：上次高 本次低 = 按下 */
        if (key_prev && !key_curr)
        {
            UART1_SendString("key pressed\r\n", 13);
        }

        key_prev = key_curr;

        DelayMs(20);  // 消抖延时 + 扫描间隔
    }
}
