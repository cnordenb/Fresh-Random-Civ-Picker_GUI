#include "PixelSurface.h"

#include "Assertions.h"

bool PixelSurface::Create(PixelSize surface_size)
{
    if (!c_assert(dc == NULL) || !c_assert(surface_size.width > 0 && surface_size.height > 0)) return false;
    BITMAPINFO info = {};
    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = surface_size.width;
    info.bmiHeader.biHeight = -surface_size.height;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;

    dc = CreateCompatibleDC(NULL);
    if (!c_assert(dc != NULL)) return false;
    void* bits = nullptr;
    bitmap = CreateDIBSection(dc, &info, DIB_RGB_COLORS, &bits, NULL, 0);
    if (!c_assert(bitmap != NULL) || !c_assert(bits != nullptr))
    {
        Destroy();
        return false;
    }
    previous_bitmap = SelectObject(dc, bitmap);
    pixels = static_cast<uint32_t*>(bits);
    size = surface_size;
    return true;
}

void PixelSurface::Destroy()
{
    if (dc != NULL && previous_bitmap != NULL) (void)SelectObject(dc, previous_bitmap);
    // Freeing at shutdown: a failure leaves nothing to recover.
    if (bitmap != NULL) (void)DeleteObject(bitmap);
    if (dc != NULL) (void)DeleteDC(dc);
    dc = NULL;
    bitmap = NULL;
    previous_bitmap = NULL;
    pixels = nullptr;
    size = { 0, 0 };
}

HDC PixelSurface::Dc() const { return dc; }

PixelSize PixelSurface::Size() const { return size; }

uint32_t* PixelSurface::Pixels() const { return pixels; }

bool PixelSurface::Contains(const PixelRect& area) const
{
    return area.left >= 0 && area.top >= 0 && area.right <= size.width && area.bottom <= size.height &&
           area.left <= area.right && area.top <= area.bottom;
}
