/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2023/12/25
 * Description        : Main program body.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*
 * @Note
 * INA226 电压 / 电流 / 功率监测例程：
 * 本板 INA226 通过 I2C1（SCL=PC2 / SDA=PC1）与 MCU 通信，
 * 7-bit 地址 0x40（A1=A0=GND），分流电阻 RSHUNT = 1mΩ。
 *
 * 测量 VIN 输入：Bus Voltage（VIN 电压）、Shunt 电流、功率。
 * 系统时钟 SYSCLK = 8MHz（HSE），I2C 速率 400kHz。
 *
 * 串口（USART1，PD5=TX / PD6=RX，115200 8N1）每 500ms 输出一次：
 *   Vbus=xxxmV I=xxxmA P=xxxmW
 */

#include "debug.h"

/* INA226 寄存器地址 */
#define INA226_REG_CONFIG   0x00
#define INA226_REG_SHUNTV   0x01
#define INA226_REG_BUSV     0x02
#define INA226_REG_POWER    0x03
#define INA226_REG_CURRENT  0x04
#define INA226_REG_CALIB    0x05
#define INA226_REG_MASKEN   0x06
#define INA226_REG_ALERT    0x07
#define INA226_REG_MANUFID  0xFE
#define INA226_REG_DIEID    0xFF

/* INA226 I2C 地址：7-bit = 0x40，传给 I2C_Send7bitAddress 需左移 1 位 = 0x80 */
#define INA226_ADDR         0x80

/*
 * Configuration（字段布局：D15=RST, D14-12=保留, D11-9=AVG, D8-6=VBUSCT, D5-3=VSHCT, D2-0=MODE）
 *   RST=0, AVG=16(010), VBUSCT=140us(000), VSHCT=140us(000), MODE=Shunt+Bus 连续(111)
 *   = (2<<9)|(0<<6)|(0<<3)|7 = 0x0407
 */
#define INA226_CONFIG       0x0407

/*
 * 分流电阻 RSHUNT = 1mΩ，Current_LSB = 0.2mA
 * Calibration = 0.00512 / (0.0002 * 0.001) = 25600
 * 最大可测电流 = 0.2mA * 32767 ≈ 6.55A
 */
#define INA226_CALIB        25600

/* 制造商 ID，用于验证 I2C 通信 */
#define INA226_MANUF_ID     0x5449

/* Function Declaration */
void I2C_CFG(void);
void INA226_WriteReg(u8 reg, u16 val);
u16  INA226_ReadReg(u8 reg);
void USART1_CFG(void);
void UART_SendChar(u8 ch);
void UART_SendString(const char *s);
void UART_SendU32(u32 v);
void UART_SendS32(s32 v);

/*********************************************************************
 * @fn      I2C_CFG
 *
 * @brief   Initializes the I2C1 peripheral (SCL=PC2, SDA=PC1, 400kHz).
 *
 * @return  none
 */
void I2C_CFG(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    I2C_InitTypeDef I2C_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

    /* I2C1 SCL-->PC2   SDA-->PC1，复用开漏 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    I2C_InitStructure.I2C_ClockSpeed = 400000;
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = 0x00;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_Init(I2C1, &I2C_InitStructure);

    I2C_Cmd(I2C1, ENABLE);
}

/*********************************************************************
 * @fn      INA226_WriteReg
 *
 * @brief   Writes a 16-bit value to an INA226 register.
 *
 * @param   reg - register address.
 *          val - 16-bit value (MSB first).
 *
 * @return  none
 */
void INA226_WriteReg(u8 reg, u16 val)
{
    while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY) != RESET)
    {
    }

    I2C_GenerateSTART(I2C1, ENABLE);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))
    {
    }
    I2C_Send7bitAddress(I2C1, INA226_ADDR, I2C_Direction_Transmitter);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    {
    }

    I2C_SendData(I2C1, reg);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
    }

    I2C_SendData(I2C1, (u8)(val >> 8));
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
    }

    I2C_SendData(I2C1, (u8)(val & 0xFF));
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
    }

    I2C_GenerateSTOP(I2C1, ENABLE);
}

/*********************************************************************
 * @fn      INA226_ReadReg
 *
 * @brief   Reads a 16-bit value from an INA226 register.
 *
 * @param   reg - register address.
 *
 * @return  16-bit value (MSB first).
 */
u16 INA226_ReadReg(u8 reg)
{
    u8 msb = 0;
    u8 lsb = 0;

    while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY) != RESET)
    {
    }

    I2C_GenerateSTART(I2C1, ENABLE);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))
    {
    }
    I2C_Send7bitAddress(I2C1, INA226_ADDR, I2C_Direction_Transmitter);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    {
    }

    I2C_SendData(I2C1, reg);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
    }

    /* Repeated START + 读 */
    I2C_GenerateSTART(I2C1, ENABLE);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))
    {
    }
    I2C_Send7bitAddress(I2C1, INA226_ADDR, I2C_Direction_Receiver);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
    {
    }

    /* 读 MSB（ACK 保持，继续接收） */
    while(I2C_GetFlagStatus(I2C1, I2C_FLAG_RXNE) == RESET)
    {
    }
    msb = I2C_ReceiveData(I2C1);

    /* 读 LSB（先关 ACK，表示最后一字节） */
    I2C_AcknowledgeConfig(I2C1, DISABLE);
    while(I2C_GetFlagStatus(I2C1, I2C_FLAG_RXNE) == RESET)
    {
    }
    lsb = I2C_ReceiveData(I2C1);

    I2C_GenerateSTOP(I2C1, ENABLE);
    I2C_AcknowledgeConfig(I2C1, ENABLE);

    return ((u16)msb << 8) | lsb;
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
 * @fn      UART_SendS32
 *
 * @brief   Sends a signed 32-bit integer in decimal via USART1.
 *
 * @return  none
 */
void UART_SendS32(s32 v)
{
    if(v < 0)
    {
        UART_SendChar('-');
        v = -v;
    }
    UART_SendU32((u32)v);
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
    u16 id;

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();

    USART1_CFG();
    I2C_CFG();

    UART_SendString("\r\n=== 02_demo_ina226 ===\r\n");

    /* 读 Manufacturer ID 验证 I2C 通信 */
    id = INA226_ReadReg(INA226_REG_MANUFID);
    if(id == INA226_MANUF_ID)
    {
        UART_SendString("INA226 OK\r\n");
    }
    else
    {
        UART_SendString("INA226 FAIL, ID=0x");
        UART_SendU32(id);
        UART_SendString("\r\n");
    }

    /* 配置并启动连续转换 */
    INA226_WriteReg(INA226_REG_CONFIG, INA226_CONFIG);
    INA226_WriteReg(INA226_REG_CALIB, INA226_CALIB);

    while(1)
    {
        /* Bus Voltage：LSB = 1.25mV，mV = raw * 5 / 4 */
        u32 vbus_mv = (u32)INA226_ReadReg(INA226_REG_BUSV) * 5 / 4;

        /* Shunt Voltage：LSB = 2.5uV，有符号 */
        s32 shunt_uv = (s32)(s16)INA226_ReadReg(INA226_REG_SHUNTV) * 5 / 2;

        /* Current：Current_LSB = 0.2mA，有符号，mA = raw / 5 */
        s32 current_ma = (s32)(s16)INA226_ReadReg(INA226_REG_CURRENT) / 5;

        /* Power：Power_LSB = 25 * Current_LSB = 5mW，mW = raw * 5 */
        u32 power_mw = (u32)INA226_ReadReg(INA226_REG_POWER) * 5;

        UART_SendString("Vbus=");
        UART_SendU32(vbus_mv);
        UART_SendString("mV  Vshunt=");
        UART_SendS32(shunt_uv);
        UART_SendString("uV  I=");
        UART_SendS32(current_ma);
        UART_SendString("mA  P=");
        UART_SendU32(power_mw);
        UART_SendString("mW\r\n");

        Delay_Ms(500);
    }
}
