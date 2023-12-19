################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../stm32_thread_OS/stm32_thread_OS.c \
../stm32_thread_OS/stm32_thread_OS_test.c 

OBJS += \
./stm32_thread_OS/stm32_thread_OS.o \
./stm32_thread_OS/stm32_thread_OS_test.o 

C_DEPS += \
./stm32_thread_OS/stm32_thread_OS.d \
./stm32_thread_OS/stm32_thread_OS_test.d 


# Each subdirectory must supply rules for building sources it contributes
stm32_thread_OS/%.o stm32_thread_OS/%.su stm32_thread_OS/%.cyclo: ../stm32_thread_OS/%.c stm32_thread_OS/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I"C:/Users/NIRUJA/Desktop/Github/stm32_threadOS/stm32_threadOS/stm32_thread_OS" -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-stm32_thread_OS

clean-stm32_thread_OS:
	-$(RM) ./stm32_thread_OS/stm32_thread_OS.cyclo ./stm32_thread_OS/stm32_thread_OS.d ./stm32_thread_OS/stm32_thread_OS.o ./stm32_thread_OS/stm32_thread_OS.su ./stm32_thread_OS/stm32_thread_OS_test.cyclo ./stm32_thread_OS/stm32_thread_OS_test.d ./stm32_thread_OS/stm32_thread_OS_test.o ./stm32_thread_OS/stm32_thread_OS_test.su

.PHONY: clean-stm32_thread_OS

