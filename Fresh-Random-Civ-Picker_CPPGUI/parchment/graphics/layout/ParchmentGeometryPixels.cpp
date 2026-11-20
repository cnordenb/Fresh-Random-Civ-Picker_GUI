#include "ParchmentGeometry.h"

#include <algorithm>
#include <cmath>
#include <cstddef>

#include "Assertions.h"

namespace
{
    constexpr int BOX_BLUR_PASSES = 3;
    constexpr float PI = 3.14159265f;

    uint8_t ToChannel(float value)
    {
        if (value <= 0.0f) return 0;
        if (value >= 255.0f) return 255;
        return static_cast<uint8_t>(std::lround(value));
    }

    Rgb ApplyMatrix(const Rgb& colour, const float (&matrix)[9])
    {
        const float red = colour.red;
        const float green = colour.green;
        const float blue = colour.blue;
        return { ToChannel(matrix[0] * red + matrix[1] * green + matrix[2] * blue),
                 ToChannel(matrix[3] * red + matrix[4] * green + matrix[5] * blue),
                 ToChannel(matrix[6] * red + matrix[7] * green + matrix[8] * blue) };
    }

    int BoxRadius(float sigma)
    {
        const float ideal_width = std::sqrt(4.0f * sigma * sigma + 1.0f);
        const int radius = static_cast<int>(std::lround((ideal_width - 1.0f) / 2.0f));
        return std::max(radius, 1);
    }
}

bool OpaqueRuns::Scan(const uint8_t* alpha_row, int width)
{
    count = 0;
    if (!c_assert(alpha_row != nullptr) || !c_assert(width >= 0)) return false;
    int run_start = -1;
    for (int x = 0; x <= width; x++)
    {
        const bool opaque = x < width && alpha_row[x] == 255;
        if (opaque && run_start < 0) run_start = x;
        if (opaque || run_start < 0) continue;
        if (!c_assert(count < MAX_RUNS_PER_ROW)) return false;
        runs[count] = { run_start, x };
        count++;
        run_start = -1;
    }
    return true;
}

int OpaqueRuns::Count() const { return count; }

PixelRun OpaqueRuns::Run(int index) const
{
    if (!c_assert(index >= 0 && index < count)) return { 0, 0 };
    return runs[index];
}

bool ShadowBlur::Blur(const BlurRequest& request)
{
    if (!c_assert(request.mask != nullptr) || !c_assert(request.sigma > 0.0f)) return false;
    if (!c_assert(request.width > 0 && request.width <= MAX_SHADOW_SIDE)) return false;
    if (!c_assert(request.height > 0 && request.height <= MAX_SHADOW_SIDE)) return false;
    const int radius = BoxRadius(request.sigma);
    for (int pass = 0; pass < BOX_BLUR_PASSES; pass++)
    {
        BlurRows(request, radius);
        BlurColumns(request, radius);
    }
    return true;
}

void ShadowBlur::BlurRows(const BlurRequest& request, int radius)
{
    const int window = 2 * radius + 1;
    for (int y = 0; y < request.height; y++)
    {
        uint8_t* row = request.mask + static_cast<ptrdiff_t>(y) * request.width;
        for (int x = 0; x < request.width; x++) line[x] = row[x];
        int sum = 0;
        for (int x = -radius; x <= radius; x++) sum += (x >= 0 && x < request.width) ? line[x] : 0;
        for (int x = 0; x < request.width; x++)
        {
            row[x] = static_cast<uint8_t>(sum / window);
            const int leaving = x - radius;
            const int entering = x + radius + 1;
            sum += (entering < request.width ? line[entering] : 0) - (leaving >= 0 ? line[leaving] : 0);
        }
    }
}

void ShadowBlur::BlurColumns(const BlurRequest& request, int radius)
{
    const int window = 2 * radius + 1;
    for (int x = 0; x < request.width; x++)
    {
        for (int y = 0; y < request.height; y++) line[y] = request.mask[static_cast<ptrdiff_t>(y) * request.width + x];
        int sum = 0;
        for (int y = -radius; y <= radius; y++) sum += (y >= 0 && y < request.height) ? line[y] : 0;
        for (int y = 0; y < request.height; y++)
        {
            request.mask[static_cast<ptrdiff_t>(y) * request.width + x] = static_cast<uint8_t>(sum / window);
            const int leaving = y - radius;
            const int entering = y + radius + 1;
            sum += (entering < request.height ? line[entering] : 0) - (leaving >= 0 ? line[leaving] : 0);
        }
    }
}

bool BoxShadow::Apply(const BoxShadowRequest& request)
{
    const PixelSize frame = request.frame;
    if (!c_assert(request.pixels != nullptr) || !c_assert(request.stride >= frame.width)) return false;
    if (!c_assert(frame.width > 0 && frame.height > 0 && frame.width * frame.height <= MAX_BOX_SHADOW_PIXELS))
        return false;
    const PixelRect cast = OffsetRect(request.box, request.offset);
    for (int y = 0; y < frame.height; y++)
    {
        for (int x = 0; x < frame.width; x++) mask[y * frame.width + x] = RectContains(cast, { x, y }) ? 255 : 0;
    }
    (void)blur.Blur({ mask, frame.width, frame.height, request.sigma });
    for (int y = 0; y < frame.height; y++)
    {
        for (int x = 0; x < frame.width; x++)
        {
            uint32_t& pixel = request.pixels[static_cast<ptrdiff_t>(y) * request.stride + x];
            const float alpha = request.opacity * static_cast<float>(mask[y * frame.width + x]);
            pixel = RectContains(request.box, { x, y }) ? (pixel | 0xff000000u)
                                                        : static_cast<uint32_t>(std::lround(alpha)) << 24;
        }
    }
    return true;
}

Rgb Grayscale(const Rgb& colour)
{
    const float matrix[9] = { 0.2126f, 0.7152f, 0.0722f, 0.2126f, 0.7152f, 0.0722f, 0.2126f, 0.7152f, 0.0722f };
    return ApplyMatrix(colour, matrix);
}

Rgb Brightness(const Rgb& colour, float amount)
{
    if (!c_assert(amount >= 0.0f)) return colour;
    return { ToChannel(colour.red * amount), ToChannel(colour.green * amount), ToChannel(colour.blue * amount) };
}

Rgb HueRotate(const Rgb& colour, float degrees)
{
    const float radians = degrees * PI / 180.0f;
    const float c = std::cos(radians);
    const float s = std::sin(radians);
    const float matrix[9] = {
        0.213f + c * 0.787f - s * 0.213f, 0.715f - c * 0.715f - s * 0.715f, 0.072f - c * 0.072f + s * 0.928f,
        0.213f - c * 0.213f + s * 0.143f, 0.715f + c * 0.285f + s * 0.140f, 0.072f - c * 0.072f - s * 0.283f,
        0.213f - c * 0.213f - s * 0.787f, 0.715f - c * 0.715f + s * 0.715f, 0.072f + c * 0.928f + s * 0.072f
    };
    return ApplyMatrix(colour, matrix);
}

Rgb Saturate(const Rgb& colour, float amount)
{
    if (!c_assert(amount >= 0.0f)) return colour;
    const float s = amount;
    const float matrix[9] = { 0.213f + 0.787f * s, 0.715f - 0.715f * s, 0.072f - 0.072f * s,
                              0.213f - 0.213f * s, 0.715f + 0.285f * s, 0.072f - 0.072f * s,
                              0.213f - 0.213f * s, 0.715f - 0.715f * s, 0.072f + 0.928f * s };
    return ApplyMatrix(colour, matrix);
}
