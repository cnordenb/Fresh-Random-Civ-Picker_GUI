#pragma once

#include <cstdint>

#include "ParchmentTheme.h"

struct IDWriteFactory;
struct IDWriteFontFace;
struct IDWriteTextFormat;

struct TextSpan
{
    const wchar_t* text;
    int length;
    FontId font;
};

// The theme's fonts through DirectWrite (View, part of the Direct2D boundary), set up at startup: a text format
// per font and alignment, and the font metrics a browser lays text out with. Text is measured from the fonts'
// glyph advances and kerning, which needs no memory at run time.
class ParchmentFonts final
{
public:
    static constexpr int MAX_MEASURED_LENGTH = 16384;

    bool Load();
    void Release();

    IDWriteTextFormat* Format(FontId font, TextAlign align) const;
    float Size(FontId font) const;
    float Ascent(FontId font) const;
    float LayoutAscent(FontId font) const;
    float LayoutDescent(FontId font) const;
    float StrikeOffset(FontId font) const;
    float UnderlineOffset(FontId font) const;
    float UnderlineThickness(FontId font) const;

    float Measure(const TextSpan& span);
    const float* Advances(const TextSpan& span);

private:
    struct FontSpec
    {
        FontId font;
        float size;
        bool bold;
        bool italic;
        bool blackletter;
    };

    struct FontMetrics
    {
        float size;
        float ascent;
        float descent;
        float strike_offset;
        float underline_offset;
        float underline_thickness;
        float design_units_per_pixel;
    };

    const FontMetrics& MetricsOf(FontId font) const;
    bool LoadFont(const FontSpec& spec);
    bool LoadFontFace(const FontSpec& spec, const wchar_t* family);
    const wchar_t* BlackletterFamily() const;
    bool MeasureChunk(const TextSpan& span, int first);

    IDWriteFactory* factory = nullptr;
    IDWriteTextFormat* formats[FONT_COUNT][3] = {};
    IDWriteFontFace* faces[FONT_COUNT] = {};
    FontMetrics metrics[FONT_COUNT] = {};
    float advances[MAX_MEASURED_LENGTH] = {};
};
