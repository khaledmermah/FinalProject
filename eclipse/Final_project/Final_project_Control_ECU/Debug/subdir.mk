################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../DC_motor.c \
../PWM.c \
../TIMER.c \
../buzzer.c \
../control_main.c \
../external_eeprom.c \
../gpio.c \
../lcd.c \
../twi.c \
../uart.c 

OBJS += \
./DC_motor.o \
./PWM.o \
./TIMER.o \
./buzzer.o \
./control_main.o \
./external_eeprom.o \
./gpio.o \
./lcd.o \
./twi.o \
./uart.o 

C_DEPS += \
./DC_motor.d \
./PWM.d \
./TIMER.d \
./buzzer.d \
./control_main.d \
./external_eeprom.d \
./gpio.d \
./lcd.d \
./twi.d \
./uart.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: AVR Compiler'
	avr-gcc -Wall -g2 -gstabs -O0 -fpack-struct -fshort-enums -ffunction-sections -fdata-sections -std=gnu99 -funsigned-char -funsigned-bitfields -mmcu=atmega32 -DF_CPU=8000000UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


