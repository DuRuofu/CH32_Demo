/********************************** (C) COPYRIGHT *******************************
 * File Name          : Main.c
 * Description        : DataFlash read/write test
 *******************************************************************************/

#include "CH59x_common.h"

#define TEST_ADDR 0
#define TEST_SIZE 64

static uint8_t write_buf[TEST_SIZE];
static uint8_t read_buf[TEST_SIZE];

static void DebugInit(void)
{
    GPIOA_SetBits(GPIO_Pin_9);
    GPIOA_ModeCfg(GPIO_Pin_8, GPIO_ModeIN_PU);
    GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA);
    UART1_DefInit();
}

static uint8_t DataFlashTest(void)
{
    uint16_t i;
    uint32_t result;

    for(i = 0; i < TEST_SIZE; i++) write_buf[i] = (uint8_t)(0x30 + i);
    result = EEPROM_ERASE(TEST_ADDR, EEPROM_BLOCK_SIZE);
    if(result) return 1;
    result = EEPROM_WRITE(TEST_ADDR, write_buf, TEST_SIZE);
    if(result) return 2;
    EEPROM_READ(TEST_ADDR, read_buf, TEST_SIZE);
    for(i = 0; i < TEST_SIZE; i++)
    {
        if(read_buf[i] != write_buf[i]) return 3;
    }
    return 0;
}

int main(void)
{
    uint8_t uid[8];
    uint8_t mac[6];
    uint8_t result;
    uint8_t i;

    SetSysClock(CLK_SOURCE_PLL_60MHz);
    DebugInit();

    PRINT("\r\n=== DEMO DATAFLASH ===\r\n");
    GET_UNIQUE_ID(uid);
    GetMACAddress(mac);
    PRINT("UID:");
    for(i = 0; i < sizeof(uid); i++) PRINT(" %02X", uid[i]);
    PRINT("\r\nMAC:");
    for(i = 0; i < sizeof(mac); i++) PRINT(" %02X", mac[i]);
    PRINT("\r\n");

    result = DataFlashTest();
    PRINT("DataFlash test: %s", result ? "FAIL" : "PASS");
    if(result) PRINT(" (%d)", result);
    PRINT("\r\n");

    while(1) DelayMs(1000);
}
