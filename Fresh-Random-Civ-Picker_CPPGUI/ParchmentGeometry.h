#pragma once

#include <cstdint>

// Layout and pixel arithmetic of the parchment theme, the look of the web version (frcp.net) that this program
// shares. Part of the View. Plain C++ without Win32 or Direct2D types, so all of it is unit-tested; the values
// reproduce the web version's CSS rules.

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

// outline: 1px dotted, as Chrome draws it: each side of length pixels gets a dot at both of its ends.
bool DottedLineHasDot(int index, int length);
PixelRect OffsetRect(const PixelRect& rect, PixelPoint offset);
FloatRect ToFloatRect(const PixelRect& rect);

// --- Nine-slice images (CSS border-image) ---

// border-image-repeat: stretch one tile, repeat whole tiles centred in the area, or round them to fit.
enum class TileRule { stretch, repeat, round };

struct TileRequest
{
    int area_length;
    float tile_length;
    TileRule rule;
};

// Where the tiling of one axis starts (at or before the area's start) and how long each tile is.
struct TileSpan
{
    float origin;
    float length;
};

TileSpan LayOutTiles(const TileRequest& request);

constexpr int NINE_SLICE_PARTS = 9;

// The nine areas of a nine-slice target, row by row: top-left corner, top edge, top-right corner, left edge,
// middle, right edge, bottom-left corner, bottom edge, bottom-right corner.
struct NineSliceAreas
{
    PixelRect area[NINE_SLICE_PARTS];
};

NineSliceAreas SplitNineSlice(const PixelRect& target, int border_width);

// --- The log scrollbar: a 27 px wide bar of an up button, a track and a down button, with a 21 px knob ---

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

// --- The slider: a track image as wide as the slider, whose groove runs from 9.41% to 90.59% of it ---

struct SliderRange
{
    int minimum;
    int maximum;
    int value;
};

struct SliderTrack
{
    int width;
    SliderRange range;
};

float SliderFill(const SliderRange& range);
float SliderKnobCentre(int slider_width, float fill);
int SliderValueAt(const SliderTrack& track, float pointer_x);

// --- Text: word wrap at spaces and at the text's own line breaks (CSS white-space: pre-line) ---

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

class WrappedLines
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

// --- Window shapes: the fully opaque runs of an alpha row, for a window region ---

constexpr int MAX_RUNS_PER_ROW = 64;

struct PixelRun
{
    int start;
    int end;
};

class OpaqueRuns
{
public:
    bool Scan(const uint8_t* alpha_row, int width);
    int Count() const;
    PixelRun Run(int index) const;

private:
    PixelRun runs[MAX_RUNS_PER_ROW] = {};
    int count = 0;
};

// --- Drop shadows: a Gaussian blur of an alpha mask, approximated by three box blurs ---

constexpr int MAX_SHADOW_SIDE = 1600;

struct BlurRequest
{
    uint8_t* mask;
    int width;
    int height;
    float sigma;
};

class ShadowBlur
{
public:
    bool Blur(const BlurRequest& request);

private:
    void BlurRows(const BlurRequest& request, int radius);
    void BlurColumns(const BlurRequest& request, int radius);

    uint8_t line[MAX_SHADOW_SIDE] = {};
};

// --- Box shadows of layered windows: the box stays opaque, its shadow is premultiplied black around it ---

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

class BoxShadow
{
public:
    bool Apply(const BoxShadowRequest& request);

private:
    ShadowBlur blur;
    uint8_t mask[MAX_BOX_SHADOW_PIXELS] = {};
};

// --- CSS filter functions on one sRGB colour ---

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

// --- Tooltips: 10 px right of and 20 px below the cursor, kept 4 px inside the bounds ---

struct TooltipRequest
{
    PixelPoint cursor;
    PixelSize size;
    PixelRect bounds;
};

PixelPoint PlaceTooltip(const TooltipRequest& request);
