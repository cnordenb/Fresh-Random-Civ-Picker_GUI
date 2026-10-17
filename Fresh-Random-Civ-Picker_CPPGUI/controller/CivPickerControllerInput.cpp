#include "CivPickerController.h"

#include <shellapi.h>

#include "Resource.h"

// The user's input to the controller (CivPickerController) other than the hotkeys (CivPickerControllerHotkeys.cpp):
// the commands of the menus, buttons and toggles, each turned into one of the controller's actions.

namespace
{
    // Cap on user-triggered draws. Each draw starts its jingle as a new asynchronous sound, and Windows
    // frees the interrupted ones only gradually: holding Space at keyboard-repeat speed piled up about
    // 1 MB per draw. At 10 draws per second the memory use was measured to stay flat.
    const ULONGLONG MAX_DRAWS_PER_SECOND = 10;
    const ULONGLONG MIN_DRAW_INTERVAL_MS = 1000 / MAX_DRAWS_PER_SECOND;

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

void CivPickerController::DrawCivWithinRateCap()
{
    const ULONGLONG now = GetTickCount64();
    if (now - last_draw_tick < MIN_DRAW_INTERVAL_MS) return;
    last_draw_tick = now;
    model.DrawCiv(false, L"");
}
