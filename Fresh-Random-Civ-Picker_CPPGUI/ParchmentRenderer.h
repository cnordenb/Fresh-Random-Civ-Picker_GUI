#pragma once

#include "framework.h"
#include "ParchmentFonts.h"
#include "ParchmentGeometry.h"
#include "ParchmentImages.h"
#include "ParchmentTheme.h"

struct ID2D1DCRenderTarget;
struct ID2D1DeviceContext;
struct ID2D1Factory1;
struct ID2D1Layer;
struct ID2D1RenderTarget;
struct ID2D1SolidColorBrush;
struct IWICImagingFactory;

// Draws the parchment theme (View): the one place that talks to Direct2D, DirectWrite and WIC (CC8 boundary).
// Started once before the windows exist; everything it draws with is built then. A frame binds a device
// context, takes the drawing calls and ends; opaque frames get ClearType text like a browser, premultiplied
// frames (for layered windows) keep their alpha.
class ParchmentRenderer final
{
public:
    enum class SurfaceAlpha { opaque, premultiplied };
    enum class Smoothing { smooth, pixelated };

    // The frame covers size pixels of the device context from origin on; (0, 0) is drawn at origin.
    struct FrameTarget
    {
        HDC dc;
        PixelSize size;
        SurfaceAlpha alpha;
        PixelPoint origin;
    };

    struct ImageDraw
    {
        ImageId image;
        PixelRect source;
        FloatRect target;
        float opacity;
        Smoothing smoothing;
    };

    struct AtlasDraw
    {
        ImageId atlas;
        int cell;
        int columns;
        PixelSize cell_size;
        FloatRect target;
    };

    struct NineSliceDraw
    {
        NineSliceId nine_slice;
        FloatRect target;
        float border;
        TileRule edges;
        TileRule middle;
        bool fill_middle;
    };

    struct TextDraw
    {
        TextSpan span;
        Colour colour;
        float left;
        float right;
        float baseline;
        TextAlign align;
    };

    bool Start(const wchar_t* image_folder);
    void Stop();
    void ForgetImage(ImageId image);

    bool BeginFrame(const FrameTarget& frame);
    bool EndFrame();
    void SetOrigin(PixelPoint origin);
    void PushClip(const FloatRect& area);
    void PopClip();
    void PushOpacity(float opacity);
    void PopOpacity();

    void DrawImage(const ImageDraw& draw);
    void DrawAtlasCell(const AtlasDraw& draw);
    void DrawNineSlice(const NineSliceDraw& draw);
    void Clear();
    void FillRect(const FloatRect& area, Colour colour);
    void FillRoundedRect(const FloatRect& area, float radius, Colour colour);
    void DrawDottedFrame(const FloatRect& area, Colour colour);
    void DrawText(const TextDraw& draw);

    float MeasureText(const TextSpan& span);
    const float* MeasureAdvances(const TextSpan& span);
    float BaselineInLine(FontId font, float line_top, float line_height) const;
    float BaselineCentred(FontId font, float box_top, float box_height) const;
    float StrikeOffset(FontId font) const;
    float UnderlineOffset(FontId font) const;
    PixelSize ImageSize(ImageId image) const;
    HCURSOR Cursor(CursorId cursor) const;

private:
    bool CreateTargets();
    void DrawSlicePart(const NineSliceDraw& draw, const NineSliceAreas& areas, int part);
    void FillTiled(const NineSliceDraw& draw, const FloatRect& area, int part);
    void FillDot(PixelPoint pixel);

    ID2D1Factory1* factory = nullptr;
    IWICImagingFactory* decoder_factory = nullptr;
    ID2D1DCRenderTarget* opaque_target = nullptr;
    ID2D1DCRenderTarget* alpha_target = nullptr;
    ID2D1DeviceContext* opaque_context = nullptr;
    ID2D1DeviceContext* alpha_context = nullptr;
    ID2D1RenderTarget* current_target = nullptr;
    ID2D1DeviceContext* current_context = nullptr;
    ID2D1SolidColorBrush* solid_brush = nullptr;
    ID2D1Layer* opacity_layer = nullptr;
    bool com_started = false;
    ParchmentImages images;
    ParchmentFonts fonts;
};
