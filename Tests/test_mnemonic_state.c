#include "mnemonic_state.h"
#include "bip39_lookup.h"
#include "sha256.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void add_repeating_bits(MnemonicState *state, uint16_t count)
{
    uint16_t ii;

    for (ii = 0; ii < count; ii++) {
        assert(mnemonic_state_add_flip(state, (uint8_t)(ii & 1U)) == 0);
    }
}

static void test_initial_state_and_validation(void)
{
    MnemonicState state;

    mnemonic_state_init(&state);
    assert(mnemonic_state_get_bit_count(&state) == 0);
    assert(mnemonic_state_get_completed_word_count(&state) == 0);
    assert(mnemonic_state_get_current_word_number(&state) == 1);
    assert(mnemonic_state_get_current_word_bit_count(&state) == 0);
    assert(!mnemonic_state_entropy_complete(&state));
    assert(mnemonic_state_add_flip(&state, 2) == -2);
    assert(mnemonic_state_get_bit_count(&state) == 0);
    assert(mnemonic_state_backspace(&state) == 0);
}

static void test_word_completion_and_index(void)
{
    MnemonicState state;
    uint16_t index = 0;

    mnemonic_state_init(&state);
    add_repeating_bits(&state, 11);

    assert(mnemonic_state_get_completed_word_count(&state) == 1);
    assert(mnemonic_state_get_current_word_number(&state) == 2);
    assert(mnemonic_state_get_current_word_bit_count(&state) == 0);
    assert(mnemonic_state_get_word_index(&state, 1, &index) == 0);
    assert(index == 0x2AA);
    assert(mnemonic_state_get_word_index(&state, 2, &index) == -3);
}

static void test_backspace_stops_at_current_word(void)
{
    MnemonicState state;
    uint16_t ii;

    mnemonic_state_init(&state);
    add_repeating_bits(&state, 16);
    for (ii = 0; ii < 5; ii++) {
        assert(mnemonic_state_backspace(&state) == 1);
    }
    assert(mnemonic_state_get_bit_count(&state) == 11);
    assert(mnemonic_state_get_completed_word_count(&state) == 1);
    assert(mnemonic_state_backspace(&state) == 0);
}

static void test_backspace_reopens_completed_word(void)
{
    MnemonicState state;
    uint16_t ii;

    mnemonic_state_init(&state);
    add_repeating_bits(&state, 11);
    for (ii = 0; ii < 11; ii++) {
        assert(mnemonic_state_backspace(&state) == 1);
    }
    assert(mnemonic_state_get_bit_count(&state) == 0);
    assert(mnemonic_state_backspace(&state) == 0);
}

static void test_final_entropy_bits(void)
{
    MnemonicState state;

    mnemonic_state_init(&state);
    add_repeating_bits(&state, 253);
    assert(mnemonic_state_get_completed_word_count(&state) == 23);
    assert(mnemonic_state_get_current_word_number(&state) == 24);
    assert(mnemonic_state_get_current_word_bit_count(&state) == 0);

    add_repeating_bits(&state, 3);
    assert(mnemonic_state_get_current_word_bit_count(&state) == 3);
    assert(mnemonic_state_entropy_complete(&state));
    assert(mnemonic_state_add_flip(&state, 0) == -3);

    assert(mnemonic_state_backspace(&state) == 1);
    assert(mnemonic_state_get_bit_count(&state) == 255);
    assert(!mnemonic_state_entropy_complete(&state));
    assert(mnemonic_state_backspace(&state) == 1);
    assert(mnemonic_state_backspace(&state) == 1);
    assert(mnemonic_state_get_bit_count(&state) == 253);
    assert(mnemonic_state_backspace(&state) == 0);
}

static void test_secure_restart(void)
{
    MnemonicState state;
    uint16_t ii;

    mnemonic_state_init(&state);
    for (ii = 0; ii < MNEMONIC_ENTROPY_BITS; ii++) {
        assert(mnemonic_state_add_flip(&state, 1) == 0);
    }
    mnemonic_state_init(&state);

    assert(mnemonic_state_get_bit_count(&state) == 0);
    for (ii = 0; ii < MNEMONIC_ENTROPY_BYTES; ii++) {
        assert(state.entropy[ii] == 0);
    }
}

static void assert_digest(const uint8_t *message, size_t length,
                          const uint8_t expected[SHA256_DIGEST_SIZE])
{
    uint8_t actual[SHA256_DIGEST_SIZE];

    sha256(message, length, actual);
    assert(memcmp(actual, expected, SHA256_DIGEST_SIZE) == 0);
}

static void test_sha256_vectors(void)
{
    static const uint8_t empty_digest[SHA256_DIGEST_SIZE] = {
        0xe3, 0xb0, 0xc4, 0x42, 0x98, 0xfc, 0x1c, 0x14,
        0x9a, 0xfb, 0xf4, 0xc8, 0x99, 0x6f, 0xb9, 0x24,
        0x27, 0xae, 0x41, 0xe4, 0x64, 0x9b, 0x93, 0x4c,
        0xa4, 0x95, 0x99, 0x1b, 0x78, 0x52, 0xb8, 0x55
    };
    static const uint8_t abc_digest[SHA256_DIGEST_SIZE] = {
        0xba, 0x78, 0x16, 0xbf, 0x8f, 0x01, 0xcf, 0xea,
        0x41, 0x41, 0x40, 0xde, 0x5d, 0xae, 0x22, 0x23,
        0xb0, 0x03, 0x61, 0xa3, 0x96, 0x17, 0x7a, 0x9c,
        0xb4, 0x10, 0xff, 0x61, 0xf2, 0x00, 0x15, 0xad
    };
    static const uint8_t two_block_digest[SHA256_DIGEST_SIZE] = {
        0x24, 0x8d, 0x6a, 0x61, 0xd2, 0x06, 0x38, 0xb8,
        0xe5, 0xc0, 0x26, 0x93, 0x0c, 0x3e, 0x60, 0x39,
        0xa3, 0x3c, 0xe4, 0x59, 0x64, 0xff, 0x21, 0x67,
        0xf6, 0xec, 0xed, 0xd4, 0x19, 0xdb, 0x06, 0xc1
    };
    static const char two_block_message[] =
        "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";

    assert_digest((const uint8_t *)"", 0, empty_digest);
    assert_digest((const uint8_t *)"abc", 3, abc_digest);
    assert_digest((const uint8_t *)two_block_message,
                  strlen(two_block_message), two_block_digest);
}

static void load_entropy(MnemonicState *state,
                         const uint8_t entropy[MNEMONIC_ENTROPY_BYTES])
{
    uint16_t bit;

    mnemonic_state_init(state);
    for (bit = 0; bit < MNEMONIC_ENTROPY_BITS; bit++) {
        uint8_t value = (uint8_t)((entropy[bit / 8U] >>
                                  (7U - (bit % 8U))) & 1U);
        assert(mnemonic_state_add_flip(state, value) == 0);
    }
}

static void assert_mnemonic(const uint8_t entropy[MNEMONIC_ENTROPY_BYTES],
                            const char *const expected[MNEMONIC_WORD_COUNT])
{
    MnemonicState state;
    uint16_t index;
    uint8_t word;

    load_entropy(&state, entropy);
    for (word = 1; word <= MNEMONIC_DIRECT_WORDS; word++) {
        assert(mnemonic_state_get_word_index(&state, word, &index) == 0);
        assert(strcmp(bip39_get_word_by_index(index), expected[word - 1U]) == 0);
    }
    assert(mnemonic_state_get_final_word_index(&state, &index) == 0);
    assert(strcmp(bip39_get_word_by_index(index), expected[23]) == 0);
}

static void test_bip39_vectors(void)
{
    static const uint8_t zero_entropy[MNEMONIC_ENTROPY_BYTES] = { 0 };
    static const char *const zero_words[MNEMONIC_WORD_COUNT] = {
        "abandon", "abandon", "abandon", "abandon", "abandon", "abandon",
        "abandon", "abandon", "abandon", "abandon", "abandon", "abandon",
        "abandon", "abandon", "abandon", "abandon", "abandon", "abandon",
        "abandon", "abandon", "abandon", "abandon", "abandon", "art"
    };
    static const uint8_t varied_entropy[MNEMONIC_ENTROPY_BYTES] = {
        0x68, 0xa7, 0x9e, 0xac, 0xa2, 0x32, 0x48, 0x73,
        0xea, 0xcc, 0x50, 0xcb, 0x9c, 0x6e, 0xca, 0x8c,
        0xc6, 0x8e, 0xa5, 0xd9, 0x36, 0xf9, 0x87, 0x87,
        0xc6, 0x0c, 0x7e, 0xbc, 0x74, 0xe6, 0xce, 0x7c
    };
    static const char *const varied_words[MNEMONIC_WORD_COUNT] = {
        "hamster", "diagram", "private", "dutch", "cause", "delay",
        "private", "meat", "slide", "toddler", "razor", "book",
        "happy", "fancy", "gospel", "tennis", "maple", "dilemma",
        "loan", "word", "shrug", "inflict", "delay", "length"
    };

    assert_mnemonic(zero_entropy, zero_words);
    assert_mnemonic(varied_entropy, varied_words);
}

int main(void)
{
    test_initial_state_and_validation();
    test_word_completion_and_index();
    test_backspace_stops_at_current_word();
    test_backspace_reopens_completed_word();
    test_final_entropy_bits();
    test_secure_restart();
    test_sha256_vectors();
    test_bip39_vectors();

    puts("mnemonic_state tests passed");
    return 0;
}
