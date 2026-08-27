/********************************** (C) COPYRIGHT *******************************
 * File Name          : Main.c
 * Description        : ADC input sampling test
 *******************************************************************************/

#include "CH59x_common.h"

#define ADC_PIN GPIO_Pin_4             /* PA4 / AIN0 */
#define ADC_SAMPLES 16

static void DebugInit(void)
{
    GPIOA_SetBits(GPIO_Pin_9);
    GPIOA_ModeCfg(GPIO_Pin_8, GPIO_ModeIN_PU);
    GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA);
    UART1_DefInit();
}

static uint16_t ReadAdcAverage(void)
{
    uint8_t i;
    uint32_t sum = 0;
    for(i = 0; i < ADC_SAMPLES; i++)
    {
        sum += ADC_ExcutSingleConver();
    }
    return (uint16_t)(sum / ADC_SAMPLES);
}

int main(void)
{
    uint16_t value;

    SetSysClock(CLK_SOURCE_PLL_60MHz);
    DebugInit();

    R8_TEM_SENSOR = 0;
    GPIOA_ModeCfg(ADC_PIN, GPIO_ModeIN_Floating);
    ADC_ExtSingleChSampInit(SampleFreq_3_2, ADC_PGA_0);
    ADC_ChannelCfg(0);

    PRINT("\r\n=== DEMO ADC ===\r\n");
    PRINT("PA4/AIN0, 16-sample average\r\n");

    while(1)
    {
        value = ReadAdcAverage();
        PRINT("ADC0=%d\r\n", value);
        DelayMs(1000);
    }
}
