#pragma once

#include <cstdint>

#include "framework.h"
#include "ParchmentGeometry.h"
#include "PixelSurface.h"

class ShellWorkspace final
{
public:
    static constexpr int MAX_CANVAS_PIXELS = 1000 * 700;
    static constexpr int MAX_REGION_RECTS = 8192;

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

class ParchmentShell final
{
public:
    struct Setup
    {
        PixelSize max_window_size;
        int shadow_margin;
        ShellWorkspace* workspace;
    };

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
    static constexpr float SHADOW_OPACITY = 0.35f;
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
