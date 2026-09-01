PICO_SDK_PATH ?= ../../pico-sdk
BUILD_DIR ?= build
RELEASE_DIR ?= release
VERSION ?= dev
PICOTOOL_FETCH_FROM_GIT_PATH ?=
LANGUAGE ?= english
FLASH_TARGET ?= $(if $(filter english,$(LANGUAGE)),coinflip_rp2350,coinflip_rp2350_$(LANGUAGE))
LANGUAGES = english czech french italian portuguese spanish

.PHONY: all configure clean flash release test audit

all: configure
	$(MAKE) -C $(BUILD_DIR)

configure:
	cmake -S . -B $(BUILD_DIR) -G "Unix Makefiles" \
		-DPICO_SDK_PATH=$(abspath $(PICO_SDK_PATH)) \
		-DPICOTOOL_FETCH_FROM_GIT_PATH=$(abspath $(PICOTOOL_FETCH_FROM_GIT_PATH))

test:
	$(CC) -std=c11 -Wall -Wextra -Werror -I../common \
		../common/bip39_lookup.c ../common/mnemonic_state.c ../common/sha256.c ../common/utf8.c \
		../tests/test_mnemonic_state.c -o /tmp/coinflip_mnemonic_tests
	/tmp/coinflip_mnemonic_tests

audit: configure
	$(MAKE) -C $(BUILD_DIR) coinflip_rp2350
	@if nm -C $(BUILD_DIR)/coinflip_rp2350.elf | \
		rg -q 'flash_range_(erase|program)|flash_do_cmd'; then \
		echo "ERROR: production image contains flash-write symbols"; exit 1; \
	fi
	@echo "Flash-write audit passed: no erase/program symbols in production ELF"

release: all
	@mkdir -p $(RELEASE_DIR)
	@set -e; \
	for language in $(LANGUAGES); do \
		target=coinflip_rp2350; \
		if [ "$$language" != "english" ]; then \
			target=coinflip_rp2350_$$language; \
		fi; \
		cp $(BUILD_DIR)/$$target.uf2 \
			$(RELEASE_DIR)/coinflip-rp2350-$$language-$(VERSION).uf2; \
		echo "Release file: $(RELEASE_DIR)/coinflip-rp2350-$$language-$(VERSION).uf2"; \
	done

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
