#pragma once

#include <cstdint>

#include "framework.h"
#include "ParchmentGeometry.h"
#include "ParchmentTheme.h"

struct ID2D1Bitmap;
struct ID2D1BitmapBrush;
struct ID2D1RenderTarget;
struct IWICBitmapDecoder;
struct IWICBitmapFrameDecode;
struct IWICFormatConverter;
struct IWICImagingFactory;

// The theme's images as Direct2D bitmaps (View, part of the Direct2D boundary). Everything is decoded and built at
// startup: the PNG files through WIC, the CSS-filtered variants (greyed civ icons, dimmed tabs, recoloured icons),
// the tiles of each nine-slice image, and the two cursors. A file that can't be loaded leaves its image empty,
// which the renderer then skips, so a missing images folder costs pictures but not the program.
class ParchmentImages final
{
public:
    struct LoadRequest
    {
        const wchar_t* image_folder;
        ID2D1RenderTarget* creator;
        IWICImagingFactory* decoder_factory;
    };

    bool Load(const LoadRequest& request);
    void Release();
    void Forget(ImageId image);

    ID2D1Bitmap* Bitmap(ImageId image) const;
    PixelSize ImageSize(ImageId image) const;
    ID2D1Bitmap* Slice(NineSliceId nine_slice, int part) const;
    ID2D1BitmapBrush* SliceBrush(NineSliceId nine_slice, int part) const;
    int SliceSize(NineSliceId nine_slice) const;
    HCURSOR Cursor(CursorId cursor) const;

private:
    // Images decode a strip of rows at a time through the scratch buffer, so it stays small; the nine-slice
    // images and the cursors are small enough to fit in it whole.
    static constexpr int MAX_SCRATCH_PIXELS = 48 * 1024;
    static constexpr int MAX_IMAGE_HEIGHT = 4096;

    // An image file opened through WIC, converted to straight BGRA; Close releases it.
    struct OpenedImage
    {
        IWICBitmapDecoder* decoder;
        IWICBitmapFrameDecode* frame;
        IWICFormatConverter* converter;
        PixelSize size;
    };

    struct SlicePart
    {
        int nine_slice;
        int part;
        PixelRect area;
        int image_width;
    };

    OpenedImage Open(const LoadRequest& request, const wchar_t* file) const;
    static void Close(const OpenedImage& image);
    bool DecodeWhole(const OpenedImage& image);
    bool CreateImageBitmap(const LoadRequest& request, int image_index);
    bool FillBitmap(const OpenedImage& image, int image_index);
    bool CreateSlices(const LoadRequest& request, NineSliceId nine_slice, PixelSize image_size);
    bool CreateSlice(const LoadRequest& request, const SlicePart& slice_part);
    bool CreateGameCursor(const LoadRequest& request, CursorId cursor);

    ID2D1Bitmap* bitmaps[IMAGE_COUNT] = {};
    PixelSize sizes[IMAGE_COUNT] = {};
    ID2D1Bitmap* slices[NINE_SLICE_COUNT][NINE_SLICE_PARTS] = {};
    ID2D1BitmapBrush* slice_brushes[NINE_SLICE_COUNT][NINE_SLICE_PARTS] = {};
    HCURSOR cursors[CURSOR_COUNT] = {};
    uint32_t scratch[MAX_SCRATCH_PIXELS] = {};
};
