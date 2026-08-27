/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2024/11/28
 * Description        : Main program body.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*
 * @Note
 * PWM 输出例程（串口参数可调）：
 * 本板 PWM 输出引脚为 PC3（TIM1_CH3），经 H2 接口引出。
 * 系统时钟 SYSCLK = 8MHz（HSE），TIM1 计数时钟 = 8MHz。
 *
 * 串口（USART1，PD5=TX / PD6=RX，115200 8N1）接收命令：
 *   F<freq>  设置 PWM 频率，单位 Hz，例如 F1000
 *   D<duty>  设置占空比，单位 0.1%（0~1000），例如 D500 表示 50%
 *
 * PWM 频率 = 8000000 / ((psc+1) * (arr+1))，其中 arr 固定 999（占空比分辨率 0.1%）。
 */

#include "debug.h"

/* 周期寄存器，占空比分辨率 = 1/(ARR+1) = 0.1% */
#define PWM_ARR            999
#define PWM_DEFAULT_FREQ   1000
#define PWM_DEFAULT_DUTY   500

/* Global Variable */
u32 g_freq = PWM_DEFAULT_FREQ;
u16 g_duty = PWM_DEFAULT_DUTY;

u8  g_rxbuf[16];
u8  g_rxlen = 0;

/* Function Declaration */
void USART1_CFG(void);
void PWM_Set(u32 freq, u16 duty);
void UART_SendChar(u8 ch);
void UART_SendString(const char *s);
void UART_SendU32(u32 v);
void Cmd_Process(void);

/*********************************************************************
 * @fn      USART1_CFG
 *
 * @brief   Initializes the USART1 peripheral (PD5=TX, PD6=RX).
 *
 * @return  none
 */
void USART1_CFG(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure = {0};
    USART_InitTypeDef USART_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_USART1, ENABLE);

    /* USART1 TX-->PD5   RX-->PD6 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;

    USART_Init(USART1, &USART_InitStructure);
    USART_Cmd(USART1, ENABLE);
}

/*********************************************************************
 * @fn      PWM_Set
 *
 * @brief   Configures TIM1_CH3 (PC3) PWM output with given frequency and duty.
 *
 * @param   freq - PWM frequency in Hz (1 ~ 8000).
 *          duty - duty cycle in 0.1% (0 ~ 1000, 1000 = 100%).
 *
 * @return  none
 */
void PWM_Set(u32 freq, u16 duty)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    TIM_OCInitTypeDef TIM_OCInitStructure = {0};
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure = {0};
    u32 psc;
    u16 ccp;

    if(freq == 0)
    {
        freq = 1;
    }
    if(freq > 8000)
    {
        freq = 8000;
    }

    /* psc = TIMCLK / (freq * (ARR+1)) - 1 */
    psc = SystemCoreClock / (freq * (PWM_ARR + 1));
    if(psc < 1)
    {
        psc = 1;
    }
    psc -= 1;
    if(psc > 65535)
    {
        psc = 65535;
    }

    /* duty: 0~1000, 占空比 = ccp/(ARR+1) */
    ccp = duty;
    if(ccp > (PWM_ARR + 1))
    {
        ccp = PWM_ARR + 1;
    }

    /* PC3 -> TIM1_CH3 复用推挽输出 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_TIM1, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    TIM_Cmd(TIM1, DISABLE);

    TIM_TimeBaseInitStructure.TIM_Period = PWM_ARR;
    TIM_TimeBaseInitStructure.TIM_Prescaler = psc;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStructure);

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = ccp;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC3Init(TIM1, &TIM_OCInitStructure);

    TIM_CtrlPWMOutputs(TIM1, ENABLE);
    TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Disable);
    TIM_ARRPreloadConfig(TIM1, ENABLE);
    TIM_Cmd(TIM1, ENABLE);

    g_freq = freq;
    g_duty = ccp;
}

/*********************************************************************
 * @fn      UART_SendChar
 *
 * @brief   Sends one byte via USART1 (blocking).
 *
 * @return  none
 */
void UART_SendChar(u8 ch)
{
    while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
    {
    }
    USART_SendData(USART1, ch);
}

/*********************************************************************
 * @fn      UART_SendString
 *
 * @brief   Sends a null-terminated string via USART1.
 *
 * @return  none
 */
void UART_SendString(const char *s)
{
    while(*s)
    {
        UART_SendChar((u8)*s++);
    }
}

/*********************************************************************
 * @fn      UART_SendU32
 *
 * @brief   Sends an unsigned 32-bit integer in decimal via USART1.
 *
 * @return  none
 */
void UART_SendU32(u32 v)
{
    char buf[12];
    int i = 0;

    if(v == 0)
    {
        UART_SendChar('0');
        return;
    }

    while(v > 0)
    {
        buf[i++] = (char)('0' + (v % 10));
        v /= 10;
    }
    while(i > 0)
    {
        UART_SendChar((u8)buf[--i]);
    }
}

/*********************************************************************
 * @fn      Cmd_Process
 *
 * @brief   Parses a received line and executes the command.
 *
 * @return  none
 */
void Cmd_Process(void)
{
    u32 val = 0;
    u8  i;
    u8  cmd = g_rxbuf[0];

    for(i = 1; i < g_rxlen; i++)
    {
        u8 c = g_rxbuf[i];
        if(c >= '0' && c <= '9')
        {
            val = val * 10 + (c - '0');
        }
        else
        {
            return;
        }
    }

    if(cmd == 'F' || cmd == 'f')
    {
        PWM_Set(val, g_duty);
        UART_SendString("Freq=");
        UART_SendU32(g_freq);
        UART_SendString("Hz, Duty=");
        UART_SendU32(g_duty);
        UART_SendString("/1000\r\n");
    }
    else if(cmd == 'D' || cmd == 'd')
    {
        if(val > 1000)
        {
            val = 1000;
        }
        PWM_Set(g_freq, (u16)val);
        UART_SendString("Freq=");
        UART_SendU32(g_freq);
        UART_SendString("Hz, Duty=");
        UART_SendU32(g_duty);
        UART_SendString("/1000\r\n");
    }
    else
    {
        UART_SendString("unknown cmd\r\n");
    }
}

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(void)
{
    u8 ch;

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();

    USART1_CFG();
    PWM_Set(g_freq, g_duty);

    UART_SendString("\r\n=== 01_demo_pwm ===\r\n");
    UART_SendString("F<freq> set frequency (Hz)\r\n");
    UART_SendString("D<duty> set duty (0-1000, 0.1%)\r\n");

    while(1)
    {
        if(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) != RESET)
        {
            ch = USART_ReceiveData(USART1);
            if(ch == '\r' || ch == '\n')
            {
                if(g_rxlen > 0)
                {
                    Cmd_Process();
                }
                g_rxlen = 0;
            }
            else if(g_rxlen < (sizeof(g_rxbuf) - 1))
            {
                g_rxbuf[g_rxlen++] = ch;
            }
        }
    }
}
