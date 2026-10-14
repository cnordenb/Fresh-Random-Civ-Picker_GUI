#include "CivPickerView.h"

#include <commdlg.h>
#include <cmath>
#include <cwchar>
#include <cwctype>

#include "Assertions.h"
#include "CivPickerController.h"
#include "DialogMetrics.h"
#include "Resource.h"

// The dialogs of the view, laid out as the web version's (index.html, style.css): a panel with its title and
// close button, the body 14 px in from the sides and 12 px below the title, and the buttons at the bottom right
// (DialogMetrics.h).

namespace
{
    // --- Options (#dlgOptions): five groups in a grid of 214 px columns ---

    const DialogDecoration OPTIONS_DECORATIONS[] = {
        { DecorationKind::group, { 27, 55, 241, 212 }, L"Display" },
        { DecorationKind::group, { 251, 55, 465, 212 }, L"Sound" },
        { DecorationKind::group, { 475, 55, 689, 212 }, L"Continuous freshness" },
        { DecorationKind::group, { 27, 222, 241, 339 }, L"When the program starts" },
        { DecorationKind::group, { 251, 222, 689, 339 }, L"Hotkeys" },
        { DecorationKind::text, { 262, 101, 330, 118 }, L"Jingle type" },
        { DecorationKind::hint, { 486, 143, 678, 194 }, L"When a round ends, its last civs sit out the first draws of "
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
        { PartKind::default_button, IDOK, { 614, 351, 689, 374 }, L"OK" },
    };

    // --- Join Lobby (#dlgLobby) ---

    constexpr int LOBBY_ERROR_TOP = 106;
    constexpr int LOBBY_ERROR_ROOM = 25; // the error line and its 8 px margin
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

    // --- History (desktop only): the civ, its icon and pool state, and its history text ---

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

    // --- Hotkeys (#dlgHotkeys): sections of keys and what they do, the global ones in two columns ---

    using HotkeyItem = CivPickerView::HotkeyItem;

    const HotkeyItem GLOBAL_HOTKEYS[] = {
        { L"Tab", L"Cycle tabs" },           { L"1", L"Go to Draw Civ tab" },     { L"2", L"Go to Log tab" },
        { L"3", L"Go to Civ Pool tab" },     { L"F1", L"Open Options" },          { L"F2", L"Open Hotkeys" },
        { L"F3", L"Open About" },            { L"F4", L"Open Tech Tree" },        { L"F5", L"Quicksave" },
        { L"F9", L"Quickload" },             { L"Ctrl+S", L"Save preset" },       { L"Ctrl+R", L"Load preset" },
        { L"P", L"Copy share link" },        { L"U", L"Open a share link" },
        { L"Ctrl+F", L"Join Lobby" },        { L"Ctrl+Z", L"Undo a draw or reset" }, { L"Ctrl+X", L"Redraw" },
        { L"Ctrl+T", L"Open Sustainability App" }, { L"Z", L"Toggle civ name" },  { L"X", L"Toggle civ icon" },
        { L"C", L"Toggle round counter" },   { L"V", L"Toggle civ jingles" },     { L"B", L"Toggle UI sounds" },
        { L"Esc", L"Exit" },
    };
    const HotkeyItem DRAW_TAB_HOTKEYS[] = {
        { L"Space", L"Draw civ" },    { L"Enter", L"Reset drawn civs" },           { L"Q", L"Open Options" },
        { L"R", L"Open Sustainability App" }, { L"T", L"Open Tech Tree" },        { L"H", L"Open History" },
    };
    const HotkeyItem LOG_TAB_HOTKEYS[] = {
        { L"Space", L"Draw civ" },
        { L"Enter", L"Reset drawn civs" },
        { L"Q", L"Clear drawn log" },
        { L"W", L"Toggle remaining log" },
    };

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

    // Non-ASCII characters are written as \u escapes: without a byte order mark, MSVC reads a source file in the
    // system code page.
    const wchar_t* const AUTHOR_ALIAS = L"Hj\u00F6rleif";
    const wchar_t* const ABOUT_CREATED = L"Created by Carl Nordenbrink, alias Hj\u00F6rleif. Also available as a "
                                         L"web version.";
    const wchar_t* const ABOUT_SAVED =
        L"Custom civ pool, draw log and user settings are saved in the program's folder.";
    constexpr int HISTORY_TEXT_LENGTH = 256;
    constexpr int MAX_ADDED_TEXT = 128;

    UINT ToCheckState(bool checked) { return checked ? BST_CHECKED : BST_UNCHECKED; }
}

// ---------------------------------------------------------------------------------------------------
// Running the dialogs

CivPickerView* CivPickerView::AttachDialog(HWND dialog, UINT message, LPARAM lParam)
{
    if (!c_assert(dialog != NULL)) return nullptr;
    if (message == WM_INITDIALOG) (void)SetWindowLongPtr(dialog, DWLP_USER, lParam);
    return reinterpret_cast<CivPickerView*>(GetWindowLongPtr(dialog, DWLP_USER));
}

INT_PTR CivPickerView::RunDialog(const ParchmentDialogs::Run& run)
{
    if (!c_assert(run.owner != NULL)) return -1;
    const INT_PTR result = dialogs.RunModal(run);
    // The dialog's input went through its own message loop; its closing counts as the last activity.
    NoteActivity();
    return result;
}

void CivPickerView::ShowOptionsDialog(HWND owner)
{
    if (!c_assert(owner != NULL)) return;
    (void)RunDialog({ owner, OptionsDialogProc, reinterpret_cast<LPARAM>(this), &OPTIONS_LAYOUT });
}

void CivPickerView::ShowHotkeysDialog(HWND owner)
{
    if (!c_assert(owner != NULL)) return;
    (void)RunDialog({ owner, HotkeysDialogProc, reinterpret_cast<LPARAM>(this), &hotkeys_layout });
}

void CivPickerView::ShowAboutDialog(HWND owner)
{
    if (!c_assert(owner != NULL)) return;
    (void)RunDialog({ owner, AboutDialogProc, reinterpret_cast<LPARAM>(this), &about_layout });
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

// A message in the web version's message dialog (#dlgMsg), over the window that is active.
void CivPickerView::ShowInformation(const wchar_t* text, const wchar_t* caption)
{
    if (!c_assert(text != nullptr && caption != nullptr)) return;
    if (main_window == NULL || !BuildMessageLayout(text, caption))
    {
        MessageBox(main_window, text, caption, MB_OK | MB_ICONINFORMATION);
        return;
    }
    const HWND active = GetActiveWindow();
    const HWND owner = active != NULL ? active : main_window;
    if (RunDialog({ owner, MessageDialogProc, reinterpret_cast<LPARAM>(this), &message_layout }) == -1)
        MessageBox(main_window, text, caption, MB_OK | MB_ICONINFORMATION);
}

CivPickerView::FileChoice CivPickerView::AskSaveFilePath(const std::wstring& default_name,
                                                         const std::wstring& initial_folder) const
{
    if (!c_assert(default_name.size() < MAX_PATH) || !c_assert(initial_folder.size() < MAX_PATH)) return { false, L"" };
    wchar_t file_name[MAX_PATH] = L"";
    wcscpy_s(file_name, default_name.c_str());

    OPENFILENAME dialog_settings = {};
    dialog_settings.lStructSize = sizeof(dialog_settings);
    dialog_settings.hwndOwner = NULL;
    dialog_settings.lpstrFile = file_name;
    dialog_settings.nMaxFile = MAX_PATH;
    dialog_settings.lpstrFilter = L"Text Files\0*.txt\0All Files\0*.*\0";
    dialog_settings.nFilterIndex = 1;
    dialog_settings.lpstrInitialDir = initial_folder.c_str();
    dialog_settings.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;

    if (GetSaveFileName(&dialog_settings) != TRUE) return { false, L"" };
    return { true, file_name };
}

CivPickerView::FileChoice CivPickerView::AskOpenFilePath(const std::wstring& initial_folder) const
{
    if (!c_assert(initial_folder.size() < MAX_PATH) || !c_assert(main_window != NULL)) return { false, L"" };
    wchar_t file_name[MAX_PATH] = L"";

    OPENFILENAME dialog_settings = {};
    dialog_settings.lStructSize = sizeof(dialog_settings);
    dialog_settings.hwndOwner = main_window;
    dialog_settings.lpstrFile = file_name;
    dialog_settings.nMaxFile = MAX_PATH;
    dialog_settings.lpstrFilter = L"Text Files\0*.txt\0All Files\0*.*\0";
    dialog_settings.nFilterIndex = 1;
    dialog_settings.lpstrInitialDir = initial_folder.c_str();
    dialog_settings.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;

    if (GetOpenFileName(&dialog_settings) != TRUE) return { false, L"" };
    return { true, file_name };
}

// ---------------------------------------------------------------------------------------------------
// Layouts made from measured text (at startup, except a message's)

bool CivPickerView::BuildDialogLayouts() { return BuildHotkeysLayout() && BuildAboutLayout(); }

float CivPickerView::KeyWidth(const wchar_t* key)
{
    if (!c_assert(key != nullptr)) return KEY_MIN_WIDTH;
    const float text = renderer.MeasureText({ key, TextLength(key), FontId::key });
    return text + KEY_PADDING > KEY_MIN_WIDTH ? text + KEY_PADDING : KEY_MIN_WIDTH;
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
    const int rows_used = section.items.count < section.rows ? section.items.count : section.rows;
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
    if (!BuildPoolHotkeys()) return false;
    hotkey_decoration_count = 0;
    AddHotkeyDecoration({ DecorationKind::hint, { BODY_LEFT, BODY_TOP, 800, BODY_TOP + 17 },
                          L"Hotkeys work while the Fresh Random Civ Picker window is active." });
    const float grid_top = BODY_TOP + DIALOG_LINE_HEIGHT + PARAGRAPH_GAP;
    const float global_column = HotkeyItemWidth({ GLOBAL_HOTKEYS, CountOf(GLOBAL_HOTKEYS) });
    const float tab_column = HotkeyItemWidth({ DRAW_TAB_HOTKEYS, CountOf(DRAW_TAB_HOTKEYS) }) >
                                     HotkeyItemWidth({ LOG_TAB_HOTKEYS, CountOf(LOG_TAB_HOTKEYS) })
                                 ? HotkeyItemWidth({ DRAW_TAB_HOTKEYS, CountOf(DRAW_TAB_HOTKEYS) })
                                 : HotkeyItemWidth({ LOG_TAB_HOTKEYS, CountOf(LOG_TAB_HOTKEYS) });
    const float pool_column = HotkeyItemWidth({ pool_hotkeys, pool_hotkey_count });
    const float tab_left = BODY_LEFT + 2.0f * global_column + 2.0f * HOTKEY_COLUMN_GAP;
    const float pool_left = tab_left + tab_column + HOTKEY_COLUMN_GAP;

    const float global_bottom = AddHotkeySection({ L"Global", { GLOBAL_HOTKEYS, CountOf(GLOBAL_HOTKEYS) },
                                                   BODY_LEFT, grid_top, global_column, GLOBAL_ROWS });
    const float draw_bottom = AddHotkeySection({ L"Draw Civ tab", { DRAW_TAB_HOTKEYS, CountOf(DRAW_TAB_HOTKEYS) },
                                                 tab_left, grid_top, tab_column, CountOf(DRAW_TAB_HOTKEYS) });
    const float log_bottom = AddHotkeySection({ L"Log tab", { LOG_TAB_HOTKEYS, CountOf(LOG_TAB_HOTKEYS) }, tab_left,
                                                draw_bottom + HOTKEY_SECTION_GAP, tab_column,
                                                CountOf(LOG_TAB_HOTKEYS) });
    const float pool_bottom = AddHotkeySection({ L"Civ Pool tab", { pool_hotkeys, pool_hotkey_count }, pool_left,
                                                 grid_top, pool_column, pool_hotkey_count });
    float grid_bottom = global_bottom > log_bottom ? global_bottom : log_bottom;
    if (pool_bottom > grid_bottom) grid_bottom = pool_bottom;

    const int width = Rounded(pool_left + pool_column) + BUTTON_ROW_PADDING + PANEL_PADDING;
    const PixelSize size = { width, HeightBelowBody(grid_bottom - PARAGRAPH_GAP) };
    hotkeys_parts[0] = { PartKind::default_button, IDOK, BottomRightButton(size), L"OK" };
    hotkeys_layout = { L"Hotkeys", size, hotkeys_parts, 1, hotkey_decorations, hotkey_decoration_count, nullptr, 0,
                       IDOK };
    return c_assert(size.width <= ParchmentDialogs::MAX_DIALOG_WIDTH &&
                    size.height <= ParchmentDialogs::MAX_DIALOG_HEIGHT);
}

// The Civ Pool tab's keys: the fixed ones, one for each edition, and one for each place in the editions' lists of
// DLC toggles, which toggles the DLCs in that place (the web version's dlcSlotText).
bool CivPickerView::BuildPoolHotkeys()
{
    const CivCatalog& catalog = model.Catalog();
    pool_hotkey_count = 0;
    AddPoolHotkey(L"Space", L"Enable all civs");
    AddPoolHotkey(L"Enter", L"Disable all civs");
    for (int edition = 0; edition < catalog.EditionCount() && edition < MAX_EDITIONS; edition++)
        AddPoolHotkey(EDITION_HOTKEY_KEYS[edition], catalog.EditionAt(edition).short_name);
    AddPoolHotkey(L"R", L"Auto-toggle older civs");
    AddPoolHotkey(L"T", L"Auto-reset on change");
    for (int slot = 0; slot < MAX_DLC_TOGGLES; slot++)
    {
        const std::wstring names = DlcSlotNames(slot);
        if (!names.empty()) AddPoolHotkey(DLC_SLOT_HOTKEY_KEYS[slot], L"Toggle " + names);
    }
    return c_assert(pool_hotkey_count > 0 && pool_hotkey_count <= MAX_POOL_HOTKEYS);
}

void CivPickerView::AddPoolHotkey(const wchar_t* key, const std::wstring& text)
{
    if (!c_assert(key != nullptr) || !c_assert(pool_hotkey_count < MAX_POOL_HOTKEYS)) return;
    // A text longer than its buffer is cut short, which still shows its start.
    (void)wcsncpy_s(pool_hotkey_texts[pool_hotkey_count], text.c_str(), _TRUNCATE);
    pool_hotkeys[pool_hotkey_count] = { key, pool_hotkey_texts[pool_hotkey_count] };
    pool_hotkey_count++;
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

// The height of a dialog paragraph wrapped at width: its lines at the dialog's line height.
float CivPickerView::ParagraphHeight(const wchar_t* text, float width)
{
    if (!c_assert(text != nullptr) || !c_assert(width > 0.0f)) return DIALOG_LINE_HEIGHT;
    const TextSpan span = { text, TextLength(text), FontId::ui };
    const bool wrapped = paragraph_lines.Wrap({ text, renderer.MeasureAdvances(span), span.length, width });
    const int lines = wrapped ? paragraph_lines.Count() : 1;
    return static_cast<float>(lines) * DIALOG_LINE_HEIGHT;
}

// #dlgAbout: the name and version, the author with links, where the program keeps its data, and the notice.
bool CivPickerView::BuildAboutLayout()
{
    swprintf_s(about_version, L", desktop version v%s", VERSION);
    ComposeAboutLegal();
    const wchar_t* name = L"Fresh Random Civ Picker";
    const int name_right = BODY_LEFT + Rounded(renderer.MeasureText({ name, TextLength(name), FontId::ui_bold }));
    const float width = 412.0f;
    const float created_top = BODY_TOP + DIALOG_LINE_HEIGHT + PARAGRAPH_GAP;
    const float saved_top = created_top + ParagraphHeight(ABOUT_CREATED, width) + PARAGRAPH_GAP;
    const float legal_top = saved_top + ParagraphHeight(ABOUT_SAVED, width) + PARAGRAPH_GAP;
    const float legal_bottom = legal_top + ParagraphHeight(about_legal, width);
    const int right = BODY_LEFT + Rounded(width);
    about_decorations[0] = { DecorationKind::bold_text, { BODY_LEFT, BODY_TOP, name_right, BODY_TOP + 17 }, name };
    about_decorations[1] = { DecorationKind::text, { name_right, BODY_TOP, right, BODY_TOP + 17 }, about_version };
    about_decorations[2] = { DecorationKind::paragraph, { BODY_LEFT, Rounded(created_top), right, Rounded(saved_top) },
                             ABOUT_CREATED };
    about_decorations[3] = { DecorationKind::paragraph, { BODY_LEFT, Rounded(saved_top), right, Rounded(legal_top) },
                             ABOUT_SAVED };
    about_decorations[4] = { DecorationKind::hint, { BODY_LEFT, Rounded(legal_top), right, Rounded(legal_bottom) },
                             about_legal };
    about_links[0] = { 2, AUTHOR_ALIAS, IDC_HYPERLINK };
    about_links[1] = { 2, L"web version", IDM_WEBVERSION };
    about_links[2] = { 4, L"Game Content Usage Rules", IDM_USAGERULES };
    const PixelSize size = { 466, HeightBelowBody(legal_bottom) };
    about_parts[0] = { PartKind::default_button, IDOK, BottomRightButton(size), L"OK" };
    about_layout = { L"About Fresh Random Civ Picker", size, about_parts, 1, about_decorations, 5, about_links, 3,
                     IDOK };
    return c_assert(size.height <= ParchmentDialogs::MAX_DIALOG_HEIGHT);
}

// The notice names the game and the edition whose assets the program uses, the first in data.json.
void CivPickerView::ComposeAboutLegal()
{
    const EditionInfo& edition = model.Catalog().EditionAt(0);
    const std::wstring legal = edition.series + L" \u00A9 Microsoft Corporation. Fresh Random Civ Picker was created "
                               L"under Microsoft's \"Game Content Usage Rules\" using assets from " + edition.series +
                               L" and " + edition.series + L": " + edition.name +
                               L", and it is not endorsed by or affiliated with Microsoft.";
    // A notice longer than its buffer is cut short, which still shows its start.
    (void)wcsncpy_s(about_legal, legal.c_str(), _TRUNCATE);
}

// #dlgMsg: its title and its text, which wraps; the dialog grows with the text.
bool CivPickerView::BuildMessageLayout(const wchar_t* text, const wchar_t* caption)
{
    if (!c_assert(text != nullptr && caption != nullptr)) return false;
    if (wcsncpy_s(message_text, text, _TRUNCATE) == STRUNCATE) message_text[MAX_MESSAGE_TEXT - 1] = L'\0';
    if (wcsncpy_s(message_title, caption, _TRUNCATE) == STRUNCATE) message_title[MAX_MESSAGE_TITLE - 1] = L'\0';
    const float width = 352.0f;
    const float text_bottom = BODY_TOP + ParagraphHeight(message_text, width);
    const PixelSize size = { 406, HeightBelowBody(text_bottom) };
    if (!c_assert(size.height <= ParchmentDialogs::MAX_DIALOG_HEIGHT)) return false;
    const PixelRect text_area = { BODY_LEFT, BODY_TOP, BODY_LEFT + Rounded(width), Rounded(text_bottom) };
    message_decorations[0] = { DecorationKind::paragraph, text_area, message_text };
    message_parts[0] = { PartKind::default_button, IDOK, BottomRightButton(size), L"OK" };
    message_layout = { message_title, size, message_parts, 1, message_decorations, 1, nullptr, 0, IDOK };
    return true;
}

// ---------------------------------------------------------------------------------------------------
// Dialog procedures. Every dialog procedure is a function pointer Win32 requires (CLAUDE2.md I2); the view
// travels to it as the DialogBoxParam argument and is kept in the dialog's DWLP_USER slot. The parchment
// look's messages go to the dialog framework first.

INT_PTR CALLBACK CivPickerView::AboutDialogProc(HWND dialog, UINT message, WPARAM wParam,
                                                LPARAM lParam) noexcept
{
    CivPickerView* view = AttachDialog(dialog, message, lParam);
    if (view == nullptr) return FALSE;
    if (message == WM_INITDIALOG) return view->BeginDialog(dialog);
    const ParchmentDialogs::Handled frame = view->dialogs.HandleMessage({ dialog, message, wParam, lParam });
    if (frame.handled) return frame.result;
    if (message != WM_COMMAND) return FALSE;
    return view->OnAboutCommand(dialog, LOWORD(wParam)) ? TRUE : FALSE;
}

bool CivPickerView::OnAboutCommand(HWND dialog, int command_id)
{
    if (!c_assert(dialog != NULL)) return false;
    if (command_id == IDC_HYPERLINK)
    {
        controller.OpenAuthorPage();
        return true;
    }
    if (command_id == IDM_WEBVERSION || command_id == IDM_USAGERULES) return controller.OnCommand(command_id);
    if (command_id != IDOK && command_id != IDCANCEL) return false;
    PlayAudio(SoundType::button);
    return EndDialog(dialog, command_id) != FALSE;
}

// Builds a dialog from its layout when it opens; one that can't be built closes again at once.
INT_PTR CivPickerView::BeginDialog(HWND dialog)
{
    if (!c_assert(dialog != NULL)) return FALSE;
    if (dialogs.Build(dialog)) return TRUE;
    (void)EndDialog(dialog, -1);
    return TRUE;
}

INT_PTR CALLBACK CivPickerView::HotkeysDialogProc(HWND dialog, UINT message, WPARAM wParam,
                                                LPARAM lParam) noexcept
{
    CivPickerView* view = AttachDialog(dialog, message, lParam);
    if (view == nullptr) return FALSE;
    if (message == WM_INITDIALOG) return view->BeginDialog(dialog);
    const ParchmentDialogs::Handled frame = view->dialogs.HandleMessage({ dialog, message, wParam, lParam });
    if (frame.handled) return frame.result;
    if (message != WM_COMMAND || (LOWORD(wParam) != IDCANCEL && LOWORD(wParam) != IDOK)) return FALSE;
    view->PlayAudio(SoundType::button);
    return EndDialog(dialog, LOWORD(wParam));
}

INT_PTR CALLBACK CivPickerView::MessageDialogProc(HWND dialog, UINT message, WPARAM wParam,
                                                LPARAM lParam) noexcept
{
    CivPickerView* view = AttachDialog(dialog, message, lParam);
    if (view == nullptr) return FALSE;
    if (message == WM_INITDIALOG) return view->BeginDialog(dialog);
    const ParchmentDialogs::Handled frame = view->dialogs.HandleMessage({ dialog, message, wParam, lParam });
    if (frame.handled) return frame.result;
    if (message != WM_COMMAND || (LOWORD(wParam) != IDCANCEL && LOWORD(wParam) != IDOK)) return FALSE;
    view->PlayAudio(SoundType::button);
    return EndDialog(dialog, LOWORD(wParam));
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
    // The first edition's jingles, or the original game's.
    const std::wstring& first_edition = model.Catalog().EditionAt(0).name;
    SendMessage(jingle_type, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(first_edition.c_str()));
    SendMessage(jingle_type, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Legacy"));
    SendMessage(jingle_type, CB_SETCURSEL, settings.legacy_jingle_enabled ? 1 : 0, 0);
    if (!settings.jingles_enabled) ShowJingleTypeOption(dialog, false);

    HWND strength_slider = GetDlgItem(dialog, IDC_SLIDER_CONTFRESHSTRENGTH);
    if (!c_assert(strength_slider != NULL)) return;
    SendMessage(strength_slider, TBM_SETRANGE, TRUE, MAKELPARAM(1, 10));
    SendMessage(strength_slider, TBM_SETPOS, TRUE, settings.contfresh_strength);
    if (!settings.contfresh_enabled) ShowContfreshOptions(dialog, false);
    SetContfreshValueText(dialog, static_cast<int>(SendMessage(strength_slider, TBM_GETPOS, 0, 0)));
}

void CivPickerView::SetContfreshValueText(HWND dialog, int value) const
{
    if (!c_assert(dialog != NULL) || !c_assert(value >= 1 && value <= 10)) return;
    wchar_t value_text[64];
    swprintf_s(value_text, L"Continuous freshness strength: %d", value);
    SetDlgItemText(dialog, IDC_STATIC_CONTFRESHVALUE, value_text);
}

void CivPickerView::OnContfreshSlider(HWND dialog, HWND slider)
{
    if (!c_assert(dialog != NULL)) return;
    if (slider != GetDlgItem(dialog, IDC_SLIDER_CONTFRESHSTRENGTH)) return;
    const int position = static_cast<int>(SendMessage(slider, TBM_GETPOS, 0, 0));
    SetContfreshValueText(dialog, position);
    controller.OnContfreshStrengthChanged(position);
}

bool CivPickerView::OnOptionsCommand(HWND dialog, int command_id, int notification)
{
    if (!c_assert(dialog != NULL)) return false;
    OptionsEvent options_event;
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

// The web version greys out the jingle type and the freshness slider while they don't apply.
void CivPickerView::ShowJingleTypeOption(HWND dialog, bool shown) const
{
    if (!c_assert(dialog != NULL)) return;
    (void)EnableWindow(GetDlgItem(dialog, IDC_LEGACY_OPTION), shown ? TRUE : FALSE);
}

void CivPickerView::ShowContfreshOptions(HWND dialog, bool shown) const
{
    if (!c_assert(dialog != NULL)) return;
    (void)EnableWindow(GetDlgItem(dialog, IDC_SLIDER_CONTFRESHSTRENGTH), shown ? TRUE : FALSE);
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

// An invalid code shows the web version's error line under the field, and the dialog grows to make room for it.
bool CivPickerView::SubmitLobbyCode(HWND dialog, int command_id)
{
    if (!c_assert(dialog != NULL)) return false;
    if (!c_assert(command_id == IDC_BUTTON_OK || command_id == IDC_BUTTON_SPEC)) return false;
    PlayAudio(SoundType::button);
    wchar_t entered_code[256];
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
    // Before a civ is drawn the list starts at its first civ.
    if (model.FindCivIndex(history_selected_civ) == CivCatalog::NONE)
        history_selected_civ = catalog.CivAt(catalog.DisplayCiv(0)).name;

    UpdateCivAddedInfo(dialog, model.GetCiv(history_selected_civ).dlc);
    UpdateCivPoolInfo(dialog, history_selected_civ);
    SetDlgItemTextW(dialog, IDC_HISTORY_EDIT, model.FetchHistory(history_selected_civ).c_str());

    HWND civ_list_box = GetDlgItem(dialog, IDC_HISTORY_COMBO);
    // In the order of their names, as the Civ Pool tab shows them.
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

// The civ's place in the History dialog's list, which lists the civs in the order of their names.
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

// Selects the first civ whose name starts with the typed text, while the text grows. The selection
// change is handled by a direct call rather than a WM_COMMAND sent to this dialog (CLAUDE2.md I7).
void CivPickerView::OnHistorySearchChanged(HWND dialog)
{
    if (!c_assert(dialog != NULL)) return;
    HWND search_box = GetDlgItem(dialog, IDC_HIS_SEARCH);
    HWND civ_list_box = GetDlgItem(dialog, IDC_HISTORY_COMBO);
    wchar_t search_text[HISTORY_TEXT_LENGTH];
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

// When the civ's DLC came out, and its name (data.json's released and name).
void CivPickerView::UpdateCivAddedInfo(HWND dialog, int dlc) const
{
    if (!c_assert(dialog != NULL) || !c_assert(dlc >= 0 && dlc < model.Catalog().DlcCount())) return;
    const DlcInfo& info = model.Catalog().DlcAt(dlc);
    wchar_t added[MAX_ADDED_TEXT] = L"";
    // A text longer than its buffer is cut short, which still shows the date.
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
