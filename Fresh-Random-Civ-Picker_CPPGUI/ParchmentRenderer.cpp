#include "ParchmentRenderer.h"

#include <cmath>
#include <d2d1_1.h>
#include <d2d1_1helper.h>
#include <dwrite.h>
#include <objbase.h>
#include <wincodec.h>

#include "Assertions.h"

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "ole32.lib")

namespace
{
    constexpr float TEXT_BOX_HEIGHT = 1000.0f;
    constexpr int MAX_DOTTED_SIDE = 2048;
    constexpr int CORNER_PARTS[] = { 0, 2, 6, 8 };

    template <typename Interface> void ReleaseInterface(Interface*& pointer)
    {
        if (pointer != nullptr) (void)pointer->Release();
        pointer = nullptr;
    }

    D2D1_RECT_F ToD2DRect(const FloatRect& rect) { return D2D1::RectF(rect.left, rect.top, rect.right, rect.bottom); }

    D2D1_COLOR_F ToD2DColour(Colour colour)
    {
        return D2D1::ColorF(colour.red / 255.0f, colour.green / 255.0f, colour.blue / 255.0f, colour.alpha);
    }

    bool IsCorner(int part)
    {
        for (int corner : CORNER_PARTS)
        {
            if (corner == part) return true;
        }
        return false;
    }

    // The tiling rule of a nine-slice part along x and y: edges tile along their length, the middle both ways.
    struct PartRules
    {
        TileRule horizontal;
        TileRule vertical;
    };

    PartRules RulesOf(int part, TileRule edges, TileRule middle)
    {
        if (IsCorner(part)) return { TileRule::stretch, TileRule::stretch };
        if (part == 1 || part == 7) return { edges, TileRule::stretch };
        if (part == 3 || part == 5) return { TileRule::stretch, edges };
        return { middle, middle };
    }
}

bool ParchmentRenderer::Start(const wchar_t* image_folder)
{
    if (!c_assert(image_folder != nullptr) || !c_assert(factory == nullptr)) return false;
    const HRESULT com_result = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    com_started = SUCCEEDED(com_result);
    D2D1_FACTORY_OPTIONS options = {};
    HRESULT result = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory1), &options,
                                       reinterpret_cast<void**>(&factory));
    if (SUCCEEDED(result))
        result = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, __uuidof(IWICImagingFactory),
                                  reinterpret_cast<void**>(&decoder_factory));
    if (!c_assert(SUCCEEDED(result)) || !CreateTargets()) return false;

    // Images are checked one by one as they load; a missing one is drawn as nothing, so startup goes on.
    (void)images.Load({ image_folder, opaque_target, decoder_factory });
    return fonts.Load();
}

// Two software DC render targets share every resource: an opaque one for windows (ClearType text) and one
// with premultiplied alpha for layered windows.
bool ParchmentRenderer::CreateTargets()
{
    const D2D1_RENDER_TARGET_PROPERTIES opaque_properties = D2D1::RenderTargetProperties(
        D2D1_RENDER_TARGET_TYPE_SOFTWARE, D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE));
    const D2D1_RENDER_TARGET_PROPERTIES alpha_properties = D2D1::RenderTargetProperties(
        D2D1_RENDER_TARGET_TYPE_SOFTWARE, D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));
    HRESULT result = factory->CreateDCRenderTarget(&opaque_properties, &opaque_target);
    if (SUCCEEDED(result)) result = factory->CreateDCRenderTarget(&alpha_properties, &alpha_target);
    if (SUCCEEDED(result))
        result = opaque_target->QueryInterface(__uuidof(ID2D1DeviceContext), reinterpret_cast<void**>(&opaque_context));
    if (SUCCEEDED(result))
        result = alpha_target->QueryInterface(__uuidof(ID2D1DeviceContext), reinterpret_cast<void**>(&alpha_context));
    if (SUCCEEDED(result)) result = opaque_target->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f), &solid_brush);
    if (SUCCEEDED(result)) result = opaque_target->CreateLayer(&opacity_layer);
    if (!c_assert(SUCCEEDED(result))) return false;
    opaque_target->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);
    alpha_target->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
    return true;
}

// Lets go of an image only needed while the windows were built.
void ParchmentRenderer::ForgetImage(ImageId image)
{
    if (!c_assert(current_target == nullptr) || !c_assert(static_cast<int>(image) < IMAGE_COUNT)) return;
    images.Forget(image);
}

void ParchmentRenderer::Stop()
{
    images.Release();
    fonts.Release();
    ReleaseInterface(opacity_layer);
    ReleaseInterface(solid_brush);
    ReleaseInterface(alpha_context);
    ReleaseInterface(opaque_context);
    ReleaseInterface(alpha_target);
    ReleaseInterface(opaque_target);
    ReleaseInterface(decoder_factory);
    ReleaseInterface(factory);
    if (com_started) CoUninitialize();
    com_started = false;
}

bool ParchmentRenderer::BeginFrame(const FrameTarget& frame)
{
    if (!c_assert(current_target == nullptr) || !c_assert(frame.dc != NULL)) return false;
    if (!c_assert(opaque_target != nullptr && alpha_target != nullptr)) return false;
    const bool opaque = frame.alpha == SurfaceAlpha::opaque;
    ID2D1DCRenderTarget* target = opaque ? opaque_target : alpha_target;
    const RECT bounds = { frame.origin.x, frame.origin.y, frame.origin.x + frame.size.width,
                          frame.origin.y + frame.size.height };
    if (!c_assert(SUCCEEDED(target->BindDC(frame.dc, &bounds)))) return false;
    current_target = target;
    current_context = opaque ? opaque_context : alpha_context;
    current_target->BeginDraw();
    current_target->SetTransform(D2D1::Matrix3x2F::Identity());
    return true;
}

bool ParchmentRenderer::EndFrame()
{
    if (!c_assert(current_target != nullptr)) return false;
    const HRESULT result = current_target->EndDraw();
    current_target = nullptr;
    current_context = nullptr;
    return c_assert(SUCCEEDED(result));
}

// Moves where (0, 0) is drawn in the frame, for drawing several controls in one frame.
void ParchmentRenderer::SetOrigin(PixelPoint origin)
{
    if (!c_assert(current_target != nullptr)) return;
    current_target->SetTransform(
        D2D1::Matrix3x2F::Translation(static_cast<float>(origin.x), static_cast<float>(origin.y)));
}

void ParchmentRenderer::PushClip(const FloatRect& area)
{
    if (!c_assert(current_target != nullptr) || !c_assert(area.right >= area.left)) return;
    current_target->PushAxisAlignedClip(ToD2DRect(area), D2D1_ANTIALIAS_MODE_ALIASED);
}

void ParchmentRenderer::PopClip()
{
    if (!c_assert(current_target != nullptr)) return;
    current_target->PopAxisAlignedClip();
}

// Draws everything until PopOpacity at this opacity, as CSS opacity does for a disabled control.
void ParchmentRenderer::PushOpacity(float opacity)
{
    if (!c_assert(current_target != nullptr && opacity_layer != nullptr)) return;
    if (!c_assert(opacity >= 0.0f && opacity <= 1.0f))
        return;
    current_target->PushLayer(D2D1::LayerParameters(D2D1::InfiniteRect(), nullptr, D2D1_ANTIALIAS_MODE_ALIASED,
                                                    D2D1::IdentityMatrix(), opacity),
                              opacity_layer);
}

void ParchmentRenderer::PopOpacity()
{
    if (!c_assert(current_target != nullptr)) return;
    current_target->PopLayer();
}

void ParchmentRenderer::DrawImage(const ImageDraw& draw)
{
    if (!c_assert(current_context != nullptr) || !c_assert(draw.opacity >= 0.0f && draw.opacity <= 1.0f)) return;
    ID2D1Bitmap* bitmap = images.Bitmap(draw.image);
    if (bitmap == nullptr) return;
    const D2D1_RECT_F target = ToD2DRect(draw.target);
    const D2D1_RECT_F source = ToD2DRect(ToFloatRect(draw.source));
    const bool smooth = draw.smoothing == Smoothing::smooth;
    const D2D1_INTERPOLATION_MODE mode =
        smooth ? D2D1_INTERPOLATION_MODE_HIGH_QUALITY_CUBIC : D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR;
    current_context->DrawBitmap(bitmap, &target, draw.opacity, mode, &source, nullptr);
}

void ParchmentRenderer::DrawAtlasCell(const AtlasDraw& draw)
{
    if (!c_assert(draw.cell >= 0) || !c_assert(draw.columns > 0)) return;
    const int column = draw.cell % draw.columns;
    const int row = draw.cell / draw.columns;
    const PixelRect source = { column * draw.cell_size.width, row * draw.cell_size.height,
                               (column + 1) * draw.cell_size.width, (row + 1) * draw.cell_size.height };
    DrawImage({ draw.atlas, source, draw.target, 1.0f, Smoothing::smooth });
}

void ParchmentRenderer::DrawNineSlice(const NineSliceDraw& draw)
{
    if (!c_assert(current_context != nullptr) || !c_assert(draw.border >= 0.0f)) return;
    const PixelRect target = { static_cast<int>(draw.target.left), static_cast<int>(draw.target.top),
                               static_cast<int>(draw.target.right), static_cast<int>(draw.target.bottom) };
    const NineSliceAreas areas = SplitNineSlice(target, static_cast<int>(draw.border));
    const int middle_part = NINE_SLICE_PARTS / 2;
    for (int part = 0; part < NINE_SLICE_PARTS; part++)
    {
        if (part != middle_part || draw.fill_middle) DrawSlicePart(draw, areas, part);
    }
}

void ParchmentRenderer::DrawSlicePart(const NineSliceDraw& draw, const NineSliceAreas& areas, int part)
{
    const PixelRect& area = areas.area[part];
    if (RectWidth(area) <= 0 || RectHeight(area) <= 0) return;
    const PartRules rules = RulesOf(part, draw.edges, draw.middle);
    if (rules.horizontal != TileRule::stretch || rules.vertical != TileRule::stretch)
    {
        FillTiled(draw, ToFloatRect(area), part);
        return;
    }
    ID2D1Bitmap* slice = images.Slice(draw.nine_slice, part);
    if (slice == nullptr) return;
    const D2D1_RECT_F target = ToD2DRect(ToFloatRect(area));
    current_context->DrawBitmap(slice, &target, 1.0f, D2D1_INTERPOLATION_MODE_HIGH_QUALITY_CUBIC, nullptr, nullptr);
}

// Tiles one nine-slice part over its area: the tile keeps the slice's proportions at the border's scale
// (CSS border-image), and the tiling starts where LayOutTiles puts it.
void ParchmentRenderer::FillTiled(const NineSliceDraw& draw, const FloatRect& area, int part)
{
    ID2D1Bitmap* slice = images.Slice(draw.nine_slice, part);
    ID2D1BitmapBrush* brush = images.SliceBrush(draw.nine_slice, part);
    const float slice_size = static_cast<float>(images.SliceSize(draw.nine_slice));
    if (slice == nullptr || brush == nullptr || !c_assert(slice_size > 0.0f)) return;
    const D2D1_SIZE_F slice_pixels = slice->GetSize();
    const float scale = draw.border / slice_size;
    const PartRules rules = RulesOf(part, draw.edges, draw.middle);
    const int area_width = static_cast<int>(area.right - area.left);
    const int area_height = static_cast<int>(area.bottom - area.top);
    const TileSpan across = LayOutTiles({ area_width, slice_pixels.width * scale, rules.horizontal });
    const TileSpan down = LayOutTiles({ area_height, slice_pixels.height * scale, rules.vertical });
    brush->SetTransform(D2D1::Matrix3x2F::Scale(across.length / slice_pixels.width, down.length / slice_pixels.height) *
                        D2D1::Matrix3x2F::Translation(area.left + across.origin, area.top + down.origin));
    current_target->FillRectangle(ToD2DRect(area), brush);
}

// Clears the whole frame to transparent, for an image built from scratch on a reused surface.
void ParchmentRenderer::Clear()
{
    if (!c_assert(current_target != nullptr)) return;
    current_target->Clear(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.0f));
}

void ParchmentRenderer::FillRoundedRect(const FloatRect& area, float radius, Colour colour)
{
    if (!c_assert(current_target != nullptr) || !c_assert(radius >= 0.0f)) return;
    solid_brush->SetColor(ToD2DColour(colour));
    current_target->FillRoundedRectangle(D2D1::RoundedRect(ToD2DRect(area), radius, radius), solid_brush);
}

void ParchmentRenderer::FillRect(const FloatRect& area, Colour colour)
{
    if (!c_assert(current_target != nullptr) || !c_assert(solid_brush != nullptr)) return;
    solid_brush->SetColor(ToD2DColour(colour));
    current_target->FillRectangle(ToD2DRect(area), solid_brush);
}

// outline: 1px dotted on the outermost pixels of the area, dot by dot where Chrome puts them.
void ParchmentRenderer::DrawDottedFrame(const FloatRect& area, Colour colour)
{
    if (!c_assert(current_target != nullptr) || !c_assert(area.right > area.left && area.bottom > area.top)) return;
    const PixelRect frame = { static_cast<int>(std::lround(area.left)), static_cast<int>(std::lround(area.top)),
                              static_cast<int>(std::lround(area.right)), static_cast<int>(std::lround(area.bottom)) };
    const int width = RectWidth(frame);
    const int height = RectHeight(frame);
    if (!c_assert(width <= MAX_DOTTED_SIDE && height <= MAX_DOTTED_SIDE)) return;
    solid_brush->SetColor(ToD2DColour(colour));
    for (int i = 0; i < width && i < MAX_DOTTED_SIDE; i++)
    {
        if (!DottedLineHasDot(i, width)) continue;
        FillDot({ frame.left + i, frame.top });
        FillDot({ frame.left + i, frame.bottom - 1 });
    }
    for (int i = 0; i < height && i < MAX_DOTTED_SIDE; i++)
    {
        if (!DottedLineHasDot(i, height)) continue;
        FillDot({ frame.left, frame.top + i });
        FillDot({ frame.right - 1, frame.top + i });
    }
}

void ParchmentRenderer::FillDot(PixelPoint pixel)
{
    const float x = static_cast<float>(pixel.x);
    const float y = static_cast<float>(pixel.y);
    current_target->FillRectangle(D2D1::RectF(x, y, x + 1.0f, y + 1.0f), solid_brush);
}

// Draws one line of text whose baseline lies at draw.baseline, aligned between draw.left and draw.right.
void ParchmentRenderer::DrawText(const TextDraw& draw)
{
    if (!c_assert(current_target != nullptr) || !c_assert(draw.span.text != nullptr)) return;
    if (!c_assert(draw.span.length >= 0) || draw.span.length == 0) return;
    IDWriteTextFormat* format = fonts.Format(draw.span.font, draw.align);
    if (!c_assert(format != nullptr)) return;
    solid_brush->SetColor(ToD2DColour(draw.colour));
    const float top = draw.baseline - fonts.Ascent(draw.span.font);
    const D2D1_RECT_F box = D2D1::RectF(draw.left, top, draw.right, top + TEXT_BOX_HEIGHT);
    current_target->DrawText(draw.span.text, static_cast<UINT32>(draw.span.length), format, box, solid_brush,
                             D2D1_DRAW_TEXT_OPTIONS_NONE, DWRITE_MEASURING_MODE_NATURAL);
}

float ParchmentRenderer::MeasureText(const TextSpan& span) { return fonts.Measure(span); }

const float* ParchmentRenderer::MeasureAdvances(const TextSpan& span) { return fonts.Advances(span); }

// The baseline of a line box (CSS line-height): half the leading above the font's rounded ascent.
float ParchmentRenderer::BaselineInLine(FontId font, float line_top, float line_height) const
{
    const float ascent = fonts.LayoutAscent(font);
    const float content = ascent + fonts.LayoutDescent(font);
    return line_top + (line_height - content) / 2.0f + ascent;
}

// The baseline of one line with line-height normal, centred in a box, as in a button.
float ParchmentRenderer::BaselineCentred(FontId font, float box_top, float box_height) const
{
    const float ascent = fonts.LayoutAscent(font);
    const float content = ascent + fonts.LayoutDescent(font);
    return box_top + (box_height - content) / 2.0f + ascent;
}

float ParchmentRenderer::StrikeOffset(FontId font) const { return fonts.StrikeOffset(font); }

float ParchmentRenderer::UnderlineOffset(FontId font) const { return fonts.UnderlineOffset(font); }

PixelSize ParchmentRenderer::ImageSize(ImageId image) const { return images.ImageSize(image); }

HCURSOR ParchmentRenderer::Cursor(CursorId cursor) const { return images.Cursor(cursor); }
