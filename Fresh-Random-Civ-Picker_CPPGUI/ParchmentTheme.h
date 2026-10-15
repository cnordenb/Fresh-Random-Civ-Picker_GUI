#pragma once

#include <cstdint>

// The parchment theme of the web version (frcp.net, style.css), which this program shares: the images, fonts,
// colours and sizes every View component draws with. Values carry the CSS they come from.

// The images are the game's originals where the program has them (the parchment, the cursors, the slider and the
// dialogs' panel) and the web version's otherwise. The atlases are built at startup from one file per icon, which
// data.json names: a civ's is its name in lower case, an edition's and a DLC's its "icon" (and "banner").
enum class ImageId
{
    background, // ui/background.png, the parchment the whole window is drawn on, at its own aspect ratio
    text_button, // ui/button.png
    text_button_pressed, // ui/button_pressed.png
    icon_button_frame, // ui/button_9slice.png, 6 px nine-slice
    icon_button_frame_pressed,
    checkbox, // ui/checkbox.png: unticked over ticked, normal left and pressed right
    dropdown_button, // ui/dropdown_button.png: normal and pressed side by side
    external_icon, // ui/external.png in the menus' cream
    external_icon_dark, // the same, filtered brightness(0) for a hovered menu item
    external_icon_tooltip, // the same, filtered to the tooltip's text colour
    log_frame, // ui/log_9slice.png, 6 px nine-slice
    off_cross, // ui/off.png, drawn over a disabled civ or DLC
    options_button, // ui/options_button.png: normal, hover and pressed side by side
    panel_inlay, // ui/9slice/boxstyle3a_inlay.png, the middle of the dialogs' panel
    scrollbar, // ui/scrollbar_3slice.png: up button, track and down button
    scrollbar_knob, // ui/scrollbar_knob.png
    slider_bar, // ui/slider/slider_bar.png, the slider's groove
    slider_progress, // ui/slider/slider_bar_progress.png, the groove's fill up to the handle
    slider_left_arrow, // ui/slider/arrow_left_normal.png
    slider_left_arrow_hover, // ui/slider/arrow_left_hover.png
    slider_left_arrow_active, // ui/slider/arrow_left_active.png, while pressed
    slider_right_arrow, // ui/slider/arrow_right_normal.png
    slider_right_arrow_hover,
    slider_right_arrow_active,
    slider_handle, // ui/slider/slider_handle_normal.png
    slider_handle_hover,
    slider_handle_active, // while dragged
    tabs, // ui/tabs.png: tab 1 over tab 2, normal left and hover right
    tabs_dimmed, // the same, filtered brightness(.88) for the tabs that aren't selected
    techtree_button, // ui/techtree_button.png: normal and pressed side by side
    text_field_frame, // ui/textfield_9slice.png, 15 px nine-slice
    close_button, // ui/x_button.png: normal and pressed side by side
    share_icon, // ui/share_icon.png, the share buttons' icon
    program_icon, // ui/frcp.png, the program's icon (500 px), drawn small before the title (desktop only)
    civ_atlas, // civ/<civ>.png in the order of the civs' IDs, then civ/random.png: 104 px cells, 8 per row
    civ_atlas_grey, // the same, filtered grayscale(1) for disabled civs
    dlc_atlas, // dlc/<icon>.png: each edition's three looks, then the DLCs' icons; 128 px cells, 6 per row
    dlc_atlas_grey,
    edition_banners, // dlc/<banner>.png: 128 x 93 banners in data.json's order of editions
    history_book, // history.bmp, the History button (desktop only)
    aoe2vils_icon, // aoe2vils.png, 12 px pixel art for the aoe2vils.net button (desktop only)
    count
};

constexpr int IMAGE_COUNT = static_cast<int>(ImageId::count);

// The nine-slice images and their slice size in the image.
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

// The cursors come in 32 px (ui/cursor_x1.png, ui/cursor_pointing_x1.png) and, for displays scaled to 150 % or
// more, 48 px (the _x2 files).
enum class CursorSize { standard, large, count };

constexpr int CURSOR_SIZE_COUNT = static_cast<int>(CursorSize::count);
constexpr unsigned int LARGE_CURSOR_DPI = 144;

enum class FontId
{
    ui, // Georgia 12 px: labels, menus, logs, tooltips, dialogs
    ui_bold, // Georgia 12 px, weight 600
    ui_italic, // Georgia 12 px italic: disabled civ and DLC names
    civ_name, // Georgia 13 px, weight 600: the drawn civ's name
    button, // 14 px blackletter: the wooden text buttons
    key, // Georgia 11 px: the keys in the Hotkeys dialog
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

// --- Colours (style.css, parchment theme) ---

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
constexpr Colour TOAST_COLOUR = { 0x2b, 0x2b, 0x2b, 1.0f }; // #toast
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

// --- Line heights (CSS line-height), in the browser's layout units of 1/64 px ---

constexpr float UI_LINE_HEIGHT = 16.1875f; // 12px/1.35 = 16.2 px
constexpr float CIV_NAME_LINE_HEIGHT = 17.546875f; // 13px/1.35 = 17.55 px
constexpr float DIALOG_LINE_HEIGHT = 16.796875f; // 12px/1.4 = 16.8 px
constexpr float KEY_LINE_HEIGHT = 15.390625f; // 11px/1.4 = 15.4 px

// --- The main window (.window, .titlebar, .menubar, .client) ---

// The parchment is never stretched: the visible part of ui/background.png (1024 x 1167; the paper and its burnt edge
// without the transparent margin around them) fills the window's width, and the window is as tall as that makes it.
constexpr int BACKGROUND_VISIBLE_LEFT = 26;
constexpr int BACKGROUND_VISIBLE_TOP = 13;
constexpr int BACKGROUND_VISIBLE_RIGHT = 995;
constexpr int BACKGROUND_VISIBLE_BOTTOM = 1151;
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
constexpr int CLIENT_FRAME_TOP = 21; // .frame { top: 21px }
constexpr int TITLE_TEXT_LEFT = 10; // .titlebar { padding-left: 10px }
// Desktop only: the program's small icon before the title, as in a window caption; the title follows it.
constexpr int TITLE_ICON_SIZE = 16;
constexpr int TITLE_ICON_GAP = 6;
constexpr int CLOSE_BUTTON_WIDTH = 46;
constexpr int CLOSE_BUTTON_IMAGE_SIZE = 17;

// --- Image geometry ---

constexpr int CIV_ATLAS_COLUMNS = 8;
constexpr int CIV_ATLAS_CELL = 104;
constexpr int DLC_ATLAS_COLUMNS = 6;
constexpr int DLC_ATLAS_CELL = 128;
// The DLC atlas holds three looks (normal, hover, pressed) of each edition's icon, then the DLCs' icons.
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
constexpr int SLIDER_BAR_IMAGE_CAP = 4; // the bar's frame at each end, which isn't stretched with the bar
constexpr int SLIDER_ARROW_IMAGE_WIDTH = 35;
constexpr int SLIDER_ARROW_IMAGE_HEIGHT = 30;
constexpr int SLIDER_HANDLE_IMAGE = 43;
constexpr int DROPDOWN_BUTTON_CELL_WIDTH = 56;
constexpr int DROPDOWN_BUTTON_CELL_HEIGHT = 78;
constexpr int EXTERNAL_ICON_SIZE = 12;
