#include "ParchmentScrollBar.h"

#include "Assertions.h"

namespace
{
    constexpr UINT_PTR REPEAT_TIMER = 1;
    constexpr UINT FIRST_REPEAT_DELAY_MS = 350; // a held button repeats as the logs' does
    constexpr UINT REPEAT_DELAY_MS = 50;
}

void DrawScrollbar(ParchmentRenderer& renderer, const ScrollbarDraw& draw)
{
    const FloatRect& bar = draw.bar;
    const float button = static_cast<float>(SCROLLBAR_BUTTON_LENGTH);
    const float knob = static_cast<float>(SCROLLBAR_KNOB_SIZE);
    if (!c_assert(bar.bottom - bar.top >= 2.0f * button + knob) || !c_assert(draw.knob_offset >= 0)) return;
    const float right = bar.left + static_cast<float>(SCROLLBAR_WIDTH);
    const ParchmentRenderer::Smoothing smooth = ParchmentRenderer::Smoothing::smooth;
    renderer.DrawImage({ ImageId::scrollbar, { 0, 0, SCROLLBAR_SLICE, SCROLLBAR_SLICE },
                         { bar.left, bar.top, right, bar.top + button }, 1.0f, smooth });
    renderer.DrawImage({ ImageId::scrollbar, { 0, SCROLLBAR_SLICE, SCROLLBAR_SLICE, 2 * SCROLLBAR_SLICE },
                         { bar.left, bar.top + button, right, bar.bottom - button }, 1.0f, smooth });
    renderer.DrawImage({ ImageId::scrollbar, { 0, 2 * SCROLLBAR_SLICE, SCROLLBAR_SLICE, 3 * SCROLLBAR_SLICE },
                         { bar.left, bar.bottom - button, right, bar.bottom }, 1.0f, smooth });
    const float knob_left = bar.left + static_cast<float>(SCROLLBAR_KNOB_INSET);
    const float knob_top = bar.top + button + static_cast<float>(draw.knob_offset);
    renderer.DrawImage({ ImageId::scrollbar_knob, { 0, 0, SCROLLBAR_KNOB_SIZE, SCROLLBAR_KNOB_SIZE },
                         { knob_left, knob_top, knob_left + knob, knob_top + knob }, 1.0f, smooth });
}

// The scrollbar counts in rows where the logs' counts in pixels: the scroll arithmetic is the same.
void ScrollBar::SetRows(const Rows& list_rows)
{
    if (!c_assert(list_rows.total >= 0 && list_rows.shown > 0)) return;
    rows = list_rows;
    rows.first = ClampScrollTop({ rows.total, rows.shown, rows.first });
    Redraw();
}

int ScrollBar::FirstRow() const { return rows.first; }

// Scrolls by rows, up for a negative count, and says whether the list moved. The caller, who asked for it, moves
// the list; no message goes to the parent.
bool ScrollBar::ScrollBy(int row_count)
{
    if (!c_assert(rows.shown > 0)) return false;
    const int first = ClampScrollTop({ rows.total, rows.shown, rows.first + row_count });
    if (first == rows.first) return false;
    rows.first = first;
    Redraw();
    return true;
}

void ScrollBar::Paint(ParchmentRenderer& renderer, const FloatRect& bounds)
{
    if (!c_assert(HasArea(bounds)) || !c_assert(TrackLength() >= SCROLLBAR_KNOB_SIZE)) return;
    DrawScrollbar(renderer, { bounds, CurrentKnobOffset() });
}

CursorId ScrollBar::CursorShape() const { return CursorId::arrow; }

bool ScrollBar::ShowsFocus() const { return false; }

bool ScrollBar::NotifiesClicks() const { return false; }

void ScrollBar::OnPointerPressed(PixelPoint point)
{
    if (!c_assert(Window() != NULL) || !c_assert(TrackLength() >= SCROLLBAR_KNOB_SIZE)) return;
    pressed_part = HitTestScrollbar({ point.y, BarHeight(), CurrentKnobOffset() });
    pressed_y = point.y;
    drag_start_y = point.y;
    drag_start_first = rows.first;
    if (pressed_part == ScrollbarPart::knob || pressed_part == ScrollbarPart::none) return;
    // Without the timer the press still scrolls once; holding it just doesn't repeat.
    if (Step()) (void)SetTimer(Window(), REPEAT_TIMER, FIRST_REPEAT_DELAY_MS, nullptr);
}

void ScrollBar::OnPointerMoved(PixelPoint point)
{
    if (!IsPressed()) return;
    pressed_y = point.y;
    if (pressed_part != ScrollbarPart::knob) return;
    const int free_track = TrackLength() - SCROLLBAR_KNOB_SIZE;
    if (!c_assert(free_track >= 0)) return;
    const int max_first = MaxScrollTop({ rows.total, rows.shown, 0 });
    const int first = ScrollTopForKnobDrag({ drag_start_first, point.y - drag_start_y, max_first, free_track });
    (void)ScrollAndNotify(first - rows.first);
}

LRESULT ScrollBar::OnOtherMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message != WM_TIMER || wParam != REPEAT_TIMER) return ParchmentControl::OnOtherMessage(message, wParam, lParam);
    OnRepeatTimer();
    return 0;
}

// A held button or track repeats until it is let go, or until the track has paged up to the pointer.
void ScrollBar::OnRepeatTimer()
{
    if (!c_assert(Window() != NULL)) return;
    if (!IsPressed() || !Step())
    {
        // The timer may already be gone; either way no repeat follows.
        (void)KillTimer(Window(), REPEAT_TIMER);
        return;
    }
    (void)SetTimer(Window(), REPEAT_TIMER, REPEAT_DELAY_MS, nullptr);
}

// One step of the pressed button or track; the track pages towards the pointer until the knob reaches it.
bool ScrollBar::Step()
{
    if (!c_assert(rows.shown > 0)) return false;
    const ScrollbarPart under_pointer = HitTestScrollbar({ pressed_y, BarHeight(), CurrentKnobOffset() });
    const int page = rows.shown > 1 ? rows.shown - 1 : 1;
    switch (pressed_part)
    {
        case ScrollbarPart::up_button: return ScrollAndNotify(-1);
        case ScrollbarPart::down_button: return ScrollAndNotify(1);
        case ScrollbarPart::track_above_knob: return under_pointer == pressed_part && ScrollAndNotify(-page);
        case ScrollbarPart::track_below_knob: return under_pointer == pressed_part && ScrollAndNotify(page);
        case ScrollbarPart::knob:
        case ScrollbarPart::none: return false;
    }
    return false;
}

bool ScrollBar::ScrollAndNotify(int row_count)
{
    if (!ScrollBy(row_count)) return false;
    NotifyParent(WM_VSCROLL, MAKEWPARAM(SB_THUMBPOSITION, static_cast<WORD>(rows.first)));
    return true;
}

int ScrollBar::CurrentKnobOffset() const
{
    return KnobOffset({ rows.total, rows.shown, rows.first }, TrackLength());
}

int ScrollBar::BarHeight() const
{
    if (!c_assert(Window() != NULL)) return 0;
    RECT client = {};
    if (!GetClientRect(Window(), &client)) return 0;
    return client.bottom;
}

int ScrollBar::TrackLength() const { return BarHeight() - 2 * SCROLLBAR_BUTTON_LENGTH; }
