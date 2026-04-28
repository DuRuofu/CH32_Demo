#include "LCD.h"

/*********************************************************************
 * @fn      LCD_Init
 *
 * @brief   Initialize LCD and ST7789 controller with hardware SPI2
 *
 * @return  none
 */
void LCD_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    /* 使能时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    /* 初始化控制引脚: DC, CS, RST, LED */
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Pin = ST7789_DC_PIN;
    GPIO_Init(ST7789_DC_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = ST7789_CS_PIN;
    GPIO_Init(ST7789_CS_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = ST7789_RST_PIN;
    GPIO_Init(ST7789_RST_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = ST7789_LED_PIN;
    GPIO_Init(ST7789_LED_PORT, &GPIO_InitStructure);

    /* 初始化 SPI2 */
    SPI2_Init();

    /* 取消选中 LCD */
    ST7789_CS_Set();

    /* RST 复位 */
    ST7789_RST_Set();
    Delay_Ms(50);
    ST7789_RST_Clr();
    Delay_Ms(100);
    ST7789_RST_Set();
    Delay_Ms(50);

    /* 点亮背光 */
    ST7789_LED_Set();

    /* 软件复位 ST7789 */
    ST7789_WriteCommand(ST7789_SWRESET);
    Delay_Ms(150);

    /* Gate Control */
    ST7789_WriteCommand(0xB7);
    ST7789_WriteData(0x35);

    /* VCOM setting */
    ST7789_WriteCommand(0xBB);
    ST7789_WriteData(0x19);

    /* LCMCTRL */
    ST7789_WriteCommand(0xC0);
    ST7789_WriteData(0x2C);

    /* VDV and VRH command Enable */
    ST7789_WriteCommand(0xC2);
    ST7789_WriteData(0x01);

    /* VRH set */
    ST7789_WriteCommand(0xC3);
    ST7789_WriteData(0x12);

    /* VDV set */
    ST7789_WriteCommand(0xC4);
    ST7789_WriteData(0x20);

    /* Frame rate control */
    ST7789_WriteCommand(0xC6);
    ST7789_WriteData(0x0F);

    /* Power control */
    ST7789_WriteCommand(0xD0);
    ST7789_WriteData(0xA4);
    ST7789_WriteData(0xA1);

    /* Gamma adjustment */
    ST7789_WriteCommand(0xE0);
    {
        uint8_t data[] = {0xD0, 0x04, 0x0D, 0x11, 0x13, 0x2B, 0x3F, 0x54, 0x4C, 0x18, 0x0D, 0x0B, 0x1F, 0x23};
        ST7789_WriteDataArr(data, sizeof(data));
    }

    ST7789_WriteCommand(0xE1);
    {
        uint8_t data[] = {0xD0, 0x04, 0x0C, 0x11, 0x13, 0x2C, 0x3F, 0x44, 0x51, 0x2F, 0x1F, 0x1F, 0x20, 0x23};
        ST7789_WriteDataArr(data, sizeof(data));
    }

    /* Inversion ON */
    ST7789_WriteCommand(ST7789_INVON);

    /* Exit sleep mode */
    ST7789_WriteCommand(ST7789_SLPOUT);
    Delay_Ms(120);

    /* Pixel Format Set - 16 bits per pixel */
    ST7789_WriteCommand(ST7789_COLMOD);
    ST7789_WriteData(0x55);

    /* Memory Data Access Control */
    ST7789_WriteCommand(ST7789_MADCTL);
    ST7789_WriteData(0x00);

    /* Normal Display on */
    ST7789_WriteCommand(ST7789_NORON);

    /* Main screen turned on */
    ST7789_WriteCommand(ST7789_DISPON);

    Delay_Ms(120);
}

/*********************************************************************
 * @fn      ST7789_WriteCommand
 *
 * @brief   Write command to ST7789
 *
 * @param   cmd - command byte
 *
 * @return  none
 */
void ST7789_WriteCommand(uint8_t cmd)
{
    ST7789_CS_Clr();
    ST7789_DC_Clr();
    SPI2_Write(cmd);
    ST7789_CS_Set();
}

/*********************************************************************
 * @fn      ST7789_WriteData
 *
 * @brief   Write single data byte to ST7789
 *
 * @param   data - data byte
 *
 * @return  none
 */
void ST7789_WriteData(uint8_t data)
{
    ST7789_CS_Clr();
    ST7789_DC_Set();
    SPI2_Write(data);
    ST7789_CS_Set();
}

/*********************************************************************
 * @fn      ST7789_WriteDataArr
 *
 * @brief   Write multiple data bytes to ST7789
 *
 * @param   buff - data buffer
 * @param   buff_size - number of bytes
 *
 * @return  none
 */
void ST7789_WriteDataArr(uint8_t *buff, size_t buff_size)
{
    ST7789_CS_Clr();
    ST7789_DC_Set();
    while(buff_size--)
    {
        SPI2_Write(*buff);
        buff++;
    }
    ST7789_CS_Set();
}

/*********************************************************************
 * @fn      ST7789_SetAddressWindow
 *
 * @brief   Set the address window for ST7789 display operations
 *
 * @param   x0, y0 - start coordinates
 * @param   x1, y1 - end coordinates
 *
 * @return  none
 */
void ST7789_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    uint16_t x_start = x0 + X_SHIFT;
    uint16_t x_end = x1 + X_SHIFT;
    uint16_t y_start = y0 + Y_SHIFT;
    uint16_t y_end = y1 + Y_SHIFT;

    /* Column Address Set */
    ST7789_WriteCommand(ST7789_CASET);
    ST7789_WriteData((x_start >> 8) & 0xFF);
    ST7789_WriteData(x_start & 0xFF);
    ST7789_WriteData((x_end >> 8) & 0xFF);
    ST7789_WriteData(x_end & 0xFF);

    /* Row Address Set */
    ST7789_WriteCommand(ST7789_RASET);
    ST7789_WriteData((y_start >> 8) & 0xFF);
    ST7789_WriteData(y_start & 0xFF);
    ST7789_WriteData((y_end >> 8) & 0xFF);
    ST7789_WriteData(y_end & 0xFF);

    /* Write to RAM */
    ST7789_WriteCommand(ST7789_RAMWR);
}

/*********************************************************************
 * @fn      LCD_Fill
 *
 * @brief   Fill the entire screen with a solid color
 *
 * @param   color - 16-bit color value (RGB565)
 *
 * @return  none
 */
void LCD_Fill(uint16_t color)
{
    uint32_t i;
    uint32_t total_pixels = ST7789_WIDTH * ST7789_HEIGHT;

    ST7789_SetAddressWindow(0, 0, ST7789_WIDTH - 1, ST7789_HEIGHT - 1);

    ST7789_CS_Clr();
    ST7789_DC_Set();

    uint8_t color_bytes[2] = {(color >> 8) & 0xFF, color & 0xFF};
    for(i = 0; i < total_pixels; i++)
    {
        SPI2_Write(color_bytes[0]);
        SPI2_Write(color_bytes[1]);
    }

    ST7789_CS_Set();
}

/*********************************************************************
 * @fn      ST7789_DrawPixel
 *
 * @brief   Draw a single pixel
 *
 * @param   x, y - coordinates
 * @param   color - 16-bit color value
 *
 * @return  none
 */
void ST7789_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
    if((x >= ST7789_WIDTH) || (y >= ST7789_HEIGHT))
        return;

    uint8_t color_bytes[2] = {(color >> 8) & 0xFF, color & 0xFF};

    ST7789_SetAddressWindow(x, y, x, y);

    ST7789_CS_Clr();
    ST7789_DC_Set();
    SPI2_Write(color_bytes[0]);
    SPI2_Write(color_bytes[1]);
    ST7789_CS_Set();
}

/*********************************************************************
 * @fn      ST7789_DrawLine
 *
 * @brief   Draw a line using Bresenham algorithm
 *
 * @param   x0, y0 - start coordinates
 * @param   x1, y1 - end coordinates
 * @param   color - 16-bit color value
 *
 * @return  none
 */
void ST7789_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
    int16_t dx, dy, err;
    int16_t steep;
    int16_t ystep;

    /* Check if line is vertical */
    if(x0 == x1)
    {
        if(y0 > y1)
        {
            uint16_t temp = y0;
            y0 = y1;
            y1 = temp;
        }
        ST7789_SetAddressWindow(x0, y0, x0, y1);
        ST7789_CS_Clr();
        ST7789_DC_Set();
        uint8_t color_bytes[2] = {(color >> 8) & 0xFF, color & 0xFF};
        while(y0 <= y1)
        {
            SPI2_Write(color_bytes[0]);
            SPI2_Write(color_bytes[1]);
            y0++;
        }
        ST7789_CS_Set();
        return;
    }

    /* Check if line is horizontal */
    if(y0 == y1)
    {
        if(x0 > x1)
        {
            uint16_t temp = x0;
            x0 = x1;
            x1 = temp;
        }
        ST7789_SetAddressWindow(x0, y0, x1, y0);
        ST7789_CS_Clr();
        ST7789_DC_Set();
        uint8_t color_bytes[2] = {(color >> 8) & 0xFF, color & 0xFF};
        while(x0 <= x1)
        {
            SPI2_Write(color_bytes[0]);
            SPI2_Write(color_bytes[1]);
            x0++;
        }
        ST7789_CS_Set();
        return;
    }

    /* General Bresenham algorithm */
    steep = abs(y1 - y0) > abs(x1 - x0);

    if(steep)
    {
        uint16_t temp = x0;
        x0 = y0;
        y0 = temp;
        temp = x1;
        x1 = y1;
        y1 = temp;
    }

    if(x0 > x1)
    {
        uint16_t temp = x0;
        x0 = x1;
        x1 = temp;
        temp = y0;
        y0 = y1;
        y1 = temp;
    }

    dx = x1 - x0;
    dy = abs(y1 - y0);
    err = dx / 2;

    if(y0 < y1)
        ystep = 1;
    else
        ystep = -1;

    while(x0 <= x1)
    {
        if(steep)
            ST7789_DrawPixel(y0, x0, color);
        else
            ST7789_DrawPixel(x0, y0, color);

        err -= dy;
        if(err < 0)
        {
            y0 += ystep;
            err += dx;
        }
        x0++;
    }
}

/*********************************************************************
 * @fn      ST7789_DrawRectangle
 *
 * @brief   Draw a rectangle outline
 *
 * @param   x1, y1 - top-left corner
 * @param   x2, y2 - bottom-right corner
 * @param   color - 16-bit color value
 *
 * @return  none
 */
void ST7789_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    /* Sort coordinates */
    if(x1 > x2)
    {
        uint16_t temp = x1;
        x1 = x2;
        x2 = temp;
    }
    if(y1 > y2)
    {
        uint16_t temp = y1;
        y1 = y2;
        y2 = temp;
    }

    ST7789_DrawLine(x1, y1, x2, y1, color);
    ST7789_DrawLine(x1, y2, x2, y2, color);
    ST7789_DrawLine(x1, y1, x1, y2, color);
    ST7789_DrawLine(x2, y1, x2, y2, color);
}

/*********************************************************************
 * @fn      ST7789_DrawImage
 *
 * @brief   Draw an image from data array
 *
 * @param   x, y - top-left position
 * @param   w, h - width and height
 * @param   data - image data array (RGB565)
 *
 * @return  none
 */
void ST7789_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *data)
{
    if((x >= ST7789_WIDTH) || (y >= ST7789_HEIGHT))
        return;
    if((x + w - 1) >= ST7789_WIDTH)
        return;
    if((y + h - 1) >= ST7789_HEIGHT)
        return;

    uint32_t total_pixels = w * h;
    uint8_t *p = (uint8_t *)data;

    ST7789_SetAddressWindow(x, y, x + w - 1, y + h - 1);

    ST7789_CS_Clr();
    ST7789_DC_Set();

    while(total_pixels--)
    {
        SPI2_Write(*(p+1));  // 低字节在前 (little-endian)
        SPI2_Write(*p);       // 高字节在后
        p += 2;
    }

    ST7789_CS_Set();
}
