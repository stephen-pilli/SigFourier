################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
./src/FDFiltering.cpp \
./src/main.cpp 

OBJS += \
./obj/FDFiltering.o \
./obj/main.o 

CPP_DEPS += \
./obj/FDFiltering.d \
./obj/main.d 


# Each subdirectory must supply rules for building sources it contributes
obj/%.o: ./src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++11 -I/usr/local/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


