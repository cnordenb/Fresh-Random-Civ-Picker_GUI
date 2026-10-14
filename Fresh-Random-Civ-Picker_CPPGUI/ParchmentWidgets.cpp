#include "ParchmentWidgets.h"

#include <cwchar>

#include "Assertions.h"
#include "CivDataLimits.h"

namespace
{
    constexpr int TAB_TEXT_HEIGHT = 22; // .tab { height: 25px; padding-bottom: 3px }
    constexpr float FOCUS_OFFSET = 2.0f; // outline-offset 1px plus the 1 px outline
    constexpr float OFF_CROSS_SCALE = 0.9f; // .chk-off { transform: scale(.9) }
    constexpr float STRIKE_THICKNESS = 2.0f; // text-decoration-thickness: 2px
    constexpr int EDITION_ICON_SIZE = 28; // .ed-icon
    constexpr int EDITION_ICON_GAP = 4; // .edition { gap: 4px }

    PixelRect AtlasCell(int column, int row, PixelSize cell)
    {
        if (!c_assert(column >= 0 && row >= 0) || !c_assert(cell.width > 0 && cell.height > 0)) return { 0, 0, 0, 0 };
        return { column * cell.width, row * cell.height, (column + 1) * cell.width, (row + 1) * cell.height };
    }

    // A square box of the given size, at the left of the bounds and centred vertically.
    FloatRect BoxAtLeft(const FloatRect& bounds, float size)
    {
        const FloatRect empty = { bounds.left, bounds.top, bounds.left, bounds.top };
        if (!c_assert(size > 0.0f) || !c_assert(HasArea(bounds))) return empty;
        const float top = bounds.top + (bounds.bottom - bounds.top - size) / 2.0f;
        return { bounds.left, top, bounds.left + size, top + size };
    }

    float CentredLineTop(const FloatRect& bounds, float line_height)
    {
        if (!c_assert(line_height > 0.0f)) return bounds.top;
        return bounds.top + (bounds.bottom - bounds.top - line_height) / 2.0f;
    }
}

// --- Label ---

void Label::SetStyle(const Style& label_style)
{
    if (!c_assert(label_style.line_height > 0.0f)) return;
    if (!c_assert(label_style.colour.alpha >= 0.0f && label_style.colour.alpha <= 1.0f)) return;
    style = label_style;
    Redraw();
}

void Label::Paint(ParchmentRenderer& renderer, const FloatRect& bounds)
{
    if (!c_assert(HasArea(bounds))) return;
    const float baseline = renderer.BaselineInLine(style.font, bounds.top, style.line_height);
    renderer.DrawText({ Text(style.font), style.colour, bounds.left, bounds.right, baseline, style.align });
}

bool Label::IsClickable() const { return false; }

// --- TabStrip ---

void TabStrip::SetNames(const wchar_t* const (&tab_names)[TAB_COUNT])
{
    for (int i = 0; i < TAB_COUNT; i++)
    {
        if (!c_assert(tab_names[i] != nullptr)) return;
        names[i] = tab_names[i];
    }
    Redraw();
}

void TabStrip::Select(int tab_index)
{
    if (!c_assert(tab_index >= 0 && tab_index < TAB_COUNT) || tab_index == selected) return;
    selected = tab_index;
    Redraw();
}

int TabStrip::Selected() const { return selected; }

void TabStrip::Paint(ParchmentRenderer& renderer, const FloatRect& bounds)
{
    if (!c_assert(bounds.right - bounds.left >= TAB_STEP * (TAB_COUNT - 1) + TAB_WIDTH)) return;
    if (!c_assert(selected >= 0 && selected < TAB_COUNT)) return;
    for (int i = 0; i < TAB_COUNT; i++)
    {
        if (i != selected) PaintTab(renderer, i);
    }
    PaintTab(renderer, selected);
}

// Draw Civ and Civ Pool use the first tab image and Log the second (.tab, #tab-1).
void TabStrip::PaintTab(ParchmentRenderer& renderer, int tab_index)
{
    if (!c_assert(tab_index >= 0 && tab_index < TAB_COUNT) || !c_assert(names[tab_index] != nullptr)) return;
    const bool is_selected = tab_index == selected;
    const float left = static_cast<float>(tab_index * TAB_STEP);
    const FloatRect area = { left, 0.0f, left + TAB_WIDTH, static_cast<float>(TAB_HEIGHT) };
    const PixelRect source = AtlasCell(hovered == tab_index ? 1 : 0, tab_index == 1 ? 1 : 0,
                                       { TAB_CELL_WIDTH, TAB_CELL_HEIGHT });
    renderer.DrawImage({ is_selected ? ImageId::tabs : ImageId::tabs_dimmed, source, area, 1.0f,
                         ParchmentRenderer::Smoothing::smooth });
    const FontId font = is_selected ? FontId::ui_bold : FontId::ui;
    const TextSpan name = { names[tab_index], static_cast<int>(wcslen(names[tab_index])), font };
    const float baseline = renderer.BaselineCentred(font, 0.0f, static_cast<float>(TAB_TEXT_HEIGHT));
    renderer.DrawText({ name, TEXT_COLOUR, area.left, area.right, baseline, TextAlign::centre });
}

CursorId TabStrip::CursorShape() const { return CursorId::arrow; }

bool TabStrip::ShowsFocus() const { return false; }

// The selected tab lies on top, and of the others each lies over the one before it.
int TabStrip::TabAt(PixelPoint point) const
{
    if (!c_assert(selected >= 0 && selected < TAB_COUNT)) return -1;
    if (point.y < 0 || point.y >= TAB_HEIGHT) return -1;
    const int selected_left = selected * TAB_STEP;
    if (point.x >= selected_left && point.x < selected_left + TAB_WIDTH) return selected;
    for (int i = TAB_COUNT - 1; i >= 0; i--)
    {
        if (point.x >= i * TAB_STEP && point.x < i * TAB_STEP + TAB_WIDTH) return i;
    }
    return -1;
}

void TabStrip::OnPointerMoved(PixelPoint point)
{
    const int tab_under_pointer = TabAt(point);
    if (tab_under_pointer == hovered) return;
    hovered = tab_under_pointer;
    Redraw();
}

void TabStrip::OnPointerPressed(PixelPoint point) { pressed_tab = TabAt(point); }

void TabStrip::OnPointerLeft() { hovered = -1; }

void TabStrip::OnClicked()
{
    // pressed_tab is -1 when the press was between the tabs.
    if (!c_assert(pressed_tab < TAB_COUNT)) return;
    if (pressed_tab >= 0 && pressed_tab < TAB_COUNT) selected = pressed_tab;
}

// --- Checkbox ---

void Checkbox::SetStyle(const Style& checkbox_style)
{
    if (!c_assert(checkbox_style.gap >= 0) || !c_assert(checkbox_style.line_height > 0.0f)) return;
    style = checkbox_style;
    Redraw();
}

void Checkbox::Paint(ParchmentRenderer& renderer, const FloatRect& bounds)
{
    if (!c_assert(HasArea(bounds))) return;
    const Look look = CurrentLook();
    const FloatRect box = BoxAtLeft(bounds, static_cast<float>(CHECKBOX_SIZE));
    const PixelRect cell = AtlasCell(look.pressed ? 1 : 0, look.checked ? 1 : 0, { CHECKBOX_CELL, CHECKBOX_CELL });
    renderer.DrawImage({ ImageId::checkbox, cell, box, 1.0f, ParchmentRenderer::Smoothing::smooth });

    const float text_left = box.right + static_cast<float>(style.gap);
    const float baseline =
        renderer.BaselineInLine(FontId::ui, CentredLineTop(bounds, style.line_height), style.line_height);
    renderer.DrawText({ Text(FontId::ui), TEXT_COLOUR, text_left, bounds.right, baseline, TextAlign::left });
    if (!look.focused) return;
    renderer.DrawDottedFrame(
        { box.left - FOCUS_OFFSET, box.top - FOCUS_OFFSET, box.right + FOCUS_OFFSET, box.bottom + FOCUS_OFFSET },
        FOCUS_COLOUR);
}

void Checkbox::OnClicked() { Check(style.radio ? true : !IsChecked()); }

LRESULT Checkbox::DialogCode() const { return style.radio ? DLGC_BUTTON | DLGC_RADIOBUTTON : DLGC_BUTTON; }

// --- IconToggle ---

void IconToggle::SetIcon(const Icon& toggle_icon)
{
    if (!c_assert(toggle_icon.columns > 0 && toggle_icon.cell_size > 0)) return;
    if (!c_assert(toggle_icon.size > 0 && toggle_icon.gap >= 0) || !c_assert(toggle_icon.cell >= 0)) return;
    icon = toggle_icon;
    Redraw();
}

void IconToggle::Paint(ParchmentRenderer& renderer, const FloatRect& bounds)
{
    if (!c_assert(HasArea(bounds))) return;
    const Look look = CurrentLook();
    if (look.hot && icon.highlights) renderer.FillRect(bounds, look.checked ? HOVER_GOLD_COLOUR : HOVER_RED_COLOUR);
    PaintIcon(renderer, bounds);
    PaintName(renderer, bounds);
}

// The atlas cell, grey under the off cross while disabled (.chk-off: object-fit contain, scaled to 90%).
void IconToggle::PaintIcon(ParchmentRenderer& renderer, const FloatRect& bounds)
{
    if (!c_assert(HasArea(bounds)) || !c_assert(icon.size > 0 && icon.columns > 0)) return;
    const FloatRect box = BoxAtLeft(bounds, static_cast<float>(icon.size));
    const bool enabled = IsChecked();
    renderer.DrawAtlasCell({ enabled ? icon.atlas : icon.grey_atlas, icon.cell, icon.columns,
                             { icon.cell_size, icon.cell_size }, box });
    if (enabled) return;

    const PixelSize cross = renderer.ImageSize(ImageId::off_cross);
    if (cross.width <= 0 || cross.height <= 0) return;
    const float side = box.right - box.left;
    const float fit = side / static_cast<float>(cross.width > cross.height ? cross.width : cross.height);
    const float width = static_cast<float>(cross.width) * fit * OFF_CROSS_SCALE;
    const float height = static_cast<float>(cross.height) * fit * OFF_CROSS_SCALE;
    const float centre_x = (box.left + box.right) / 2.0f;
    const float centre_y = (box.top + box.bottom) / 2.0f;
    renderer.DrawImage({ ImageId::off_cross, { 0, 0, cross.width, cross.height },
                         { centre_x - width / 2.0f, centre_y - height / 2.0f, centre_x + width / 2.0f,
                           centre_y + height / 2.0f },
                         1.0f, ParchmentRenderer::Smoothing::smooth });
}

void IconToggle::PaintName(ParchmentRenderer& renderer, const FloatRect& bounds)
{
    if (!c_assert(HasArea(bounds))) return;
    const bool enabled = IsChecked();
    const FontId font = enabled ? FontId::ui_bold : FontId::ui_italic;
    const TextSpan name = Text(font);
    const float left = bounds.left + static_cast<float>(icon.size + icon.gap);
    const float baseline = renderer.BaselineInLine(font, CentredLineTop(bounds, UI_LINE_HEIGHT), UI_LINE_HEIGHT);
    renderer.DrawText({ name, TEXT_COLOUR, left, bounds.right, baseline, TextAlign::left });
    if (enabled) return;
    const float strike_centre = baseline - renderer.StrikeOffset(font);
    const float width = renderer.MeasureText(name);
    renderer.FillRect({ left, strike_centre - STRIKE_THICKNESS / 2.0f, left + width,
                        strike_centre + STRIKE_THICKNESS / 2.0f },
                      TEXT_COLOUR);
}

void IconToggle::OnClicked() { Check(!IsChecked()); }

// --- EditionRow ---

void EditionRow::SetEditionIndex(int atlas_edition_index)
{
    if (!c_assert(atlas_edition_index >= 0 && atlas_edition_index < MAX_EDITIONS)) return;
    edition_index = atlas_edition_index;
    Redraw();
}

void EditionRow::Paint(ParchmentRenderer& renderer, const FloatRect& bounds)
{
    if (!c_assert(HasArea(bounds)) || !c_assert(edition_index >= 0 && edition_index < MAX_EDITIONS)) return;
    const Look look = CurrentLook();
    int shown_look = 0;
    if (look.checked || look.pressed) shown_look = 2;
    else if (look.hot) shown_look = 1;
    const FloatRect box = BoxAtLeft(bounds, static_cast<float>(EDITION_ICON_SIZE));
    renderer.DrawAtlasCell({ ImageId::dlc_atlas, EDITION_LOOKS * edition_index + shown_look, DLC_ATLAS_COLUMNS,
                             { DLC_ATLAS_CELL, DLC_ATLAS_CELL }, box });

    const FontId font = look.checked ? FontId::ui_bold : FontId::ui;
    const float left = box.right + static_cast<float>(EDITION_ICON_GAP);
    const float baseline = renderer.BaselineInLine(font, CentredLineTop(bounds, UI_LINE_HEIGHT), UI_LINE_HEIGHT);
    renderer.DrawText({ Text(font), TEXT_COLOUR, left, bounds.right, baseline, TextAlign::left });
}

void EditionRow::OnClicked() { Check(true); }
