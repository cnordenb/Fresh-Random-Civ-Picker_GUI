#include "ParchmentShell.h"

#include <cmath>

#include "Assertions.h"

namespace
{
    const wchar_t* const LAYER_CLASS = L"FrcpParchmentLayer";
    constexpr float DIM_PER_DIALOG = 0.85f; // dialog::backdrop { background: rgba(0, 0, 0, .15) }
    constexpr int MAX_ROW_WIDTH = MAX_SHADOW_SIDE;

    uint32_t AlphaOf(uint32_t pixel) { return pixel >> 24; }
}

bool ParchmentShell::RegisterWindowClass(HINSTANCE instance)
{
    if (!c_assert(instance != NULL)) return false;
    WNDCLASSEXW class_info = {};
    class_info.cbSize = sizeof(class_info);
    // The layers only show an image and never handle a message themselves, so Windows' own default procedure is
    // their window procedure (a function pointer Win32 requires, CLAUDE2.md I2).
    class_info.lpfnWndProc = DefWindowProcW;
    class_info.hInstance = instance;
    class_info.lpszClassName = LAYER_CLASS;
    return c_assert(RegisterClassExW(&class_info) != 0);
}

const wchar_t* ParchmentShell::LayerClass() { return LAYER_CLASS; }

bool ShellWorkspace::Create(PixelSize max_canvas_size)
{
    if (!c_assert(max_canvas_size.width > 0 && max_canvas_size.width <= MAX_ROW_WIDTH)) return false;
    if (!c_assert(max_canvas_size.width * max_canvas_size.height <= MAX_CANVAS_PIXELS)) return false;
    return canvas.Create(max_canvas_size);
}

void ShellWorkspace::Destroy() { canvas.Destroy(); }

bool ParchmentShell::Create(const Setup& setup)
{
    if (!c_assert(setup.shadow_margin >= 0) || !c_assert(setup.max_window_size.width > 0)) return false;
    if (!c_assert(setup.workspace != nullptr)) return false;
    workspace = setup.workspace;
    margin = setup.shadow_margin;
    const PixelSize canvas_limit = { setup.max_window_size.width + 2 * margin,
                                     setup.max_window_size.height + 2 * margin };
    const PixelSize workspace_size = workspace->canvas.Size();
    if (!c_assert(canvas_limit.width <= workspace_size.width && canvas_limit.height <= workspace_size.height))
        return false;
    return backdrop.Create(setup.max_window_size);
}

void ParchmentShell::Destroy()
{
    Detach();
    backdrop.Destroy();
}

// The window's image is drawn here before Attach: the shared workspace's canvas.
PixelSurface& ParchmentShell::Canvas() { return workspace->canvas; }

PixelPoint ParchmentShell::WindowOrigin() const { return { margin, margin }; }

PixelSurface& ParchmentShell::Backdrop() { return backdrop; }

const PixelSurface& ParchmentShell::Backdrop() const { return backdrop; }

// Shapes the window after the image drawn on the canvas (at WindowOrigin, premultiplied BGRA): the opaque
// pixels become the window and its backdrop, the rest the skin, with the shadow if the shell has a margin.
bool ParchmentShell::Attach(HWND window, PixelSize size)
{
    if (!c_assert(window != NULL) || !c_assert(skin == NULL) || !c_assert(workspace != nullptr)) return false;
    const PixelSize canvas_limit = workspace->canvas.Size();
    if (!c_assert(size.width + 2 * margin <= canvas_limit.width && size.height + 2 * margin <= canvas_limit.height))
        return false;
    owner = window;
    window_size = size;
    canvas_size = { size.width + 2 * margin, size.height + 2 * margin };
    if (!ApplyRegion() || !CopyBackdrop()) return false;
    CastShadow();
    ComposeSkin();
    skin = CreateLayer();
    dimmer = CreateLayer();
    if (!c_assert(skin != NULL && dimmer != NULL) || !ShowLayer(skin, 255)) return false;
    // The dimmer gets its picture now, while the workspace holds this shape; dimming only changes its opacity.
    ComposeDimMask();
    return ShowLayer(dimmer, 255);
}

// Called as the window shows. A new window starts out black, and with WS_CLIPCHILDREN its own paint leaves its
// controls' places black until each control has painted; the backdrop goes under them first, then they paint.
void ParchmentShell::Reveal()
{
    if (!c_assert(skin != NULL)) return;
    // Without DCX_CLIPCHILDREN (or DCX_USESTYLE) the device context reaches under the controls.
    const HDC target = GetDCEx(owner, NULL, DCX_CACHE);
    if (target != NULL)
    {
        ShowBackdrop(target);
        // ReleaseDC can't fail for a cache device context.
        (void)ReleaseDC(owner, target);
    }
    // A paint that fails is repeated by the window's next one.
    (void)RedrawWindow(owner, nullptr, NULL, RDW_UPDATENOW | RDW_ALLCHILDREN);
    // ShowWindow reports the earlier visibility, not success.
    (void)ShowWindow(skin, SW_SHOWNOACTIVATE);
}

// Called by the window's paint before it paints itself. Windows paints a window before its controls; drawing
// the controls first lets the window and its controls change on the screen together, with no moment where the
// window shows its new backdrop around the controls' old pictures.
void ParchmentShell::PaintWaitingChildren() const
{
    if (owner == NULL) return;
    HWND child = GetWindow(owner, GW_CHILD);
    for (int i = 0; i < MAX_CHILDREN && child != NULL; i++)
    {
        // UpdateWindow paints only a child that waits for its paint; a failed paint is repeated by the next one.
        if (IsWindowVisible(child)) (void)UpdateWindow(child);
        child = GetWindow(child, GW_HWNDNEXT);
    }
    // Children past MAX_CHILDREN paint after the window, as they would without this.
    (void)c_assert(child == NULL);
}

void ParchmentShell::ShowBackdrop(HDC target) const
{
    if (!c_assert(target != NULL) || !c_assert(owner != NULL)) return;
    // A failed copy leaves the window's earlier picture until its next paint.
    (void)BitBlt(target, 0, 0, window_size.width, window_size.height, backdrop.Dc(), 0, 0, SRCCOPY);
}

void ParchmentShell::Detach()
{
    // The layers belong to the owner and go with it anyway; destroying them first keeps no stale handles.
    if (skin != NULL) (void)DestroyWindow(skin);
    if (dimmer != NULL) (void)DestroyWindow(dimmer);
    skin = NULL;
    dimmer = NULL;
    owner = NULL;
}

bool ParchmentShell::ApplyRegion()
{
    RGNDATA* data = reinterpret_cast<RGNDATA*>(workspace->region_data);
    int rect_count = 0;
    for (int y = 0; y < window_size.height && rect_count >= 0; y++) rect_count = AddRowRuns(y, rect_count);
    if (!c_assert(rect_count > 0)) return false;
    data->rdh.dwSize = sizeof(RGNDATAHEADER);
    data->rdh.iType = RDH_RECTANGLES;
    data->rdh.nCount = static_cast<DWORD>(rect_count);
    data->rdh.nRgnSize = static_cast<DWORD>(rect_count) * sizeof(RECT);
    data->rdh.rcBound = { 0, 0, window_size.width, window_size.height };
    const DWORD bytes = sizeof(RGNDATAHEADER) + data->rdh.nRgnSize;
    const HRGN region = ExtCreateRegion(nullptr, bytes, data);
    if (!c_assert(region != NULL)) return false;
    // On success the window owns the region; on failure it is ours to free.
    const bool applied = SetWindowRgn(owner, region, TRUE) != 0;
    if (!applied) (void)DeleteObject(region);
    return c_assert(applied);
}

// Adds a rectangle for every fully opaque run of one row; returns the new count, or -1 when full.
int ParchmentShell::AddRowRuns(int y, int rect_count)
{
    uint8_t alpha_row[MAX_ROW_WIDTH] = {};
    const PixelSurface& canvas = workspace->canvas;
    const uint32_t* row = canvas.Pixels() + static_cast<ptrdiff_t>(y + margin) * canvas.Size().width + margin;
    for (int x = 0; x < window_size.width && x < MAX_ROW_WIDTH; x++)
    {
        alpha_row[x] = static_cast<uint8_t>(AlphaOf(row[x]));
    }
    OpaqueRuns& runs = workspace->runs;
    if (!runs.Scan(alpha_row, window_size.width)) return -1;
    RECT* rects = reinterpret_cast<RECT*>(workspace->region_data + sizeof(RGNDATAHEADER));
    int count = rect_count;
    for (int i = 0; i < runs.Count(); i++)
    {
        if (!c_assert(count < ShellWorkspace::MAX_REGION_RECTS)) return -1;
        const PixelRun run = runs.Run(i);
        rects[count] = { run.start, y, run.end, y + 1 };
        count++;
    }
    return count;
}

bool ParchmentShell::CopyBackdrop()
{
    return c_assert(BitBlt(backdrop.Dc(), 0, 0, window_size.width, window_size.height, workspace->canvas.Dc(),
                           margin, margin, SRCCOPY) != FALSE);
}

// The CSS drop shadow: the image's alpha, moved down 12 px and blurred (a Gaussian of 12 px).
void ParchmentShell::CastShadow()
{
    if (margin == 0) return;
    const int width = canvas_size.width;
    const PixelSurface& canvas = workspace->canvas;
    const uint32_t* pixels = canvas.Pixels();
    uint8_t* shadow = workspace->coverage;
    for (int y = 0; y < canvas_size.height; y++)
    {
        const int source_y = y - SHADOW_OFFSET_Y;
        for (int x = 0; x < width; x++)
        {
            const bool inside = source_y >= 0 && source_y < canvas_size.height;
            const uint32_t source = inside ? pixels[static_cast<ptrdiff_t>(source_y) * canvas.Size().width + x] : 0;
            shadow[static_cast<ptrdiff_t>(y) * width + x] = static_cast<uint8_t>(AlphaOf(source));
        }
    }
    // A failed blur leaves a hard-edged shadow, which still shows the dialog; nothing else depends on it.
    (void)workspace->blur.Blur({ shadow, width, canvas_size.height, SHADOW_SIGMA });
}

// Turns the canvas into the skin: the image over its shadow, cleared where the window itself shows the image.
// The coverage of every pixel (image and shadow together) stays behind for the dimming mask.
void ParchmentShell::ComposeSkin()
{
    uint32_t* pixels = workspace->canvas.Pixels();
    uint8_t* coverage = workspace->coverage;
    const int stride = workspace->canvas.Size().width;
    for (int y = 0; y < canvas_size.height; y++)
    {
        for (int x = 0; x < canvas_size.width; x++)
        {
            const ptrdiff_t index = static_cast<ptrdiff_t>(y) * stride + x;
            const uint32_t alpha = AlphaOf(pixels[index]);
            const ptrdiff_t coverage_index = static_cast<ptrdiff_t>(y) * canvas_size.width + x;
            const float shadow = static_cast<float>(coverage[coverage_index]);
            const float shadow_alpha = margin > 0 ? SHADOW_OPACITY * shadow : 0.0f;
            const float uncovered = 255.0f - static_cast<float>(alpha);
            const uint32_t total = alpha + static_cast<uint32_t>(std::lround(shadow_alpha * uncovered / 255.0f));
            coverage[coverage_index] = static_cast<uint8_t>(total);
            const bool in_window = x >= margin && y >= margin && x < margin + window_size.width &&
                                   y < margin + window_size.height;
            pixels[index] = (in_window && alpha == 255) ? 0 : (pixels[index] & 0x00ffffffu) | (total << 24);
        }
    }
}

// The dimming mask: the shape in black, as opaque as the image and its shadow cover each pixel.
void ParchmentShell::ComposeDimMask()
{
    uint32_t* pixels = workspace->canvas.Pixels();
    const uint8_t* coverage = workspace->coverage;
    const int stride = workspace->canvas.Size().width;
    for (int y = 0; y < canvas_size.height; y++)
    {
        for (int x = 0; x < canvas_size.width; x++)
        {
            const uint32_t total = coverage[static_cast<ptrdiff_t>(y) * canvas_size.width + x];
            pixels[static_cast<ptrdiff_t>(y) * stride + x] = total << 24;
        }
    }
}

HWND ParchmentShell::CreateLayer()
{
    RECT owner_rect = {};
    if (!c_assert(GetWindowRect(owner, &owner_rect) != FALSE)) return NULL;
    const HINSTANCE instance = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(owner, GWLP_HINSTANCE));
    const DWORD extended_style = WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW;
    return CreateWindowExW(extended_style, LAYER_CLASS, L"", LAYER_STYLE, owner_rect.left - margin,
                           owner_rect.top - margin, canvas_size.width, canvas_size.height, owner, NULL, instance, NULL);
}

// Hands the layer the workspace canvas as its picture; the layer keeps its own copy.
bool ParchmentShell::ShowLayer(HWND layer, BYTE opacity)
{
    RECT owner_rect = {};
    if (!c_assert(layer != NULL) || !c_assert(GetWindowRect(owner, &owner_rect) != FALSE)) return false;
    POINT position = { owner_rect.left - margin, owner_rect.top - margin };
    SIZE size = { canvas_size.width, canvas_size.height };
    POINT origin = { 0, 0 };
    BLENDFUNCTION blend = { AC_SRC_OVER, 0, opacity, AC_SRC_ALPHA };
    return c_assert(UpdateLayeredWindow(layer, NULL, &position, &size, workspace->canvas.Dc(), &origin, 0, &blend,
                                        ULW_ALPHA) != FALSE);
}

// Dims the shape once for each dialog open over it.
void ParchmentShell::Dim(int depth)
{
    if (!c_assert(depth >= 0) || dimmer == NULL) return;
    if (depth == 0)
    {
        (void)ShowWindow(dimmer, SW_HIDE);
        return;
    }
    const float dimmed = 1.0f - std::pow(DIM_PER_DIALOG, static_cast<float>(depth));
    BLENDFUNCTION blend = { AC_SRC_OVER, 0, static_cast<BYTE>(std::lround(dimmed * 255.0f)), AC_SRC_ALPHA };
    // The dimmer's picture is unchanged, so only its opacity goes to UpdateLayeredWindow (no source DC).
    if (!c_assert(UpdateLayeredWindow(dimmer, NULL, nullptr, nullptr, NULL, nullptr, 0, &blend, ULW_ALPHA) != FALSE))
        return;
    (void)ShowWindow(dimmer, SW_SHOWNOACTIVATE);
}

void ParchmentShell::Follow()
{
    RECT owner_rect = {};
    if (owner == NULL || !GetWindowRect(owner, &owner_rect)) return;
    const UINT flags = SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE;
    const int left = owner_rect.left - margin;
    const int top = owner_rect.top - margin;
    // A layer that couldn't move catches up with the next move; nothing else depends on it.
    if (skin != NULL) (void)SetWindowPos(skin, NULL, left, top, 0, 0, flags);
    if (dimmer != NULL) (void)SetWindowPos(dimmer, NULL, left, top, 0, 0, flags);
}
