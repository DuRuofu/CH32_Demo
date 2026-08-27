################################################################################
# MRS Version: 2.4.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Profile/devinfoservice.c \
../Profile/gattprofile.c 

C_DEPS += \
./Profile/devinfoservice.d \
./Profile/gattprofile.d 

OBJS += \
./Profile/devinfoservice.o \
./Profile/gattprofile.o 

DIR_OBJS += \
./Profile/*.o \

DIR_DEPS += \
./Profile/*.d \

DIR_EXPANDS += \
./Profile/*.234r.expand \


# Each subdirectory must supply rules for building sources it contributes
Profile/%.o: ../Profile/%.c
	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -g -DDEBUG=1 -I"e:/DuRuofu/Project/fr-ble-light/software/ch582/SRC/Startup" -I"e:/DuRuofu/Project/fr-ble-light/software/ch582/08-ble-gatt/APP/include" -I"e:/DuRuofu/Project/fr-ble-light/software/ch582/08-ble-gatt/Profile/include" -I"e:/DuRuofu/Project/fr-ble-light/software/ch582/SRC/StdPeriphDriver/inc" -I"e:/DuRuofu/Project/fr-ble-light/software/ch582/HAL/include" -I"e:/DuRuofu/Project/fr-ble-light/software/ch582/SRC/Ld" -I"e:/DuRuofu/Project/fr-ble-light/software/ch582/LIB" -I"e:/DuRuofu/Project/fr-ble-light/software/ch582/SRC/RVMSIS" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

