#include "ParchmentToast.h"

#include <cmath>
#include <cwchar>

#include "Assertions.h"
#include "ParchmentShell.h"

namespace
{
    // #toast { margin-top: 14px; padding: 5px 12px; border-radius: 3px; box-shadow: 0 2px 8px rgba(0, 0, 0, .3);
    // transition: opacity .15s }, shown for 2.2 s (toast()).
    constexpr int GAP_BELOW_WINDOW = 14;
    constexpr int PADDING_X = 12;
    constexpr int PADDING_Y = 5;
    constexpr float RADIUS = 3.0f;
    constexpr int SHADOW_OFFSET_Y = 2;
    constexpr float SHADOW_SIGMA = 4.0f;
    constexpr float SHADOW_OPACITY = 0.3f;
    constexpr ULONGLONG FADE_MS = 150;
    constexpr ULONGLONG SHOWN_MS = 2200;
}

bool ParchmentToast::Create(HWND owner_window, ParchmentRenderer& toast_renderer)
{
    if (!c_assert(owner_window != NULL) || !c_assert(window == NULL)) return false;
    owner = owner_window;
    renderer = &toast_renderer;
    if (!surface.Create({ MAX_BOX_WIDTH + 2 * SHADOW_MARGIN, MAX_BOX_HEIGHT + 2 * SHADOW_MARGIN })) return false;
    const HINSTANCE instance = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(owner, GWLP_HINSTANCE));
    const DWORD extended_style = WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW;
    window = CreateWindowExW(extended_style, ParchmentShell::LayerClass(), L"", ParchmentShell::LAYER_STYLE, 0, 0, 1,
                             1, owner, NULL, instance, NULL);
    return c_assert(window != NULL);
}

void ParchmentToast::Destroy()
{
    // The window goes with its owner anyway; destroying it first leaves no stale handle.
    if (window != NULL) (void)DestroyWindow(window);
    window = NULL;
    owner = NULL;
    shown = false;
    surface.Destroy();
}

// Shows a message, or puts it in place of the one on show, which then stays its full time again.
bool ParchmentToast::Show(const wchar_t* message)
{
    if (!c_assert(window != NULL) || !c_assert(message != nullptr)) return false;
    if (!LayOut(message) || !Render()) return false;
    const ULONGLONG now = GetTickCount64();
    // A message replacing one on show appears at once; a new one fades in.
    shown_at = shown ? now - FADE_MS : now;
    if (!Present(OpacityAt(now - shown_at))) return false;
    // ShowWindow reports the earlier visibility, not success.
    if (!shown) (void)ShowWindow(window, SW_SHOWNOACTIVATE);
    shown = true;
    return true;
}

// Fades the message in and out; false once it has gone, so the owner can stop its timer.
bool ParchmentToast::Tick()
{
    if (!shown || !c_assert(window != NULL)) return false;
    const ULONGLONG elapsed = GetTickCount64() - shown_at;
    if (elapsed >= SHOWN_MS + FADE_MS)
    {
        Hide();
        return false;
    }
    // While fully shown nothing changes; a failed update leaves the message as it was until the next tick.
    if (elapsed < FADE_MS || elapsed > SHOWN_MS) (void)Present(OpacityAt(elapsed));
    return true;
}

void ParchmentToast::Hide()
{
    if (!shown || window == NULL) return;
    (void)ShowWindow(window, SW_HIDE);
    shown = false;
}

// One line of text in the box's padding; the box is as wide as the text.
bool ParchmentToast::LayOut(const wchar_t* message)
{
    if (!c_assert(message != nullptr) || !c_assert(renderer != nullptr)) return false;
    text_length = wcsncpy_s(text, message, _TRUNCATE) == 0 ? static_cast<int>(wcslen(text)) : 0;
    if (!c_assert(text_length > 0)) return false;
    const float width = renderer->MeasureText({ text, text_length, FontId::ui });
    box_size = { static_cast<int>(std::lround(width)) + 2 * PADDING_X,
                 static_cast<int>(std::lround(UI_LINE_HEIGHT)) + 2 * PADDING_Y };
    if (box_size.width > MAX_BOX_WIDTH) box_size.width = MAX_BOX_WIDTH;
    return c_assert(box_size.height <= MAX_BOX_HEIGHT);
}

bool ParchmentToast::Render()
{
    if (!c_assert(renderer != nullptr)) return false;
    if (!c_assert(box_size.width > 0 && box_size.width <= MAX_BOX_WIDTH)) return false;
    const PixelSize frame = { box_size.width + 2 * SHADOW_MARGIN, box_size.height + 2 * SHADOW_MARGIN };
    if (!renderer->BeginFrame({ surface.Dc(), frame, ParchmentRenderer::SurfaceAlpha::premultiplied })) return false;
    renderer->Clear();
    const float left = static_cast<float>(SHADOW_MARGIN);
    const float top = static_cast<float>(SHADOW_MARGIN);
    const FloatRect box = { left, top, left + static_cast<float>(box_size.width),
                            top + static_cast<float>(box_size.height) };
    renderer->FillRoundedRect(box, RADIUS, TOAST_COLOUR);
    const float baseline = renderer->BaselineInLine(FontId::ui, top + static_cast<float>(PADDING_Y), UI_LINE_HEIGHT);
    renderer->DrawText({ { text, text_length, FontId::ui }, TOAST_TEXT_COLOUR, left + static_cast<float>(PADDING_X),
                         box.right, baseline, TextAlign::left });
    if (!renderer->EndFrame()) return false;
    AddShadow();
    return true;
}

// The shadow: the box's area moved 2 px down and blurred, in black at 30 %, under the box's own pixels (they are
// premultiplied, so what the box leaves uncovered, such as its rounded corners, lets the shadow through).
void ParchmentToast::AddShadow()
{
    const PixelSize frame = { box_size.width + 2 * SHADOW_MARGIN, box_size.height + 2 * SHADOW_MARGIN };
    if (!c_assert(frame.width * frame.height <= MAX_FRAME_PIXELS)) return;
    const PixelRect cast = { SHADOW_MARGIN, SHADOW_MARGIN + SHADOW_OFFSET_Y, SHADOW_MARGIN + box_size.width,
                             SHADOW_MARGIN + SHADOW_OFFSET_Y + box_size.height };
    for (int y = 0; y < frame.height; y++)
    {
        for (int x = 0; x < frame.width; x++) shadow[y * frame.width + x] = RectContains(cast, { x, y }) ? 255 : 0;
    }
    // Without the blur the shadow keeps a hard edge, which still reads as a shadow.
    (void)blur.Blur({ shadow, frame.width, frame.height, SHADOW_SIGMA });
    uint32_t* pixels = surface.Pixels();
    const int stride = surface.Size().width;
    for (int y = 0; y < frame.height; y++)
    {
        for (int x = 0; x < frame.width; x++)
        {
            uint32_t& pixel = pixels[static_cast<ptrdiff_t>(y) * stride + x];
            const uint32_t box_alpha = pixel >> 24;
            const float uncovered = static_cast<float>(255u - box_alpha) / 255.0f;
            const float shade = SHADOW_OPACITY * static_cast<float>(shadow[y * frame.width + x]) * uncovered;
            pixel = (pixel & 0x00ffffffu) | ((box_alpha + static_cast<uint32_t>(std::lround(shade))) << 24);
        }
    }
}

// Puts the message centred below the owner, as the web version's #toast sits below its window.
bool ParchmentToast::Present(BYTE opacity)
{
    RECT owner_rect = {};
    if (!c_assert(window != NULL) || !GetWindowRect(owner, &owner_rect)) return false;
    POINT position = { (owner_rect.left + owner_rect.right - box_size.width) / 2 - SHADOW_MARGIN,
                       owner_rect.bottom + GAP_BELOW_WINDOW - SHADOW_MARGIN };
    SIZE size = { box_size.width + 2 * SHADOW_MARGIN, box_size.height + 2 * SHADOW_MARGIN };
    POINT origin = { 0, 0 };
    BLENDFUNCTION blend = { AC_SRC_OVER, 0, opacity, AC_SRC_ALPHA };
    return UpdateLayeredWindow(window, NULL, &position, &size, surface.Dc(), &origin, 0, &blend, ULW_ALPHA) != FALSE;
}

BYTE ParchmentToast::OpacityAt(ULONGLONG elapsed) const
{
    float opacity = 1.0f;
    if (elapsed < FADE_MS) opacity = static_cast<float>(elapsed) / static_cast<float>(FADE_MS);
    else if (elapsed > SHOWN_MS) opacity = 1.0f - static_cast<float>(elapsed - SHOWN_MS) / static_cast<float>(FADE_MS);
    if (opacity < 0.0f) opacity = 0.0f;
    return static_cast<BYTE>(std::lround(opacity * 255.0f));
}
