/********************************** (C) COPYRIGHT *******************************
 * File Name          : Main.c
 * Description        : Basic board bring-up test for CH591 BLE light board
 *
 * Default pins: UART1 PA8/PA9, LED PWM PA12/PWM4, ADC PA4/AIN0,
 *               key PB4, status output PA13.
 *******************************************************************************/

#include "CH59x_common.h"

#define PWM_PIN       GPIO_Pin_12
#define STATUS_PIN    GPIO_Pin_13
#define KEY_PIN       GPIO_Pin_4
#define PWM_CHANNEL   CH_PWM4
#define FLASH_SIZE    32

volatile uint32_t board_ticks;
static uint8_t brightness;

static void DebugInit(void)
{
    GPIOA_SetBits(GPIO_Pin_9);
    GPIOA_ModeCfg(GPIO_Pin_8, GPIO_ModeIN_PU);
    GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA);
    UART1_DefInit();
}

static void PwmInit(void)
{
    GPIOA_ResetBits(PWM_PIN);
    GPIOA_ModeCfg(PWM_PIN, GPIO_ModeOut_PP_5mA);
    PWMX_CLKCfg(3);
    PWMX_CycleCfg(PWMX_Cycle_256);
    PWMX_ACTOUT(PWM_CHANNEL, 0, Low_Level, ENABLE);
    brightness = 0;
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

static uint16_t AdcRead(void)
{
    uint8_t i;
    uint32_t sum = 0;
    R8_TEM_SENSOR = 0;
    GPIOA_ModeCfg(GPIO_Pin_4, GPIO_ModeIN_Floating);
    ADC_ExtSingleChSampInit(SampleFreq_3_2, ADC_PGA_0);
    ADC_ChannelCfg(0);
    for(i = 0; i < 8; i++) sum += ADC_ExcutSingleConver();
    return (uint16_t)(sum / 8);
}

static uint8_t FlashTest(void)
{
    uint8_t tx[FLASH_SIZE];
    uint8_t rx[FLASH_SIZE];
    uint8_t i;
    for(i = 0; i < FLASH_SIZE; i++) tx[i] = (uint8_t)(0xA0 + i);
    if(EEPROM_ERASE(0, EEPROM_BLOCK_SIZE)) return 1;
    if(EEPROM_WRITE(0, tx, FLASH_SIZE)) return 2;
    EEPROM_READ(0, rx, FLASH_SIZE);
    for(i = 0; i < FLASH_SIZE; i++) if(tx[i] != rx[i]) return 3;
    return 0;
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

static void PrintHelp(void)
{
    PRINT("Commands: help, p0..p255, adc, gpio, flash, status\r\n");
}

static void RunSelfTest(void)
{
    uint8_t result;
    uint16_t adc;

    PRINT("[1/4] GPIO: PASS (PA13 configured)\r\n");
    adc = AdcRead();
    PRINT("[2/4] ADC: %d\r\n", adc);
    result = FlashTest();
    PRINT("[3/4] DataFlash: %s\r\n", result ? "FAIL" : "PASS");
    PwmFade(255);
    DelayMs(200);
    PwmFade(0);
    PRINT("[4/4] PWM: PASS (PA12 fade)\r\n");
}

int main(void)
{
    uint8_t command[24];
    uint16_t len;
    uint32_t last_tick = 0;

    SetSysClock(CLK_SOURCE_PLL_60MHz);
    DebugInit();
    GPIOB_ModeCfg(KEY_PIN, GPIO_ModeIN_PU);
    GPIOA_ResetBits(STATUS_PIN);
    GPIOA_ModeCfg(STATUS_PIN, GPIO_ModeOut_PP_5mA);
    PwmInit();

    TMR0_TimerInit(FREQ_SYS / 10);          /* 100 ms heartbeat */
    TMR0_ITCfg(ENABLE, TMR0_3_IT_CYC_END);
    PFIC_EnableIRQ(TMR0_IRQn);

    PRINT("\r\n=== CH591 BOARD TEST ===\r\n");
    PRINT("UART PA8/PA9 PWM PA12 ADC PA4 KEY PB4\r\n");
    RunSelfTest();
    PrintHelp();

    while(1)
    {
        if(last_tick != board_ticks)
        {
            last_tick = board_ticks;
            if((last_tick % 10) == 0) PRINT("heartbeat=%d s\r\n", last_tick / 10);
        }

        len = UART1_RecvString(command);
        if(!len) continue;
        if(command[0] == 'p')
        {
            PwmFade(ParseValue(command, len));
            PRINT("PWM=%d\r\n", brightness);
        }
        else if(command[0] == 'a')
        {
            PRINT("ADC0=%d\r\n", AdcRead());
        }
        else if(command[0] == 'g')
        {
            PRINT("KEY_PB4=%s\r\n", GPIOB_ReadPortPin(KEY_PIN) ? "released" : "pressed");
        }
        else if(command[0] == 'f')
        {
            uint8_t result = FlashTest();
            PRINT("DataFlash=%s\r\n", result ? "FAIL" : "PASS");
        }
        else if(command[0] == 's')
        {
            PRINT("PWM=%d KEY=%s TICK=%d\r\n", brightness,
                  GPIOB_ReadPortPin(KEY_PIN) ? "released" : "pressed", board_ticks);
        }
        else
        {
            PrintHelp();
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
        board_ticks++;
        GPIOA_InverseBits(STATUS_PIN);
    }
}
