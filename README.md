# Coinflip BIP-39

Coinflip is a transparent, air-gapped aid for generating a 24-word BIP-39
mnemonic from physical coin flips or dice rolls.

This repository contains both supported hardware implementations:

* `STM32/` — STM32F469I-DISCO
* `RP2350/` — Waveshare RP2350-Touch-LCD-4.3B

The two platform directories currently retain their own build systems and
hardware-specific code. The shared BIP-39/state code will be consolidated as
part of the ongoing repository reorganisation.

## Building

From the repository root:

```bash
make stm
make rp
make test
```

The STM32 build requires the Arm GNU toolchain, GNU Make, STM32CubeF4, and
STM32CubeProgrammer. The RP2350 build requires the Arm GNU toolchain, CMake,
GNU Make, and the Raspberry Pi Pico SDK.

By default, the external SDKs are expected beside this repository:

```text
Developer/
├── coinflip-bip39/
├── STM32CubeF4/
└── pico-sdk/
```

If they are installed elsewhere, pass their locations on the command line:

```bash
make stm CUBE=/path/to/STM32CubeF4
make rp PICO_SDK_PATH=/path/to/pico-sdk
```

The RP2350 build also uses the locally prepared Waveshare demo build when it
needs Picotool. Its location can be overridden with
`PICOTOOL_FETCH_FROM_GIT_PATH` when invoking the RP2350 Makefile.
