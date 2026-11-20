#pragma once

#include <cstdint>

enum class ImageId
{
    background,
    text_button,
    text_button_pressed,
    icon_button_frame,
    icon_button_frame_pressed,
    checkbox,
    dropdown_button,
    external_icon,
    external_icon_dark,
    external_icon_tooltip,
    log_frame,
    off_cross,
    options_button,
    panel_inlay,
    scrollbar,
    scrollbar_knob,
    slider_bar,
    slider_progress,
    slider_left_arrow,
    slider_left_arrow_hover,
    slider_left_arrow_active,
    slider_right_arrow,
    slider_right_arrow_hover,
    slider_right_arrow_active,
    slider_handle,
    slider_handle_hover,
    slider_handle_active,
    tabs,
    tabs_dimmed,
    techtree_button,
    text_field_frame,
    close_button,
    share_icon,
    program_icon,
    civ_atlas,
    civ_atlas_grey,
    dlc_atlas,
    dlc_atlas_grey,
    edition_banners,
    history_book,
    aoe2vils_icon,
    count
};

constexpr int IMAGE_COUNT = static_cast<int>(ImageId::count);

enum class NineSliceId
{
    log_frame,
    icon_button_frame,
    icon_button_frame_pressed,
    panel_frame,
    text_field_frame,
    count
};

constexpr int NINE_SLICE_COUNT = static_cast<int>(NineSliceId::count);

enum class CursorId { arrow, hand, count };

constexpr int CURSOR_COUNT = static_cast<int>(CursorId::count);

enum class CursorSize { standard, large, count };

constexpr int CURSOR_SIZE_COUNT = static_cast<int>(CursorSize::count);
constexpr unsigned int LARGE_CURSOR_DPI = 144;

enum class FontId
{
    ui,
    ui_bold,
    ui_italic,
    civ_name,
    button,
    key,
    count
};

constexpr int FONT_COUNT = static_cast<int>(FontId::count);

enum class TextAlign { left, centre, right };

struct Colour
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    float alpha;
};

constexpr Colour TEXT_COLOUR = { 0, 0, 0, 1.0f };
constexpr Colour BUTTON_TEXT_COLOUR = { 0xf1, 0xde, 0xae, 1.0f };
constexpr Colour BUTTON_TEXT_HOVER_COLOUR = { 0xff, 0xf8, 0xe0, 1.0f };
constexpr Colour MENU_COLOUR = { 0x65, 0x52, 0x3b, 1.0f };
constexpr Colour MENU_BORDER_COLOUR = { 0x3d, 0x2c, 0x1c, 1.0f };
constexpr Colour MENU_TEXT_COLOUR = { 0xf1, 0xde, 0xae, 1.0f };
constexpr Colour MENU_SEPARATOR_COLOUR = { 0xf1, 0xde, 0xae, 0.3f };
constexpr Colour HOVER_GOLD_COLOUR = { 0xdc, 0xcb, 0x70, 1.0f };
constexpr Colour HOVER_RED_COLOUR = { 0xdc, 0x6a, 0x5c, 1.0f };
constexpr Colour LOG_COLOUR = { 0x61, 0x50, 0x35, 1.0f };
constexpr Colour LOG_TEXT_COLOUR = { 0xd9, 0xc6, 0x9d, 1.0f };
constexpr Colour LOG_SELECTION_COLOUR = { 0xd9, 0xc6, 0x9d, 0.35f };
constexpr Colour TOOLTIP_COLOUR = { 0xd7, 0xb3, 0x94, 1.0f };
constexpr Colour TOOLTIP_TEXT_COLOUR = { 0x39, 0x1c, 0x1b, 1.0f };
constexpr Colour TOOLTIP_BORDER_COLOUR = { 0x76, 0x76, 0x76, 1.0f };
constexpr Colour TOAST_COLOUR = { 0x2b, 0x2b, 0x2b, 1.0f };
constexpr Colour TOAST_TEXT_COLOUR = { 0xff, 0xff, 0xff, 1.0f };
constexpr Colour CLIENT_FRAME_COLOUR = { 90, 60, 30, 0.35f };
constexpr Colour GROUP_FRAME_COLOUR = { 60, 40, 20, 0.35f };
constexpr Colour HINT_COLOUR = { 0x3d, 0x2c, 0x1c, 1.0f };
constexpr Colour LINK_COLOUR = { 0x08, 0x30, 0x6a, 1.0f };
constexpr Colour ERROR_COLOUR = { 0x68, 0x0a, 0x0a, 1.0f };
constexpr Colour FIELD_COLOUR = { 0x47, 0x47, 0x47, 1.0f };
constexpr Colour FIELD_TEXT_COLOUR = { 0xff, 0xff, 0xff, 1.0f };
constexpr Colour FIELD_PLACEHOLDER_COLOUR = { 0xff, 0xff, 0xff, 0.45f };
constexpr Colour KEY_COLOUR = { 0xd7, 0xb3, 0x94, 1.0f };
constexpr Colour KEY_BORDER_COLOUR = { 0xb4, 0xb4, 0xb4, 1.0f };
constexpr Colour FOCUS_COLOUR = { 0, 0, 0, 1.0f };
constexpr Colour BUTTON_FOCUS_COLOUR = { 0xf1, 0xde, 0xae, 1.0f };
constexpr Colour CONTENT_UNDERLAY_COLOUR = { 0, 0, 0, 1.0f };

constexpr float UI_LINE_HEIGHT = 16.1875f;
constexpr float CIV_NAME_LINE_HEIGHT = 17.546875f;
constexpr float DIALOG_LINE_HEIGHT = 16.796875f;
constexpr float KEY_LINE_HEIGHT = 15.390625f;

constexpr int BACKGROUND_VISIBLE_LEFT = 26;
constexpr int BACKGROUND_VISIBLE_TOP = 13;
constexpr int BACKGROUND_VISIBLE_RIGHT = 995;
constexpr int BACKGROUND_VISIBLE_BOTTOM = 1054;
constexpr int BACKGROUND_VISIBLE_WIDTH = BACKGROUND_VISIBLE_RIGHT - BACKGROUND_VISIBLE_LEFT;
constexpr int BACKGROUND_VISIBLE_HEIGHT = BACKGROUND_VISIBLE_BOTTOM - BACKGROUND_VISIBLE_TOP;
constexpr int WINDOW_WIDTH = 578;
constexpr int WINDOW_HEIGHT =
    (WINDOW_WIDTH * BACKGROUND_VISIBLE_HEIGHT + BACKGROUND_VISIBLE_WIDTH / 2) / BACKGROUND_VISIBLE_WIDTH;
constexpr int WINDOW_PADDING = 22;
constexpr int TITLE_BAR_HEIGHT = 30;
constexpr int MENU_BAR_HEIGHT = 20;
constexpr int CLIENT_LEFT = WINDOW_PADDING;
constexpr int CLIENT_TOP = WINDOW_PADDING + TITLE_BAR_HEIGHT + MENU_BAR_HEIGHT;
constexpr int CLIENT_WIDTH = 534;
constexpr int CLIENT_HEIGHT = 511;
constexpr int CLIENT_FRAME_TOP = 21;
constexpr int TITLE_TEXT_LEFT = 10;
constexpr int TITLE_ICON_SIZE = 16;
constexpr int TITLE_ICON_GAP = 6;
constexpr int CLOSE_BUTTON_WIDTH = 46;
constexpr int CLOSE_BUTTON_IMAGE_SIZE = 17;

constexpr int CIV_ATLAS_COLUMNS = 8;
constexpr int CIV_ATLAS_CELL = 104;
constexpr int DLC_ATLAS_COLUMNS = 6;
constexpr int DLC_ATLAS_CELL = 128;
constexpr int EDITION_LOOKS = 3;
constexpr int EDITION_BANNER_WIDTH = 128;
constexpr int EDITION_BANNER_HEIGHT = 93;
constexpr int OPTIONS_BUTTON_CELL = 346;
constexpr int TECHTREE_BUTTON_CELL = 122;
constexpr int CHECKBOX_CELL = 78;
constexpr int CHECKBOX_SIZE = 16;
constexpr int TAB_CELL_WIDTH = 329;
constexpr int TAB_CELL_HEIGHT = 158;
constexpr int SCROLLBAR_SLICE = 27;
constexpr int SLIDER_BAR_IMAGE_WIDTH = 386;
constexpr int SLIDER_BAR_IMAGE_HEIGHT = 15;
constexpr int SLIDER_BAR_IMAGE_CAP = 4;
constexpr int SLIDER_ARROW_IMAGE_WIDTH = 35;
constexpr int SLIDER_ARROW_IMAGE_HEIGHT = 30;
constexpr int SLIDER_HANDLE_IMAGE = 43;
constexpr int DROPDOWN_BUTTON_CELL_WIDTH = 56;
constexpr int DROPDOWN_BUTTON_CELL_HEIGHT = 78;
constexpr int EXTERNAL_ICON_SIZE = 12;
