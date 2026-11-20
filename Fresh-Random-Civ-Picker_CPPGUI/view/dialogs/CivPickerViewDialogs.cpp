#include "CivPickerView.h"

#include <commdlg.h>
#include <algorithm>
#include <cwchar>

#include "Assertions.h"
#include "CivPickerController.h"
#include "DialogMetrics.h"
#include "Resource.h"

namespace
{
    const wchar_t* const AUTHOR_ALIAS = L"Hj\u00F6rleif";
    const wchar_t* const ABOUT_CREATED = L"Created by Carl Nordenbrink, alias Hj\u00F6rleif. Also available as a "
                                         L"web version.";
    const wchar_t* const ABOUT_SAVED =
        L"Custom civ pool, draw log and user settings are saved in the program's folder.";
}

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
    NoteActivity();
    return result;
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

bool CivPickerView::BuildDialogLayouts() { return BuildHotkeyLayouts() && BuildAboutLayout(); }

float CivPickerView::ParagraphHeight(const wchar_t* text, float width)
{
    if (!c_assert(text != nullptr) || !c_assert(width > 0.0f)) return DIALOG_LINE_HEIGHT;
    const TextSpan span = { text, TextLength(text), FontId::ui };
    const bool wrapped = paragraph_lines.Wrap({ text, renderer.MeasureAdvances(span), span.length, width });
    const int lines = wrapped ? paragraph_lines.Count() : 1;
    return static_cast<float>(lines) * DIALOG_LINE_HEIGHT;
}

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

void CivPickerView::ComposeAboutLegal()
{
    const EditionInfo& edition = model.Catalog().EditionAt(0);
    const std::wstring legal = edition.series + L" \u00A9 Microsoft Corporation. Fresh Random Civ Picker was created "
                               L"under Microsoft's \"Game Content Usage Rules\" using assets from " + edition.series +
                               L" and " + edition.series + L": " + edition.name +
                               L", and it is not endorsed by or affiliated with Microsoft.";
    (void)wcsncpy_s(about_legal, legal.c_str(), _TRUNCATE);
}

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
