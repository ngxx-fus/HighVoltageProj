# Toolchain definition
CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SZ = arm-none-eabi-size

# CPU architecture
CPU = -mcpu=cortex-m3 -mthumb

# 1. Source files (Declare ALL .c files here)
SRCS = \
	main.c \
	startup.c \
	AppUtils/AppUtils.c \
	AppUtils/Deque.c \
	AppComm/INT/INT.c \
	AppComm/PWM/PWM.c \
	AppComm/I2C1/I2C1.c \
	AppComm/OLED128x64/OLED128x64.c \
	AppFonts/fontUtils.c \
	AppLog/SerialLog.c

# 2. Include directories (Helps GCC find the .h files)
INCLUDES = \
	-I. \
	-IAppUtils \
	-IAppComm/INT \
	-IAppComm/PWM \
	-IAppComm/I2C1 \
	-IAppComm/OLED128x64 \
	-IAppFonts \
	-IAppFonts/localFonts \
	-IAppLog

# 3. Flags
CFLAGS = $(CPU) -Wall -O0 $(INCLUDES)
LDFLAGS = -T linker.ld -specs=nano.specs -specs=nosys.specs -nostartfiles $(CPU)

# 4. Generate Object files list (.c -> .o)
OBJS = $(SRCS:.c=.o)

# Target executable name
TARGET = final

# Targets
all: $(TARGET).bin size

# Rule: Compile C files to Object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Rule: Link Object files to create ELF
$(TARGET).elf: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

# Rule: Convert ELF to BIN for flashing
$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

# Rule: Print memory size (Very useful for checking RAM/Flash usage)
size: $(TARGET).elf
	$(SZ) $<

# Flash command using st-flash
flash: $(TARGET).bin
	sudo st-flash write $(TARGET).bin 0x08000000

# Clean up build files (Remove ALL .o files recursively across all components)
clean:
	find . -name "*.o" -type f -delete
	rm -f $(TARGET).elf $(TARGET).bin

.PHONY: all clean flash size