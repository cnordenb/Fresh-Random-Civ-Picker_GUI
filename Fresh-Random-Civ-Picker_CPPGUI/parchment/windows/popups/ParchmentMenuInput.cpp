#include "ParchmentMenu.h"

#include <windowsx.h>
#include <cwctype>

#include "Assertions.h"

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
            (void)TrackMouseEvent(&tracking);
            Redraw();
        }
        return;
    }
    if (title >= 0 && title != open_menu) Open(title);
    const int item = ItemAt(point);
    if (item == highlighted_item) return;
    highlighted_item = item;
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

bool ParchmentMenuBar::Remeasure()
{
    if (!c_assert(window != NULL)) return false;
    Close();
    return LayOut();
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
    (void)ShowList();
}

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
    (void)ShowWindow(list_window, SW_HIDE);
    if (GetCapture() == window) (void)ReleaseCapture();
    Redraw();
}

void ParchmentMenuBar::Choose(int item_index)
{
    if (!c_assert(IsOpen())) return;
    const MenuSpec& menu = menus.specs[open_menu];
    if (!c_assert(item_index >= 0 && item_index < menu.item_count)) return;
    const int command = menu.items[item_index].command;
    Close();
    (void)PostMessage(parent, WM_COMMAND, MAKEWPARAM(command, 0), 0);
}
