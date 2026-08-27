################################################################################
# MRS Version: 2.4.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../APP/hidmouse.c \
../APP/hidmouse_main.c 

C_DEPS += \
./APP/hidmouse.d \
./APP/hidmouse_main.d 

OBJS += \
./APP/hidmouse.o \
./APP/hidmouse_main.o 

DIR_OBJS += \
./APP/*.o \

DIR_DEPS += \
./APP/*.d \

DIR_EXPANDS += \
./APP/*.234r.expand \


# Each subdirectory must supply rules for building sources it contributes
APP/%.o: ../APP/%.c
	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -g -DDEBUG=1 -I"c:/Users/DuRuofu/Downloads/CH583EVT/EVT/EXAM/SRC/Startup" -I"c:/Users/DuRuofu/Downloads/CH583EVT/EVT/EXAM/BLE/HID_Mouse/APP/include" -I"c:/Users/DuRuofu/Downloads/CH583EVT/EVT/EXAM/BLE/HID_Mouse/Profile/include" -I"c:/Users/DuRuofu/Downloads/CH583EVT/EVT/EXAM/SRC/StdPeriphDriver/inc" -I"c:/Users/DuRuofu/Downloads/CH583EVT/EVT/EXAM/BLE/HAL/include" -I"c:/Users/DuRuofu/Downloads/CH583EVT/EVT/EXAM/SRC/Ld" -I"c:/Users/DuRuofu/Downloads/CH583EVT/EVT/EXAM/BLE/LIB" -I"c:/Users/DuRuofu/Downloads/CH583EVT/EVT/EXAM/SRC/RVMSIS" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

