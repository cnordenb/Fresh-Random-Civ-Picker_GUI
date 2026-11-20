#pragma once

#include <cstdint>

struct PixelPoint
{
    int x;
    int y;
};

struct PixelSize
{
    int width;
    int height;
};

struct PixelRect
{
    int left;
    int top;
    int right;
    int bottom;
};

struct FloatRect
{
    float left;
    float top;
    float right;
    float bottom;
};

int RectWidth(const PixelRect& rect);
int RectHeight(const PixelRect& rect);
bool RectContains(const PixelRect& rect, PixelPoint point);
bool HasArea(const PixelRect& rect);
bool HasArea(const FloatRect& rect);
bool RectsOverlap(const PixelRect& first, const PixelRect& second);
PixelRect BoundingRect(const PixelRect& first, const PixelRect& second);

bool DottedLineHasDot(int index, int length);
PixelRect OffsetRect(const PixelRect& rect, PixelPoint offset);
FloatRect ToFloatRect(const PixelRect& rect);

enum class TileRule { stretch, repeat, round };

struct TileRequest
{
    int area_length;
    float tile_length;
    TileRule rule;
};

struct TileSpan
{
    float origin;
    float length;
};

TileSpan LayOutTiles(const TileRequest& request);

constexpr int NINE_SLICE_PARTS = 9;

struct NineSliceAreas
{
    PixelRect area[NINE_SLICE_PARTS];
};

NineSliceAreas SplitNineSlice(const PixelRect& target, int border_width);

constexpr int SCROLLBAR_WIDTH = 27;
constexpr int SCROLLBAR_BUTTON_LENGTH = 27;
constexpr int SCROLLBAR_KNOB_SIZE = 21;
constexpr int SCROLLBAR_KNOB_INSET = 3;

struct ScrollMetrics
{
    int content_height;
    int viewport_height;
    int scroll_top;
};

int MaxScrollTop(const ScrollMetrics& metrics);
bool NeedsScrollbar(const ScrollMetrics& metrics);
int ClampScrollTop(const ScrollMetrics& metrics);
int KnobOffset(const ScrollMetrics& metrics, int track_length);

struct KnobDrag
{
    int start_scroll_top;
    int drag_distance;
    int max_scroll_top;
    int free_track_length;
};

int ScrollTopForKnobDrag(const KnobDrag& drag);

enum class ScrollbarPart { none, up_button, down_button, track_above_knob, track_below_knob, knob };

struct ScrollbarHit
{
    int y;
    int bar_height;
    int knob_offset;
};

ScrollbarPart HitTestScrollbar(const ScrollbarHit& hit);

struct SliderRange
{
    int minimum;
    int maximum;
    int value;
};

struct SliderLayout
{
    FloatRect left_arrow;
    FloatRect right_arrow;
    FloatRect bar;
    float handle_size;
    float groove_start;
    float groove_end;
};

struct SliderTrack
{
    SliderLayout layout;
    SliderRange range;
};

enum class SliderPart { none, left_arrow, right_arrow, handle, bar };

SliderLayout LayOutSlider(PixelSize size);
float SliderFill(const SliderRange& range);
float SliderHandleCentre(const SliderTrack& track);
int SliderValueAt(const SliderTrack& track, float pointer_x);
SliderPart HitTestSlider(const SliderTrack& track, PixelPoint point);

PixelSize ContainedSize(PixelSize image, PixelSize box);

constexpr int MAX_WRAPPED_LINES = 1024;

struct TextLine
{
    int start;
    int length;
    float width;
};

struct WrapRequest
{
    const wchar_t* text;
    const float* advances;
    int length;
    float max_width;
};

class WrappedLines final
{
public:
    bool Wrap(const WrapRequest& request);
    int Count() const;
    TextLine Line(int index) const;
    float WidestLine() const;

private:
    bool AddLine(const TextLine& line);

    TextLine lines[MAX_WRAPPED_LINES] = {};
    int count = 0;
};

constexpr int MAX_ICON_LABELS = 128;
constexpr int MAX_ICON_LABEL_LENGTH = 64;

struct IconLabel
{
    const wchar_t* text;
    int cell;
};

class IconLabels final
{
public:
    static constexpr int NO_ICON = -1;

    bool Add(const IconLabel& label);
    int CellOfLeadingLabel(const wchar_t* line, int length) const;

private:
    struct StoredLabel
    {
        wchar_t text[MAX_ICON_LABEL_LENGTH + 1];
        int length;
        int cell;
    };

    static bool StartsLine(const StoredLabel& label, const wchar_t* line, int length);

    StoredLabel labels[MAX_ICON_LABELS] = {};
    int count = 0;
};

constexpr int MAX_RUNS_PER_ROW = 64;

struct PixelRun
{
    int start;
    int end;
};

class OpaqueRuns final
{
public:
    bool Scan(const uint8_t* alpha_row, int width);
    int Count() const;
    PixelRun Run(int index) const;

private:
    PixelRun runs[MAX_RUNS_PER_ROW] = {};
    int count = 0;
};

constexpr int MAX_SHADOW_SIDE = 1600;

struct BlurRequest
{
    uint8_t* mask;
    int width;
    int height;
    float sigma;
};

class ShadowBlur final
{
public:
    bool Blur(const BlurRequest& request);

private:
    void BlurRows(const BlurRequest& request, int radius);
    void BlurColumns(const BlurRequest& request, int radius);

    uint8_t line[MAX_SHADOW_SIDE] = {};
};

constexpr int MAX_BOX_SHADOW_PIXELS = 512 * 512;

struct BoxShadowRequest
{
    uint32_t* pixels;
    int stride;
    PixelSize frame;
    PixelRect box;
    PixelPoint offset;
    float sigma;
    float opacity;
};

class BoxShadow final
{
public:
    bool Apply(const BoxShadowRequest& request);

private:
    ShadowBlur blur;
    uint8_t mask[MAX_BOX_SHADOW_PIXELS] = {};
};

struct Rgb
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

Rgb Grayscale(const Rgb& colour);
Rgb Brightness(const Rgb& colour, float amount);
Rgb HueRotate(const Rgb& colour, float degrees);
Rgb Saturate(const Rgb& colour, float amount);

struct TooltipRequest
{
    PixelPoint cursor;
    PixelSize size;
    PixelRect bounds;
};

PixelPoint PlaceTooltip(const TooltipRequest& request);
