################################################################################
# MRS Version: 2.4.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
c:/Users/DuRuofu/Downloads/CH583EVT/EVT/EXAM/SRC/RVMSIS/core_riscv.c 

C_DEPS += \
./RVMSIS/core_riscv.d 

OBJS += \
./RVMSIS/core_riscv.o 

DIR_OBJS += \
./RVMSIS/*.o \

DIR_DEPS += \
./RVMSIS/*.d \

DIR_EXPANDS += \
./RVMSIS/*.234r.expand \


# Each subdirectory must supply rules for building sources it contributes
RVMSIS/core_riscv.o: c:/Users/DuRuofu/Downloads/CH583EVT/EVT/EXAM/SRC/RVMSIS/core_riscv.c
	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -g -DDEBUG=1 -I"c:/Users/DuRuofu/Downloads/CH583EVT/EVT/EXAM/SRC/Startup" -I"c:/Users/DuRuofu/Downloads/CH583EVT/EVT/EXAM/BLE/HID_Mouse/APP/include" -I"c:/Users/DuRuofu/Downloads/CH583EVT/EVT/EXAM/BLE/HID_Mouse/Profile/include" -I"c:/Users/DuRuofu/Downloads/CH583EVT/EVT/EXAM/SRC/StdPeriphDriver/inc" -I"c:/Users/DuRuofu/Downloads/CH583EVT/EVT/EXAM/BLE/HAL/include" -I"c:/Users/DuRuofu/Downloads/CH583EVT/EVT/EXAM/SRC/Ld" -I"c:/Users/DuRuofu/Downloads/CH583EVT/EVT/EXAM/BLE/LIB" -I"c:/Users/DuRuofu/Downloads/CH583EVT/EVT/EXAM/SRC/RVMSIS" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

