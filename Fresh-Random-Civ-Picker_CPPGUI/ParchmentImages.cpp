#include "ParchmentImages.h"

#include <d2d1.h>
#include <wincodec.h>

#include "Assertions.h"
#include "CivDataLimits.h"

namespace
{
    // CSS filters some images get before use (style.css): see ParchmentGeometry for the colour arithmetic.
    enum class Filter { none, grayscale, dim_tab, black, tooltip_tint };

    struct ImageSource
    {
        ImageId image;
        const wchar_t* file; // nullptr for an atlas, which is built from icon files (ATLAS_SOURCES)
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
        { ImageId::panel_inlay, L"ui\\9slice\\boxstyle3a_inlay.png", Filter::none },
        { ImageId::scrollbar, L"ui\\scrollbar_3slice.png", Filter::none },
        { ImageId::scrollbar_knob, L"ui\\scrollbar_knob.png", Filter::none },
        { ImageId::slider_bar, L"ui\\slider\\slider_bar.png", Filter::none },
        { ImageId::slider_progress, L"ui\\slider\\slider_bar_progress.png", Filter::none },
        { ImageId::slider_left_arrow, L"ui\\slider\\arrow_left_normal.png", Filter::none },
        { ImageId::slider_left_arrow_hover, L"ui\\slider\\arrow_left_hover.png", Filter::none },
        { ImageId::slider_left_arrow_active, L"ui\\slider\\arrow_left_active.png", Filter::none },
        { ImageId::slider_right_arrow, L"ui\\slider\\arrow_right_normal.png", Filter::none },
        { ImageId::slider_right_arrow_hover, L"ui\\slider\\arrow_right_hover.png", Filter::none },
        { ImageId::slider_right_arrow_active, L"ui\\slider\\arrow_right_active.png", Filter::none },
        { ImageId::slider_handle, L"ui\\slider\\slider_handle_normal.png", Filter::none },
        { ImageId::slider_handle_hover, L"ui\\slider\\slider_handle_hover.png", Filter::none },
        { ImageId::slider_handle_active, L"ui\\slider\\slider_handle_active.png", Filter::none },
        { ImageId::tabs, L"ui\\tabs.png", Filter::none },
        { ImageId::tabs_dimmed, L"ui\\tabs.png", Filter::dim_tab },
        { ImageId::techtree_button, L"ui\\techtree_button.png", Filter::none },
        { ImageId::text_field_frame, L"ui\\textfield_9slice.png", Filter::none },
        { ImageId::close_button, L"ui\\x_button.png", Filter::none },
        { ImageId::share_icon, L"ui\\share_icon.png", Filter::none },
        { ImageId::program_icon, L"ui\\frcp.png", Filter::none },
        { ImageId::civ_atlas, nullptr, Filter::none },
        { ImageId::civ_atlas_grey, nullptr, Filter::grayscale },
        { ImageId::dlc_atlas, nullptr, Filter::none },
        { ImageId::dlc_atlas_grey, nullptr, Filter::grayscale },
        { ImageId::edition_banners, nullptr, Filter::none },
        { ImageId::history_book, L"history.bmp", Filter::none },
        { ImageId::aoe2vils_icon, L"aoe2vils.png", Filter::none },
    };

    // The atlases built from the icon files data.json names: which files, the cells' layout, and the atlas of the
    // same icons greyed for disabled civs and DLCs (ImageId::count for none).
    enum class AtlasContent { civ_icons, dlc_icons, edition_banners };

    struct AtlasSource
    {
        ImageId atlas;
        ImageId grey_atlas;
        AtlasContent content;
        int columns;
        PixelSize cell;
    };

    const AtlasSource ATLAS_SOURCES[] = {
        { ImageId::civ_atlas, ImageId::civ_atlas_grey, AtlasContent::civ_icons, CIV_ATLAS_COLUMNS,
          { CIV_ATLAS_CELL, CIV_ATLAS_CELL } },
        { ImageId::dlc_atlas, ImageId::dlc_atlas_grey, AtlasContent::dlc_icons, DLC_ATLAS_COLUMNS,
          { DLC_ATLAS_CELL, DLC_ATLAS_CELL } },
        { ImageId::edition_banners, ImageId::count, AtlasContent::edition_banners, 1,
          { EDITION_BANNER_WIDTH, EDITION_BANNER_HEIGHT } },
    };
    constexpr int ATLAS_COUNT = static_cast<int>(sizeof(ATLAS_SOURCES) / sizeof(ATLAS_SOURCES[0]));
    // The most icons an atlas holds: every civ and Random, or every edition's three looks and every DLC's icon.
    constexpr int MAX_CIV_ICONS = MAX_CIVS + 1;
    constexpr int MAX_DLC_ICONS = EDITION_LOOKS * MAX_EDITIONS + MAX_DLCS;
    constexpr int MAX_ATLAS_CELLS = MAX_CIV_ICONS > MAX_DLC_ICONS ? MAX_CIV_ICONS : MAX_DLC_ICONS;

    struct NineSliceSource
    {
        NineSliceId nine_slice;
        ImageId image; // the image the tiles are cut from; ImageId::count for tiles that are files of their own
        int slice;
    };

    const NineSliceSource NINE_SLICE_SOURCES[NINE_SLICE_COUNT] = {
        { NineSliceId::log_frame, ImageId::log_frame, 6 },
        { NineSliceId::icon_button_frame, ImageId::icon_button_frame, 6 },
        { NineSliceId::icon_button_frame_pressed, ImageId::icon_button_frame_pressed, 6 },
        { NineSliceId::panel_frame, ImageId::count, 64 },
        { NineSliceId::text_field_frame, ImageId::text_field_frame, 15 },
    };

    // The dialogs' panel is the game's boxstyle3a, one 64 px file per tile, in the order of NineSliceAreas.
    const wchar_t* const PANEL_TILE_FILES[NINE_SLICE_PARTS] = {
        L"ui\\9slice\\boxstyle3a_angle_topleft.png",    L"ui\\9slice\\boxstyle3a_edge_top.png",
        L"ui\\9slice\\boxstyle3a_angle_topright.png",   L"ui\\9slice\\boxstyle3a_edge_left.png",
        L"ui\\9slice\\boxstyle3a_inlay.png",            L"ui\\9slice\\boxstyle3a_edge_right.png",
        L"ui\\9slice\\boxstyle3a_angle_bottomleft.png", L"ui\\9slice\\boxstyle3a_edge_bottom.png",
        L"ui\\9slice\\boxstyle3a_angle_bottomright.png",
    };

    struct CursorSource
    {
        CursorId cursor;
        CursorSize size;
        const wchar_t* file;
        PixelPoint hotspot;
    };

    // The game's cursors in both sizes, with their hotspots: the arrow's tip, the middle of the finger's tip.
    const CursorSource CURSOR_SOURCES[] = {
        { CursorId::arrow, CursorSize::standard, L"ui\\cursor_x1.png", { 0, 0 } },
        { CursorId::hand, CursorSize::standard, L"ui\\cursor_pointing_x1.png", { 2, 0 } },
        { CursorId::arrow, CursorSize::large, L"ui\\cursor_x2.png", { 0, 0 } },
        { CursorId::hand, CursorSize::large, L"ui\\cursor_pointing_x2.png", { 3, 0 } },
    };
    constexpr int CURSOR_SOURCE_COUNT = static_cast<int>(sizeof(CURSOR_SOURCES) / sizeof(CURSOR_SOURCES[0]));
    static_assert(CURSOR_SOURCE_COUNT == CURSOR_COUNT * CURSOR_SIZE_COUNT, "every cursor has a file in every size");

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

    // Filters a pixel's colour and keeps its alpha. Grayscale and brightness are linear, so they may also filter a
    // premultiplied pixel.
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

    AtlasFiles FilesOf(const ImageSources& sources, AtlasContent content)
    {
        switch (content)
        {
            case AtlasContent::civ_icons: return sources.civ_icons;
            case AtlasContent::dlc_icons: return sources.dlc_icons;
            case AtlasContent::edition_banners: return sources.edition_banners;
        }
        return { nullptr, 0 };
    }

    D2D1_RECT_U CellRect(int cell, int columns, PixelSize cell_size)
    {
        if (!c_assert(cell >= 0 && columns > 0)) return { 0, 0, 0, 0 };
        const UINT32 left = static_cast<UINT32>((cell % columns) * cell_size.width);
        const UINT32 top = static_cast<UINT32>((cell / columns) * cell_size.height);
        return { left, top, left + static_cast<UINT32>(cell_size.width), top + static_cast<UINT32>(cell_size.height) };
    }
}

bool ParchmentImages::Load(const LoadRequest& request)
{
    if (!c_assert(request.sources.folder != nullptr) || !c_assert(request.creator != nullptr)) return false;
    if (!c_assert(request.decoder_factory != nullptr)) return false;
    missing_file_count = 0;
    first_missing_file[0] = L'\0';
    bool all_loaded = true;
    for (int i = 0; i < IMAGE_COUNT; i++)
    {
        if (!c_assert(static_cast<int>(IMAGE_SOURCES[i].image) == i)) return false;
        if (IMAGE_SOURCES[i].file != nullptr) all_loaded = CreateImageBitmap(request, i) && all_loaded;
    }
    for (int i = 0; i < ATLAS_COUNT; i++) all_loaded = BuildAtlas(request, i) && all_loaded;
    all_loaded = CreatePanelSlices(request) && all_loaded;
    for (int i = 0; i < CURSOR_SOURCE_COUNT; i++) all_loaded = CreateGameCursor(request, i) && all_loaded;
    return all_loaded;
}

// Opens a PNG or BMP file of the image folder through WIC, converted to BGRA with straight or premultiplied alpha.
// A file that can't be opened or used gives an image without a converter, and counts as missing.
ParchmentImages::OpenedImage ParchmentImages::Open(const LoadRequest& request, const ImageOpening& opening)
{
    OpenedImage image = { nullptr, nullptr, nullptr, { 0, 0 } };
    wchar_t path[MAX_PATH_LENGTH] = L"";
    if (!c_assert(opening.file != nullptr) || !c_assert(request.decoder_factory != nullptr)) return image;
    const bool premultiplied = opening.order == PixelOrder::premultiplied;
    const WICPixelFormatGUID format = premultiplied ? GUID_WICPixelFormat32bppPBGRA : GUID_WICPixelFormat32bppBGRA;
    UINT width = 0;
    UINT height = 0;
    HRESULT result = wcscpy_s(path, request.sources.folder) == 0 && wcscat_s(path, opening.file) == 0 ? S_OK : E_FAIL;
    if (SUCCEEDED(result))
        result = request.decoder_factory->CreateDecoderFromFilename(path, nullptr, GENERIC_READ,
                                                                    WICDecodeMetadataCacheOnDemand, &image.decoder);
    if (SUCCEEDED(result)) result = image.decoder->GetFrame(0, &image.frame);
    if (SUCCEEDED(result)) result = request.decoder_factory->CreateFormatConverter(&image.converter);
    if (SUCCEEDED(result))
        result = image.converter->Initialize(image.frame, format, WICBitmapDitherTypeNone, nullptr, 0.0,
                                             WICBitmapPaletteTypeCustom);
    if (SUCCEEDED(result)) result = image.converter->GetSize(&width, &height);
    const bool usable = SUCCEEDED(result) && width > 0 && height > 0 && width <= MAX_SCRATCH_PIXELS &&
                        height <= MAX_IMAGE_HEIGHT;
    if (!usable)
    {
        NoteMissing(opening.file);
        ReleaseInterface(image.converter);
    }
    image.size = { static_cast<int>(width), static_cast<int>(height) };
    return image;
}

void ParchmentImages::NoteMissing(const wchar_t* file)
{
    if (!c_assert(file != nullptr) || !c_assert(missing_file_count >= 0)) return;
    // A path longer than the buffer is cut short, which still points to the file.
    if (missing_file_count == 0) (void)wcsncpy_s(first_missing_file, file, _TRUNCATE);
    missing_file_count++;
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
    const OpenedImage image = Open(request, { IMAGE_SOURCES[image_index].file, PixelOrder::straight });
    const D2D1_BITMAP_PROPERTIES properties = PremultipliedProperties();
    const D2D1_SIZE_U size = D2D1::SizeU(static_cast<UINT32>(image.size.width), static_cast<UINT32>(image.size.height));
    HRESULT result = image.converter != nullptr ? S_OK : E_FAIL;
    if (SUCCEEDED(result)) result = request.creator->CreateBitmap(size, nullptr, 0, &properties, &bitmaps[image_index]);
    const bool filled = SUCCEEDED(result) && FillBitmap(image, image_index);
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

// Loads the dialogs' panel, whose tiles are files of their own; a tile that can't be loaded is left out.
bool ParchmentImages::CreatePanelSlices(const LoadRequest& request)
{
    const int index = static_cast<int>(NineSliceId::panel_frame);
    if (!c_assert(NINE_SLICE_SOURCES[index].image == ImageId::count)) return false;
    bool all_loaded = true;
    for (int part = 0; part < NINE_SLICE_PARTS; part++)
    {
        const OpenedImage image = Open(request, { PANEL_TILE_FILES[part], PixelOrder::premultiplied });
        const bool decoded = image.converter != nullptr && DecodeWhole(image);
        Close(image);
        const PixelRect tile = { 0, 0, image.size.width, image.size.height };
        all_loaded = decoded && CreateSlice(request, { index, part, tile, image.size.width }) && all_loaded;
    }
    return all_loaded;
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

// Builds an atlas, and its grey twin, cell by cell from its icon files, with as many rows as the files need.
bool ParchmentImages::BuildAtlas(const LoadRequest& request, int atlas_source)
{
    if (!c_assert(atlas_source >= 0 && atlas_source < ATLAS_COUNT)) return false;
    const AtlasSource& source = ATLAS_SOURCES[atlas_source];
    const AtlasFiles files = FilesOf(request.sources, source.content);
    if (!c_assert(files.files != nullptr && files.count > 0 && files.count <= MAX_ATLAS_CELLS)) return false;
    const int rows = (files.count + source.columns - 1) / source.columns;
    const PixelSize size = { source.columns * source.cell.width, rows * source.cell.height };
    if (!c_assert(size.height <= MAX_IMAGE_HEIGHT)) return false;
    const bool has_grey = source.grey_atlas != ImageId::count;
    if (!CreateAtlasBitmap(request, source.atlas, size)) return false;
    if (has_grey && !CreateAtlasBitmap(request, source.grey_atlas, size)) return false;
    const int grey_atlas = has_grey ? static_cast<int>(source.grey_atlas) : -1;
    bool all_loaded = true;
    for (int cell = 0; cell < rows * source.columns && cell < MAX_ATLAS_CELLS + source.columns; cell++)
    {
        const wchar_t* file = cell < files.count ? files.files[cell].path : nullptr;
        const AtlasCell atlas_cell = { static_cast<int>(source.atlas), grey_atlas, cell, source.columns, source.cell,
                                       file };
        all_loaded = FillAtlasCell(request, atlas_cell) && all_loaded;
    }
    return all_loaded;
}

bool ParchmentImages::CreateAtlasBitmap(const LoadRequest& request, ImageId atlas, PixelSize size)
{
    const int index = static_cast<int>(atlas);
    if (!c_assert(index >= 0 && index < IMAGE_COUNT) || !c_assert(size.width > 0 && size.height > 0)) return false;
    const D2D1_BITMAP_PROPERTIES properties = PremultipliedProperties();
    const D2D1_SIZE_U bitmap_size = D2D1::SizeU(static_cast<UINT32>(size.width), static_cast<UINT32>(size.height));
    const HRESULT result = request.creator->CreateBitmap(bitmap_size, nullptr, 0, &properties, &bitmaps[index]);
    if (!c_assert(SUCCEEDED(result))) return false;
    sizes[index] = size;
    return true;
}

// Fills one cell of an atlas, and of its grey twin, from its file. A cell past the files, or whose file can't be
// loaded, stays empty (the bitmaps start out with undefined pixels, so every cell is written).
bool ParchmentImages::FillAtlasCell(const LoadRequest& request, const AtlasCell& cell)
{
    const int pixel_count = cell.size.width * cell.size.height;
    if (!c_assert(pixel_count > 0 && pixel_count <= MAX_SCRATCH_PIXELS)) return false;
    if (!c_assert(bitmaps[cell.atlas] != nullptr)) return false;
    if (!c_assert(cell.grey_atlas < 0 || bitmaps[cell.grey_atlas] != nullptr)) return false;
    for (int i = 0; i < pixel_count && i < MAX_SCRATCH_PIXELS; i++) scratch[i] = 0;
    const bool loaded = cell.file == nullptr || DecodeIntoCell(request, cell);
    const D2D1_RECT_U target = CellRect(cell.cell, cell.columns, cell.size);
    const UINT32 stride = static_cast<UINT32>(cell.size.width) * 4;
    if (!c_assert(SUCCEEDED(bitmaps[cell.atlas]->CopyFromMemory(&target, scratch, stride)))) return false;
    if (cell.grey_atlas < 0) return loaded;
    const Filter grey_filter = IMAGE_SOURCES[cell.grey_atlas].filter;
    for (int i = 0; i < pixel_count && i < MAX_SCRATCH_PIXELS; i++) scratch[i] = FilterPixel(scratch[i], grey_filter);
    return c_assert(SUCCEEDED(bitmaps[cell.grey_atlas]->CopyFromMemory(&target, scratch, stride))) && loaded;
}

// Decodes an icon file into the scratch buffer as one premultiplied cell.
bool ParchmentImages::DecodeIntoCell(const LoadRequest& request, const AtlasCell& cell)
{
    if (!c_assert(cell.file != nullptr)) return false;
    const OpenedImage image = Open(request, { cell.file, PixelOrder::premultiplied });
    const bool copied = image.converter != nullptr && CopyScaled(request, image, cell);
    Close(image);
    return copied;
}

// An icon of another size than its cell is scaled to fit it, keeping its proportions and centred, as the web
// version's object-fit: contain draws it.
bool ParchmentImages::CopyScaled(const LoadRequest& request, const OpenedImage& image, const AtlasCell& cell)
{
    if (!c_assert(image.converter != nullptr) || !c_assert(cell.size.width * cell.size.height <= MAX_SCRATCH_PIXELS))
        return false;
    const PixelSize fitted = ContainedSize(image.size, cell.size);
    if (!c_assert(fitted.width <= cell.size.width && fitted.height <= cell.size.height)) return false;
    const int offset = (cell.size.height - fitted.height) / 2 * cell.size.width + (cell.size.width - fitted.width) / 2;
    const UINT stride = static_cast<UINT>(cell.size.width) * 4;
    const UINT room = static_cast<UINT>(MAX_SCRATCH_PIXELS - offset) * 4;
    BYTE* target = reinterpret_cast<BYTE*>(scratch + offset);
    if (fitted.width == image.size.width && fitted.height == image.size.height)
        return c_assert(SUCCEEDED(image.converter->CopyPixels(nullptr, stride, room, target)));
    IWICBitmapScaler* scaler = nullptr;
    HRESULT result = request.decoder_factory->CreateBitmapScaler(&scaler);
    if (SUCCEEDED(result))
        result = scaler->Initialize(image.converter, static_cast<UINT>(fitted.width), static_cast<UINT>(fitted.height),
                                    WICBitmapInterpolationModeFant);
    if (SUCCEEDED(result)) result = scaler->CopyPixels(nullptr, stride, room, target);
    ReleaseInterface(scaler);
    return c_assert(SUCCEEDED(result));
}

// A cursor is made while the thread is per-monitor DPI aware: Windows then shows it at its own size on every
// display instead of enlarging it for this DPI-unaware program, and the view picks the size that suits the
// display (SelectCursorSize).
bool ParchmentImages::CreateGameCursor(const LoadRequest& request, int cursor_source)
{
    if (!c_assert(cursor_source >= 0 && cursor_source < CURSOR_SOURCE_COUNT)) return false;
    const CursorSource& source = CURSOR_SOURCES[cursor_source];
    const OpenedImage image = Open(request, { source.file, PixelOrder::straight });
    const bool decoded_whole = image.converter != nullptr && DecodeWhole(image);
    Close(image);
    if (!decoded_whole) return false;
    const DPI_AWARENESS_CONTEXT unaware = SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    HCURSOR& cursor = cursors[static_cast<int>(source.size)][static_cast<int>(source.cursor)];
    cursor = CreateCursorFromScratch(image.size, source.hotspot);
    // Going back to the context the switch just left can't fail; the thread's windows are made after this.
    if (unaware != NULL) (void)SetThreadDpiAwarenessContext(unaware);
    return c_assert(cursor != NULL);
}

// A cursor needs straight alpha in a DIB plus an empty mask (CreateIconIndirect); the scratch buffer holds the image.
HCURSOR ParchmentImages::CreateCursorFromScratch(PixelSize size, PixelPoint hotspot) const
{
    if (!c_assert(size.width > 0 && size.height > 0 && size.width * size.height <= MAX_SCRATCH_PIXELS)) return NULL;
    if (!c_assert(hotspot.x >= 0 && hotspot.x < size.width && hotspot.y >= 0 && hotspot.y < size.height)) return NULL;
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
    HCURSOR cursor = NULL;
    if (colour != NULL && bits != nullptr && mask != NULL)
    {
        memcpy(bits, scratch, static_cast<size_t>(size.width) * static_cast<size_t>(size.height) * 4);
        ICONINFO icon = { FALSE, static_cast<DWORD>(hotspot.x), static_cast<DWORD>(hotspot.y), mask, colour };
        cursor = CreateIconIndirect(&icon);
    }
    // CreateIconIndirect copies both bitmaps, so they are freed either way; a failure leaves the system cursor.
    if (colour != NULL) (void)DeleteObject(colour);
    if (mask != NULL) (void)DeleteObject(mask);
    return cursor;
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
    for (int size = 0; size < CURSOR_SIZE_COUNT; size++)
    {
        for (int i = 0; i < CURSOR_COUNT; i++)
        {
            // Destroying a cursor at shutdown: a failure leaves nothing to recover.
            if (cursors[size][i] != NULL) (void)DestroyCursor(cursors[size][i]);
            cursors[size][i] = NULL;
        }
    }
}

// Lets go of an image only needed at startup (the main window's parchment); drawing it later draws nothing.
void ParchmentImages::Forget(ImageId image)
{
    const int index = static_cast<int>(image);
    if (!c_assert(index >= 0 && index < IMAGE_COUNT)) return;
    ReleaseInterface(bitmaps[index]);
}

void ParchmentImages::SelectCursorSize(CursorSize size)
{
    if (!c_assert(static_cast<int>(size) >= 0 && static_cast<int>(size) < CURSOR_SIZE_COUNT)) return;
    cursor_size = size;
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

// The cursor in the size the view chose; a size whose file is missing gives way to the other size.
HCURSOR ParchmentImages::Cursor(CursorId cursor) const
{
    const int index = static_cast<int>(cursor);
    const int size = static_cast<int>(cursor_size);
    if (!c_assert(index >= 0 && index < CURSOR_COUNT) || !c_assert(size >= 0 && size < CURSOR_SIZE_COUNT)) return NULL;
    static_assert(CURSOR_SIZE_COUNT == 2, "a cursor size has one other size to give way to");
    const int other_size = CURSOR_SIZE_COUNT - 1 - size;
    return cursors[size][index] != NULL ? cursors[size][index] : cursors[other_size][index];
}

int ParchmentImages::MissingFileCount() const { return missing_file_count; }

const wchar_t* ParchmentImages::FirstMissingFile() const { return first_missing_file; }
