#include "CivPickerView.h"

#include <windowsx.h>

#include "Assertions.h"
#include "CivPickerLayout.h"

// The DLC toggles of the main window's Civ Pool tab (CivPickerView): each edition's list where its dlcToggleLayout in
// data.json puts it. A list with more toggles than fit above the civ grid shows as many as fit and scrolls by its
// scrollbar or the mouse wheel. How many fit follows from the layout, so a new DLC in data.json needs nothing here.

namespace
{
    // The scrollbar at the client area's right edge; the toggles stop short of it.
    constexpr int DLC_SCROLLBAR_LEFT = CLIENT_WIDTH - SCROLLBAR_WIDTH - 4;
    constexpr int DLC_TOGGLE_RIGHT = DLC_SCROLLBAR_LEFT - 2;
    constexpr int SCROLLBAR_MIN_HEIGHT = 2 * SCROLLBAR_BUTTON_LENGTH + SCROLLBAR_KNOB_SIZE;
}

// Each edition's DLC toggles in the order of its dlcToggles, and a scrollbar for a list that doesn't fit.
bool CivPickerView::CreateDlcToggles()
{
    if (!c_assert(main_window != NULL)) return false;
    const CivCatalog& catalog = model.Catalog();
    for (int edition = 0; edition < catalog.EditionCount() && edition < MAX_EDITIONS; edition++)
    {
        const EditionInfo& info = catalog.EditionAt(edition);
        const DlcToggleLayout& layout = info.toggle_layout;
        for (int slot = 0; slot < info.dlc_toggle_count && slot < MAX_DLC_TOGGLES; slot++)
        {
            const DlcToggleSpot spot = { layout.left, layout.top + layout.step * slot, layout.size, layout.gap };
            if (!CreateDlcToggle(info.dlc_toggles[slot], spot)) return false;
        }
        if (!CreateDlcScrollbar(edition)) return false;
    }
    return true;
}

// A DLC toggle is as wide as its icon, the gap and its name in bold, the widest the name gets; a name too long for
// the room left of the scrollbar is cut short.
bool CivPickerView::CreateDlcToggle(int dlc, const DlcToggleSpot& spot)
{
    if (!c_assert(HasDlcToggle(dlc))) return false;
    if (!c_assert(spot.size > 0 && spot.gap >= 0) || !c_assert(spot.left + spot.size < DLC_TOGGLE_RIGHT)) return false;
    const CivCatalog& catalog = model.Catalog();
    const DlcInfo& info = catalog.DlcAt(dlc);
    if (!c_assert(info.icon_index != CivCatalog::NONE)) return false;
    const TextSpan name = { info.name.c_str(), static_cast<int>(info.name.size()), FontId::ui_bold };
    const int width = spot.size + spot.gap + static_cast<int>(renderer.MeasureText(name)) + 2;
    const int right = spot.left + width < DLC_TOGGLE_RIGHT ? spot.left + width : DLC_TOGGLE_RIGHT;
    const int atlas_cell = EDITION_LOOKS * catalog.EditionCount() + info.icon_index;
    IconToggle& toggle = dlc_toggles[dlc];
    toggle.SetIcon({ ImageId::dlc_atlas, ImageId::dlc_atlas_grey, DLC_ATLAS_COLUMNS, DLC_ATLAS_CELL, atlas_cell,
                     spot.size, spot.gap, false });
    toggle.SetChecked(true);
    const PixelRect area = { spot.left, spot.top, right, spot.top + spot.size };
    const ControlContext context = { &renderer, &back_buffer, text_menu };
    return toggle.Create(context, PlaceInClient(DLC_TOGGLE_BASE_ID + dlc, area, info.name.c_str()));
}

// A list that doesn't fit gets a scrollbar at the client area's right edge, beside the rows it shows.
bool CivPickerView::CreateDlcScrollbar(int edition)
{
    const EditionInfo& info = model.Catalog().EditionAt(edition);
    const int rows = VisibleDlcRows(edition);
    if (info.dlc_toggle_count <= rows) return true;
    if (!c_assert(rows > 0) || !c_assert(edition >= 0 && edition < MAX_EDITIONS)) return false;
    const int top = info.toggle_layout.top;
    const int bottom = top + DlcListHeight(edition);
    const PixelRect area = { DLC_SCROLLBAR_LEFT, top, DLC_SCROLLBAR_LEFT + SCROLLBAR_WIDTH, bottom };
    ScrollBar& scrollbar = dlc_scrollbars[edition];
    scrollbar.SetRows({ info.dlc_toggle_count, rows, 0 });
    const ControlContext context = { &renderer, &back_buffer, text_menu };
    if (!scrollbar.Create(context, PlaceInClient(DLC_SCROLLBAR_BASE_ID + edition, area, L""))) return false;
    // The scrollbar shows with its edition's toggles.
    ShowControl(scrollbar.Window(), false);
    return true;
}

// How many of the edition's toggles fit from their layout's top down to the civ grid; all of them if they fit.
int CivPickerView::VisibleDlcRows(int edition) const
{
    const EditionInfo& info = model.Catalog().EditionAt(edition);
    const DlcToggleLayout& layout = info.toggle_layout;
    const int room_below_first_row = DLC_LIST_BOTTOM - layout.top - layout.size;
    const int fitting = layout.step > 0 && room_below_first_row >= 0 ? room_below_first_row / layout.step + 1 : 1;
    if (!c_assert(fitting >= 1)) return 1;
    return info.dlc_toggle_count < fitting ? info.dlc_toggle_count : fitting;
}

// The height of the rows the edition's list shows, at least the height its scrollbar needs.
int CivPickerView::DlcListHeight(int edition) const
{
    const DlcToggleLayout& layout = model.Catalog().EditionAt(edition).toggle_layout;
    const int rows = VisibleDlcRows(edition);
    const int rows_height = rows > 0 ? layout.step * (rows - 1) + layout.size : 0;
    return rows_height > SCROLLBAR_MIN_HEIGHT ? rows_height : SCROLLBAR_MIN_HEIGHT;
}

// Where the edition's list lies in the main window, with its scrollbar.
PixelRect CivPickerView::DlcListArea(int edition) const
{
    const DlcToggleLayout& layout = model.Catalog().EditionAt(edition).toggle_layout;
    const PixelRect area = { layout.left, layout.top, CLIENT_WIDTH, layout.top + DlcListHeight(edition) };
    return OffsetRect(area, { CLIENT_LEFT, CLIENT_TOP });
}

// Only the edition's own list shows.
void CivPickerView::ShowDlcToggles(int edition)
{
    const CivCatalog& catalog = model.Catalog();
    if (!c_assert(edition >= 0 && edition < catalog.EditionCount())) return;
    for (int other = 0; other < catalog.EditionCount() && other < MAX_EDITIONS; other++)
    {
        if (other != edition) HideDlcToggles(other);
    }
    LayOutDlcToggles(edition);
}

// Shows the rows the edition's list is scrolled to, each in its place from the layout's top, and hides the others.
void CivPickerView::LayOutDlcToggles(int edition)
{
    const CivCatalog& catalog = model.Catalog();
    if (!c_assert(edition >= 0 && edition < catalog.EditionCount() && edition < MAX_EDITIONS)) return;
    const EditionInfo& info = catalog.EditionAt(edition);
    const DlcToggleLayout& layout = info.toggle_layout;
    const int first = dlc_scrollbars[edition].FirstRow();
    const int rows = VisibleDlcRows(edition);
    for (int slot = 0; slot < info.dlc_toggle_count && slot < MAX_DLC_TOGGLES; slot++)
    {
        const HWND toggle = dlc_toggles[info.dlc_toggles[slot]].Window();
        const bool shown = slot >= first && slot < first + rows;
        const PixelPoint place = { CLIENT_LEFT + layout.left, CLIENT_TOP + layout.top + layout.step * (slot - first) };
        const UINT visibility = shown ? SWP_SHOWWINDOW : (SWP_HIDEWINDOW | SWP_NOMOVE);
        const UINT flags = SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | visibility;
        // A toggle that couldn't move keeps its place until the next layout.
        (void)SetWindowPos(toggle, NULL, place.x, place.y, 0, 0, flags);
    }
    const HWND scrollbar = dlc_scrollbars[edition].Window();
    if (scrollbar != NULL) ShowControl(scrollbar, true);
}

void CivPickerView::HideDlcToggles(int edition)
{
    const CivCatalog& catalog = model.Catalog();
    if (!c_assert(edition >= 0 && edition < catalog.EditionCount() && edition < MAX_EDITIONS)) return;
    const EditionInfo& info = catalog.EditionAt(edition);
    for (int slot = 0; slot < info.dlc_toggle_count && slot < MAX_DLC_TOGGLES; slot++)
        ShowControl(dlc_toggles[info.dlc_toggles[slot]].Window(), false);
    const HWND scrollbar = dlc_scrollbars[edition].Window();
    if (scrollbar != NULL) ShowControl(scrollbar, false);
}

// A DLC list's scrollbar moved: its edition's toggles follow.
LRESULT CivPickerView::OnScroll(LPARAM lParam)
{
    const HWND source = reinterpret_cast<HWND>(lParam);
    if (!c_assert(main_window != NULL) || source == NULL) return 0;
    const int edition_count = model.Catalog().EditionCount();
    for (int edition = 0; edition < edition_count && edition < MAX_EDITIONS; edition++)
    {
        if (source == dlc_scrollbars[edition].Window()) LayOutDlcToggles(edition);
    }
    return 0;
}

// The wheel scrolls the shown DLC list while the mouse is over it, as many rows a notch as the system says for
// lines, but at most a page.
LRESULT CivPickerView::OnMouseWheel(WPARAM wParam, LPARAM lParam)
{
    if (!c_assert(main_window != NULL) || current_tab != CIV_POOL_TAB) return 0;
    const int edition = model.GetEdition();
    if (!c_assert(edition >= 0 && edition < MAX_EDITIONS) || dlc_scrollbars[edition].Window() == NULL) return 0;
    POINT point = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
    if (!ScreenToClient(main_window, &point) || !RectContains(DlcListArea(edition), { point.x, point.y })) return 0;
    UINT lines_per_notch = 3;
    // SPI_GETWHEELSCROLLLINES only fails on a broken system; the default of 3 then stands.
    (void)SystemParametersInfoW(SPI_GETWHEELSCROLLLINES, 0, &lines_per_notch, 0);
    const int page = VisibleDlcRows(edition);
    const int rows_per_notch = lines_per_notch < static_cast<UINT>(page) ? static_cast<int>(lines_per_notch) : page;
    const int notches = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
    if (dlc_scrollbars[edition].ScrollBy(-notches * rows_per_notch)) LayOutDlcToggles(edition);
    return 0;
}
