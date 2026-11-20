#include "ParchmentDialog.h"

#include <commctrl.h>
#include <windowsx.h>
#include <cmath>
#include <cwchar>

#include "Assertions.h"

namespace
{
    constexpr int PANEL_PADDING = 13;
    constexpr int TITLE_HEIGHT = 30;
    constexpr int TITLE_LEFT = 10;
    constexpr float PANEL_BORDER = 38.0f;
    constexpr int LEGEND_LEFT = 11;
    constexpr float LEGEND_PADDING = 3.0f;

    int TextLength(const wchar_t* text) { return text != nullptr ? static_cast<int>(wcslen(text)) : 0; }

    ParchmentDialogs::Handled Unhandled() { return { false, FALSE }; }

    PixelRect AreaInParent(HWND child, HWND parent)
    {
        if (!c_assert(child != NULL && parent != NULL)) return { 0, 0, 0, 0 };
        RECT client = {};
        POINT origin = { 0, 0 };
        if (!c_assert(GetClientRect(child, &client) != FALSE)) return { 0, 0, 0, 0 };
        (void)MapWindowPoints(child, parent, &origin, 1);
        return { origin.x, origin.y, origin.x + client.right, origin.y + client.bottom };
    }

    bool IsTextField(HWND control)
    {
        if (!c_assert(control != NULL)) return false;
        wchar_t class_name[16] = L"";
        return GetClassNameW(control, class_name, 16) > 0 && _wcsicmp(class_name, WC_EDITW) == 0;
    }

    ParchmentDialogs::Handled HandledWith(HWND dialog, LRESULT result)
    {
        if (!c_assert(dialog != NULL)) return Unhandled();
        (void)SetWindowLongPtr(dialog, DWLP_MSGRESULT, result);
        return { true, TRUE };
    }
}

bool ParchmentDialogs::RenderPanel(Level& level)
{
    if (!c_assert(context.renderer != nullptr)) return false;
    if (!c_assert(level.size.width > 0 && level.size.height > 0)) return false;
    PixelSurface& canvas = level.shell.Canvas();
    const PixelPoint origin = level.shell.WindowOrigin();
    const PixelSize canvas_size = { level.size.width + 2 * origin.x, level.size.height + 2 * origin.y };
    ParchmentRenderer& renderer = *context.renderer;
    if (!renderer.BeginFrame({ canvas.Dc(), canvas_size, ParchmentRenderer::SurfaceAlpha::premultiplied }))
        return false;
    renderer.Clear();
    const FloatRect panel =
        ToFloatRect({ origin.x, origin.y, origin.x + level.size.width, origin.y + level.size.height });
    const float inlay = PANEL_BORDER / 2.0f;
    const PixelSize middle = renderer.ImageSize(ImageId::panel_inlay);
    renderer.DrawImage({ ImageId::panel_inlay, { 0, 0, middle.width, middle.height },
                         { panel.left + inlay, panel.top + inlay, panel.right - inlay, panel.bottom - inlay }, 1.0f,
                         ParchmentRenderer::Smoothing::smooth });
    renderer.DrawNineSlice({ NineSliceId::panel_frame, panel, PANEL_BORDER, TileRule::round, TileRule::round, false });
    return renderer.EndFrame();
}

bool ParchmentDialogs::Decorate(Level& level)
{
    if (!c_assert(context.renderer != nullptr) || !c_assert(level.layout != nullptr)) return false;
    ParchmentRenderer& renderer = *context.renderer;
    PixelSurface& backdrop = level.shell.Backdrop();
    if (!renderer.BeginFrame({ backdrop.Dc(), level.size, ParchmentRenderer::SurfaceAlpha::opaque })) return false;
    const DialogLayout& layout = *level.layout;
    const float title_top = PANEL_PADDING + (TITLE_HEIGHT - DIALOG_LINE_HEIGHT) / 2.0f;
    renderer.DrawText({ { layout.title, TextLength(layout.title), FontId::ui }, TEXT_COLOUR,
                        static_cast<float>(PANEL_PADDING + TITLE_LEFT), static_cast<float>(level.size.width),
                        renderer.BaselineInLine(FontId::ui, title_top, DIALOG_LINE_HEIGHT), TextAlign::left });
    for (int i = 0; i < layout.decoration_count; i++) DrawDecoration(level, i);
    for (int i = 0; i < layout.part_count; i++)
    {
        const PartKind kind = layout.parts[i].kind;
        if (kind == PartKind::text_field || kind == PartKind::readonly_field)
            PaintTextFieldFrame(renderer, layout.parts[i].bounds);
    }
    return renderer.EndFrame();
}

void ParchmentDialogs::DrawDecoration(Level& level, int index)
{
    if (!c_assert(level.layout != nullptr)) return;
    const DialogLayout& layout = *level.layout;
    if (!c_assert(index >= 0 && index < layout.decoration_count)) return;
    const DialogDecoration& decoration = layout.decorations[index];
    switch (decoration.kind)
    {
        case DecorationKind::text:
        case DecorationKind::bold_text:
        case DecorationKind::paragraph:
        case DecorationKind::hint: DrawParagraph(level, index); return;
        case DecorationKind::group: DrawGroup(decoration); return;
        case DecorationKind::key: DrawKey(decoration); return;
        default: (void)c_assert(false); return;
    }
}

void ParchmentDialogs::DrawParagraph(Level& level, int index)
{
    if (!c_assert(level.layout != nullptr) || !c_assert(context.renderer != nullptr)) return;
    const DialogLayout& layout = *level.layout;
    if (!c_assert(index >= 0 && index < layout.decoration_count)) return;
    const DialogDecoration& decoration = layout.decorations[index];
    const FontId font = decoration.kind == DecorationKind::bold_text ? FontId::ui_bold : FontId::ui;
    const TextSpan span = { decoration.text, TextLength(decoration.text), font };
    const float* advances = context.renderer->MeasureAdvances(span);
    if (!wrapped.Wrap({ span.text, advances, span.length, static_cast<float>(RectWidth(decoration.bounds)) })) return;
    for (int line = 0; line < wrapped.Count() && line < MAX_WRAPPED_LINES; line++)
    {
        const float top = static_cast<float>(decoration.bounds.top) + static_cast<float>(line) * DIALOG_LINE_HEIGHT;
        DrawLine({ wrapped.Line(line), decoration, top, advances, &level });
    }
}

void ParchmentDialogs::DrawLine(const LineDraw& draw)
{
    if (!c_assert(draw.level != nullptr) || !c_assert(draw.advances != nullptr)) return;
    Level& level = *draw.level;
    if (!c_assert(level.layout != nullptr)) return;
    const DialogLayout& layout = *level.layout;
    const DialogDecoration& decoration = draw.decoration;
    const FontId font = decoration.kind == DecorationKind::bold_text ? FontId::ui_bold : FontId::ui;
    const Colour colour = decoration.kind == DecorationKind::hint ? HINT_COLOUR : TEXT_COLOUR;
    const float baseline = context.renderer->BaselineInLine(font, draw.top, DIALOG_LINE_HEIGHT);
    int piece_start = draw.line.start;
    float x = static_cast<float>(decoration.bounds.left);
    float piece_x = x;
    const int line_end = draw.line.start + draw.line.length;
    for (int i = draw.line.start; i <= line_end && i <= ParchmentFonts::MAX_MEASURED_LENGTH; i++)
    {
        const int link = LinkStartingAt(layout, { &decoration, i });
        if (i < line_end && link < 0)
        {
            x += draw.advances[i];
            continue;
        }
        context.renderer->DrawText({ { decoration.text + piece_start, i - piece_start, font }, colour, piece_x,
                                     static_cast<float>(decoration.bounds.right), baseline, TextAlign::left });
        if (link < 0) break;
        const int link_length = TextLength(layout.links[link].text);
        float link_width = 0.0f;
        for (int j = i; j < i + link_length && j < line_end; j++) link_width += draw.advances[j];
        level.link_bounds[link] = { static_cast<int>(std::floor(x)), static_cast<int>(std::floor(draw.top)),
                                    static_cast<int>(std::ceil(x + link_width)),
                                    static_cast<int>(std::ceil(draw.top + DIALOG_LINE_HEIGHT)) };
        x += link_width;
        i += link_length - 1;
        piece_start = i + 1;
        piece_x = x;
    }
}

int ParchmentDialogs::LinkStartingAt(const DialogLayout& layout, const TextPosition& position) const
{
    if (!c_assert(position.decoration != nullptr) || !c_assert(position.index >= 0)) return -1;
    for (int i = 0; i < layout.link_count && i < MAX_LINKS; i++)
    {
        const DialogLink& link = layout.links[i];
        if (&layout.decorations[link.decoration] != position.decoration) continue;
        const wchar_t* found = wcsstr(position.decoration->text, link.text);
        if (found != nullptr && found - position.decoration->text == position.index) return i;
    }
    return -1;
}

void ParchmentDialogs::DrawGroup(const DialogDecoration& decoration)
{
    if (!c_assert(context.renderer != nullptr) || !c_assert(HasArea(decoration.bounds))) return;
    ParchmentRenderer& renderer = *context.renderer;
    const FloatRect frame = ToFloatRect(decoration.bounds);
    const TextSpan legend = { decoration.text, TextLength(decoration.text), FontId::ui };
    const float legend_left = frame.left + LEGEND_LEFT;
    const float legend_right = legend_left + renderer.MeasureText(legend) + 2.0f * LEGEND_PADDING;
    const float line = frame.top + std::floor(DIALOG_LINE_HEIGHT / 2.0f);
    renderer.FillRect({ frame.left, line, legend_left, line + 1.0f }, GROUP_FRAME_COLOUR);
    renderer.FillRect({ legend_right, line, frame.right, line + 1.0f }, GROUP_FRAME_COLOUR);
    renderer.FillRect({ frame.left, line + 1.0f, frame.left + 1.0f, frame.bottom }, GROUP_FRAME_COLOUR);
    renderer.FillRect({ frame.right - 1.0f, line + 1.0f, frame.right, frame.bottom }, GROUP_FRAME_COLOUR);
    renderer.FillRect({ frame.left + 1.0f, frame.bottom - 1.0f, frame.right - 1.0f, frame.bottom }, GROUP_FRAME_COLOUR);
    const float baseline = renderer.BaselineInLine(FontId::ui, frame.top, DIALOG_LINE_HEIGHT);
    renderer.DrawText({ legend, TEXT_COLOUR, legend_left + LEGEND_PADDING, legend_right, baseline, TextAlign::left });
}

void ParchmentDialogs::DrawKey(const DialogDecoration& decoration)
{
    if (!c_assert(context.renderer != nullptr) || !c_assert(HasArea(decoration.bounds))) return;
    const TextSpan name = { decoration.text, TextLength(decoration.text), FontId::key };
    PaintKeyCap(*context.renderer, { ToFloatRect(decoration.bounds), name, KEY_COLOUR });
}

bool ParchmentDialogs::CreateCloseButton(Level& level)
{
    if (!c_assert(level.dialog != NULL)) return false;
    if (!c_assert(level.size.width > PANEL_PADDING + CLOSE_BUTTON_WIDTH)) return false;
    const ControlPlacement close = { level.dialog, IDCANCEL,
                                     { level.size.width - PANEL_PADDING - CLOSE_BUTTON_WIDTH, PANEL_PADDING,
                                       level.size.width - PANEL_PADDING, PANEL_PADDING + TITLE_HEIGHT },
                                     &level.shell.Backdrop(), L"" };
    return level.close_button.Create(context, close);
}

ParchmentDialogs::Handled ParchmentDialogs::HandleMessage(const MSG& message)
{
    Level* level = LevelOf(message.hwnd);
    if (level == nullptr || level->dialog == NULL) return Unhandled();
    switch (message.message)
    {
        case WM_PAINT: return OnPaint(*level);
        case WM_ERASEBKGND: return HandledWith(message.hwnd, 1);
        case WM_NCHITTEST: return OnHitTest(message.hwnd, *level, message.lParam);
        case WM_SETCURSOR: return OnSetCursor(message.hwnd, message.wParam, message.lParam);
        case WM_CTLCOLOREDIT: return OnFieldColour(message.wParam);
        case WM_CTLCOLORSTATIC: return OnReadOnlyFieldColour(message.wParam, message.lParam);
        case WM_CTLCOLORLISTBOX: return { true, reinterpret_cast<INT_PTR>(list_brush) };
        case WM_MEASUREITEM: return OnMeasureItem(message.hwnd, message.lParam);
        case WM_DRAWITEM: return OnDrawItem(message.lParam);
        case WM_WINDOWPOSCHANGED: return OnWindowPosChanged(*level, message.lParam);
        case WM_DESTROY: return OnDestroy(*level);
        case WM_COMMAND: return OnCommand(*level, message.wParam, message.lParam);
        default: return Unhandled();
    }
}

ParchmentDialogs::Handled ParchmentDialogs::OnWindowPosChanged(Level& level, LPARAM lParam)
{
    if (!c_assert(level.dialog != NULL)) return Unhandled();
    const WINDOWPOS* position = reinterpret_cast<const WINDOWPOS*>(lParam);
    level.shell.Follow();
    if (position != nullptr && (position->flags & SWP_SHOWWINDOW) != 0)
    {
        level.shell.Reveal();
        DimLevelsBelow(depth);
    }
    return Unhandled();
}

ParchmentDialogs::Handled ParchmentDialogs::OnDestroy(Level& level)
{
    ReleaseLevel(level);
    return Unhandled();
}

ParchmentDialogs::Handled ParchmentDialogs::OnPaint(Level& level)
{
    if (!c_assert(level.dialog != NULL)) return Unhandled();
    level.shell.PaintWaitingChildren();
    PAINTSTRUCT paint = {};
    const HDC target = BeginPaint(level.dialog, &paint);
    if (target != NULL) level.shell.ShowBackdrop(target);
    if (target != NULL) PaintFieldFocus(level, target);
    (void)EndPaint(level.dialog, &paint);
    return { true, TRUE };
}

void ParchmentDialogs::PaintFieldFocus(const Level& level, HDC target) const
{
    if (!c_assert(target != NULL) || !c_assert(level.dialog != NULL)) return;
    const HWND focus = GetFocus();
    if (focus == NULL || GetParent(focus) != level.dialog || !IsTextField(focus)) return;
    const PixelRect frame = TextFieldFocusFrame(AreaInParent(focus, level.dialog));
    PixelSurface* buffer = context.back_buffer;
    ParchmentRenderer* renderer = context.renderer;
    if (!c_assert(buffer != nullptr && renderer != nullptr) || !c_assert(buffer->Contains(frame))) return;
    const PixelSize size = { RectWidth(frame), RectHeight(frame) };
    const HDC buffer_dc = buffer->Dc();
    if (!BitBlt(buffer_dc, frame.left, frame.top, size.width, size.height, level.shell.Backdrop().Dc(), frame.left,
                frame.top, SRCCOPY))
        return;
    if (!renderer->BeginFrame({ buffer_dc, size, ParchmentRenderer::SurfaceAlpha::opaque, { frame.left, frame.top } }))
        return;
    renderer->DrawDottedFrame(ToFloatRect({ 0, 0, size.width, size.height }), FOCUS_COLOUR);
    if (!renderer->EndFrame()) return;
    (void)BitBlt(target, frame.left, frame.top, size.width, size.height, buffer_dc, frame.left, frame.top, SRCCOPY);
}

ParchmentDialogs::Handled ParchmentDialogs::OnCommand(const Level& level, WPARAM wParam, LPARAM lParam) const
{
    if (!c_assert(level.dialog != NULL)) return Unhandled();
    const WORD notification = HIWORD(wParam);
    const HWND control = reinterpret_cast<HWND>(lParam);
    if ((notification != EN_SETFOCUS && notification != EN_KILLFOCUS) || control == NULL) return Unhandled();
    const PixelRect frame = TextFieldFocusFrame(AreaInParent(control, level.dialog));
    const RECT area = { frame.left, frame.top, frame.right, frame.bottom };
    (void)InvalidateRect(level.dialog, &area, FALSE);
    return Unhandled();
}

ParchmentDialogs::Handled ParchmentDialogs::OnHitTest(HWND dialog, const Level& level, LPARAM lParam)
{
    if (!c_assert(dialog != NULL && dialog == level.dialog)) return Unhandled();
    POINT point = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
    if (!ScreenToClient(dialog, &point)) return Unhandled();
    const bool in_title = point.y >= PANEL_PADDING && point.y < PANEL_PADDING + TITLE_HEIGHT &&
                          point.x >= PANEL_PADDING && point.x < level.size.width - PANEL_PADDING - CLOSE_BUTTON_WIDTH;
    if (!in_title) return Unhandled();
    return HandledWith(dialog, HTCAPTION);
}

ParchmentDialogs::Handled ParchmentDialogs::OnSetCursor(HWND dialog, WPARAM wParam, LPARAM lParam)
{
    if (!c_assert(dialog != NULL) || !c_assert(context.renderer != nullptr)) return Unhandled();
    const bool over_dialog = reinterpret_cast<HWND>(wParam) == dialog;
    const UINT hit = LOWORD(lParam);
    if (!over_dialog || (hit != HTCLIENT && hit != HTCAPTION)) return Unhandled();
    (void)SetCursor(context.renderer->Cursor(CursorId::arrow));
    return HandledWith(dialog, TRUE);
}

ParchmentDialogs::Handled ParchmentDialogs::OnFieldColour(WPARAM wParam) const
{
    const HDC dc = reinterpret_cast<HDC>(wParam);
    if (!c_assert(dc != NULL) || !c_assert(field_brush != NULL)) return Unhandled();
    (void)SetTextColor(dc, RGB(FIELD_TEXT_COLOUR.red, FIELD_TEXT_COLOUR.green, FIELD_TEXT_COLOUR.blue));
    (void)SetBkColor(dc, RGB(FIELD_COLOUR.red, FIELD_COLOUR.green, FIELD_COLOUR.blue));
    return { true, reinterpret_cast<INT_PTR>(field_brush) };
}

ParchmentDialogs::Handled ParchmentDialogs::OnReadOnlyFieldColour(WPARAM wParam, LPARAM lParam) const
{
    const HWND control = reinterpret_cast<HWND>(lParam);
    if (control == NULL || !IsTextField(control)) return Unhandled();
    return OnFieldColour(wParam);
}

ParchmentDialogs::Handled ParchmentDialogs::OnMeasureItem(HWND dialog, LPARAM lParam) const
{
    if (!c_assert(dialog != NULL)) return Unhandled();
    MEASUREITEMSTRUCT* measure = reinterpret_cast<MEASUREITEMSTRUCT*>(lParam);
    if (measure == nullptr || measure->CtlType != ODT_COMBOBOX) return Unhandled();
    const bool selection_field = measure->itemID == static_cast<UINT>(-1);
    measure->itemHeight = static_cast<UINT>(selection_field ? DROPDOWN_SELECTION_HEIGHT : DROPDOWN_ITEM_HEIGHT);
    return HandledWith(dialog, TRUE);
}

ParchmentDialogs::Handled ParchmentDialogs::OnDrawItem(LPARAM lParam) const
{
    const DRAWITEMSTRUCT* item = reinterpret_cast<const DRAWITEMSTRUCT*>(lParam);
    if (item == nullptr || item->CtlType != ODT_COMBOBOX) return Unhandled();
    if (!c_assert(context.renderer != nullptr && context.back_buffer != nullptr)) return Unhandled();
    DrawDropdownItem(*context.renderer, *context.back_buffer, *item);
    return { true, TRUE };
}
