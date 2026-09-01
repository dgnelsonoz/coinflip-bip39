PICO_SDK_PATH ?= ../../pico-sdk
BUILD_DIR ?= build
PICOTOOL_FETCH_FROM_GIT_PATH ?= ../../RP2350/RP2350-Touch-LCD-4.3B-Demo/build/_deps
FLASH_TARGET ?= coinflip_rp2350

.PHONY: all configure clean flash test audit

all: configure
	$(MAKE) -C $(BUILD_DIR)

configure:
	cmake -S . -B $(BUILD_DIR) -G "Unix Makefiles" \
		-DPICO_SDK_PATH=$(abspath $(PICO_SDK_PATH)) \
		-DPICOTOOL_FETCH_FROM_GIT_PATH=$(abspath $(PICOTOOL_FETCH_FROM_GIT_PATH))

test:
	$(CC) -std=c11 -Wall -Wextra -Werror -Iinclude \
		src/core/bip39_lookup.c src/core/mnemonic_state.c src/core/sha256.c \
		tests/test_mnemonic_state.c -o /tmp/coinflip_mnemonic_tests
	/tmp/coinflip_mnemonic_tests

audit: configure
	$(MAKE) -C $(BUILD_DIR) coinflip_rp2350
	@if nm -C $(BUILD_DIR)/coinflip_rp2350.elf | \
		rg -q 'flash_range_(erase|program)|flash_do_cmd'; then \
		echo "ERROR: production image contains flash-write symbols"; exit 1; \
	fi
	@echo "Flash-write audit passed: no erase/program symbols in production ELF"

flash: configure
	$(MAKE) -C $(BUILD_DIR) $(FLASH_TARGET)
	@set -e; \
	uf2="$(abspath $(BUILD_DIR))/$(FLASH_TARGET).uf2"; \
	echo "Waiting for RP2350 BOOTSEL volume..."; \
	seconds=0; volume=""; \
	while [ -z "$$volume" ]; do \
		if [ -d /Volumes/RP2350 ]; then volume=/Volumes/RP2350; fi; \
		if [ -d /Volumes/RPI-RP2 ]; then volume=/Volumes/RPI-RP2; fi; \
		if [ $$seconds -ge 120 ]; then \
			echo "Timed out waiting for BOOTSEL mode"; exit 1; \
		fi; \
		if [ -z "$$volume" ]; then sleep 1; seconds=$$((seconds + 1)); fi; \
	done; \
	echo "Copying $$uf2 to $$volume"; \
	cp "$$uf2" "$$volume/"; \
	sync; \
	echo "Flash complete; the board should now be rebooting."

clean:
	$(MAKE) -C $(BUILD_DIR) clean
