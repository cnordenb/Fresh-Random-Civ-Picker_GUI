#include "CivPickerView.h"

#include <cwchar>

#include "Assertions.h"

void CivPickerView::SetUpTooltips()
{
    const std::wstring tech_tree_line =
        DrawTabHotkeyLine(HotkeyAction::draw_tab_open_tech_tree, HotkeyAction::open_tech_tree);
    const std::wstring survapp_line =
        DrawTabHotkeyLine(HotkeyAction::draw_tab_open_sustainability_app, HotkeyAction::open_sustainability_app);
    const std::wstring history_line = DrawTabHotkeyLine(HotkeyAction::open_history, HotkeyAction::none);
    SetTooltip(TOOLTIP_TECHTREE, (L"Opens the tech tree" + tech_tree_line).c_str());
    const std::wstring survapp_text =
        L"Opens aoe2vils.net to calculate villagers required for sustainable production" + survapp_line;
    SetTooltip(TOOLTIP_SURVAPP, survapp_text.c_str());
    SetTooltip(TOOLTIP_HISTORY, (L"Opens the game's History info" + history_line).c_str());
    SetHotkeyTooltip(TOOLTIP_CLEAR, L"Clears the log of previously drawn civs", HotkeyAction::clear_drawn_log);
    SetHotkeyTooltip(TOOLTIP_REMAININGTOGGLE, L"Toggles the display of the remaining civs log",
                     HotkeyAction::toggle_remaining_log);
    SetHotkeyTooltip(TOOLTIP_AUTOTOGGLE, L"Toggles older civs as well as the automatic enabling of older civs when "
                                         L"switching between editions",
                     HotkeyAction::toggle_auto_toggle);
    SetHotkeyTooltip(TOOLTIP_AUTORESET,
                     L"Automatically resets the pool of drawn civs when the civilisation pool is changed",
                     HotkeyAction::toggle_auto_reset);
    SetHotkeyTooltip(TOOLTIP_ENABLEALL, L"Enables all civilisations so that they are made available in the pool of "
                                        L"civilisations for drawing",
                     HotkeyAction::enable_all_civs);
    SetHotkeyTooltip(TOOLTIP_DISABLEALL, L"Disables all civilisations so that they are removed from the pool of "
                                         L"civilisations for drawing",
                     HotkeyAction::disable_all_civs);
    SetHotkeyTooltip(TOOLTIP_SHARE, L"Copies a share link with the currently selected civ pool and this round's draws",
                     HotkeyAction::copy_share_link);
    SetTooltip(TOOLTIP_POOLSHARE, L"Copies a share link with this civ pool");
    ShowTabTooltips(current_tab);
    SetUpDataTooltips();
    tooltips[TOOLTIP_TECHTREE].external = true;
    tooltips[TOOLTIP_SURVAPP].external = true;
}

void CivPickerView::SetUpDataTooltips()
{
    const CivCatalog& catalog = model.Catalog();
    for (int edition = 0; edition < catalog.EditionCount() && edition < MAX_EDITIONS; edition++)
    {
        const EditionInfo& info = catalog.EditionAt(edition);
        const std::wstring text = L"Selects the " + info.short_name + L" (" + std::to_wstring(info.year) +
                                  L") civilisation pool" + HotkeyLine(HotkeyBindings::EditionHotkey(edition));
        SetTooltip(EDITION_TOOLTIP_BASE + edition, text.c_str());
    }
    for (int dlc = 0; dlc < catalog.DlcCount() && dlc < MAX_DLCS; dlc++)
    {
        if (HasDlcToggle(dlc)) SetTooltip(DLC_TOOLTIP_BASE + dlc, DlcTooltipText(dlc).c_str());
    }
}

std::wstring CivPickerView::DlcTooltipText(int dlc) const
{
    if (!c_assert(HasDlcToggle(dlc))) return L"";
    const CivCatalog& catalog = model.Catalog();
    const DlcInfo& info = catalog.DlcAt(dlc);
    if (!c_assert(info.toggle_slot >= 0 && info.toggle_slot < MAX_DLC_TOGGLES)) return L"";
    std::wstring names;
    int civ_count = 0;
    for (int civ = 0; civ < catalog.CivCount() && civ < MAX_CIVS; civ++)
    {
        if (catalog.CivAt(civ).dlc != dlc) continue;
        names += (civ_count > 0 ? L", " : L"") + catalog.CivAt(civ).name;
        civ_count++;
    }
    const wchar_t* const noun = civ_count == 1 ? L" civilisation\n(" : L" civilisations\n(";
    return L"Toggles " + info.name + noun + names + L")" + HotkeyLine(HotkeyBindings::DlcSlotHotkey(info.toggle_slot));
}

void CivPickerView::SetHotkeyTooltip(int tooltip_id, const wchar_t* text, HotkeyAction action)
{
    if (!c_assert(text != nullptr)) return;
    SetTooltip(tooltip_id, (text + HotkeyLine(action)).c_str());
}

void CivPickerView::SetTooltip(int tooltip_id, const wchar_t* text)
{
    if (!c_assert(tooltip_id >= 0 && tooltip_id < TOOLTIP_AMOUNT) || !c_assert(text != nullptr)) return;
    (void)wcsncpy_s(tooltips[tooltip_id].text, text, _TRUNCATE);
}

void CivPickerView::CollectTooltipTargets()
{
    const TooltipTarget first_targets[] = { { button_draw.Window(), TOOLTIP_DRAW },
                                            { button_reset.Window(), TOOLTIP_RESET },
                                            { button_techtree.Window(), TOOLTIP_TECHTREE },
                                            { button_options.Window(), TOOLTIP_OPTIONS },
                                            { button_clearlog.Window(), TOOLTIP_CLEAR },
                                            { button_enableall.Window(), TOOLTIP_ENABLEALL },
                                            { button_disableall.Window(), TOOLTIP_DISABLEALL },
                                            { checkbox_showremainlog.Window(), TOOLTIP_REMAININGTOGGLE },
                                            { button_share.Window(), TOOLTIP_SHARE },
                                            { button_poolshare.Window(), TOOLTIP_POOLSHARE } };
    const TooltipTarget pool_targets[] = { { checkbox_autotoggle.Window(), TOOLTIP_AUTOTOGGLE },
                                           { checkbox_autoreset.Window(), TOOLTIP_AUTORESET },
                                           { button_survapp.Window(), TOOLTIP_SURVAPP },
                                           { button_history.Window(), TOOLTIP_HISTORY } };
    static_assert(sizeof(first_targets) / sizeof(first_targets[0]) + sizeof(pool_targets) / sizeof(pool_targets[0]) +
                      MAX_EDITIONS + MAX_DLCS <= TOOLTIP_AMOUNT, "every tooltip target has a slot");
    if (!c_assert(main_window != NULL)) return;
    const CivCatalog& catalog = model.Catalog();
    int count = 0;
    for (const TooltipTarget& target : first_targets) tooltip_targets[count++] = target;
    for (int edition = 0; edition < catalog.EditionCount() && edition < MAX_EDITIONS; edition++)
        tooltip_targets[count++] = { edition_rows[edition].Window(), EDITION_TOOLTIP_BASE + edition };
    for (const TooltipTarget& target : pool_targets) tooltip_targets[count++] = target;
    for (int edition = 0; edition < catalog.EditionCount() && edition < MAX_EDITIONS; edition++)
    {
        const EditionInfo& info = catalog.EditionAt(edition);
        for (int slot = 0; slot < info.dlc_toggle_count && slot < MAX_DLC_TOGGLES && count < TOOLTIP_AMOUNT; slot++)
        {
            const int dlc = info.dlc_toggles[slot];
            tooltip_targets[count++] = { dlc_toggles[dlc].Window(), DLC_TOOLTIP_BASE + dlc };
        }
    }
}
