# 软件示例工程

本目录包含基于 **CH32V003F4P6** 的示例工程（demo），每个 demo 针对板卡的一项外设/功能。

## 1. 目录结构

```text
demo/
├── SRC/               # 共享外设库（Core / Peripheral / Startup / Ld / Debug）
├── 00_demo_uart/      # 串口回环
├── 01_demo_pwm/       # PWM 输出（串口调参）
├── 02_demo_ina226/    # INA226 电压/电流/功率监测
├── 03_demo_pwm_control_gpio/  # PWM 脉宽检测 + 外部控制 GPIO（切断 MOSFET）
├── 04_demo_power_integral/    # INA226 读取 + 电流积分计算电量（mAh）
└── README.md
```

每个 `NN_demo_xxx/` 都是一个独立的 MounRiver Studio 工程，通过链接资源引用同目录下的 `SRC` 共享库。
> 完整功能固件见 `../firmware/`（含 MSP 通信与校准），上位机见 `../host/`。

## 2. 编译与烧录

- 使用 **MounRiver Studio** 打开对应 demo 目录，编译后产物在工程下的 `obj/`。
- 烧录使用 **WCH-Link**。
- 注意：CH32V003 为单线调试（SWIO，数据脚 PD1）。本板 H3 接口丝印为 SWCLK/SWDIO，其中
  **烧录器的 SWDIO 需接到 H3 的 SWCLK 引脚**（H3 的 SWCLK 实际连接 PD1/SWIO）。
- 各 demo 均使用 **USART1**（PD5=TX，PD6=RX，115200 8N1），对应板卡 **H1** 接口（3V3/TX/RX/GND）。

## 3. 各 demo 说明

### 00_demo_uart —— 串口回环

- **作用**：验证 UART 通信。收到一个字节后原样回发。
- **使用**：串口助手连接 H1，115200 8N1，发送任意数据，原样返回。
- **注意事项**：无。

### 01_demo_pwm —— PWM 输出（串口调参）

- **作用**：在 **PC3（TIM1_CH3）** 输出 PWM 波形，经 **H2**（PWM/GND）引出，频率/占空比由串口设定。
- **使用**：示波器/逻辑分析仪接 H2 观察波形；串口（H1）发送命令：

  | 命令 | 作用 | 示例 |
  | ---- | ---- | ---- |
  | `F<freq>` | 设置频率（Hz，1~8000） | `F1000` |
  | `D<duty>` | 设置占空比（0.1%，0~1000） | `D500`（50%） |

  上电默认 1kHz / 50%。命令执行后回显 `Freq=xxxHz, Duty=xxx/1000`。
- **注意事项**：
  - 频率上限 8000Hz（周期寄存器 ARR 固定 999）。如需更高频率，可减小 `PWM_ARR`，但会降低占空比分辨率。
  - 占空比分辨率为 0.1%（1/1000）。

### 02_demo_ina226 —— 电压/电流/功率监测

- **作用**：通过 I2C1（**SCL=PC2，SDA=PC1**，400kHz）读取 INA226，监测 VIN 输入的电压/电流/功率。
- **使用**：给板卡 P1 供电（VIN），串口（H1）每 500ms 输出一行：

  ```
  Vbus=xxxmV  Vshunt=xxxuV  I=xxxmA  P=xxxmW
  ```

  上电会先读 Manufacturer ID（0x5449）并打印 `INA226 OK` 或 `INA226 FAIL`。
- **注意事项**：
  - INA226 的 7-bit I2C 地址为 **0x40**（A1=A0=GND，已按原理图确认）。
  - 分流电阻 **1mΩ**；Current_LSB=0.2mA，Calibration=25600，最大可测电流约 **6.55A**。
  - 若实际电流超出 6.55A，需调整 `INA226_CALIB`（参考文档 `../../docs/INA226_I2C_通信说明.md`）。
  - 当前监测对象为 VIN 输入支路（VIN+/VBUS 接分流电阻高侧，VIN- 接低侧）。

### 03_demo_pwm_control_gpio —— PWM 脉宽检测 + 外部控制 GPIO

- **作用**：在 **PC3（TIM1_CH3）** 输入捕获外部 PWM 脉宽；当脉宽 **> 1500us** 时，将 **PD0** 输出高电平
  **切断外部 MOSFET 供电**；脉宽低于阈值时输出低电平恢复供电（带 ±100us 迟滞防抖动）。
- **使用**：给 PC3 输入标准 PWM/遥控器脉宽信号（如 1~2ms），用万用表/示波器观察 PD0 电平变化；
  串口（H1）每约 500ms 输出一行：

  ```
  Pulse=xxxxus  CtrlGPIO=0/1
  ```

- **注意事项**：
  - 控制输出引脚暂定为 **PD0**（普通 GPIO），如需更换请改 `CTRL_GPIO_PORT` / `CTRL_GPIO_PIN` 宏。
  - 阈值通过 `PWM_THRESHOLD_US`（默认 1500）修改；`CTRL_OFF_LEVEL`/`CTRL_ON_LEVEL` 定义切断/恢复电平极性。
  - TIM1 计数时钟经 PSC=7 分频为 1MHz（每 tick 1us），计数范围约 65.5ms；测量脉宽精度 1us。
  - 上电默认 PD0 输出低（恢复供电）。

### 04_demo_power_integral —— 读取并积分计算电量

- **作用**：通过 I2C1（**SCL=PC2，SDA=PC1**，400kHz）读取 INA226，每 **100ms** 采样一次瞬时电流，
  在 MCU 内对时间积分，实时累加已消耗电量（mAh），并将校准参数持久化到 Flash。
- **使用**：给板卡 P1 供电（VIN），串口（H1）每 1000ms 输出一行：

  ```
  Vbus=xxxmV  I=xxxmA  mAh=xxx.x
  ```

  其中 `mAh` 为自复位以来累计消耗电量（保留一位小数）。上电会先读 Manufacturer ID（0x5449）打印 `INA226 OK/FAIL`，
  随后打印当前生效的校准参数 `Calib: offset=…mA gain=…`。
- **校准（掉电保持，存 Flash）**：串口发送命令（回车/换行触发）：

  | 命令 | 作用 | 示例 |
  | ---- | ---- | ---- |
  | `Z` | 零点校准：空载时采样 50 次取平均作为零偏，写入 Flash | `Z` |
  | `O<off>` | 手动设置零偏（mA）并写入 Flash | `O-10`（-10mA） |
  | `G<gain>` | 设置增益（单位 0.001，默认 1000）并写入 Flash | `G1000` |
  | `R` | 恢复默认校准（off=0，gain=1.0）并写入 Flash | `R` |

  > 校正公式：`I_corr = (I_raw - offset) × gain`。电流修正后，功率与 mAh 会自动同步变准。
- **注意事项**：
  - **校准参数存储**：写入 Flash 末尾预留的 **1KB 页**（物理地址 `0x08003C00`）。为不与代码冲突，
    `SRC/Ld/Link.ld` 已将 FLASH 由 16K 缩短为 15K 预留该页。结构体含魔数 + CRC32 校验，上电校验失败自动回退默认值。
  - 读电流偏大约 10~12mA 通常来自 INA226 输入失调电压（约 10µV）经 1mΩ 分流电阻放大，用 `Z` 零点校准即可消除。
  - 积分原理：每 100ms 累加 `I(mA) × 0.1s`，内部以 0.1 mA·s 为单位累加，输出时换算 `mAh = 累计值 / 36000`。
  - 采样周期 `SAMPLE_PERIOD_MS`（默认 100）、打印周期 `PRINT_PERIOD_MS`（默认 1000）可改宏调整。
  - INA226 配置（地址 0x40、1mΩ 分流电阻、Current_LSB=0.2mA、最大约 6.55A）与 02 demo 一致，详见 `../../docs/INA226_I2C_通信说明.md`。
  - 电流为有符号，可正确累加充电/放电方向电量。

## 4. 公共注意事项

- 板卡使用 **8MHz 外部晶振**。各 demo 的 `system_ch32v00x.c` 均已配置 `SYSCLK_FREQ_8MHz_HSE`，
  且共享库 `SRC` 中的 `HSE_VALUE` 已改为 8MHz。**请勿改回 24/48MHz 配置**，否则波特率等会失准。
- 共享库 `SRC/Ld/Link.ld` 已将 FLASH 缩短 1K（16K→15K），**末尾 1KB 预留**给 demo 04 存校准参数，勿移除。
- 各 demo 均用 **USART1**，如与其他外设冲突（复用 PD5/PD6）需注意。
- 编译产物 `obj/`、MounRiver 工作区 `.mrs/` 已加入 `.gitignore`，不会入库。
