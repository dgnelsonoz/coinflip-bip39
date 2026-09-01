VERSION ?= dev

.PHONY: stm rp release-stm release-rp flash-stm flash-rp generate-wordlists test clean

generate-wordlists:
	python3 tools/generate_wordlists.py

stm:
	$(MAKE) -C STM32

rp:
	$(MAKE) -C RP2350

release-stm:
	$(MAKE) -C STM32 release VERSION=$(VERSION)

release-rp:
	$(MAKE) -C RP2350 release VERSION=$(VERSION)

flash-stm:
	$(MAKE) -C STM32 flash WORDLIST=$(LANGUAGE)

flash-rp:
	$(MAKE) -C RP2350 flash LANGUAGE=$(LANGUAGE)

test:
	$(MAKE) -C STM32 test
	$(MAKE) -C RP2350 test

clean:
	$(MAKE) -C STM32 clean
	$(MAKE) -C RP2350 clean
