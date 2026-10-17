#include "CivPickerView.h"

#include <algorithm>
#include <cwchar>

#include "Assertions.h"
#include "CivPickerController.h"
#include "DialogMetrics.h"
#include "Resource.h"

// The hotkeys in the view (CivPickerView): their keys in the Hotkeys dialog, the menus and the tooltips, and the
// Customise Hotkeys dialog (Options > Customise), where the user gives them other keys.

namespace
{
    // What each hotkey does, as the Hotkeys dialog says it; the editions' and the DLC slots' come from data.json.
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

    // The sections of the hotkeys, as the dialogs title them and as the status line says where one works.
    const wchar_t* const SECTION_TITLES[HOTKEY_SCOPE_COUNT] = { L"Global", L"Draw Civ tab", L"Log tab",
                                                                L"Civ Pool tab" };
    const wchar_t* const SECTION_PLACES[HOTKEY_SCOPE_COUNT] = { L"every tab", L"Draw Civ tab", L"Log tab",
                                                                L"Civ Pool tab" };
    const wchar_t* const NO_KEY = L"\u2014"; // the dialogs' key for a hotkey without one

    // --- Hotkeys (#dlgHotkeys): sections of keys and what they do, the global ones in two columns ---

    // .hk-grid: columns 28 px apart, headings (h3) 4 px above their list, list items 21 px high (line-height 1.75)
    // with their keys 2 px below the item's top, and sections 12 px apart.
    constexpr float HOTKEY_COLUMN_GAP = 28.0f;
    constexpr float HOTKEY_ITEM_HEIGHT = 21.0f;
    constexpr float HOTKEY_LIST_GAP = 4.0f;
    constexpr float HOTKEY_SECTION_GAP = 12.0f;
    constexpr float KEY_TOP_IN_ITEM = 2.0f;
    constexpr float KEY_HEIGHT = 18.390625f;
    constexpr float KEY_MIN_WIDTH = 20.0f;
    constexpr float KEY_PADDING = 10.0f; // padding 0 4px plus the 1 px borders
    constexpr float ITEM_TEXT_DROP = 2.1f; // a 16.8 px text line centred in the 21 px item
    constexpr int GLOBAL_ROWS = 12;
    constexpr int MAX_HOTKEY_ITEMS = 32; // the longest hotkey list, the global one, has 24

    // --- Customise Hotkeys (desktop only): the hint, the sections to pick, then a section's hotkeys in up to two
    // columns of rows, each a key to click and what it does, then two lines of status and the buttons ---

    const wchar_t* const EDITOR_HINT = L"Click a hotkey's key, then press the key you want for it; Backspace leaves "
                                       L"it without one.";
    const wchar_t* const UNNAMED_KEY_HELP = L"Letters, digits, F1 to F11, the editing and arrow keys and the number "
                                            L"pad can, also with Ctrl, Alt or Shift.";
    constexpr int EDITOR_SECTIONS_TOP = 80;
    constexpr int EDITOR_SECTION_HEIGHT = 22;
    constexpr int EDITOR_SECTION_GAP = 16;
    constexpr int RADIO_TEXT_GAP = 6; // the dialogs' .row { gap: 6px }
    constexpr int EDITOR_ROWS_TOP = 114;
    constexpr int EDITOR_ROW_HEIGHT = 24;
    constexpr int EDITOR_ROWS = 12; // the most rows in a column
    constexpr int EDITOR_KEY_WIDTH = 104; // a 100 px key and its focus outline's margins
    constexpr int EDITOR_KEY_HEIGHT = 22;
    constexpr int EDITOR_TEXT_GAP = 6;
    constexpr int EDITOR_COLUMN_GAP = 28;
    constexpr int MAX_EDITOR_TEXT_WIDTH = 240;
    constexpr int EDITOR_STATUS_TOP = EDITOR_ROWS_TOP + EDITOR_ROWS * EDITOR_ROW_HEIGHT + 8;
    constexpr int EDITOR_DETAIL_TOP = EDITOR_STATUS_TOP + 17;
    constexpr int EDITOR_SECTION_BASE_ID = 1100;
    constexpr int EDITOR_KEY_BASE_ID = 1110;
    constexpr int EDITOR_TEXT_BASE_ID = 1140;
    constexpr int EDITOR_STATUS_ID = 1170;
    constexpr int EDITOR_DETAIL_ID = 1171;
    constexpr int EDITOR_DEFAULTS_ID = 1172;
    constexpr int NO_ROW = -1;
    static_assert(EDITOR_KEY_BASE_ID + ParchmentDialogs::MAX_KEY_BUTTONS <= EDITOR_TEXT_BASE_ID &&
                      EDITOR_TEXT_BASE_ID + ParchmentDialogs::MAX_KEY_BUTTONS <= EDITOR_STATUS_ID,
                  "the rows' keys and texts have IDs apart");
    static_assert(2 * EDITOR_ROWS == ParchmentDialogs::MAX_KEY_BUTTONS, "two columns of rows");

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

    PixelRect EditorKeyArea(PixelPoint row_origin)
    {
        return { row_origin.x, row_origin.y + 1, row_origin.x + EDITOR_KEY_WIDTH,
                 row_origin.y + 1 + EDITOR_KEY_HEIGHT };
    }

    // The row's text, its 17 px line centred in the row.
    PixelRect EditorTextArea(PixelPoint row_origin, int column_width)
    {
        const int left = row_origin.x + EDITOR_KEY_WIDTH + EDITOR_TEXT_GAP;
        return { left, row_origin.y + 3, std::max(left + 1, row_origin.x + column_width), row_origin.y + 20 };
    }

    // A section of more rows than a column takes splits in two even columns.
    int RowsPerColumn(int row_count)
    {
        if (row_count > EDITOR_ROWS) return (row_count + 1) / 2;
        return std::max(row_count, 1);
    }
}

// ---------------------------------------------------------------------------------------------------
// The keys' names

// Each hotkey's key by name, for the menus, the tooltips and the dialogs; empty for a hotkey without one.
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

// The key as the dialogs show it, a dash for none.
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

// The texts of the Civ Pool tab's hotkeys that data.json names: an edition's short name, and for a DLC slot the DLCs
// in that place of the editions' toggles (the web version's dlcSlotText).
void CivPickerView::NameSlotHotkeys()
{
    const CivCatalog& catalog = model.Catalog();
    for (int edition = 0; edition < catalog.EditionCount() && edition < MAX_EDITIONS; edition++)
    {
        // A text longer than its buffer is cut short, which still shows its start.
        (void)wcsncpy_s(slot_hotkey_texts[edition], catalog.EditionAt(edition).short_name.c_str(), _TRUNCATE);
    }
    for (int slot = 0; slot < DLC_HOTKEY_COUNT; slot++)
    {
        const std::wstring names = DlcSlotNames(slot);
        if (names.empty()) continue;
        (void)wcsncpy_s(slot_hotkey_texts[MAX_EDITIONS + slot], (L"Toggle " + names).c_str(), _TRUNCATE);
    }
}

// The short names of the DLCs whose toggles are in this place in their edition's list, joined by slashes.
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

// "\nHotkey: Q" at the end of a tooltip, as the web version's tooltips end; nothing for a hotkey without a key.
std::wstring CivPickerView::HotkeyLine(HotkeyAction action) const
{
    const wchar_t* key = KeyNameOf(action);
    if (key[0] == L'\0') return L"";
    return std::wstring(L"\nHotkey: ") + key;
}

// A Draw Civ tab hotkey with the global one that does the same, if any: "\nHotkey: T (Draw tab only) / F4".
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

// The tooltips that differ between the tabs: Options says its Draw Civ tab key there, and Draw and Reset, which the
// Log tab shows too, say the keys of the tab they are on.
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

// The menus, the tooltips and the Hotkeys dialog show the keys the hotkeys have now.
void CivPickerView::RefreshHotkeyTexts()
{
    RefreshHotkeyNames();
    // A layout that doesn't fit has said so, and the Hotkeys dialog then doesn't open.
    (void)BuildHotkeysLayout();
    // A list that doesn't fit has said so; the menu keeps the room it had.
    (void)menu_bar.Remeasure();
    SetUpTooltips();
    UpdateTooltips(model.FindCivIndex(model.GetCurrentCiv()) != CivCatalog::NONE);
}

// ---------------------------------------------------------------------------------------------------
// The Hotkeys dialog's layout, made from measured text at startup and again whenever the keys change

bool CivPickerView::BuildHotkeyLayouts()
{
    RefreshHotkeyNames();
    NameSlotHotkeys();
    return BuildHotkeysLayout() && BuildHotkeyEditorLayout();
}

// The hotkeys in use, each section's together and in the order of HotkeyAction, with their keys as they are now.
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

// Adds a heading and its list of keys at a column's place; returns where the next section can start.
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

// ---------------------------------------------------------------------------------------------------
// The Customise Hotkeys dialog's layout, made at startup: every row is there, and a section shows its rows

bool CivPickerView::BuildHotkeyEditorLayout()
{
    editor_part_count = 0;
    AddEditorSections();
    const int text_width = std::min(Rounded(WidestHotkeyText()), MAX_EDITOR_TEXT_WIDTH);
    editor_column_width = EDITOR_KEY_WIDTH + EDITOR_TEXT_GAP + text_width;
    for (int row = 0; row < ParchmentDialogs::MAX_KEY_BUTTONS; row++)
    {
        const PixelPoint origin = EditorRowOrigin(row, EDITOR_ROWS);
        AddEditorPart({ PartKind::key_button, EDITOR_KEY_BASE_ID + row, EditorKeyArea(origin), L"" });
        AddEditorPart({ PartKind::label, EDITOR_TEXT_BASE_ID + row, EditorTextArea(origin, editor_column_width),
                        L"" });
    }
    const int body_right = BODY_LEFT + 2 * editor_column_width + EDITOR_COLUMN_GAP;
    AddEditorPart({ PartKind::label, EDITOR_STATUS_ID,
                    { BODY_LEFT, EDITOR_STATUS_TOP, body_right, EDITOR_STATUS_TOP + 17 }, L"" });
    AddEditorPart({ PartKind::label, EDITOR_DETAIL_ID,
                    { BODY_LEFT, EDITOR_DETAIL_TOP, body_right, EDITOR_DETAIL_TOP + 17 }, L"" });
    const PixelSize size = { body_right + BODY_PADDING_X + PANEL_PADDING,
                             HeightBelowBody(EDITOR_DETAIL_TOP + DIALOG_LINE_HEIGHT) };
    const PixelRect ok = BottomRightButton(size);
    const PixelRect cancel = ButtonLeftOf(ok);
    AddEditorPart({ PartKind::button, EDITOR_DEFAULTS_ID, ButtonLeftOf(cancel), L"Defaults" });
    AddEditorPart({ PartKind::button, IDC_BUTTON_CANCEL, cancel, L"Cancel" });
    AddEditorPart({ PartKind::default_button, IDOK, ok, L"OK" });
    editor_decorations[0] = { DecorationKind::hint, { BODY_LEFT, BODY_TOP, body_right, BODY_TOP + 17 }, EDITOR_HINT };
    editor_layout = { L"Customise Hotkeys", size, editor_parts, editor_part_count, editor_decorations, 1, nullptr,
                      0, IDOK };
    return c_assert(size.width <= ParchmentDialogs::MAX_DIALOG_WIDTH &&
                    size.height <= ParchmentDialogs::MAX_DIALOG_HEIGHT);
}

// A radio button for each section, side by side.
void CivPickerView::AddEditorSections()
{
    int left = BODY_LEFT;
    for (int section = 0; section < HOTKEY_SCOPE_COUNT; section++)
    {
        const wchar_t* section_title = SECTION_TITLES[section];
        const float text = renderer.MeasureText({ section_title, TextLength(section_title), FontId::ui });
        const int width = CHECKBOX_SIZE + RADIO_TEXT_GAP + Rounded(text) + 1;
        AddEditorPart({ PartKind::radio, EDITOR_SECTION_BASE_ID + section,
                        { left, EDITOR_SECTIONS_TOP, left + width, EDITOR_SECTIONS_TOP + EDITOR_SECTION_HEIGHT },
                        section_title });
        left += width + EDITOR_SECTION_GAP;
    }
}

float CivPickerView::WidestHotkeyText()
{
    float widest = 0.0f;
    for (const HotkeyItemSpan& section : hotkey_sections)
    {
        for (int i = 0; i < section.count && i < HOTKEY_ACTION_COUNT; i++)
        {
            const wchar_t* text = section.items[i].text;
            widest = std::max(widest, renderer.MeasureText({ text, TextLength(text), FontId::ui }));
        }
    }
    return widest;
}

void CivPickerView::AddEditorPart(const DialogPart& part)
{
    if (!c_assert(editor_part_count < MAX_EDITOR_PARTS)) return;
    editor_parts[editor_part_count] = part;
    editor_part_count++;
}

// Where a row goes when its section has this many rows in a column.
PixelPoint CivPickerView::EditorRowOrigin(int row, int rows_per_column) const
{
    if (!c_assert(row >= 0 && rows_per_column > 0)) return { BODY_LEFT, EDITOR_ROWS_TOP };
    const int column = row / rows_per_column;
    return { BODY_LEFT + column * (editor_column_width + EDITOR_COLUMN_GAP),
             EDITOR_ROWS_TOP + (row % rows_per_column) * EDITOR_ROW_HEIGHT };
}

// ---------------------------------------------------------------------------------------------------
// Running the Customise Hotkeys dialog

// True when the user closed it with OK.
bool CivPickerView::ShowHotkeyEditor(HWND owner)
{
    if (!c_assert(owner != NULL)) return false;
    RefreshHotkeyNames();
    editor_listening_row = NO_ROW;
    return RunDialog({ owner, HotkeyEditorProc, reinterpret_cast<LPARAM>(this), &editor_layout }) == IDOK;
}

// The dialog procedure is a function pointer Win32 requires (CLAUDE2.md I2), as the other dialogs' are.
INT_PTR CALLBACK CivPickerView::HotkeyEditorProc(HWND dialog, UINT message, WPARAM wParam, LPARAM lParam) noexcept
{
    CivPickerView* view = AttachDialog(dialog, message, lParam);
    if (view == nullptr) return FALSE;
    if (message == WM_INITDIALOG)
    {
        const INT_PTR result = view->BeginDialog(dialog);
        view->InitialiseHotkeyEditor(dialog);
        return result;
    }
    const ParchmentDialogs::Handled frame = view->dialogs.HandleMessage({ dialog, message, wParam, lParam });
    if (frame.handled) return frame.result;
    if (message != WM_COMMAND) return FALSE;
    return view->OnHotkeyEditorCommand(dialog, wParam, lParam) ? TRUE : FALSE;
}

void CivPickerView::InitialiseHotkeyEditor(HWND editor)
{
    if (!c_assert(editor != NULL)) return;
    // A dialog that couldn't be built has closed again and has no rows to fill.
    if (GetDlgItem(editor, EDITOR_KEY_BASE_ID) == NULL) return;
    (void)CheckRadioButton(editor, EDITOR_SECTION_BASE_ID, EDITOR_SECTION_BASE_ID + HOTKEY_SCOPE_COUNT - 1,
                           EDITOR_SECTION_BASE_ID);
    ShowHotkeySection(editor, HotkeyScope::global);
}

bool CivPickerView::OnHotkeyEditorCommand(HWND editor, WPARAM wParam, LPARAM lParam)
{
    if (!c_assert(editor != NULL)) return false;
    const int command_id = LOWORD(wParam);
    const HWND control = reinterpret_cast<HWND>(lParam);
    const int row = command_id - EDITOR_KEY_BASE_ID;
    if (row >= 0 && row < editor_row_count) return OnEditorKey(editor, { row, HIWORD(wParam), control });
    if (HIWORD(wParam) != BN_CLICKED) return false;
    const int section = command_id - EDITOR_SECTION_BASE_ID;
    if (section >= 0 && section < HOTKEY_SCOPE_COUNT)
    {
        PlayAudio(SoundType::view);
        StopEditorListening(editor);
        (void)CheckRadioButton(editor, EDITOR_SECTION_BASE_ID, EDITOR_SECTION_BASE_ID + HOTKEY_SCOPE_COUNT - 1,
                               command_id);
        ShowHotkeySection(editor, static_cast<HotkeyScope>(section));
        return true;
    }
    if (command_id == EDITOR_DEFAULTS_ID)
    {
        StopEditorListening(editor);
        controller.OnDefaultKeysChosen(editor);
        return true;
    }
    if (command_id != IDOK && command_id != IDCANCEL && command_id != IDC_BUTTON_CANCEL) return false;
    PlayAudio(SoundType::button);
    return EndDialog(editor, command_id == IDOK ? IDOK : IDCANCEL) != FALSE;
}

// A key starts listening (and takes the focus, so the keys go to it), stops without a key, or chose one, which the
// controller gives its hotkey.
bool CivPickerView::OnEditorKey(HWND editor, const EditorKeyEvent& key_event)
{
    if (!c_assert(editor != NULL && key_event.control != NULL)) return false;
    if (!c_assert(key_event.row >= 0 && key_event.row < editor_row_count)) return false;
    const HotkeyAction action = editor_rows[key_event.row];
    if (key_event.notification == KeyButton::KEY_LISTENING)
    {
        PlayAudio(SoundType::button);
        editor_listening_row = key_event.row;
        (void)SetFocus(key_event.control);
        ShowListeningStatus(editor, action);
        return true;
    }
    if (key_event.notification == KeyButton::KEY_KEPT)
    {
        // A key that stopped because another started listening leaves that one's status.
        if (key_event.row != editor_listening_row) return true;
        editor_listening_row = NO_ROW;
        SetEditorStatus(editor, {});
        return true;
    }
    if (key_event.notification != KeyButton::KEY_CHOSEN) return false;
    editor_listening_row = NO_ROW;
    const LRESULT chosen = SendMessage(key_event.control, KeyButton::GET_KEYSTROKE, 0, 0);
    const Keystroke keystroke = { static_cast<uint8_t>(HIWORD(chosen)), static_cast<uint8_t>(LOWORD(chosen)) };
    controller.OnKeyChosen(editor, action, keystroke);
    return true;
}

void CivPickerView::StopEditorListening(HWND editor)
{
    if (!c_assert(editor != NULL) || editor_listening_row == NO_ROW) return;
    const HWND key = GetDlgItem(editor, EDITOR_KEY_BASE_ID + editor_listening_row);
    // A key that has stopped already answers nothing.
    if (key != NULL) (void)SendMessage(key, KeyButton::STOP_LISTENING, 0, 0);
    editor_listening_row = NO_ROW;
    SetEditorStatus(editor, {});
}

// Shows a section's hotkeys in its rows, in one column or two even ones, and hides the rows it doesn't need.
void CivPickerView::ShowHotkeySection(HWND editor, HotkeyScope section)
{
    if (!c_assert(editor != NULL) || !c_assert(static_cast<int>(section) < HOTKEY_SCOPE_COUNT)) return;
    const HotkeyItemSpan& items = hotkey_sections[static_cast<int>(section)];
    editor_row_count = std::min(items.count, ParchmentDialogs::MAX_KEY_BUTTONS);
    const int rows_per_column = RowsPerColumn(editor_row_count);
    for (int row = 0; row < ParchmentDialogs::MAX_KEY_BUTTONS; row++)
    {
        const bool shown = row < editor_row_count;
        if (shown)
        {
            editor_rows[row] = items.items[row].action;
            PlaceEditorRow(editor, row, rows_per_column);
        }
        ShowControl(GetDlgItem(editor, EDITOR_KEY_BASE_ID + row), shown);
        ShowControl(GetDlgItem(editor, EDITOR_TEXT_BASE_ID + row), shown);
    }
}

void CivPickerView::PlaceEditorRow(HWND editor, int row, int rows_per_column)
{
    if (!c_assert(editor != NULL) || !c_assert(row >= 0 && row < editor_row_count)) return;
    const HWND key = GetDlgItem(editor, EDITOR_KEY_BASE_ID + row);
    const HWND text = GetDlgItem(editor, EDITOR_TEXT_BASE_ID + row);
    if (!c_assert(key != NULL && text != NULL)) return;
    const PixelPoint origin = EditorRowOrigin(row, rows_per_column);
    const PixelRect key_area = EditorKeyArea(origin);
    const PixelRect text_area = EditorTextArea(origin, editor_column_width);
    const UINT flags = SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE;
    // A row that couldn't move stays usable where it is.
    (void)SetWindowPos(key, NULL, key_area.left, key_area.top, 0, 0, flags);
    (void)SetWindowPos(text, NULL, text_area.left, text_area.top, 0, 0, flags);
    SetControlText(key, ShownKeyOf(editor_rows[row]));
    SetControlText(text, HotkeyTextOf(editor_rows[row]));
}

// The keys of the rows shown, after keys changed (taking one from another hotkey changes two rows).
void CivPickerView::RefreshEditorKeys(HWND editor)
{
    if (!c_assert(editor != NULL)) return;
    for (int row = 0; row < editor_row_count && row < ParchmentDialogs::MAX_KEY_BUTTONS; row++)
    {
        const HWND key = GetDlgItem(editor, EDITOR_KEY_BASE_ID + row);
        if (key != NULL) SetControlText(key, ShownKeyOf(editor_rows[row]));
    }
}

// ---------------------------------------------------------------------------------------------------
// The Customise Hotkeys dialog's status lines

void CivPickerView::ShowListeningStatus(HWND editor, HotkeyAction action)
{
    if (!c_assert(editor != NULL)) return;
    EditorStatus text = {};
    // A status longer than its buffer is cut short, which still shows its start.
    (void)_snwprintf_s(text.status, _TRUNCATE, L"Press the new key for %ls, or Backspace for no key.",
                       HotkeyTextOf(action));
    SetEditorStatus(editor, text);
}

// What became of a key the user chose: the controller has given it to the hotkey, or refused it.
void CivPickerView::ShowKeyChange(HWND editor, const KeyChange& change)
{
    if (!c_assert(editor != NULL)) return;
    RefreshHotkeyNames();
    RefreshEditorKeys(editor);
    SetEditorStatus(editor, DescribeKeyChange(change));
}

CivPickerView::EditorStatus CivPickerView::DescribeKeyChange(const KeyChange& change) const
{
    EditorStatus text = {};
    const KeystrokeName key = NameKeystroke(change.keystroke);
    // Each status fits its buffer; one cut short would still begin right.
    switch (change.choice)
    {
        case KeyChoice::assigned: return DescribeAssignedKey(change);
        case KeyChoice::cleared:
            (void)_snwprintf_s(text.status, _TRUNCATE, L"%ls has no key now.", HotkeyTextOf(change.action));
            return text;
        case KeyChoice::kept_by_windows:
            (void)_snwprintf_s(text.status, _TRUNCATE, L"Windows keeps %ls for itself.", key.characters);
            return text;
        case KeyChoice::unnamed_key:
            (void)wcsncpy_s(text.status, L"That key can't be a hotkey.", _TRUNCATE);
            (void)wcsncpy_s(text.detail, UNNAMED_KEY_HELP, _TRUNCATE);
            return text;
        case KeyChoice::defaults_restored:
            (void)wcsncpy_s(text.status, L"Every hotkey has its default key again.", _TRUNCATE);
            return text;
        default: (void)c_assert(false); return text;
    }
}

// "The key for Open Options is now O.", and the hotkeys it was taken from: "O was taken from Clear drawn log (Log
// tab) and Definitive Edition (Civ Pool tab)."
CivPickerView::EditorStatus CivPickerView::DescribeAssignedKey(const KeyChange& change) const
{
    EditorStatus text = {};
    const KeystrokeName key = NameKeystroke(change.keystroke);
    const wchar_t* const elsewhere =
        change.used_by_other_program ? L" Another program has it too, so it may not work." : L"";
    // Each line fits its buffer; one cut short would still begin right.
    (void)_snwprintf_s(text.status, _TRUNCATE, L"The key for %ls is now %ls.%ls", HotkeyTextOf(change.action),
                       key.characters, elsewhere);
    const int count = change.taken_from.count;
    if (!c_assert(count >= 0 && count <= HotkeyBindings::MAX_CLASHES) || count == 0) return text;
    (void)_snwprintf_s(text.detail, _TRUNCATE, L"%ls was taken from ", key.characters);
    for (int i = 0; i < count; i++)
    {
        const HotkeyAction taken = change.taken_from.actions[i];
        const wchar_t* separator = i == count - 1 ? L" and " : L", ";
        if (i == 0) separator = L"";
        wchar_t part[MAX_HOTKEY_TEXT + 32] = L"";
        (void)_snwprintf_s(part, _TRUNCATE, L"%ls%ls (%ls)", separator, HotkeyTextOf(taken),
                           SECTION_PLACES[static_cast<int>(HotkeyBindings::ScopeOf(taken))]);
        (void)wcsncat_s(text.detail, part, _TRUNCATE);
    }
    (void)wcsncat_s(text.detail, L".", _TRUNCATE);
    return text;
}

void CivPickerView::SetEditorStatus(HWND editor, const EditorStatus& text)
{
    if (!c_assert(editor != NULL)) return;
    SetControlText(GetDlgItem(editor, EDITOR_STATUS_ID), text.status);
    SetControlText(GetDlgItem(editor, EDITOR_DETAIL_ID), text.detail);
}
