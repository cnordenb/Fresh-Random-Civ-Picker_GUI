#pragma once

#include <cstdint>

#include "framework.h"
#include "ParchmentGeometry.h"

class PixelSurface final
{
public:
    bool Create(PixelSize surface_size);
    void Destroy();

    HDC Dc() const;
    PixelSize Size() const;
    uint32_t* Pixels() const;
    bool Contains(const PixelRect& area) const;

private:
    HDC dc = NULL;
    HBITMAP bitmap = NULL;
    HGDIOBJ previous_bitmap = NULL;
    uint32_t* pixels = nullptr;
    PixelSize size = { 0, 0 };
};
