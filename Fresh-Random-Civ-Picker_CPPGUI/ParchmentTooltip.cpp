#include "ParchmentTooltip.h"

#include <cmath>
#include <cwchar>

#include "Assertions.h"
#include "ParchmentShell.h"

namespace
{
    // #tip { max-width: 300px; padding: 3px 7px 4px; border: 1px; box-shadow: 1px 1px 3px rgba(0, 0, 0, .18) }
    constexpr int MAX_BOX_WIDTH = 300;
    constexpr int MAX_BOX_HEIGHT = 148;
    constexpr int BORDER = 1;
    constexpr int PADDING_TOP = 3;
    constexpr int PADDING_BOTTOM = 4;
    constexpr int PADDING_SIDE = 7;
    constexpr float SHADOW_OPACITY = 0.18f;
    constexpr int SHADOW_OFFSET = 1;
    constexpr float SHADOW_SIGMA = 1.5f;
    // #tip.ext::before { width: 12px; margin-right: 4px; vertical-align: -1px }
    constexpr float ICON_ROOM = EXTERNAL_ICON_SIZE + 4.0f;
    constexpr float ICON_DROP = 1.0f;
}

bool ParchmentTooltip::Create(HWND owner, ParchmentRenderer& tooltip_renderer)
{
    if (!c_assert(owner != NULL) || !c_assert(window == NULL)) return false;
    renderer = &tooltip_renderer;
    if (!surface.Create({ MAX_BOX_WIDTH + 2 * SHADOW_MARGIN, MAX_BOX_HEIGHT + 2 * SHADOW_MARGIN })) return false;
    const HINSTANCE instance = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(owner, GWLP_HINSTANCE));
    const DWORD extended_style =
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW | WS_EX_TOPMOST;
    window = CreateWindowExW(extended_style, ParchmentShell::LayerClass(), L"", ParchmentShell::LAYER_STYLE, 0, 0, 1,
                             1, owner, NULL, instance, NULL);
    return c_assert(window != NULL);
}

void ParchmentTooltip::Destroy()
{
    // The window goes with its owner anyway; destroying it first leaves no stale handle.
    if (window != NULL) (void)DestroyWindow(window);
    window = NULL;
    surface.Destroy();
}

void ParchmentTooltip::Show(const Content& content, PixelPoint cursor)
{
    if (!c_assert(window != NULL) || !c_assert(content.text != nullptr)) return;
    const bool same = content.external == external && wcsncmp(content.text, text, MAX_TEXT_LENGTH) == 0;
    if (!same && (!LayOut(content) || !Render())) return;

    const PixelPoint box = PlaceTooltip({ cursor, box_size, WorkArea(cursor) });
    POINT position = { box.x - SHADOW_MARGIN, box.y - SHADOW_MARGIN };
    SIZE size = { box_size.width + 2 * SHADOW_MARGIN, box_size.height + 2 * SHADOW_MARGIN };
    POINT origin = { 0, 0 };
    BLENDFUNCTION blend = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
    if (!UpdateLayeredWindow(window, NULL, &position, &size, surface.Dc(), &origin, 0, &blend, ULW_ALPHA)) return;
    if (shown) return;
    // ShowWindow reports the earlier visibility, not success.
    (void)ShowWindow(window, SW_SHOWNOACTIVATE);
    shown = true;
}

void ParchmentTooltip::Hide()
{
    if (!shown || window == NULL) return;
    (void)ShowWindow(window, SW_HIDE);
    shown = false;
}

// Wraps the text to the box's widest content (white-space: pre-line); the icon takes room on the first line.
bool ParchmentTooltip::LayOut(const Content& content)
{
    if (!c_assert(content.text != nullptr) || !c_assert(renderer != nullptr)) return false;
    text_length = wcsncpy_s(text, content.text, _TRUNCATE) == 0 ? static_cast<int>(wcslen(text)) : 0;
    external = content.external;
    if (!c_assert(renderer != nullptr)) return false;
    float advances[MAX_TEXT_LENGTH] = {};
    const float* measured = renderer->MeasureAdvances({ text, text_length, FontId::ui });
    for (int i = 0; i < text_length; i++) advances[i] = measured[i];
    if (external && text_length > 0) advances[0] += ICON_ROOM;
    const float max_width = static_cast<float>(MAX_BOX_WIDTH - 2 * (BORDER + PADDING_SIDE));
    if (!lines.Wrap({ text, advances, text_length, max_width })) return false;

    const float content_width = lines.WidestLine();
    const float content_height = static_cast<float>(lines.Count()) * UI_LINE_HEIGHT;
    box_size = { static_cast<int>(std::lround(content_width)) + 2 * (BORDER + PADDING_SIDE),
                 static_cast<int>(std::lround(content_height)) + 2 * BORDER + PADDING_TOP + PADDING_BOTTOM };
    if (box_size.width > MAX_BOX_WIDTH) box_size.width = MAX_BOX_WIDTH;
    return c_assert(box_size.height <= MAX_BOX_HEIGHT);
}

bool ParchmentTooltip::Render()
{
    if (!c_assert(renderer != nullptr)) return false;
    if (!c_assert(box_size.width > 0 && box_size.width <= MAX_BOX_WIDTH && box_size.height <= MAX_BOX_HEIGHT))
        return false;
    const PixelSize frame = { box_size.width + 2 * SHADOW_MARGIN, box_size.height + 2 * SHADOW_MARGIN };
    if (!renderer->BeginFrame({ surface.Dc(), frame, ParchmentRenderer::SurfaceAlpha::opaque })) return false;
    PaintBox(box_size);
    if (!renderer->EndFrame()) return false;
    AddShadow(box_size);
    return true;
}

void ParchmentTooltip::PaintBox(PixelSize box)
{
    if (!c_assert(box.width > 0 && box.height > 0) || !c_assert(renderer != nullptr)) return;
    const float left = static_cast<float>(SHADOW_MARGIN);
    const float top = static_cast<float>(SHADOW_MARGIN);
    const FloatRect outer = { left, top, left + static_cast<float>(box.width), top + static_cast<float>(box.height) };
    renderer->FillRect(outer, TOOLTIP_BORDER_COLOUR);
    renderer->FillRect({ outer.left + BORDER, outer.top + BORDER, outer.right - BORDER, outer.bottom - BORDER },
                       TOOLTIP_COLOUR);
    const float text_left = left + static_cast<float>(BORDER + PADDING_SIDE);
    for (int i = 0; i < lines.Count() && i < MAX_WRAPPED_LINES; i++)
    {
        const TextLine line = lines.Line(i);
        const float line_top = top + static_cast<float>(BORDER + PADDING_TOP) + static_cast<float>(i) * UI_LINE_HEIGHT;
        const float baseline = renderer->BaselineInLine(FontId::ui, line_top, UI_LINE_HEIGHT);
        float line_left = text_left;
        if (i == 0 && external)
        {
            const float icon_top = baseline + ICON_DROP - EXTERNAL_ICON_SIZE;
            const PixelSize icon = renderer->ImageSize(ImageId::external_icon_tooltip);
            renderer->DrawImage({ ImageId::external_icon_tooltip, { 0, 0, icon.width, icon.height },
                                  { line_left, icon_top, line_left + EXTERNAL_ICON_SIZE,
                                    icon_top + EXTERNAL_ICON_SIZE },
                                  1.0f, ParchmentRenderer::Smoothing::smooth });
            line_left += ICON_ROOM;
        }
        renderer->DrawText({ { text + line.start, line.length, FontId::ui }, TOOLTIP_TEXT_COLOUR, line_left,
                             outer.right, baseline, TextAlign::left });
    }
}

// The box is opaque; around it lies its shadow, 1 px right and down and softened, in premultiplied black.
void ParchmentTooltip::AddShadow(PixelSize box)
{
    if (!c_assert(box.width > 0 && box.width <= MAX_BOX_WIDTH && box.height <= MAX_BOX_HEIGHT)) return;
    const PixelSize frame = { box.width + 2 * SHADOW_MARGIN, box.height + 2 * SHADOW_MARGIN };
    const PixelRect box_area = { SHADOW_MARGIN, SHADOW_MARGIN, SHADOW_MARGIN + box.width, SHADOW_MARGIN + box.height };
    // A tooltip without its shadow is still readable, so a failure here changes nothing else.
    (void)box_shadow.Apply({ surface.Pixels(), surface.Size().width, frame, box_area, { SHADOW_OFFSET, SHADOW_OFFSET },
                             SHADOW_SIGMA, SHADOW_OPACITY });
}

PixelRect ParchmentTooltip::WorkArea(PixelPoint cursor) const
{
    MONITORINFO monitor = {};
    monitor.cbSize = sizeof(monitor);
    const HMONITOR handle = MonitorFromPoint({ cursor.x, cursor.y }, MONITOR_DEFAULTTONEAREST);
    if (!GetMonitorInfoW(handle, &monitor))
        return { 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN) };
    return { monitor.rcWork.left, monitor.rcWork.top, monitor.rcWork.right, monitor.rcWork.bottom };
}
