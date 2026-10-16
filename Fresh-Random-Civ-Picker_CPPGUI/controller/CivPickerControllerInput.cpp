#include "CivPickerController.h"

#include <shellapi.h>

#include "Resource.h"

// The user's input to the controller (CivPickerController): the hotkeys, registered while the main window is
// active, and the commands of the menus, buttons and toggles, each turned into one of the controller's actions.

namespace
{
    struct Hotkey { UINT modifier; UINT key; int id; };

    const Hotkey HOTKEYS[HOTKEY_AMOUNT] = { { 0, VK_TAB, HOTKEY_ID_TAB },
                                            { 0, VK_SPACE, HOTKEY_ID_SPACE },
                                            { 0, VK_RETURN, HOTKEY_ID_RETURN },
                                            { 0, VK_ESCAPE, HOTKEY_ID_ESC },
                                            { 0, 0x5A, HOTKEY_ID_Z },
                                            { 0, 0x58, HOTKEY_ID_X },
                                            { 0, 0x43, HOTKEY_ID_C },
                                            { 0, 0x56, HOTKEY_ID_V },
                                            { 0, 0x54, HOTKEY_ID_T },
                                            { 0, 0x31, HOTKEY_ID_1 },
                                            { 0, 0x32, HOTKEY_ID_2 },
                                            { 0, 0x33, HOTKEY_ID_3 },
                                            { 0, 0x51, HOTKEY_ID_Q },
                                            { 0, 0x57, HOTKEY_ID_W },
                                            { 0, 0x45, HOTKEY_ID_E },
                                            { 0, 0x41, HOTKEY_ID_A },
                                            { 0, 0x53, HOTKEY_ID_S },
                                            { 0, 0x44, HOTKEY_ID_D },
                                            { 0, 0x46, HOTKEY_ID_F },
                                            { 0, 0x47, HOTKEY_ID_G },
                                            { 0, 0x48, HOTKEY_ID_H },
                                            { 0, 0x52, HOTKEY_ID_R },
                                            { 0, 0x54, HOTKEY_ID_T },
                                            { 0, VK_F1, HOTKEY_ID_F1 },
                                            { 0, 0x42, HOTKEY_ID_B },
                                            { 0, VK_F2, HOTKEY_ID_F2 },
                                            { 0, VK_F3, HOTKEY_ID_F3 },
                                            { 0, VK_F4, HOTKEY_ID_F4 },
                                            { 0, VK_F5, HOTKEY_ID_F5 },
                                            { 0, VK_F6, HOTKEY_ID_F5 },
                                            { 0, VK_F9, HOTKEY_ID_F9 },
                                            { MOD_CONTROL, 0x53, HOTKEY_ID_CTRLS },
                                            { MOD_CONTROL, 0x52, HOTKEY_ID_CTRLR },
                                            { MOD_CONTROL, 0x46, HOTKEY_ID_CTRLF },
                                            { MOD_CONTROL, 0x5A, HOTKEY_ID_CTRLZ },
                                            { MOD_CONTROL, 0x58, HOTKEY_ID_CTRLX },
                                            { MOD_CONTROL, 0x54, HOTKEY_ID_CTRLT },
                                            { 0, 0x4a, HOTKEY_ID_J },
                                            { 0, 0x4b, HOTKEY_ID_K },
                                            { 0, 0x4c, HOTKEY_ID_L },
                                            { 0, 0x50, HOTKEY_ID_P },
                                            { 0, 0x55, HOTKEY_ID_U } };

    // Cap on user-triggered draws. Each draw starts its jingle as a new asynchronous sound, and Windows
    // frees the interrupted ones only gradually: holding Space at keyboard-repeat speed piled up about
    // 1 MB per draw. At 10 draws per second the memory use was measured to stay flat.
    const ULONGLONG MAX_DRAWS_PER_SECOND = 10;
    const ULONGLONG MIN_DRAW_INTERVAL_MS = 1000 / MAX_DRAWS_PER_SECOND;

    // On the Civ Pool tab these hotkeys change the pool, so auto-reset resets before them.
    const WPARAM POOL_CHANGING_HOTKEYS[] =
    {
        HOTKEY_ID_SPACE, HOTKEY_ID_RETURN, HOTKEY_ID_Q, HOTKEY_ID_W, HOTKEY_ID_E, HOTKEY_ID_A, HOTKEY_ID_S,
        HOTKEY_ID_D, HOTKEY_ID_F, HOTKEY_ID_G, HOTKEY_ID_H, HOTKEY_ID_J, HOTKEY_ID_K, HOTKEY_ID_L
    };

    struct WebLink { int command_id; const wchar_t* url; };
    const WebLink WEB_LINKS[] =
    {
        { IDM_AOE2INSIGHTS, L"https://www.aoe2insights.com/lobbies/" },
        { IDM_SPECTATORDASHBOARD, L"https://aoe2recs.com/" },
        { IDM_GITHUB, L"https://github.com/cnordenb/Fresh-Random-Civ-Picker_CPPGUI" },
        { IDM_WEBVERSION, L"https://frcp.net/" },
        { IDM_DISCORD, L"https://discord.gg/rcFqrBdVUN" },
        { IDM_DONATE, L"https://ko-fi.com/hjoerleif" },
        { IDM_UPDATE, L"https://github.com/cnordenb/Fresh-Random-Civ-Picker_GUI/releases" },
        { IDM_TECHTREE, L"https://aoe2techtree.net/" },
        { IDM_STATS, L"https://aoestats.io/" },
        { IDM_USAGERULES, L"https://www.xbox.com/en-US/developers/rules" }
    };
}

// ---------------------------------------------------------------------------------------------------
// Hotkeys

void CivPickerController::EnableHotkeys()
{
    for (const Hotkey& hotkey : HOTKEYS) RegisterHotKey(main_window, hotkey.id, hotkey.modifier, hotkey.key);
}

void CivPickerController::DisableHotkeys()
{
    for (int hotkey_id = 1; hotkey_id < HOTKEY_AMOUNT; hotkey_id++) UnregisterHotKey(main_window, hotkey_id);
}

void CivPickerController::OnActivate(WPARAM activation)
{
    if (activation == WA_ACTIVE || activation == WA_CLICKACTIVE) EnableHotkeys();
    else if (activation == WA_INACTIVE) DisableHotkeys();
}

void CivPickerController::OnHotkey(WPARAM hotkey_id)
{
    hotkey_pressed = true;
    if (GetForegroundWindow() != main_window)
    {
        DisableHotkeys();
        return;
    }
    EnableHotkeys();

    HandleGlobalHotkey(hotkey_id);
    if (view.GetCurrentTab() == CivPickerView::DRAW_TAB) HandleDrawTabHotkey(hotkey_id);
    if (view.GetCurrentTab() != CivPickerView::CIV_POOL_TAB) HandleDrawingHotkey(hotkey_id);
    else HandleCivPoolHotkey(hotkey_id);
    if (view.GetCurrentTab() == CivPickerView::LOG_TAB) HandleLogTabHotkey(hotkey_id);
    HandleTabHotkey(hotkey_id);
    HandleDisplayToggleHotkey(hotkey_id);
}

void CivPickerController::HandleGlobalHotkey(WPARAM hotkey_id)
{
    switch (hotkey_id)
    {
        case HOTKEY_ID_ESC: Quit(); break;
        case HOTKEY_ID_F1: OpenOptions(main_window); break;
        case HOTKEY_ID_F2: OpenHotkeys(main_window); break;
        case HOTKEY_ID_F3: OpenAbout(main_window); break;
        case HOTKEY_ID_F4: OpenTechTree(); break;
        case HOTKEY_ID_F5: SaveLog(SaveType::quick); break;
        case HOTKEY_ID_F9: LoadLog(SaveType::quick); break;
        case HOTKEY_ID_CTRLS: SaveLog(SaveType::manual); break;
        case HOTKEY_ID_CTRLR: LoadLog(SaveType::manual); break;
        case HOTKEY_ID_CTRLF: JoinLobby(); break;
        case HOTKEY_ID_CTRLZ: Undo(); break;
        case HOTKEY_ID_CTRLX: model.RedrawCiv(); break;
        case HOTKEY_ID_CTRLT: OpenSurvapp(); break;
        case HOTKEY_ID_P: CopyRoundLink(); break;
        case HOTKEY_ID_U: OpenShareLink(); break;
        default: break;
    }
}

void CivPickerController::HandleDrawTabHotkey(WPARAM hotkey_id)
{
    switch (hotkey_id)
    {
        case HOTKEY_ID_R: OpenSurvapp(); break;
        case HOTKEY_ID_Q: OpenOptions(main_window); break;
        case HOTKEY_ID_T: OpenTechTree(); break;
        case HOTKEY_ID_H: OpenHistory(main_window); break;
        default: break;
    }
}

void CivPickerController::HandleDrawingHotkey(WPARAM hotkey_id)
{
    if (hotkey_id == HOTKEY_ID_SPACE) DrawCivWithinRateCap();
    if (hotkey_id == HOTKEY_ID_RETURN) model.ResetProgram(false);
}

void CivPickerController::DrawCivWithinRateCap()
{
    const ULONGLONG now = GetTickCount64();
    if (now - last_draw_tick < MIN_DRAW_INTERVAL_MS) return;
    last_draw_tick = now;
    model.DrawCiv(false, L"");
}

void CivPickerController::HandleCivPoolHotkey(WPARAM hotkey_id)
{
    bool changes_pool = false;
    for (WPARAM pool_hotkey : POOL_CHANGING_HOTKEYS) if (pool_hotkey == hotkey_id) changes_pool = true;
    if (changes_pool && model.GetSettings().autoreset_enabled) model.ResetProgram(true);

    switch (hotkey_id)
    {
        case HOTKEY_ID_SPACE: EnableAll(true); break;
        case HOTKEY_ID_RETURN: DisableAll(true); break;
        case HOTKEY_ID_R: ToggleAutoToggle(); break;
        case HOTKEY_ID_T: ToggleAutoReset(); break;
        default: break;
    }
    HandleEditionHotkey(hotkey_id);
    HandleDlcHotkey(hotkey_id);
}

// Q, W and E pick the editions in data.json's order.
void CivPickerController::HandleEditionHotkey(WPARAM hotkey_id)
{
    const int edition_count = model.Catalog().EditionCount();
    for (int edition = 0; edition < edition_count && edition < MAX_EDITIONS; edition++)
    {
        if (hotkey_id == static_cast<WPARAM>(EDITION_HOTKEY_IDS[edition])) model.SetEdition(edition);
    }
}

// A to L toggle the first DLCs listed in the current edition's dlcToggles, top to bottom.
void CivPickerController::HandleDlcHotkey(WPARAM hotkey_id)
{
    const EditionInfo& edition = model.Catalog().EditionAt(model.GetEdition());
    for (int slot = 0; slot < edition.dlc_toggle_count && slot < DLC_HOTKEY_COUNT; slot++)
    {
        if (hotkey_id == static_cast<WPARAM>(DLC_SLOT_HOTKEY_IDS[slot])) ToggleDlc(edition.dlc_toggles[slot]);
    }
}

void CivPickerController::HandleLogTabHotkey(WPARAM hotkey_id)
{
    if (hotkey_id == HOTKEY_ID_Q) ClearDrawnLog();
    if (hotkey_id == HOTKEY_ID_W) ToggleRemainLog();
}

void CivPickerController::HandleTabHotkey(WPARAM hotkey_id)
{
    if (hotkey_id == HOTKEY_ID_TAB)
    {
        const int selected_tab = view.GetSelectedTab();
        SelectTab(selected_tab < CivPickerView::CIV_POOL_TAB ? selected_tab + 1 : CivPickerView::DRAW_TAB);
    }
    if (hotkey_id == HOTKEY_ID_1) SelectTab(CivPickerView::DRAW_TAB);
    if (hotkey_id == HOTKEY_ID_2) SelectTab(CivPickerView::LOG_TAB);
    if (hotkey_id == HOTKEY_ID_3) SelectTab(CivPickerView::CIV_POOL_TAB);
}

void CivPickerController::HandleDisplayToggleHotkey(WPARAM hotkey_id)
{
    Settings& settings = model.EditSettings();
    if (hotkey_id == HOTKEY_ID_Z)
    {
        view.PlayAudio(SoundType::button);
        settings.civ_labels_enabled = !settings.civ_labels_enabled;
        view.RefreshCivLabel();
    }
    if (hotkey_id == HOTKEY_ID_X)
    {
        view.PlayAudio(SoundType::button);
        settings.icons_enabled = !settings.icons_enabled;
        view.RefreshCivIcon();
    }
    if (hotkey_id == HOTKEY_ID_V)
    {
        settings.jingles_enabled = !settings.jingles_enabled;
        if (settings.jingles_enabled) view.PlayJingle(model.GetCurrentCiv());
        else view.StopSound();
    }
    if (hotkey_id == HOTKEY_ID_B)
    {
        if (!settings.ui_sounds_enabled) view.PlayAudio(SoundType::button);
        settings.ui_sounds_enabled = !settings.ui_sounds_enabled;
    }
    if (hotkey_id == HOTKEY_ID_C)
    {
        view.PlayAudio(SoundType::button);
        settings.iteration_label_enabled = !settings.iteration_label_enabled;
        view.RefreshCornerLabel();
    }
}

// ---------------------------------------------------------------------------------------------------
// Menu, button and checkbox commands

bool CivPickerController::OnCommand(int command_id)
{
    hotkey_pressed = false;
    HandleCivCheckbox(command_id);

    if (model.GetSettings().autoreset_enabled && IsPoolControl(command_id)) model.ResetProgram(true);

    HandleEditionRadio(command_id);
    HandleDlcCheckbox(command_id);
    if (OpenWebPage(command_id) || OpenEditionLink(command_id)) return true;
    if (HandleMenuCommand(command_id)) return true;
    return HandleButtonCommand(command_id);
}

// The controls whose clicks change the pool, so auto-reset resets before them: the civ and DLC toggles, the edition
// rows, and Enable All and Disable All.
bool CivPickerController::IsPoolControl(int command_id) const
{
    const CivCatalog& catalog = model.Catalog();
    const int civ = command_id - CivPickerView::CIV_CHECKBOX_BASE_ID;
    const int dlc = command_id - CivPickerView::DLC_TOGGLE_BASE_ID;
    const int edition = command_id - CivPickerView::EDITION_ROW_BASE_ID;
    return (civ >= 0 && civ < catalog.CivCount()) || (dlc >= 0 && dlc < catalog.DlcCount()) ||
           (edition >= 0 && edition < catalog.EditionCount()) || command_id == IDC_BUTTON_ENABLEALL ||
           command_id == IDC_BUTTON_DISABLEALL;
}

void CivPickerController::HandleCivCheckbox(int command_id)
{
    const int civ_index = command_id - CivPickerView::CIV_CHECKBOX_BASE_ID;
    if (civ_index < 0 || civ_index >= model.Catalog().CivCount()) return;

    view.PlayAudio(SoundType::button);
    const Civ& clicked_civ = model.GetCiv(civ_index);
    if (view.IsCivChecked(civ_index)) model.AddCiv(clicked_civ.name);
    else model.RemoveCiv(clicked_civ.name);
    view.ValidateDlcToggle(clicked_civ.dlc);
}

void CivPickerController::HandleEditionRadio(int command_id)
{
    const int edition = command_id - CivPickerView::EDITION_ROW_BASE_ID;
    if (edition >= 0 && edition < model.Catalog().EditionCount()) model.SetEdition(edition);
}

void CivPickerController::HandleDlcCheckbox(int command_id)
{
    const int dlc = command_id - CivPickerView::DLC_TOGGLE_BASE_ID;
    if (dlc >= 0 && dlc < model.Catalog().DlcCount()) ToggleDlc(dlc);
}

bool CivPickerController::OpenWebPage(int command_id)
{
    for (const WebLink& link : WEB_LINKS)
    {
        if (command_id != link.command_id) continue;
        view.PlayAudio(SoundType::button);
        ShellExecute(0, 0, link.url, 0, 0, SW_SHOW);
        return true;
    }
    return false;
}

bool CivPickerController::HandleMenuCommand(int command_id)
{
    switch (command_id)
    {
        case IDM_ABOUT: OpenAbout(main_window); return true;
        case IDM_EXIT: Quit(); return true;
        case IDM_OPTIONS: OpenOptions(main_window); return true;
        case IDM_SAVELOG: SaveLog(SaveType::manual); return true;
        case IDM_LOADLOG: LoadLog(SaveType::manual); return true;
        case IDM_QUICKSAVE: SaveLog(SaveType::quick); return true;
        case IDM_QUICKLOAD: LoadLog(SaveType::quick); return true;
        case IDM_JOINLOBBY: JoinLobby(); return true;
        case IDM_UNDRAW: model.UndrawCiv(); return true;
        case IDM_REDRAW: model.RedrawCiv(); return true;
        case IDM_HOTKEYS: OpenHotkeys(main_window); return true;
        case IDM_SURVAPP: OpenSurvapp(); return true;
        case IDM_COPYLINK: CopyRoundLink(); return true;
        case IDM_SHARE: OpenShare(); return true;
        case IDM_OPENLINK: OpenShareLink(); return true;
        default: break;
    }
    return false;
}

bool CivPickerController::HandleButtonCommand(int command_id)
{
    switch (command_id)
    {
        case IDC_BUTTON_HISTORY: OpenHistory(main_window); return true;
        case IDC_BUTTON_DRAW: DrawCivWithinRateCap(); return true;
        case IDC_BUTTON_RESET: model.ResetProgram(false); return true;
        case IDC_BUTTON_ENABLEALL: EnableAll(true); return true;
        case IDC_BUTTON_DISABLEALL: DisableAll(true); return true;
        case IDC_BUTTON_CLEARLOG: ClearDrawnLog(); return true;
        case IDC_BUTTON_OPTIONS:
            view.DeactivateTooltips();
            OpenOptions(main_window);
            return true;
        case IDC_BUTTON_SURVAPP: OpenSurvapp(); return true;
        case IDC_BUTTON_TECHTREE: OpenTechTree(); return true;
        case IDC_CHECKBOX_REMAINLOG: ToggleRemainLog(); return true;
        case IDC_CHECKBOX_AUTORESET: ToggleAutoReset(); return true;
        case IDC_CHECKBOX_AUTOTOGGLE: ToggleAutoToggle(); return true;
        case IDC_ICON_CIV: view.PlayJingle(model.GetCurrentCiv()); return true;
        case IDC_ICON_EDITION: OpenEditionPage(); return true;
        case IDC_BUTTON_SHARE: CopyRoundLink(); return true;
        case IDC_BUTTON_POOLSHARE: CopyPoolLink(); return true;
        default: break;
    }
    return false;
}
