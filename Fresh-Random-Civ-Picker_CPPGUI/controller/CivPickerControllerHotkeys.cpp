#include "CivPickerController.h"

#include "Assertions.h"

// The hotkeys of the controller (CivPickerController): registered while the main window is active, each keystroke
// turned into what its hotkey does on the tab shown, their keys in settings.ini, and the Customise Hotkeys dialog
// (Options > Customise), where the user gives them other keys.

namespace
{
    // The model numbers keystrokes as RegisterHotKey and the keyboard messages do.
    static_assert(MODIFIER_ALT == MOD_ALT && MODIFIER_CONTROL == MOD_CONTROL && MODIFIER_SHIFT == MOD_SHIFT,
                  "the model's modifiers are Windows' hotkey modifiers");
    static_assert(KEY_BACKSPACE == VK_BACK && KEY_TAB == VK_TAB && KEY_ENTER == VK_RETURN && KEY_SHIFT == VK_SHIFT &&
                      KEY_CONTROL == VK_CONTROL && KEY_ALT == VK_MENU && KEY_ESCAPE == VK_ESCAPE &&
                      KEY_SPACE == VK_SPACE, "the model's keys are Windows' virtual-key codes");
    static_assert(KEY_PAGE_UP == VK_PRIOR && KEY_PAGE_DOWN == VK_NEXT && KEY_END == VK_END && KEY_HOME == VK_HOME &&
                      KEY_LEFT == VK_LEFT && KEY_UP == VK_UP && KEY_RIGHT == VK_RIGHT && KEY_DOWN == VK_DOWN &&
                      KEY_INSERT == VK_INSERT && KEY_DELETE == VK_DELETE, "the editing and arrow keys are Windows'");
    static_assert(KEY_NUMPAD_0 == VK_NUMPAD0 && KEY_NUMPAD_9 == VK_NUMPAD9 && KEY_MULTIPLY == VK_MULTIPLY &&
                      KEY_ADD == VK_ADD && KEY_SUBTRACT == VK_SUBTRACT && KEY_DECIMAL == VK_DECIMAL &&
                      KEY_DIVIDE == VK_DIVIDE, "the number pad's keys are Windows'");
    static_assert(KEY_F1 == VK_F1 && KEY_F11 == VK_F11 && KEY_F12 == VK_F12, "the function keys are Windows'");

    const wchar_t* const HOTKEYS_SECTION = L"Hotkeys";
    constexpr int MAX_SETTING_TEXT = 64;
    // In the Customise Hotkeys dialog, Backspace leaves a hotkey without a key.
    constexpr Keystroke CLEARING_KEYSTROKE = { 0, KEY_BACKSPACE };
    // A keystroke the dialog tries, to see whether another program has it, is registered under the ID after the
    // hotkeys'.
    constexpr int TRIAL_HOTKEY_ID = HOTKEY_ACTION_COUNT + 1;

    // A hotkey is registered under its action's number, from 1.
    int HotkeyId(int action_index) { return action_index + 1; }

    HotkeyAction ActionAt(int action_index) { return static_cast<HotkeyAction>(action_index); }

    // Whether a hotkey before this one, which is registered first, has the same keystroke: hotkeys of different tabs
    // can share one.
    bool SharesEarlierKeystroke(const HotkeyBindings& hotkeys, int action_index)
    {
        if (!c_assert(action_index >= 0 && action_index < HOTKEY_ACTION_COUNT)) return true;
        const Keystroke keystroke = hotkeys.KeystrokeOf(ActionAt(action_index));
        for (int i = 0; i < action_index; i++)
        {
            if (hotkeys.IsUsed(ActionAt(i)) && hotkeys.KeystrokeOf(ActionAt(i)) == keystroke) return true;
        }
        return false;
    }

    // The Civ Pool tab's hotkeys that change the pool, so auto-reset resets before them, as before the pool's clicks.
    bool ChangesPool(HotkeyAction action)
    {
        const bool all_civs = action == HotkeyAction::enable_all_civs || action == HotkeyAction::disable_all_civs;
        return all_civs || HotkeyBindings::EditionOf(action) != HotkeyBindings::NO_PLACE ||
               HotkeyBindings::DlcSlotOf(action) != HotkeyBindings::NO_PLACE;
    }
}

// ---------------------------------------------------------------------------------------------------
// Registering the hotkeys and running them

// Registers each keystroke a hotkey in use has, once. A keystroke another program has taken stays without its
// hotkey, as Windows leaves it to that program; the Customise Hotkeys dialog warns of that.
void CivPickerController::EnableHotkeys()
{
    const HotkeyBindings& hotkeys = model.GetHotkeys();
    for (int i = 0; i < HOTKEY_ACTION_COUNT; i++)
    {
        const Keystroke keystroke = hotkeys.KeystrokeOf(ActionAt(i));
        if (keystroke == NO_KEYSTROKE || !hotkeys.IsUsed(ActionAt(i)) || SharesEarlierKeystroke(hotkeys, i)) continue;
        (void)RegisterHotKey(main_window, HotkeyId(i), keystroke.modifiers, keystroke.key);
    }
}

void CivPickerController::DisableHotkeys()
{
    // An ID that has no hotkey registered has nothing to unregister.
    for (int i = 0; i < HOTKEY_ACTION_COUNT; i++) (void)UnregisterHotKey(main_window, HotkeyId(i));
}

void CivPickerController::OnActivate(WPARAM activation)
{
    if (activation == WA_ACTIVE || activation == WA_CLICKACTIVE) EnableHotkeys();
    else if (activation == WA_INACTIVE) DisableHotkeys();
}

void CivPickerController::OnHotkey(Keystroke keystroke)
{
    hotkey_pressed = true;
    if (GetForegroundWindow() != main_window)
    {
        DisableHotkeys();
        return;
    }
    EnableHotkeys();
    const HotkeyAction action = model.GetHotkeys().ActionFor(keystroke, ShownTabScope());
    if (action != HotkeyAction::none) RunHotkey(action);
}

HotkeyScope CivPickerController::ShownTabScope() const
{
    switch (view.GetCurrentTab())
    {
        case CivPickerView::DRAW_TAB: return HotkeyScope::draw_tab;
        case CivPickerView::LOG_TAB: return HotkeyScope::log_tab;
        case CivPickerView::CIV_POOL_TAB: return HotkeyScope::civ_pool_tab;
        default: (void)c_assert(false); return HotkeyScope::draw_tab;
    }
}

void CivPickerController::RunHotkey(HotkeyAction action)
{
    switch (HotkeyBindings::ScopeOf(action))
    {
        case HotkeyScope::global: RunGlobalHotkey(action); return;
        case HotkeyScope::draw_tab: RunDrawTabHotkey(action); return;
        case HotkeyScope::log_tab: RunLogTabHotkey(action); return;
        case HotkeyScope::civ_pool_tab: RunCivPoolHotkey(action); return;
        default: (void)c_assert(false); return;
    }
}

void CivPickerController::RunGlobalHotkey(HotkeyAction action)
{
    switch (action)
    {
        case HotkeyAction::cycle_tabs: CycleTabs(); return;
        case HotkeyAction::show_draw_tab: SelectTab(CivPickerView::DRAW_TAB); return;
        case HotkeyAction::show_log_tab: SelectTab(CivPickerView::LOG_TAB); return;
        case HotkeyAction::show_civ_pool_tab: SelectTab(CivPickerView::CIV_POOL_TAB); return;
        case HotkeyAction::open_options: OpenOptions(main_window); return;
        case HotkeyAction::open_hotkeys: OpenHotkeys(main_window); return;
        case HotkeyAction::open_about: OpenAbout(main_window); return;
        case HotkeyAction::open_tech_tree: OpenTechTree(); return;
        case HotkeyAction::quicksave: SaveLog(SaveType::quick); return;
        case HotkeyAction::quickload: LoadLog(SaveType::quick); return;
        case HotkeyAction::save_preset: SaveLog(SaveType::manual); return;
        case HotkeyAction::load_preset: LoadLog(SaveType::manual); return;
        case HotkeyAction::copy_share_link: CopyRoundLink(); return;
        case HotkeyAction::open_share_link: OpenShareLink(); return;
        case HotkeyAction::join_lobby: JoinLobby(); return;
        case HotkeyAction::undo: Undo(); return;
        case HotkeyAction::redraw: model.RedrawCiv(); return;
        case HotkeyAction::open_sustainability_app: OpenSurvapp(); return;
        case HotkeyAction::toggle_civ_name:
        case HotkeyAction::toggle_civ_icon:
        case HotkeyAction::toggle_round_counter:
        case HotkeyAction::toggle_civ_jingles:
        case HotkeyAction::toggle_ui_sounds: ToggleDisplaySetting(action); return;
        case HotkeyAction::quit: Quit(); return;
        default: (void)c_assert(false); return;
    }
}

void CivPickerController::CycleTabs()
{
    const int selected_tab = view.GetSelectedTab();
    SelectTab(selected_tab < CivPickerView::CIV_POOL_TAB ? selected_tab + 1 : CivPickerView::DRAW_TAB);
}

// The display and sound settings that a hotkey turns on and off.
void CivPickerController::ToggleDisplaySetting(HotkeyAction action)
{
    Settings& settings = model.EditSettings();
    if (action == HotkeyAction::toggle_civ_name)
    {
        view.PlayAudio(SoundType::button);
        settings.civ_labels_enabled = !settings.civ_labels_enabled;
        view.RefreshCivLabel();
    }
    if (action == HotkeyAction::toggle_civ_icon)
    {
        view.PlayAudio(SoundType::button);
        settings.icons_enabled = !settings.icons_enabled;
        view.RefreshCivIcon();
    }
    if (action == HotkeyAction::toggle_civ_jingles)
    {
        settings.jingles_enabled = !settings.jingles_enabled;
        if (settings.jingles_enabled) view.PlayJingle(model.GetCurrentCiv());
        else view.StopSound();
    }
    if (action == HotkeyAction::toggle_ui_sounds)
    {
        if (!settings.ui_sounds_enabled) view.PlayAudio(SoundType::button);
        settings.ui_sounds_enabled = !settings.ui_sounds_enabled;
    }
    if (action == HotkeyAction::toggle_round_counter)
    {
        view.PlayAudio(SoundType::button);
        settings.iteration_label_enabled = !settings.iteration_label_enabled;
        view.RefreshCornerLabel();
    }
}

void CivPickerController::RunDrawTabHotkey(HotkeyAction action)
{
    switch (action)
    {
        case HotkeyAction::draw_tab_draw_civ: DrawCivWithinRateCap(); return;
        case HotkeyAction::draw_tab_reset_round: model.ResetProgram(false); return;
        case HotkeyAction::draw_tab_open_options: OpenOptions(main_window); return;
        case HotkeyAction::draw_tab_open_sustainability_app: OpenSurvapp(); return;
        case HotkeyAction::draw_tab_open_tech_tree: OpenTechTree(); return;
        case HotkeyAction::open_history: OpenHistory(main_window); return;
        default: (void)c_assert(false); return;
    }
}

void CivPickerController::RunLogTabHotkey(HotkeyAction action)
{
    switch (action)
    {
        case HotkeyAction::log_tab_draw_civ: DrawCivWithinRateCap(); return;
        case HotkeyAction::log_tab_reset_round: model.ResetProgram(false); return;
        case HotkeyAction::clear_drawn_log: ClearDrawnLog(); return;
        case HotkeyAction::toggle_remaining_log: ToggleRemainLog(); return;
        default: (void)c_assert(false); return;
    }
}

void CivPickerController::RunCivPoolHotkey(HotkeyAction action)
{
    if (ChangesPool(action) && model.GetSettings().autoreset_enabled) model.ResetProgram(true);
    switch (action)
    {
        case HotkeyAction::enable_all_civs: EnableAll(true); return;
        case HotkeyAction::disable_all_civs: DisableAll(true); return;
        case HotkeyAction::edition_1:
        case HotkeyAction::edition_2:
        case HotkeyAction::edition_3: SelectEditionByPlace(HotkeyBindings::EditionOf(action)); return;
        case HotkeyAction::toggle_auto_toggle: ToggleAutoToggle(); return;
        case HotkeyAction::toggle_auto_reset: ToggleAutoReset(); return;
        case HotkeyAction::dlc_slot_1:
        case HotkeyAction::dlc_slot_2:
        case HotkeyAction::dlc_slot_3:
        case HotkeyAction::dlc_slot_4:
        case HotkeyAction::dlc_slot_5:
        case HotkeyAction::dlc_slot_6:
        case HotkeyAction::dlc_slot_7:
        case HotkeyAction::dlc_slot_8:
        case HotkeyAction::dlc_slot_9: ToggleDlcBySlot(HotkeyBindings::DlcSlotOf(action)); return;
        default: (void)c_assert(false); return;
    }
}

// The editions' hotkeys pick the editions in data.json's order.
void CivPickerController::SelectEditionByPlace(int edition)
{
    if (edition >= 0 && edition < model.Catalog().EditionCount()) model.SetEdition(edition);
}

// The DLC slots' hotkeys toggle the DLCs listed first in the current edition's dlcToggles, top to bottom.
void CivPickerController::ToggleDlcBySlot(int slot)
{
    const EditionInfo& edition = model.Catalog().EditionAt(model.GetEdition());
    if (slot >= 0 && slot < edition.dlc_toggle_count) ToggleDlc(edition.dlc_toggles[slot]);
}

// ---------------------------------------------------------------------------------------------------
// The hotkeys' keys in settings.ini

// [Hotkeys] has a line for each hotkey whose key the user changed: its setting name and its keystroke's name, or
// None. A line that names no keystroke the hotkey can have is left out, and the hotkey keeps its default key.
void CivPickerController::LoadHotkeys()
{
    HotkeyBindings& hotkeys = model.EditHotkeys();
    for (int i = 0; i < HOTKEY_ACTION_COUNT; i++)
    {
        wchar_t text[MAX_SETTING_TEXT] = L"";
        // A missing line reads as empty, which no keystroke is called.
        (void)GetPrivateProfileStringW(HOTKEYS_SECTION, HotkeyBindings::SettingNameOf(ActionAt(i)), L"", text,
                                       MAX_SETTING_TEXT, settings_file_path.c_str());
        const ReadKeystroke read = ParseKeystroke(text);
        if (!read.ok) continue;
        if (read.keystroke == NO_KEYSTROKE) hotkeys.Clear(ActionAt(i));
        // One that can't be a hotkey (F12) or a hotkey data.json leaves unused keep their default key.
        else (void)hotkeys.Assign(ActionAt(i), read.keystroke);
    }
}

// The section is written anew, so a hotkey given its default key again loses its line.
void CivPickerController::SaveHotkeys() const
{
    const HotkeyBindings& hotkeys = model.GetHotkeys();
    const wchar_t* ini_file = settings_file_path.c_str();
    // Removing a section that isn't there leaves nothing to do; a line that can't be written keeps its default.
    (void)WritePrivateProfileStringW(HOTKEYS_SECTION, nullptr, nullptr, ini_file);
    for (int i = 0; i < HOTKEY_ACTION_COUNT; i++)
    {
        const Keystroke keystroke = hotkeys.KeystrokeOf(ActionAt(i));
        if (keystroke == HotkeyBindings::DefaultKeystrokeOf(ActionAt(i))) continue;
        (void)WritePrivateProfileStringW(HOTKEYS_SECTION, HotkeyBindings::SettingNameOf(ActionAt(i)),
                                         NameKeystroke(keystroke).characters, ini_file);
    }
}

// ---------------------------------------------------------------------------------------------------
// The Customise Hotkeys dialog

// The hotkeys change as the user gives them keys; Cancel gives them back the keys they had. The keys registered go
// first: a hotkey registered again under its ID with another key would keep the old key too. The main window
// registers the hotkeys' keys anew once it is active again.
void CivPickerController::CustomiseHotkeys(HWND owner)
{
    view.PlayAudio(SoundType::button);
    DisableHotkeys();
    const HotkeyBindings before = model.GetHotkeys();
    if (!view.ShowHotkeyEditor(owner)) model.EditHotkeys() = before;
    view.RefreshHotkeyTexts();
}

// A key the user pressed for a hotkey: Backspace takes its key away, a keystroke Windows keeps or one without a name
// is refused, and any other becomes its key, taken from the hotkeys it clashes with.
void CivPickerController::OnKeyChosen(HWND editor, HotkeyAction action, Keystroke keystroke)
{
    if (!c_assert(editor != NULL) || !c_assert(model.GetHotkeys().IsUsed(action))) return;
    CivPickerView::KeyChange change = { CivPickerView::KeyChoice::cleared, action, keystroke, {}, false };
    if (keystroke == CLEARING_KEYSTROKE)
    {
        view.PlayAudio(SoundType::button);
        model.EditHotkeys().Clear(action);
        view.ShowKeyChange(editor, change);
        return;
    }
    const KeystrokeUse use = CheckKeystroke(keystroke);
    if (use != KeystrokeUse::usable)
    {
        view.PlayAudio(SoundType::error);
        change.choice = use == KeystrokeUse::kept_by_windows ? CivPickerView::KeyChoice::kept_by_windows
                                                              : CivPickerView::KeyChoice::unnamed_key;
        view.ShowKeyChange(editor, change);
        return;
    }
    change.choice = CivPickerView::KeyChoice::assigned;
    change.taken_from = model.GetHotkeys().ClashesWith(action, keystroke);
    change.used_by_other_program = IsAnotherProgramsHotkey(keystroke);
    view.PlayAudio(SoundType::button);
    if (!c_assert(model.EditHotkeys().Assign(action, keystroke))) return;
    view.ShowKeyChange(editor, change);
}

// While the Customise Hotkeys dialog is open the program's own hotkeys are unregistered (CustomiseHotkeys), so a
// keystroke that can't be registered now is another program's hotkey.
bool CivPickerController::IsAnotherProgramsHotkey(Keystroke keystroke) const
{
    if (!c_assert(main_window != NULL)) return false;
    if (!RegisterHotKey(main_window, TRIAL_HOTKEY_ID, keystroke.modifiers, keystroke.key))
        return GetLastError() == ERROR_HOTKEY_ALREADY_REGISTERED;
    // The trial hotkey was registered a moment ago, so it can be unregistered.
    (void)UnregisterHotKey(main_window, TRIAL_HOTKEY_ID);
    return false;
}

void CivPickerController::OnDefaultKeysChosen(HWND editor)
{
    if (!c_assert(editor != NULL)) return;
    view.PlayAudio(SoundType::button);
    model.EditHotkeys().RestoreDefaults();
    view.ShowKeyChange(editor, { CivPickerView::KeyChoice::defaults_restored, HotkeyAction::none, NO_KEYSTROKE, {},
                                 false });
}
