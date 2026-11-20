#include "HotkeyBindings.h"

#include <algorithm>
#include <cwchar>

#include "Assertions.h"

namespace
{
    struct HotkeyDefinition
    {
        HotkeyAction action;
        HotkeyScope scope;
        Keystroke default_keystroke;
        const wchar_t* setting_name; // its key in settings.ini
    };

    constexpr Keystroke Plain(uint8_t key) { return { 0, key }; }

    constexpr Keystroke WithControl(uint8_t key) { return { MODIFIER_CONTROL, key }; }

    constexpr uint8_t FunctionKey(int number) { return static_cast<uint8_t>(KEY_F1 + number - 1); }

    // The keys the program always had, global ones first, then the Draw Civ, Log and Civ Pool tabs'.
    constexpr HotkeyDefinition DEFINITIONS[HOTKEY_ACTION_COUNT] = {
        { HotkeyAction::cycle_tabs, HotkeyScope::global, Plain(KEY_TAB), L"CycleTabs" },
        { HotkeyAction::show_draw_tab, HotkeyScope::global, Plain('1'), L"ShowDrawTab" },
        { HotkeyAction::show_log_tab, HotkeyScope::global, Plain('2'), L"ShowLogTab" },
        { HotkeyAction::show_civ_pool_tab, HotkeyScope::global, Plain('3'), L"ShowCivPoolTab" },
        { HotkeyAction::open_options, HotkeyScope::global, Plain(KEY_F1), L"OpenOptions" },
        { HotkeyAction::open_hotkeys, HotkeyScope::global, Plain(FunctionKey(2)), L"OpenHotkeys" },
        { HotkeyAction::open_about, HotkeyScope::global, Plain(FunctionKey(3)), L"OpenAbout" },
        { HotkeyAction::open_tech_tree, HotkeyScope::global, Plain(FunctionKey(4)), L"OpenTechTree" },
        { HotkeyAction::quicksave, HotkeyScope::global, Plain(FunctionKey(5)), L"Quicksave" },
        { HotkeyAction::quickload, HotkeyScope::global, Plain(FunctionKey(9)), L"Quickload" },
        { HotkeyAction::save_preset, HotkeyScope::global, WithControl('S'), L"SavePreset" },
        { HotkeyAction::load_preset, HotkeyScope::global, WithControl('R'), L"LoadPreset" },
        { HotkeyAction::copy_share_link, HotkeyScope::global, Plain('P'), L"CopyShareLink" },
        { HotkeyAction::open_share_link, HotkeyScope::global, Plain('U'), L"OpenShareLink" },
        { HotkeyAction::join_lobby, HotkeyScope::global, WithControl('F'), L"JoinLobby" },
        { HotkeyAction::undo, HotkeyScope::global, WithControl('Z'), L"Undo" },
        { HotkeyAction::redraw, HotkeyScope::global, WithControl('X'), L"Redraw" },
        { HotkeyAction::open_sustainability_app, HotkeyScope::global, WithControl('T'), L"OpenSustainabilityApp" },
        { HotkeyAction::toggle_civ_name, HotkeyScope::global, Plain('Z'), L"ToggleCivName" },
        { HotkeyAction::toggle_civ_icon, HotkeyScope::global, Plain('X'), L"ToggleCivIcon" },
        { HotkeyAction::toggle_round_counter, HotkeyScope::global, Plain('C'), L"ToggleRoundCounter" },
        { HotkeyAction::toggle_civ_jingles, HotkeyScope::global, Plain('V'), L"ToggleCivJingles" },
        { HotkeyAction::toggle_ui_sounds, HotkeyScope::global, Plain('B'), L"ToggleUISounds" },
        { HotkeyAction::quit, HotkeyScope::global, Plain(KEY_ESCAPE), L"Exit" },
        { HotkeyAction::draw_tab_draw_civ, HotkeyScope::draw_tab, Plain(KEY_SPACE), L"DrawTabDrawCiv" },
        { HotkeyAction::draw_tab_reset_round, HotkeyScope::draw_tab, Plain(KEY_ENTER), L"DrawTabReset" },
        { HotkeyAction::draw_tab_open_options, HotkeyScope::draw_tab, Plain('Q'), L"DrawTabOpenOptions" },
        { HotkeyAction::draw_tab_open_sustainability_app, HotkeyScope::draw_tab, Plain('R'),
          L"DrawTabOpenSustainabilityApp" },
        { HotkeyAction::draw_tab_open_tech_tree, HotkeyScope::draw_tab, Plain('T'), L"DrawTabOpenTechTree" },
        { HotkeyAction::open_history, HotkeyScope::draw_tab, Plain('H'), L"DrawTabOpenHistory" },
        { HotkeyAction::log_tab_draw_civ, HotkeyScope::log_tab, Plain(KEY_SPACE), L"LogTabDrawCiv" },
        { HotkeyAction::log_tab_reset_round, HotkeyScope::log_tab, Plain(KEY_ENTER), L"LogTabReset" },
        { HotkeyAction::clear_drawn_log, HotkeyScope::log_tab, Plain('Q'), L"LogTabClearDrawnLog" },
        { HotkeyAction::toggle_remaining_log, HotkeyScope::log_tab, Plain('W'), L"LogTabToggleRemainingLog" },
        { HotkeyAction::enable_all_civs, HotkeyScope::civ_pool_tab, Plain(KEY_SPACE), L"CivPoolTabEnableAll" },
        { HotkeyAction::disable_all_civs, HotkeyScope::civ_pool_tab, Plain(KEY_ENTER), L"CivPoolTabDisableAll" },
        { HotkeyAction::edition_1, HotkeyScope::civ_pool_tab, Plain('Q'), L"CivPoolTabEdition1" },
        { HotkeyAction::edition_2, HotkeyScope::civ_pool_tab, Plain('W'), L"CivPoolTabEdition2" },
        { HotkeyAction::edition_3, HotkeyScope::civ_pool_tab, Plain('E'), L"CivPoolTabEdition3" },
        { HotkeyAction::toggle_auto_toggle, HotkeyScope::civ_pool_tab, Plain('R'), L"CivPoolTabAutoToggle" },
        { HotkeyAction::toggle_auto_reset, HotkeyScope::civ_pool_tab, Plain('T'), L"CivPoolTabAutoReset" },
        { HotkeyAction::dlc_slot_1, HotkeyScope::civ_pool_tab, Plain('A'), L"CivPoolTabDlc1" },
        { HotkeyAction::dlc_slot_2, HotkeyScope::civ_pool_tab, Plain('S'), L"CivPoolTabDlc2" },
        { HotkeyAction::dlc_slot_3, HotkeyScope::civ_pool_tab, Plain('D'), L"CivPoolTabDlc3" },
        { HotkeyAction::dlc_slot_4, HotkeyScope::civ_pool_tab, Plain('F'), L"CivPoolTabDlc4" },
        { HotkeyAction::dlc_slot_5, HotkeyScope::civ_pool_tab, Plain('G'), L"CivPoolTabDlc5" },
        { HotkeyAction::dlc_slot_6, HotkeyScope::civ_pool_tab, Plain('H'), L"CivPoolTabDlc6" },
        { HotkeyAction::dlc_slot_7, HotkeyScope::civ_pool_tab, Plain('J'), L"CivPoolTabDlc7" },
        { HotkeyAction::dlc_slot_8, HotkeyScope::civ_pool_tab, Plain('K'), L"CivPoolTabDlc8" },
        { HotkeyAction::dlc_slot_9, HotkeyScope::civ_pool_tab, Plain('L'), L"CivPoolTabDlc9" },
    };

    constexpr bool DefinitionsInActionOrder()
    {
        for (int i = 0; i < HOTKEY_ACTION_COUNT; i++)
        {
            if (static_cast<int>(DEFINITIONS[i].action) != i) return false;
        }
        return true;
    }
    static_assert(DefinitionsInActionOrder(), "DEFINITIONS defines every action, in the order of HotkeyAction");

    // The keys named here besides the letters, the digits, the number pad's digits and F1 to F12.
    struct NamedKey
    {
        uint8_t key;
        const wchar_t* name;
    };

    const NamedKey NAMED_KEYS[] = {
        { KEY_BACKSPACE, L"Backspace" }, { KEY_TAB, L"Tab" },        { KEY_ENTER, L"Enter" },
        { KEY_ESCAPE, L"Esc" },          { KEY_SPACE, L"Space" },    { KEY_PAGE_UP, L"PgUp" },
        { KEY_PAGE_DOWN, L"PgDn" },      { KEY_END, L"End" },        { KEY_HOME, L"Home" },
        { KEY_LEFT, L"Left" },           { KEY_UP, L"Up" },          { KEY_RIGHT, L"Right" },
        { KEY_DOWN, L"Down" },           { KEY_INSERT, L"Insert" },  { KEY_DELETE, L"Delete" },
        { KEY_MULTIPLY, L"Num *" },      { KEY_ADD, L"Num +" },      { KEY_SUBTRACT, L"Num -" },
        { KEY_DECIMAL, L"Num ." },       { KEY_DIVIDE, L"Num /" },
    };

    // The modifiers in the order their names lead a keystroke's name.
    struct NamedModifier
    {
        uint8_t modifier;
        const wchar_t* prefix;
    };

    const NamedModifier NAMED_MODIFIERS[] = {
        { MODIFIER_CONTROL, L"Ctrl+" }, { MODIFIER_ALT, L"Alt+" }, { MODIFIER_SHIFT, L"Shift+" } };
    constexpr uint8_t ALL_MODIFIERS = MODIFIER_ALT | MODIFIER_CONTROL | MODIFIER_SHIFT;

    // A key Windows answers itself while a modifier is held: switching windows (Alt+Tab, Alt+Esc), the Start menu
    // (Ctrl+Esc), closing the window (Alt+F4), its window menu (Alt+Space) and the security screen
    // (Ctrl+Alt+Delete). F12 it keeps for debuggers with any modifiers, or none.
    struct KeptKeystroke
    {
        uint8_t key;
        uint8_t held_modifiers;
    };

    constexpr KeptKeystroke KEPT_BY_WINDOWS[] = {
        { KEY_TAB, MODIFIER_ALT },
        { KEY_ESCAPE, MODIFIER_ALT },
        { KEY_ESCAPE, MODIFIER_CONTROL },
        { FunctionKey(4), MODIFIER_ALT },
        { KEY_SPACE, MODIFIER_ALT },
        { KEY_DELETE, MODIFIER_CONTROL | MODIFIER_ALT },
        { KEY_F12, 0 },
    };

    constexpr int MAX_KEY_LABEL = 10; // "Backspace" and its terminating null
    constexpr int HIGHEST_KEY = 0xFE; // Windows' virtual-key codes run from 1 to 254

    // The name of a key without its modifiers; empty for a key without one.
    struct KeyLabel
    {
        wchar_t text[MAX_KEY_LABEL];
    };

    bool IsKnownAction(HotkeyAction action)
    {
        const int index = static_cast<int>(action);
        return index >= 0 && index < HOTKEY_ACTION_COUNT;
    }

    // The action's place among count actions from first on, or NO_PLACE.
    int PlaceAmong(HotkeyAction action, HotkeyAction first, int count)
    {
        const int place = static_cast<int>(action) - static_cast<int>(first);
        return place >= 0 && place < count ? place : HotkeyBindings::NO_PLACE;
    }

    // The action in this place among count actions from first on, or none.
    HotkeyAction ActionInPlace(HotkeyAction first, int place, int count)
    {
        if (place < 0 || place >= count) return HotkeyAction::none;
        return static_cast<HotkeyAction>(static_cast<int>(first) + place);
    }

    bool IsLetterOrDigit(uint8_t key) { return (key >= '0' && key <= '9') || (key >= 'A' && key <= 'Z'); }

    KeyLabel LabelOf(uint8_t key)
    {
        KeyLabel label = {};
        if (IsLetterOrDigit(key))
        {
            label.text[0] = static_cast<wchar_t>(key);
            return label;
        }
        // The longest label written here, "Num 9" or "F12", fits its buffer.
        if (key >= KEY_NUMPAD_0 && key <= KEY_NUMPAD_9) (void)swprintf_s(label.text, L"Num %d", key - KEY_NUMPAD_0);
        if (key >= KEY_F1 && key <= KEY_F12) (void)swprintf_s(label.text, L"F%d", key - KEY_F1 + 1);
        for (const NamedKey& named : NAMED_KEYS)
        {
            if (named.key == key) (void)wcscpy_s(label.text, named.name);
        }
        return label;
    }

    bool HasLabel(uint8_t key) { return LabelOf(key).text[0] != L'\0'; }

    // The key whose label this text is, in any case, or 0 if none has.
    uint8_t KeyLabelled(const wchar_t* text)
    {
        if (!c_assert(text != nullptr) || text[0] == L'\0') return 0;
        for (int key = 1; key <= HIGHEST_KEY; key++)
        {
            const KeyLabel label = LabelOf(static_cast<uint8_t>(key));
            if (label.text[0] != L'\0' && _wcsicmp(label.text, text) == 0) return static_cast<uint8_t>(key);
        }
        return 0;
    }

    bool IsKeptByWindows(Keystroke keystroke)
    {
        for (const KeptKeystroke& kept : KEPT_BY_WINDOWS)
        {
            const bool held = (keystroke.modifiers & kept.held_modifiers) == kept.held_modifiers;
            if (keystroke.key == kept.key && held) return true;
        }
        return false;
    }

    // Hotkeys can work at the same time when one is global or both work on the same tab.
    bool ScopesOverlap(HotkeyScope first, HotkeyScope second)
    {
        return first == second || first == HotkeyScope::global || second == HotkeyScope::global;
    }
}

bool operator==(Keystroke left, Keystroke right) { return left.modifiers == right.modifiers && left.key == right.key; }

bool operator!=(Keystroke left, Keystroke right) { return !(left == right); }

// Modifiers first, in the order Ctrl, Alt, Shift: "Ctrl+Shift+S". A key without a name gives an empty name.
KeystrokeName NameKeystroke(Keystroke keystroke)
{
    KeystrokeName name = {};
    if (keystroke == NO_KEYSTROKE)
    {
        (void)wcscpy_s(name.characters, L"None");
        return name;
    }
    const KeyLabel label = LabelOf(keystroke.key);
    if (label.text[0] == L'\0') return name;
    for (const NamedModifier& named : NAMED_MODIFIERS)
    {
        if ((keystroke.modifiers & named.modifier) != 0) (void)wcscat_s(name.characters, named.prefix);
    }
    // The longest name, "Ctrl+Alt+Shift+Backspace", fits the buffer.
    (void)wcscat_s(name.characters, label.text);
    return name;
}

// Reads a keystroke's name, in any case: its modifiers, each once and in NameKeystroke's order, then its key.
ReadKeystroke ParseKeystroke(const wchar_t* text)
{
    const ReadKeystroke refused = { false, NO_KEYSTROKE };
    if (!c_assert(text != nullptr)) return refused;
    if (_wcsicmp(text, L"None") == 0) return { true, NO_KEYSTROKE };
    Keystroke keystroke = NO_KEYSTROKE;
    const wchar_t* rest = text;
    for (const NamedModifier& named : NAMED_MODIFIERS)
    {
        const size_t length = wcslen(named.prefix);
        if (_wcsnicmp(rest, named.prefix, length) != 0) continue;
        keystroke.modifiers = static_cast<uint8_t>(keystroke.modifiers | named.modifier);
        rest += length;
    }
    keystroke.key = KeyLabelled(rest);
    if (keystroke.key == 0) return refused;
    return { true, keystroke };
}

KeystrokeUse CheckKeystroke(Keystroke keystroke)
{
    if (IsKeptByWindows(keystroke)) return KeystrokeUse::kept_by_windows;
    const bool known_modifiers = (keystroke.modifiers & ~ALL_MODIFIERS) == 0;
    if (!known_modifiers || !HasLabel(keystroke.key)) return KeystrokeUse::unnamed_key;
    return KeystrokeUse::usable;
}

// Every hotkey starts with its default key. The editions' hotkeys beyond data.json's editions, and the DLC slots'
// beyond the most DLC toggles an edition has, are unused.
HotkeyBindings::HotkeyBindings(int edition_count, int most_dlc_toggles) :
    used_editions(std::min(std::max(edition_count, 0), MAX_EDITIONS)),
    used_dlc_slots(std::min(std::max(most_dlc_toggles, 0), DLC_HOTKEY_COUNT))
{
    (void)c_assert(edition_count >= 0 && edition_count <= MAX_EDITIONS);
    (void)c_assert(most_dlc_toggles >= 0 && most_dlc_toggles <= MAX_DLC_TOGGLES);
    RestoreDefaults();
}

HotkeyScope HotkeyBindings::ScopeOf(HotkeyAction action)
{
    if (!c_assert(IsKnownAction(action))) return HotkeyScope::global;
    return DEFINITIONS[static_cast<int>(action)].scope;
}

Keystroke HotkeyBindings::DefaultKeystrokeOf(HotkeyAction action)
{
    if (!c_assert(IsKnownAction(action))) return NO_KEYSTROKE;
    return DEFINITIONS[static_cast<int>(action)].default_keystroke;
}

const wchar_t* HotkeyBindings::SettingNameOf(HotkeyAction action)
{
    if (!c_assert(IsKnownAction(action))) return L"";
    return DEFINITIONS[static_cast<int>(action)].setting_name;
}

int HotkeyBindings::EditionOf(HotkeyAction action)
{
    return PlaceAmong(action, HotkeyAction::edition_1, MAX_EDITIONS);
}

int HotkeyBindings::DlcSlotOf(HotkeyAction action)
{
    return PlaceAmong(action, HotkeyAction::dlc_slot_1, DLC_HOTKEY_COUNT);
}

HotkeyAction HotkeyBindings::EditionHotkey(int edition)
{
    return ActionInPlace(HotkeyAction::edition_1, edition, MAX_EDITIONS);
}

HotkeyAction HotkeyBindings::DlcSlotHotkey(int slot)
{
    return ActionInPlace(HotkeyAction::dlc_slot_1, slot, DLC_HOTKEY_COUNT);
}

bool HotkeyBindings::IsUsed(HotkeyAction action) const
{
    if (!c_assert(IsKnownAction(action))) return false;
    const int edition = EditionOf(action);
    if (edition != NO_PLACE) return edition < used_editions;
    const int slot = DlcSlotOf(action);
    if (slot != NO_PLACE) return slot < used_dlc_slots;
    return true;
}

Keystroke HotkeyBindings::KeystrokeOf(HotkeyAction action) const
{
    if (!c_assert(IsKnownAction(action))) return NO_KEYSTROKE;
    return keystrokes[static_cast<int>(action)];
}

// What the keystroke does on this tab: a global hotkey's, else the tab's own, else none.
HotkeyAction HotkeyBindings::ActionFor(Keystroke keystroke, HotkeyScope tab) const
{
    if (keystroke == NO_KEYSTROKE) return HotkeyAction::none;
    HotkeyAction tab_action = HotkeyAction::none;
    for (int i = 0; i < HOTKEY_ACTION_COUNT; i++)
    {
        const HotkeyAction action = static_cast<HotkeyAction>(i);
        if (keystrokes[i] != keystroke || !IsUsed(action)) continue;
        if (DEFINITIONS[i].scope == HotkeyScope::global) return action;
        if (DEFINITIONS[i].scope == tab) tab_action = action;
    }
    return tab_action;
}

// The hotkeys that have this keystroke where this one would work too, which giving it to this one takes it from.
HotkeyBindings::Clashes HotkeyBindings::ClashesWith(HotkeyAction action, Keystroke keystroke) const
{
    Clashes clashes = {};
    if (!c_assert(IsKnownAction(action)) || keystroke == NO_KEYSTROKE) return clashes;
    const HotkeyScope scope = DEFINITIONS[static_cast<int>(action)].scope;
    for (int i = 0; i < HOTKEY_ACTION_COUNT; i++)
    {
        const HotkeyAction other = static_cast<HotkeyAction>(i);
        if (other == action || keystrokes[i] != keystroke || !IsUsed(other)) continue;
        if (!ScopesOverlap(scope, DEFINITIONS[i].scope)) continue;
        // No two hotkeys that work together share a keystroke, so it is in use at most once on each tab.
        if (!c_assert(clashes.count < MAX_CLASHES)) return clashes;
        clashes.actions[clashes.count] = other;
        clashes.count++;
    }
    return clashes;
}

// Gives the hotkey a keystroke that can be one, taking it from the hotkeys it clashes with; false if it can't.
bool HotkeyBindings::Assign(HotkeyAction action, Keystroke keystroke)
{
    if (!c_assert(IsKnownAction(action)) || !IsUsed(action)) return false;
    if (CheckKeystroke(keystroke) != KeystrokeUse::usable) return false;
    const Clashes clashes = ClashesWith(action, keystroke);
    for (int i = 0; i < clashes.count && i < MAX_CLASHES; i++) Clear(clashes.actions[i]);
    keystrokes[static_cast<int>(action)] = keystroke;
    return true;
}

void HotkeyBindings::Clear(HotkeyAction action)
{
    if (!c_assert(IsKnownAction(action))) return;
    keystrokes[static_cast<int>(action)] = NO_KEYSTROKE;
}

void HotkeyBindings::RestoreDefaults()
{
    for (int i = 0; i < HOTKEY_ACTION_COUNT; i++) keystrokes[i] = DEFINITIONS[i].default_keystroke;
}
