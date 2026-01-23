################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Middlewares/PahoMQTT/src/MQTTClient.c \
../Middlewares/PahoMQTT/src/MQTTConnectClient.c \
../Middlewares/PahoMQTT/src/MQTTConnectServer.c \
../Middlewares/PahoMQTT/src/MQTTDeserializePublish.c \
../Middlewares/PahoMQTT/src/MQTTFormat.c \
../Middlewares/PahoMQTT/src/MQTTPacket.c \
../Middlewares/PahoMQTT/src/MQTTSerializePublish.c \
../Middlewares/PahoMQTT/src/MQTTSubscribeClient.c \
../Middlewares/PahoMQTT/src/MQTTSubscribeServer.c \
../Middlewares/PahoMQTT/src/MQTTUnsubscribeClient.c \
../Middlewares/PahoMQTT/src/MQTTUnsubscribeServer.c \
../Middlewares/PahoMQTT/src/NetworkInterface.c \
../Middlewares/PahoMQTT/src/Timer.c 

OBJS += \
./Middlewares/PahoMQTT/src/MQTTClient.o \
./Middlewares/PahoMQTT/src/MQTTConnectClient.o \
./Middlewares/PahoMQTT/src/MQTTConnectServer.o \
./Middlewares/PahoMQTT/src/MQTTDeserializePublish.o \
./Middlewares/PahoMQTT/src/MQTTFormat.o \
./Middlewares/PahoMQTT/src/MQTTPacket.o \
./Middlewares/PahoMQTT/src/MQTTSerializePublish.o \
./Middlewares/PahoMQTT/src/MQTTSubscribeClient.o \
./Middlewares/PahoMQTT/src/MQTTSubscribeServer.o \
./Middlewares/PahoMQTT/src/MQTTUnsubscribeClient.o \
./Middlewares/PahoMQTT/src/MQTTUnsubscribeServer.o \
./Middlewares/PahoMQTT/src/NetworkInterface.o \
./Middlewares/PahoMQTT/src/Timer.o 

C_DEPS += \
./Middlewares/PahoMQTT/src/MQTTClient.d \
./Middlewares/PahoMQTT/src/MQTTConnectClient.d \
./Middlewares/PahoMQTT/src/MQTTConnectServer.d \
./Middlewares/PahoMQTT/src/MQTTDeserializePublish.d \
./Middlewares/PahoMQTT/src/MQTTFormat.d \
./Middlewares/PahoMQTT/src/MQTTPacket.d \
./Middlewares/PahoMQTT/src/MQTTSerializePublish.d \
./Middlewares/PahoMQTT/src/MQTTSubscribeClient.d \
./Middlewares/PahoMQTT/src/MQTTSubscribeServer.d \
./Middlewares/PahoMQTT/src/MQTTUnsubscribeClient.d \
./Middlewares/PahoMQTT/src/MQTTUnsubscribeServer.d \
./Middlewares/PahoMQTT/src/NetworkInterface.d \
./Middlewares/PahoMQTT/src/Timer.d 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/PahoMQTT/src/%.o Middlewares/PahoMQTT/src/%.su Middlewares/PahoMQTT/src/%.cyclo: ../Middlewares/PahoMQTT/src/%.c Middlewares/PahoMQTT/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F756xx -c -I../Core/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1 -I../Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../Drivers/CMSIS/Include -I../LWIP/App -I../LWIP/Target -I../Middlewares/Third_Party/LwIP/src/include -I../Middlewares/Third_Party/LwIP/system -I../Middlewares/Third_Party/LwIP/src/include/netif/ppp -I../Middlewares/Third_Party/LwIP/src/include/lwip -I../Middlewares/Third_Party/LwIP/src/include/lwip/apps -I../Middlewares/Third_Party/LwIP/src/include/lwip/priv -I../Middlewares/Third_Party/LwIP/src/include/lwip/prot -I../Middlewares/Third_Party/LwIP/src/include/netif -I../Middlewares/Third_Party/LwIP/src/include/compat/posix -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/arpa -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/net -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/sys -I../Middlewares/Third_Party/LwIP/src/include/compat/stdc -I../Middlewares/Third_Party/LwIP/system/arch -I../Drivers/BSP/Components/lan8742 -I../Middlewares/PahoMQTT/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Middlewares-2f-PahoMQTT-2f-src

clean-Middlewares-2f-PahoMQTT-2f-src:
	-$(RM) ./Middlewares/PahoMQTT/src/MQTTClient.cyclo ./Middlewares/PahoMQTT/src/MQTTClient.d ./Middlewares/PahoMQTT/src/MQTTClient.o ./Middlewares/PahoMQTT/src/MQTTClient.su ./Middlewares/PahoMQTT/src/MQTTConnectClient.cyclo ./Middlewares/PahoMQTT/src/MQTTConnectClient.d ./Middlewares/PahoMQTT/src/MQTTConnectClient.o ./Middlewares/PahoMQTT/src/MQTTConnectClient.su ./Middlewares/PahoMQTT/src/MQTTConnectServer.cyclo ./Middlewares/PahoMQTT/src/MQTTConnectServer.d ./Middlewares/PahoMQTT/src/MQTTConnectServer.o ./Middlewares/PahoMQTT/src/MQTTConnectServer.su ./Middlewares/PahoMQTT/src/MQTTDeserializePublish.cyclo ./Middlewares/PahoMQTT/src/MQTTDeserializePublish.d ./Middlewares/PahoMQTT/src/MQTTDeserializePublish.o ./Middlewares/PahoMQTT/src/MQTTDeserializePublish.su ./Middlewares/PahoMQTT/src/MQTTFormat.cyclo ./Middlewares/PahoMQTT/src/MQTTFormat.d ./Middlewares/PahoMQTT/src/MQTTFormat.o ./Middlewares/PahoMQTT/src/MQTTFormat.su ./Middlewares/PahoMQTT/src/MQTTPacket.cyclo ./Middlewares/PahoMQTT/src/MQTTPacket.d ./Middlewares/PahoMQTT/src/MQTTPacket.o ./Middlewares/PahoMQTT/src/MQTTPacket.su ./Middlewares/PahoMQTT/src/MQTTSerializePublish.cyclo ./Middlewares/PahoMQTT/src/MQTTSerializePublish.d ./Middlewares/PahoMQTT/src/MQTTSerializePublish.o ./Middlewares/PahoMQTT/src/MQTTSerializePublish.su ./Middlewares/PahoMQTT/src/MQTTSubscribeClient.cyclo ./Middlewares/PahoMQTT/src/MQTTSubscribeClient.d ./Middlewares/PahoMQTT/src/MQTTSubscribeClient.o ./Middlewares/PahoMQTT/src/MQTTSubscribeClient.su ./Middlewares/PahoMQTT/src/MQTTSubscribeServer.cyclo ./Middlewares/PahoMQTT/src/MQTTSubscribeServer.d ./Middlewares/PahoMQTT/src/MQTTSubscribeServer.o ./Middlewares/PahoMQTT/src/MQTTSubscribeServer.su ./Middlewares/PahoMQTT/src/MQTTUnsubscribeClient.cyclo ./Middlewares/PahoMQTT/src/MQTTUnsubscribeClient.d ./Middlewares/PahoMQTT/src/MQTTUnsubscribeClient.o ./Middlewares/PahoMQTT/src/MQTTUnsubscribeClient.su ./Middlewares/PahoMQTT/src/MQTTUnsubscribeServer.cyclo ./Middlewares/PahoMQTT/src/MQTTUnsubscribeServer.d ./Middlewares/PahoMQTT/src/MQTTUnsubscribeServer.o ./Middlewares/PahoMQTT/src/MQTTUnsubscribeServer.su ./Middlewares/PahoMQTT/src/NetworkInterface.cyclo ./Middlewares/PahoMQTT/src/NetworkInterface.d ./Middlewares/PahoMQTT/src/NetworkInterface.o ./Middlewares/PahoMQTT/src/NetworkInterface.su ./Middlewares/PahoMQTT/src/Timer.cyclo ./Middlewares/PahoMQTT/src/Timer.d ./Middlewares/PahoMQTT/src/Timer.o ./Middlewares/PahoMQTT/src/Timer.su

.PHONY: clean-Middlewares-2f-PahoMQTT-2f-src

