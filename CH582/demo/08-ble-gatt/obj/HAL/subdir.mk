################################################################################
# MRS Version: 2.4.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
e:/DuRuofu/Project/fr-ble-light/software/ch582/HAL/MCU.c \
e:/DuRuofu/Project/fr-ble-light/software/ch582/HAL/RTC.c \
e:/DuRuofu/Project/fr-ble-light/software/ch582/HAL/SLEEP.c 

C_DEPS += \
./HAL/MCU.d \
./HAL/RTC.d \
./HAL/SLEEP.d 

OBJS += \
./HAL/MCU.o \
./HAL/RTC.o \
./HAL/SLEEP.o 

DIR_OBJS += \
./HAL/*.o \

DIR_DEPS += \
./HAL/*.d \

DIR_EXPANDS += \
./HAL/*.234r.expand \


# Each subdirectory must supply rules for building sources it contributes
HAL/MCU.o: e:/DuRuofu/Project/fr-ble-light/software/ch582/HAL/MCU.c
	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -g -DDEBUG=1 -I"e:/DuRuofu/Project/fr-ble-light/software/ch582/SRC/Startup" -I"e:/DuRuofu/Project/fr-ble-light/software/ch582/08-ble-gatt/APP/include" -I"e:/DuRuofu/Project/fr-ble-light/software/ch582/08-ble-gatt/Profile/include" -I"e:/DuRuofu/Project/fr-ble-light/software/ch582/SRC/StdPeriphDriver/inc" -I"e:/DuRuofu/Project/fr-ble-light/software/ch582/HAL/include" -I"e:/DuRuofu/Project/fr-ble-light/software/ch582/SRC/Ld" -I"e:/DuRuofu/Project/fr-ble-light/software/ch582/LIB" -I"e:/DuRuofu/Project/fr-ble-light/software/ch582/SRC/RVMSIS" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
HAL/RTC.o: e:/DuRuofu/Project/fr-ble-light/software/ch582/HAL/RTC.c
	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -g -DDEBUG=1 -I"e:/DuRuofu/Project/fr-ble-light/software/ch582/SRC/Startup" -I"e:/DuRuofu/Project/fr-ble-light/software/ch582/08-ble-gatt/APP/include" -I"e:/DuRuofu/Project/fr-ble-light/software/ch582/08-ble-gatt/Profile/include" -I"e:/DuRuofu/Project/fr-ble-light/software/ch582/SRC/StdPeriphDriver/inc" -I"e:/DuRuofu/Project/fr-ble-light/software/ch582/HAL/include" -I"e:/DuRuofu/Project/fr-ble-light/software/ch582/SRC/Ld" -I"e:/DuRuofu/Project/fr-ble-light/software/ch582/LIB" -I"e:/DuRuofu/Project/fr-ble-light/software/ch582/SRC/RVMSIS" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
HAL/SLEEP.o: e:/DuRuofu/Project/fr-ble-light/software/ch582/HAL/SLEEP.c
	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -g -DDEBUG=1 -I"e:/DuRuofu/Project/fr-ble-light/software/ch582/SRC/Startup" -I"e:/DuRuofu/Project/fr-ble-light/software/ch582/08-ble-gatt/APP/include" -I"e:/DuRuofu/Project/fr-ble-light/software/ch582/08-ble-gatt/Profile/include" -I"e:/DuRuofu/Project/fr-ble-light/software/ch582/SRC/StdPeriphDriver/inc" -I"e:/DuRuofu/Project/fr-ble-light/software/ch582/HAL/include" -I"e:/DuRuofu/Project/fr-ble-light/software/ch582/SRC/Ld" -I"e:/DuRuofu/Project/fr-ble-light/software/ch582/LIB" -I"e:/DuRuofu/Project/fr-ble-light/software/ch582/SRC/RVMSIS" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

