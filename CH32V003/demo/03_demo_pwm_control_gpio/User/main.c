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
 * PWM 脉宽检测 + 外部控制输出例程（03_demo_pwm_control_gpio）：
 * 本板外置控制信号输入引脚为 PC3（TIM1_CH3，复用输入捕获），
 * 控制输出引脚暂定为 PD0（普通 GPIO，控制外部 MOSFET 通断）。
 *
 * TIM1 计数时钟：SYSCLK = 8MHz（HSE），预分频 PSC=7，计数器每 1us 递增 1，
 * 因此捕获到的计数差值即脉宽（单位 us），周期 ARR = 0xFFFF ≈ 65.5ms。
 *
 * 通过 CC3（上升沿）与 CC4（下降沿，间接选择 TI3）同时捕获 PC3 上的 PWM：
 *   脉宽 us = CC4 捕获值 - CC3 捕获值
 *
 * 控制逻辑（默认阈值 1500us，可改 PWM_THRESHOLD_US）：
 *   脉宽 > 1500us  -> PD0 输出高电平（切断外部 MOSFET 供电）
 *   脉宽 <= 1500us -> PD0 输出低电平（恢复供电）
 *
 * 串口（USART1，PD5=TX / PD6=RX，115200 8N1）每 500ms 打印一次：
 *   Pulse=xxxxus  CtrlGPIO=0/1
 */

#include "debug.h"

/* 脉宽阈值（us）：脉宽大于该值则切断供电 */
#define PWM_THRESHOLD_US    1500

/* 阈值两侧 ±100us 迟滞，避免临界处抖动反复切换 */
#define PWM_HYSTERESIS_US   100

/* 控制输出引脚：PD0（普通 GPIO，控制 MOSFET） */
#define CTRL_GPIO_PORT      GPIOD
#define CTRL_GPIO_PIN       GPIO_Pin_0

/* 电平定义：OFF = 切断供电，ON = 恢复供电 */
#define CTRL_OFF_LEVEL      1
#define CTRL_ON_LEVEL       0

/* Global Variable */
volatile u32 g_pulse_us = 0;        /* 最近一次测量的脉宽（us），由中断更新 */
volatile u32 g_uptick = 0;          /* TIM1 更新中断计数（约 65.5ms 一次） */
u8 g_ctrl_level = CTRL_ON_LEVEL;    /* 当前控制输出电平 */

/* Function Declaration */
void TIM1_InputCapture_CFG(void);
void CTRL_GPIO_CFG(void);
void CTRL_GPIO_Set(u8 level);
void USART1_CFG(void);
void UART_SendChar(u8 ch);
void UART_SendString(const char *s);
void UART_SendU32(u32 v);

/*********************************************************************
 * @fn      TIM1_InputCapture_CFG
 *
 * @brief   Configures TIM1_CH3 (PC3) input capture to measure PWM pulse
 *          width. CC3 captures rising edge, CC4 captures falling edge.
 *
 * @return  none
 */
void TIM1_InputCapture_CFG(void)
{
    GPIO_InitTypeDef        GPIO_InitStructure = {0};
    TIM_ICInitTypeDef       TIM_ICInitStructure = {0};
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure = {0};
    NVIC_InitTypeDef        NVIC_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_TIM1, ENABLE);

    /* PC3 -> TIM1_CH3 输入，浮空 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* 时基：PSC=7，8MHz/8 = 1MHz，每 tick 1us，ARR=0xFFFF（约 65.5ms） */
    TIM_TimeBaseInitStructure.TIM_Period = 0xFFFF;
    TIM_TimeBaseInitStructure.TIM_Prescaler = 7;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStructure);

    /* CC3：上升沿 + 直接选择 TI3（PC3） */
    TIM_ICInitStructure.TIM_Channel = TIM_Channel_3;
    TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
    TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    TIM_ICInitStructure.TIM_ICFilter = 0x00;
    TIM_ICInit(TIM1, &TIM_ICInitStructure);

    /* CC4：下降沿 + 间接选择 TI3（与 CC3 共用 PC3 输入，测量高电平脉宽） */
    TIM_ICInitStructure.TIM_Channel = TIM_Channel_4;
    TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;
    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_IndirectTI;
    TIM_ICInit(TIM1, &TIM_ICInitStructure);

    /* CC3、CC4 中断 */
    NVIC_InitStructure.NVIC_IRQChannel = TIM1_CC_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    TIM_ITConfig(TIM1, TIM_IT_CC3 | TIM_IT_CC4 | TIM_IT_Update, ENABLE);

    TIM_Cmd(TIM1, ENABLE);
}

/*********************************************************************
 * @fn      CTRL_GPIO_CFG
 *
 * @brief   Configures the control output GPIO (PD0) as push-pull output.
 *
 * @return  none
 */
void CTRL_GPIO_CFG(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);

    GPIO_InitStructure.GPIO_Pin = CTRL_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_Init(CTRL_GPIO_PORT, &GPIO_InitStructure);

    CTRL_GPIO_Set(CTRL_ON_LEVEL);
}

/*********************************************************************
 * @fn      CTRL_GPIO_Set
 *
 * @brief   Sets the control output GPIO to a given level.
 *
 * @param   level - CTRL_ON_LEVEL (0) or CTRL_OFF_LEVEL (1).
 *
 * @return  none
 */
void CTRL_GPIO_Set(u8 level)
{
    if(level == CTRL_OFF_LEVEL)
    {
        GPIO_SetBits(CTRL_GPIO_PORT, CTRL_GPIO_PIN);
    }
    else
    {
        GPIO_ResetBits(CTRL_GPIO_PORT, CTRL_GPIO_PIN);
    }
    g_ctrl_level = level;
}

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
 * @fn      TIM1_CC_IRQHandler
 *
 * @brief   Handles TIM1 capture compare interrupt: measures PWM pulse
 *          width as the difference between falling (CC4) and rising (CC3).
 *
 * @return  none
 */
void TIM1_CC_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

void TIM1_CC_IRQHandler(void)
{
    static u32 rising = 0;
    u32 falling;

    if(TIM_GetITStatus(TIM1, TIM_IT_CC3) != RESET)
    {
        rising = TIM_GetCapture3(TIM1);
        TIM_ClearITPendingBit(TIM1, TIM_IT_CC3);
    }

    if(TIM_GetITStatus(TIM1, TIM_IT_CC4) != RESET)
    {
        falling = TIM_GetCapture4(TIM1);
        if(falling >= rising)
        {
            g_pulse_us = falling - rising;
        }
        TIM_ClearITPendingBit(TIM1, TIM_IT_CC4);
    }

    if(TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET)
    {
        g_uptick++;
        TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
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
    u32 last_tick = 0;

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();

    USART1_CFG();
    CTRL_GPIO_CFG();
    TIM1_InputCapture_CFG();

    UART_SendString("\r\n=== 03_demo_pwm_control_gpio ===\r\n");
    UART_SendString("Pulse>");
    UART_SendU32(PWM_THRESHOLD_US);
    UART_SendString("us -> CtrlGPIO OFF\r\n");

    while(1)
    {
        u32 pulse = g_pulse_us;

        /* 控制逻辑：脉宽超过阈值（含迟滞）切断供电，否则恢复供电 */
        if(pulse > (PWM_THRESHOLD_US + PWM_HYSTERESIS_US))
        {
            CTRL_GPIO_Set(CTRL_OFF_LEVEL);
        }
        else if(pulse < (PWM_THRESHOLD_US - PWM_HYSTERESIS_US))
        {
            CTRL_GPIO_Set(CTRL_ON_LEVEL);
        }

        /* 每约 8 个更新中断（≈ 500ms）打印一次 */
        if((g_uptick - last_tick) >= 8)
        {
            last_tick = g_uptick;
            UART_SendString("Pulse=");
            UART_SendU32(pulse);
            UART_SendString("us  CtrlGPIO=");
            UART_SendU32(g_ctrl_level);
            UART_SendString("\r\n");
        }
    }
}
