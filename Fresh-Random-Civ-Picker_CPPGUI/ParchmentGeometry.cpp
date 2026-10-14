#include "ParchmentGeometry.h"

#include <cmath>
#include <cstddef>

#include "Assertions.h"

namespace
{
    // The slider groove of slider.png runs from 32 to 308 px of 340 px; the CSS rounds this to percentages.
    constexpr float SLIDER_GROOVE_START = 0.0941f;
    constexpr float SLIDER_GROOVE_LENGTH = 0.8118f;
    constexpr int TOOLTIP_OFFSET_X = 10;
    constexpr int TOOLTIP_OFFSET_Y = 20;
    constexpr int TOOLTIP_FLIP_GAP = 8;
    constexpr int TOOLTIP_MARGIN = 4;
    constexpr int BOX_BLUR_PASSES = 3;
    constexpr float PI = 3.14159265f;

    uint8_t ToChannel(float value)
    {
        if (value <= 0.0f) return 0;
        if (value >= 255.0f) return 255;
        return static_cast<uint8_t>(std::lround(value));
    }

    // Applies a CSS filter colour matrix (three rows of red, green and blue weights).
    Rgb ApplyMatrix(const Rgb& colour, const float (&matrix)[9])
    {
        const float red = colour.red;
        const float green = colour.green;
        const float blue = colour.blue;
        return { ToChannel(matrix[0] * red + matrix[1] * green + matrix[2] * blue),
                 ToChannel(matrix[3] * red + matrix[4] * green + matrix[5] * blue),
                 ToChannel(matrix[6] * red + matrix[7] * green + matrix[8] * blue) };
    }

    // Radius of each of the three box blurs that together approximate a Gaussian of this sigma.
    int BoxRadius(float sigma)
    {
        const float ideal_width = std::sqrt(4.0f * sigma * sigma + 1.0f);
        const int radius = static_cast<int>(std::lround((ideal_width - 1.0f) / 2.0f));
        return radius < 1 ? 1 : radius;
    }
}

int RectWidth(const PixelRect& rect) { return rect.right - rect.left; }

int RectHeight(const PixelRect& rect) { return rect.bottom - rect.top; }

bool RectContains(const PixelRect& rect, PixelPoint point)
{
    return point.x >= rect.left && point.x < rect.right && point.y >= rect.top && point.y < rect.bottom;
}

bool HasArea(const PixelRect& rect) { return rect.right > rect.left && rect.bottom > rect.top; }

bool HasArea(const FloatRect& rect) { return rect.right > rect.left && rect.bottom > rect.top; }

// True when the two share at least one pixel; rects that only touch don't.
bool RectsOverlap(const PixelRect& first, const PixelRect& second)
{
    const bool apart_horizontally = first.right <= second.left || second.right <= first.left;
    const bool apart_vertically = first.bottom <= second.top || second.bottom <= first.top;
    return !apart_horizontally && !apart_vertically;
}

PixelRect BoundingRect(const PixelRect& first, const PixelRect& second)
{
    const int left = first.left < second.left ? first.left : second.left;
    const int top = first.top < second.top ? first.top : second.top;
    const int right = first.right > second.right ? first.right : second.right;
    const int bottom = first.bottom > second.bottom ? first.bottom : second.bottom;
    return { left, top, right, bottom };
}

// Every other pixel carries a dot. A side of even length can't start and end on a dot that way, so its first dot
// is doubled.
bool DottedLineHasDot(int index, int length)
{
    if (index < 0 || index >= length) return false;
    if (length % 2 == 1) return index % 2 == 0;
    return index == 0 || index % 2 == 1;
}

PixelRect OffsetRect(const PixelRect& rect, PixelPoint offset)
{
    return { rect.left + offset.x, rect.top + offset.y, rect.right + offset.x, rect.bottom + offset.y };
}

FloatRect ToFloatRect(const PixelRect& rect)
{
    return { static_cast<float>(rect.left), static_cast<float>(rect.top), static_cast<float>(rect.right),
             static_cast<float>(rect.bottom) };
}

// --- Nine-slice images ---

TileSpan LayOutTiles(const TileRequest& request)
{
    const float area = static_cast<float>(request.area_length);
    if (!c_assert(request.area_length >= 0) || !c_assert(request.tile_length > 0.0f)) return { 0.0f, area };
    if (request.rule == TileRule::stretch || request.area_length == 0) return { 0.0f, area };
    if (request.rule == TileRule::round)
    {
        const long tile_count = std::lround(area / request.tile_length);
        return { 0.0f, area / static_cast<float>(tile_count < 1 ? 1 : tile_count) };
    }
    // repeat: one tile is centred in the area, and the tiling starts at or before the area's start.
    const float centred_start = (area - request.tile_length) / 2.0f;
    const float tiles_before = std::ceil(centred_start / request.tile_length);
    return { centred_start - tiles_before * request.tile_length, request.tile_length };
}

NineSliceAreas SplitNineSlice(const PixelRect& target, int border_width)
{
    NineSliceAreas areas = {};
    if (!c_assert(border_width >= 0) || !c_assert(RectWidth(target) >= 0 && RectHeight(target) >= 0)) return areas;
    const int columns[] = { target.left, target.left + border_width, target.right - border_width, target.right };
    const int rows[] = { target.top, target.top + border_width, target.bottom - border_width, target.bottom };
    for (int row = 0; row < 3; row++)
    {
        for (int column = 0; column < 3; column++)
        {
            areas.area[row * 3 + column] = { columns[column], rows[row], columns[column + 1], rows[row + 1] };
        }
    }
    return areas;
}

// --- The log scrollbar ---

int MaxScrollTop(const ScrollMetrics& metrics)
{
    if (!c_assert(metrics.viewport_height >= 0)) return 0;
    const int hidden = metrics.content_height - metrics.viewport_height;
    return hidden > 0 ? hidden : 0;
}

bool NeedsScrollbar(const ScrollMetrics& metrics) { return MaxScrollTop(metrics) > 0; }

int ClampScrollTop(const ScrollMetrics& metrics)
{
    const int max_scroll_top = MaxScrollTop(metrics);
    if (metrics.scroll_top < 0) return 0;
    return metrics.scroll_top > max_scroll_top ? max_scroll_top : metrics.scroll_top;
}

int KnobOffset(const ScrollMetrics& metrics, int track_length)
{
    const int max_scroll_top = MaxScrollTop(metrics);
    const int free_length = track_length - SCROLLBAR_KNOB_SIZE;
    if (!c_assert(free_length >= 0) || max_scroll_top <= 0) return 0;
    const float position = static_cast<float>(ClampScrollTop(metrics)) / static_cast<float>(max_scroll_top);
    return static_cast<int>(std::lround(position * static_cast<float>(free_length)));
}

int ScrollTopForKnobDrag(const KnobDrag& drag)
{
    if (!c_assert(drag.max_scroll_top >= 0) || !c_assert(drag.free_track_length >= 0)) return 0;
    if (drag.free_track_length == 0) return drag.start_scroll_top;
    const float ratio = static_cast<float>(drag.max_scroll_top) / static_cast<float>(drag.free_track_length);
    const int scroll_top =
        drag.start_scroll_top + static_cast<int>(std::lround(static_cast<float>(drag.drag_distance) * ratio));
    return ClampScrollTop({ drag.max_scroll_top, 0, scroll_top });
}

ScrollbarPart HitTestScrollbar(const ScrollbarHit& hit)
{
    if (!c_assert(hit.bar_height >= 2 * SCROLLBAR_BUTTON_LENGTH)) return ScrollbarPart::none;
    if (hit.y < 0 || hit.y >= hit.bar_height) return ScrollbarPart::none;
    if (hit.y < SCROLLBAR_BUTTON_LENGTH) return ScrollbarPart::up_button;
    if (hit.y >= hit.bar_height - SCROLLBAR_BUTTON_LENGTH) return ScrollbarPart::down_button;
    const int knob_top = SCROLLBAR_BUTTON_LENGTH + hit.knob_offset;
    if (hit.y < knob_top) return ScrollbarPart::track_above_knob;
    if (hit.y < knob_top + SCROLLBAR_KNOB_SIZE) return ScrollbarPart::knob;
    return ScrollbarPart::track_below_knob;
}

// --- The slider ---

float SliderFill(const SliderRange& range)
{
    if (!c_assert(range.maximum >= range.minimum)) return 0.0f;
    if (range.maximum == range.minimum) return 0.0f;
    const float span = static_cast<float>(range.maximum - range.minimum);
    const float fill = static_cast<float>(range.value - range.minimum) / span;
    if (fill < 0.0f) return 0.0f;
    return fill > 1.0f ? 1.0f : fill;
}

float SliderKnobCentre(int slider_width, float fill)
{
    if (!c_assert(slider_width >= 0) || !c_assert(fill >= 0.0f && fill <= 1.0f)) return 0.0f;
    const float width = static_cast<float>(slider_width);
    return width * SLIDER_GROOVE_START + width * SLIDER_GROOVE_LENGTH * fill;
}

int SliderValueAt(const SliderTrack& track, float pointer_x)
{
    const SliderRange& range = track.range;
    if (!c_assert(track.width > 0) || !c_assert(range.maximum >= range.minimum)) return range.minimum;
    const float width = static_cast<float>(track.width);
    float fill = (pointer_x - width * SLIDER_GROOVE_START) / (width * SLIDER_GROOVE_LENGTH);
    if (fill < 0.0f) fill = 0.0f;
    if (fill > 1.0f) fill = 1.0f;
    const float steps = fill * static_cast<float>(range.maximum - range.minimum);
    return range.minimum + static_cast<int>(std::lround(steps));
}

// --- Text ---

bool WrappedLines::Wrap(const WrapRequest& request)
{
    count = 0;
    if (!c_assert(request.length >= 0) || !c_assert(request.length == 0 || request.advances != nullptr)) return false;
    if (!c_assert(request.text != nullptr)) return false;
    TextLine line = { 0, 0, 0.0f };
    int last_space = -1;
    float width_at_space = 0.0f;
    for (int i = 0; i < request.length; i++)
    {
        const wchar_t character = request.text[i];
        if (character == L'\n')
        {
            if (!AddLine(line)) return false;
            line = { i + 1, 0, 0.0f };
            last_space = -1;
            continue;
        }
        if (character == L' ')
        {
            last_space = i;
            width_at_space = line.width;
        }
        const bool overflows = line.width + request.advances[i] > request.max_width && character != L' ';
        if (overflows && last_space >= line.start)
        {
            if (!AddLine({ line.start, last_space - line.start, width_at_space })) return false;
            line = { last_space + 1, i - last_space - 1, line.width - width_at_space - request.advances[last_space] };
            last_space = -1;
        }
        line.length++;
        line.width += request.advances[i];
    }
    return AddLine(line);
}

// Adds a line without its trailing spaces, which hang past the line's end as in a browser.
bool WrappedLines::AddLine(const TextLine& line)
{
    if (!c_assert(count < MAX_WRAPPED_LINES) || !c_assert(line.length >= 0)) return false;
    lines[count] = line;
    count++;
    return true;
}

int WrappedLines::Count() const { return count; }

TextLine WrappedLines::Line(int index) const
{
    if (!c_assert(index >= 0 && index < count)) return { 0, 0, 0.0f };
    return lines[index];
}

float WrappedLines::WidestLine() const
{
    float widest = 0.0f;
    for (int i = 0; i < count && i < MAX_WRAPPED_LINES; i++)
    {
        if (lines[i].width > widest) widest = lines[i].width;
    }
    return widest;
}

// --- Window shapes ---

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

// --- Drop shadows ---

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

// One box blur along every row: each pixel becomes the mean of the 2 * radius + 1 pixels around it.
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

// One box blur down every column.
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

// --- Box shadows ---

// Leaves the box's pixels opaque and turns everything around it into its shadow (CSS box-shadow: the box moved
// by the offset, blurred, in black at the given opacity).
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
    // Without the blur the shadow keeps a hard edge, which still reads as a shadow.
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

// --- CSS filter functions (Filter Effects Level 1, applied to sRGB values) ---

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

// --- Tooltips ---

PixelPoint PlaceTooltip(const TooltipRequest& request)
{
    if (!c_assert(request.size.width >= 0 && request.size.height >= 0)) return request.cursor;
    int x = request.cursor.x + TOOLTIP_OFFSET_X;
    int y = request.cursor.y + TOOLTIP_OFFSET_Y;
    if (x + request.size.width > request.bounds.right - TOOLTIP_MARGIN)
    {
        const int leftmost = request.bounds.left + TOOLTIP_MARGIN;
        const int fitted = request.bounds.right - request.size.width - TOOLTIP_MARGIN;
        x = fitted > leftmost ? fitted : leftmost;
    }
    if (y + request.size.height > request.bounds.bottom - TOOLTIP_MARGIN)
        y = request.cursor.y - request.size.height - TOOLTIP_FLIP_GAP;
    return { x, y };
}
