################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../user/conversion.c \
../user/print.c \
../user/task.c \
../user/user.c 

OBJS += \
./user/conversion.o \
./user/print.o \
./user/task.o \
./user/user.o 

C_DEPS += \
./user/conversion.d \
./user/print.d \
./user/task.d \
./user/user.d 


# Each subdirectory must supply rules for building sources it contributes
user/%.o user/%.su user/%.cyclo: ../user/%.c user/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I"C:/Users/peter/OneDrive/Desktop/Github/stm32_pseudo_parallel/pseudo_parallel_v2/user" -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-user

clean-user:
	-$(RM) ./user/conversion.cyclo ./user/conversion.d ./user/conversion.o ./user/conversion.su ./user/print.cyclo ./user/print.d ./user/print.o ./user/print.su ./user/task.cyclo ./user/task.d ./user/task.o ./user/task.su ./user/user.cyclo ./user/user.d ./user/user.o ./user/user.su

.PHONY: clean-user

