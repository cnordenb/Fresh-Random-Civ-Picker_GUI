#include "CivPickerView.h"

#include <cmath>
#include <cwchar>

#include "Assertions.h"
#include "DialogMetrics.h"
#include "Resource.h"

// Share links in the view: the web version's Share, Open Link and Open Shared Link dialogs (index.html #dlgShare,
// #dlgOpen, #dlgPreview), the clipboard, and the status messages (#toast) that say what happened.

namespace
{
    // --- Share (#dlgShare): .share { width: 600px }; .share-row { grid-template-columns: 120px 1fr auto; gap: 8px;
    // align-items: center; margin-bottom: 8px } with a 22 px field and a 23 px button ---
    constexpr int SHARE_BODY_WIDTH = 600;
    constexpr int SHARE_LABEL_WIDTH = 120;
    constexpr int SHARE_ROW_GAP = 8;
    constexpr int SHARE_ROW_HEIGHT = 23;
    constexpr int SHARE_FIELD_HEIGHT = 22;
    constexpr int SHARE_LABEL_DROP = 3; // a 17 px line centred in the 23 px row
    constexpr int LINE_BOX = 17;
    const wchar_t* const SHARE_INTRO =
        L"Anyone who opens one of these links gets this civ pool. The log of past rounds isn't included.";

    // --- Open Link (#dlgOpen): a .field of 420 px, its label 4 px above the field, and its error line ---
    constexpr int LINK_ERROR_TOP = 106;
    constexpr int LINK_ERROR_ROOM = 25; // the error line and its 8 px margin
    const wchar_t* const LINK_ERROR = L"That isn't a Fresh Random Civ Picker link.";
    const DialogDecoration OPEN_LINK_DECORATIONS[] = {
        { DecorationKind::text, { 27, 55, 447, 72 }, L"Paste a Fresh Random Civ Picker link:" },
    };
    const DialogPart OPEN_LINK_PARTS[] = {
        { PartKind::text_field, IDC_EDIT_SHARELINK, { 27, 76, 447, 98 }, nullptr },
        { PartKind::error_label, IDC_SHARELINK_ERROR, { 27, LINK_ERROR_TOP, 447, 123 }, L"" },
        { PartKind::default_button, IDC_BUTTON_OPENLINK, { 289, 118, 364, 141 }, L"Open" },
        { PartKind::button, IDCANCEL, { 372, 118, 447, 141 }, L"Cancel" },
    };
    const DialogLayout OPEN_LINK_LAYOUT = { L"Open Link", { DialogWidth(420), 168 }, OPEN_LINK_PARTS, 4,
                                            OPEN_LINK_DECORATIONS, 1, nullptr, 0, IDC_BUTTON_OPENLINK };

    // --- Open Shared Link (#dlgPreview): .preview { width: 440px }; .facts { grid-template-columns: auto 1fr;
    // gap: 3px 12px; margin: 0 0 10px } ---
    constexpr int PREVIEW_BODY_WIDTH = 440;
    constexpr float FACT_ROW_GAP = 3.0f;
    constexpr float FACT_COLUMN_GAP = 12.0f;
    constexpr float FACTS_BOTTOM_MARGIN = 10.0f;
    constexpr int MAX_LEFT_OUT_SHOWN = 12;
    const wchar_t* const PREVIEW_INTRO = L"This link holds:";
    const wchar_t* const PREVIEW_NOTE =
        L"Loading it replaces the current civ pool and current round. The log of past rounds is still kept.";

    PixelRect ShareLabelArea(int row_top)
    {
        return { BODY_LEFT, row_top + SHARE_LABEL_DROP, BODY_LEFT + SHARE_LABEL_WIDTH,
                 row_top + SHARE_LABEL_DROP + LINE_BOX };
    }

    PixelRect ShareFieldArea(int row_top)
    {
        const int left = BODY_LEFT + SHARE_LABEL_WIDTH + SHARE_ROW_GAP;
        const int right = BODY_LEFT + SHARE_BODY_WIDTH - DIALOG_BUTTON_WIDTH - SHARE_ROW_GAP;
        return { left, row_top, right, row_top + SHARE_FIELD_HEIGHT };
    }

    PixelRect ShareCopyArea(int row_top)
    {
        const int right = BODY_LEFT + SHARE_BODY_WIDTH;
        return { right - DIALOG_BUTTON_WIDTH, row_top, right, row_top + DIALOG_BUTTON_HEIGHT };
    }
}

// ---------------------------------------------------------------------------------------------------
// Share

void CivPickerView::ShowShareDialog(HWND owner, const ShareLinks& links)
{
    if (!c_assert(owner != NULL) || !BuildShareLayout(links)) return;
    (void)RunDialog({ owner, ShareDialogProc, reinterpret_cast<LPARAM>(this), &share_layout });
}

// #dlgShare: the pool's link, and its link with this round's draws, each in a read-only field with its Copy button,
// and a note on what the second link adds.
bool CivPickerView::BuildShareLayout(const ShareLinks& links)
{
    if (!c_assert(links.pool != nullptr && links.round != nullptr) || !c_assert(links.draw_count >= 0)) return false;
    (void)wcsncpy_s(share_pool_link, links.pool, _TRUNCATE);
    (void)wcsncpy_s(share_round_link, links.round, _TRUNCATE);
    if (links.draw_count == 0)
        (void)wcscpy_s(share_note, L"Nothing has been drawn this round, so both links are the same.");
    else
        (void)swprintf_s(share_note, L"The second link also carries the %d civ%ls drawn this round, in order.",
                         links.draw_count, links.draw_count == 1 ? L"" : L"s");
    const float width = static_cast<float>(SHARE_BODY_WIDTH);
    const int right = BODY_LEFT + SHARE_BODY_WIDTH;
    const float intro_bottom = BODY_TOP + ParagraphHeight(SHARE_INTRO, width);
    const int first_row = Rounded(intro_bottom + PARAGRAPH_GAP);
    const int second_row = first_row + SHARE_ROW_HEIGHT + SHARE_ROW_GAP;
    const float note_top = static_cast<float>(second_row + SHARE_ROW_HEIGHT + SHARE_ROW_GAP);
    const float note_bottom = note_top + ParagraphHeight(share_note, width);
    const PixelSize size = { DialogWidth(SHARE_BODY_WIDTH), HeightBelowBody(note_bottom) };
    share_decorations[0] = { DecorationKind::paragraph, { BODY_LEFT, BODY_TOP, right, Rounded(intro_bottom) },
                             SHARE_INTRO };
    share_decorations[1] = { DecorationKind::text, ShareLabelArea(first_row), L"Share pool" };
    share_decorations[2] = { DecorationKind::text, ShareLabelArea(second_row), L"Share pool + draws" };
    share_decorations[3] = { DecorationKind::hint, { BODY_LEFT, Rounded(note_top), right, Rounded(note_bottom) },
                             share_note };
    share_parts[0] = { PartKind::readonly_field, IDC_SHARE_POOL, ShareFieldArea(first_row), nullptr };
    share_parts[1] = { PartKind::button, IDC_COPY_POOL, ShareCopyArea(first_row), L"Copy" };
    share_parts[2] = { PartKind::readonly_field, IDC_SHARE_ROUND, ShareFieldArea(second_row), nullptr };
    share_parts[3] = { PartKind::button, IDC_COPY_ROUND, ShareCopyArea(second_row), L"Copy" };
    share_parts[4] = { PartKind::default_button, IDOK, BottomRightButton(size), L"Close" };
    share_layout = { L"Share", size, share_parts, 5, share_decorations, 4, nullptr, 0, IDOK };
    return c_assert(size.height <= ParchmentDialogs::MAX_DIALOG_HEIGHT);
}

// Dialog procedure of Share, a function pointer Win32 requires (CLAUDE2.md I2).
INT_PTR CALLBACK CivPickerView::ShareDialogProc(HWND dialog, UINT message, WPARAM wParam, LPARAM lParam) noexcept
{
    CivPickerView* view = AttachDialog(dialog, message, lParam);
    if (view == nullptr) return FALSE;
    if (message == WM_INITDIALOG) return view->BeginShareDialog(dialog);
    const ParchmentDialogs::Handled frame = view->dialogs.HandleMessage({ dialog, message, wParam, lParam });
    if (frame.handled) return frame.result;
    if (message != WM_COMMAND) return FALSE;
    return view->OnShareCommand(dialog, LOWORD(wParam)) ? TRUE : FALSE;
}

// The fields get their links, and Close gets the focus, as the web version's autofocus gives it.
INT_PTR CivPickerView::BeginShareDialog(HWND dialog)
{
    if (!c_assert(dialog != NULL)) return FALSE;
    const INT_PTR built = BeginDialog(dialog);
    // A field that didn't take its link stays empty; its Copy button still copies the link.
    (void)SetDlgItemTextW(dialog, IDC_SHARE_POOL, share_pool_link);
    (void)SetDlgItemTextW(dialog, IDC_SHARE_ROUND, share_round_link);
    const HWND close = GetDlgItem(dialog, IDOK);
    if (close == NULL) return built;
    // The focus went where it was set, or stays where Windows put it.
    (void)SetFocus(close);
    return FALSE;
}

bool CivPickerView::OnShareCommand(HWND dialog, int command_id)
{
    if (!c_assert(dialog != NULL)) return false;
    if (command_id == IDC_COPY_POOL || command_id == IDC_COPY_ROUND)
    {
        CopyFieldLink(dialog, command_id == IDC_COPY_POOL ? IDC_SHARE_POOL : IDC_SHARE_ROUND);
        return true;
    }
    if (command_id != IDOK && command_id != IDCANCEL) return false;
    PlayAudio(SoundType::button);
    return EndDialog(dialog, command_id) != FALSE;
}

// Copies a field's link; if the clipboard can't take it, the link is selected for copying by hand.
void CivPickerView::CopyFieldLink(HWND dialog, int field_id)
{
    if (!c_assert(dialog != NULL) || !c_assert(field_id == IDC_SHARE_POOL || field_id == IDC_SHARE_ROUND)) return;
    if (CopyText(field_id == IDC_SHARE_POOL ? share_pool_link : share_round_link))
    {
        PlayAudio(SoundType::button);
        return;
    }
    const HWND field = GetDlgItem(dialog, field_id);
    if (field == NULL) return;
    // Focusing and selecting only help the user copy by hand; the dialog works without them.
    (void)SetFocus(field);
    (void)SendMessage(field, EM_SETSEL, 0, -1);
}

// ---------------------------------------------------------------------------------------------------
// Open Link

// The pool and round of a link the user pastes; not ok if the dialog was closed without one.
ShareLink::Opened CivPickerView::AskShareLink(HWND owner)
{
    opened_link = { false, {} };
    if (!c_assert(owner != NULL)) return opened_link;
    link_error_shown = false;
    const INT_PTR result = RunDialog({ owner, OpenLinkDialogProc, reinterpret_cast<LPARAM>(this), &OPEN_LINK_LAYOUT });
    if (result != IDOK) return { false, {} };
    return opened_link;
}

// Dialog procedure of Open Link, a function pointer Win32 requires (CLAUDE2.md I2).
INT_PTR CALLBACK CivPickerView::OpenLinkDialogProc(HWND dialog, UINT message, WPARAM wParam, LPARAM lParam) noexcept
{
    CivPickerView* view = AttachDialog(dialog, message, lParam);
    if (view == nullptr) return FALSE;
    if (message == WM_INITDIALOG) return view->BeginDialog(dialog);
    const ParchmentDialogs::Handled frame = view->dialogs.HandleMessage({ dialog, message, wParam, lParam });
    if (frame.handled) return frame.result;
    if (message != WM_COMMAND) return FALSE;
    const int command_id = LOWORD(wParam);
    if (command_id == IDC_BUTTON_OPENLINK || command_id == IDOK) return view->SubmitShareLink(dialog) ? TRUE : FALSE;
    if (command_id != IDCANCEL) return FALSE;
    view->PlayAudio(SoundType::button);
    return EndDialog(dialog, IDCANCEL);
}

// A text that isn't a link shows the web version's error line under the field, and the dialog grows to make room.
bool CivPickerView::SubmitShareLink(HWND dialog)
{
    if (!c_assert(dialog != NULL)) return false;
    const HWND field = GetDlgItem(dialog, IDC_EDIT_SHARELINK);
    if (!c_assert(field != NULL)) return false;
    share_link_input[0] = L'\0';
    // A text longer than any link can be isn't one; only a text that fits is read.
    const bool fits = GetWindowTextLengthW(field) <= ShareLink::MAX_INPUT_LENGTH;
    if (fits) (void)GetWindowTextW(field, share_link_input, ShareLink::MAX_INPUT_LENGTH + 1);
    opened_link = fits ? ShareLink::Open(model.Catalog(), share_link_input) : ShareLink::Opened{ false, {} };
    if (opened_link.ok) return EndDialog(dialog, IDOK) != FALSE;
    PlayAudio(SoundType::error);
    (void)SetDlgItemTextW(dialog, IDC_SHARELINK_ERROR, LINK_ERROR);
    if (!link_error_shown && dialogs.Grow(dialog, { LINK_ERROR_ROOM, LINK_ERROR_TOP, IDC_SHARELINK_ERROR }))
        link_error_shown = true;
    (void)ShowWindow(GetDlgItem(dialog, IDC_SHARELINK_ERROR), SW_SHOW);
    return false;
}

// ---------------------------------------------------------------------------------------------------
// Open Shared Link

// Shows what a link holds before it replaces the current pool and round; true if the user loads it.
bool CivPickerView::ConfirmSharedPool(HWND owner, const SharedPool& pool)
{
    if (!c_assert(owner != NULL) || !BuildPreviewLayout(pool)) return false;
    return RunDialog({ owner, PreviewDialogProc, reinterpret_cast<LPARAM>(this), &preview_layout }) == IDOK;
}

// Dialog procedure of Open Shared Link, a function pointer Win32 requires (CLAUDE2.md I2).
INT_PTR CALLBACK CivPickerView::PreviewDialogProc(HWND dialog, UINT message, WPARAM wParam, LPARAM lParam) noexcept
{
    CivPickerView* view = AttachDialog(dialog, message, lParam);
    if (view == nullptr) return FALSE;
    if (message == WM_INITDIALOG) return view->BeginDialog(dialog);
    const ParchmentDialogs::Handled frame = view->dialogs.HandleMessage({ dialog, message, wParam, lParam });
    if (frame.handled) return frame.result;
    const int command_id = LOWORD(wParam);
    if (message != WM_COMMAND || (command_id != IDOK && command_id != IDCANCEL)) return FALSE;
    // Loading the link plays its own sound.
    if (command_id == IDCANCEL) view->PlayAudio(SoundType::button);
    return EndDialog(dialog, command_id);
}

// #dlgPreview: what the link holds (its edition, civ pool and round) as a list of facts, and what loading it does.
bool CivPickerView::BuildPreviewLayout(const SharedPool& pool)
{
    const CivCatalog& catalog = model.Catalog();
    if (!c_assert(pool.edition >= 0 && pool.edition < catalog.EditionCount())) return false;
    (void)wcsncpy_s(preview_edition, catalog.EditionAt(pool.edition).name.c_str(), _TRUNCATE);
    ComposePreviewPool(pool);
    ComposePreviewRound(pool);
    const PreviewFact facts[] = { { L"Edition", preview_edition }, { L"Civ pool", preview_pool },
                                  { L"This round", preview_round } };
    float term_width = 0.0f;
    for (const PreviewFact& fact : facts)
    {
        const float width = renderer.MeasureText({ fact.term, TextLength(fact.term), FontId::ui });
        if (width > term_width) term_width = width;
    }
    preview_decoration_count = 0;
    const int right = BODY_LEFT + PREVIEW_BODY_WIDTH;
    AddPreviewDecoration({ DecorationKind::text, { BODY_LEFT, BODY_TOP, right, BODY_TOP + LINE_BOX }, PREVIEW_INTRO });
    float top = BODY_TOP + DIALOG_LINE_HEIGHT + PARAGRAPH_GAP;
    top = AddPreviewFact(facts[0], top, std::ceil(term_width)) + FACT_ROW_GAP;
    top = AddPreviewFact(facts[1], top, std::ceil(term_width)) + FACT_ROW_GAP;
    top = AddPreviewFact(facts[2], top, std::ceil(term_width));
    const float note_top = top + FACTS_BOTTOM_MARGIN;
    const float note_bottom = note_top + ParagraphHeight(PREVIEW_NOTE, static_cast<float>(PREVIEW_BODY_WIDTH));
    AddPreviewDecoration({ DecorationKind::paragraph, { BODY_LEFT, Rounded(note_top), right, Rounded(note_bottom) },
                           PREVIEW_NOTE });
    const PixelSize size = { DialogWidth(PREVIEW_BODY_WIDTH), HeightBelowBody(note_bottom) };
    const PixelRect cancel = BottomRightButton(size);
    preview_parts[0] = { PartKind::default_button, IDOK, ButtonLeftOf(cancel), L"Load Link" };
    preview_parts[1] = { PartKind::button, IDCANCEL, cancel, L"Cancel" };
    preview_layout = { L"Open Shared Link", size, preview_parts, 2, preview_decorations, preview_decoration_count,
                       nullptr, 0, IDOK };
    return c_assert(size.height <= ParchmentDialogs::MAX_DIALOG_HEIGHT);
}

// "N of M civs", and the names of the edition's civs it leaves out if there are at most 12 (as openPreview says it).
void CivPickerView::ComposePreviewPool(const SharedPool& pool)
{
    const CivCatalog& catalog = model.Catalog();
    if (!c_assert(pool.edition >= 0 && pool.edition < catalog.EditionCount())) return;
    int enabled = 0;
    int left_out = 0;
    for (int civ = 0; civ < catalog.CivCount() && civ < MAX_CIVS; civ++)
    {
        if (pool.enabled[civ]) enabled++;
        else if (catalog.InEdition(civ, pool.edition)) left_out++;
    }
    (void)swprintf_s(preview_pool, L"%d of %d civs", enabled, catalog.EditionCivCount(pool.edition));
    if (left_out == 0 || left_out > MAX_LEFT_OUT_SHOWN) return;
    // A list longer than its buffer is cut short, which still shows its start.
    (void)wcsncat_s(preview_pool, L". Left out: ", _TRUNCATE);
    int listed = 0;
    for (int position = 0; position < catalog.CivCount() && position < MAX_CIVS; position++)
    {
        const int civ = catalog.DisplayCiv(position);
        if (pool.enabled[civ] || !catalog.InEdition(civ, pool.edition)) continue;
        if (listed > 0) (void)wcsncat_s(preview_pool, L", ", _TRUNCATE);
        (void)wcsncat_s(preview_pool, catalog.CivAt(civ).name.c_str(), _TRUNCATE);
        listed++;
    }
}

// "N drawn:" and the civs in draw order, or that loading starts a new round.
void CivPickerView::ComposePreviewRound(const SharedPool& pool)
{
    const CivCatalog& catalog = model.Catalog();
    if (!c_assert(pool.draw_count >= 0 && pool.draw_count <= catalog.CivCount())) return;
    if (pool.draw_count == 0)
    {
        (void)wcscpy_s(preview_round, L"Nothing drawn. Loading the link starts a new round.");
        return;
    }
    (void)swprintf_s(preview_round, L"%d drawn: ", pool.draw_count);
    for (int i = 0; i < pool.draw_count && i < MAX_CIVS; i++)
    {
        if (!c_assert(pool.draws[i] >= 0 && pool.draws[i] < catalog.CivCount())) return;
        // A list longer than its buffer is cut short, which still shows its start.
        if (i > 0) (void)wcsncat_s(preview_round, L", ", _TRUNCATE);
        (void)wcsncat_s(preview_round, catalog.CivAt(pool.draws[i]).name.c_str(), _TRUNCATE);
    }
}

// A term in the hint colour and its text, which wraps beside it; returns where the row ends.
float CivPickerView::AddPreviewFact(const PreviewFact& fact, float top, float term_width)
{
    if (!c_assert(fact.term != nullptr && fact.text != nullptr) || !c_assert(term_width > 0.0f)) return top;
    const float text_left = static_cast<float>(BODY_LEFT) + term_width + FACT_COLUMN_GAP;
    const int right = BODY_LEFT + PREVIEW_BODY_WIDTH;
    const float height = ParagraphHeight(fact.text, static_cast<float>(right) - text_left);
    const int row_top = Rounded(top);
    AddPreviewDecoration({ DecorationKind::hint,
                           { BODY_LEFT, row_top, BODY_LEFT + Rounded(term_width), row_top + LINE_BOX }, fact.term });
    AddPreviewDecoration({ DecorationKind::paragraph, { Rounded(text_left), row_top, right, Rounded(top + height) },
                           fact.text });
    return top + height;
}

void CivPickerView::AddPreviewDecoration(const DialogDecoration& decoration)
{
    if (!c_assert(preview_decoration_count < MAX_PREVIEW_DECORATIONS) || !c_assert(decoration.text != nullptr)) return;
    preview_decorations[preview_decoration_count] = decoration;
    preview_decoration_count++;
}

// ---------------------------------------------------------------------------------------------------
// The clipboard and the status messages

// Puts a text on the clipboard; false if the clipboard is busy or the copy fails. The clipboard takes a memory
// block it then owns, the one allocation the platform demands here.
bool CivPickerView::CopyText(const wchar_t* text) const
{
    if (!c_assert(text != nullptr) || !c_assert(main_window != NULL)) return false;
    const size_t length = wcsnlen(text, ShareLink::MAX_LINK_LENGTH);
    if (!c_assert(length < static_cast<size_t>(ShareLink::MAX_LINK_LENGTH))) return false;
    if (!OpenClipboard(main_window)) return false;
    HGLOBAL block = EmptyClipboard() ? GlobalAlloc(GMEM_MOVEABLE, (length + 1) * sizeof(wchar_t)) : NULL;
    wchar_t* copy = block != NULL ? static_cast<wchar_t*>(GlobalLock(block)) : nullptr;
    bool copied = false;
    if (copy != nullptr)
    {
        (void)wmemcpy_s(copy, length + 1, text, length);
        copy[length] = L'\0';
        (void)GlobalUnlock(block);
        copied = SetClipboardData(CF_UNICODETEXT, block) != NULL;
        if (copied) block = NULL;
    }
    if (block != NULL) (void)GlobalFree(block);
    (void)CloseClipboard();
    return copied;
}

void CivPickerView::ShowLinkCopied(int draw_count)
{
    if (!c_assert(draw_count >= 0)) return;
    wchar_t message[ParchmentToast::MAX_TEXT_LENGTH] = L"Link copied: civ pool";
    if (draw_count > 0)
        (void)swprintf_s(message, L"Link copied: civ pool + %d drawn civ%ls", draw_count, draw_count == 1 ? L"" : L"s");
    ShowToast(message);
}

void CivPickerView::ShowLinkLoaded() { ShowToast(L"Loaded the shared link"); }

// The toast fades by the main window's timer, which stops once the message has gone.
void CivPickerView::ShowToast(const wchar_t* text)
{
    if (!c_assert(text != nullptr) || !c_assert(main_window != NULL)) return;
    // A timer that couldn't start leaves the message shown until the next one replaces it.
    if (toast.Show(text)) (void)SetTimer(main_window, TOAST_TIMER, ParchmentToast::TICK_MS, nullptr);
}

// Whether the Log tab shows any draws (past rounds or this one), which loading a link would keep but move on from.
bool CivPickerView::HasDrawnLog() const
{
    if (!c_assert(drawn_log.Window() != NULL)) return false;
    return GetWindowTextLengthW(drawn_log.Window()) > 0;
}
