PROJECT = coinflip
BUILD_DIR = build
RELEASE_DIR = release
VERSION ?= dev

CUBE ?= ../../STM32CubeF4

CC = arm-none-eabi-gcc
HOST_CC ?= cc
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size

CPU = -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard
DEFS = -DSTM32F469xx -DUSE_HAL_DRIVER
OPT = -Og

CFLAGS = $(CPU) $(DEFS) $(OPT) -Wall -ffunction-sections -fdata-sections
CFLAGS += -Iinc
CFLAGS += -I$(CUBE)/Drivers/CMSIS/Device/ST/STM32F4xx/Include
CFLAGS += -I$(CUBE)/Drivers/CMSIS/Include
CFLAGS += -I$(CUBE)/Drivers/STM32F4xx_HAL_Driver/Inc
CFLAGS += -I$(CUBE)/Drivers/BSP/STM32469I-Discovery
CFLAGS += -I$(CUBE)/Drivers/BSP/Components/Common
CFLAGS += -I$(CUBE)/Utilities/Fonts
CFLAGS += -I$(CUBE)/Drivers/BSP/Components/ft6x06

LDSCRIPT = STM32F469NIHX_FLASH.ld
LDFLAGS = $(CPU) -T$(LDSCRIPT) -Wl,--gc-sections
LDFLAGS += -Wl,-Map=$(BUILD_DIR)/$(PROJECT).map
LDFLAGS += -Wl,--no-warn-rwx-segments

LDLIBS = -lc -lm -lnosys

VPATH = \
	src \
	$(CUBE)/Drivers/STM32F4xx_HAL_Driver/Src \
	$(CUBE)/Drivers/BSP/STM32469I-Discovery \
	$(CUBE)/Drivers/BSP/Components/nt35510 \
	$(CUBE)/Drivers/BSP/Components/ft6x06

SRCS = \
	main.c \
	bip39_lookup.c \
	mnemonic_state.c \
	mnemonic_ui.c \
	sha256.c \
	stm32f4xx_it.c \
	system_stm32f4xx.c \
	startup_stm32f469nihx.s \
	stm32469i_discovery_ts.c \
	ft6x06.c \
	syscalls.c \
	stm32f4xx_hal.c \
	stm32f4xx_hal_cortex.c \
	stm32f4xx_hal_gpio.c \
	stm32f4xx_hal_rcc.c \
	stm32f4xx_hal_rcc_ex.c \
	stm32f4xx_hal_pwr.c \
	stm32f4xx_hal_pwr_ex.c \
	stm32469i_discovery.c \
	stm32469i_discovery_lcd.c \
	stm32f4xx_hal_dma2d.c \
	stm32f4xx_hal_ltdc.c \
	stm32f4xx_hal_ltdc_ex.c \
	stm32f4xx_hal_dsi.c \
	stm32f4xx_hal_dma.c \
	stm32f4xx_hal_sdram.c \
	stm32f4xx_ll_fmc.c \
	stm32469i_discovery_sdram.c \
	nt35510.c \
	stm32f4xx_hal_i2c.c

OBJS = $(addprefix $(BUILD_DIR)/,$(SRCS:.c=.o))
OBJS := $(OBJS:.s=.o)

all: $(BUILD_DIR)/$(PROJECT).elf

$(BUILD_DIR)/$(PROJECT).elf: $(OBJS) $(LDSCRIPT)
	@mkdir -p $(dir $@)
	$(CC) $(OBJS) $(LDFLAGS) $(LDLIBS) -o $@
	$(SIZE) $@

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.s
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CPU) -c $< -o $@

flash: $(BUILD_DIR)/$(PROJECT).elf
	STM32_Programmer_CLI -c port=SWD -w $(BUILD_DIR)/$(PROJECT).elf -v -rst

release: $(BUILD_DIR)/$(PROJECT).elf
	@mkdir -p $(RELEASE_DIR)
	cp $(BUILD_DIR)/$(PROJECT).elf $(RELEASE_DIR)/$(PROJECT)-$(VERSION).elf
	@echo "Release file: $(RELEASE_DIR)/$(PROJECT)-$(VERSION).elf"

test: $(BUILD_DIR)/test_mnemonic_state
	$(BUILD_DIR)/test_mnemonic_state

$(BUILD_DIR)/test_mnemonic_state: tests/test_mnemonic_state.c src/mnemonic_state.c src/sha256.c src/bip39_lookup.c inc/mnemonic_state.h inc/sha256.h inc/bip39_lookup.h
	@mkdir -p $(BUILD_DIR)
	$(HOST_CC) -std=c11 -Wall -Wextra -Werror -Iinc tests/test_mnemonic_state.c src/mnemonic_state.c src/sha256.c src/bip39_lookup.c -o $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean flash release test
