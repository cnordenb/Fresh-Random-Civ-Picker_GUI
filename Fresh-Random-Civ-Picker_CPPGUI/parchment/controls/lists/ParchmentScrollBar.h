#pragma once

#include "ParchmentControl.h"

// The game's scrollbar as the logs draw it (.sb): an up button, the track and a down button, SCROLLBAR_WIDTH wide
// from the bar's left edge, and the knob knob_offset pixels down the track.
struct ScrollbarDraw
{
    FloatRect bar;
    int knob_offset;
};

void DrawScrollbar(ParchmentRenderer& renderer, const ScrollbarDraw& draw);

// A scrollbar control of the parchment theme (View) for a list of rows, of which it shows `shown` from `first` on.
// A button scrolls a row and the track a page, both repeating while held, and the knob drags. Each scroll made
// with it tells the parent the new first row, as a scrollbar control does: WM_VSCROLL with SB_THUMBPOSITION.
class ScrollBar final : public ParchmentControl
{
public:
    struct Rows
    {
        int total;
        int shown;
        int first;
    };

    void SetRows(const Rows& list_rows);
    int FirstRow() const;
    bool ScrollBy(int row_count);

private:
    void Paint(ParchmentRenderer& renderer, const FloatRect& bounds) override;
    CursorId CursorShape() const override;
    bool ShowsFocus() const override;
    bool NotifiesClicks() const override;
    void OnPointerPressed(PixelPoint point) override;
    void OnPointerMoved(PixelPoint point) override;
    LRESULT OnOtherMessage(UINT message, WPARAM wParam, LPARAM lParam) override;
    void OnRepeatTimer();
    bool Step();
    bool ScrollAndNotify(int row_count);
    int CurrentKnobOffset() const;
    int BarHeight() const;
    int TrackLength() const;

    Rows rows = { 0, 1, 0 };
    ScrollbarPart pressed_part = ScrollbarPart::none;
    int pressed_y = 0;
    int drag_start_y = 0;
    int drag_start_first = 0;
};
