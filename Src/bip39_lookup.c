#include "bip39_lookup.h"

#include <stddef.h>

static const uint8_t BIP39_BITS_PER_WORD = 11;
enum{ BIP39_WORD_COUNT = 2048 };

static const char *wordlist[BIP39_WORD_COUNT] = {
    #include "words.c.inc"
};

static uint16_t current_value = 0;
static uint8_t bit_count = 0;

void bip39_clear(void)
{
    current_value = 0;
    bit_count = 0;
}

int bip39_add_bit(uint8_t bit)
{
    if (bit_count >= BIP39_BITS_PER_WORD) {
        return -1;
    }

    if (bit != 0 && bit != 1) {
        return -2;
    }

    current_value = (current_value << 1) | bit;
    bit_count++;

    return 0;
}

void bip39_backspace(void)
{
    if (bit_count == 0) {
        return;
    }

    current_value >>= 1;
    bit_count--;
}

uint8_t bip39_get_bit_count(void)
{
    return bit_count;
}

uint16_t bip39_get_value(void)
{
    return current_value;
}

int bip39_is_complete(void)
{
    return bit_count == BIP39_BITS_PER_WORD;
}

const char *bip39_get_word(void)
{
    if (!bip39_is_complete()) {
        return "";
    }

    return bip39_get_word_by_index(current_value);
}

const char *bip39_get_word_by_index(uint16_t index)
{
    if (index >= BIP39_WORD_COUNT) {
        return "ERR";
    }

    return wordlist[index];
}
