#pragma once

#include "framework.h"
#include "ParchmentControl.h"

// The controls only dialogs have, drawn as the web version's (style.css).

// The slider (.slider): the off track with the on track over it up to the knob, and the knob image. It answers
// like a trackbar: TBM_SETRANGE, TBM_SETPOS and TBM_GETPOS, and WM_HSCROLL to the parent while it moves.
class SliderControl final : public ParchmentControl
{
public:
    SliderControl() = default;

private:
    void Paint(ParchmentRenderer& renderer, const FloatRect& bounds) override;
    CursorId CursorShape() const override;
    LRESULT DialogCode() const override;
    bool NotifiesClicks() const override;
    bool OnKeyPressed(WPARAM key) override;
    void OnPointerPressed(PixelPoint point) override;
    void OnPointerMoved(PixelPoint point) override;
    LRESULT OnOtherMessage(UINT message, WPARAM wParam, LPARAM lParam) override;
    LRESULT OnSetRange(LPARAM lParam);
    LRESULT OnSetPosition(LPARAM lParam);
    void MoveTo(int value);
    int Width() const;

    SliderRange range = { 0, 10, 0 };
};

// Dropdowns and text fields stay the standard COMBOBOX and EDIT controls, which already behave as the user
// expects; these functions give them the parchment look. A dropdown (owner-drawn, with strings) is dark brown with
// the wooden button on the right, and its list items turn gold under the mouse; a text field is a dark box drawn
// on the dialog's backdrop with white text, showing its placeholder while empty.
struct FieldLook
{
    ControlContext context;
    HFONT font;
    const wchar_t* placeholder;
};

// A disabled dropdown is translucent, so it paints over the piece of its dialog's backdrop under it.
struct DropdownLook
{
    ControlContext context;
    const PixelSurface* backdrop; // the dialog's client area at the backdrop's top-left corner
};

bool StyleDropdown(HWND dropdown, const DropdownLook& look);
bool StyleTextField(HWND field, const FieldLook& look);
void DrawDropdownItem(ParchmentRenderer& renderer, PixelSurface& back_buffer, const DRAWITEMSTRUCT& item);
void PaintTextFieldFrame(ParchmentRenderer& renderer, const PixelRect& box);
PixelRect TextFieldEditArea(const PixelRect& box);
PixelRect TextFieldFocusFrame(const PixelRect& edit_area);

constexpr int DROPDOWN_ITEM_HEIGHT = 22;
constexpr int DROPDOWN_SELECTION_HEIGHT = 16;
