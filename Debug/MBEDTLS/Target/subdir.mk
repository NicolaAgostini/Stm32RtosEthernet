################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../MBEDTLS/Target/hardware_rng.c 

OBJS += \
./MBEDTLS/Target/hardware_rng.o 

C_DEPS += \
./MBEDTLS/Target/hardware_rng.d 


# Each subdirectory must supply rules for building sources it contributes
MBEDTLS/Target/%.o MBEDTLS/Target/%.su MBEDTLS/Target/%.cyclo: ../MBEDTLS/Target/%.c MBEDTLS/Target/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F756xx '-DMBEDTLS_CONFIG_FILE=<mbedtls_config.h>' -c -I../Core/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1 -I../Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../Drivers/CMSIS/Include -I../LWIP/App -I../LWIP/Target -I../Middlewares/Third_Party/LwIP/src/include -I../Middlewares/Third_Party/LwIP/system -I../Middlewares/Third_Party/LwIP/src/include/netif/ppp -I../Middlewares/Third_Party/LwIP/src/include/lwip -I../Middlewares/Third_Party/LwIP/src/include/lwip/apps -I../Middlewares/Third_Party/LwIP/src/include/lwip/priv -I../Middlewares/Third_Party/LwIP/src/include/lwip/prot -I../Middlewares/Third_Party/LwIP/src/include/netif -I../Middlewares/Third_Party/LwIP/src/include/compat/posix -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/arpa -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/net -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/sys -I../Middlewares/Third_Party/LwIP/src/include/compat/stdc -I../Middlewares/Third_Party/LwIP/system/arch -I../Drivers/BSP/Components/lan8742 -I../Middlewares/PahoMQTT/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../MBEDTLS/App -I../Middlewares/Third_Party/LwIP/src/apps/http -I../Middlewares/Third_Party/mbedTLS/include/mbedtls -I../Middlewares/Third_Party/LwIP/src/apps/altcp_tls -I../Middlewares/Third_Party/mbedTLS/include -I"C:/Users/n.agostini/Documents/STM32CUBEProjects/DISCOF756ZGFreeRTOS/Middlewares" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-MBEDTLS-2f-Target

clean-MBEDTLS-2f-Target:
	-$(RM) ./MBEDTLS/Target/hardware_rng.cyclo ./MBEDTLS/Target/hardware_rng.d ./MBEDTLS/Target/hardware_rng.o ./MBEDTLS/Target/hardware_rng.su

.PHONY: clean-MBEDTLS-2f-Target

