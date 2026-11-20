#include "ParchmentLog.h"

#include <windowsx.h>
#include <algorithm>
#include <cmath>
#include <cwchar>

#include "Assertions.h"

namespace
{
    constexpr int COMMAND_COPY = 1;
    constexpr int COMMAND_SELECT_ALL = 2;
}

void ParchmentLog::PaintSelection(ParchmentRenderer& renderer, int line_index, float top)
{
    const int first = std::min(selection_anchor, selection_caret);
    const int last = std::max(selection_anchor, selection_caret);
    if (!c_assert(line_index >= 0 && line_index < lines.Count())) return;
    if (!c_assert(first >= 0 && last <= text_length)) return;
    const TextLine line = lines.Line(line_index);
    const int start = std::max(first, line.start);
    const int end = std::min(last, line.start + line.length);
    if (start >= end) return;
    const COLORREF highlight = GetSysColor(COLOR_HIGHLIGHT);
    const Colour colour = { GetRValue(highlight), GetGValue(highlight), GetBValue(highlight), 1.0f };
    renderer.FillRect({ CharacterX(line_index, start), top, CharacterX(line_index, end), top + line_height }, colour);
}

void ParchmentLog::OnContextMenu(LPARAM lParam)
{
    if (!c_assert(window != NULL)) return;
    if (context.text_menu == NULL) return;
    POINT point = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
    if (point.x == -1 && point.y == -1)
    {
        point = { 0, 0 };
        (void)ClientToScreen(window, &point);
    }
    const BOOL command = TrackPopupMenu(context.text_menu, TPM_RETURNCMD | TPM_NONOTIFY, point.x, point.y, 0, window,
                                        nullptr);
    if (command == COMMAND_COPY) CopySelection();
    if (command == COMMAND_SELECT_ALL) SelectAll();
}

int ParchmentLog::CharacterAt(PixelPoint point) const
{
    if (!c_assert(text_length >= 0 && text_length <= MAX_TEXT_LENGTH)) return 0;
    if (lines.Count() == 0) return 0;
    const float content_y = static_cast<float>(point.y - layout.text_area.top + scroll_top);
    const int line_index = static_cast<int>(std::floor(content_y / line_height));
    if (line_index < 0) return 0;
    if (line_index >= lines.Count()) return text_length;
    const TextLine line = lines.Line(line_index);
    float x = static_cast<float>(layout.text_area.left);
    for (int i = line.start; i < line.start + line.length && i < MAX_TEXT_LENGTH; i++)
    {
        if (static_cast<float>(point.x) < x + advances[i] / 2.0f) return i;
        x += advances[i];
    }
    return line.start + line.length;
}

float ParchmentLog::CharacterX(int line_index, int character) const
{
    float x = static_cast<float>(layout.text_area.left);
    if (!c_assert(line_index >= 0 && line_index < lines.Count())) return x;
    if (!c_assert(character >= 0 && character <= text_length)) return x;
    const TextLine line = lines.Line(line_index);
    for (int i = line.start; i < character && i < MAX_TEXT_LENGTH; i++) x += advances[i];
    return x;
}

void ParchmentLog::SelectAll()
{
    selection_anchor = 0;
    selection_caret = text_length;
    Redraw();
}

void ParchmentLog::CopySelection() const
{
    const int first = std::min(selection_anchor, selection_caret);
    const int last = std::max(selection_anchor, selection_caret);
    if (!c_assert(window != NULL)) return;
    if (!c_assert(first >= 0 && last <= text_length) || first == last || !OpenClipboard(window)) return;
    const SIZE_T bytes = static_cast<SIZE_T>(last - first + 1) * sizeof(wchar_t);
    HGLOBAL block = EmptyClipboard() ? GlobalAlloc(GMEM_MOVEABLE, bytes) : NULL;
    wchar_t* copy = block != NULL ? static_cast<wchar_t*>(GlobalLock(block)) : nullptr;
    if (copy != nullptr)
    {
        (void)wmemcpy_s(copy, static_cast<rsize_t>(last - first + 1), text + first, static_cast<rsize_t>(last - first));
        copy[last - first] = L'\0';
        (void)GlobalUnlock(block);
        if (SetClipboardData(CF_UNICODETEXT, block) != NULL) block = NULL;
    }
    if (block != NULL) (void)GlobalFree(block);
    (void)CloseClipboard();
}
