#include "ParchmentImages.h"

#include <d2d1.h>
#include <wincodec.h>
#include <algorithm>
#include <cstring>

#include "Assertions.h"
#include "CivDataLimits.h"

namespace
{
    constexpr int MAX_PATH_LENGTH = 512;

    template <typename Interface> void ReleaseInterface(Interface*& pointer)
    {
        if (pointer != nullptr) (void)pointer->Release();
        pointer = nullptr;
    }
}

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
    if (missing_file_count == 0) (void)wcsncpy_s(first_missing_file, file, _TRUNCATE);
    missing_file_count++;
}

void ParchmentImages::Close(const OpenedImage& image)
{
    if (image.converter != nullptr) (void)image.converter->Release();
    if (image.frame != nullptr) (void)image.frame->Release();
    if (image.decoder != nullptr) (void)image.decoder->Release();
}

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

bool ParchmentImages::DecodeIntoCell(const LoadRequest& request, const AtlasCell& cell)
{
    if (!c_assert(cell.file != nullptr)) return false;
    const OpenedImage image = Open(request, { cell.file, PixelOrder::premultiplied });
    const bool copied = image.converter != nullptr && CopyScaled(request, image, cell);
    Close(image);
    return copied;
}

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
        std::memcpy(bits, scratch, static_cast<size_t>(size.width) * static_cast<size_t>(size.height) * 4);
        ICONINFO icon = { FALSE, static_cast<DWORD>(hotspot.x), static_cast<DWORD>(hotspot.y), mask, colour };
        cursor = CreateIconIndirect(&icon);
    }
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
            if (cursors[size][i] != NULL) (void)DestroyCursor(cursors[size][i]);
            cursors[size][i] = NULL;
        }
    }
}

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
