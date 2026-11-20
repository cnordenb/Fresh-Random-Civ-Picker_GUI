#pragma once

#include "framework.h"

#include "CivPickerModel.h"
#include "ParchmentDialog.h"
#include "ParchmentLog.h"

class CivPickerController;

enum class SoundType { button, tabsound, hover, view, error };

struct ShareLinks
{
    const wchar_t* pool;
    const wchar_t* round;
    int draw_count;
};

constexpr int MAX_TOOLTIP_TEXT = 256;
enum FixedTooltip
{
    TOOLTIP_DRAW,
    TOOLTIP_RESET,
    TOOLTIP_CLEAR,
    TOOLTIP_ENABLEALL,
    TOOLTIP_DISABLEALL,
    TOOLTIP_TECHTREE,
    TOOLTIP_REMAININGTOGGLE,
    TOOLTIP_AUTOTOGGLE,
    TOOLTIP_AUTORESET,
    TOOLTIP_OPTIONS,
    TOOLTIP_SURVAPP,
    TOOLTIP_HISTORY,
    TOOLTIP_SHARE,
    TOOLTIP_POOLSHARE,
    FIXED_TOOLTIP_COUNT
};
constexpr int EDITION_TOOLTIP_BASE = FIXED_TOOLTIP_COUNT;
constexpr int DLC_TOOLTIP_BASE = EDITION_TOOLTIP_BASE + MAX_EDITIONS;
constexpr int TOOLTIP_AMOUNT = DLC_TOOLTIP_BASE + MAX_DLCS;
constexpr int MENU_COUNT = 6;
constexpr int MAX_GAME_ITEMS = MAX_EDITIONS * MAX_EDITION_LINKS + 1;
constexpr int MAX_MENU_LABEL = 64;
constexpr int MAX_HOTKEY_TEXT = 96;
constexpr int MAX_EDITOR_PARTS = HOTKEY_SCOPE_COUNT + 2 * ParchmentDialogs::MAX_KEY_BUTTONS + 5;
constexpr int MAX_EDITOR_STATUS = 256;
constexpr int MAX_ABOUT_LEGAL = 512;
constexpr UINT_PTR TOAST_TIMER = 2;
constexpr int MAX_SHARE_NOTE = 128;
constexpr int MAX_PREVIEW_DECORATIONS = 8;
constexpr int MAX_PREVIEW_POOL = 768;
constexpr int MAX_PREVIEW_ROUND = 3072;

constexpr int MAX_LOG_TEXT = ParchmentLog::MAX_TEXT_LENGTH;
constexpr int MAX_HOTKEY_DECORATIONS = 160;
constexpr int MAX_MESSAGE_TEXT = 512;
constexpr int MAX_MESSAGE_TITLE = 64;
constexpr int ABOUT_DECORATIONS = 5;
constexpr int ABOUT_LINKS = 3;

enum class DrawnLogUpdate { count_only, entry, blank_line };

struct SoundResource
{
    HRSRC info;
    HGLOBAL handle;
    LPVOID data;
    LPCWSTR name;
};

struct PreviewFact
{
    const wchar_t* term;
    const wchar_t* text;
};

struct MappedJingle
{
    const void* data;
};

struct TooltipTarget { HWND control; int tooltip_id; };

struct DlcToggleSpot
{
    int left;
    int top;
    int size;
    int gap;
};

struct Tooltip
{
    wchar_t text[MAX_TOOLTIP_TEXT];
    bool external;
};

struct HotkeyItem
{
    HotkeyAction action;
    const wchar_t* key;
    const wchar_t* text;
};

struct HotkeyItemSpan
{
    const HotkeyItem* items;
    int count;
};

struct HotkeySection
{
    const wchar_t* title;
    HotkeyItemSpan items;
    float left;
    float top;
    float column_width;
    int rows;
};

struct EditorKeyEvent
{
    int row;
    WORD notification;
    HWND control;
};

struct EditorStatus
{
    wchar_t status[MAX_EDITOR_STATUS];
    wchar_t detail[MAX_EDITOR_STATUS];
};

namespace CivPickerViewDetail
{
    const wchar_t* const SECTION_TITLES[HOTKEY_SCOPE_COUNT] = { L"Global", L"Draw Civ tab", L"Log tab",
                                                                L"Civ Pool tab" };
}
