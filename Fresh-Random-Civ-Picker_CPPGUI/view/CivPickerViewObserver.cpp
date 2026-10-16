#include "CivPickerView.h"

#include "Assertions.h"
#include "CivPickerController.h"

// The view as the model's observer (CivPickerView): the model's notifications, and the labels, logs and tooltips
// they bring up to date from the model's state.

// ---------------------------------------------------------------------------------------------------
// Rendering model state (Observer notifications, HFDP ch. 2 and 12)

void CivPickerView::OnPoolChanged(int civ_index)
{
    if (!c_assert(civ_index >= 0 && civ_index < model.Catalog().CivCount())) return;
    civ_toggles[civ_index].SetChecked(model.GetCiv(civ_index).enabled);
    SetControlText(label_corner.Window(), IterationText().c_str());
    UpdateRemainingLog();
    UpdateDrawnLog(DrawnLogUpdate::count_only);
    if (!model.IsBatchToggle()) UpdateEnabledCivsCounter();
}

void CivPickerView::OnBulkPoolChanged()
{
    ValidateAllDlcToggles();
    UpdateEnabledCivsCounter();
}

void CivPickerView::OnDlcPoolChanged() { UpdateEnabledCivsCounter(); }

void CivPickerView::OnEditionChanged()
{
    if (!c_assert(main_window != NULL)) return;
    if (!model.IsStartingUp()) PlayAudio(SoundType::view);
    ShowEditionControls(model.GetEdition());
    ValidateAllDlcToggles();
    UpdateEnabledCivsCounter();
}

void CivPickerView::OnResetStarted(bool auto_reset)
{
    if (!model.IsStartingUp() && !auto_reset && !model.GetSettings().jingles_enabled) PlayAudio(SoundType::button);
}

void CivPickerView::OnDrawSequenceBroken() { UpdateDrawnLog(DrawnLogUpdate::blank_line); }

void CivPickerView::OnPoolReset(bool auto_reset)
{
    if (!c_assert(main_window != NULL)) return;
    const Settings& settings = model.GetSettings();
    SetControlText(label_corner.Window(), (L"0/" + std::to_wstring(model.GetCustomMaxCivs())).c_str());
    if (!settings.iteration_label_enabled) ShowControl(label_corner.Window(), false);
    SetControlText(label_centre.Window(), L"?");
    if (!settings.civ_labels_enabled) ShowControl(label_centre.Window(), false);
    ShowCivIconFor(L"Random");

    if (!model.IsStartingUp() && !auto_reset && settings.jingles_enabled && current_tab != CIV_POOL_TAB)
        PlayJingle(model.GetCurrentCiv());
    UpdateRemainingLog();
    UpdateTooltips(false);
}

void CivPickerView::OnDrawStarted()
{
    if (!model.IsStartingUp() && !model.GetSettings().jingles_enabled) PlayAudio(SoundType::button);
}

void CivPickerView::OnEmptyPoolDraw()
{
    PlayAudio(SoundType::error);
    SetControlText(label_centre.Window(), L"Empty pool!");
}

void CivPickerView::OnCivDrawn()
{
    const Settings& settings = model.GetSettings();
    const std::wstring& current_civ = model.GetCurrentCiv();
    if (!c_assert(!current_civ.empty())) return;
    SetControlText(label_corner.Window(), IterationText().c_str());
    if (!settings.iteration_label_enabled) ShowControl(label_corner.Window(), false);
    SetControlText(label_centre.Window(), current_civ.c_str());
    if (!settings.civ_labels_enabled) ShowControl(label_centre.Window(), false);
    ShowCivIconFor(current_civ);

    UpdateDrawnLog(DrawnLogUpdate::entry);
    UpdateRemainingLog();
    if (!model.IsStartingUp()) PlayJingle(current_civ);
    UpdateTooltips(true);
}

void CivPickerView::OnUndrawAccepted() { PlayAudio(SoundType::button); }

void CivPickerView::OnCivUndrawn()
{
    const Settings& settings = model.GetSettings();
    const std::wstring& current_civ = model.GetCurrentCiv();
    if (!c_assert(!current_civ.empty())) return;
    ShowCivIconFor(current_civ);
    PlayJingle(current_civ);
    SetControlText(label_centre.Window(), current_civ == L"Random" ? L"?" : current_civ.c_str());
    if (!settings.civ_labels_enabled) ShowControl(label_centre.Window(), false);

    SetControlText(label_drawncount.Window(), (L"Drawn: " + IterationText()).c_str());
    SetControlText(label_corner.Window(), IterationText().c_str());
    if (!settings.iteration_label_enabled || current_tab != DRAW_TAB) ShowControl(label_corner.Window(), false);
    if (model.GetIterator() == 0) ShowCivIconFor(L"Random");

    RemoveNewestDrawnLogEntry();
    UpdateTooltips(true);
    UpdateRemainingLog();
}

void CivPickerView::OnCivRedrawn()
{
    const std::wstring& current_civ = model.GetCurrentCiv();
    if (!c_assert(!current_civ.empty())) return;
    ShowCivIconFor(current_civ);
    PlayJingle(current_civ);
    SetControlText(label_centre.Window(), current_civ.c_str());
    SetControlText(label_corner.Window(), IterationText().c_str());
    if (!model.GetSettings().iteration_label_enabled || current_tab != DRAW_TAB)
        ShowControl(label_corner.Window(), false);

    UpdateDrawnLog(DrawnLogUpdate::entry);
    UpdateRemainingLog();
    UpdateTooltips(true);
}

void CivPickerView::OnActionRejected() { PlayAudio(SoundType::error); }

void CivPickerView::OnLogLoaded(SaveType type)
{
    if (!c_assert(type == SaveType::automatic || type == SaveType::manual || type == SaveType::quick)) return;
    const Settings& settings = model.GetSettings();
    const std::wstring& current_civ = model.GetCurrentCiv();
    const std::wstring iteration_text = IterationText();
    SetControlText(label_corner.Window(), iteration_text.c_str());
    if (!settings.iteration_label_enabled) ShowControl(label_corner.Window(), false);
    if (model.GetIterator() == 0) SetControlText(label_drawncount.Window(), (L"Drawn: " + iteration_text).c_str());

    ShowCivIconFor(current_civ);
    if (!settings.icons_enabled) ShowControl(civ_icon.Window(), false);
    UpdateRemainingLog();

    const bool nothing_drawn = type == SaveType::automatic && current_civ == L"Random";
    UpdateTooltips(!nothing_drawn);
    SetControlText(label_centre.Window(), nothing_drawn ? L"?" : current_civ.c_str());
    if (!settings.civ_labels_enabled) ShowControl(label_centre.Window(), false);
}

void CivPickerView::OnModelError(const std::wstring& message, bool fatal)
{
    if (!c_assert(!message.empty())) return;
    ShowInformation(message.c_str(), L"Error");
    if (fatal) controller.OnFatalError();
}

// ---------------------------------------------------------------------------------------------------
// Labels, logs and tooltips

std::wstring CivPickerView::IterationText() const
{
    return std::to_wstring(model.GetIterator()) + L"/" + std::to_wstring(model.GetCustomMaxCivs());
}

// The civ's cell in the civ icon atlas, which holds the civs in the order of their IDs and Random after them.
int CivPickerView::CivAtlasCell(const std::wstring& civ_name) const
{
    const int random_cell = model.Catalog().CivCount();
    if (!c_assert(!civ_name.empty())) return random_cell;
    const int civ = model.FindCivIndex(civ_name);
    return civ != CivCatalog::NONE ? civ : random_cell;
}

void CivPickerView::ShowCivIconFor(const std::wstring& civ_name)
{
    const int cell = CivAtlasCell(civ_name);
    if (!c_assert(cell >= 0 && cell <= model.Catalog().CivCount())) return;
    const int column = cell % CIV_ATLAS_COLUMNS;
    const int row = cell / CIV_ATLAS_COLUMNS;
    civ_icon.ShowPicture({ ImageId::civ_atlas, { column * CIV_ATLAS_CELL, row * CIV_ATLAS_CELL,
                                                 (column + 1) * CIV_ATLAS_CELL, (row + 1) * CIV_ATLAS_CELL } });
}

void CivPickerView::SetControlText(HWND control, const wchar_t* text)
{
    if (!c_assert(text != nullptr) || control == NULL) return;
    // A label that didn't take the text keeps showing the old one until the next update.
    (void)SetWindowTextW(control, text);
}

void CivPickerView::UpdateDrawnLog(DrawnLogUpdate update)
{
    if (!c_assert(drawn_log.Window() != NULL)) return;
    SetControlText(label_drawncount.Window(), (L"Drawn: " + IterationText()).c_str());

    const int log_length = GetWindowTextLength(drawn_log.Window());
    if (!c_assert(log_length >= 0)) return;
    std::wstring log_text(static_cast<size_t>(log_length) + 1, L'\0');
    GetWindowText(drawn_log.Window(), &log_text[0], log_length + 1);
    log_text.pop_back();
    const bool starts_with_blank_line = log_text.substr(0, 2) == L"\r\n";

    if (model.IsCacheLoading())
    {
        if (!starts_with_blank_line) return;
        log_text.erase(0, 2);
        SetControlText(drawn_log.Window(), log_text.c_str());
        drawn_log_linecount--;
        return;
    }

    if (update == DrawnLogUpdate::entry)
        AddDrawnLogEntry(log_text, model.GetCurrentCiv() + L" (" + IterationText() + L")\r\n");
    else if (update == DrawnLogUpdate::blank_line && !model.IsResetState() && !starts_with_blank_line)
        AddDrawnLogEntry(log_text, L"\r\n");
}

void CivPickerView::AddDrawnLogEntry(std::wstring& log_text, const std::wstring& entry)
{
    if (!c_assert(!entry.empty())) return;
    if (!c_assert(drawn_log_linecount >= 0 && drawn_log_linecount <= MAX_DRAWNLOG_LINECOUNT)) return;
    log_text = entry + log_text;
    if (drawn_log_linecount == MAX_DRAWNLOG_LINECOUNT)
    {
        const size_t last_newline = log_text.rfind(L"\r");
        if (last_newline != std::wstring::npos) log_text.erase(last_newline);
        SetControlText(drawn_log.Window(), log_text.c_str());
        return;
    }
    SetControlText(drawn_log.Window(), log_text.c_str());
    drawn_log_linecount++;
}

void CivPickerView::RemoveNewestDrawnLogEntry()
{
    if (!c_assert(drawn_log.Window() != NULL)) return;
    const int log_length = GetWindowTextLength(drawn_log.Window());
    if (log_length <= 0) return;

    std::wstring log_text(static_cast<size_t>(log_length) + 1, L'\0');
    GetWindowText(drawn_log.Window(), &log_text[0], log_length + 1);
    log_text.pop_back();
    const size_t first_line_end = log_text.find(L"\r\n");
    if (first_line_end == std::wstring::npos) return;

    log_text = log_text.substr(first_line_end + 2);
    SetControlText(drawn_log.Window(), log_text.c_str());
    drawn_log_linecount--;
}

void CivPickerView::UpdateRemainingLog()
{
    if (!c_assert(remaining_log.Window() != NULL && label_remainingcount.Window() != NULL)) return;
    // In the order of their names, as the Civ Pool tab shows them.
    std::wstring remaining_text;
    const CivCatalog& catalog = model.Catalog();
    for (int position = 0; position < catalog.CivCount() && position < MAX_CIVS; position++)
    {
        const std::wstring& pool_entry = model.GetPoolEntry(catalog.DisplayCiv(position));
        if (model.GetCiv(pool_entry).enabled && pool_entry != L"") remaining_text += pool_entry + L"\r\n";
    }
    const std::wstring remaining_label =
        L"Remaining: " + std::to_wstring(model.GetRemaining()) + L"/" + std::to_wstring(model.GetCustomMaxCivs());
    SetControlText(remaining_log.Window(), remaining_text.c_str());
    SetControlText(label_remainingcount.Window(), remaining_label.c_str());
}

void CivPickerView::UpdateEnabledCivsCounter()
{
    if (!c_assert(label_enabledcount.Window() != NULL)) return;
    const int edition_civ_amount = model.Catalog().EditionCivCount(model.GetEdition());

    const std::wstring enabled_text =
        L"Enabled: " + std::to_wstring(model.GetCustomMaxCivs()) + L"/" + std::to_wstring(edition_civ_amount);
    SetControlText(label_enabledcount.Window(), enabled_text.c_str());
}

void CivPickerView::UpdateTooltips(bool drawn)
{
    if (drawn)
    {
        const std::wstring& current_civ = model.GetCurrentCiv();
        UpdateTooltipText(TOOLTIP_TECHTREE, L"Opens the tech tree for the " + current_civ + TECHTREE_HOTKEY_LINE);
        UpdateTooltipText(TOOLTIP_HISTORY, L"Opens the game's history info for the " + current_civ + L"\nHotkey: H");
        return;
    }
    UpdateTooltipText(TOOLTIP_TECHTREE, std::wstring(L"Opens the tech tree") + TECHTREE_HOTKEY_LINE);
    UpdateTooltipText(TOOLTIP_HISTORY, L"Opens the game's history info\nHotkey: H");
}

// Changes a tooltip's text; a tooltip on show changes with it, as the web version's does.
void CivPickerView::UpdateTooltipText(int tooltip_id, const std::wstring& text)
{
    if (!c_assert(tooltip_id >= 0 && tooltip_id < TOOLTIP_AMOUNT) || !c_assert(!text.empty())) return;
    SetTooltip(tooltip_id, text.c_str());
    POINT cursor = {};
    if (shown_tooltip != tooltip_id || !GetCursorPos(&cursor)) return;
    tooltip.Show({ tooltips[tooltip_id].text, tooltips[tooltip_id].external }, { cursor.x, cursor.y });
}
