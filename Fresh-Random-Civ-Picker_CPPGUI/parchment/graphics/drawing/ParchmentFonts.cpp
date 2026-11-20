#include "ParchmentFonts.h"

#include <dwrite_1.h>
#include <algorithm>
#include <cmath>

#include "Assertions.h"

namespace
{
    constexpr int MEASURE_CHUNK = 256;

    // The fonts of style.css: Georgia for all text, and for the wooden buttons the first installed of
    // "Lucida Blackletter", "Old English Text MT" and the browser's default serif font.
    const wchar_t* const UI_FAMILY = L"Georgia";
    const wchar_t* const BLACKLETTER_FAMILIES[] = { L"Lucida Blackletter", L"Old English Text MT", L"Times New Roman" };
    const wchar_t* const LOCALE = L"en-us";

    const DWRITE_TEXT_ALIGNMENT ALIGNMENTS[3] = { DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_TEXT_ALIGNMENT_CENTER,
                                                  DWRITE_TEXT_ALIGNMENT_TRAILING };

    template <typename Interface> void ReleaseInterface(Interface*& pointer)
    {
        if (pointer != nullptr) (void)pointer->Release();
        pointer = nullptr;
    }

    bool FamilyInstalled(IDWriteFontCollection* fonts, const wchar_t* family)
    {
        UINT32 index = 0;
        BOOL exists = FALSE;
        return SUCCEEDED(fonts->FindFamilyName(family, &index, &exists)) && exists == TRUE;
    }
}

bool ParchmentFonts::Load()
{
    if (!c_assert(factory == nullptr)) return false;
    const HRESULT result = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
                                               reinterpret_cast<IUnknown**>(&factory));
    if (!c_assert(SUCCEEDED(result))) return false;
    const FontSpec specs[FONT_COUNT] = {
        { FontId::ui, 12.0f, false, false, false },     { FontId::ui_bold, 12.0f, true, false, false },
        { FontId::ui_italic, 12.0f, false, true, false }, { FontId::civ_name, 13.0f, true, false, false },
        { FontId::button, 14.0f, false, false, true },  { FontId::key, 11.0f, false, false, false },
    };
    for (int i = 0; i < FONT_COUNT; i++)
    {
        if (!c_assert(static_cast<int>(specs[i].font) == i) || !LoadFont(specs[i])) return false;
    }
    return true;
}

bool ParchmentFonts::LoadFont(const FontSpec& spec)
{
    const int index = static_cast<int>(spec.font);
    const wchar_t* family = spec.blackletter ? BlackletterFamily() : UI_FAMILY;
    if (!c_assert(family != nullptr) || !c_assert(index >= 0 && index < FONT_COUNT)) return false;
    const DWRITE_FONT_WEIGHT weight = spec.bold ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL;
    const DWRITE_FONT_STYLE style = spec.italic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL;
    for (int align = 0; align < 3; align++)
    {
        IDWriteTextFormat*& format = formats[index][align];
        HRESULT result = factory->CreateTextFormat(family, nullptr, weight, style, DWRITE_FONT_STRETCH_NORMAL,
                                                   spec.size, LOCALE, &format);
        if (SUCCEEDED(result)) result = format->SetTextAlignment(ALIGNMENTS[align]);
        if (SUCCEEDED(result)) result = format->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
        if (SUCCEEDED(result)) result = format->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
        if (!c_assert(SUCCEEDED(result))) return false;
    }
    return LoadFontFace(spec, family);
}

// The font face measures text; its metrics place text the way a browser does (ascent and descent rounded).
bool ParchmentFonts::LoadFontFace(const FontSpec& spec, const wchar_t* family)
{
    IDWriteFontCollection* fonts = nullptr;
    IDWriteFontFamily* font_family = nullptr;
    IDWriteFont* font = nullptr;
    UINT32 family_index = 0;
    BOOL exists = FALSE;
    HRESULT result = factory->GetSystemFontCollection(&fonts);
    if (SUCCEEDED(result)) result = fonts->FindFamilyName(family, &family_index, &exists);
    if (SUCCEEDED(result) && exists == TRUE) result = fonts->GetFontFamily(family_index, &font_family);
    if (SUCCEEDED(result) && font_family != nullptr)
        result = font_family->GetFirstMatchingFont(spec.bold ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL,
                                                   DWRITE_FONT_STRETCH_NORMAL,
                                                   spec.italic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL,
                                                   &font);
    const int index = static_cast<int>(spec.font);
    if (SUCCEEDED(result) && font != nullptr) result = font->CreateFontFace(&faces[index]);
    ReleaseInterface(font);
    ReleaseInterface(font_family);
    ReleaseInterface(fonts);
    if (!c_assert(SUCCEEDED(result)) || !c_assert(faces[index] != nullptr)) return false;

    DWRITE_FONT_METRICS design = {};
    faces[index]->GetMetrics(&design);
    const float scale = spec.size / static_cast<float>(design.designUnitsPerEm);
    metrics[index] = { spec.size,
                       static_cast<float>(design.ascent) * scale,
                       static_cast<float>(design.descent) * scale,
                       static_cast<float>(design.strikethroughPosition) * scale,
                       -static_cast<float>(design.underlinePosition) * scale,
                       static_cast<float>(design.underlineThickness) * scale,
                       static_cast<float>(design.designUnitsPerEm) / spec.size };
    return true;
}

const wchar_t* ParchmentFonts::BlackletterFamily() const
{
    IDWriteFontCollection* fonts = nullptr;
    if (!c_assert(SUCCEEDED(factory->GetSystemFontCollection(&fonts)))) return BLACKLETTER_FAMILIES[2];
    const wchar_t* chosen = BLACKLETTER_FAMILIES[2];
    for (const wchar_t* family : BLACKLETTER_FAMILIES)
    {
        if (!FamilyInstalled(fonts, family)) continue;
        chosen = family;
        break;
    }
    ReleaseInterface(fonts);
    return chosen;
}

void ParchmentFonts::Release()
{
    for (int font = 0; font < FONT_COUNT; font++)
    {
        for (int align = 0; align < 3; align++) ReleaseInterface(formats[font][align]);
        ReleaseInterface(faces[font]);
    }
    ReleaseInterface(factory);
}

IDWriteTextFormat* ParchmentFonts::Format(FontId font, TextAlign align) const
{
    const int index = static_cast<int>(font);
    const int align_index = static_cast<int>(align);
    if (!c_assert(index >= 0 && index < FONT_COUNT) || !c_assert(align_index >= 0 && align_index < 3)) return nullptr;
    return formats[index][align_index];
}

// Every metric query goes through here, so an out-of-range font falls back to the UI font.
const ParchmentFonts::FontMetrics& ParchmentFonts::MetricsOf(FontId font) const
{
    const int index = static_cast<int>(font);
    if (!c_assert(index >= 0 && index < FONT_COUNT)) return metrics[0];
    return metrics[index];
}

float ParchmentFonts::Size(FontId font) const { return MetricsOf(font).size; }

float ParchmentFonts::Ascent(FontId font) const { return MetricsOf(font).ascent; }

float ParchmentFonts::LayoutAscent(FontId font) const { return std::round(MetricsOf(font).ascent); }

float ParchmentFonts::LayoutDescent(FontId font) const { return std::round(MetricsOf(font).descent); }

float ParchmentFonts::StrikeOffset(FontId font) const { return MetricsOf(font).strike_offset; }

float ParchmentFonts::UnderlineOffset(FontId font) const { return MetricsOf(font).underline_offset; }

float ParchmentFonts::UnderlineThickness(FontId font) const { return MetricsOf(font).underline_thickness; }

float ParchmentFonts::Measure(const TextSpan& span)
{
    const float* widths = Advances(span);
    float width = 0.0f;
    for (int i = 0; i < span.length && i < MAX_MEASURED_LENGTH; i++) width += widths[i];
    return width;
}

// The advance of every character of the span, valid until the next call.
const float* ParchmentFonts::Advances(const TextSpan& span)
{
    const bool valid = (span.text != nullptr || span.length == 0) && span.length >= 0 &&
                       span.length <= MAX_MEASURED_LENGTH;
    if (!c_assert(valid)) return advances;
    for (int first = 0; first < span.length && first < MAX_MEASURED_LENGTH; first += MEASURE_CHUNK)
    {
        if (!MeasureChunk(span, first)) break;
    }
    return advances;
}

bool ParchmentFonts::MeasureChunk(const TextSpan& span, int first)
{
    const int index = static_cast<int>(span.font);
    if (!c_assert(index >= 0 && index < FONT_COUNT) || !c_assert(faces[index] != nullptr)) return false;
    const int remaining = span.length - first;
    const int count = std::min(remaining, MEASURE_CHUNK);
    UINT32 code_points[MEASURE_CHUNK] = {};
    UINT16 glyphs[MEASURE_CHUNK] = {};
    DWRITE_GLYPH_METRICS glyph_metrics[MEASURE_CHUNK] = {};
    INT32 kerning[MEASURE_CHUNK] = {};
    for (int i = 0; i < count; i++) code_points[i] = static_cast<UINT32>(span.text[first + i]);

    IDWriteFontFace* face = faces[index];
    HRESULT result = face->GetGlyphIndices(code_points, static_cast<UINT32>(count), glyphs);
    if (SUCCEEDED(result))
        result = face->GetDesignGlyphMetrics(glyphs, static_cast<UINT32>(count), glyph_metrics, FALSE);
    if (!c_assert(SUCCEEDED(result))) return false;
    IDWriteFontFace1* kerning_face = nullptr;
    if (SUCCEEDED(face->QueryInterface(__uuidof(IDWriteFontFace1), reinterpret_cast<void**>(&kerning_face))))
    {
        // Text without kerning pairs measures a little wide at worst, so a failure here is no reason to stop.
        if (kerning_face->HasKerningPairs())
            (void)kerning_face->GetKerningPairAdjustments(static_cast<UINT32>(count), glyphs, kerning);
        ReleaseInterface(kerning_face);
    }
    const float units_per_pixel = metrics[index].design_units_per_pixel;
    for (int i = 0; i < count; i++)
    {
        const float design_advance = static_cast<float>(glyph_metrics[i].advanceWidth) + static_cast<float>(kerning[i]);
        advances[first + i] = design_advance / units_per_pixel;
    }
    return true;
}
