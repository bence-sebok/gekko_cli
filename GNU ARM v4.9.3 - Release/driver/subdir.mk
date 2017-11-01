################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/SiliconLabs/SimplicityStudio/v4/developer/sdks/gecko_sdk_suite/v1.1/hardware/kit/common/drivers/segmentlcd.c 

OBJS += \
./driver/segmentlcd.o 

C_DEPS += \
./driver/segmentlcd.d 


# Each subdirectory must supply rules for building sources it contributes
driver/segmentlcd.o: C:/SiliconLabs/SimplicityStudio/v4/developer/sdks/gecko_sdk_suite/v1.1/hardware/kit/common/drivers/segmentlcd.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -mcpu=cortex-m3 -mthumb -std=c99 '-DNDEBUG=1' '-DEFM32GG990F1024=1' -I"C:\Workspace\gyak4\inc" -I"C:/Workspace/gyak4/inc" -I"C:/SiliconLabs/SimplicityStudio/v4/developer/sdks/gecko_sdk_suite/v1.1//hardware/kit/common/bsp" -I"C:/SiliconLabs/SimplicityStudio/v4/developer/sdks/gecko_sdk_suite/v1.1//platform/CMSIS/Include" -I"C:/SiliconLabs/SimplicityStudio/v4/developer/sdks/gecko_sdk_suite/v1.1//hardware/kit/EFM32GG_STK3700/config" -I"C:/SiliconLabs/SimplicityStudio/v4/developer/sdks/gecko_sdk_suite/v1.1//platform/Device/SiliconLabs/EFM32GG/Include" -I"C:/SiliconLabs/SimplicityStudio/v4/developer/sdks/gecko_sdk_suite/v1.1//platform/emlib/inc" -I"C:/SiliconLabs/SimplicityStudio/v4/developer/sdks/gecko_sdk_suite/v1.1//hardware/kit/common/drivers" -O3 -Wall -c -fmessage-length=0 -ffunction-sections -fdata-sections -MMD -MP -MF"driver/segmentlcd.d" -MT"driver/segmentlcd.o" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


