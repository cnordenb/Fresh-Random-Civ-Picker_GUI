#pragma once

#include <cstdint>

#include "framework.h"
#include "ParchmentGeometry.h"
#include "ParchmentRenderer.h"
#include "PixelSurface.h"

class ParchmentToast final
{
public:
    static constexpr int MAX_TEXT_LENGTH = 128;
    static constexpr UINT TICK_MS = 15;

    bool Create(HWND owner_window, ParchmentRenderer& renderer);
    void Destroy();
    bool Show(const wchar_t* message);
    bool Tick();
    void Hide();

private:
    static constexpr int MAX_BOX_WIDTH = 480;
    static constexpr int MAX_BOX_HEIGHT = 40;
    static constexpr int SHADOW_MARGIN = 14;
    static constexpr int MAX_FRAME_PIXELS = (MAX_BOX_WIDTH + 2 * SHADOW_MARGIN) * (MAX_BOX_HEIGHT + 2 * SHADOW_MARGIN);

    bool LayOut(const wchar_t* message);
    bool Render();
    void AddShadow();
    bool Present(BYTE opacity);
    BYTE OpacityAt(ULONGLONG elapsed) const;

    HWND window = NULL;
    HWND owner = NULL;
    ParchmentRenderer* renderer = nullptr;
    PixelSurface surface;
    ShadowBlur blur;
    uint8_t shadow[MAX_FRAME_PIXELS] = {};
    wchar_t text[MAX_TEXT_LENGTH] = L"";
    int text_length = 0;
    PixelSize box_size = { 0, 0 };
    ULONGLONG shown_at = 0;
    bool shown = false;
};
