#pragma once

#include "ParchmentControl.h"

// Labels, tabs and the checkbox-like controls of the parchment theme (View), drawn as their web version
// counterparts (style.css).

// One line of text (.label); the text is the window text, so SetWindowText changes it.
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

// The three tabs (.tabstrip): each 70 x 25 from the tabs image, overlapping by 2 px, with the selected one on top
// in bold at full brightness and the others dimmed. Clicking a tab selects it and then tells the parent.
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

// A checkbox with its label (.chk, and .row in dialogs): the wooden box ticked or not, pressed while held. As a
// radio button it only ever ticks itself; the view unticks the others of its group.
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

// A civ or DLC toggle (.icon-chk): its icon stands in for the checkbox. Enabled, the name is bold; disabled, the
// icon is grey under the off cross and the name italic and struck through. Civ toggles highlight on hover, gold
// while enabled and red while disabled.
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

// An edition choice (.edition): the edition's icon in place of a radio circle, which shows its hover look and,
// while pressed or chosen, its pressed look; the chosen edition's name is bold.
class EditionRow final : public ParchmentControl
{
public:
    void SetEditionIndex(int atlas_edition_index);

private:
    void Paint(ParchmentRenderer& renderer, const FloatRect& bounds) override;
    void OnClicked() override;

    int edition_index = 0;
};
