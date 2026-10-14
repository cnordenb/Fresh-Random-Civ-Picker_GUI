#include "ParchmentImages.h"

#include <d2d1.h>
#include <wincodec.h>

#include "Assertions.h"

namespace
{
    // CSS filters some images get before use (style.css): see ParchmentGeometry for the colour arithmetic.
    enum class Filter { none, grayscale, dim_tab, black, tooltip_tint };

    struct ImageSource
    {
        ImageId image;
        const wchar_t* file;
        Filter filter;
    };

    const ImageSource IMAGE_SOURCES[IMAGE_COUNT] = {
        { ImageId::background, L"ui\\background.png", Filter::none },
        { ImageId::text_button, L"ui\\button.png", Filter::none },
        { ImageId::text_button_pressed, L"ui\\button_pressed.png", Filter::none },
        { ImageId::icon_button_frame, L"ui\\button_9slice.png", Filter::none },
        { ImageId::icon_button_frame_pressed, L"ui\\button_pressed_9slice.png", Filter::none },
        { ImageId::checkbox, L"ui\\checkbox.png", Filter::none },
        { ImageId::dropdown_button, L"ui\\dropdown_button.png", Filter::none },
        { ImageId::external_icon, L"ui\\external.png", Filter::none },
        { ImageId::external_icon_dark, L"ui\\external.png", Filter::black },
        { ImageId::external_icon_tooltip, L"ui\\external.png", Filter::tooltip_tint },
        { ImageId::log_frame, L"ui\\log_9slice.png", Filter::none },
        { ImageId::off_cross, L"ui\\off.png", Filter::none },
        { ImageId::options_button, L"ui\\options_button.png", Filter::none },
        { ImageId::panel_frame, L"ui\\panel_9slice.png", Filter::none },
        { ImageId::scrollbar, L"ui\\scrollbar_3slice.png", Filter::none },
        { ImageId::scrollbar_knob, L"ui\\scrollbar_knob.png", Filter::none },
        { ImageId::slider, L"ui\\slider.png", Filter::none },
        { ImageId::slider_knob, L"ui\\slider_knob.png", Filter::none },
        { ImageId::tabs, L"ui\\tabs.png", Filter::none },
        { ImageId::tabs_dimmed, L"ui\\tabs.png", Filter::dim_tab },
        { ImageId::techtree_button, L"ui\\techtree_button.png", Filter::none },
        { ImageId::text_field_frame, L"ui\\textfield_9slice.png", Filter::none },
        { ImageId::close_button, L"ui\\x_button.png", Filter::none },
        { ImageId::share_icon, L"ui\\share_icon.png", Filter::none },
        { ImageId::civ_atlas, L"civ\\civicon_atlas.png", Filter::none },
        { ImageId::civ_atlas_grey, L"civ\\civicon_atlas.png", Filter::grayscale },
        { ImageId::dlc_atlas, L"dlc\\dlc_atlas.png", Filter::none },
        { ImageId::dlc_atlas_grey, L"dlc\\dlc_atlas.png", Filter::grayscale },
        { ImageId::edition_banners, L"dlc\\edition_banners_atlas.png", Filter::none },
        { ImageId::history_book, L"history.bmp", Filter::none },
        { ImageId::aoe2vils_icon, L"aoe2vils.png", Filter::none },
    };

    struct NineSliceSource
    {
        NineSliceId nine_slice;
        ImageId image;
        int slice;
    };

    const NineSliceSource NINE_SLICE_SOURCES[NINE_SLICE_COUNT] = {
        { NineSliceId::log_frame, ImageId::log_frame, 6 },
        { NineSliceId::icon_button_frame, ImageId::icon_button_frame, 6 },
        { NineSliceId::icon_button_frame_pressed, ImageId::icon_button_frame_pressed, 6 },
        { NineSliceId::panel_frame, ImageId::panel_frame, 64 },
        { NineSliceId::text_field_frame, ImageId::text_field_frame, 15 },
    };

    struct CursorSource
    {
        CursorId cursor;
        const wchar_t* file;
        DWORD hotspot_x;
        DWORD hotspot_y;
    };

    // The game's cursors and their hotspots (style.css: --arrow 0 0, --hand 1 0).
    const CursorSource CURSOR_SOURCES[CURSOR_COUNT] = {
        { CursorId::arrow, L"ui\\cursor.png", 0, 0 },
        { CursorId::hand, L"ui\\cursor_pointing.png", 1, 0 },
    };

    constexpr int MAX_PATH_LENGTH = 512;

    Rgb ApplyFilter(const Rgb& colour, Filter filter)
    {
        switch (filter)
        {
            case Filter::grayscale: return Grayscale(colour);
            case Filter::dim_tab: return Brightness(colour, 0.88f);
            case Filter::black: return Brightness(colour, 0.0f);
            case Filter::tooltip_tint: return Saturate(HueRotate(Brightness(colour, 0.15f), -54.0f), 3.1f);
            case Filter::none: return colour;
        }
        return colour;
    }

    uint32_t FilterPixel(uint32_t pixel, Filter filter)
    {
        const Rgb colour = { static_cast<uint8_t>(pixel >> 16), static_cast<uint8_t>(pixel >> 8),
                             static_cast<uint8_t>(pixel) };
        const Rgb filtered = ApplyFilter(colour, filter);
        return (pixel & 0xff000000u) | (static_cast<uint32_t>(filtered.red) << 16) |
               (static_cast<uint32_t>(filtered.green) << 8) | filtered.blue;
    }

    uint32_t PremultiplyPixel(uint32_t pixel)
    {
        const uint32_t alpha = pixel >> 24;
        const uint32_t red = ((pixel >> 16) & 0xff) * alpha / 255;
        const uint32_t green = ((pixel >> 8) & 0xff) * alpha / 255;
        const uint32_t blue = (pixel & 0xff) * alpha / 255;
        return (alpha << 24) | (red << 16) | (green << 8) | blue;
    }

    D2D1_BITMAP_PROPERTIES PremultipliedProperties()
    {
        return D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));
    }

    template <typename Interface> void ReleaseInterface(Interface*& pointer)
    {
        if (pointer != nullptr) (void)pointer->Release();
        pointer = nullptr;
    }
}

bool ParchmentImages::Load(const LoadRequest& request)
{
    if (!c_assert(request.image_folder != nullptr) || !c_assert(request.creator != nullptr)) return false;
    if (!c_assert(request.decoder_factory != nullptr)) return false;
    bool all_loaded = true;
    for (int i = 0; i < IMAGE_COUNT; i++)
    {
        if (!c_assert(static_cast<int>(IMAGE_SOURCES[i].image) == i)) return false;
        all_loaded = CreateImageBitmap(request, i) && all_loaded;
    }
    for (const CursorSource& source : CURSOR_SOURCES)
    {
        all_loaded = CreateGameCursor(request, source.cursor) && all_loaded;
    }
    return all_loaded;
}

// Opens a PNG or BMP file of the image folder through WIC, converted to straight (unpremultiplied) BGRA. A file
// that can't be opened gives an image without a converter.
ParchmentImages::OpenedImage ParchmentImages::Open(const LoadRequest& request, const wchar_t* file) const
{
    OpenedImage image = { nullptr, nullptr, nullptr, { 0, 0 } };
    wchar_t path[MAX_PATH_LENGTH] = L"";
    if (!c_assert(file != nullptr) || !c_assert(request.decoder_factory != nullptr)) return image;
    if (wcscpy_s(path, request.image_folder) != 0 || wcscat_s(path, file) != 0) return image;
    UINT width = 0;
    UINT height = 0;
    HRESULT result = request.decoder_factory->CreateDecoderFromFilename(path, nullptr, GENERIC_READ,
                                                                        WICDecodeMetadataCacheOnDemand, &image.decoder);
    if (SUCCEEDED(result)) result = image.decoder->GetFrame(0, &image.frame);
    if (SUCCEEDED(result)) result = request.decoder_factory->CreateFormatConverter(&image.converter);
    if (SUCCEEDED(result))
        result = image.converter->Initialize(image.frame, GUID_WICPixelFormat32bppBGRA, WICBitmapDitherTypeNone,
                                             nullptr, 0.0, WICBitmapPaletteTypeCustom);
    if (SUCCEEDED(result)) result = image.converter->GetSize(&width, &height);
    const bool usable = SUCCEEDED(result) && width > 0 && height > 0 && width <= MAX_SCRATCH_PIXELS &&
                        height <= MAX_IMAGE_HEIGHT;
    if (!c_assert(usable)) ReleaseInterface(image.converter);
    image.size = { static_cast<int>(width), static_cast<int>(height) };
    return image;
}

void ParchmentImages::Close(const OpenedImage& image)
{
    if (image.converter != nullptr) (void)image.converter->Release();
    if (image.frame != nullptr) (void)image.frame->Release();
    if (image.decoder != nullptr) (void)image.decoder->Release();
}

// Decodes a small image whole into the scratch buffer.
bool ParchmentImages::DecodeWhole(const OpenedImage& image)
{
    if (!c_assert(image.converter != nullptr)) return false;
    const PixelSize size = image.size;
    if (!c_assert(size.width * size.height <= MAX_SCRATCH_PIXELS)) return false;
    const UINT stride = static_cast<UINT>(size.width) * 4;
    const HRESULT result = image.converter->CopyPixels(nullptr, stride, stride * static_cast<UINT>(size.height),
                                                       reinterpret_cast<BYTE*>(scratch));
    return c_assert(SUCCEEDED(result));
}

bool ParchmentImages::CreateImageBitmap(const LoadRequest& request, int image_index)
{
    if (!c_assert(image_index >= 0 && image_index < IMAGE_COUNT)) return false;
    const OpenedImage image = Open(request, IMAGE_SOURCES[image_index].file);
    const D2D1_BITMAP_PROPERTIES properties = PremultipliedProperties();
    const D2D1_SIZE_U size = D2D1::SizeU(static_cast<UINT32>(image.size.width), static_cast<UINT32>(image.size.height));
    HRESULT result = image.converter != nullptr ? S_OK : E_FAIL;
    if (SUCCEEDED(result)) result = request.creator->CreateBitmap(size, nullptr, 0, &properties, &bitmaps[image_index]);
    const bool filled = c_assert(SUCCEEDED(result)) && FillBitmap(image, image_index);
    Close(image);
    if (!filled) return false;
    sizes[image_index] = image.size;

    for (const NineSliceSource& nine_slice : NINE_SLICE_SOURCES)
    {
        if (static_cast<int>(nine_slice.image) != image_index) continue;
        // A nine-slice image fills its bitmap in one strip, so the scratch buffer still holds all of it.
        if (!c_assert(image.size.width * image.size.height <= MAX_SCRATCH_PIXELS)) return false;
        if (!CreateSlices(request, nine_slice.nine_slice, image.size)) return false;
    }
    return true;
}

// Copies an image into its bitmap a strip of rows at a time, filtered and premultiplied on the way.
bool ParchmentImages::FillBitmap(const OpenedImage& image, int image_index)
{
    if (!c_assert(image.converter != nullptr) || !c_assert(bitmaps[image_index] != nullptr)) return false;
    const PixelSize size = image.size;
    const Filter filter = IMAGE_SOURCES[image_index].filter;
    const int strip_rows = MAX_SCRATCH_PIXELS / size.width;
    const UINT stride = static_cast<UINT>(size.width) * 4;
    for (int top = 0; top < size.height && top < MAX_IMAGE_HEIGHT; top += strip_rows)
    {
        const int rows = size.height - top < strip_rows ? size.height - top : strip_rows;
        const WICRect strip = { 0, top, size.width, rows };
        HRESULT result = image.converter->CopyPixels(&strip, stride, stride * static_cast<UINT>(rows),
                                                     reinterpret_cast<BYTE*>(scratch));
        if (!c_assert(SUCCEEDED(result))) return false;
        const int pixel_count = size.width * rows;
        for (int i = 0; i < pixel_count && i < MAX_SCRATCH_PIXELS; i++)
        {
            scratch[i] = PremultiplyPixel(FilterPixel(scratch[i], filter));
        }
        const D2D1_RECT_U target = { 0, static_cast<UINT32>(top), static_cast<UINT32>(size.width),
                                     static_cast<UINT32>(top + rows) };
        result = bitmaps[image_index]->CopyFromMemory(&target, scratch, stride);
        if (!c_assert(SUCCEEDED(result))) return false;
    }
    return true;
}

// Cuts the nine tiles of a nine-slice image out of the scratch buffer, each with a brush that tiles it.
bool ParchmentImages::CreateSlices(const LoadRequest& request, NineSliceId nine_slice, PixelSize image_size)
{
    const int index = static_cast<int>(nine_slice);
    if (!c_assert(index >= 0 && index < NINE_SLICE_COUNT)) return false;
    const int slice = NINE_SLICE_SOURCES[index].slice;
    if (!c_assert(2 * slice < image_size.width && 2 * slice < image_size.height)) return false;
    const NineSliceAreas parts = SplitNineSlice({ 0, 0, image_size.width, image_size.height }, slice);
    for (int part = 0; part < NINE_SLICE_PARTS; part++)
    {
        if (!CreateSlice(request, { index, part, parts.area[part], image_size.width })) return false;
    }
    return true;
}

bool ParchmentImages::CreateSlice(const LoadRequest& request, const SlicePart& slice_part)
{
    const PixelRect& area = slice_part.area;
    if (!c_assert(slice_part.part >= 0 && slice_part.part < NINE_SLICE_PARTS)) return false;
    if (!c_assert(RectWidth(area) > 0 && RectHeight(area) > 0)) return false;
    const uint32_t* first_pixel = scratch + area.top * slice_part.image_width + area.left;
    const D2D1_BITMAP_PROPERTIES properties = PremultipliedProperties();
    ID2D1Bitmap*& slice = slices[slice_part.nine_slice][slice_part.part];
    HRESULT result = request.creator->CreateBitmap(
        D2D1::SizeU(static_cast<UINT32>(RectWidth(area)), static_cast<UINT32>(RectHeight(area))), first_pixel,
        static_cast<UINT32>(slice_part.image_width) * 4, &properties, &slice);
    if (!c_assert(SUCCEEDED(result))) return false;
    const D2D1_BITMAP_BRUSH_PROPERTIES brush_properties =
        D2D1::BitmapBrushProperties(D2D1_EXTEND_MODE_WRAP, D2D1_EXTEND_MODE_WRAP,
                                    D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
    result = request.creator->CreateBitmapBrush(slice, brush_properties,
                                                &slice_brushes[slice_part.nine_slice][slice_part.part]);
    return c_assert(SUCCEEDED(result));
}

// A cursor needs straight alpha in a DIB plus an empty mask (CreateIconIndirect).
bool ParchmentImages::CreateGameCursor(const LoadRequest& request, CursorId cursor)
{
    const CursorSource& source = CURSOR_SOURCES[static_cast<int>(cursor)];
    if (!c_assert(source.cursor == cursor)) return false;
    const OpenedImage image = Open(request, source.file);
    const bool decoded_whole = image.converter != nullptr && DecodeWhole(image);
    Close(image);
    if (!decoded_whole) return false;
    const PixelSize size = image.size;

    BITMAPV5HEADER header = {};
    header.bV5Size = sizeof(header);
    header.bV5Width = size.width;
    header.bV5Height = -size.height;
    header.bV5Planes = 1;
    header.bV5BitCount = 32;
    header.bV5Compression = BI_RGB;
    void* bits = nullptr;
    HBITMAP colour = CreateDIBSection(NULL, reinterpret_cast<const BITMAPINFO*>(&header), DIB_RGB_COLORS, &bits,
                                      NULL, 0);
    HBITMAP mask = CreateBitmap(size.width, size.height, 1, 1, nullptr);
    if (colour != NULL && bits != nullptr && mask != NULL)
    {
        memcpy(bits, scratch, static_cast<size_t>(size.width) * static_cast<size_t>(size.height) * 4);
        ICONINFO icon = { FALSE, source.hotspot_x, source.hotspot_y, mask, colour };
        cursors[static_cast<int>(cursor)] = CreateIconIndirect(&icon);
    }
    // CreateIconIndirect copies both bitmaps, so they are freed either way; a failure leaves the system cursor.
    if (colour != NULL) (void)DeleteObject(colour);
    if (mask != NULL) (void)DeleteObject(mask);
    return c_assert(cursors[static_cast<int>(cursor)] != NULL);
}

void ParchmentImages::Release()
{
    for (int i = 0; i < IMAGE_COUNT; i++) ReleaseInterface(bitmaps[i]);
    for (int i = 0; i < NINE_SLICE_COUNT; i++)
    {
        for (int part = 0; part < NINE_SLICE_PARTS; part++)
        {
            ReleaseInterface(slice_brushes[i][part]);
            ReleaseInterface(slices[i][part]);
        }
    }
    for (int i = 0; i < CURSOR_COUNT; i++)
    {
        // Destroying a cursor at shutdown: a failure leaves nothing to recover.
        if (cursors[i] != NULL) (void)DestroyCursor(cursors[i]);
        cursors[i] = NULL;
    }
}

// Lets go of an image only needed at startup (the main window's parchment); drawing it later draws nothing.
void ParchmentImages::Forget(ImageId image)
{
    const int index = static_cast<int>(image);
    if (!c_assert(index >= 0 && index < IMAGE_COUNT)) return;
    ReleaseInterface(bitmaps[index]);
}

ID2D1Bitmap* ParchmentImages::Bitmap(ImageId image) const
{
    const int index = static_cast<int>(image);
    if (!c_assert(index >= 0 && index < IMAGE_COUNT)) return nullptr;
    return bitmaps[index];
}

PixelSize ParchmentImages::ImageSize(ImageId image) const
{
    const int index = static_cast<int>(image);
    if (!c_assert(index >= 0 && index < IMAGE_COUNT)) return { 0, 0 };
    return sizes[index];
}

ID2D1Bitmap* ParchmentImages::Slice(NineSliceId nine_slice, int part) const
{
    const int index = static_cast<int>(nine_slice);
    if (!c_assert(index >= 0 && index < NINE_SLICE_COUNT) || !c_assert(part >= 0 && part < NINE_SLICE_PARTS))
        return nullptr;
    return slices[index][part];
}

ID2D1BitmapBrush* ParchmentImages::SliceBrush(NineSliceId nine_slice, int part) const
{
    const int index = static_cast<int>(nine_slice);
    if (!c_assert(index >= 0 && index < NINE_SLICE_COUNT) || !c_assert(part >= 0 && part < NINE_SLICE_PARTS))
        return nullptr;
    return slice_brushes[index][part];
}

int ParchmentImages::SliceSize(NineSliceId nine_slice) const
{
    const int index = static_cast<int>(nine_slice);
    if (!c_assert(index >= 0 && index < NINE_SLICE_COUNT)) return 0;
    return NINE_SLICE_SOURCES[index].slice;
}

HCURSOR ParchmentImages::Cursor(CursorId cursor) const
{
    const int index = static_cast<int>(cursor);
    if (!c_assert(index >= 0 && index < CURSOR_COUNT)) return NULL;
    return cursors[index];
}
