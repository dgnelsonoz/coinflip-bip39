.PHONY: stm rp test clean

stm:
	$(MAKE) -C STM32

rp:
	$(MAKE) -C RP2350

test:
	$(MAKE) -C STM32 test
	$(MAKE) -C RP2350 test

clean:
	$(MAKE) -C STM32 clean
	$(MAKE) -C RP2350 clean
