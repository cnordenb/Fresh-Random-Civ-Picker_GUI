#include "ParchmentDialogControls.h"

#include <commctrl.h>

#include "Assertions.h"

#pragma comment(lib, "comctl32.lib")

namespace
{
    constexpr float DISABLED_OPACITY = 0.55f; // :disabled { opacity: .55 }
    constexpr int DROPDOWN_BUTTON_WIDTH = 16; // the wooden button fills the right 16 px
    constexpr float DROPDOWN_TEXT_LEFT = 5.0f; // border 1px + padding-left 4px
    constexpr float OPTION_PADDING_X = 6.0f; // option { padding: 3px 6px }
    constexpr float OPTION_PADDING_Y = 3.0f;
    constexpr int FIELD_BORDER = 7; // border-image: ... 15 fill / 7px stretch
    constexpr int FIELD_INSET_X = 6; // border 1px + padding 0 5px
    constexpr int FIELD_INSET_Y = 4; // a 14 px line centred in the 22 px box
    constexpr int FIELD_FOCUS_OFFSET = 2; // outline-offset 1px plus the 1 px outline
    constexpr COLORREF PLACEHOLDER_COLOUR = RGB(0x9a, 0x9a, 0x9a); // rgba(255, 255, 255, .45) over #474747
    constexpr UINT_PTR DROPDOWN_SUBCLASS = 1;
    constexpr UINT_PTR FIELD_SUBCLASS = 2;
    constexpr int MAX_ITEM_TEXT = 64;

    int ClampToRange(const SliderRange& range, int value)
    {
        if (!c_assert(range.maximum >= range.minimum)) return range.minimum;
        if (value < range.minimum) return range.minimum;
        return value > range.maximum ? range.maximum : value;
    }

    // A slider bar image over the target: its frame's ends keep their proportions, its middle stretches.
    void DrawBarImage(ParchmentRenderer& renderer, ImageId image, const FloatRect& target)
    {
        if (!c_assert(HasArea(target))) return;
        const float cap = static_cast<float>(SLIDER_BAR_IMAGE_CAP) * (target.bottom - target.top) /
                          static_cast<float>(SLIDER_BAR_IMAGE_HEIGHT);
        if (!c_assert(2.0f * cap < target.right - target.left)) return;
        const int middle_end = SLIDER_BAR_IMAGE_WIDTH - SLIDER_BAR_IMAGE_CAP;
        const ParchmentRenderer::Smoothing smooth = ParchmentRenderer::Smoothing::smooth;
        renderer.DrawImage({ image, { 0, 0, SLIDER_BAR_IMAGE_CAP, SLIDER_BAR_IMAGE_HEIGHT },
                             { target.left, target.top, target.left + cap, target.bottom }, 1.0f, smooth });
        renderer.DrawImage({ image, { SLIDER_BAR_IMAGE_CAP, 0, middle_end, SLIDER_BAR_IMAGE_HEIGHT },
                             { target.left + cap, target.top, target.right - cap, target.bottom }, 1.0f, smooth });
        renderer.DrawImage({ image, { middle_end, 0, SLIDER_BAR_IMAGE_WIDTH, SLIDER_BAR_IMAGE_HEIGHT },
                             { target.right - cap, target.top, target.right, target.bottom }, 1.0f, smooth });
    }

    // Paints a dropdown's closed box: dark brown with a 1 px border, the chosen text, and the wooden button, which
    // shows its pressed look while the list is open.
    void PaintDropdownBox(ParchmentRenderer& renderer, const FloatRect& bounds, const COMBOBOXINFO& info)
    {
        if (!c_assert(HasArea(bounds)) || !c_assert(info.cbSize == sizeof(COMBOBOXINFO))) return;
        renderer.FillRect(bounds, MENU_BORDER_COLOUR);
        renderer.FillRect({ bounds.left + 1.0f, bounds.top + 1.0f, bounds.right - 1.0f, bounds.bottom - 1.0f },
                          MENU_COLOUR);
        wchar_t text[MAX_ITEM_TEXT] = L"";
        const LRESULT selected = SendMessage(info.hwndList, LB_GETCURSEL, 0, 0);
        const WPARAM item = static_cast<WPARAM>(selected);
        const LRESULT length = selected >= 0 ? SendMessage(info.hwndList, LB_GETTEXTLEN, item, 0) : 0;
        if (selected >= 0 && length > 0 && length < MAX_ITEM_TEXT)
            (void)SendMessage(info.hwndList, LB_GETTEXT, item, reinterpret_cast<LPARAM>(text));
        const float box_height = bounds.bottom - bounds.top - 2.0f;
        const float baseline = renderer.BaselineCentred(FontId::ui, bounds.top + 1.0f, box_height);
        renderer.DrawText({ { text, static_cast<int>(wcslen(text)), FontId::ui }, MENU_TEXT_COLOUR,
                            bounds.left + DROPDOWN_TEXT_LEFT, bounds.right - 1.0f - DROPDOWN_BUTTON_WIDTH, baseline,
                            TextAlign::left });
        const bool open = (info.stateButton & STATE_SYSTEM_PRESSED) != 0;
        const PixelRect cell = { open ? DROPDOWN_BUTTON_CELL_WIDTH : 0, 0,
                                 open ? 2 * DROPDOWN_BUTTON_CELL_WIDTH : DROPDOWN_BUTTON_CELL_WIDTH,
                                 DROPDOWN_BUTTON_CELL_HEIGHT };
        renderer.DrawImage({ ImageId::dropdown_button, cell,
                             { bounds.right - 1.0f - DROPDOWN_BUTTON_WIDTH, bounds.top + 1.0f, bounds.right - 1.0f,
                               bounds.bottom - 1.0f },
                             1.0f, ParchmentRenderer::Smoothing::smooth });
    }

    // Puts the piece of the dialog's backdrop under the dropdown into the back buffer, where a disabled dropdown's
    // translucent box shows it.
    bool CopyBackdropUnder(HWND dropdown, const DropdownLook& look, PixelSize size)
    {
        if (!c_assert(look.backdrop != nullptr) || !c_assert(look.context.back_buffer != nullptr)) return false;
        POINT place = { 0, 0 };
        if (!ClientToScreen(dropdown, &place) || !ScreenToClient(GetParent(dropdown), &place)) return false;
        if (!c_assert(look.backdrop->Contains({ place.x, place.y, place.x + size.width, place.y + size.height })))
            return false;
        return c_assert(BitBlt(look.context.back_buffer->Dc(), 0, 0, size.width, size.height, look.backdrop->Dc(),
                               place.x, place.y, SRCCOPY) != FALSE);
    }

    void PaintDropdown(HWND dropdown, const DropdownLook& look)
    {
        if (!c_assert(dropdown != NULL)) return;
        PAINTSTRUCT paint = {};
        const HDC target = BeginPaint(dropdown, &paint);
        RECT client = {};
        COMBOBOXINFO info = {};
        info.cbSize = sizeof(info);
        PixelSurface* buffer = look.context.back_buffer;
        ParchmentRenderer* renderer = look.context.renderer;
        const bool ready = target != NULL && buffer != nullptr && renderer != nullptr &&
                           GetClientRect(dropdown, &client) && GetComboBoxInfo(dropdown, &info);
        const PixelSize size = { client.right, client.bottom };
        if (ready && buffer->Contains({ 0, 0, size.width, size.height }) && CopyBackdropUnder(dropdown, look, size) &&
            renderer->BeginFrame({ buffer->Dc(), size, ParchmentRenderer::SurfaceAlpha::opaque }))
        {
            const bool enabled = IsWindowEnabled(dropdown) == TRUE;
            if (!enabled) renderer->PushOpacity(DISABLED_OPACITY);
            PaintDropdownBox(*renderer, ToFloatRect({ 0, 0, size.width, size.height }), info);
            if (!enabled) renderer->PopOpacity();
            if (renderer->EndFrame())
                (void)BitBlt(target, 0, 0, size.width, size.height, buffer->Dc(), 0, 0, SRCCOPY);
        }
        (void)EndPaint(dropdown, &paint);
    }

    // Over its box the dropdown shows the game's hand cursor; elsewhere the combobox decides.
    LRESULT OnDropdownSetCursor(const MSG& message, const ControlContext& context)
    {
        if (!c_assert(message.hwnd != NULL)) return FALSE;
        if (context.renderer == nullptr || LOWORD(message.lParam) != HTCLIENT)
            return DefSubclassProc(message.hwnd, message.message, message.wParam, message.lParam);
        (void)SetCursor(context.renderer->Cursor(CursorId::hand));
        return TRUE;
    }

    // Subclass procedure of a dropdown. comctl32 subclassing only works through a function pointer (CLAUDE2.md
    // I2); the dropdown's look travels as the subclass data.
    LRESULT CALLBACK DropdownProc(HWND dropdown, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR,
                                  DWORD_PTR data) noexcept
    {
        const DropdownLook* look = reinterpret_cast<const DropdownLook*>(data);
        if (look == nullptr) return DefSubclassProc(dropdown, message, wParam, lParam);
        switch (message)
        {
            case WM_PAINT: PaintDropdown(dropdown, *look); return 0;
            case WM_ERASEBKGND: return 1;
            case WM_SETCURSOR: return OnDropdownSetCursor({ dropdown, message, wParam, lParam }, look->context);
            case WM_NCDESTROY: (void)RemoveWindowSubclass(dropdown, DropdownProc, DROPDOWN_SUBCLASS); break;
            default: break;
        }
        return DefSubclassProc(dropdown, message, wParam, lParam);
    }

    // The placeholder of an empty text field, drawn over the field after it painted itself.
    void PaintPlaceholder(HWND field, const FieldLook& look)
    {
        if (!c_assert(field != NULL) || !c_assert(look.font != NULL)) return;
        if (look.placeholder == nullptr || GetWindowTextLengthW(field) > 0) return;
        const HDC dc = GetDC(field);
        if (dc == NULL) return;
        const HGDIOBJ previous_font = SelectObject(dc, look.font);
        (void)SetTextColor(dc, PLACEHOLDER_COLOUR);
        (void)SetBkMode(dc, TRANSPARENT);
        // A placeholder that fails to draw only leaves the field blank; the field still works.
        (void)TextOutW(dc, 0, 0, look.placeholder, static_cast<int>(wcslen(look.placeholder)));
        (void)SelectObject(dc, previous_font);
        (void)ReleaseDC(field, dc);
    }

    // Subclass procedure of a text field (CLAUDE2.md I2, as above); its look travels as the subclass data.
    LRESULT CALLBACK TextFieldProc(HWND field, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR,
                                   DWORD_PTR data) noexcept
    {
        const FieldLook* look = reinterpret_cast<const FieldLook*>(data);
        if (message == WM_NCDESTROY) (void)RemoveWindowSubclass(field, TextFieldProc, FIELD_SUBCLASS);
        const LRESULT result = DefSubclassProc(field, message, wParam, lParam);
        if (look != nullptr && message == WM_PAINT) PaintPlaceholder(field, *look);
        return result;
    }
}

// --- SliderControl ---

// The bar with its fill up to the handle, the arrows over the bar's ends, and the handle on top.
void SliderControl::Paint(ParchmentRenderer& renderer, const FloatRect& bounds)
{
    if (!c_assert(HasArea(bounds)) || !c_assert(range.maximum >= range.minimum)) return;
    const Look look = CurrentLook();
    const SliderTrack track = Track();
    const SliderLayout& layout = track.layout;
    const float handle_centre = SliderHandleCentre(track);
    const float middle = (bounds.top + bounds.bottom) / 2.0f;
    const float half_handle = layout.handle_size / 2.0f;
    const ParchmentRenderer::Smoothing smooth = ParchmentRenderer::Smoothing::smooth;
    if (!look.enabled) renderer.PushOpacity(DISABLED_OPACITY);
    PaintBar(renderer, track);
    const PixelRect arrow_source = { 0, 0, SLIDER_ARROW_IMAGE_WIDTH, SLIDER_ARROW_IMAGE_HEIGHT };
    renderer.DrawImage({ ArrowImage(SliderPart::left_arrow), arrow_source, layout.left_arrow, 1.0f, smooth });
    renderer.DrawImage({ ArrowImage(SliderPart::right_arrow), arrow_source, layout.right_arrow, 1.0f, smooth });
    renderer.DrawImage({ HandleImage(), { 0, 0, SLIDER_HANDLE_IMAGE, SLIDER_HANDLE_IMAGE },
                         { handle_centre - half_handle, middle - half_handle, handle_centre + half_handle,
                           middle + half_handle },
                         1.0f, smooth });
    if (!look.enabled) renderer.PopOpacity();
    if (look.focused) renderer.DrawDottedFrame(bounds, FOCUS_COLOUR);
}

void SliderControl::PaintBar(ParchmentRenderer& renderer, const SliderTrack& track)
{
    const FloatRect& bar = track.layout.bar;
    if (!c_assert(HasArea(bar))) return;
    DrawBarImage(renderer, ImageId::slider_bar, bar);
    renderer.PushClip({ bar.left, bar.top, SliderHandleCentre(track), bar.bottom });
    DrawBarImage(renderer, ImageId::slider_progress, bar);
    renderer.PopClip();
}

// An arrow is active while it is held and lit while the mouse is over it, as long as nothing else is held.
ImageId SliderControl::ArrowImage(SliderPart arrow) const
{
    const bool left = arrow == SliderPart::left_arrow;
    if (!c_assert(left || arrow == SliderPart::right_arrow)) return ImageId::slider_left_arrow;
    if (IsPressed() && pressed_part == arrow)
        return left ? ImageId::slider_left_arrow_active : ImageId::slider_right_arrow_active;
    if (!IsPressed() && hovered_part == arrow)
        return left ? ImageId::slider_left_arrow_hover : ImageId::slider_right_arrow_hover;
    return left ? ImageId::slider_left_arrow : ImageId::slider_right_arrow;
}

ImageId SliderControl::HandleImage() const
{
    if (IsPressed() && pressed_part == SliderPart::handle) return ImageId::slider_handle_active;
    if (!IsPressed() && hovered_part == SliderPart::handle) return ImageId::slider_handle_hover;
    return ImageId::slider_handle;
}

CursorId SliderControl::CursorShape() const { return CursorId::arrow; }

LRESULT SliderControl::DialogCode() const { return DLGC_WANTARROWS; }

bool SliderControl::NotifiesClicks() const { return false; }

bool SliderControl::OnKeyPressed(WPARAM key)
{
    if (!c_assert(range.maximum >= range.minimum)) return false;
    switch (key)
    {
        case VK_LEFT:
        case VK_DOWN: MoveTo(range.value - 1); return true;
        case VK_RIGHT:
        case VK_UP: MoveTo(range.value + 1); return true;
        case VK_HOME: MoveTo(range.minimum); return true;
        case VK_END: MoveTo(range.maximum); return true;
        default: return false;
    }
}

// An arrow steps once; a press on the bar moves the handle there and drags it from there on, as one on the handle
// drags it from where it is.
void SliderControl::OnPointerPressed(PixelPoint point)
{
    const SliderTrack track = Track();
    pressed_part = HitTestSlider(track, point);
    if (pressed_part == SliderPart::left_arrow) MoveTo(range.value - 1);
    if (pressed_part == SliderPart::right_arrow) MoveTo(range.value + 1);
    if (pressed_part != SliderPart::bar) return;
    pressed_part = SliderPart::handle;
    MoveTo(SliderValueAt(track, static_cast<float>(point.x)));
}

void SliderControl::OnPointerMoved(PixelPoint point)
{
    const SliderTrack track = Track();
    if (IsPressed())
    {
        if (pressed_part == SliderPart::handle) MoveTo(SliderValueAt(track, static_cast<float>(point.x)));
        return;
    }
    const SliderPart part = HitTestSlider(track, point);
    if (part == hovered_part) return;
    hovered_part = part;
    Redraw();
}

void SliderControl::OnPointerLeft() { hovered_part = SliderPart::none; }

// The trackbar messages the Options dialog uses.
LRESULT SliderControl::OnOtherMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
    if (!c_assert(Window() != NULL)) return 0;
    switch (message)
    {
        case TBM_SETRANGE: return OnSetRange(lParam);
        case TBM_SETPOS: return OnSetPosition(lParam);
        case TBM_GETPOS: return range.value;
        default: return ParchmentControl::OnOtherMessage(message, wParam, lParam);
    }
}

// TBM_SETRANGE: the low word of lParam is the minimum, the high word the maximum.
LRESULT SliderControl::OnSetRange(LPARAM lParam)
{
    const int minimum = static_cast<short>(LOWORD(lParam));
    const int maximum = static_cast<short>(HIWORD(lParam));
    if (!c_assert(maximum >= minimum)) return 0;
    range.minimum = minimum;
    range.maximum = maximum;
    range.value = ClampToRange(range, range.value);
    Redraw();
    return 0;
}

LRESULT SliderControl::OnSetPosition(LPARAM lParam)
{
    if (!c_assert(range.maximum >= range.minimum)) return 0;
    range.value = ClampToRange(range, static_cast<int>(lParam));
    Redraw();
    return 0;
}

void SliderControl::MoveTo(int value)
{
    const int clamped = ClampToRange(range, value);
    if (clamped == range.value || !c_assert(range.maximum >= range.minimum)) return;
    range.value = clamped;
    Redraw();
    NotifyParent(WM_HSCROLL, MAKEWPARAM(TB_THUMBTRACK, static_cast<WORD>(clamped)));
}

// The slider's layout at its window's size, with its range.
SliderTrack SliderControl::Track() const
{
    RECT client = { 0, 0, 1, 1 };
    if (!c_assert(Window() != NULL) || !GetClientRect(Window(), &client) || client.right <= 0 || client.bottom <= 0)
        client = { 0, 0, 1, 1 };
    return { LayOutSlider({ client.right, client.bottom }), range };
}

// --- Dropdowns and text fields ---

bool StyleDropdown(HWND dropdown, const DropdownLook& look)
{
    if (!c_assert(dropdown != NULL) || !c_assert(look.context.renderer != nullptr)) return false;
    if (!c_assert(look.backdrop != nullptr)) return false;
    return c_assert(SetWindowSubclass(dropdown, DropdownProc, DROPDOWN_SUBCLASS,
                                      reinterpret_cast<DWORD_PTR>(&look)) != FALSE);
}

bool StyleTextField(HWND field, const FieldLook& look)
{
    if (!c_assert(field != NULL) || !c_assert(look.font != NULL)) return false;
    (void)SendMessage(field, WM_SETFONT, reinterpret_cast<WPARAM>(look.font), FALSE);
    (void)SendMessage(field, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, 0);
    const DWORD_PTR subclass_data = reinterpret_cast<DWORD_PTR>(&look);
    return c_assert(SetWindowSubclass(field, TextFieldProc, FIELD_SUBCLASS, subclass_data) != FALSE);
}

// One item of an open dropdown list: gold with black text under the mouse, otherwise brown with cream text.
void DrawDropdownItem(ParchmentRenderer& renderer, PixelSurface& back_buffer, const DRAWITEMSTRUCT& item)
{
    if (!c_assert(item.hwndItem != NULL && item.hDC != NULL)) return;
    const PixelSize size = { item.rcItem.right - item.rcItem.left, item.rcItem.bottom - item.rcItem.top };
    if (item.itemID == static_cast<UINT>(-1) || !back_buffer.Contains({ 0, 0, size.width, size.height })) return;
    wchar_t text[MAX_ITEM_TEXT] = L"";
    const LRESULT length = SendMessage(item.hwndItem, CB_GETLBTEXTLEN, item.itemID, 0);
    if (length > 0 && length < MAX_ITEM_TEXT)
        (void)SendMessage(item.hwndItem, CB_GETLBTEXT, item.itemID, reinterpret_cast<LPARAM>(text));
    if (!renderer.BeginFrame({ back_buffer.Dc(), size, ParchmentRenderer::SurfaceAlpha::opaque })) return;
    const bool selected = (item.itemState & ODS_SELECTED) != 0;
    renderer.FillRect(ToFloatRect({ 0, 0, size.width, size.height }), selected ? HOVER_GOLD_COLOUR : MENU_COLOUR);
    const float baseline = renderer.BaselineInLine(FontId::ui, OPTION_PADDING_Y, UI_LINE_HEIGHT);
    renderer.DrawText({ { text, static_cast<int>(wcslen(text)), FontId::ui }, selected ? TEXT_COLOUR : MENU_TEXT_COLOUR,
                        OPTION_PADDING_X, static_cast<float>(size.width), baseline, TextAlign::left });
    if (!renderer.EndFrame()) return;
    (void)BitBlt(item.hDC, item.rcItem.left, item.rcItem.top, size.width, size.height, back_buffer.Dc(), 0, 0, SRCCOPY);
}

// The dark box of a text field (a nine-slice drawn over the field's grey), drawn on the dialog's backdrop.
void PaintTextFieldFrame(ParchmentRenderer& renderer, const PixelRect& box)
{
    if (!c_assert(HasArea(box))) return;
    renderer.FillRect(ToFloatRect(box), FIELD_COLOUR);
    renderer.DrawNineSlice({ NineSliceId::text_field_frame, ToFloatRect(box), static_cast<float>(FIELD_BORDER),
                             TileRule::stretch, TileRule::stretch, true });
}

PixelRect TextFieldEditArea(const PixelRect& box)
{
    if (!c_assert(RectWidth(box) > 2 * FIELD_INSET_X && RectHeight(box) > 2 * FIELD_INSET_Y)) return box;
    return { box.left + FIELD_INSET_X, box.top + FIELD_INSET_Y, box.right - FIELD_INSET_X, box.bottom - FIELD_INSET_Y };
}

// The focus outline around a text field's box (outline: 1px dotted; outline-offset: 1px), from its edit area.
PixelRect TextFieldFocusFrame(const PixelRect& edit_area)
{
    if (!c_assert(HasArea(edit_area))) return edit_area;
    const int outset_x = FIELD_INSET_X + FIELD_FOCUS_OFFSET;
    const int outset_y = FIELD_INSET_Y + FIELD_FOCUS_OFFSET;
    return { edit_area.left - outset_x, edit_area.top - outset_y, edit_area.right + outset_x,
             edit_area.bottom + outset_y };
}
