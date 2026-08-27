/********************************** (C) COPYRIGHT *******************************
 * File Name          : Main.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2021/03/09
 * Description        : ADC示例 — 内部温度 + DMA单通道(PA4)
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#include "CH58x_common.h"

int main()
{
    uint8_t i;

    SetSysClock(CLK_SOURCE_PLL_60MHz);

    /* UART1 调试串口 */
    GPIOA_SetBits(GPIO_Pin_9);
    GPIOA_ModeCfg(GPIO_Pin_8, GPIO_ModeIN_PU);
    GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA);
    UART1_DefInit();
    PRINT("ADC Test Start\n");

    /* ─── 1. 内部温度 (原始值) ─── */
    PRINT("\n--- Temperature (raw ADC) ---\n");
    ADC_InterTSSampInit();
    for (i = 0; i < 10; i++)
    {
        PRINT("%d\n", ADC_ExcutSingleConver());
    }

    /* ─── 2. 单通道轮询 PA4 ─── */
    R8_TEM_SENSOR = 0;  /* 关闭内部温度传感器 */
    PRINT("\n--- Polling CH0 (PA4) ---\n");
    GPIOA_ModeCfg(GPIO_Pin_4, GPIO_ModeIN_Floating);
    ADC_ExtSingleChSampInit(SampleFreq_3_2, ADC_PGA_0);
    ADC_ChannelCfg(0);

    for (i = 0; i < 10; i++)
    {
        PRINT("%d\n", ADC_ExcutSingleConver());
    }

    PRINT("Done\n");
    while (1);
}