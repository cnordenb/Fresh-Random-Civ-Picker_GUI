#include "CivPickerView.h"

#include <windowsx.h>
#include <algorithm>
#include <cwchar>

#include "Assertions.h"
#include "CivPickerController.h"
#include "Resource.h"

namespace
{
    constexpr UINT_PTR IDLE_TRIM_TIMER = 1;
    constexpr UINT IDLE_TRIM_DELAY_MS = 4000;

    bool IsUserInput(UINT message)
    {
        const bool key = message >= WM_KEYFIRST && message <= WM_KEYLAST;
        const bool mouse = message >= WM_MOUSEFIRST && message <= WM_MOUSELAST;
        return key || mouse || message == WM_HOTKEY;
    }

    constexpr PixelSize BACK_BUFFER_SIZE = { std::max(WINDOW_WIDTH, ParchmentDialogs::MAX_DIALOG_WIDTH),
                                             std::max(WINDOW_HEIGHT, ParchmentDialogs::MAX_DIALOG_HEIGHT) };
    constexpr int DIALOG_MARGINS = 2 * ParchmentDialogs::SHADOW_MARGIN;
    constexpr PixelSize SHELL_WORKSPACE_SIZE = {
        std::max(WINDOW_WIDTH, ParchmentDialogs::MAX_DIALOG_WIDTH + DIALOG_MARGINS),
        std::max(WINDOW_HEIGHT, ParchmentDialogs::MAX_DIALOG_HEIGHT + DIALOG_MARGINS) };
    static_assert(SHELL_WORKSPACE_SIZE.width * SHELL_WORKSPACE_SIZE.height <= ShellWorkspace::MAX_CANVAS_PIXELS,
                  "the shell workspace holds the largest window");
    constexpr int TEXT_MENU_COPY = 1;
    constexpr int TEXT_MENU_SELECT_ALL = 2;
}

CivPickerView::CivPickerView(const CivPickerModel& picker_model, CivPickerController& picker_controller) :
    model(picker_model),
    controller(picker_controller)
{
    const LPCWSTR sound_names[SOUND_AMOUNT] = { L"button_sound", L"tab_sound", L"hover_sound", L"error_sound",
                                                L"view_sound" };
    for (int i = 0; i < SOUND_AMOUNT; i++) sound_resources[i].name = sound_names[i];
}

void CivPickerView::SetInstance(HINSTANCE app_instance)
{
    if (!c_assert(app_instance != NULL)) return;
    instance = app_instance;
}

void CivPickerView::SetImageFolder(const std::wstring& folder)
{
    if (!c_assert(!folder.empty() && folder.back() == L'\\')) return;
    image_folder = folder;
}

void CivPickerView::SetJingleFolder(const std::wstring& folder)
{
    if (!c_assert(!folder.empty() && folder.back() == L'\\')) return;
    jingle_folder = folder;
}

bool CivPickerView::StartGraphics()
{
    if (!c_assert(instance != NULL) || !renderer.Start(CollectImageSources())) return false;
    ReportMissingImages();
    if (!back_buffer.Create(BACK_BUFFER_SIZE) || !shell_workspace.Create(SHELL_WORKSPACE_SIZE)) return false;
    if (!main_shell.Create({ { WINDOW_WIDTH, WINDOW_HEIGHT }, 0, &shell_workspace })) return false;
    text_menu = CreatePopupMenu();
    if (!c_assert(text_menu != NULL)) return false;
    const bool items_added = AppendMenuW(text_menu, MF_STRING, TEXT_MENU_COPY, L"Copy") &&
                             AppendMenuW(text_menu, MF_STRING, TEXT_MENU_SELECT_ALL, L"Select All");
    if (!c_assert(items_added) || !RegisterWindowClasses() || !BuildDialogLayouts()) return false;
    SetUpTooltips();
    return dialogs.Start({ instance, { &renderer, &back_buffer, text_menu }, &main_shell, &shell_workspace });
}

void CivPickerView::StopGraphics()
{
    dialogs.Stop();
    toast.Destroy();
    tooltip.Destroy();
    menu_bar.Destroy();
    main_shell.Destroy();
    shell_workspace.Destroy();
    back_buffer.Destroy();
    if (text_menu != NULL) (void)DestroyMenu(text_menu);
    text_menu = NULL;
    renderer.Stop();
}

bool CivPickerView::RegisterWindowClasses()
{
    if (!c_assert(instance != NULL)) return false;
    (void)LoadStringW(instance, IDC_WINDOWSPROJECT1, window_class, MAX_LOADSTRING);
    WNDCLASSEXW class_info = {};
    class_info.cbSize = sizeof(WNDCLASSEX);
    class_info.lpfnWndProc = MainWindowProc;
    class_info.hInstance = instance;
    class_info.hIcon = LoadIcon(instance, MAKEINTRESOURCE(IDI_WINDOWSPROJECT1));
    class_info.lpszClassName = window_class;
    class_info.hIconSm = LoadIcon(instance, MAKEINTRESOURCE(IDI_SMALL));
    if (!c_assert(RegisterClassExW(&class_info) != 0)) return false;
    return ParchmentControl::RegisterWindowClass(instance) && ParchmentLog::RegisterWindowClass(instance) &&
           ParchmentMenuBar::RegisterWindowClass(instance) && ParchmentShell::RegisterWindowClass(instance);
}

bool CivPickerView::CreateMainWindow(int show_command)
{
    if (!c_assert(main_window == NULL) || !c_assert(show_command >= SW_HIDE && show_command <= SW_MAX)) return false;
    wcscpy_s(title, L"Fresh Random Civ Picker");
    const PixelPoint position = CentredWindowPosition();
    const DWORD style = WS_POPUP | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN;
    const HWND window = CreateWindowExW(WS_EX_APPWINDOW, window_class, title, style, position.x, position.y,
                                        WINDOW_WIDTH, WINDOW_HEIGHT, nullptr, nullptr, instance, this);
    if (!c_assert(window != NULL)) return false;
    (void)ShowWindow(window, show_command);
    main_shell.Reveal();
    return true;
}

PixelPoint CivPickerView::CentredWindowPosition() const
{
    RECT work_area = {};
    if (!SystemParametersInfoW(SPI_GETWORKAREA, 0, &work_area, 0)) return { 0, 0 };
    return { (work_area.left + work_area.right - WINDOW_WIDTH) / 2,
             (work_area.top + work_area.bottom - WINDOW_HEIGHT) / 2 };
}

bool CivPickerView::PreTranslateMessage(const MSG& message)
{
    if (main_window == NULL) return false;
    if (IsUserInput(message.message)) NoteActivity();
    if (message.hwnd != main_window && !IsChild(main_window, message.hwnd)) return false;
    return menu_bar.HandleKey(message);
}

CivPickerView* CivPickerView::FromWindow(HWND window)
{
    if (!c_assert(window != NULL)) return nullptr;
    return reinterpret_cast<CivPickerView*>(GetWindowLongPtr(window, GWLP_USERDATA));
}

LRESULT CALLBACK CivPickerView::MainWindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_NCCREATE)
    {
        const CREATESTRUCT* create_struct = reinterpret_cast<const CREATESTRUCT*>(lParam);
        if (create_struct == nullptr) return FALSE;
        (void)SetWindowLongPtr(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(create_struct->lpCreateParams));
    }
    CivPickerView* view = FromWindow(window);
    if (view == nullptr) return DefWindowProc(window, message, wParam, lParam);
    return view->HandleMainWindowMessage({ window, message, wParam, lParam });
}

LRESULT CivPickerView::HandleMainWindowMessage(const MSG& message)
{
    if (!c_assert(message.hwnd != NULL)) return 0;
    const WPARAM wParam = message.wParam;
    const LPARAM lParam = message.lParam;
    switch (message.message)
    {
        case WM_DESTROY: return OnDestroy();
        case WM_CREATE: return OnCreate(message.hwnd);
        case WM_PAINT: return OnPaint();
        case WM_ERASEBKGND: return 1;
        case WM_NCHITTEST: return OnHitTest(lParam);
        case WM_SETCURSOR: return OnSetCursor(wParam, lParam);
        case WM_WINDOWPOSCHANGED: return OnWindowMoved(wParam, lParam);
        case WM_ACTIVATE: return OnActivate(wParam);
        case WM_MOUSEMOVE: return OnMainWindowMouseMove();
        case WM_MOUSELEAVE: return OnMouseLeave();
        case WM_HOTKEY: return OnHotkey(lParam);
        case WM_COMMAND: return OnCommand(wParam, lParam);
        case WM_TIMER: return OnTimer(wParam);
        case WM_MOUSEWHEEL: return OnMouseWheel(wParam, lParam);
        case WM_VSCROLL: return OnScroll(lParam);
        default: return DefWindowProc(message.hwnd, message.message, wParam, lParam);
    }
}

LRESULT CivPickerView::OnDestroy()
{
    controller.Quit();
    return 0;
}

LRESULT CivPickerView::OnCreate(HWND window)
{
    if (!c_assert(window != NULL) || !c_assert(main_window == NULL)) return -1;
    main_window = window;
    UpdateCursorSize();
    INITCOMMONCONTROLSEX common_controls = { sizeof(INITCOMMONCONTROLSEX), ICC_STANDARD_CLASSES | ICC_WIN95_CLASSES };
    if (!InitCommonControlsEx(&common_controls))
    {
        MessageBox(window, L"Failed to initialize common controls.", L"Error", MB_OK | MB_ICONERROR);
        return -1;
    }
    if (!BuildMainShell() || !DecorateMainBackdrop() || !CreateMainControls() || !tooltip.Create(window, renderer))
        return -1;
    if (!toast.Create(window, renderer)) return -1;
    renderer.ForgetImage(ImageId::background);
    renderer.ForgetImage(ImageId::program_icon);
    CollectTooltipTargets();
    controller.OnMainWindowCreated(window);
    NoteActivity();
    if (GetWindowTextLengthW(label_corner.Window()) == 0)
        SetControlText(label_corner.Window(), IterationText().c_str());
    return 0;
}

LRESULT CivPickerView::OnPaint()
{
    if (!c_assert(main_window != NULL)) return 0;
    main_shell.PaintWaitingChildren();
    PAINTSTRUCT paint = {};
    const HDC target = BeginPaint(main_window, &paint);
    if (target != NULL) main_shell.ShowBackdrop(target);
    (void)EndPaint(main_window, &paint);
    return 0;
}

LRESULT CivPickerView::OnHitTest(LPARAM lParam)
{
    if (!c_assert(main_window != NULL)) return HTCLIENT;
    POINT point = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
    if (!ScreenToClient(main_window, &point)) return HTCLIENT;
    const bool in_title_row = point.y >= WINDOW_PADDING && point.y < WINDOW_PADDING + TITLE_BAR_HEIGHT &&
                              point.x >= WINDOW_PADDING && point.x < WINDOW_WIDTH - WINDOW_PADDING - CLOSE_BUTTON_WIDTH;
    return in_title_row ? HTCAPTION : HTCLIENT;
}

LRESULT CivPickerView::OnSetCursor(WPARAM wParam, LPARAM lParam)
{
    if (!c_assert(main_window != NULL)) return FALSE;
    const UINT hit = LOWORD(lParam);
    const HCURSOR arrow = renderer.Cursor(CursorId::arrow);
    if ((hit != HTCLIENT && hit != HTCAPTION) || arrow == NULL)
        return DefWindowProc(main_window, WM_SETCURSOR, wParam, lParam);
    (void)SetCursor(arrow);
    return TRUE;
}

LRESULT CivPickerView::OnWindowMoved(WPARAM wParam, LPARAM lParam)
{
    if (!c_assert(main_window != NULL)) return 0;
    main_shell.Follow();
    menu_bar.Close();
    UpdateCursorSize();
    return DefWindowProc(main_window, WM_WINDOWPOSCHANGED, wParam, lParam);
}

LRESULT CivPickerView::OnActivate(WPARAM wParam)
{
    if (!c_assert(LOWORD(wParam) <= WA_CLICKACTIVE)) return 0;
    if (LOWORD(wParam) == WA_INACTIVE)
    {
        menu_bar.Close();
        DeactivateTooltips();
    }
    controller.OnActivate(LOWORD(wParam));
    return 0;
}

LRESULT CivPickerView::OnHotkey(LPARAM lParam)
{
    if (!c_assert(main_window != NULL)) return 0;
    const Keystroke keystroke = { static_cast<uint8_t>(LOWORD(lParam) & (MOD_ALT | MOD_CONTROL | MOD_SHIFT)),
                                  static_cast<uint8_t>(HIWORD(lParam)) };
    if (menu_bar.IsOpen())
    {
        const MSG menu_key = { main_window, WM_KEYDOWN, keystroke.key, 0 };
        if (keystroke.modifiers == 0 && menu_bar.HandleKey(menu_key)) return 0;
        menu_bar.Close();
    }
    controller.OnHotkey(keystroke);
    return 0;
}

LRESULT CivPickerView::OnCommand(WPARAM wParam, LPARAM lParam)
{
    if (!c_assert(main_window != NULL)) return 0;
    const int command_id = LOWORD(wParam);
    if (command_id == IDC_TAB_STRIP)
    {
        if (tab_strip.Selected() != current_tab) controller.OnTabSelected(tab_strip.Selected());
        return 0;
    }
    if (command_id == IDC_BUTTON_CLOSE)
    {
        (void)PostMessage(main_window, WM_SYSCOMMAND, SC_CLOSE, 0);
        return 0;
    }
    if (controller.OnCommand(command_id)) return 0;
    return DefWindowProc(main_window, WM_COMMAND, wParam, lParam);
}

LRESULT CivPickerView::OnMainWindowMouseMove()
{
    if (!c_assert(main_window != NULL)) return 0;
    TRACKMOUSEEVENT tracking = { sizeof(TRACKMOUSEEVENT), TME_LEAVE, main_window, 0 };
    (void)TrackMouseEvent(&tracking);
    POINT cursor = {};
    if (!model.GetSettings().tooltips_enabled || menu_bar.IsOpen() || !GetCursorPos(&cursor))
    {
        DeactivateTooltips();
        return 0;
    }
    for (const TooltipTarget& target : tooltip_targets)
    {
        RECT area = {};
        if (target.control == NULL || !IsWindowVisible(target.control)) continue;
        if (!GetWindowRect(target.control, &area)) continue;
        if (!PtInRect(&area, cursor)) continue;
        if (!c_assert(target.tooltip_id >= 0 && target.tooltip_id < TOOLTIP_AMOUNT)) return 0;
        const Tooltip& shown = tooltips[target.tooltip_id];
        tooltip.Show({ shown.text, shown.external }, { cursor.x, cursor.y });
        shown_tooltip = target.tooltip_id;
        return 0;
    }
    DeactivateTooltips();
    return 0;
}

void CivPickerView::NoteActivity()
{
    if (!c_assert(main_window != NULL)) return;
    (void)SetTimer(main_window, IDLE_TRIM_TIMER, IDLE_TRIM_DELAY_MS, nullptr);
}

LRESULT CivPickerView::OnTimer(WPARAM timer_id)
{
    if (!c_assert(main_window != NULL)) return 0;
    if (timer_id == TOAST_TIMER)
    {
        if (!toast.Tick()) (void)KillTimer(main_window, TOAST_TIMER);
        return 0;
    }
    if (timer_id != IDLE_TRIM_TIMER) return 0;
    (void)KillTimer(main_window, IDLE_TRIM_TIMER);
    (void)SetProcessWorkingSetSize(GetCurrentProcess(), static_cast<SIZE_T>(-1), static_cast<SIZE_T>(-1));
    return 0;
}

LRESULT CivPickerView::OnMouseLeave()
{
    if (!c_assert(main_window != NULL)) return 0;
    POINT cursor = {};
    if (!GetCursorPos(&cursor)) return 0;
    const HWND under_cursor = WindowFromPoint(cursor);
    if (under_cursor == main_window || IsChild(main_window, under_cursor)) return 0;
    DeactivateTooltips();
    return 0;
}
