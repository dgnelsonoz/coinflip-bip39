#ifndef MNEMONIC_UI_H
#define MNEMONIC_UI_H

#include "mnemonic_state.h"

#include <stdint.h>

typedef enum {
    MNEMONIC_UI_BUTTON_NONE = 0,
    MNEMONIC_UI_BUTTON_RESTART,
    MNEMONIC_UI_BUTTON_BACK,
    MNEMONIC_UI_BUTTON_ZERO,
    MNEMONIC_UI_BUTTON_ONE
} MnemonicUiButton;

void mnemonic_ui_draw(const MnemonicState *state);
void mnemonic_ui_update(const MnemonicState *state);
MnemonicUiButton mnemonic_ui_hit_test(uint16_t x, uint16_t y);
void mnemonic_ui_show_hold_progress(MnemonicUiButton button,
                                    uint32_t elapsed_ms,
                                    uint32_t required_ms);
void mnemonic_ui_clear_hold_progress(MnemonicUiButton button,
                                     int phrase_complete);

#endif
