#pragma once

#include "ParchmentControl.h"

struct ScrollbarDraw
{
    FloatRect bar;
    int knob_offset;
};

void DrawScrollbar(ParchmentRenderer& renderer, const ScrollbarDraw& draw);

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
