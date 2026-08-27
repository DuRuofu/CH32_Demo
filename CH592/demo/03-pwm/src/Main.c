/********************************** (C) COPYRIGHT *******************************
 * File Name          : Main.c
 * Description        : PWM4 LED dimming test
 *******************************************************************************/

#include "CH59x_common.h"

#define PWM_PIN     GPIO_Pin_12
#define PWM_CHANNEL CH_PWM4

static uint8_t brightness;

static void DebugInit(void)
{
    GPIOA_SetBits(GPIO_Pin_9);
    GPIOA_ModeCfg(GPIO_Pin_8, GPIO_ModeIN_PU);
    GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA);
    UART1_DefInit();
}

static void PwmSet(uint8_t value)
{
    brightness = value;
    PWMX_ACTOUT(PWM_CHANNEL, value, Low_Level, ENABLE);
}

static void PwmFade(uint8_t target)
{
    while(brightness != target)
    {
        PwmSet((brightness < target) ? (brightness + 1) : (brightness - 1));
        DelayMs(5);
    }
}

static uint8_t ParseValue(uint8_t *buf, uint16_t len)
{
    uint16_t i;
    uint16_t value = 0;
    for(i = 1; i < len; i++)
    {
        if(buf[i] >= '0' && buf[i] <= '9')
        {
            value = value * 10 + (buf[i] - '0');
            if(value > 255) return 255;
        }
    }
    return (uint8_t)value;
}

int main(void)
{
    uint8_t command[20];
    uint16_t len;

    SetSysClock(CLK_SOURCE_PLL_60MHz);
    DebugInit();

    GPIOA_ResetBits(PWM_PIN);
    GPIOA_ModeCfg(PWM_PIN, GPIO_ModeOut_PP_5mA);
    PWMX_CLKCfg(3);                     /* 60 MHz / 3 / 256 ~= 78 kHz */
    PWMX_CycleCfg(PWMX_Cycle_256);      /* 8-bit duty resolution */
    PwmSet(0);

    PRINT("\r\n=== DEMO PWM ===\r\n");
    PRINT("PA12/PWM4, duty 0..255\r\n");
    PRINT("Commands: p0..p255, on, off, status\r\n");

    PwmFade(255);
    DelayMs(300);
    PwmFade(0);

    while(1)
    {
        len = UART1_RecvString(command);
        if(!len) continue;

        if(command[0] == 'p')
        {
            PwmFade(ParseValue(command, len));
            PRINT("PWM=%d\r\n", brightness);
        }
        else if(command[0] == 'o' && len > 1 && command[1] == 'n')
        {
            PwmFade(255);
            PRINT("PWM=255\r\n");
        }
        else if(command[0] == 'o')
        {
            PwmFade(0);
            PRINT("PWM=0\r\n");
        }
        else if(command[0] == 's')
        {
            PRINT("PWM=%d\r\n", brightness);
        }
        else
        {
            PRINT("Unknown command\r\n");
        }
    }
}
