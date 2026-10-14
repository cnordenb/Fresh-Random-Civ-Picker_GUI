#pragma once

#include <cstdint>

#include "framework.h"
#include "ParchmentGeometry.h"
#include "ParchmentRenderer.h"
#include "PixelSurface.h"

// The tooltip of the parchment theme (View): the web version's tan box (#tip) that follows the mouse, with the
// external-link icon before the text of a button that opens another site. One layered window, made at startup,
// shows every tooltip; its soft shadow is drawn with it.
class ParchmentTooltip final
{
public:
    static constexpr int MAX_TEXT_LENGTH = 512;

    struct Content
    {
        const wchar_t* text;
        bool external;
    };

    bool Create(HWND owner, ParchmentRenderer& renderer);
    void Destroy();
    void Show(const Content& content, PixelPoint cursor);
    void Hide();

private:
    static constexpr int SHADOW_MARGIN = 6;

    bool LayOut(const Content& content);
    bool Render();
    void PaintBox(PixelSize box);
    void AddShadow(PixelSize box);
    PixelRect WorkArea(PixelPoint cursor) const;

    HWND window = NULL;
    ParchmentRenderer* renderer = nullptr;
    PixelSurface surface;
    WrappedLines lines;
    BoxShadow box_shadow;
    wchar_t text[MAX_TEXT_LENGTH] = L"";
    int text_length = 0;
    bool external = false;
    PixelSize box_size = { 0, 0 };
    bool shown = false;
};
