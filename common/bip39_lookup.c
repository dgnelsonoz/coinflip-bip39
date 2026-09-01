#include "bip39_lookup.h"

enum { BIP39_WORD_COUNT = 2048 };

static const char *wordlist[ BIP39_WORD_COUNT ] =
{
#include "wordlists/english_words.c.inc"
};

const char *bip39_get_word_by_index( uint16_t index )
{
    if( index >= BIP39_WORD_COUNT )
        return "ERR";

    return wordlist[ index ];
}
