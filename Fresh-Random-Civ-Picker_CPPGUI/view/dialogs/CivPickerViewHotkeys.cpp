#include "CivPickerView.h"

#include <algorithm>
#include <cwchar>

#include "Assertions.h"
#include "DialogMetrics.h"

namespace
{
    struct ActionText
    {
        HotkeyAction action;
        const wchar_t* text;
    };

    const ActionText ACTION_TEXTS[] = {
        { HotkeyAction::cycle_tabs, L"Cycle tabs" },
        { HotkeyAction::show_draw_tab, L"Go to Draw Civ tab" },
        { HotkeyAction::show_log_tab, L"Go to Log tab" },
        { HotkeyAction::show_civ_pool_tab, L"Go to Civ Pool tab" },
        { HotkeyAction::open_options, L"Open Options" },
        { HotkeyAction::open_hotkeys, L"Open Hotkeys" },
        { HotkeyAction::open_about, L"Open About" },
        { HotkeyAction::open_tech_tree, L"Open Tech Tree" },
        { HotkeyAction::quicksave, L"Quicksave" },
        { HotkeyAction::quickload, L"Quickload" },
        { HotkeyAction::save_preset, L"Save preset" },
        { HotkeyAction::load_preset, L"Load preset" },
        { HotkeyAction::copy_share_link, L"Copy share link" },
        { HotkeyAction::open_share_link, L"Open a share link" },
        { HotkeyAction::join_lobby, L"Join Lobby" },
        { HotkeyAction::undo, L"Undo a draw or reset" },
        { HotkeyAction::redraw, L"Redraw" },
        { HotkeyAction::open_sustainability_app, L"Open Sustainability App" },
        { HotkeyAction::toggle_civ_name, L"Toggle civ name" },
        { HotkeyAction::toggle_civ_icon, L"Toggle civ icon" },
        { HotkeyAction::toggle_round_counter, L"Toggle round counter" },
        { HotkeyAction::toggle_civ_jingles, L"Toggle civ jingles" },
        { HotkeyAction::toggle_ui_sounds, L"Toggle UI sounds" },
        { HotkeyAction::quit, L"Exit" },
        { HotkeyAction::draw_tab_draw_civ, L"Draw civ" },
        { HotkeyAction::draw_tab_reset_round, L"Reset drawn civs" },
        { HotkeyAction::draw_tab_open_options, L"Open Options" },
        { HotkeyAction::draw_tab_open_sustainability_app, L"Open Sustainability App" },
        { HotkeyAction::draw_tab_open_tech_tree, L"Open Tech Tree" },
        { HotkeyAction::open_history, L"Open History" },
        { HotkeyAction::log_tab_draw_civ, L"Draw civ" },
        { HotkeyAction::log_tab_reset_round, L"Reset drawn civs" },
        { HotkeyAction::clear_drawn_log, L"Clear drawn log" },
        { HotkeyAction::toggle_remaining_log, L"Toggle remaining log" },
        { HotkeyAction::enable_all_civs, L"Enable all civs" },
        { HotkeyAction::disable_all_civs, L"Disable all civs" },
        { HotkeyAction::toggle_auto_toggle, L"Auto-toggle older civs" },
        { HotkeyAction::toggle_auto_reset, L"Auto-reset on change" },
    };

    using CivPickerViewDetail::SECTION_TITLES;
    const wchar_t* const NO_KEY = L"\u2014";

    constexpr float HOTKEY_COLUMN_GAP = 28.0f;
    constexpr float HOTKEY_ITEM_HEIGHT = 21.0f;
    constexpr float HOTKEY_LIST_GAP = 4.0f;
    constexpr float HOTKEY_SECTION_GAP = 12.0f;
    constexpr float KEY_TOP_IN_ITEM = 2.0f;
    constexpr float KEY_HEIGHT = 18.390625f;
    constexpr float KEY_MIN_WIDTH = 20.0f;
    constexpr float KEY_PADDING = 10.0f;
    constexpr float ITEM_TEXT_DROP = 2.1f;
    constexpr int GLOBAL_ROWS = 12;
    constexpr int MAX_HOTKEY_ITEMS = 32;

    HotkeyAction ActionAt(int index) { return static_cast<HotkeyAction>(index); }

    int IndexOf(HotkeyAction action) { return static_cast<int>(action); }

    bool IsAction(HotkeyAction action) { return IndexOf(action) >= 0 && IndexOf(action) < HOTKEY_ACTION_COUNT; }

    const wchar_t* FixedTextOf(HotkeyAction action)
    {
        for (const ActionText& entry : ACTION_TEXTS)
        {
            if (entry.action == action) return entry.text;
        }
        (void)c_assert(false);
        return L"";
    }
}

void CivPickerView::RefreshHotkeyNames()
{
    const HotkeyBindings& hotkeys = model.GetHotkeys();
    for (int i = 0; i < HOTKEY_ACTION_COUNT; i++)
    {
        const Keystroke keystroke = hotkeys.KeystrokeOf(ActionAt(i));
        hotkey_keys[i] = keystroke == NO_KEYSTROKE ? KeystrokeName{} : NameKeystroke(keystroke);
    }
}

const wchar_t* CivPickerView::KeyNameOf(HotkeyAction action) const
{
    if (action == HotkeyAction::none || !c_assert(IsAction(action))) return L"";
    return hotkey_keys[IndexOf(action)].characters;
}

const wchar_t* CivPickerView::ShownKeyOf(HotkeyAction action) const
{
    const wchar_t* key = KeyNameOf(action);
    return key[0] != L'\0' ? key : NO_KEY;
}

const wchar_t* CivPickerView::HotkeyTextOf(HotkeyAction action) const
{
    if (!c_assert(IsAction(action))) return L"";
    const int edition = HotkeyBindings::EditionOf(action);
    if (edition != HotkeyBindings::NO_PLACE) return slot_hotkey_texts[edition];
    const int slot = HotkeyBindings::DlcSlotOf(action);
    if (slot != HotkeyBindings::NO_PLACE) return slot_hotkey_texts[MAX_EDITIONS + slot];
    return FixedTextOf(action);
}

void CivPickerView::NameSlotHotkeys()
{
    const CivCatalog& catalog = model.Catalog();
    for (int edition = 0; edition < catalog.EditionCount() && edition < MAX_EDITIONS; edition++)
    {
        (void)wcsncpy_s(slot_hotkey_texts[edition], catalog.EditionAt(edition).short_name.c_str(), _TRUNCATE);
    }
    for (int slot = 0; slot < DLC_HOTKEY_COUNT; slot++)
    {
        const std::wstring names = DlcSlotNames(slot);
        if (names.empty()) continue;
        (void)wcsncpy_s(slot_hotkey_texts[MAX_EDITIONS + slot], (L"Toggle " + names).c_str(), _TRUNCATE);
    }
}

std::wstring CivPickerView::DlcSlotNames(int slot) const
{
    if (!c_assert(slot >= 0 && slot < MAX_DLC_TOGGLES)) return L"";
    const CivCatalog& catalog = model.Catalog();
    std::wstring names;
    for (int edition = 0; edition < catalog.EditionCount() && edition < MAX_EDITIONS; edition++)
    {
        const EditionInfo& info = catalog.EditionAt(edition);
        if (slot >= info.dlc_toggle_count) continue;
        const DlcInfo& dlc = catalog.DlcAt(info.dlc_toggles[slot]);
        if (!names.empty()) names += L" / ";
        names += dlc.short_name.empty() ? dlc.name : dlc.short_name;
    }
    return names;
}

std::wstring CivPickerView::HotkeyLine(HotkeyAction action) const
{
    const wchar_t* key = KeyNameOf(action);
    if (key[0] == L'\0') return L"";
    return std::wstring(L"\nHotkey: ") + key;
}

std::wstring CivPickerView::DrawTabHotkeyLine(HotkeyAction draw_tab_action, HotkeyAction global_action) const
{
    if (!c_assert(IsAction(draw_tab_action))) return L"";
    const wchar_t* tab_key = KeyNameOf(draw_tab_action);
    const wchar_t* global_key = KeyNameOf(global_action);
    if (tab_key[0] == L'\0') return HotkeyLine(global_action);
    std::wstring line = std::wstring(L"\nHotkey: ") + tab_key + L" (Draw tab only)";
    if (global_key[0] != L'\0') line += std::wstring(L" / ") + global_key;
    return line;
}

void CivPickerView::ShowTabTooltips(int tab_index)
{
    if (!c_assert(tab_index >= DRAW_TAB && tab_index <= CIV_POOL_TAB)) return;
    const bool log_tab = tab_index == LOG_TAB;
    const HotkeyAction draw = log_tab ? HotkeyAction::log_tab_draw_civ : HotkeyAction::draw_tab_draw_civ;
    const HotkeyAction reset = log_tab ? HotkeyAction::log_tab_reset_round : HotkeyAction::draw_tab_reset_round;
    UpdateTooltipText(TOOLTIP_DRAW, L"Draws a fresh random civilisation" + HotkeyLine(draw));
    UpdateTooltipText(TOOLTIP_RESET,
                      L"Resets the pool of drawn civs and renders all enabled civs available" + HotkeyLine(reset));
    const std::wstring options_line = tab_index == DRAW_TAB
                                          ? DrawTabHotkeyLine(HotkeyAction::draw_tab_open_options,
                                                              HotkeyAction::open_options)
                                          : HotkeyLine(HotkeyAction::open_options);
    UpdateTooltipText(TOOLTIP_OPTIONS, L"Opens options" + options_line);
}

void CivPickerView::RefreshHotkeyTexts()
{
    RefreshHotkeyNames();
    (void)BuildHotkeysLayout();
    (void)menu_bar.Remeasure();
    SetUpTooltips();
    UpdateTooltips(model.FindCivIndex(model.GetCurrentCiv()) != CivCatalog::NONE);
}

bool CivPickerView::BuildHotkeyLayouts()
{
    RefreshHotkeyNames();
    NameSlotHotkeys();
    return BuildHotkeysLayout() && BuildHotkeyEditorLayout();
}

void CivPickerView::CollectHotkeyItems()
{
    const HotkeyBindings& hotkeys = model.GetHotkeys();
    int count = 0;
    for (int section = 0; section < HOTKEY_SCOPE_COUNT; section++)
    {
        const int first = count;
        for (int i = 0; i < HOTKEY_ACTION_COUNT; i++)
        {
            const HotkeyAction action = ActionAt(i);
            const bool in_section = static_cast<int>(HotkeyBindings::ScopeOf(action)) == section;
            if (!in_section || !hotkeys.IsUsed(action)) continue;
            if (!c_assert(count < HOTKEY_ACTION_COUNT)) return;
            hotkey_items[count] = { action, ShownKeyOf(action), HotkeyTextOf(action) };
            count++;
        }
        hotkey_sections[section] = { hotkey_items + first, count - first };
    }
}

float CivPickerView::KeyWidth(const wchar_t* key)
{
    if (!c_assert(key != nullptr)) return KEY_MIN_WIDTH;
    const float text = renderer.MeasureText({ key, TextLength(key), FontId::key });
    return std::max(text + KEY_PADDING, KEY_MIN_WIDTH);
}

float CivPickerView::HotkeyItemWidth(const HotkeyItemSpan& items)
{
    const HotkeyItem* list = items.items;
    if (!c_assert(list != nullptr) || !c_assert(items.count > 0 && items.count <= MAX_HOTKEY_ITEMS)) return 0.0f;
    float widest = 0.0f;
    const float space = renderer.MeasureText({ L" ", 1, FontId::ui });
    for (int i = 0; i < items.count && i < MAX_HOTKEY_ITEMS; i++)
    {
        const float width = KeyWidth(list[i].key) + space +
                            renderer.MeasureText({ list[i].text, TextLength(list[i].text), FontId::ui });
        if (width > widest) widest = width;
    }
    return widest;
}

float CivPickerView::AddHotkeySection(const HotkeySection& section)
{
    const HotkeyItem* items = section.items.items;
    if (!c_assert(items != nullptr && section.rows > 0)) return section.top;
    if (!c_assert(section.items.count > 0 && section.items.count <= MAX_HOTKEY_ITEMS)) return section.top;
    const float space = renderer.MeasureText({ L" ", 1, FontId::ui });
    AddHotkeyDecoration({ DecorationKind::bold_text,
                          { Rounded(section.left), Rounded(section.top), Rounded(section.left + section.column_width),
                            Rounded(section.top + DIALOG_LINE_HEIGHT) },
                          section.title });
    const float list_top = section.top + DIALOG_LINE_HEIGHT + HOTKEY_LIST_GAP;
    for (int i = 0; i < section.items.count && i < MAX_HOTKEY_ITEMS; i++)
    {
        const int column = i / section.rows;
        const float left = section.left + static_cast<float>(column) * (section.column_width + HOTKEY_COLUMN_GAP);
        const float top = list_top + static_cast<float>(i % section.rows) * HOTKEY_ITEM_HEIGHT;
        const float key_right = left + KeyWidth(items[i].key);
        AddHotkeyDecoration({ DecorationKind::key,
                              { Rounded(left), Rounded(top + KEY_TOP_IN_ITEM), Rounded(key_right),
                                Rounded(top + KEY_TOP_IN_ITEM + KEY_HEIGHT) },
                              items[i].key });
        const int text_left = Rounded(key_right + space);
        AddHotkeyDecoration({ DecorationKind::text,
                              { text_left, Rounded(top + ITEM_TEXT_DROP), text_left + Rounded(section.column_width),
                                Rounded(top + ITEM_TEXT_DROP + DIALOG_LINE_HEIGHT) },
                              items[i].text });
    }
    const int rows_used = std::min(section.items.count, section.rows);
    return list_top + static_cast<float>(rows_used) * HOTKEY_ITEM_HEIGHT;
}

void CivPickerView::AddHotkeyDecoration(const DialogDecoration& decoration)
{
    if (!c_assert(hotkey_decoration_count < MAX_HOTKEY_DECORATIONS) || !c_assert(decoration.text != nullptr)) return;
    hotkey_decorations[hotkey_decoration_count] = decoration;
    hotkey_decoration_count++;
}

bool CivPickerView::BuildHotkeysLayout()
{
    CollectHotkeyItems();
    hotkey_decoration_count = 0;
    AddHotkeyDecoration({ DecorationKind::hint, { BODY_LEFT, BODY_TOP, 800, BODY_TOP + 17 },
                          L"Hotkeys work while the Fresh Random Civ Picker window is active." });
    const float grid_top = BODY_TOP + DIALOG_LINE_HEIGHT + PARAGRAPH_GAP;
    const HotkeyItemSpan& global = hotkey_sections[static_cast<int>(HotkeyScope::global)];
    const HotkeyItemSpan& draw_tab = hotkey_sections[static_cast<int>(HotkeyScope::draw_tab)];
    const HotkeyItemSpan& log_tab = hotkey_sections[static_cast<int>(HotkeyScope::log_tab)];
    const HotkeyItemSpan& pool_tab = hotkey_sections[static_cast<int>(HotkeyScope::civ_pool_tab)];
    const float global_column = HotkeyItemWidth(global);
    const float tab_column = std::max(HotkeyItemWidth(draw_tab), HotkeyItemWidth(log_tab));
    const float pool_column = HotkeyItemWidth(pool_tab);
    const float tab_left = BODY_LEFT + 2.0f * global_column + 2.0f * HOTKEY_COLUMN_GAP;
    const float pool_left = tab_left + tab_column + HOTKEY_COLUMN_GAP;

    const float global_bottom = AddHotkeySection({ SECTION_TITLES[0], global, BODY_LEFT, grid_top, global_column,
                                                   GLOBAL_ROWS });
    const float draw_bottom = AddHotkeySection({ SECTION_TITLES[1], draw_tab, tab_left, grid_top, tab_column,
                                                 draw_tab.count });
    const float log_bottom = AddHotkeySection({ SECTION_TITLES[2], log_tab, tab_left, draw_bottom + HOTKEY_SECTION_GAP,
                                                tab_column, log_tab.count });
    const float pool_bottom = AddHotkeySection({ SECTION_TITLES[3], pool_tab, pool_left, grid_top, pool_column,
                                                 pool_tab.count });
    float grid_bottom = std::max(global_bottom, log_bottom);
    if (pool_bottom > grid_bottom) grid_bottom = pool_bottom;

    const int width = Rounded(pool_left + pool_column) + BUTTON_ROW_PADDING + PANEL_PADDING;
    const PixelSize size = { width, HeightBelowBody(grid_bottom - PARAGRAPH_GAP) };
    hotkeys_parts[0] = { PartKind::default_button, IDOK, BottomRightButton(size), L"OK" };
    hotkeys_layout = { L"Hotkeys", size, hotkeys_parts, 1, hotkey_decorations, hotkey_decoration_count, nullptr, 0,
                       IDOK };
    return c_assert(size.width <= ParchmentDialogs::MAX_DIALOG_WIDTH &&
                    size.height <= ParchmentDialogs::MAX_DIALOG_HEIGHT);
}
