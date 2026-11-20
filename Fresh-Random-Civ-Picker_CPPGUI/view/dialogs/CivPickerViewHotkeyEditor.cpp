#include "CivPickerView.h"

#include <algorithm>
#include <cwchar>

#include "Assertions.h"
#include "CivPickerController.h"
#include "DialogMetrics.h"
#include "Resource.h"

namespace
{
    using CivPickerViewDetail::SECTION_TITLES;
    const wchar_t* const SECTION_PLACES[HOTKEY_SCOPE_COUNT] = { L"every tab", L"Draw Civ tab", L"Log tab",
                                                                L"Civ Pool tab" };

    const wchar_t* const EDITOR_HINT = L"Click a hotkey's key, then press the key you want for it; Backspace leaves "
                                       L"it without one.";
    const wchar_t* const UNNAMED_KEY_HELP = L"Letters, digits, F1 to F11, the editing and arrow keys and the number "
                                            L"pad can, also with Ctrl, Alt or Shift.";
    constexpr int EDITOR_SECTIONS_TOP = 80;
    constexpr int EDITOR_SECTION_HEIGHT = 22;
    constexpr int EDITOR_SECTION_GAP = 16;
    constexpr int RADIO_TEXT_GAP = 6;
    constexpr int EDITOR_ROWS_TOP = 114;
    constexpr int EDITOR_ROW_HEIGHT = 24;
    constexpr int EDITOR_ROWS = 12;
    constexpr int EDITOR_KEY_WIDTH = 104;
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

    PixelRect EditorKeyArea(PixelPoint row_origin)
    {
        return { row_origin.x, row_origin.y + 1, row_origin.x + EDITOR_KEY_WIDTH,
                 row_origin.y + 1 + EDITOR_KEY_HEIGHT };
    }

    PixelRect EditorTextArea(PixelPoint row_origin, int column_width)
    {
        const int left = row_origin.x + EDITOR_KEY_WIDTH + EDITOR_TEXT_GAP;
        return { left, row_origin.y + 3, std::max(left + 1, row_origin.x + column_width), row_origin.y + 20 };
    }

    int RowsPerColumn(int row_count)
    {
        if (row_count > EDITOR_ROWS) return (row_count + 1) / 2;
        return std::max(row_count, 1);
    }
}

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

PixelPoint CivPickerView::EditorRowOrigin(int row, int rows_per_column) const
{
    if (!c_assert(row >= 0 && rows_per_column > 0)) return { BODY_LEFT, EDITOR_ROWS_TOP };
    const int column = row / rows_per_column;
    return { BODY_LEFT + column * (editor_column_width + EDITOR_COLUMN_GAP),
             EDITOR_ROWS_TOP + (row % rows_per_column) * EDITOR_ROW_HEIGHT };
}

bool CivPickerView::ShowHotkeyEditor(HWND owner)
{
    if (!c_assert(owner != NULL)) return false;
    RefreshHotkeyNames();
    editor_listening_row = NO_ROW;
    return RunDialog({ owner, HotkeyEditorProc, reinterpret_cast<LPARAM>(this), &editor_layout }) == IDOK;
}

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
    if (key != NULL) (void)SendMessage(key, KeyButton::STOP_LISTENING, 0, 0);
    editor_listening_row = NO_ROW;
    SetEditorStatus(editor, {});
}

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
    (void)SetWindowPos(key, NULL, key_area.left, key_area.top, 0, 0, flags);
    (void)SetWindowPos(text, NULL, text_area.left, text_area.top, 0, 0, flags);
    SetControlText(key, ShownKeyOf(editor_rows[row]));
    SetControlText(text, HotkeyTextOf(editor_rows[row]));
}

void CivPickerView::RefreshEditorKeys(HWND editor)
{
    if (!c_assert(editor != NULL)) return;
    for (int row = 0; row < editor_row_count && row < ParchmentDialogs::MAX_KEY_BUTTONS; row++)
    {
        const HWND key = GetDlgItem(editor, EDITOR_KEY_BASE_ID + row);
        if (key != NULL) SetControlText(key, ShownKeyOf(editor_rows[row]));
    }
}

void CivPickerView::ShowListeningStatus(HWND editor, HotkeyAction action)
{
    if (!c_assert(editor != NULL)) return;
    EditorStatus text = {};
    (void)_snwprintf_s(text.status, _TRUNCATE, L"Press the new key for %ls, or Backspace for no key.",
                       HotkeyTextOf(action));
    SetEditorStatus(editor, text);
}

void CivPickerView::ShowKeyChange(HWND editor, const KeyChange& change)
{
    if (!c_assert(editor != NULL)) return;
    RefreshHotkeyNames();
    RefreshEditorKeys(editor);
    SetEditorStatus(editor, DescribeKeyChange(change));
}

EditorStatus CivPickerView::DescribeKeyChange(const KeyChange& change) const
{
    EditorStatus text = {};
    const KeystrokeName key = NameKeystroke(change.keystroke);
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

EditorStatus CivPickerView::DescribeAssignedKey(const KeyChange& change) const
{
    EditorStatus text = {};
    const KeystrokeName key = NameKeystroke(change.keystroke);
    const wchar_t* const elsewhere =
        change.used_by_other_program ? L" Another program has it too, so it may not work." : L"";
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
        (void)wcsncat_s(text.detail, _countof(text.detail), part, _TRUNCATE);
    }
    (void)wcsncat_s(text.detail, _countof(text.detail), L".", _TRUNCATE);
    return text;
}

void CivPickerView::SetEditorStatus(HWND editor, const EditorStatus& text)
{
    if (!c_assert(editor != NULL)) return;
    SetControlText(GetDlgItem(editor, EDITOR_STATUS_ID), text.status);
    SetControlText(GetDlgItem(editor, EDITOR_DETAIL_ID), text.detail);
}
