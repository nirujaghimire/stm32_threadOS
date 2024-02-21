################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/stm32_thread_OS/_comparison_task.c \
../Core/stm32_thread_OS/comparision_task.c \
../Core/stm32_thread_OS/stm32_thread_OS.c \
../Core/stm32_thread_OS/stm32_thread_OS_test.c 

OBJS += \
./Core/stm32_thread_OS/_comparison_task.o \
./Core/stm32_thread_OS/comparision_task.o \
./Core/stm32_thread_OS/stm32_thread_OS.o \
./Core/stm32_thread_OS/stm32_thread_OS_test.o 

C_DEPS += \
./Core/stm32_thread_OS/_comparison_task.d \
./Core/stm32_thread_OS/comparision_task.d \
./Core/stm32_thread_OS/stm32_thread_OS.d \
./Core/stm32_thread_OS/stm32_thread_OS_test.d 


# Each subdirectory must supply rules for building sources it contributes
Core/stm32_thread_OS/%.o Core/stm32_thread_OS/%.su Core/stm32_thread_OS/%.cyclo: ../Core/stm32_thread_OS/%.c Core/stm32_thread_OS/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-stm32_thread_OS

clean-Core-2f-stm32_thread_OS:
	-$(RM) ./Core/stm32_thread_OS/_comparison_task.cyclo ./Core/stm32_thread_OS/_comparison_task.d ./Core/stm32_thread_OS/_comparison_task.o ./Core/stm32_thread_OS/_comparison_task.su ./Core/stm32_thread_OS/comparision_task.cyclo ./Core/stm32_thread_OS/comparision_task.d ./Core/stm32_thread_OS/comparision_task.o ./Core/stm32_thread_OS/comparision_task.su ./Core/stm32_thread_OS/stm32_thread_OS.cyclo ./Core/stm32_thread_OS/stm32_thread_OS.d ./Core/stm32_thread_OS/stm32_thread_OS.o ./Core/stm32_thread_OS/stm32_thread_OS.su ./Core/stm32_thread_OS/stm32_thread_OS_test.cyclo ./Core/stm32_thread_OS/stm32_thread_OS_test.d ./Core/stm32_thread_OS/stm32_thread_OS_test.o ./Core/stm32_thread_OS/stm32_thread_OS_test.su

.PHONY: clean-Core-2f-stm32_thread_OS

