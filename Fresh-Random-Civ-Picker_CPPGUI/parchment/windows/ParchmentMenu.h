#pragma once

#include "framework.h"
#include "ParchmentControl.h"
#include "ParchmentGeometry.h"

// One item of a menu; an item without a label is a separator. A shortcut is shown on the right, and an item that
// opens another site shows the external-link icon.
struct MenuItemSpec
{
    const wchar_t* label;
    const wchar_t* shortcut;
    int command;
    bool external;
};

struct MenuSpec
{
    const wchar_t* title;
    wchar_t mnemonic;
    const MenuItemSpec* items;
    int item_count;
};

// The menu bar of the parchment theme (View): the web version's titles (.menubar) and their dark brown lists with
// gold highlights (.menu-list). It replaces the window menu and keeps its keyboard use: Alt with a title's
// letter or F10 opens a menu, the arrow keys move through the menus and items, Enter chooses and Esc closes.
// A chosen item is posted to the parent as WM_COMMAND with its command ID, as a window menu does.
class ParchmentMenuBar final
{
public:
    static constexpr int MAX_MENUS = 8;
    static constexpr int MAX_ITEMS = 16;

    struct Menus
    {
        const MenuSpec* specs;
        int count;
    };

    static bool RegisterWindowClass(HINSTANCE instance);

    bool Create(const ControlContext& bar_context, const ControlPlacement& placement, const Menus& specs);
    void Destroy();
    HWND Window() const;
    bool IsOpen() const;
    void Close();
    bool HandleKey(const MSG& message);
    bool ChooseHighlighted();

private:
    struct TitleLayout
    {
        float left;
        float width;
    };

    struct ListLayout
    {
        PixelSize size;
        float item_top[MAX_ITEMS];
        float item_height[MAX_ITEMS];
    };

    static LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(UINT message, WPARAM wParam, LPARAM lParam);

    bool LayOut();
    bool LayOutList(int menu_index);
    void OnPaint();
    void PaintTitles(ParchmentRenderer& renderer);
    bool RenderList();
    void PaintList(ParchmentRenderer& renderer, const MenuSpec& menu, const ListLayout& list);
    void PaintItem(ParchmentRenderer& renderer, int item_index, const FloatRect& area);
    bool ShowList();

    void OnMouseMove(LPARAM lParam);
    void OnMouseLeave();
    void OnButtonDown(LPARAM lParam);
    void OnButtonUp(LPARAM lParam);
    void OnCaptureLost();
    LRESULT OnSetCursor(WPARAM wParam, LPARAM lParam);
    bool HandleOpenMenuKey(WPARAM key);
    bool OpenByMnemonic(wchar_t letter);

    void Open(int menu_index);
    void OpenHighlighted(int menu_index);
    void MoveHighlight(int step);
    void Choose(int item_index);
    int TitleAt(PixelPoint point) const;
    int ItemAt(PixelPoint point) const;
    PixelRect ListArea() const;
    void Redraw();

    HWND window = NULL;
    HWND parent = NULL;
    HWND list_window = NULL;
    ControlContext context = { nullptr, nullptr, NULL };
    const PixelSurface* backdrop = nullptr;
    PixelSize size = { 0, 0 };
    Menus menus = { nullptr, 0 };
    TitleLayout titles[MAX_MENUS] = {};
    ListLayout lists[MAX_MENUS] = {};
    PixelSurface list_surface;
    BoxShadow list_shadow;
    int open_menu = -1;
    int hovered_title = -1;
    int highlighted_item = -1;
};
