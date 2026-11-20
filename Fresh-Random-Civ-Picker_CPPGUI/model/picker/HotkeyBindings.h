#pragma once

#include <cstdint>

#include "CivDataLimits.h"
#include "GlobalConstants.h"

// Part of the MODEL: the program's hotkeys, what each one does, where it works and the key it has, which the user
// can change (Options > Customise). A hotkey works on every tab or on one; a keystroke means at most one thing on
// each tab, so no two hotkeys that can work at the same time share it. Plain C++, unit-tested.

// Where a hotkey works, which is also its section of the Hotkeys dialog.
enum class HotkeyScope { global, draw_tab, log_tab, civ_pool_tab };

constexpr int HOTKEY_SCOPE_COUNT = 4;

// What a hotkey does: one for each line of the Hotkeys dialog, in its order (the global ones, then each tab's).
// The editions' hotkeys pick the editions by their place in data.json, the DLC slots' toggle the DLC in that place
// of the current edition's toggles (A to L by default).
enum class HotkeyAction
{
    cycle_tabs,
    show_draw_tab,
    show_log_tab,
    show_civ_pool_tab,
    open_options,
    open_hotkeys,
    open_about,
    open_tech_tree,
    quicksave,
    quickload,
    save_preset,
    load_preset,
    copy_share_link,
    open_share_link,
    join_lobby,
    undo,
    redraw,
    open_sustainability_app,
    toggle_civ_name,
    toggle_civ_icon,
    toggle_round_counter,
    toggle_civ_jingles,
    toggle_ui_sounds,
    quit,
    draw_tab_draw_civ,
    draw_tab_reset_round,
    draw_tab_open_options,
    draw_tab_open_sustainability_app,
    draw_tab_open_tech_tree,
    open_history,
    log_tab_draw_civ,
    log_tab_reset_round,
    clear_drawn_log,
    toggle_remaining_log,
    enable_all_civs,
    disable_all_civs,
    edition_1,
    edition_2,
    edition_3,
    toggle_auto_toggle,
    toggle_auto_reset,
    dlc_slot_1,
    dlc_slot_2,
    dlc_slot_3,
    dlc_slot_4,
    dlc_slot_5,
    dlc_slot_6,
    dlc_slot_7,
    dlc_slot_8,
    dlc_slot_9,
    none // no hotkey; also the number of the others
};

constexpr int HOTKEY_ACTION_COUNT = static_cast<int>(HotkeyAction::none);
static_assert(static_cast<int>(HotkeyAction::edition_3) - static_cast<int>(HotkeyAction::edition_1) + 1 ==
                  MAX_EDITIONS, "an edition hotkey for each edition the main window has room for");
static_assert(static_cast<int>(HotkeyAction::dlc_slot_9) - static_cast<int>(HotkeyAction::dlc_slot_1) + 1 ==
                  DLC_HOTKEY_COUNT, "a DLC slot hotkey for each DLC toggle that has one");

// A key and the modifier keys held with it, numbered as Windows numbers them for RegisterHotKey: a virtual-key
// code (0 for no key; letters and digits are their capital ASCII characters) and MOD_ALT, MOD_CONTROL and
// MOD_SHIFT. The controller checks that these numbers are Windows'.
struct Keystroke
{
    uint8_t modifiers;
    uint8_t key;
};

bool operator==(Keystroke left, Keystroke right);
bool operator!=(Keystroke left, Keystroke right);

constexpr uint8_t MODIFIER_ALT = 0x1;
constexpr uint8_t MODIFIER_CONTROL = 0x2;
constexpr uint8_t MODIFIER_SHIFT = 0x4;
constexpr Keystroke NO_KEYSTROKE = { 0, 0 };

constexpr uint8_t KEY_BACKSPACE = 0x08;
constexpr uint8_t KEY_TAB = 0x09;
constexpr uint8_t KEY_ENTER = 0x0D;
constexpr uint8_t KEY_SHIFT = 0x10;
constexpr uint8_t KEY_CONTROL = 0x11;
constexpr uint8_t KEY_ALT = 0x12;
constexpr uint8_t KEY_ESCAPE = 0x1B;
constexpr uint8_t KEY_SPACE = 0x20;
constexpr uint8_t KEY_PAGE_UP = 0x21;
constexpr uint8_t KEY_PAGE_DOWN = 0x22;
constexpr uint8_t KEY_END = 0x23;
constexpr uint8_t KEY_HOME = 0x24;
constexpr uint8_t KEY_LEFT = 0x25;
constexpr uint8_t KEY_UP = 0x26;
constexpr uint8_t KEY_RIGHT = 0x27;
constexpr uint8_t KEY_DOWN = 0x28;
constexpr uint8_t KEY_INSERT = 0x2D;
constexpr uint8_t KEY_DELETE = 0x2E;
constexpr uint8_t KEY_NUMPAD_0 = 0x60;
constexpr uint8_t KEY_NUMPAD_9 = 0x69;
constexpr uint8_t KEY_MULTIPLY = 0x6A;
constexpr uint8_t KEY_ADD = 0x6B;
constexpr uint8_t KEY_SUBTRACT = 0x6D;
constexpr uint8_t KEY_DECIMAL = 0x6E;
constexpr uint8_t KEY_DIVIDE = 0x6F;
constexpr uint8_t KEY_F1 = 0x70;
constexpr uint8_t KEY_F11 = 0x7A;
constexpr uint8_t KEY_F12 = 0x7B;

// A keystroke's name, as the program shows and saves it: "Ctrl+S", "Space", "Num 5"; "None" for no key, and empty
// for a key without a name.
struct KeystrokeName
{
    static constexpr int MAX_LENGTH = 31;
    wchar_t characters[MAX_LENGTH + 1];
};

struct ReadKeystroke
{
    bool ok;
    Keystroke keystroke;
};

// Whether a keystroke can be a hotkey: the key has a name here (letters, digits, F1 to F12, the editing and arrow
// keys, the number pad), and Windows doesn't keep it for itself (F12 for debuggers, Alt+Tab and the like).
enum class KeystrokeUse { usable, unnamed_key, kept_by_windows };

KeystrokeName NameKeystroke(Keystroke keystroke);
ReadKeystroke ParseKeystroke(const wchar_t* text);
KeystrokeUse CheckKeystroke(Keystroke keystroke);

// Which key each hotkey has. The editions' and DLC slots' hotkeys that data.json gives nothing to do are unused:
// they keep their default keys, take no key from the others, and do nothing.
class HotkeyBindings final
{
public:
    // A global keystroke can be in use on each of the three tabs: at most this many hotkeys lose it to another.
    static constexpr int MAX_CLASHES = 3;
    static constexpr int NO_PLACE = -1;

    // The hotkeys whose keystroke another hotkey would take from them.
    struct Clashes
    {
        HotkeyAction actions[MAX_CLASHES];
        int count;
    };

    HotkeyBindings(int edition_count, int most_dlc_toggles);

    static HotkeyScope ScopeOf(HotkeyAction action);
    static Keystroke DefaultKeystrokeOf(HotkeyAction action);
    static const wchar_t* SettingNameOf(HotkeyAction action);
    // An edition's hotkey's edition by its place in data.json, and a DLC slot's hotkey's place in an edition's
    // toggles; NO_PLACE for the other hotkeys. The hotkeys in those places, and none past the last.
    static int EditionOf(HotkeyAction action);
    static int DlcSlotOf(HotkeyAction action);
    static HotkeyAction EditionHotkey(int edition);
    static HotkeyAction DlcSlotHotkey(int slot);

    bool IsUsed(HotkeyAction action) const;
    Keystroke KeystrokeOf(HotkeyAction action) const;
    HotkeyAction ActionFor(Keystroke keystroke, HotkeyScope tab) const;
    Clashes ClashesWith(HotkeyAction action, Keystroke keystroke) const;

    bool Assign(HotkeyAction action, Keystroke keystroke);
    void Clear(HotkeyAction action);
    void RestoreDefaults();

private:
    Keystroke keystrokes[HOTKEY_ACTION_COUNT] = {};
    int used_editions = 0;
    int used_dlc_slots = 0;
};
