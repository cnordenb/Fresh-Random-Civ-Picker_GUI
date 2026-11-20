#include "ParchmentMenu.h"

#include <windowsx.h>
#include <algorithm>
#include <cmath>
#include <cwchar>
#include <cwctype>

#include "Assertions.h"
#include "ParchmentShell.h"

namespace
{
    const wchar_t* const MENU_BAR_CLASS = L"FrcpParchmentMenuBar";
    constexpr float TITLE_PADDING = 7.0f;
    constexpr float TITLE_BORDER = 1.0f;
    constexpr int LIST_BORDER = 1;
    constexpr int LIST_PADDING = 2;
    constexpr int LIST_MIN_WIDTH = 250;
    constexpr float ITEM_PADDING_Y = 3.0f;
    constexpr float ITEM_PADDING_LEFT = 30.0f;
    constexpr float ITEM_PADDING_RIGHT = 18.0f;
    constexpr float ITEM_GAP = 24.0f;
    constexpr float SEPARATOR_HEIGHT = 7.0f;
    constexpr float SEPARATOR_MARGIN = 3.0f;
    constexpr float SEPARATOR_RIGHT = 2.0f;
    constexpr float ICON_LEFT = 9.0f;
    constexpr int SHADOW_MARGIN = 6;
    constexpr PixelPoint SHADOW_OFFSET = { 2, 2 };
    constexpr float SHADOW_SIGMA = 1.5f;
    constexpr float SHADOW_OPACITY = 0.25f;
    constexpr PixelSize MAX_LIST_SIZE = { 420, 340 };

    HMENU ControlId(int id) { return reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)); }

    int TextLength(const wchar_t* text) { return text != nullptr ? static_cast<int>(wcslen(text)) : 0; }

    bool IsSeparator(const MenuItemSpec& item) { return item.label == nullptr; }
}

bool ParchmentMenuBar::RegisterWindowClass(HINSTANCE instance)
{
    if (!c_assert(instance != NULL)) return false;
    WNDCLASSEXW class_info = {};
    class_info.cbSize = sizeof(class_info);
    class_info.lpfnWndProc = WindowProc;
    class_info.hInstance = instance;
    class_info.lpszClassName = MENU_BAR_CLASS;
    return c_assert(RegisterClassExW(&class_info) != 0);
}

bool ParchmentMenuBar::Create(const ControlContext& bar_context, const ControlPlacement& placement, const Menus& specs)
{
    if (!c_assert(window == NULL) || !c_assert(placement.parent != NULL && placement.backdrop != nullptr)) return false;
    if (!c_assert(specs.specs != nullptr && specs.count > 0 && specs.count <= MAX_MENUS)) return false;
    context = bar_context;
    parent = placement.parent;
    backdrop = placement.backdrop;
    menus = specs;
    size = { RectWidth(placement.bounds), RectHeight(placement.bounds) };
    if (!LayOut()) return false;
    if (!list_surface.Create({ MAX_LIST_SIZE.width + 2 * SHADOW_MARGIN, MAX_LIST_SIZE.height + 2 * SHADOW_MARGIN }))
        return false;
    const HINSTANCE instance = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(parent, GWLP_HINSTANCE));
    window = CreateWindowExW(0, MENU_BAR_CLASS, L"", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, placement.bounds.left,
                             placement.bounds.top, size.width, size.height, parent, ControlId(placement.id), instance,
                             this);
    const DWORD list_style = WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW;
    list_window = CreateWindowExW(list_style, ParchmentShell::LayerClass(), L"", ParchmentShell::LAYER_STYLE, 0, 0,
                                  1, 1, parent, NULL, instance, NULL);
    return c_assert(window != NULL && list_window != NULL);
}

void ParchmentMenuBar::Destroy()
{
    if (list_window != NULL) (void)DestroyWindow(list_window);
    list_window = NULL;
    list_surface.Destroy();
}

HWND ParchmentMenuBar::Window() const { return window; }

bool ParchmentMenuBar::IsOpen() const { return open_menu >= 0; }

bool ParchmentMenuBar::LayOut()
{
    if (!c_assert(context.renderer != nullptr)) return false;
    float left = 0.0f;
    for (int i = 0; i < menus.count && i < MAX_MENUS; i++)
    {
        const MenuSpec& menu = menus.specs[i];
        if (!c_assert(menu.title != nullptr)) return false;
        const float text_width = context.renderer->MeasureText({ menu.title, TextLength(menu.title), FontId::ui });
        titles[i] = { left, text_width + 2.0f * (TITLE_PADDING + TITLE_BORDER) };
        left += titles[i].width;
        if (!LayOutList(i)) return false;
    }
    return true;
}

bool ParchmentMenuBar::LayOutList(int menu_index)
{
    if (!c_assert(menu_index >= 0 && menu_index < menus.count && menu_index < MAX_MENUS)) return false;
    const MenuSpec& menu = menus.specs[menu_index];
    if (!c_assert(menu.items != nullptr && menu.item_count > 0 && menu.item_count <= MAX_ITEMS)) return false;
    ListLayout& list = lists[menu_index];
    float top = static_cast<float>(LIST_BORDER + LIST_PADDING);
    float widest = 0.0f;
    for (int i = 0; i < menu.item_count; i++)
    {
        const MenuItemSpec& item = menu.items[i];
        const bool separator = IsSeparator(item);
        list.item_top[i] = top;
        list.item_height[i] = separator ? SEPARATOR_HEIGHT : 2.0f * ITEM_PADDING_Y + UI_LINE_HEIGHT;
        top += list.item_height[i];
        if (separator) continue;
        const float label = context.renderer->MeasureText({ item.label, TextLength(item.label), FontId::ui });
        const float shortcut = context.renderer->MeasureText({ item.shortcut, TextLength(item.shortcut), FontId::ui });
        const float width = ITEM_PADDING_LEFT + label + ITEM_GAP + shortcut + ITEM_PADDING_RIGHT;
        if (width > widest) widest = width;
    }
    const int frame = 2 * (LIST_BORDER + LIST_PADDING);
    const int fitted = static_cast<int>(std::ceil(widest)) + frame;
    list.size = { std::max(fitted, LIST_MIN_WIDTH),
                  static_cast<int>(std::lround(top)) + LIST_BORDER + LIST_PADDING };
    return c_assert(list.size.width <= MAX_LIST_SIZE.width && list.size.height <= MAX_LIST_SIZE.height);
}

LRESULT CALLBACK ParchmentMenuBar::WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_NCCREATE)
    {
        const CREATESTRUCT* create_struct = reinterpret_cast<const CREATESTRUCT*>(lParam);
        if (create_struct == nullptr) return FALSE;
        (void)SetWindowLongPtr(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(create_struct->lpCreateParams));
    }
    ParchmentMenuBar* bar = reinterpret_cast<ParchmentMenuBar*>(GetWindowLongPtr(window, GWLP_USERDATA));
    if (bar == nullptr) return DefWindowProc(window, message, wParam, lParam);
    if (message == WM_NCCREATE) bar->window = window;
    return bar->HandleMessage(message, wParam, lParam);
}

LRESULT ParchmentMenuBar::HandleMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
    if (!c_assert(window != NULL)) return 0;
    switch (message)
    {
        case WM_PAINT: OnPaint(); return 0;
        case WM_ERASEBKGND: return 1;
        case WM_MOUSEMOVE: OnMouseMove(lParam); return 0;
        case WM_MOUSELEAVE: OnMouseLeave(); return 0;
        case WM_LBUTTONDOWN: OnButtonDown(lParam); return 0;
        case WM_LBUTTONUP: OnButtonUp(lParam); return 0;
        case WM_CAPTURECHANGED: OnCaptureLost(); return 0;
        case WM_SETCURSOR: return OnSetCursor(wParam, lParam);
        default: return DefWindowProc(window, message, wParam, lParam);
    }
}

void ParchmentMenuBar::OnPaint()
{
    if (!c_assert(window != NULL && parent != NULL)) return;
    PAINTSTRUCT paint = {};
    const HDC target = BeginPaint(window, &paint);
    PixelSurface* buffer = context.back_buffer;
    ParchmentRenderer* renderer = context.renderer;
    POINT origin = { 0, 0 };
    (void)MapWindowPoints(window, parent, &origin, 1);
    const bool ready = target != NULL && buffer != nullptr && renderer != nullptr && backdrop != nullptr;
    if (ready && c_assert(buffer->Contains({ 0, 0, size.width, size.height })) &&
        BitBlt(buffer->Dc(), 0, 0, size.width, size.height, backdrop->Dc(), origin.x, origin.y, SRCCOPY) &&
        renderer->BeginFrame({ buffer->Dc(), size, ParchmentRenderer::SurfaceAlpha::opaque }))
    {
        PaintTitles(*renderer);
        if (renderer->EndFrame())
            (void)BitBlt(target, 0, 0, size.width, size.height, buffer->Dc(), 0, 0, SRCCOPY);
    }
    (void)EndPaint(window, &paint);
}

void ParchmentMenuBar::PaintTitles(ParchmentRenderer& renderer)
{
    if (!c_assert(size.height > 2 * static_cast<int>(TITLE_BORDER))) return;
    const float height = static_cast<float>(size.height);
    const float baseline = renderer.BaselineCentred(FontId::ui, TITLE_BORDER, height - 2.0f * TITLE_BORDER);
    for (int i = 0; i < menus.count && i < MAX_MENUS; i++)
    {
        const TitleLayout& title = titles[i];
        const FloatRect area = { std::round(title.left), 0.0f, std::round(title.left + title.width), height };
        Colour text_colour = TEXT_COLOUR;
        if (i == open_menu)
        {
            renderer.FillRect(area, MENU_COLOUR);
            text_colour = MENU_TEXT_COLOUR;
        }
        else if (i == hovered_title) renderer.FillRect(area, HOVER_GOLD_COLOUR);
        const MenuSpec& menu = menus.specs[i];
        const float text_left = title.left + TITLE_BORDER + TITLE_PADDING;
        renderer.DrawText({ { menu.title, TextLength(menu.title), FontId::ui }, text_colour, text_left,
                            title.left + title.width, baseline, TextAlign::left });
    }
}

bool ParchmentMenuBar::RenderList()
{
    if (!c_assert(open_menu >= 0 && open_menu < menus.count)) return false;
    const ListLayout& list = lists[open_menu];
    const PixelSize frame = { list.size.width + 2 * SHADOW_MARGIN, list.size.height + 2 * SHADOW_MARGIN };
    ParchmentRenderer* renderer = context.renderer;
    if (!c_assert(renderer != nullptr)) return false;
    if (!renderer->BeginFrame({ list_surface.Dc(), frame, ParchmentRenderer::SurfaceAlpha::opaque })) return false;
    PaintList(*renderer, menus.specs[open_menu], list);
    if (!renderer->EndFrame()) return false;
    const PixelRect box = { SHADOW_MARGIN, SHADOW_MARGIN, SHADOW_MARGIN + list.size.width,
                            SHADOW_MARGIN + list.size.height };
    return list_shadow.Apply({ list_surface.Pixels(), list_surface.Size().width, frame, box, SHADOW_OFFSET,
                               SHADOW_SIGMA, SHADOW_OPACITY });
}

void ParchmentMenuBar::PaintList(ParchmentRenderer& renderer, const MenuSpec& menu, const ListLayout& list)
{
    if (!c_assert(menu.items != nullptr) || !c_assert(list.size.width > 0 && list.size.height > 0)) return;
    const float left = static_cast<float>(SHADOW_MARGIN);
    const float top = static_cast<float>(SHADOW_MARGIN);
    const FloatRect box = { left, top, left + static_cast<float>(list.size.width),
                            top + static_cast<float>(list.size.height) };
    renderer.FillRect(box, MENU_BORDER_COLOUR);
    const FloatRect inside = { box.left + LIST_BORDER, box.top + LIST_BORDER, box.right - LIST_BORDER,
                               box.bottom - LIST_BORDER };
    renderer.FillRect(inside,
                      MENU_COLOUR);
    const float inset = static_cast<float>(LIST_BORDER + LIST_PADDING);
    for (int i = 0; i < menu.item_count && i < MAX_ITEMS; i++)
    {
        const FloatRect area = { box.left + inset, top + list.item_top[i], box.right - inset,
                                 top + list.item_top[i] + list.item_height[i] };
        if (!IsSeparator(menu.items[i]))
        {
            PaintItem(renderer, i, area);
            continue;
        }
        const float line_top = std::round(area.top + SEPARATOR_MARGIN);
        renderer.FillRect({ area.left + ITEM_PADDING_LEFT, line_top, area.right - SEPARATOR_RIGHT, line_top + 1.0f },
                          MENU_SEPARATOR_COLOUR);
    }
}

void ParchmentMenuBar::PaintItem(ParchmentRenderer& renderer, int item_index, const FloatRect& area)
{
    if (!c_assert(IsOpen()) || !c_assert(HasArea(area))) return;
    const MenuSpec& menu = menus.specs[open_menu];
    if (!c_assert(item_index >= 0 && item_index < menu.item_count)) return;
    const MenuItemSpec& item = menu.items[item_index];
    const bool highlighted = item_index == highlighted_item;
    if (highlighted) renderer.FillRect({ area.left, std::round(area.top), area.right, std::round(area.bottom) },
                                       HOVER_GOLD_COLOUR);
    const Colour colour = highlighted ? TEXT_COLOUR : MENU_TEXT_COLOUR;
    const float baseline = renderer.BaselineInLine(FontId::ui, area.top + ITEM_PADDING_Y, UI_LINE_HEIGHT);
    renderer.DrawText({ { item.label, TextLength(item.label), FontId::ui }, colour, area.left + ITEM_PADDING_LEFT,
                        area.right, baseline, TextAlign::left });
    if (item.shortcut != nullptr)
        renderer.DrawText({ { item.shortcut, TextLength(item.shortcut), FontId::ui }, colour, area.left,
                            area.right - ITEM_PADDING_RIGHT, baseline, TextAlign::right });
    if (!item.external) return;
    const ImageId icon = highlighted ? ImageId::external_icon_dark : ImageId::external_icon;
    const PixelSize icon_size = renderer.ImageSize(icon);
    const float icon_top = (area.top + area.bottom - EXTERNAL_ICON_SIZE) / 2.0f;
    renderer.DrawImage({ icon, { 0, 0, icon_size.width, icon_size.height },
                         { area.left + ICON_LEFT, icon_top, area.left + ICON_LEFT + EXTERNAL_ICON_SIZE,
                           icon_top + EXTERNAL_ICON_SIZE },
                         1.0f, ParchmentRenderer::Smoothing::smooth });
}

bool ParchmentMenuBar::ShowList()
{
    if (!c_assert(list_window != NULL && window != NULL)) return false;
    if (!RenderList()) return false;
    const PixelRect area = ListArea();
    POINT position = { area.left - SHADOW_MARGIN, area.top - SHADOW_MARGIN };
    if (!c_assert(ClientToScreen(window, &position) != FALSE)) return false;
    const ListLayout& list = lists[open_menu];
    SIZE frame = { list.size.width + 2 * SHADOW_MARGIN, list.size.height + 2 * SHADOW_MARGIN };
    POINT origin = { 0, 0 };
    BLENDFUNCTION blend = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
    if (!UpdateLayeredWindow(list_window, NULL, &position, &frame, list_surface.Dc(), &origin, 0, &blend, ULW_ALPHA))
        return false;
    (void)ShowWindow(list_window, SW_SHOWNOACTIVATE);
    return true;
}

void ParchmentMenuBar::MoveHighlight(int step)
{
    if (!IsOpen() || !c_assert(step == 1 || step == -1)) return;
    const MenuSpec& menu = menus.specs[open_menu];
    if (!c_assert(menu.item_count > 0 && menu.items != nullptr)) return;
    int item = highlighted_item;
    for (int tried = 0; tried < menu.item_count && tried < MAX_ITEMS; tried++)
    {
        item = (item + step + menu.item_count) % menu.item_count;
        if (IsSeparator(menu.items[item])) continue;
        highlighted_item = item;
        (void)ShowList();
        return;
    }
}

int ParchmentMenuBar::TitleAt(PixelPoint point) const
{
    if (point.y < 0 || point.y >= size.height) return -1;
    const float x = static_cast<float>(point.x);
    for (int i = 0; i < menus.count && i < MAX_MENUS; i++)
    {
        if (x >= titles[i].left && x < titles[i].left + titles[i].width) return i;
    }
    return -1;
}

int ParchmentMenuBar::ItemAt(PixelPoint point) const
{
    if (!IsOpen()) return -1;
    if (!c_assert(open_menu < menus.count)) return -1;
    const PixelRect area = ListArea();
    const int inset = LIST_BORDER + LIST_PADDING;
    if (point.x < area.left + inset || point.x >= area.right - inset) return -1;
    const MenuSpec& menu = menus.specs[open_menu];
    const ListLayout& list = lists[open_menu];
    const float y = static_cast<float>(point.y - area.top);
    for (int i = 0; i < menu.item_count && i < MAX_ITEMS; i++)
    {
        const bool inside = y >= list.item_top[i] && y < list.item_top[i] + list.item_height[i];
        if (inside) return IsSeparator(menu.items[i]) ? -1 : i;
    }
    return -1;
}

PixelRect ParchmentMenuBar::ListArea() const
{
    if (!IsOpen()) return { 0, 0, 0, 0 };
    if (!c_assert(open_menu < menus.count)) return { 0, 0, 0, 0 };
    const int left = static_cast<int>(std::lround(titles[open_menu].left));
    const PixelSize list = lists[open_menu].size;
    return { left, size.height, left + list.width, size.height + list.height };
}

void ParchmentMenuBar::Redraw()
{
    if (window == NULL) return;
    (void)InvalidateRect(window, nullptr, FALSE);
}
