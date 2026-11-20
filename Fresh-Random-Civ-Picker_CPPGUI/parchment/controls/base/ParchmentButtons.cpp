#include "ParchmentButtons.h"

#include "Assertions.h"

namespace
{
    // .btn { padding: 0 6px }, and while pressed { padding: 0 2px 2px 4px }: the face moves with the pressed image.
    constexpr float TEXT_BUTTON_PADDING = 6.0f;
    constexpr float PRESSED_PADDING_LEFT = 4.0f;
    constexpr float PRESSED_PADDING_RIGHT = 2.0f;
    constexpr float PRESSED_PADDING_BOTTOM = 2.0f;
    constexpr float BUTTON_FOCUS_INSET = 4.0f;
    // The share buttons' 6 px nine-slice frame; a pressed button's icon moves 1 px right and up.
    constexpr float ICON_BUTTON_BORDER = 6.0f;
    constexpr float PRESSED_ICON_SHIFT = 1.0f;
    // .dlg-x { padding: 6.5px 14.5px }: the browser snaps the 17 px image to (15, 7).
    constexpr float CLOSE_IMAGE_LEFT = 15.0f;
    constexpr float CLOSE_IMAGE_TOP = 7.0f;
    // kbd.key { border: 1px #b4b4b4; border-bottom-width: 2px; border-radius: 3px; padding: 0 4px }
    constexpr float KEY_RADIUS = 3.0f;
    constexpr float KEY_BORDER = 1.0f;
    constexpr float KEY_BOTTOM_BORDER = 2.0f;
    // A key button's cap leaves room around it for the dotted focus outline.
    constexpr float KEY_FOCUS_MARGIN = 2.0f;
    const wchar_t* const LISTENING_TEXT = L"\u2026";

    PixelRect WholeImage(PixelSize size)
    {
        if (!c_assert(size.width >= 0 && size.height >= 0)) return { 0, 0, 0, 0 };
        return { 0, 0, size.width, size.height };
    }

    PixelRect CellOf(int index, int cell_size)
    {
        if (!c_assert(index >= 0) || !c_assert(cell_size > 0)) return { 0, 0, 0, 0 };
        return { index * cell_size, 0, (index + 1) * cell_size, cell_size };
    }

    bool IsHeld(int key) { return GetKeyState(key) < 0; }

    // A modifier pressed on its own is held for the key that follows.
    bool IsModifierKey(WPARAM key)
    {
        const WPARAM modifier_keys[] = { VK_SHIFT, VK_CONTROL, VK_MENU, VK_LSHIFT, VK_RSHIFT, VK_LCONTROL,
                                         VK_RCONTROL, VK_LMENU, VK_RMENU, VK_LWIN, VK_RWIN };
        for (WPARAM modifier : modifier_keys)
        {
            if (key == modifier) return true;
        }
        return false;
    }
}

void TextButton::Paint(ParchmentRenderer& renderer, const FloatRect& bounds)
{
    if (!c_assert(HasArea(bounds))) return;
    const Look look = CurrentLook();
    const ImageId image = look.pressed ? ImageId::text_button_pressed : ImageId::text_button;
    renderer.DrawImage(
        { image, WholeImage(renderer.ImageSize(image)), bounds, 1.0f, ParchmentRenderer::Smoothing::smooth });

    FloatRect content = { bounds.left + TEXT_BUTTON_PADDING, bounds.top, bounds.right - TEXT_BUTTON_PADDING,
                          bounds.bottom };
    if (look.pressed)
        content = { bounds.left + PRESSED_PADDING_LEFT, bounds.top, bounds.right - PRESSED_PADDING_RIGHT,
                    bounds.bottom - PRESSED_PADDING_BOTTOM };
    const float baseline = renderer.BaselineCentred(FontId::button, content.top, content.bottom - content.top);
    const Colour colour = look.hot ? BUTTON_TEXT_HOVER_COLOUR : BUTTON_TEXT_COLOUR;
    renderer.DrawText({ Text(FontId::button), colour, content.left, content.right, baseline, TextAlign::centre });
    if (!look.focused) return;
    renderer.DrawDottedFrame({ bounds.left + BUTTON_FOCUS_INSET, bounds.top + BUTTON_FOCUS_INSET,
                               bounds.right - BUTTON_FOCUS_INSET, bounds.bottom - BUTTON_FOCUS_INSET },
                             BUTTON_FOCUS_COLOUR);
}

void AtlasButton::SetLooks(const Looks& button_looks)
{
    if (!c_assert(button_looks.cell_size > 0)) return;
    if (!c_assert(static_cast<int>(button_looks.image) < IMAGE_COUNT)) return;
    looks = button_looks;
    Redraw();
}

void AtlasButton::Paint(ParchmentRenderer& renderer, const FloatRect& bounds)
{
    if (!c_assert(HasArea(bounds)) || !c_assert(looks.cell_size > 0)) return;
    const Look look = CurrentLook();
    const int pressed_look = looks.has_hover_look ? 2 : 1;
    int shown_look = 0;
    if (look.pressed) shown_look = pressed_look;
    else if (look.hot && looks.has_hover_look) shown_look = 1;
    renderer.DrawImage({ looks.image, CellOf(shown_look, looks.cell_size), bounds, 1.0f,
                         ParchmentRenderer::Smoothing::smooth });
}

void FramedIconButton::SetIcon(const Icon& button_icon)
{
    if (!c_assert(button_icon.size > 0)) return;
    if (!c_assert(static_cast<int>(button_icon.image) < IMAGE_COUNT)) return;
    icon = button_icon;
    Redraw();
}

void FramedIconButton::Paint(ParchmentRenderer& renderer, const FloatRect& bounds)
{
    if (!c_assert(HasArea(bounds))) return;
    const Look look = CurrentLook();
    const NineSliceId frame = look.pressed ? NineSliceId::icon_button_frame_pressed : NineSliceId::icon_button_frame;
    renderer.DrawNineSlice({ frame, bounds, ICON_BUTTON_BORDER, TileRule::repeat, TileRule::repeat, true });
    if (!c_assert(icon.size > 0)) return;

    const float shift = look.pressed ? PRESSED_ICON_SHIFT : 0.0f;
    const float size = static_cast<float>(icon.size);
    const float left = (bounds.left + bounds.right - size) / 2.0f + shift;
    const float top = (bounds.top + bounds.bottom - size) / 2.0f - shift;
    renderer.DrawImage({ icon.image, WholeImage(renderer.ImageSize(icon.image)), { left, top, left + size, top + size },
                         1.0f, icon.smoothing });
}

void PictureButton::ShowPicture(const Picture& shown_picture)
{
    if (!c_assert(RectWidth(shown_picture.source) >= 0 && RectHeight(shown_picture.source) >= 0)) return;
    if (!c_assert(static_cast<int>(shown_picture.image) < IMAGE_COUNT)) return;
    picture = shown_picture;
    Redraw();
}

void PictureButton::Paint(ParchmentRenderer& renderer, const FloatRect& bounds)
{
    if (!c_assert(HasArea(bounds))) return;
    if (RectWidth(picture.source) <= 0 || RectHeight(picture.source) <= 0) return;
    renderer.DrawImage({ picture.image, picture.source, bounds, 1.0f, ParchmentRenderer::Smoothing::smooth });
}

void CloseButton::Paint(ParchmentRenderer& renderer, const FloatRect& bounds)
{
    if (!c_assert(HasArea(bounds))) return;
    const Look look = CurrentLook();
    const float size = static_cast<float>(CLOSE_BUTTON_IMAGE_SIZE);
    const float left = bounds.left + CLOSE_IMAGE_LEFT;
    const float top = bounds.top + CLOSE_IMAGE_TOP;
    renderer.DrawImage({ ImageId::close_button, CellOf(look.pressed ? 1 : 0, CLOSE_BUTTON_IMAGE_SIZE),
                         { left, top, left + size, top + size }, 1.0f, ParchmentRenderer::Smoothing::smooth });
}

CursorId CloseButton::CursorShape() const { return CursorId::arrow; }

void LinkButton::Paint(ParchmentRenderer& renderer, const FloatRect& bounds)
{
    if (!c_assert(HasArea(bounds))) return;
    const TextSpan span = Text(FontId::ui);
    const float baseline = renderer.BaselineInLine(FontId::ui, bounds.top, DIALOG_LINE_HEIGHT);
    renderer.DrawText({ span, LINK_COLOUR, bounds.left, bounds.right, baseline, TextAlign::left });
    const float underline_top = baseline + renderer.UnderlineOffset(FontId::ui);
    const float width = renderer.MeasureText(span);
    renderer.FillRect({ bounds.left, underline_top, bounds.left + width, underline_top + 1.0f }, LINK_COLOUR);
}

bool LinkButton::ShowsFocus() const { return false; }

void PaintKeyCap(ParchmentRenderer& renderer, const KeyCap& cap)
{
    if (!c_assert(HasArea(cap.box))) return;
    const FloatRect& box = cap.box;
    renderer.FillRoundedRect(box, KEY_RADIUS, KEY_BORDER_COLOUR);
    renderer.FillRoundedRect({ box.left + KEY_BORDER, box.top + KEY_BORDER, box.right - KEY_BORDER,
                               box.bottom - KEY_BOTTOM_BORDER },
                             KEY_RADIUS - KEY_BORDER, cap.face);
    const float baseline = renderer.BaselineInLine(FontId::key, box.top + KEY_BORDER, KEY_LINE_HEIGHT);
    renderer.DrawText({ cap.name, TOOLTIP_TEXT_COLOUR, box.left, box.right, baseline, TextAlign::centre });
}

// Gold while listening or under the pointer.
void KeyButton::Paint(ParchmentRenderer& renderer, const FloatRect& bounds)
{
    if (!c_assert(HasArea(bounds))) return;
    const Look look = CurrentLook();
    const FloatRect cap = { bounds.left + KEY_FOCUS_MARGIN, bounds.top + KEY_FOCUS_MARGIN,
                            bounds.right - KEY_FOCUS_MARGIN, bounds.bottom - KEY_FOCUS_MARGIN };
    const TextSpan name = listening ? TextSpan{ LISTENING_TEXT, 1, FontId::key } : Text(FontId::key);
    const bool gold = listening || look.hot || look.pressed;
    PaintKeyCap(renderer, { cap, name, gold ? HOVER_GOLD_COLOUR : KEY_COLOUR });
    if (look.focused && !listening) renderer.DrawDottedFrame(bounds, FOCUS_COLOUR);
}

void KeyButton::OnClicked()
{
    listening = !listening;
    Notify(listening ? KEY_LISTENING : KEY_KEPT);
}

bool KeyButton::NotifiesClicks() const { return false; }

// While listening it takes every key, Tab, Enter and Esc too, which a dialog would use itself.
LRESULT KeyButton::DialogCode() const
{
    return listening ? DLGC_WANTALLKEYS | DLGC_WANTCHARS : DLGC_BUTTON | DLGC_UNDEFPUSHBUTTON;
}

bool KeyButton::OnKeyPressed(WPARAM key)
{
    if (!listening) return false;
    if (!IsModifierKey(key)) Choose(key);
    return true;
}

// Alt and F10 arrive as system keys; while listening, they and the characters keys make go no further.
LRESULT KeyButton::OnOtherMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == GET_KEYSTROKE) return chosen;
    if (message == STOP_LISTENING)
    {
        listening = false;
        Redraw();
        return 0;
    }
    const bool key_message = message == WM_SYSKEYDOWN || message == WM_SYSKEYUP || message == WM_CHAR ||
                             message == WM_SYSCHAR;
    if (!listening || !key_message) return ParchmentControl::OnOtherMessage(message, wParam, lParam);
    if (message == WM_SYSKEYDOWN && !IsModifierKey(wParam)) Choose(wParam);
    return 0;
}

void KeyButton::OnFocusLost()
{
    if (!listening) return;
    listening = false;
    Notify(KEY_KEPT);
}

void KeyButton::Choose(WPARAM key)
{
    if (!c_assert(listening) || !c_assert(key > 0 && key <= 0xFE)) return;
    const UINT modifiers = (IsHeld(VK_CONTROL) ? MOD_CONTROL : 0) | (IsHeld(VK_MENU) ? MOD_ALT : 0) |
                           (IsHeld(VK_SHIFT) ? MOD_SHIFT : 0);
    chosen = MAKELRESULT(static_cast<WORD>(key), static_cast<WORD>(modifiers));
    listening = false;
    Redraw();
    Notify(KEY_CHOSEN);
}

// Posted, so the parent hears of it once this control's message is handled and can change the control then.
void KeyButton::Notify(WORD notification) const
{
    const int control_id = GetDlgCtrlID(Window());
    if (!c_assert(control_id != 0)) return;
    NotifyParentLater(WM_COMMAND, MAKEWPARAM(control_id, notification));
}
