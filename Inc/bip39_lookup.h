#ifndef BIP39_LOOKUP_H
#define BIP39_LOOKUP_H

#include <stdint.h>

void bip39_clear(void);
int bip39_add_bit(uint8_t bit);
void bip39_backspace(void);

uint8_t bip39_get_bit_count(void);
uint16_t bip39_get_value(void);
int bip39_is_complete(void);

const char *bip39_get_word(void);
const char *bip39_get_word_by_index(uint16_t index);

#endif
