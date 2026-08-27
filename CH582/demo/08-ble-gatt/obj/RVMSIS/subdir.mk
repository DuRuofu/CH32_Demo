################################################################################
# MRS Version: 2.4.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
e:/DuRuofu/Project/fr-ble-light/software/ch582/SRC/RVMSIS/core_riscv.c 

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
RVMSIS/core_riscv.o: e:/DuRuofu/Project/fr-ble-light/software/ch582/SRC/RVMSIS/core_riscv.c
	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -g -DDEBUG=1 -I"e:/DuRuofu/Project/fr-ble-light/software/ch582/SRC/Startup" -I"e:/DuRuofu/Project/fr-ble-light/software/ch582/08-ble-gatt/APP/include" -I"e:/DuRuofu/Project/fr-ble-light/software/ch582/08-ble-gatt/Profile/include" -I"e:/DuRuofu/Project/fr-ble-light/software/ch582/SRC/StdPeriphDriver/inc" -I"e:/DuRuofu/Project/fr-ble-light/software/ch582/HAL/include" -I"e:/DuRuofu/Project/fr-ble-light/software/ch582/SRC/Ld" -I"e:/DuRuofu/Project/fr-ble-light/software/ch582/LIB" -I"e:/DuRuofu/Project/fr-ble-light/software/ch582/SRC/RVMSIS" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

