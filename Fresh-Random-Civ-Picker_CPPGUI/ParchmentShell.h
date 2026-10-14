#pragma once

#include <cstdint>

#include "framework.h"
#include "ParchmentGeometry.h"
#include "PixelSurface.h"

// What shaping a window needs only while the window is being shaped: the image it is drawn on, the coverage
// of every pixel (the shadow, then the skin's alpha) and the window region's rectangles. Windows are shaped one
// at a time, so every shell shares one workspace, allocated at startup.
class ShellWorkspace final
{
public:
    static constexpr int MAX_CANVAS_PIXELS = 1000 * 660;
    static constexpr int MAX_REGION_RECTS = 8192; // the main window's parchment needs about 660

    bool Create(PixelSize max_canvas_size);
    void Destroy();

private:
    friend class ParchmentShell;

    PixelSurface canvas;
    OpaqueRuns runs;
    ShadowBlur blur;
    uint8_t coverage[MAX_CANVAS_PIXELS] = {};
    alignas(RECT) uint8_t region_data[sizeof(RGNDATAHEADER) + MAX_REGION_RECTS * sizeof(RECT)] = {};
};

// The shape of a parchment window (View): the main window's burnt-edged parchment, or a dialog's panel with its
// drop shadow. The window itself covers the image's fully opaque pixels (its window region); a layered "skin"
// window it owns lies over it and shows everything translucent (the burnt edges, the panel's corners, the
// shadow), letting clicks through. Another layered window dims the whole shape while a dialog is open over it,
// as the web version's modal backdrop does. The layers keep their own copies of their pictures, so a shell
// keeps only its window's backdrop; the rest of the work happens in the shared workspace.
class ParchmentShell final
{
public:
    struct Setup
    {
        PixelSize max_window_size;
        int shadow_margin;
        ShellWorkspace* workspace;
    };

    // Layers never take input; disabled, they are not "enabled popups" that Windows could pick over a dialog.
    static constexpr DWORD LAYER_STYLE = WS_POPUP | WS_DISABLED;

    static bool RegisterWindowClass(HINSTANCE instance);
    static const wchar_t* LayerClass();

    bool Create(const Setup& setup);
    void Destroy();

    PixelSurface& Canvas();
    PixelPoint WindowOrigin() const;
    bool Attach(HWND window, PixelSize window_size);
    void Reveal();
    void Detach();
    void Follow();
    void PaintWaitingChildren() const;
    void ShowBackdrop(HDC target) const;
    PixelSurface& Backdrop();
    const PixelSurface& Backdrop() const;
    void Dim(int depth);

private:
    static constexpr int MAX_CHILDREN = 512;
    static constexpr float SHADOW_OPACITY = 0.35f; // drop-shadow(0 12px 24px rgba(0, 0, 0, .35))
    static constexpr int SHADOW_OFFSET_Y = 12;
    static constexpr float SHADOW_SIGMA = 12.0f;

    bool ApplyRegion();
    int AddRowRuns(int y, int rect_count);
    bool CopyBackdrop();
    void CastShadow();
    void ComposeSkin();
    void ComposeDimMask();
    HWND CreateLayer();
    bool ShowLayer(HWND layer, BYTE opacity);

    HWND owner = NULL;
    HWND skin = NULL;
    HWND dimmer = NULL;
    PixelSize window_size = { 0, 0 };
    PixelSize canvas_size = { 0, 0 };
    int margin = 0;
    ShellWorkspace* workspace = nullptr;
    PixelSurface backdrop;
};
