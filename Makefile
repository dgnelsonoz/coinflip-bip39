.PHONY: stm rp flash-stm flash-rp generate-wordlists test clean

generate-wordlists:
	python3 tools/generate_wordlists.py

stm:
	$(MAKE) -C STM32

rp:
	$(MAKE) -C RP2350

flash-stm:
	$(MAKE) -C STM32 flash

flash-rp:
	$(MAKE) -C RP2350 flash

test:
	$(MAKE) -C STM32 test
	$(MAKE) -C RP2350 test

clean:
	$(MAKE) -C STM32 clean
	$(MAKE) -C RP2350 clean
