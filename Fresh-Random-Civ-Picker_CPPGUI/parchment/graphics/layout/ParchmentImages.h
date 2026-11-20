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

struct ImageFile
{
    static constexpr int MAX_PATH_LENGTH = 64;

    wchar_t path[MAX_PATH_LENGTH];
};

struct AtlasFiles
{
    const ImageFile* files;
    int count;
};

struct ImageSources
{
    const wchar_t* folder;
    AtlasFiles civ_icons;
    AtlasFiles dlc_icons;
    AtlasFiles edition_banners;
};

class ParchmentImages final
{
public:
    struct LoadRequest
    {
        ImageSources sources;
        ID2D1RenderTarget* creator;
        IWICImagingFactory* decoder_factory;
    };

    bool Load(const LoadRequest& request);
    void Release();
    void Forget(ImageId image);
    void SelectCursorSize(CursorSize size);

    ID2D1Bitmap* Bitmap(ImageId image) const;
    PixelSize ImageSize(ImageId image) const;
    ID2D1Bitmap* Slice(NineSliceId nine_slice, int part) const;
    ID2D1BitmapBrush* SliceBrush(NineSliceId nine_slice, int part) const;
    int SliceSize(NineSliceId nine_slice) const;
    HCURSOR Cursor(CursorId cursor) const;
    int MissingFileCount() const;
    const wchar_t* FirstMissingFile() const;

private:
    static constexpr int MAX_SCRATCH_PIXELS = 48 * 1024;
    static constexpr int MAX_IMAGE_HEIGHT = 4096;

    enum class PixelOrder { straight, premultiplied };

    struct OpenedImage
    {
        IWICBitmapDecoder* decoder;
        IWICBitmapFrameDecode* frame;
        IWICFormatConverter* converter;
        PixelSize size;
    };

    struct ImageOpening
    {
        const wchar_t* file;
        PixelOrder order;
    };

    struct SlicePart
    {
        int nine_slice;
        int part;
        PixelRect area;
        int image_width;
    };

    struct AtlasCell
    {
        int atlas;
        int grey_atlas;
        int cell;
        int columns;
        PixelSize size;
        const wchar_t* file;
    };

    OpenedImage Open(const LoadRequest& request, const ImageOpening& opening);
    void NoteMissing(const wchar_t* file);
    static void Close(const OpenedImage& image);
    bool DecodeWhole(const OpenedImage& image);
    bool CreateImageBitmap(const LoadRequest& request, int image_index);
    bool FillBitmap(const OpenedImage& image, int image_index);
    bool CreateSlices(const LoadRequest& request, NineSliceId nine_slice, PixelSize image_size);
    bool CreatePanelSlices(const LoadRequest& request);
    bool CreateSlice(const LoadRequest& request, const SlicePart& slice_part);
    bool BuildAtlas(const LoadRequest& request, int atlas_source);
    bool CreateAtlasBitmap(const LoadRequest& request, ImageId atlas, PixelSize size);
    bool FillAtlasCell(const LoadRequest& request, const AtlasCell& cell);
    bool DecodeIntoCell(const LoadRequest& request, const AtlasCell& cell);
    bool CopyScaled(const LoadRequest& request, const OpenedImage& image, const AtlasCell& cell);
    bool CreateGameCursor(const LoadRequest& request, int cursor_source);
    HCURSOR CreateCursorFromScratch(PixelSize size, PixelPoint hotspot) const;

    ID2D1Bitmap* bitmaps[IMAGE_COUNT] = {};
    PixelSize sizes[IMAGE_COUNT] = {};
    ID2D1Bitmap* slices[NINE_SLICE_COUNT][NINE_SLICE_PARTS] = {};
    ID2D1BitmapBrush* slice_brushes[NINE_SLICE_COUNT][NINE_SLICE_PARTS] = {};
    HCURSOR cursors[CURSOR_SIZE_COUNT][CURSOR_COUNT] = {};
    CursorSize cursor_size = CursorSize::standard;
    int missing_file_count = 0;
    wchar_t first_missing_file[ImageFile::MAX_PATH_LENGTH] = L"";
    uint32_t scratch[MAX_SCRATCH_PIXELS] = {};
};
