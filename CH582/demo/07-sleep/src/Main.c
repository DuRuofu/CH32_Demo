/********************************** (C) COPYRIGHT *******************************
 * File Name          : Main.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2020/08/06
 * Description        : Sleep mode demo — PB4 wake, 4 sleep levels
 *********************************************************************************/

#include "CH58x_common.h"

int main()
{
    SetSysClock(CLK_SOURCE_PLL_60MHz);

    /* UART1 debug */
    GPIOA_SetBits(GPIO_Pin_9);
    GPIOA_ModeCfg(GPIO_Pin_8, GPIO_ModeIN_PU);
    GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA);
    UART1_DefInit();
    PRINT("Sleep Test Start\n");

    /* Wake source: PB4 button, press = low (falling edge) */
    GPIOB_ModeCfg(GPIO_Pin_4, GPIO_ModeIN_PU);
    GPIOB_ITModeCfg(GPIO_Pin_4, GPIO_ITMode_FallEdge);
    PFIC_EnableIRQ(GPIO_B_IRQn);
    PWR_PeriphWakeUpCfg(ENABLE, RB_SLP_GPIO_WAKE, Long_Delay);

    /* Level 1: Idle */
    PRINT("Idle sleep...\n");
    DelayMs(1);
    LowPower_Idle();
    PRINT("Idle wake\n\n");

    /* Level 2: Halt */
    PRINT("Halt sleep...\n");
    DelayMs(2);
    LowPower_Halt();
    HSECFG_Current(HSE_RCur_100);
    PRINT("Halt wake\n\n");

    /* Level 3: Sleep — keep 32KB RAM, GPIO wake */
    PRINT("Sleep mode...\n");
    DelayMs(2);
    LowPower_Sleep(RB_PWR_RAM30K | RB_PWR_RAM2K);
    HSECFG_Current(HSE_RCur_100);
    PRINT("Sleep wake\n\n");

    /* Level 4: Shutdown — wake = full reset */
    PRINT("Shutdown mode...\n");
    DelayMs(2);
    LowPower_Shutdown(0);
    HSECFG_Current(HSE_RCur_100);
    PRINT("Shutdown wake\n");

    while (1);
}

__INTERRUPT
__HIGH_CODE
void GPIOB_IRQHandler(void)
{
    GPIOB_ClearITFlagBit(GPIO_Pin_4);
}
