/********************************** (C) COPYRIGHT *******************************
 * File Name          : Main.c
 * Description        : TMR0 periodic interrupt test
 *******************************************************************************/

#include "CH59x_common.h"

#define TEST_PIN GPIO_Pin_13

volatile uint32_t timer_ticks;

static void DebugInit(void)
{
    GPIOA_SetBits(GPIO_Pin_9);
    GPIOA_ModeCfg(GPIO_Pin_8, GPIO_ModeIN_PU);
    GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA);
    UART1_DefInit();
}

int main(void)
{
    uint32_t last_tick = 0;

    SetSysClock(CLK_SOURCE_PLL_60MHz);
    DebugInit();

    GPIOA_ResetBits(TEST_PIN);
    GPIOA_ModeCfg(TEST_PIN, GPIO_ModeOut_PP_5mA);
    TMR0_TimerInit(FREQ_SYS);                 /* one second */
    TMR0_ITCfg(ENABLE, TMR0_3_IT_CYC_END);
    PFIC_EnableIRQ(TMR0_IRQn);

    PRINT("\r\n=== DEMO TIMER ===\r\n");
    PRINT("TMR0 period=1 s, PA13 toggles\r\n");

    while(1)
    {
        if(last_tick != timer_ticks)
        {
            last_tick = timer_ticks;
            PRINT("tick=%d s\r\n", last_tick);
        }
    }
}

__INTERRUPT
__HIGH_CODE
void TMR0_IRQHandler(void)
{
    if(TMR0_GetITFlag(TMR0_3_IT_CYC_END))
    {
        TMR0_ClearITFlag(TMR0_3_IT_CYC_END);
        timer_ticks++;
        GPIOA_InverseBits(TEST_PIN);
    }
}
