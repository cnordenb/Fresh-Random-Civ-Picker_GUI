#include "CivPickerView.h"

#include <cwchar>

#include "Assertions.h"
#include "CivPickerController.h"
#include "Resource.h"

namespace
{
    const DialogDecoration OPTIONS_DECORATIONS[] = {
        { DecorationKind::group, { 27, 55, 241, 212 }, L"Display" },
        { DecorationKind::group, { 251, 55, 465, 212 }, L"Sound" },
        { DecorationKind::group, { 475, 55, 689, 212 }, L"Continuous freshness" },
        { DecorationKind::group, { 27, 222, 241, 339 }, L"When the program starts" },
        { DecorationKind::group, { 251, 222, 689, 339 }, L"Hotkeys" },
        { DecorationKind::text, { 262, 101, 330, 118 }, L"Jingle type" },
        { DecorationKind::hint, { 486, 143, 678, 194 }, L"When a round ends, these civs sit out the first draws of "
                                                         L"the next round." },
    };
    const DialogPart OPTIONS_PARTS[] = {
        { PartKind::checkbox, IDC_CHECKBOX_LABELS, { 38, 76, 230, 98 }, L"Show civ name" },
        { PartKind::checkbox, IDC_CHECKBOX_ICONS, { 38, 98, 230, 120 }, L"Show civ icon" },
        { PartKind::checkbox, IDC_CHECKBOX_CORNERLABEL, { 38, 120, 230, 142 }, L"Show round counter" },
        { PartKind::checkbox, IDC_CHECKBOX_TOOLTIPS, { 38, 142, 230, 164 }, L"Enable tooltips" },
        { PartKind::checkbox, IDC_CHECKBOX_JINGLES, { 262, 76, 454, 98 }, L"Enable civ jingles" },
        { PartKind::dropdown, IDC_LEGACY_OPTION, { 326, 98, 450, 120 }, nullptr },
        { PartKind::checkbox, IDC_CHECKBOX_SOUNDS, { 262, 120, 454, 142 }, L"Enable UI sounds" },
        { PartKind::checkbox, IDC_CHECKBOX_CONTFRESH, { 486, 76, 678, 98 }, L"Enable continuous freshness" },
        { PartKind::label, IDC_STATIC_CONTFRESHVALUE, { 486, 98, 678, 115 }, L"" },
        { PartKind::slider, IDC_SLIDER_CONTFRESHSTRENGTH, { 486, 117, 678, 141 }, nullptr },
        { PartKind::checkbox, IDC_CHECKBOX_STARTDRAW, { 38, 243, 230, 265 }, L"Draw a civ" },
        { PartKind::radio, IDC_RADIO_LOGGING, { 38, 265, 230, 287 }, L"Automatic civ pool logging" },
        { PartKind::radio, IDC_RADIO_STARTRESET, { 38, 287, 230, 309 }, L"Reset civ pool on startup" },
        { PartKind::button, IDC_BUTTON_HOTKEYS, { 262, 243, 358, 266 }, L"Show Hotkeys" },
        { PartKind::button, IDC_BUTTON_CUSTOMISEHOTKEYS, { 366, 243, 462, 266 }, L"Customise" },
        { PartKind::default_button, IDOK, { 614, 351, 689, 374 }, L"OK" },
    };

    constexpr int LOBBY_ERROR_TOP = 106;
    constexpr int LOBBY_ERROR_ROOM = 25;
    const DialogDecoration LOBBY_DECORATIONS[] = {
        { DecorationKind::text, { 27, 55, 447, 72 }, L"Enter AoE2DE lobby code:" },
    };
    const DialogPart LOBBY_PARTS[] = {
        { PartKind::text_field, IDC_EDIT_LOBBYCODE, { 27, 76, 447, 98 }, L"123456789" },
        { PartKind::error_label, IDC_LOBBY_ERROR, { 27, LOBBY_ERROR_TOP, 447, 123 }, L"" },
        { PartKind::label, IDC_STATIC, { 27, LOBBY_ERROR_TOP, 447, 123 }, L"NB: AoE2DE must be already running." },
        { PartKind::default_button, IDC_BUTTON_OK, { 206, 143, 281, 166 }, L"Join" },
        { PartKind::button, IDC_BUTTON_SPEC, { 289, 143, 364, 166 }, L"Spectate" },
        { PartKind::button, IDC_BUTTON_CANCEL, { 372, 143, 447, 166 }, L"Cancel" },
    };

    const DialogDecoration HISTORY_DECORATIONS[] = {
        { DecorationKind::text, { 27, 58, 110, 75 }, L"Search civ:" },
        { DecorationKind::text, { 27, 88, 110, 105 }, L"Select civ:" },
        { DecorationKind::text, { 370, 55, 425, 72 }, L"Added:" },
    };
    const DialogPart HISTORY_PARTS[] = {
        { PartKind::text_field, IDC_HIS_SEARCH, { 115, 55, 275, 77 }, nullptr },
        { PartKind::dropdown, IDC_HISTORY_COMBO, { 115, 85, 275, 107 }, nullptr },
        { PartKind::picture, IDC_CIV_ICON, { 296, 55, 348, 107 }, nullptr },
        { PartKind::label, IDC_HIS_INFO, { 425, 55, 573, 72 }, L"" },
        { PartKind::label, IDC_HIS_INFO_EN1, { 370, 72, 425, 89 }, L"Enabled:" },
        { PartKind::label, IDC_HIS_INFO_EN, { 425, 72, 573, 89 }, L"" },
        { PartKind::label, IDC_HIS_INFO_DR1, { 370, 89, 425, 106 }, L"Drawn:" },
        { PartKind::label, IDC_HIS_INFO_DR, { 425, 89, 573, 106 }, L"" },
        { PartKind::checkbox, IDC_POOL_CHECKBOX, { 27, 117, 167, 139 }, L"Show pool info" },
        { PartKind::checkbox, IDC_HIST_JINGLE, { 175, 117, 285, 139 }, L"Play jingle" },
        { PartKind::log, IDC_HISTORY_EDIT, { 27, 147, 573, 447 }, nullptr },
        { PartKind::default_button, IDOK, { 498, 459, 573, 482 }, L"OK" },
    };

    template <typename Item, int Count> constexpr int CountOf(const Item (&)[Count]) { return Count; }

    const DialogLayout OPTIONS_LAYOUT = { L"Options", { 716, 401 }, OPTIONS_PARTS, CountOf(OPTIONS_PARTS),
                                          OPTIONS_DECORATIONS, CountOf(OPTIONS_DECORATIONS), nullptr, 0, IDOK };
    const DialogLayout LOBBY_LAYOUT = { L"Join Lobby", { 474, 193 }, LOBBY_PARTS, CountOf(LOBBY_PARTS),
                                        LOBBY_DECORATIONS, CountOf(LOBBY_DECORATIONS), nullptr, 0, IDC_BUTTON_OK };
    const DialogLayout HISTORY_LAYOUT = { L"History", { 600, 509 }, HISTORY_PARTS, CountOf(HISTORY_PARTS),
                                          HISTORY_DECORATIONS, CountOf(HISTORY_DECORATIONS), nullptr, 0, IDOK };

    constexpr int HISTORY_TEXT_LENGTH = 256;
    constexpr int MAX_ADDED_TEXT = 128;

    UINT ToCheckState(bool checked) { return checked ? BST_CHECKED : BST_UNCHECKED; }
}

void CivPickerView::ShowOptionsDialog(HWND owner)
{
    if (!c_assert(owner != NULL)) return;
    (void)RunDialog({ owner, OptionsDialogProc, reinterpret_cast<LPARAM>(this), &OPTIONS_LAYOUT });
}

void CivPickerView::ShowHistoryDialog(HWND owner)
{
    if (!c_assert(owner != NULL)) return;
    (void)RunDialog({ owner, HistoryDialogProc, reinterpret_cast<LPARAM>(this), &HISTORY_LAYOUT });
}

CivPickerView::LobbyInput CivPickerView::AskLobbyCode(HWND owner)
{
    lobby_code_input.clear();
    if (!c_assert(owner != NULL)) return { -1, lobby_code_input };
    lobby_error_shown = false;
    const INT_PTR result = RunDialog({ owner, JoinLobbyDialogProc, reinterpret_cast<LPARAM>(this), &LOBBY_LAYOUT });
    return { result, lobby_code_input };
}

INT_PTR CALLBACK CivPickerView::OptionsDialogProc(HWND dialog, UINT message, WPARAM wParam,
                                                  LPARAM lParam) noexcept
{
    CivPickerView* view = AttachDialog(dialog, message, lParam);
    if (view == nullptr) return FALSE;
    if (message == WM_INITDIALOG)
    {
        const INT_PTR result = view->BeginDialog(dialog);
        view->InitialiseOptionsDialog(dialog);
        return result;
    }
    const ParchmentDialogs::Handled frame = view->dialogs.HandleMessage({ dialog, message, wParam, lParam });
    if (frame.handled) return frame.result;
    if (message == WM_HSCROLL) view->OnContfreshSlider(dialog, reinterpret_cast<HWND>(lParam));
    if (message != WM_COMMAND) return FALSE;
    return view->OnOptionsCommand(dialog, LOWORD(wParam), HIWORD(wParam)) ? TRUE : FALSE;
}

void CivPickerView::InitialiseOptionsDialog(HWND dialog)
{
    if (!c_assert(dialog != NULL)) return;
    const Settings& settings = model.GetSettings();
    CheckDlgButton(dialog, IDC_CHECKBOX_LABELS, ToCheckState(settings.civ_labels_enabled));
    CheckDlgButton(dialog, IDC_CHECKBOX_CORNERLABEL, ToCheckState(settings.iteration_label_enabled));
    CheckDlgButton(dialog, IDC_CHECKBOX_ICONS, ToCheckState(settings.icons_enabled));
    CheckDlgButton(dialog, IDC_CHECKBOX_JINGLES, ToCheckState(settings.jingles_enabled));
    CheckDlgButton(dialog, IDC_CHECKBOX_SOUNDS, ToCheckState(settings.ui_sounds_enabled));
    CheckDlgButton(dialog, IDC_CHECKBOX_TOOLTIPS, ToCheckState(settings.tooltips_enabled));
    CheckDlgButton(dialog, IDC_CHECKBOX_STARTDRAW, ToCheckState(settings.draw_on_startup));
    CheckDlgButton(dialog, IDC_CHECKBOX_CONTFRESH, ToCheckState(settings.contfresh_enabled));
    CheckRadioButton(dialog, IDC_RADIO_LOGGING, IDC_RADIO_STARTRESET,
                     settings.persistent_logging ? IDC_RADIO_LOGGING : IDC_RADIO_STARTRESET);

    HWND jingle_type = GetDlgItem(dialog, IDC_LEGACY_OPTION);
    if (!c_assert(jingle_type != NULL)) return;
    const std::wstring& first_edition = model.Catalog().EditionAt(0).name;
    SendMessage(jingle_type, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(first_edition.c_str()));
    SendMessage(jingle_type, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Legacy"));
    SendMessage(jingle_type, CB_SETCURSEL, settings.legacy_jingle_enabled ? 1 : 0, 0);
    if (!settings.jingles_enabled) ShowJingleTypeOption(dialog, false);

    HWND freshness_slider = GetDlgItem(dialog, IDC_SLIDER_CONTFRESHSTRENGTH);
    if (!c_assert(freshness_slider != NULL)) return;
    SendMessage(freshness_slider, TBM_SETRANGE, TRUE, MAKELPARAM(1, model.FreshnessMaximum()));
    SendMessage(freshness_slider, TBM_SETPOS, TRUE, model.FreshnessLimit());
    ShowContfreshOptions(dialog, settings.contfresh_enabled);
    SetFreshnessText(dialog, model.FreshnessLimit());
}

void CivPickerView::SetFreshnessText(HWND dialog, int civs) const
{
    if (!c_assert(dialog != NULL) || !c_assert(civs >= 1 && civs <= model.FreshnessMaximum())) return;
    wchar_t text[64] = L"Needs at least 2 civs in the pool";
    if (model.GetCustomMaxCivs() >= 2)
    {
        const wchar_t* const noun = civs == 1 ? L"civ" : L"civs";
        const wchar_t* const most = civs == model.FreshnessMaximum() ? L" (the most)" : L"";
        (void)_snwprintf_s(text, _TRUNCATE, L"Exclude the last %d %ls%ls", civs, noun, most);
    }
    SetDlgItemText(dialog, IDC_STATIC_CONTFRESHVALUE, text);
}

void CivPickerView::OnContfreshSlider(HWND dialog, HWND slider)
{
    if (!c_assert(dialog != NULL)) return;
    if (slider != GetDlgItem(dialog, IDC_SLIDER_CONTFRESHSTRENGTH)) return;
    const int civs = static_cast<int>(SendMessage(slider, TBM_GETPOS, 0, 0));
    SetFreshnessText(dialog, civs);
    controller.OnFreshnessChanged(civs);
}

bool CivPickerView::OnOptionsCommand(HWND dialog, int command_id, int notification)
{
    if (!c_assert(dialog != NULL)) return false;
    OptionsEvent options_event = {};
    options_event.command_id = command_id;
    options_event.notification = notification;
    options_event.ui_sounds = IsDlgButtonChecked(dialog, IDC_CHECKBOX_SOUNDS) == BST_CHECKED;
    options_event.civ_labels = IsDlgButtonChecked(dialog, IDC_CHECKBOX_LABELS) == BST_CHECKED;
    options_event.iteration_label = IsDlgButtonChecked(dialog, IDC_CHECKBOX_CORNERLABEL) == BST_CHECKED;
    options_event.icons = IsDlgButtonChecked(dialog, IDC_CHECKBOX_ICONS) == BST_CHECKED;
    options_event.jingles = IsDlgButtonChecked(dialog, IDC_CHECKBOX_JINGLES) == BST_CHECKED;
    options_event.tooltips = IsDlgButtonChecked(dialog, IDC_CHECKBOX_TOOLTIPS) == BST_CHECKED;
    options_event.draw_on_startup = IsDlgButtonChecked(dialog, IDC_CHECKBOX_STARTDRAW) == BST_CHECKED;
    options_event.contfresh = IsDlgButtonChecked(dialog, IDC_CHECKBOX_CONTFRESH) == BST_CHECKED;
    options_event.legacy_jingle = SendMessage(GetDlgItem(dialog, IDC_LEGACY_OPTION), CB_GETCURSEL, 0, 0) == 1;

    if (!controller.OnOptionsCommand(dialog, options_event)) return false;
    EndDialog(dialog, command_id);
    return true;
}

void CivPickerView::ShowJingleTypeOption(HWND dialog, bool shown) const
{
    if (!c_assert(dialog != NULL)) return;
    (void)EnableWindow(GetDlgItem(dialog, IDC_LEGACY_OPTION), shown ? TRUE : FALSE);
}

void CivPickerView::ShowContfreshOptions(HWND dialog, bool shown) const
{
    if (!c_assert(dialog != NULL)) return;
    const bool usable = shown && model.GetCustomMaxCivs() >= 2;
    (void)EnableWindow(GetDlgItem(dialog, IDC_SLIDER_CONTFRESHSTRENGTH), usable ? TRUE : FALSE);
}

INT_PTR CALLBACK CivPickerView::JoinLobbyDialogProc(HWND dialog, UINT message, WPARAM wParam,
                                                    LPARAM lParam) noexcept
{
    CivPickerView* view = AttachDialog(dialog, message, lParam);
    if (view == nullptr) return FALSE;
    if (message == WM_INITDIALOG) return view->BeginDialog(dialog);
    const ParchmentDialogs::Handled frame = view->dialogs.HandleMessage({ dialog, message, wParam, lParam });
    if (frame.handled) return frame.result;
    if (message != WM_COMMAND) return FALSE;

    const int command_id = LOWORD(wParam);
    if (command_id == IDC_BUTTON_OK || command_id == IDC_BUTTON_SPEC)
        return view->SubmitLobbyCode(dialog, command_id) ? TRUE : FALSE;
    if (command_id != IDCANCEL && command_id != IDC_BUTTON_CANCEL) return FALSE;

    view->PlayAudio(SoundType::button);
    EndDialog(dialog, command_id);
    return TRUE;
}

bool CivPickerView::SubmitLobbyCode(HWND dialog, int command_id)
{
    if (!c_assert(dialog != NULL)) return false;
    if (!c_assert(command_id == IDC_BUTTON_OK || command_id == IDC_BUTTON_SPEC)) return false;
    PlayAudio(SoundType::button);
    wchar_t entered_code[256] = {};
    GetDlgItemText(dialog, IDC_EDIT_LOBBYCODE, entered_code, 256);
    lobby_code_input = entered_code;

    if (CivPickerController::IsValidLobbyCode(lobby_code_input))
    {
        EndDialog(dialog, command_id);
        return true;
    }
    PlayAudio(SoundType::error);
    const wchar_t* error = command_id == IDC_BUTTON_SPEC
                               ? L"Enter a lobby code such as 123456789 or aoe2de://1/123456789."
                               : L"Enter a lobby code such as 123456789 or aoe2de://0/123456789.";
    SetDlgItemText(dialog, IDC_LOBBY_ERROR, error);
    if (!lobby_error_shown && dialogs.Grow(dialog, { LOBBY_ERROR_ROOM, LOBBY_ERROR_TOP, IDC_LOBBY_ERROR }))
        lobby_error_shown = true;
    (void)ShowWindow(GetDlgItem(dialog, IDC_LOBBY_ERROR), SW_SHOW);
    return false;
}

INT_PTR CALLBACK CivPickerView::HistoryDialogProc(HWND dialog, UINT message, WPARAM wParam,
                                                  LPARAM lParam) noexcept
{
    CivPickerView* view = AttachDialog(dialog, message, lParam);
    if (view == nullptr) return FALSE;
    if (message == WM_INITDIALOG)
    {
        const INT_PTR result = view->BeginDialog(dialog);
        view->InitialiseHistoryDialog(dialog);
        return result;
    }
    const ParchmentDialogs::Handled frame = view->dialogs.HandleMessage({ dialog, message, wParam, lParam });
    if (frame.handled) return frame.result;
    if (message != WM_COMMAND) return FALSE;
    return view->OnHistoryCommand(dialog, LOWORD(wParam), HIWORD(wParam)) ? TRUE : FALSE;
}

void CivPickerView::InitialiseHistoryDialog(HWND dialog)
{
    if (!c_assert(dialog != NULL)) return;
    const CivCatalog& catalog = model.Catalog();
    history_selected_civ = model.GetCurrentCiv();
    if (model.FindCivIndex(history_selected_civ) == CivCatalog::NONE)
        history_selected_civ = catalog.CivAt(catalog.DisplayCiv(0)).name;

    UpdateCivAddedInfo(dialog, model.GetCiv(history_selected_civ).dlc);
    UpdateCivPoolInfo(dialog, history_selected_civ);
    SetDlgItemTextW(dialog, IDC_HISTORY_EDIT, model.FetchHistory(history_selected_civ).c_str());

    HWND civ_list_box = GetDlgItem(dialog, IDC_HISTORY_COMBO);
    for (int position = 0; position < catalog.CivCount() && position < MAX_CIVS; position++)
    {
        const std::wstring& name = catalog.CivAt(catalog.DisplayCiv(position)).name;
        SendMessage(civ_list_box, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(name.c_str()));
    }
    ShowHistoryCivIcon(dialog);
    SendMessage(civ_list_box, CB_SETCURSEL, static_cast<WPARAM>(HistoryListPosition(history_selected_civ)), 0);

    CheckDlgButton(dialog, IDC_POOL_CHECKBOX, BST_CHECKED);
    CheckDlgButton(dialog, IDC_HIST_JINGLE, BST_UNCHECKED);
    if (!model.GetSettings().jingles_enabled) ShowWindow(GetDlgItem(dialog, IDC_HIST_JINGLE), SW_HIDE);
}

void CivPickerView::ShowHistoryCivIcon(HWND dialog)
{
    if (!c_assert(dialog != NULL)) return;
    const int cell = CivAtlasCell(history_selected_civ);
    if (!c_assert(cell >= 0 && cell <= model.Catalog().CivCount())) return;
    const int column = cell % CIV_ATLAS_COLUMNS;
    const int row = cell / CIV_ATLAS_COLUMNS;
    dialogs.ShowPicture(dialog, { ImageId::civ_atlas, { column * CIV_ATLAS_CELL, row * CIV_ATLAS_CELL,
                                                        (column + 1) * CIV_ATLAS_CELL, (row + 1) * CIV_ATLAS_CELL } });
}

int CivPickerView::HistoryListPosition(const std::wstring& civ_name) const
{
    const CivCatalog& catalog = model.Catalog();
    for (int position = 0; position < catalog.CivCount() && position < MAX_CIVS; position++)
    {
        if (catalog.CivAt(catalog.DisplayCiv(position)).name == civ_name) return position;
    }
    return 0;
}

bool CivPickerView::OnHistoryCommand(HWND dialog, int command_id, int notification)
{
    if (!c_assert(dialog != NULL)) return false;
    const bool show_pool_info = IsDlgButtonChecked(dialog, IDC_POOL_CHECKBOX) == BST_CHECKED;
    if (command_id == IDC_POOL_CHECKBOX && notification == BN_CLICKED)
    {
        PlayAudio(SoundType::button);
        if (show_pool_info) UpdateCivPoolInfo(dialog, history_selected_civ);
        else HidePoolInfo(dialog);
    }

    const bool history_jingle = IsDlgButtonChecked(dialog, IDC_HIST_JINGLE) == BST_CHECKED;
    if (command_id == IDC_HIST_JINGLE && notification == BN_CLICKED)
    {
        if (history_jingle) PlayJingle(history_selected_civ);
        else StopSound();
    }

    if (notification == CBN_DROPDOWN) PlayAudio(SoundType::hover);
    if (command_id == IDC_HISTORY_COMBO && notification == CBN_SELCHANGE)
        OnHistorySelectionChanged(dialog, show_pool_info, history_jingle);
    if (command_id == IDC_HIS_SEARCH && notification == EN_CHANGE) OnHistorySearchChanged(dialog);
    if (notification == CBN_DROPDOWN) SetWindowText(GetDlgItem(dialog, IDC_HIS_SEARCH), L"");

    if (command_id != IDOK && command_id != IDCANCEL) return false;
    PlayAudio(SoundType::button);
    EndDialog(dialog, command_id);
    return true;
}

void CivPickerView::OnHistorySelectionChanged(HWND dialog, bool show_pool_info, bool history_jingle)
{
    if (!c_assert(dialog != NULL)) return;
    HWND civ_list_box = GetDlgItem(dialog, IDC_HISTORY_COMBO);
    const int selected_index = static_cast<int>(SendMessage(civ_list_box, CB_GETCURSEL, 0, 0));
    if (selected_index == CB_ERR) return;
    const CivCatalog& catalog = model.Catalog();
    if (!c_assert(selected_index >= 0 && selected_index < catalog.CivCount())) return;

    const int previous_index = model.GetCivIndex(history_selected_civ);
    history_selected_civ = catalog.CivAt(catalog.DisplayCiv(selected_index)).name;
    if (previous_index != model.GetCivIndex(history_selected_civ))
    {
        if (history_jingle) PlayJingle(history_selected_civ);
        else PlayAudio(SoundType::view);
        ShowHistoryCivIcon(dialog);
    }

    UpdateCivAddedInfo(dialog, model.GetCiv(history_selected_civ).dlc);
    if (show_pool_info) UpdateCivPoolInfo(dialog, history_selected_civ);
    else HidePoolInfo(dialog);
    SetDlgItemText(dialog, IDC_HISTORY_EDIT, model.FetchHistory(history_selected_civ).c_str());
}

void CivPickerView::OnHistorySearchChanged(HWND dialog)
{
    if (!c_assert(dialog != NULL)) return;
    HWND search_box = GetDlgItem(dialog, IDC_HIS_SEARCH);
    HWND civ_list_box = GetDlgItem(dialog, IDC_HISTORY_COMBO);
    wchar_t search_text[HISTORY_TEXT_LENGTH] = {};
    GetWindowText(search_box, search_text, HISTORY_TEXT_LENGTH);
    const int search_length = lstrlenW(search_text);

    if (search_length > history_search_length)
    {
        int item_count = static_cast<int>(SendMessage(civ_list_box, CB_GETCOUNT, 0, 0));
        if (item_count > MAX_CIVS) item_count = MAX_CIVS;
        for (int i = 0; i < item_count; ++i)
        {
            wchar_t item_text[HISTORY_TEXT_LENGTH] = L"";
            SendMessage(civ_list_box, CB_GETLBTEXT, static_cast<WPARAM>(i), reinterpret_cast<LPARAM>(item_text));
            if (_wcsnicmp(item_text, search_text, wcslen(search_text)) != 0) continue;

            SendMessage(civ_list_box, CB_SETCURSEL, static_cast<WPARAM>(i), 0);
            OnHistorySelectionChanged(dialog, IsDlgButtonChecked(dialog, IDC_POOL_CHECKBOX) == BST_CHECKED,
                                      IsDlgButtonChecked(dialog, IDC_HIST_JINGLE) == BST_CHECKED);
            break;
        }
    }
    history_search_length = search_length;
}

void CivPickerView::UpdateCivAddedInfo(HWND dialog, int dlc) const
{
    if (!c_assert(dialog != NULL) || !c_assert(dlc >= 0 && dlc < model.Catalog().DlcCount())) return;
    const DlcInfo& info = model.Catalog().DlcAt(dlc);
    wchar_t added[MAX_ADDED_TEXT] = L"";
    (void)_snwprintf_s(added, _TRUNCATE, L"%ls (%ls)", info.released.c_str(), info.name.c_str());
    SetDlgItemText(dialog, IDC_HIS_INFO, added);
}

void CivPickerView::UpdateCivPoolInfo(HWND dialog, const std::wstring& civ_name) const
{
    if (!c_assert(dialog != NULL) || !c_assert(!civ_name.empty())) return;
    const CivCatalog& catalog = model.Catalog();
    const bool drawn_civ = model.FindCivIndex(civ_name) != CivCatalog::NONE;
    const std::wstring& shown_civ = drawn_civ ? civ_name : catalog.CivAt(catalog.DisplayCiv(0)).name;
    SetDlgItemText(dialog, IDC_HIS_INFO_EN1, L"Enabled:");
    SetDlgItemText(dialog, IDC_HIS_INFO_DR1, L"Drawn:");
    SetDlgItemText(dialog, IDC_HIS_INFO_EN, model.GetCiv(shown_civ).enabled ? L"Yes" : L"No");
    SetDlgItemText(dialog, IDC_HIS_INFO_DR, model.HasBeenDrawn(civ_name) ? L"Yes" : L"No");
}

void CivPickerView::HidePoolInfo(HWND dialog) const
{
    if (!c_assert(dialog != NULL)) return;
    SetDlgItemText(dialog, IDC_HIS_INFO_EN, L"");
    SetDlgItemText(dialog, IDC_HIS_INFO_DR, L"");
    SetDlgItemText(dialog, IDC_HIS_INFO_EN1, L"");
    SetDlgItemText(dialog, IDC_HIS_INFO_DR1, L"");
}
