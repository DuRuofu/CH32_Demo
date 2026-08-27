# CH591 基础硬件测试工程

本目录用于 CH591 植物补光灯控制板的上电和外设验证。每个子目录都是一个独立的 MounRiver Studio 工程，公共启动文件、链接脚本和外设驱动位于 `SRC/`。

## 工程列表

| 工程 | 来源示例 | 测试内容 |
|---|---|---|
| `01-uart` | `examples/UART1` | UART1 收发和回显 |
| `02-gpio` | UART 工程骨架 | PB4 输入、PA13 输出和按键消抖 |
| `03-pwm` | `examples/PWMX` | PA12/PWM4 输出和 LED 渐变调光 |
| `04-adc` | `examples/ADC` | PA4/AIN0 多次采样平均 |
| `05-timer` | `examples/TMR` | TMR0 1 秒中断和 PA13 翻转 |
| `06-flash` | `examples/FLASH` | DataFlash 擦除、写入、读取和校验 |
| `00-board-test` | UART 工程骨架 | UART、GPIO、PWM、ADC、定时器和 DataFlash 综合测试 |

## 默认引脚

| 功能 | 引脚 | 备注 |
|---|---|---|
| UART1 RX | PA8 | 115200 8N1 |
| UART1 TX | PA9 | 调试输出 |
| LED PWM | PA12 / PWM4 | 0～255 占空比 |
| ADC 输入 | PA4 / AIN0 | 外部模拟电压输入 |
| GPIO 输入 | PB4 | 默认上拉，低电平表示按下 |
| GPIO/状态输出 | PA13 | LED 或示波器观察点 |

以上为当前板测默认分配，实际接线以硬件设计为准。

## 综合板测串口命令

`00-board-test` 启动后会自动运行一次 GPIO、ADC、DataFlash 和 PWM 自检，串口参数为 `115200 8N1`。

| 命令 | 作用 |
|---|---|
| `help` | 显示命令列表 |
| `p0`～`p255` | 设置 PWM 亮度 |
| `adc` | 读取 PA4/AIN0 |
| `gpio` | 读取 PB4 状态 |
| `flash` | 执行 DataFlash 读写校验 |
| `status` | 输出 PWM、按键和定时器状态 |

## 导入工程

使用 MounRiver Studio 导入对应子目录中的 `.wvproj` 文件。工程通过相对路径引用同级的 `../SRC/` 公共目录。首次构建会在工程目录下生成 `obj/` 输出目录。

## 注意

- `06-flash` 和综合板测只操作 DataFlash 区域，不擦写存放程序的 CodeFlash 区域。
- `PA4` 接入外部电压时必须满足 CH591 ADC 输入范围。
- `PA12` 应连接 LED 驱动芯片的 PWM/调光输入，不要直接连接大功率 LED 负载。
