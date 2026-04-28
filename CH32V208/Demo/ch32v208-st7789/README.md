# CH32V208 ST7789 显示驱动

## 硬件连接 (SPI2)

| 信号      | GPIO   | 引脚  | 说明           |
|----------|--------|-------|--------------|
| SPI_SCK   | GPIOB  | PB13  | SPI时钟       |
| SPI_MOSI  | GPIOB  | PB15  | SPI数据输出     |
| SPI_MISO  | GPIOB  | PB14  | SPI数据输入(未用) |
| ST7789_CS | GPIOB  | PB12  | 片选           |
| ST7789_DC | GPIOB  | PB10  | 数据/命令选择    |
| ST7789_RST| GPIOB  | PB11  | 复位引脚       |
| ST7789_LED| GPIOB  | PB9   | 背光控制       |
| USART_TX  | GPIOA  | PA9   | 调试串口输出    |

## 核心驱动文件

| 文件      | 说明                    |
|----------|-----------------------|
| `LCD/spi.c` | 硬件 SPI2 驱动 |
| `LCD/LCD.c` | ST7789 显示驱动 |
| `LCD/LCD.h` | 驱动头文件及引脚定义 |

## 驱动核心要点

### 1. 字节序 (RGB565)

Image2Lcd 等取模软件默认输出 **大端序** (高字节在前)，但 ST7789 协议要求 **小端序** (低字节在前)。

错误：颜色会显示为乱码
```c
SPI2_Write(*p++);      // 大端序：先发高字节 ❌
SPI2_Write(*p++);      // 后发低字节
```

正确：
```c
SPI2_Write(*(p+1));    // 小端序：先发低字节 ✓
SPI2_Write(*p);        // 后发高字节
p += 2;
```

### 2. DC 和 CS 控制时序

ST7789 通过 DC 引脚区分命令和数据：
- DC=0：命令
- DC=1：数据

**重要**：每次 SPI 传输后必须等待 BSY 标志清除，否则下一字节会覆盖当前数据。

### 3. ST7789 初始化序列

ST7789 必须按特定顺序初始化：
1. 软件复位 (`SWRESET`) - 等待 150ms
2. 退出睡眠 (`SLPOUT`) - 等待 120ms
3. 设置像素格式 (`COLMOD` = 0x55 表示 16-bit)
4. 开启显示 (`DISPON`)

### 4. 地址窗口 (Column/Row Address Set)

每次写入像素前必须设置窗口：
```c
ST7789_WriteCommand(0x2A);  // CASET - 列地址
ST7789_WriteData(x_start);
ST7789_WriteData(x_end);

ST7789_WriteCommand(0x2B);  // RASET - 行地址
ST7789_WriteData(y_start);
ST7789_WriteData(y_end);

ST7789_WriteCommand(0x2C);  // RAMWR - 写入RAM
```

### 5. 图片数据格式

取模软件设置：
- **输出格式**: 16位真彩色
- **扫描模式**: 水平扫描
- **分辨率**: 240x240

生成的数组为 `const unsigned char []`，需要强转为 `(const uint16_t *)` 使用。

## 主要 API

```c
// 初始化
void LCD_Init(void);           // 初始化屏幕和SPI2

// 绘图
void LCD_Fill(uint16_t color);                              // 填充屏幕
void ST7789_DrawPixel(uint16_t x, uint16_t y, uint16_t color);  // 画点
void ST7789_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);  // 画线
void ST7789_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);  // 矩形
void ST7789_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *data);  // 显示图片
```

## 颜色定义 (RGB565)

```c
#define WHITE       0xFFFF
#define BLACK       0x0000
#define BLUE        0x001F
#define RED         0xF800
#define GREEN       0x07E0
#define YELLOW      0xFFE0
#define CYAN        0x7FFF
#define MAGENTA     0xF81F
```

RGB565 格式：
- 高5位：红色
- 中间6位：绿色
- 低5位：蓝色

## 常见问题

**Q: 屏幕不亮**
- 检查 LED (PB9) 是否输出高电平
- 检查 RST (PB11) 复位序列
- 检查 SPI 引脚连接是否正确

**Q: 颜色显示异常/乱码**
- 检查 RGB565 字节序是否正确（见上文第1点）
- 检查 COLMOD 是否设置为 0x55 (16-bit)

**Q: 图像位置偏移**
- 检查 MADCTL 寄存器设置（0x00 为默认）
- 检查 X_SHIFT/Y_SHIFT 偏移量设置

**Q: 刷屏很慢**
- 确认使用硬件 SPI 而非软件模拟
- 确认 SPI 波特率分频设置为 2 (最快)
