#pragma once

#include "ParchmentControl.h"

class Label final : public ParchmentControl
{
public:
    struct Style
    {
        FontId font;
        Colour colour;
        TextAlign align;
        float line_height;
    };

    void SetStyle(const Style& label_style);

private:
    void Paint(ParchmentRenderer& renderer, const FloatRect& bounds) override;
    bool IsClickable() const override;

    Style style = { FontId::ui, TEXT_COLOUR, TextAlign::left, UI_LINE_HEIGHT };
};

class TabStrip final : public ParchmentControl
{
public:
    static constexpr int TAB_COUNT = 3;
    static constexpr int TAB_WIDTH = 70;
    static constexpr int TAB_HEIGHT = 25;
    static constexpr int TAB_STEP = 68;

    void SetNames(const wchar_t* const (&tab_names)[TAB_COUNT]);
    void Select(int tab_index);
    int Selected() const;

private:
    void Paint(ParchmentRenderer& renderer, const FloatRect& bounds) override;
    CursorId CursorShape() const override;
    bool ShowsFocus() const override;
    void OnPointerMoved(PixelPoint point) override;
    void OnPointerPressed(PixelPoint point) override;
    void OnPointerLeft() override;
    void OnClicked() override;
    void PaintTab(ParchmentRenderer& renderer, int tab_index);
    int TabAt(PixelPoint point) const;

    const wchar_t* names[TAB_COUNT] = { L"", L"", L"" };
    int selected = 0;
    int hovered = -1;
    int pressed_tab = -1;
};

class Checkbox final : public ParchmentControl
{
public:
    struct Style
    {
        int gap;
        bool radio;
        float line_height;
    };

    void SetStyle(const Style& checkbox_style);

private:
    void Paint(ParchmentRenderer& renderer, const FloatRect& bounds) override;
    void OnClicked() override;
    LRESULT DialogCode() const override;

    Style style = { 4, false, UI_LINE_HEIGHT };
};

class IconToggle final : public ParchmentControl
{
public:
    struct Icon
    {
        ImageId atlas;
        ImageId grey_atlas;
        int columns;
        int cell_size;
        int cell;
        int size;
        int gap;
        bool highlights;
    };

    void SetIcon(const Icon& toggle_icon);

private:
    void Paint(ParchmentRenderer& renderer, const FloatRect& bounds) override;
    void PaintIcon(ParchmentRenderer& renderer, const FloatRect& bounds);
    void PaintName(ParchmentRenderer& renderer, const FloatRect& bounds);
    void OnClicked() override;

    Icon icon = { ImageId::civ_atlas, ImageId::civ_atlas_grey, CIV_ATLAS_COLUMNS, CIV_ATLAS_CELL, 0, 18, 2, true };
};

class EditionRow final : public ParchmentControl
{
public:
    void SetEditionIndex(int atlas_edition_index);

private:
    void Paint(ParchmentRenderer& renderer, const FloatRect& bounds) override;
    void OnClicked() override;

    int edition_index = 0;
};
