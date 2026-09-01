# Coinflip BIP-39 for Waveshare RP2350 Touch LCD 4.3B

This repository contains the RP2350 port of Coinflip BIP-39. Coin flips are
the entropy source; the hardware-independent BIP-39 and SHA-256 core performs
the calculation, and the result is displayed locally. Runtime entropy,
mnemonic words, and seed material are volatile and are not stored in flash.

## Build and flash

From this directory, with the Pico SDK installed:

```sh
make test
make
make flash
```

`make flash` builds `build/coinflip_rp2350.uf2` and waits for the RP2350
BOOTSEL volume. The unmodified Waveshare demo remains available separately as
the reference hardware test suite.

## Production firmware

`coinflip_rp2350.uf2` provides the 800x480 application UI. Heads and Tails
record one flip per touch. Back requires a continuous 500 ms hold. Restart
requires a continuous 1 second hold and securely clears the volatile state.
Completed words can be selected for bit/index/list verification. Word 24 is
formed from the final three entropy bits and the SHA-256 checksum.

The display uses a small project-owned drawing layer, one RGB565 framebuffer in
PSRAM, and two fixed 800x120 RGB565 transfer buffers in internal SRAM. LVGL,
networking, radio, filesystem, SD, CAN, RS485, and unnecessary USB functions
are not included. USB and UART stdio are disabled in the production target.


## Source layout

- `src/core`: BIP-39 lookup, mnemonic state, and SHA-256 logic.
- `src/ui`: Coinflip-specific framebuffer drawing primitives.
- `src/platform` and `vendor/waveshare/bsp`: board display, touch, I²C, RGB
  PIO/DMA, and PSRAM support.
- `src/app`: production application event loop and actions.
- `tests`: host-side mnemonic and SHA-256 tests.

