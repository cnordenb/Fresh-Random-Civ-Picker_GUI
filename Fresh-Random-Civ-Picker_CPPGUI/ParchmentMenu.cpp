#include "ParchmentMenu.h"

#include <cmath>
#include <cwchar>
#include <cwctype>
#include <windowsx.h>

#include "Assertions.h"
#include "ParchmentShell.h"

namespace
{
    const wchar_t* const MENU_BAR_CLASS = L"FrcpParchmentMenuBar";
    // .menu-title { padding: 0 7px; border: 1px }
    constexpr float TITLE_PADDING = 7.0f;
    constexpr float TITLE_BORDER = 1.0f;
    // .menu-list { padding: 2px; border: 1px; min-width: 250px } and its items
    // { padding: 3px 18px 3px 30px; gap: 24px }
    constexpr int LIST_BORDER = 1;
    constexpr int LIST_PADDING = 2;
    constexpr int LIST_MIN_WIDTH = 250;
    constexpr float ITEM_PADDING_Y = 3.0f;
    constexpr float ITEM_PADDING_LEFT = 30.0f;
    constexpr float ITEM_PADDING_RIGHT = 18.0f;
    constexpr float ITEM_GAP = 24.0f;
    // .menu-list hr { margin: 3px 2px 3px 30px; border-top: 1px }
    constexpr float SEPARATOR_HEIGHT = 7.0f;
    constexpr float SEPARATOR_MARGIN = 3.0f;
    constexpr float SEPARATOR_RIGHT = 2.0f;
    constexpr float ICON_LEFT = 9.0f; // the external icon in the item's left margin
    constexpr int SHADOW_MARGIN = 6; // box-shadow: 2px 2px 3px rgba(0, 0, 0, .25)
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
    // The window procedure needs this object from the first message on, so it travels as the creation parameter.
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
    // The list window goes with its owner anyway; destroying it first leaves no stale handle.
    if (list_window != NULL) (void)DestroyWindow(list_window);
    list_window = NULL;
    list_surface.Destroy();
}

HWND ParchmentMenuBar::Window() const { return window; }

bool ParchmentMenuBar::IsOpen() const { return open_menu >= 0; }

// The titles sit side by side from the left; each list fits its widest item (CSS grid of label and shortcut).
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
    list.size = { fitted > LIST_MIN_WIDTH ? fitted : LIST_MIN_WIDTH,
                  static_cast<int>(std::lround(top)) + LIST_BORDER + LIST_PADDING };
    return c_assert(list.size.width <= MAX_LIST_SIZE.width && list.size.height <= MAX_LIST_SIZE.height);
}

// Window procedure of the menu bar. Win32 can only deliver window messages through a function pointer to this
// procedure (CLAUDE2.md I2); it reaches its menu bar through the window's user data.
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

// An open menu's title matches its list; a hovered title is gold like an item.
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

// One item of the open list; the highlighted one is gold with dark text, as a hovered item.
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
    // ShowWindow reports the earlier visibility, not success.
    (void)ShowWindow(list_window, SW_SHOWNOACTIVATE);
    return true;
}

// While a menu is open the bar holds the mouse capture, so it hears about the mouse everywhere.
void ParchmentMenuBar::OnMouseMove(LPARAM lParam)
{
    if (!c_assert(window != NULL)) return;
    const PixelPoint point = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
    const int title = TitleAt(point);
    if (!IsOpen())
    {
        if (title != hovered_title)
        {
            hovered_title = title;
            TRACKMOUSEEVENT tracking = { sizeof(TRACKMOUSEEVENT), TME_LEAVE, window, 0 };
            // Without leave tracking the title just stays highlighted until the next move.
            (void)TrackMouseEvent(&tracking);
            Redraw();
        }
        return;
    }
    if (title >= 0 && title != open_menu) Open(title);
    const int item = ItemAt(point);
    if (item == highlighted_item) return;
    highlighted_item = item;
    // A list that failed to redraw keeps its previous highlight until the next change.
    (void)ShowList();
}

void ParchmentMenuBar::OnMouseLeave()
{
    hovered_title = -1;
    Redraw();
}

void ParchmentMenuBar::OnButtonDown(LPARAM lParam)
{
    if (!c_assert(menus.count > 0)) return;
    const PixelPoint point = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
    const int title = TitleAt(point);
    if (IsOpen() && title == open_menu)
    {
        Close();
        return;
    }
    if (title >= 0)
    {
        Open(title);
        return;
    }
    if (IsOpen() && !RectContains(ListArea(), point)) Close();
}

void ParchmentMenuBar::OnButtonUp(LPARAM lParam)
{
    if (!IsOpen()) return;
    if (!c_assert(open_menu < menus.count)) return;
    const int item = ItemAt({ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) });
    if (item >= 0) Choose(item);
}

void ParchmentMenuBar::OnCaptureLost()
{
    if (IsOpen()) Close();
}

LRESULT ParchmentMenuBar::OnSetCursor(WPARAM wParam, LPARAM lParam)
{
    const HCURSOR cursor = context.renderer != nullptr ? context.renderer->Cursor(CursorId::arrow) : NULL;
    if (LOWORD(lParam) != HTCLIENT || cursor == NULL) return DefWindowProc(window, WM_SETCURSOR, wParam, lParam);
    (void)SetCursor(cursor);
    return TRUE;
}

// Keys for the menus, from the message loop: Alt with a title's letter or F10 opens a menu; while one is open
// the arrows, Enter and Esc work it, and any other key closes it and goes on to do its usual job.
bool ParchmentMenuBar::HandleKey(const MSG& message)
{
    if (!c_assert(menus.count > 0 && menus.specs != nullptr)) return false;
    if (message.message == WM_SYSCHAR) return OpenByMnemonic(static_cast<wchar_t>(message.wParam));
    const bool key_down = message.message == WM_KEYDOWN || message.message == WM_SYSKEYDOWN;
    if (!key_down) return false;
    if (message.wParam == VK_F10 && !IsOpen())
    {
        OpenHighlighted(0);
        return true;
    }
    if (!IsOpen()) return false;
    if (HandleOpenMenuKey(message.wParam)) return true;
    Close();
    return false;
}

bool ParchmentMenuBar::HandleOpenMenuKey(WPARAM key)
{
    if (!c_assert(IsOpen() && open_menu < menus.count)) return false;
    switch (key)
    {
        case VK_ESCAPE:
        case VK_MENU: Close(); return true;
        case VK_LEFT: OpenHighlighted((open_menu + menus.count - 1) % menus.count); return true;
        case VK_RIGHT: OpenHighlighted((open_menu + 1) % menus.count); return true;
        case VK_UP: MoveHighlight(-1); return true;
        case VK_DOWN: MoveHighlight(1); return true;
        case VK_RETURN: return ChooseHighlighted();
        default: return false;
    }
}

// Opens the next menu after the open one whose title has this letter, so a letter shared by two titles
// alternates between them.
bool ParchmentMenuBar::OpenByMnemonic(wchar_t letter)
{
    if (!c_assert(menus.count > 0)) return false;
    const wchar_t wanted = static_cast<wchar_t>(std::towupper(letter));
    for (int step = 1; step <= menus.count && step <= MAX_MENUS; step++)
    {
        const int index = (open_menu + step + menus.count) % menus.count;
        if (std::towupper(menus.specs[index].mnemonic) != wanted) continue;
        OpenHighlighted(index);
        return true;
    }
    return false;
}

bool ParchmentMenuBar::ChooseHighlighted()
{
    if (!IsOpen()) return false;
    if (!c_assert(highlighted_item < MAX_ITEMS)) return false;
    if (highlighted_item >= 0) Choose(highlighted_item);
    else Close();
    return true;
}

void ParchmentMenuBar::Open(int menu_index)
{
    if (!c_assert(menu_index >= 0 && menu_index < menus.count) || !c_assert(window != NULL)) return;
    open_menu = menu_index;
    highlighted_item = -1;
    if (GetCapture() != window) (void)SetCapture(window);
    Redraw();
    // A list that can't be shown leaves the title marked open; the next click or key closes it.
    (void)ShowList();
}

// Opened from the keyboard, a menu starts with its first item highlighted, as a window menu does.
void ParchmentMenuBar::OpenHighlighted(int menu_index)
{
    if (!c_assert(menu_index >= 0 && menu_index < menus.count)) return;
    Open(menu_index);
    if (!IsOpen()) return;
    highlighted_item = -1;
    MoveHighlight(1);
}

void ParchmentMenuBar::Close()
{
    if (!IsOpen()) return;
    if (!c_assert(list_window != NULL)) return;
    open_menu = -1;
    highlighted_item = -1;
    // ShowWindow reports the earlier visibility, and losing the capture is the goal either way.
    (void)ShowWindow(list_window, SW_HIDE);
    if (GetCapture() == window) (void)ReleaseCapture();
    Redraw();
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
        // A list that failed to redraw keeps its previous highlight until the next change.
        (void)ShowList();
        return;
    }
}

// Closes the menu and posts the item's command to the parent, as a window menu does.
void ParchmentMenuBar::Choose(int item_index)
{
    if (!c_assert(IsOpen())) return;
    const MenuSpec& menu = menus.specs[open_menu];
    if (!c_assert(item_index >= 0 && item_index < menu.item_count)) return;
    const int command = menu.items[item_index].command;
    Close();
    // Should the post fail, the click simply does nothing, as a lost click would.
    (void)PostMessage(parent, WM_COMMAND, MAKEWPARAM(command, 0), 0);
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

// The open list, in the bar's coordinates: below its title.
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
    // Invalidating a live window can't fail in a way that needs handling; at worst the next paint shows it.
    (void)InvalidateRect(window, nullptr, FALSE);
}
