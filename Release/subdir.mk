################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../cheader.cpp \
../cresponse.cpp \
../request_router.cpp \
../web.cpp 

OBJS += \
./cheader.o \
./cresponse.o \
./request_router.o \
./web.o 

CPP_DEPS += \
./cheader.d \
./cresponse.d \
./request_router.d \
./web.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


