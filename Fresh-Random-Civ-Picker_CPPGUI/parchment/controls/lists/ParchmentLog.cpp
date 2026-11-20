#include "ParchmentLog.h"

#include <windowsx.h>
#include <algorithm>
#include <cmath>
#include <cwchar>

#include "Assertions.h"
#include "ParchmentScrollBar.h"

namespace
{
    const wchar_t* const LOG_CLASS = L"FrcpParchmentLog";

    constexpr int BORDER = 6;
    constexpr int PADDING_TOP = 6;
    constexpr int PADDING_LEFT = 8;
    constexpr int PADDING_RIGHT = 2;
    constexpr int PADDING_RIGHT_WITH_BAR = 29;
    constexpr int FIRST_REPEAT_DELAY_MS = 350;
    constexpr int REPEAT_DELAY_MS = 50;
    constexpr UINT_PTR REPEAT_TIMER = 1;
    constexpr int ICON_SIZE = 18;
    constexpr int ICON_GAP = 4;
    constexpr float ICON_LINE_HEIGHT = 20.0f;

    HMENU ControlId(int id) { return reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)); }

    int ViewportHeight(PixelSize size)
    {
        if (!c_assert(size.height >= 2 * BORDER)) return 0;
        return size.height - 2 * BORDER;
    }
}

bool ParchmentLog::RegisterWindowClass(HINSTANCE instance)
{
    if (!c_assert(instance != NULL)) return false;
    WNDCLASSEXW class_info = {};
    class_info.cbSize = sizeof(class_info);
    class_info.lpfnWndProc = WindowProc;
    class_info.hInstance = instance;
    class_info.lpszClassName = LOG_CLASS;
    return c_assert(RegisterClassExW(&class_info) != 0);
}

bool ParchmentLog::Create(const ControlContext& control_context, const ControlPlacement& placement)
{
    if (!c_assert(window == NULL) || !c_assert(placement.parent != NULL)) return false;
    if (!c_assert(control_context.renderer != nullptr && control_context.back_buffer != nullptr)) return false;
    context = control_context;
    parent = placement.parent;
    size = { RectWidth(placement.bounds), RectHeight(placement.bounds) };
    const HINSTANCE instance = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(parent, GWLP_HINSTANCE));
    window = CreateWindowExW(0, LOG_CLASS, L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_CLIPSIBLINGS,
                             placement.bounds.left, placement.bounds.top, size.width, size.height, parent,
                             ControlId(placement.id), instance, this);
    if (!c_assert(window != NULL)) return false;
    LayOutText();
    return true;
}

bool ParchmentLog::ShowIcons(const Icons& line_icons)
{
    if (!c_assert(window != NULL) || !c_assert(line_icons.labels != nullptr)) return false;
    if (!c_assert(line_icons.columns > 0 && line_icons.cell_size > 0)) return false;
    icons = line_icons;
    line_height = ICON_LINE_HEIGHT;
    LayOutText();
    Redraw();
    return true;
}

void ParchmentLog::Release()
{
    window = NULL;
    parent = NULL;
    text[0] = L'\0';
    text_length = 0;
    scroll_top = 0;
    selection_anchor = 0;
    selection_caret = 0;
    drag = Drag::none;
}

HWND ParchmentLog::Window() const { return window; }

LRESULT CALLBACK ParchmentLog::WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_NCCREATE)
    {
        const CREATESTRUCT* create_struct = reinterpret_cast<const CREATESTRUCT*>(lParam);
        if (create_struct == nullptr) return FALSE;
        (void)SetWindowLongPtr(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(create_struct->lpCreateParams));
    }
    ParchmentLog* log = reinterpret_cast<ParchmentLog*>(GetWindowLongPtr(window, GWLP_USERDATA));
    if (log == nullptr) return DefWindowProc(window, message, wParam, lParam);
    if (message == WM_NCCREATE) log->window = window;
    if (log->window != window) return DefWindowProc(window, message, wParam, lParam);
    return log->HandleMessage(message, wParam, lParam);
}

LRESULT ParchmentLog::HandleMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
    if (!c_assert(window != NULL)) return 0;
    switch (message)
    {
        case WM_SETTEXT: return OnSetText(wParam, lParam);
        case WM_PAINT: OnPaint(); return 0;
        case WM_ERASEBKGND: return 1;
        case WM_LBUTTONDOWN: OnButtonDown(lParam); return 0;
        case WM_MOUSEMOVE: OnMouseMove(wParam, lParam); return 0;
        case WM_LBUTTONUP: OnButtonUp(); return 0;
        case WM_CAPTURECHANGED: OnCaptureLost(); return 0;
        case WM_TIMER: OnTimer(); return 0;
        case WM_MOUSEWHEEL: OnWheel(wParam); return 0;
        case WM_KEYDOWN: OnKey(wParam); return 0;
        case WM_CONTEXTMENU: OnContextMenu(lParam); return 0;
        case WM_SETCURSOR: return OnSetCursor(wParam, lParam);
        case WM_GETDLGCODE: return DLGC_WANTARROWS;
        default: return DefWindowProc(window, message, wParam, lParam);
    }
}

LRESULT ParchmentLog::OnSetText(WPARAM wParam, LPARAM lParam)
{
    if (!c_assert(window != NULL)) return FALSE;
    const LRESULT result = DefWindowProc(window, WM_SETTEXT, wParam, lParam);
    const wchar_t* new_text = reinterpret_cast<const wchar_t*>(lParam);
    if (!c_assert(new_text != nullptr)) return result;
    text_length = wcsncpy_s(text, new_text, _TRUNCATE) == 0 ? static_cast<int>(wcslen(text)) : 0;
    scroll_top = 0;
    selection_anchor = 0;
    selection_caret = 0;
    LayOutText();
    Redraw();
    return result;
}

void ParchmentLog::LayOutText()
{
    if (!c_assert(context.renderer != nullptr)) return;
    if (!c_assert(text_length >= 0 && text_length <= MAX_TEXT_LENGTH)) return;
    const float* measured = context.renderer->MeasureAdvances({ text, text_length, FontId::ui });
    for (int i = 0; i < text_length && i < MAX_TEXT_LENGTH; i++) advances[i] = text[i] == L'\r' ? 0.0f : measured[i];
    const int padding_width = size.width - 2 * BORDER - PADDING_LEFT - IconColumn();
    layout.has_scrollbar = false;
    if (!WrapText(padding_width - PADDING_RIGHT)) return;
    if (NeedsScrollbar({ layout.content_height, ViewportHeight(size), 0 }))
    {
        layout.has_scrollbar = true;
        if (!WrapText(padding_width - PADDING_RIGHT_WITH_BAR)) return;
    }
    scroll_top = ClampScrollTop({ layout.content_height, ViewportHeight(size), scroll_top });
}

bool ParchmentLog::WrapText(int text_width)
{
    if (!c_assert(text_width > 0) || !c_assert(text_width < size.width)) return false;
    if (!lines.Wrap({ text, advances, text_length, static_cast<float>(text_width) })) return false;
    const float lines_height = static_cast<float>(lines.Count()) * line_height;
    layout.content_height = static_cast<int>(std::ceil(lines_height)) + 2 * PADDING_TOP;
    const int text_left = BORDER + PADDING_LEFT + IconColumn();
    layout.text_area = { text_left, BORDER + PADDING_TOP, text_left + text_width, size.height - BORDER };
    return true;
}

void ParchmentLog::OnPaint()
{
    if (!c_assert(window != NULL)) return;
    PAINTSTRUCT paint = {};
    const HDC target = BeginPaint(window, &paint);
    PixelSurface* buffer = context.back_buffer;
    ParchmentRenderer* renderer = context.renderer;
    const bool ready = target != NULL && buffer != nullptr && renderer != nullptr;
    if (ready && c_assert(buffer->Contains({ 0, 0, size.width, size.height })) &&
        renderer->BeginFrame({ buffer->Dc(), size, ParchmentRenderer::SurfaceAlpha::opaque }))
    {
        PaintLog(*renderer, size);
        if (renderer->EndFrame())
            (void)BitBlt(target, 0, 0, size.width, size.height, buffer->Dc(), 0, 0, SRCCOPY);
    }
    (void)EndPaint(window, &paint);
}

void ParchmentLog::PaintLog(ParchmentRenderer& renderer, PixelSize log_size)
{
    if (!c_assert(log_size.width > 2 * BORDER && log_size.height > 2 * BORDER)) return;
    const FloatRect bounds = { 0.0f, 0.0f, static_cast<float>(log_size.width), static_cast<float>(log_size.height) };
    renderer.FillRect(bounds, LOG_COLOUR);
    renderer.DrawNineSlice({ NineSliceId::log_frame, bounds, static_cast<float>(BORDER), TileRule::repeat,
                             TileRule::repeat, true });
    renderer.PushClip({ static_cast<float>(BORDER), static_cast<float>(BORDER), bounds.right - BORDER,
                        bounds.bottom - BORDER });
    PaintLines(renderer);
    renderer.PopClip();
    if (layout.has_scrollbar) PaintScrollbar(renderer);
}

void ParchmentLog::PaintLines(ParchmentRenderer& renderer)
{
    if (!c_assert(HasArea(layout.text_area))) return;
    const float first_top = static_cast<float>(layout.text_area.top - scroll_top);
    for (int i = 0; i < lines.Count() && i < MAX_WRAPPED_LINES; i++)
    {
        const float top = first_top + static_cast<float>(i) * line_height;
        if (top + line_height < 0.0f || top > static_cast<float>(size.height)) continue;
        PaintSelection(renderer, i, top);
        PaintIcon(renderer, i, top);
        PaintLine(renderer, i, top);
    }
}

void ParchmentLog::PaintLine(ParchmentRenderer& renderer, int line_index, float top)
{
    if (!c_assert(line_index >= 0 && line_index < lines.Count())) return;
    const TextLine line = lines.Line(line_index);
    if (!c_assert(line.start >= 0 && line.length >= 0 && line.start + line.length <= text_length)) return;
    int length = line.length;
    if (length > 0 && text[line.start + length - 1] == L'\r') length--;
    const float baseline = renderer.BaselineInLine(FontId::ui, top, line_height);
    const float left = static_cast<float>(layout.text_area.left);
    renderer.DrawText({ { text + line.start, length, FontId::ui }, LOG_TEXT_COLOUR, left,
                        static_cast<float>(size.width), baseline, TextAlign::left });
}

void ParchmentLog::PaintIcon(ParchmentRenderer& renderer, int line_index, float top)
{
    if (icons.labels == nullptr) return;
    if (!c_assert(line_index >= 0 && line_index < lines.Count())) return;
    const TextLine line = lines.Line(line_index);
    if (!c_assert(line.start >= 0 && line.start <= text_length)) return;
    if (line.start > 0 && text[line.start - 1] != L'\n') return;
    const int cell = icons.labels->CellOfLeadingLabel(text + line.start, text_length - line.start);
    if (cell == IconLabels::NO_ICON) return;
    const float left = static_cast<float>(layout.text_area.left - IconColumn());
    const float icon_top = top + (line_height - static_cast<float>(ICON_SIZE)) / 2.0f;
    const float side = static_cast<float>(ICON_SIZE);
    renderer.DrawAtlasCell({ icons.atlas, cell, icons.columns, { icons.cell_size, icons.cell_size },
                             { left, icon_top, left + side, icon_top + side } });
}

void ParchmentLog::PaintScrollbar(ParchmentRenderer& renderer)
{
    if (!c_assert(layout.has_scrollbar) || !c_assert(TrackLength() >= SCROLLBAR_KNOB_SIZE)) return;
    const float left = static_cast<float>(size.width - BORDER - SCROLLBAR_WIDTH);
    const FloatRect bar = { left, static_cast<float>(BORDER), left + static_cast<float>(SCROLLBAR_WIDTH),
                            static_cast<float>(size.height - BORDER) };
    const int knob_offset = KnobOffset({ layout.content_height, ViewportHeight(size), scroll_top }, TrackLength());
    DrawScrollbar(renderer, { bar, knob_offset });
}

void ParchmentLog::OnButtonDown(LPARAM lParam)
{
    if (!c_assert(window != NULL)) return;
    const PixelPoint point = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
    (void)SetFocus(window);
    (void)SetCapture(window);
    if (layout.has_scrollbar && point.x >= size.width - BORDER - SCROLLBAR_WIDTH)
    {
        PressScrollbar(point);
        return;
    }
    drag = Drag::selecting;
    selection_anchor = CharacterAt(point);
    selection_caret = selection_anchor;
    Redraw();
}

void ParchmentLog::PressScrollbar(PixelPoint point)
{
    if (!c_assert(layout.has_scrollbar) || !c_assert(point.x >= size.width - BORDER - SCROLLBAR_WIDTH)) return;
    const int knob_offset = KnobOffset({ layout.content_height, ViewportHeight(size), scroll_top }, TrackLength());
    pressed_part = HitTestScrollbar({ point.y - BORDER, ViewportHeight(size), knob_offset });
    pressed_y = point.y;
    if (pressed_part == ScrollbarPart::knob)
    {
        drag = Drag::knob;
        drag_start_y = point.y;
        drag_start_scroll = scroll_top;
        return;
    }
    drag = Drag::scrolling;
    if (StepScrollbar()) (void)SetTimer(window, REPEAT_TIMER, FIRST_REPEAT_DELAY_MS, nullptr);
}

void ParchmentLog::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
    if (!c_assert(window != NULL && parent != NULL)) return;
    const PixelPoint point = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
    POINT in_parent = { point.x, point.y };
    (void)MapWindowPoints(window, parent, &in_parent, 1);
    (void)SendMessage(parent, WM_MOUSEMOVE, wParam, MAKELPARAM(in_parent.x, in_parent.y));
    if (drag == Drag::scrolling) pressed_y = point.y;
    if (drag == Drag::knob)
    {
        const int free_track = TrackLength() - SCROLLBAR_KNOB_SIZE;
        (void)ScrollBy(ScrollTopForKnobDrag({ drag_start_scroll, point.y - drag_start_y, MaxScroll(), free_track }) -
                       scroll_top);
    }
    if (drag != Drag::selecting) return;
    const int caret = CharacterAt(point);
    if (caret == selection_caret) return;
    selection_caret = caret;
    Redraw();
}

void ParchmentLog::OnButtonUp()
{
    if (!c_assert(window != NULL)) return;
    if (GetCapture() == window) (void)ReleaseCapture();
}

void ParchmentLog::OnCaptureLost()
{
    drag = Drag::none;
    if (!c_assert(window != NULL)) return;
    (void)KillTimer(window, REPEAT_TIMER);
}

void ParchmentLog::OnTimer()
{
    if (!c_assert(window != NULL)) return;
    if (drag != Drag::scrolling || !StepScrollbar())
    {
        (void)KillTimer(window, REPEAT_TIMER);
        return;
    }
    (void)SetTimer(window, REPEAT_TIMER, REPEAT_DELAY_MS, nullptr);
}

bool ParchmentLog::StepScrollbar()
{
    if (!c_assert(drag == Drag::scrolling) || !c_assert(layout.has_scrollbar)) return false;
    const int knob_offset = KnobOffset({ layout.content_height, ViewportHeight(size), scroll_top }, TrackLength());
    const ScrollbarPart part_under_pointer =
        HitTestScrollbar({ pressed_y - BORDER, ViewportHeight(size), knob_offset });
    const bool on_track =
        pressed_part == ScrollbarPart::track_above_knob || pressed_part == ScrollbarPart::track_below_knob;
    if (on_track && part_under_pointer != pressed_part) return false;
    return ScrollBy(ScrollStep(pressed_part));
}

int ParchmentLog::ScrollStep(ScrollbarPart part) const
{
    if (!c_assert(part != ScrollbarPart::knob)) return 0;
    const int step = LineStep();
    const int page = std::max(ViewportHeight(size) - step, step);
    if (part == ScrollbarPart::up_button) return -step;
    if (part == ScrollbarPart::down_button) return step;
    if (part == ScrollbarPart::track_above_knob) return -page;
    if (part == ScrollbarPart::track_below_knob) return page;
    return 0;
}

int ParchmentLog::LineStep() const
{
    const int step = static_cast<int>(line_height);
    if (!c_assert(step > 0)) return 1;
    return step;
}

int ParchmentLog::IconColumn() const { return icons.labels != nullptr ? ICON_SIZE + ICON_GAP : 0; }

bool ParchmentLog::ScrollBy(int distance)
{
    if (!c_assert(scroll_top >= 0)) return false;
    const int scrolled = ClampScrollTop({ layout.content_height, ViewportHeight(size), scroll_top + distance });
    if (scrolled == scroll_top) return false;
    scroll_top = scrolled;
    Redraw();
    return true;
}

int ParchmentLog::TrackLength() const { return ViewportHeight(size) - 2 * SCROLLBAR_BUTTON_LENGTH; }

int ParchmentLog::MaxScroll() const { return MaxScrollTop({ layout.content_height, ViewportHeight(size), 0 }); }

void ParchmentLog::OnWheel(WPARAM wParam)
{
    UINT lines_per_notch = 3;
    (void)SystemParametersInfoW(SPI_GETWHEELSCROLLLINES, 0, &lines_per_notch, 0);
    const int notches = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
    (void)ScrollBy(-notches * static_cast<int>(lines_per_notch) * LineStep());
}

void ParchmentLog::OnKey(WPARAM key)
{
    const bool control_held = GetKeyState(VK_CONTROL) < 0;
    if (control_held && key == 'C') CopySelection();
    if (control_held && key == 'A') SelectAll();
    const int page = ScrollStep(ScrollbarPart::track_below_knob);
    if (key == VK_UP) (void)ScrollBy(-LineStep());
    if (key == VK_DOWN) (void)ScrollBy(LineStep());
    if (key == VK_PRIOR) (void)ScrollBy(-page);
    if (key == VK_NEXT) (void)ScrollBy(page);
    if (key == VK_HOME) (void)ScrollBy(-MaxScroll());
    if (key == VK_END) (void)ScrollBy(MaxScroll());
}

LRESULT ParchmentLog::OnSetCursor(WPARAM wParam, LPARAM lParam)
{
    const HCURSOR cursor = context.renderer != nullptr ? context.renderer->Cursor(CursorId::arrow) : NULL;
    if (LOWORD(lParam) != HTCLIENT || cursor == NULL) return DefWindowProc(window, WM_SETCURSOR, wParam, lParam);
    (void)SetCursor(cursor);
    return TRUE;
}

void ParchmentLog::Redraw()
{
    if (window == NULL) return;
    (void)InvalidateRect(window, nullptr, FALSE);
}
