/********************************** (C) COPYRIGHT *******************************
 * File Name          : Main.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2020/08/06
 * Description        : PWM4输出示例 — PA12，占空比渐变
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#include "CH58x_common.h"

/*********************************************************************
 * @fn      main
 *
 * @brief   PWM4 — PA12, ~104kHz, 占空比 50%
 *          Fsys=60M, div=9, cycle=64 → f=60M/9/64≈104kHz
 *
 * @return  none
 */
int main()
{
    SetSysClock(CLK_SOURCE_PLL_60MHz);

    /* UART1 初始化 — 调试输出 */
    GPIOA_SetBits(GPIO_Pin_9);
    GPIOA_ModeCfg(GPIO_Pin_8, GPIO_ModeIN_PU);
    GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA);
    UART1_DefInit();
    UART1_SendString("PWM4 PA12 104kHz 50%\r\n", 23);

    /* PA12 — PWM4 */
    GPIOA_ModeCfg(GPIO_Pin_12, GPIO_ModeOut_PP_5mA);

    /* f_PWM = Fsys / div / cycle = 60M / 9 / 64 ≈ 104.2 kHz */
    PWMX_CLKCfg(9);
    PWMX_CycleCfg(PWMX_Cycle_64);

    /* 占空比 50%: 64/2 = 32 */
    PWMX_ACTOUT(CH_PWM4, 32, Low_Level, ENABLE);

    while(1);
}
