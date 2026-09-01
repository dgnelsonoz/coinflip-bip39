## Coinflip BIP-39

Coinflip is a transparent, air-gapped aid for generating a 24-word BIP-39 mnemonic from coin flips. It runs on the STM32F469I Discovery development board (STM32F469I-DISCO), using its integrated 800 x 480 touchscreen.

![Coinflip running on the STM32F469I Discovery board](STM32/docs/images/stm32f469i-discovery.png)

In terms of security, the most critical part of your Bitcoin wallet is the private master key. So, do you trust the software in your wallet to randomly generate your keys for you? I’m looking at the bag that my Coldcard hardware wallet came in. On the bag, in bold letters, it says *“DON’T TRUST. VERIFY”*. If you’ve been following the Coldcard fiasco you can see the irony.

BIP-39 defines how to generate your keys from a random selection of 24 words out of a sequenced list of 2048 words. Lose your wallet and you can regenerate it with your 24-word seed phrase.

Each of the first 23 words can be randomly selected by flipping a coin eleven times. Let heads be a binary 0 and tails be a binary 1. Sequence those eleven bits and convert the resulting binary number to decimal, add 1, then look up the corresponding number in the BIP-39 word list. Do that 23 times and you have 23 randomly selected seed words. The remaining three coin flips are combined with the checksum to determine the 24th word. No computer, no algorithm, no software-generated random number, just *100% randomness, 100% transparency*.

The problem is that it’s tedious to flip a coin 256 times, write the numbers down, convert them to decimal and look them up in the BIP-39 word list. And then you have to calculate the checksum for the 24th word. The coin flip to BIP-39 converter makes that process easier, *transparently*.

## How it works

Flip a coin 256 times and enter the results into the converter. For each word generated you can view the binary digits, their decimal equivalent, and the decimal equivalent plus one. (Digital sequences usually start at 0; humans usually start counting from 1.) You can look up the word in your own independent BIP-39 word list. *Don’t trust, verify*.

You can find the English BIP-39 word list [here](https://github.com/bitcoin/bips/blob/master/bip-0039/english.txt).

For maximum security, plug the power cable into a 5 V USB wall plug and not the USB port on your computer. There is no data connection between the converter hardware and a computer, but *don’t trust that*.

The seed phrase is deleted when powered down; nothing is stored in permanent memory. But *don’t trust that either*. Don’t pass on the converter for someone else to use. Don’t borrow one from someone unless you trust them explicitly.

## Flashing the board

Download the `.elf` file from the **latest GitHub release** and install [STM32CubeProgrammer](https://www.st.com/en/development-tools/stm32cubeprog.html).

Connect the STM32F469I-DISCO to your computer using a **USB Mini-B data cable** connected to the **ST-LINK USB connector (CN1)**. Do not use the other USB connector on the board.

Flash the firmware with:

```bash
STM32_Programmer_CLI -c port=SWD -w coinflip-<version>.elf -v -rst
```

Replace `coinflip-<version>.elf` with the name of the downloaded `.elf` file.

## Building from source

The project was developed on macOS. The process on Linux and other platforms should be similar.

The following development tools are required:

* [Arm GNU Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) (`arm-none-eabi-gcc`)
* GNU Make
* [STM32CubeF4](https://www.st.com/en/embedded-software/stm32cubef4.html) — V1.28.0 was used during development
* STM32CubeProgrammer for flashing the board

Clone the repository and build with:

```bash
make CUBE=/path/to/STM32CubeF4
```

This produces:

```text
build/coinflip.elf
```

To build and flash the board in one step:

```bash
make flash
```

## Additional platform support

The repository also contains an RP2350 implementation in `RP2350/`. The
platform-specific source and build systems remain in their respective
directories, while the root Makefile provides these convenience commands:

```bash
make stm
make rp
make flash-stm
make flash-rp
make test
```

The default external SDK layout is:

```text
Developer/
├── coinflip-bip39/
├── STM32CubeF4/
└── pico-sdk/
```

Override SDK locations when necessary:

```bash
make stm CUBE=/path/to/STM32CubeF4
make rp PICO_SDK_PATH=/path/to/pico-sdk
```

The RP2350 build also uses the prepared Waveshare demo build when it needs
Picotool. Override `PICOTOOL_FETCH_FROM_GIT_PATH` if that demo is installed
elsewhere.

The host development tools and board SDKs must be installed separately. The
required tools are listed above; the project does not install or vendor them.

## Platform-specific commands

Most users should download the firmware file from the GitHub release and
follow the flashing instructions above. These commands are for developers
building from source.

For STM32, use the STM32CubeProgrammer command-line tool:

```bash
STM32_Programmer_CLI -c port=SWD -w STM32/build/coinflip.elf -v -rst
```

For RP2350, put the board into BOOTSEL mode and copy
`RP2350/build/coinflip_rp2350.uf2` to the mounted `RP2350`/`RPI-RP2` drive.
The root Makefile provides `make flash-rp` for macOS and Linux systems where
that drive is mounted automatically. On Windows, copy the `.uf2` file to the
BOOTSEL drive using File Explorer.
