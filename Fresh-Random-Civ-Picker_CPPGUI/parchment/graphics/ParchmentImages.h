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

// A file of the image folder, by its path there (L"civ\\aztecs.png").
struct ImageFile
{
    static constexpr int MAX_PATH_LENGTH = 64;

    wchar_t path[MAX_PATH_LENGTH];
};

// The files an atlas is built from, one per cell in the atlas's order.
struct AtlasFiles
{
    const ImageFile* files;
    int count;
};

// Where the images come from: the image folder, and the icon files data.json names for the atlases (ImageId).
struct ImageSources
{
    const wchar_t* folder;
    AtlasFiles civ_icons;
    AtlasFiles dlc_icons;
    AtlasFiles edition_banners;
};

// The theme's images as Direct2D bitmaps (View, part of the Direct2D boundary). Everything is decoded and built at
// startup: the PNG files through WIC, the atlases from their icon files, the CSS-filtered variants (greyed icons,
// dimmed tabs, recoloured icons), the tiles of each nine-slice, and the cursors in both sizes. A file that can't be
// loaded leaves its image, cell or tile empty, which the renderer then skips, so a missing file costs a picture but
// not the program; the files that couldn't be loaded are counted, and the first is named.
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
    // Images decode a strip of rows at a time through the scratch buffer, so it stays small; the nine-slice
    // images, the atlas cells and the cursors are small enough to fit in it whole.
    static constexpr int MAX_SCRATCH_PIXELS = 48 * 1024;
    static constexpr int MAX_IMAGE_HEIGHT = 4096;

    enum class PixelOrder { straight, premultiplied };

    // An image file opened through WIC, converted to BGRA with straight or premultiplied alpha; Close releases it.
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

    // One cell of an atlas being built, and of its grey twin (-1 for none): its place, and the file it shows
    // (nullptr for a cell past the files).
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
