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
 * INA226 读取 + 电量积分例程（04_demo_power_integral）：
 * 本板 INA226 通过 I2C1（SCL=PC2 / SDA=PC1）与 MCU 通信，
 * 7-bit 地址 0x40（A1=A0=GND），分流电阻 RSHUNT = 1mΩ。
 *
 * 每 100ms 读取一次瞬时电流 I（mA），并对时间积分累加消耗电量：
 *   累计电量（mAh）= Σ I(mA) × Δt(h)
 * 内部使用“0.1 mAs”为单位累加，避免浮点：每个 100ms 采样，累计值 += I。
 *   mAs  = 累计值 / 10
 *   mAh  = 累计值 / 36000
 *
 * 串口（USART1，PD5=TX / PD6=RX，115200 8N1）每 1000ms 输出一次：
 *   Vbus=xxxmV I=xxxmA mAh=xxx.x
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

/* 采样/积分周期（ms）：每周期读取一次电流并累加一次 */
#define SAMPLE_PERIOD_MS    100

/* 打印周期（ms） */
#define PRINT_PERIOD_MS     1000

/* ------------------------------------------------------------------ *
 * 校准参数 Flash 存储
 * 使用末尾 1KB 预留页（链接脚本已将 FLASH 缩短 1K，物理地址 0x08003C00）。
 * 结构体（4 × u32，共 16 字节）：
 *   +0  magic    固定魔数，用于校验存在性
 *   +4  offset_ma 电流零偏（mA，有符号，负值表示读数偏大需减去）
 *   +8  gain_q16  增益定点，1.0 = 65536（修正分流电阻比例误差）
 *   +12 crc32     对前 12 字节的 CRC32，用于校验数据完整性
 * ------------------------------------------------------------------ */
#define CALIB_FLASH_ADDR       ((uint32_t)0x08003C00)
#define CALIB_MAGIC            ((uint32_t)0x43414C31)   /* "CAL1" */

#define CALIB_GAIN_ONE         65536u                   /* gain_q16 = 1.0 */

typedef struct
{
    s32 offset_ma;      /* 电流零偏（mA） */
    u32 gain_q16;       /* 增益定点（1.0 = 65536） */
} calib_t;

/* Global Variable */
calib_t g_calib;        /* 当前生效的校准参数 */
u8  g_rxbuf[16];
u8  g_rxlen = 0;

/* Function Declaration */
void I2C_CFG(void);
void INA226_WriteReg(u8 reg, u16 val);
u16  INA226_ReadReg(u8 reg);
void USART1_CFG(void);
void UART_SendChar(u8 ch);
void UART_SendString(const char *s);
void UART_SendU32(u32 v);
void UART_SendS32(s32 v);
void CALIB_Load(void);
void CALIB_Save(s32 offset_ma, u32 gain_q16);
s32  CALIB_ReadCurrent_ma(void);
void Cmd_Process(void);

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
 * @fn      CALIB_CRC32
 *
 * @brief   Computes CRC32 (IEEE 802.3, reflected) over a 32-bit array.
 *
 * @param   buf - pointer to u32 words.
 *          words - number of 32-bit words.
 *
 * @return  CRC32 value.
 */
static u32 CALIB_CRC32(const u32 *buf, u32 words)
{
    u32 crc = 0xFFFFFFFF;
    u32 i, b;

    for(i = 0; i < words; i++)
    {
        crc ^= buf[i];
        for(b = 0; b < 32; b++)
        {
            if(crc & 0x80000000)
            {
                crc = (crc << 1) ^ 0x04C11DB7;
            }
            else
            {
                crc <<= 1;
            }
        }
    }
    return crc;
}

/*********************************************************************
 * @fn      CALIB_Load
 *
 * @brief   Loads calibration from flash. Falls back to defaults if
 *          magic/CRC invalid or flash erased.
 *
 * @return  none
 */
void CALIB_Load(void)
{
    u32 w0, w1, w2, w3;
    u32 words[3];

    g_calib.offset_ma = 0;
    g_calib.gain_q16  = CALIB_GAIN_ONE;

    w0 = *(volatile u32 *)CALIB_FLASH_ADDR;
    w1 = *(volatile u32 *)(CALIB_FLASH_ADDR + 4);
    w2 = *(volatile u32 *)(CALIB_FLASH_ADDR + 8);
    w3 = *(volatile u32 *)(CALIB_FLASH_ADDR + 12);

    if(w0 != CALIB_MAGIC)
    {
        return;
    }

    words[0] = w0;
    words[1] = w1;
    words[2] = w2;
    if(w3 != CALIB_CRC32(words, 3))
    {
        return;
    }

    g_calib.offset_ma = (s32)w1;
    g_calib.gain_q16  = w2;
}

/*********************************************************************
 * @fn      CALIB_Save
 *
 * @brief   Writes calibration to flash (erase 1KB page then program).
 *
 * @param   offset_ma - current zero offset in mA.
 *          gain_q16  - gain as Q16 fixed point (1.0 = 65536).
 *
 * @return  none
 */
void CALIB_Save(s32 offset_ma, u32 gain_q16)
{
    u32 words[4];

    words[0] = CALIB_MAGIC;
    words[1] = (u32)offset_ma;
    words[2] = gain_q16;
    words[3] = CALIB_CRC32(words, 3);

    FLASH_Unlock();
    FLASH_ErasePage(CALIB_FLASH_ADDR);
    FLASH_ProgramWord(CALIB_FLASH_ADDR + 0,  words[0]);
    FLASH_ProgramWord(CALIB_FLASH_ADDR + 4,  words[1]);
    FLASH_ProgramWord(CALIB_FLASH_ADDR + 8,  words[2]);
    FLASH_ProgramWord(CALIB_FLASH_ADDR + 12, words[3]);
    FLASH_Lock();

    g_calib.offset_ma = offset_ma;
    g_calib.gain_q16  = gain_q16;
}

/*********************************************************************
 * @fn      CALIB_ReadCurrent_ma
 *
 * @brief   Reads the raw current and applies calibration:
 *          I_corr = (I_raw - offset) * gain
 *
 * @return  calibrated current in mA (signed).
 */
s32 CALIB_ReadCurrent_ma(void)
{
    s32 raw = (s32)(s16)INA226_ReadReg(INA226_REG_CURRENT) / 5;
    s32 corr = (s32)(((int64_t)(raw - g_calib.offset_ma) * (int64_t)g_calib.gain_q16) >> 16);
    return corr;
}

/*********************************************************************
 * @fn      Cmd_Process
 *
 * @brief   Parses a received line and executes the calibration command.
 *
 *          Z         零点校准：空载时多次采样取平均，作为零偏并写入 Flash
 *          O<offset> 手动设置零偏并写入 Flash，例如 O-10（-10mA）
 *          G<gain>   设置增益（单位 0.001，1.000=默认）并写入 Flash，例如 G1000
 *          R         恢复默认校准并写入 Flash
 *
 * @return  none
 */
void Cmd_Process(void)
{
    u8  cmd = g_rxbuf[0];
    u32 val = 0;
    u8  i;
    u8  neg = 0;

    if(cmd == 'Z' || cmd == 'z')
    {
        s32 sum = 0;
        u32 n;
        for(n = 0; n < 50; n++)
        {
            sum += (s32)(s16)INA226_ReadReg(INA226_REG_CURRENT) / 5;
            Delay_Ms(10);
        }
        CALIB_Save(-(sum / 50), CALIB_GAIN_ONE);
        UART_SendString("ZeroCal offset=");
        UART_SendS32(g_calib.offset_ma);
        UART_SendString("mA\r\n");
        return;
    }

    if(cmd == 'O' || cmd == 'o')
    {
        u8 c = g_rxbuf[1];
        if(c == '-')
        {
            neg = 1;
            i = 2;
        }
        else
        {
            i = 1;
        }
        for(; i < g_rxlen; i++)
        {
            c = g_rxbuf[i];
            if(c >= '0' && c <= '9')
            {
                val = val * 10 + (c - '0');
            }
            else
            {
                return;
            }
        }
        CALIB_Save(neg ? -(s32)val : (s32)val, CALIB_GAIN_ONE);
        UART_SendString("Offset=");
        UART_SendS32(g_calib.offset_ma);
        UART_SendString("mA\r\n");
        return;
    }

    if(cmd == 'G' || cmd == 'g')
    {
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
        if(val == 0)
        {
            val = 1000;
        }
        CALIB_Save(g_calib.offset_ma, (u32)((uint64_t)val * 65536 / 1000));
        UART_SendString("Gain=");
        UART_SendU32(val);
        UART_SendString("/1000\r\n");
        return;
    }

    if(cmd == 'R' || cmd == 'r')
    {
        CALIB_Save(0, CALIB_GAIN_ONE);
        UART_SendString("Calib reset\r\n");
        return;
    }

    UART_SendString("unknown cmd\r\n");
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

    /* 电量累加器：单位为 0.1 mAs（0.1 mA·s）。
       每 100ms 采样一次，若电流为 I mA，则累加值 += I。 */
    int64_t total_units = 0;    /* 1 unit = 0.1 mAs */

    u32 sample_cnt = 0;     /* 累计采样次数 */
    int64_t mAh_x10 = 0;    /* mAh * 10（带一位小数） */

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();

    USART1_CFG();
    I2C_CFG();

    UART_SendString("\r\n=== 04_demo_power_integral ===\r\n");

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

    /* 从 Flash 加载校准参数（掉电保持） */
    CALIB_Load();
    UART_SendString("Calib: offset=");
    UART_SendS32(g_calib.offset_ma);
    UART_SendString("mA gain=");
    UART_SendU32(g_calib.gain_q16);
    UART_SendString("\r\n");
    UART_SendString("Cmd: Z=zero-cal  O<off>=set offset  G<gain>=set gain(/1000)  R=reset\r\n");

    while(1)
    {
        u32 vbus_mv;
        s32 current_ma;

        /* Bus Voltage：LSB = 1.25mV，mV = raw * 5 / 4 */
        vbus_mv = (u32)INA226_ReadReg(INA226_REG_BUSV) * 5 / 4;

        /* Current：读取并应用校准（偏置 + 增益） */
        current_ma = CALIB_ReadCurrent_ma();

        /* 电量积分：current(mA) × 0.1s = 0.1 mA·s = 1 unit */
        total_units += (int64_t)current_ma;
        sample_cnt++;

        /* 串口命令处理（回车/换行触发） */
        if(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) != RESET)
        {
            u8 ch = USART_ReceiveData(USART1);
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

        /* 每 PRINT_PERIOD_MS 打印一次（每周期采样次数 = 周期/采样周期） */
        if((sample_cnt % (PRINT_PERIOD_MS / SAMPLE_PERIOD_MS)) == 0)
        {
            /* mAh = mAs / 3600 = (total_units/10) / 3600 = total_units / 36000。
               mAh*10 = total_units / 3600，保留一位小数。 */
            mAh_x10 = total_units / 3600;

            UART_SendString("Vbus=");
            UART_SendU32(vbus_mv);
            UART_SendString("mV  I=");
            UART_SendS32(current_ma);
            UART_SendString("mA  mAh=");
            if(mAh_x10 < 0)
            {
                UART_SendChar('-');
                mAh_x10 = -mAh_x10;
            }
            UART_SendU32((u32)(mAh_x10 / 10));
            UART_SendChar('.');
            UART_SendU32((u32)(mAh_x10 % 10));
            UART_SendString("\r\n");
        }

        Delay_Ms(SAMPLE_PERIOD_MS);
    }
}
