#include "ParchmentImages.h"

#include <d2d1.h>
#include <wincodec.h>
#include <algorithm>

#include "Assertions.h"
#include "CivDataLimits.h"

namespace
{
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
    constexpr int MAX_CIV_ICONS = MAX_CIVS + 1;
    constexpr int MAX_DLC_ICONS = EDITION_LOOKS * MAX_EDITIONS + MAX_DLCS;
    constexpr int MAX_ATLAS_CELLS = std::max(MAX_CIV_ICONS, MAX_DLC_ICONS);

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
        { NineSliceId::panel_frame, ImageId::count, 64 },
        { NineSliceId::text_field_frame, ImageId::text_field_frame, 15 },
    };

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

    const CursorSource CURSOR_SOURCES[] = {
        { CursorId::arrow, CursorSize::standard, L"ui\\cursor_x1.png", { 0, 0 } },
        { CursorId::hand, CursorSize::standard, L"ui\\cursor_pointing_x1.png", { 2, 0 } },
        { CursorId::arrow, CursorSize::large, L"ui\\cursor_x2.png", { 0, 0 } },
        { CursorId::hand, CursorSize::large, L"ui\\cursor_pointing_x2.png", { 3, 0 } },
    };
    constexpr int CURSOR_SOURCE_COUNT = static_cast<int>(sizeof(CURSOR_SOURCES) / sizeof(CURSOR_SOURCES[0]));
    static_assert(CURSOR_SOURCE_COUNT == CURSOR_COUNT * CURSOR_SIZE_COUNT, "every cursor has a file in every size");

    Rgb ApplyFilter(const Rgb& colour, Filter filter)
    {
        switch (filter)
        {
            case Filter::grayscale: return Grayscale(colour);
            case Filter::dim_tab: return Brightness(colour, 0.88f);
            case Filter::black: return Brightness(colour, 0.0f);
            case Filter::tooltip_tint: return Saturate(HueRotate(Brightness(colour, 0.15f), -54.0f), 3.1f);
            case Filter::none: return colour;
            default: (void)c_assert(false); break;
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

    AtlasFiles FilesOf(const ImageSources& sources, AtlasContent content)
    {
        switch (content)
        {
            case AtlasContent::civ_icons: return sources.civ_icons;
            case AtlasContent::dlc_icons: return sources.dlc_icons;
            case AtlasContent::edition_banners: return sources.edition_banners;
            default: (void)c_assert(false); break;
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
        if (!c_assert(image.size.width * image.size.height <= MAX_SCRATCH_PIXELS)) return false;
        if (!CreateSlices(request, nine_slice.nine_slice, image.size)) return false;
    }
    return true;
}

bool ParchmentImages::FillBitmap(const OpenedImage& image, int image_index)
{
    if (!c_assert(image.converter != nullptr) || !c_assert(bitmaps[image_index] != nullptr)) return false;
    const PixelSize size = image.size;
    const Filter filter = IMAGE_SOURCES[image_index].filter;
    const int strip_rows = MAX_SCRATCH_PIXELS / size.width;
    const UINT stride = static_cast<UINT>(size.width) * 4;
    for (int top = 0; top < size.height && top < MAX_IMAGE_HEIGHT; top += strip_rows)
    {
        const int rows = std::min(size.height - top, strip_rows);
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
    if (unaware != NULL) (void)SetThreadDpiAwarenessContext(unaware);
    return c_assert(cursor != NULL);
}

int ParchmentImages::SliceSize(NineSliceId nine_slice) const
{
    const int index = static_cast<int>(nine_slice);
    if (!c_assert(index >= 0 && index < NINE_SLICE_COUNT)) return 0;
    return NINE_SLICE_SOURCES[index].slice;
}
