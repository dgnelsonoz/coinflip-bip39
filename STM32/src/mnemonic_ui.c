#include "mnemonic_ui.h"

#include "bip39_lookup.h"
#include "fonts.h"
#include "stm32469i_discovery_lcd.h"

#include <stdint.h>
#include <stdio.h>

enum {
    DISPLAY_WIDTH = 800,
    TITLE_HEIGHT = 32,
    WORD_GRID_TOP = TITLE_HEIGHT,
    WORD_GRID_HEIGHT = 216,
    STATUS_TOP = WORD_GRID_TOP + WORD_GRID_HEIGHT,
    STATUS_HEIGHT = 80,
    BUTTON_TOP = STATUS_TOP + STATUS_HEIGHT,
    BUTTON_HEIGHT = 152,
    WORD_COLUMN_WIDTH = 200,
    WORD_ROW_HEIGHT = 36,
    RESTART_WIDTH = 130,
    BACK_WIDTH = 130,
    BIT_BUTTON_WIDTH = 270
};

typedef char button_widths_must_fill_display[
    (RESTART_WIDTH + BACK_WIDTH + (2 * BIT_BUTTON_WIDTH) == DISPLAY_WIDTH)
    ? 1 : -1];
typedef char vertical_regions_must_fill_display[
    (TITLE_HEIGHT + WORD_GRID_HEIGHT + STATUS_HEIGHT + BUTTON_HEIGHT == 480)
    ? 1 : -1];
typedef char word_rows_must_fill_grid[
    (6 * WORD_ROW_HEIGHT == WORD_GRID_HEIGHT) ? 1 : -1];

static uint8_t selected_word;

static void display_text(uint16_t x, uint16_t y, const char *text)
{
    BSP_LCD_DisplayStringAt(x, y, (uint8_t *)text, LEFT_MODE);
}

static void draw_title(void)
{
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_FillRect(0, 0, DISPLAY_WIDTH, TITLE_HEIGHT);
    BSP_LCD_SetFont(&Font20);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
    BSP_LCD_DisplayStringAt(0, 6, (uint8_t *)"COIN FLIPS TO BIP-39",
                            CENTER_MODE);
    BSP_LCD_DrawHLine(0, TITLE_HEIGHT - 1U, DISPLAY_WIDTH);
}

static void get_word_cell(uint8_t word_number, uint16_t *x, uint16_t *y)
{
    uint8_t zero_based = (uint8_t)(word_number - 1U);

    *x = (uint16_t)(zero_based / 6U) * WORD_COLUMN_WIDTH;
    *y = WORD_GRID_TOP + (uint16_t)(zero_based % 6U) * WORD_ROW_HEIGHT;
}

static void draw_grid_lines(void)
{
    uint8_t column;
    uint8_t row;

    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    for (column = 1; column < 4; column++) {
        BSP_LCD_DrawVLine((uint16_t)column * WORD_COLUMN_WIDTH,
                          WORD_GRID_TOP, WORD_GRID_HEIGHT);
    }
    for (row = 1; row <= 6; row++) {
        BSP_LCD_DrawHLine(0, WORD_GRID_TOP +
                          (uint16_t)row * WORD_ROW_HEIGHT - 1U,
                          DISPLAY_WIDTH);
    }
}

static void format_partial_bits(const MnemonicState *state, char *bits,
                                uint8_t target_bits)
{
    uint8_t entered = mnemonic_state_get_current_word_bit_count(state);
    uint16_t start = (uint16_t)(state->bit_count - entered);
    uint8_t ii;

    for (ii = 0; ii < target_bits; ii++) {
        if (ii < entered) {
            uint16_t position = start + ii;
            uint8_t value = (uint8_t)((state->entropy[position / 8U] >>
                                      (7U - (position % 8U))) & 1U);
            bits[ii] = value != 0U ? '1' : '0';
        } else {
            bits[ii] = '-';
        }
    }
    bits[target_bits] = '\0';
}

static void format_index_bits(uint16_t index, char *bits, int checksum_word)
{
    uint8_t source_bit;
    uint8_t output = 0;

    for (source_bit = 0; source_bit < MNEMONIC_WORD_BITS; source_bit++) {
        if (checksum_word && source_bit == 3U) {
            bits[output++] = '|';
        }
        bits[output++] = (index & (1U << (10U - source_bit))) != 0U
                         ? '1' : '0';
    }
    bits[output] = '\0';
}

static void draw_word_cells(const MnemonicState *state)
{
    char entry[24];
    uint8_t word_number;

    for (word_number = 1; word_number <= MNEMONIC_WORD_COUNT; word_number++) {
        uint16_t x;
        uint16_t y;
        uint16_t index;
        int has_word;

        get_word_cell(word_number, &x, &y);
        BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
        BSP_LCD_FillRect(x + 1U, y, WORD_COLUMN_WIDTH - 1U,
                         WORD_ROW_HEIGHT - 1U);

        if (word_number <= MNEMONIC_DIRECT_WORDS) {
            has_word = mnemonic_state_get_word_index(state, word_number,
                                                     &index) == 0;
        } else {
            has_word = mnemonic_state_get_final_word_index(state, &index) == 0;
        }

        if (has_word) {
            snprintf(entry, sizeof(entry), "%02u %-8s %04u",
                     (unsigned int)word_number,
                     bip39_get_word_by_index(index),
                     (unsigned int)index + 1U);
        } else if (word_number == mnemonic_state_get_current_word_number(state)) {
            const char *state_label =
                mnemonic_state_get_current_word_bit_count(state) == 0U
                ? "ready" : "in progress";
            snprintf(entry, sizeof(entry), "%02u [%s]",
                     (unsigned int)word_number, state_label);
        } else if (word_number == 24U) {
            snprintf(entry, sizeof(entry), "%02u [checksum]",
                     (unsigned int)word_number);
        } else {
            snprintf(entry, sizeof(entry), "%02u",
                     (unsigned int)word_number);
        }

        BSP_LCD_SetFont(&Font16);
        BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
        BSP_LCD_SetTextColor(word_number ==
                            mnemonic_state_get_current_word_number(state) &&
                            !mnemonic_state_entropy_complete(state)
                            ? LCD_COLOR_CYAN : LCD_COLOR_WHITE);
        display_text(x + 10U, y + 12U, entry);

        if (word_number == selected_word) {
            BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
            BSP_LCD_DrawRect(x + 2U, y + 2U,
                             WORD_COLUMN_WIDTH - 4U, WORD_ROW_HEIGHT - 4U);
        }
    }
}

static void draw_status(const MnemonicState *state)
{
    char status[64];
    char bits[MNEMONIC_WORD_BITS + 1];
    char verification[72];
    char verification_bits[MNEMONIC_WORD_BITS + 2];
    uint8_t word_number = mnemonic_state_get_current_word_number(state);
    uint8_t entered = mnemonic_state_get_current_word_bit_count(state);
    uint8_t required = word_number == 24U ? 3U : MNEMONIC_WORD_BITS;
    uint8_t completed = mnemonic_state_get_completed_word_count(state);
    uint8_t detail_word;
    uint16_t detail_index;

    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_FillRect(0, STATUS_TOP, DISPLAY_WIDTH, STATUS_HEIGHT);

    BSP_LCD_SetFont(&Font20);
    BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
    if (mnemonic_state_entropy_complete(state)) {
        BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
        BSP_LCD_DisplayStringAt(0, STATUS_TOP + 8U,
                                (uint8_t *)"PHRASE COMPLETE - 24 WORDS",
                                CENTER_MODE);
        completed = MNEMONIC_WORD_COUNT;
    } else {
        format_partial_bits(state, bits, required);
        snprintf(status, sizeof(status),
                 "WORD %02u/24   FLIP %02u/%02u   BITS: %s",
                 (unsigned int)word_number, (unsigned int)entered,
                 (unsigned int)required, bits);
        BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
        display_text(20, STATUS_TOP + 8U, status);
    }

    if (completed > 0U) {
        detail_word = selected_word != 0U ? selected_word : completed;
        if (detail_word == MNEMONIC_WORD_COUNT) {
            mnemonic_state_get_final_word_index(state, &detail_index);
        } else {
            mnemonic_state_get_word_index(state, detail_word, &detail_index);
        }
        format_index_bits(detail_index, verification_bits,
                          detail_word == MNEMONIC_WORD_COUNT);
        snprintf(verification, sizeof(verification),
                 "WORD %02u: %s = INDEX %04u = LIST %04u = %s",
                 (unsigned int)detail_word, verification_bits,
                 (unsigned int)detail_index,
                 (unsigned int)detail_index + 1U,
                 bip39_get_word_by_index(detail_index));
        BSP_LCD_SetFont(&Font16);
        BSP_LCD_SetTextColor(LCD_COLOR_LIGHTGRAY);
        display_text(20, STATUS_TOP + 48U, verification);
    }
}

static void fill_button(uint16_t x, uint16_t width, uint32_t color)
{
    BSP_LCD_SetTextColor(color);
    BSP_LCD_FillRect(x, BUTTON_TOP, width, BUTTON_HEIGHT);
}

static uint32_t button_color(MnemonicUiButton button, int phrase_complete)
{
    if (phrase_complete && button != MNEMONIC_UI_BUTTON_RESTART) {
        return LCD_COLOR_GRAY;
    }

    switch (button) {
    case MNEMONIC_UI_BUTTON_RESTART:
        return LCD_COLOR_DARKRED;
    case MNEMONIC_UI_BUTTON_BACK:
        return LCD_COLOR_ORANGE;
    case MNEMONIC_UI_BUTTON_ZERO:
        return LCD_COLOR_LIGHTGRAY;
    case MNEMONIC_UI_BUTTON_ONE:
        return LCD_COLOR_DARKGRAY;
    default:
        return LCD_COLOR_BLACK;
    }
}

static void button_bounds(MnemonicUiButton button, uint16_t *x, uint16_t *width)
{
    switch (button) {
    case MNEMONIC_UI_BUTTON_RESTART:
        *x = 0;
        *width = RESTART_WIDTH;
        break;
    case MNEMONIC_UI_BUTTON_BACK:
        *x = RESTART_WIDTH;
        *width = BACK_WIDTH;
        break;
    case MNEMONIC_UI_BUTTON_ZERO:
        *x = RESTART_WIDTH + BACK_WIDTH;
        *width = BIT_BUTTON_WIDTH;
        break;
    case MNEMONIC_UI_BUTTON_ONE:
        *x = RESTART_WIDTH + BACK_WIDTH + BIT_BUTTON_WIDTH;
        *width = BIT_BUTTON_WIDTH;
        break;
    default:
        *x = 0;
        *width = 0;
        break;
    }
}

static void draw_buttons(int phrase_complete)
{
    uint16_t back_x = RESTART_WIDTH;
    uint16_t zero_x = RESTART_WIDTH + BACK_WIDTH;
    uint16_t one_x = zero_x + BIT_BUTTON_WIDTH;

    fill_button(0, RESTART_WIDTH,
                button_color(MNEMONIC_UI_BUTTON_RESTART, phrase_complete));
    fill_button(back_x, BACK_WIDTH,
                button_color(MNEMONIC_UI_BUTTON_BACK, phrase_complete));
    fill_button(zero_x, BIT_BUTTON_WIDTH,
                button_color(MNEMONIC_UI_BUTTON_ZERO, phrase_complete));
    fill_button(one_x, BIT_BUTTON_WIDTH,
                button_color(MNEMONIC_UI_BUTTON_ONE, phrase_complete));

    BSP_LCD_SetTextColor(phrase_complete ? LCD_COLOR_DARKGRAY : LCD_COLOR_BLACK);
    BSP_LCD_DrawVLine(back_x, BUTTON_TOP, BUTTON_HEIGHT);
    BSP_LCD_DrawVLine(zero_x, BUTTON_TOP, BUTTON_HEIGHT);
    BSP_LCD_DrawVLine(one_x, BUTTON_TOP, BUTTON_HEIGHT);

    BSP_LCD_SetFont(&Font16);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_SetBackColor(LCD_COLOR_DARKRED);
    display_text(10, 350, "HOLD 2 SEC");
    BSP_LCD_SetFont(&Font20);
    display_text(16, 395, "RESTART");

    BSP_LCD_SetFont(&Font16);
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_SetBackColor(button_color(MNEMONIC_UI_BUTTON_BACK,
                                     phrase_complete));
    display_text(173, 350, "HOLD");
    BSP_LCD_SetFont(&Font20);
    display_text(165, 395, "BACK");

    BSP_LCD_SetFont(&Font20);
    BSP_LCD_SetTextColor(phrase_complete ? LCD_COLOR_DARKGRAY : LCD_COLOR_BLACK);
    BSP_LCD_SetBackColor(button_color(MNEMONIC_UI_BUTTON_ZERO,
                                     phrase_complete));
    display_text(360, 350, "HEADS");
    BSP_LCD_SetFont(&Font24);
    display_text(386, 395, "0");

    BSP_LCD_SetFont(&Font20);
    BSP_LCD_SetTextColor(phrase_complete ? LCD_COLOR_DARKGRAY : LCD_COLOR_WHITE);
    BSP_LCD_SetBackColor(button_color(MNEMONIC_UI_BUTTON_ONE,
                                     phrase_complete));
    display_text(630, 350, "TAILS");
    BSP_LCD_SetFont(&Font24);
    display_text(656, 395, "1");
}

void mnemonic_ui_draw(const MnemonicState *state)
{
    selected_word = 0U;
    BSP_LCD_Clear(LCD_COLOR_BLACK);
    draw_title();
    draw_grid_lines();
    draw_word_cells(state);
    draw_status(state);
    draw_buttons(mnemonic_state_entropy_complete(state));
}

void mnemonic_ui_update(const MnemonicState *state)
{
    selected_word = 0U;
    draw_word_cells(state);
    draw_status(state);
    draw_buttons(mnemonic_state_entropy_complete(state));
}

void mnemonic_ui_draw_error(const char *message)
{
    BSP_LCD_Clear(LCD_COLOR_BLACK);
    BSP_LCD_SetFont(&Font24);
    BSP_LCD_SetTextColor(LCD_COLOR_RED);
    BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
    BSP_LCD_DisplayStringAt(0, 190, (uint8_t *)"HARDWARE ERROR",
                            CENTER_MODE);
    BSP_LCD_SetFont(&Font20);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_DisplayStringAt(0, 240, (uint8_t *)message, CENTER_MODE);
    BSP_LCD_DisplayStringAt(0, 275, (uint8_t *)"CHECK POWER AND RESTART",
                            CENTER_MODE);
}

MnemonicUiButton mnemonic_ui_hit_test(uint16_t x, uint16_t y)
{
    if (y < BUTTON_TOP || y >= BUTTON_TOP + BUTTON_HEIGHT || x >= DISPLAY_WIDTH) {
        return MNEMONIC_UI_BUTTON_NONE;
    }
    if (x < RESTART_WIDTH) {
        return MNEMONIC_UI_BUTTON_RESTART;
    }
    if (x < RESTART_WIDTH + BACK_WIDTH) {
        return MNEMONIC_UI_BUTTON_BACK;
    }
    if (x < RESTART_WIDTH + BACK_WIDTH + BIT_BUTTON_WIDTH) {
        return MNEMONIC_UI_BUTTON_ZERO;
    }
    return MNEMONIC_UI_BUTTON_ONE;
}

int mnemonic_ui_select_word_at(const MnemonicState *state,
                               uint16_t x, uint16_t y)
{
    uint8_t column;
    uint8_t row;
    uint8_t word_number;
    uint8_t completed;

    if (state == NULL || x >= DISPLAY_WIDTH || y < WORD_GRID_TOP ||
        y >= WORD_GRID_TOP + WORD_GRID_HEIGHT) {
        return 0;
    }

    column = (uint8_t)(x / WORD_COLUMN_WIDTH);
    row = (uint8_t)((y - WORD_GRID_TOP) / WORD_ROW_HEIGHT);
    word_number = (uint8_t)(column * 6U + row + 1U);
    completed = mnemonic_state_get_completed_word_count(state);
    if (mnemonic_state_entropy_complete(state)) {
        completed = MNEMONIC_WORD_COUNT;
    }
    if (word_number > completed) {
        return 0;
    }

    selected_word = word_number;
    draw_word_cells(state);
    draw_status(state);
    return 1;
}

void mnemonic_ui_show_hold_progress(MnemonicUiButton button,
                                    uint32_t elapsed_ms,
                                    uint32_t required_ms)
{
    uint16_t x;
    uint16_t width;
    uint16_t progress;

    button_bounds(button, &x, &width);
    if (width == 0U || required_ms == 0U) {
        return;
    }
    if (elapsed_ms > required_ms) {
        elapsed_ms = required_ms;
    }
    progress = (uint16_t)(((uint32_t)(width - 1U) * elapsed_ms) / required_ms);

    BSP_LCD_SetTextColor(button_color(button, 0));
    BSP_LCD_FillRect(x + 1U, BUTTON_TOP, width - 1U, 10U);
    if (progress > 0U) {
        BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
        BSP_LCD_FillRect(x + 1U, BUTTON_TOP, progress, 10U);
    }
}

void mnemonic_ui_clear_hold_progress(MnemonicUiButton button,
                                     int phrase_complete)
{
    uint16_t x;
    uint16_t width;

    button_bounds(button, &x, &width);
    if (width == 0U) {
        return;
    }
    BSP_LCD_SetTextColor(button_color(button, phrase_complete));
    BSP_LCD_FillRect(x + 1U, BUTTON_TOP, width - 1U, 10U);
}
