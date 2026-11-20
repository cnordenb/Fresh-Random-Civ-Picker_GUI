#pragma once

#include <cmath>
#include <cwchar>

#include "Assertions.h"
#include "ParchmentGeometry.h"

constexpr int BODY_LEFT = 27;
constexpr int BODY_TOP = 55;
constexpr int PANEL_PADDING = 13;
constexpr int BODY_PADDING_X = 14;
constexpr int BUTTON_ROW_PADDING = 14;
constexpr int BUTTON_GAP = 8;
constexpr int DIALOG_BUTTON_WIDTH = 75;
constexpr int DIALOG_BUTTON_HEIGHT = 23;
constexpr float PARAGRAPH_GAP = 8.0f;
constexpr float BODY_BOTTOM_PADDING = 4.0f;
constexpr float BUTTON_ROW_TOP_PADDING = 8.0f;

constexpr int DialogWidth(int body_width) { return body_width + 2 * (BODY_PADDING_X + PANEL_PADDING); }

inline int TextLength(const wchar_t* text) { return text != nullptr ? static_cast<int>(wcslen(text)) : 0; }

inline int Rounded(float value) { return static_cast<int>(std::lround(value)); }

inline PixelRect BottomRightButton(PixelSize dialog_size)
{
    const bool fits = dialog_size.width > DIALOG_BUTTON_WIDTH + 2 * PANEL_PADDING &&
                      dialog_size.height > DIALOG_BUTTON_HEIGHT + 2 * PANEL_PADDING;
    if (!c_assert(fits)) return { 0, 0, 0, 0 };
    const int right = dialog_size.width - PANEL_PADDING - BUTTON_ROW_PADDING;
    const int top = dialog_size.height - PANEL_PADDING - BUTTON_ROW_PADDING - DIALOG_BUTTON_HEIGHT;
    return { right - DIALOG_BUTTON_WIDTH, top, right, top + DIALOG_BUTTON_HEIGHT };
}

inline PixelRect ButtonLeftOf(const PixelRect& button)
{
    if (!c_assert(button.left >= DIALOG_BUTTON_WIDTH + BUTTON_GAP)) return button;
    const int right = button.left - BUTTON_GAP;
    return { right - DIALOG_BUTTON_WIDTH, button.top, right, button.bottom };
}

inline int HeightBelowBody(float body_text_bottom)
{
    if (!c_assert(body_text_bottom > 0.0f)) return 0;
    const float button_top = body_text_bottom + PARAGRAPH_GAP + BODY_BOTTOM_PADDING + BUTTON_ROW_TOP_PADDING;
    return Rounded(button_top) + DIALOG_BUTTON_HEIGHT + BUTTON_ROW_PADDING + PANEL_PADDING;
}
