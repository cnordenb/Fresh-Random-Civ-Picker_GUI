#include "CivPickerView.h"

#include <mmsystem.h>
#include <cwchar>
#include <windowsx.h>

#include "Assertions.h"
#include "CivPickerController.h"
#include "Resource.h"

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' " \
                        "version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

namespace
{
    // Controls are drawn at their places in their window, so the back buffer covers the largest window.
    constexpr int Larger(int first, int second) { return first > second ? first : second; }

    // Seconds without input after which the program hands its unused memory back to Windows.
    constexpr UINT_PTR IDLE_TRIM_TIMER = 1;
    constexpr UINT IDLE_TRIM_DELAY_MS = 4000;

    bool IsUserInput(UINT message)
    {
        const bool key = message >= WM_KEYFIRST && message <= WM_KEYLAST;
        const bool mouse = message >= WM_MOUSEFIRST && message <= WM_MOUSELAST;
        return key || mouse || message == WM_HOTKEY;
    }

    constexpr PixelSize BACK_BUFFER_SIZE = { Larger(WINDOW_WIDTH, ParchmentDialogs::MAX_DIALOG_WIDTH),
                                             Larger(WINDOW_HEIGHT, ParchmentDialogs::MAX_DIALOG_HEIGHT) };
    // Windows are shaped one at a time in one workspace, big enough for the largest window with its shadow.
    constexpr int DIALOG_MARGINS = 2 * ParchmentDialogs::SHADOW_MARGIN;
    constexpr PixelSize SHELL_WORKSPACE_SIZE = {
        Larger(WINDOW_WIDTH, ParchmentDialogs::MAX_DIALOG_WIDTH + DIALOG_MARGINS),
        Larger(WINDOW_HEIGHT, ParchmentDialogs::MAX_DIALOG_HEIGHT + DIALOG_MARGINS) };
    static_assert(SHELL_WORKSPACE_SIZE.width * SHELL_WORKSPACE_SIZE.height <= ShellWorkspace::MAX_CANVAS_PIXELS,
                  "the shell workspace holds the largest window");
    constexpr int TEXT_MENU_COPY = 1;
    constexpr int TEXT_MENU_SELECT_ALL = 2;

    int ToShowCommand(bool shown) { return shown ? SW_SHOW : SW_HIDE; }
}

CivPickerView::CivPickerView(const CivPickerModel& picker_model, CivPickerController& picker_controller) :
    model(picker_model),
    controller(picker_controller)
{
    const LPCWSTR sound_names[SOUND_AMOUNT] = { L"button_sound", L"tab_sound", L"hover_sound", L"error_sound",
                                                L"view_sound" };
    for (int i = 0; i < SOUND_AMOUNT; i++) sound_resources[i].name = sound_names[i];
}

// ---------------------------------------------------------------------------------------------------
// Lifecycle

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

// Builds everything the windows draw with, before any window exists (CLAUDE2.md I5: initialization).
bool CivPickerView::StartGraphics()
{
    if (!c_assert(instance != NULL) || !renderer.Start(image_folder.c_str())) return false;
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
    // Freeing at shutdown: a failure leaves nothing to recover.
    if (text_menu != NULL) (void)DestroyMenu(text_menu);
    text_menu = NULL;
    renderer.Stop();
}

bool CivPickerView::RegisterWindowClasses()
{
    if (!c_assert(instance != NULL)) return false;
    // A missing string leaves the class name empty, which RegisterClassExW then reports.
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

// The main window is the parchment itself: a borderless window of the web version's size, shaped like it, with
// the title, menu bar and close button drawn on it. It can be moved by its title and minimized.
bool CivPickerView::CreateMainWindow(int show_command)
{
    if (!c_assert(main_window == NULL) || !c_assert(show_command >= SW_HIDE && show_command <= SW_MAX)) return false;
    wcscpy_s(title, L"Fresh Random Civ Picker");
    const PixelPoint position = CentredWindowPosition();
    const DWORD style = WS_POPUP | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN;
    const HWND window = CreateWindowExW(WS_EX_APPWINDOW, window_class, title, style, position.x, position.y,
                                        WINDOW_WIDTH, WINDOW_HEIGHT, nullptr, nullptr, instance, this);
    if (!c_assert(window != NULL)) return false;
    // ShowWindow reports the earlier visibility, not success.
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

// Keys for the menu bar go to it before the window sees them, as with a window menu.
bool CivPickerView::PreTranslateMessage(const MSG& message)
{
    if (main_window == NULL) return false;
    if (IsUserInput(message.message)) NoteActivity();
    if (message.hwnd != main_window && !IsChild(main_window, message.hwnd)) return false;
    return menu_bar.HandleKey(message);
}

// ---------------------------------------------------------------------------------------------------
// Main window

CivPickerView* CivPickerView::FromWindow(HWND window)
{
    if (!c_assert(window != NULL)) return nullptr;
    return reinterpret_cast<CivPickerView*>(GetWindowLongPtr(window, GWLP_USERDATA));
}

// Window procedure of the main window. Win32 can only deliver window messages through a function
// pointer to this procedure (CLAUDE2.md I2); it reaches its view through the window's user data.
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
        case WM_HOTKEY: return OnHotkey(wParam);
        case WM_COMMAND: return OnCommand(wParam, lParam);
        case WM_TIMER: return OnTimer(wParam);
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
    INITCOMMONCONTROLSEX common_controls = { sizeof(INITCOMMONCONTROLSEX), ICC_STANDARD_CLASSES | ICC_WIN95_CLASSES };
    if (!InitCommonControlsEx(&common_controls))
    {
        MessageBox(window, L"Failed to initialize common controls.", L"Error", MB_OK | MB_ICONERROR);
        return -1;
    }
    if (!BuildMainShell() || !DecorateMainBackdrop() || !CreateMainControls() || !tooltip.Create(window, renderer))
        return -1;
    if (!toast.Create(window, renderer)) return -1;
    // The parchment image is in the window's shape and backdrop now and isn't drawn again.
    renderer.ForgetImage(ImageId::background);
    CollectTooltipTargets();
    controller.OnMainWindowCreated(window);
    NoteActivity();
    // The web version always shows the round counter, also before the first draw or reset sets it.
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

// The title row moves the window, as a title bar does.
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

// With a menu open, Esc only closes it and Enter chooses the highlighted item; any other key closes it and then
// does its usual job, as in the web version.
LRESULT CivPickerView::OnHotkey(WPARAM hotkey_id)
{
    if (!c_assert(main_window != NULL)) return 0;
    if (menu_bar.IsOpen())
    {
        if (hotkey_id == HOTKEY_ID_ESC)
        {
            menu_bar.Close();
            return 0;
        }
        if (hotkey_id == HOTKEY_ID_RETURN)
        {
            (void)menu_bar.ChooseHighlighted();
            return 0;
        }
        menu_bar.Close();
    }
    controller.OnHotkey(hotkey_id);
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
        // Closing as the system's close button does; posted, as this window may not send itself messages.
        (void)PostMessage(main_window, WM_SYSCOMMAND, SC_CLOSE, 0);
        return 0;
    }
    if (controller.OnCommand(command_id)) return 0;
    return DefWindowProc(main_window, WM_COMMAND, wParam, lParam);
}

// Shows the tooltip of the control under the mouse, as the web version does while the mouse moves.
LRESULT CivPickerView::OnMainWindowMouseMove()
{
    if (!c_assert(main_window != NULL)) return 0;
    TRACKMOUSEEVENT tracking = { sizeof(TRACKMOUSEEVENT), TME_LEAVE, main_window, 0 };
    // Without leave tracking a tooltip may linger until the mouse comes back; nothing else depends on it.
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

// Once nothing has happened for a few seconds the program hands the memory it isn't using back to Windows, as
// Windows does for a minimized window: the images, surfaces and fonts that drew the window stay allocated, and
// whatever the next paint needs comes back at once. Each input restarts the wait.
void CivPickerView::NoteActivity()
{
    if (!c_assert(main_window != NULL)) return;
    // Without the timer the memory just isn't handed back; nothing else depends on it.
    (void)SetTimer(main_window, IDLE_TRIM_TIMER, IDLE_TRIM_DELAY_MS, nullptr);
}

LRESULT CivPickerView::OnTimer(WPARAM timer_id)
{
    if (!c_assert(main_window != NULL)) return 0;
    if (timer_id == TOAST_TIMER)
    {
        // A timer that can't be stopped ticks on, and a tick without a message changes nothing.
        if (!toast.Tick()) (void)KillTimer(main_window, TOAST_TIMER);
        return 0;
    }
    if (timer_id != IDLE_TRIM_TIMER) return 0;
    // A timer that can't be stopped fires again and trims again, which does no harm.
    (void)KillTimer(main_window, IDLE_TRIM_TIMER);
    // A trim that fails leaves the memory where it is, as before.
    (void)SetProcessWorkingSetSize(GetCurrentProcess(), static_cast<SIZE_T>(-1), static_cast<SIZE_T>(-1));
    return 0;
}

// The mouse left the window itself, unless it only moved onto one of the window's controls.
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

// ---------------------------------------------------------------------------------------------------
// Tabs and control visibility

int CivPickerView::GetCurrentTab() const { return current_tab; }

int CivPickerView::GetSelectedTab() const { return tab_strip.Selected(); }

void CivPickerView::SelectTab(int tab_index)
{
    if (!c_assert(tab_index >= DRAW_TAB && tab_index <= CIV_POOL_TAB)) return;
    tab_strip.Select(tab_index);
}

void CivPickerView::ShowTabComponents(int tab_index)
{
    if (!c_assert(tab_index >= DRAW_TAB && tab_index <= CIV_POOL_TAB) || !c_assert(main_window != NULL)) return;
    if (!model.IsStartingUp()) PlayAudio(SoundType::tabsound);
    current_tab = tab_index;
    tab_strip.Select(tab_index);
    DeactivateTooltips();
    if (tab_index == DRAW_TAB)
    {
        ShowLogTab(false);
        ShowCustomTab(false);
        ShowDrawTab(true);
    }
    else if (tab_index == LOG_TAB)
    {
        ShowDrawTab(false);
        ShowCustomTab(false);
        ShowLogTab(true);
    }
    else if (tab_index == CIV_POOL_TAB)
    {
        ShowDrawTab(false);
        ShowLogTab(false);
        ShowCustomTab(true);
    }
}

void CivPickerView::ShowDrawTab(bool showing_enabled)
{
    if (!c_assert(main_window != NULL)) return;
    const Settings& settings = model.GetSettings();
    const HWND draw_tab_only[] = { button_survapp.Window(), button_techtree.Window(), button_history.Window(),
                                   button_options.Window() };
    if (showing_enabled)
    {
        ShowControl(button_draw.Window(), true);
        MoveResetButton(DRAW_TAB);
        ShowControl(button_reset.Window(), true);
        if (settings.civ_labels_enabled) ShowControl(label_centre.Window(), true);
        if (settings.icons_enabled) ShowControl(civ_icon.Window(), true);
        if (settings.iteration_label_enabled) ShowControl(label_corner.Window(), true);
        for (HWND control : draw_tab_only) ShowControl(control, true);
        UpdateTooltipText(TOOLTIP_OPTIONS, L"Opens options\nHotkey: Q (Draw tab only) / F1");
        return;
    }

    if (current_tab == CIV_POOL_TAB)
    {
        ShowControl(civ_icon.Window(), false);
        ShowControl(button_draw.Window(), false);
        ShowControl(label_centre.Window(), false);
    }
    ShowControl(button_reset.Window(), false);
    MoveResetButton(LOG_TAB);
    ShowControl(label_corner.Window(), false);
    for (HWND control : draw_tab_only) ShowControl(control, false);
    UpdateTooltipText(TOOLTIP_OPTIONS, L"Opens options\nHotkey: F1");
}

void CivPickerView::ShowLogTab(bool showing_enabled)
{
    if (!c_assert(main_window != NULL)) return;
    const Settings& settings = model.GetSettings();
    const HWND log_tab_only[] = { label_drawncount.Window(), button_clearlog.Window(), drawn_log.Window(),
                                  label_remainingcount.Window(), checkbox_showremainlog.Window(),
                                  button_share.Window() };
    if (showing_enabled)
    {
        ShowControl(button_draw.Window(), true);
        ShowControl(button_reset.Window(), true);
        if (settings.civ_labels_enabled) ShowControl(label_centre.Window(), true);
        if (settings.icons_enabled) ShowControl(civ_icon.Window(), true);
        for (HWND control : log_tab_only) ShowControl(control, true);
        if (settings.remainlog_enabled) ShowControl(remaining_log.Window(), true);
        return;
    }

    if (current_tab == CIV_POOL_TAB)
    {
        ShowControl(button_draw.Window(), false);
        ShowControl(label_centre.Window(), false);
        ShowControl(civ_icon.Window(), false);
    }
    ShowControl(button_reset.Window(), false);
    for (HWND control : log_tab_only) ShowControl(control, false);
    ShowControl(remaining_log.Window(), false);
}

void CivPickerView::ShowCustomTab(bool showing_enabled)
{
    if (!c_assert(main_window != NULL)) return;
    ShowControl(edition_icon.Window(), showing_enabled);
    ShowControl(label_enabledcount.Window(), showing_enabled);
    const HWND pool_controls[] = { button_enableall.Window(), button_disableall.Window(), checkbox_autoreset.Window(),
                                   checkbox_autotoggle.Window(), button_poolshare.Window() };
    for (HWND control : pool_controls) ShowControl(control, showing_enabled);
    const int edition_count = model.Catalog().EditionCount();
    for (int i = 0; i < edition_count && i < MAX_EDITIONS; i++) ShowControl(edition_rows[i].Window(), showing_enabled);
    if (!showing_enabled)
    {
        HideCustomPoolCheckboxes();
        return;
    }
    ShowPoolCheckboxes(model.GetEdition());
    ShowDlcToggles(model.GetEdition());
}

void CivPickerView::ShowPoolCheckboxes(int edition)
{
    if (!c_assert(edition >= 0 && edition < model.Catalog().EditionCount())) return;
    const int civ_count = model.Catalog().CivCount();
    for (int i = 0; i < civ_count && i < MAX_CIVS; i++)
        ShowControl(civ_toggles[i].Window(), model.BelongsToEdition(model.GetCiv(i), edition));
}

void CivPickerView::HideCustomPoolCheckboxes()
{
    const int civ_count = model.Catalog().CivCount();
    for (int i = 0; i < civ_count && i < MAX_CIVS; i++) ShowControl(civ_toggles[i].Window(), false);
    const int dlc_count = model.Catalog().DlcCount();
    for (int i = 0; i < dlc_count && i < MAX_DLCS; i++)
    {
        if (HasDlcToggle(i)) ShowControl(dlc_toggles[i].Window(), false);
    }
}

// The pool's controls follow the edition; they only show while the Civ Pool tab does. The banners are stacked in
// the image in data.json's order of editions.
void CivPickerView::ShowEditionControls(int edition)
{
    if (!c_assert(edition >= 0 && edition < model.Catalog().EditionCount())) return;
    if (current_tab == CIV_POOL_TAB)
    {
        ShowDlcToggles(edition);
        ShowPoolCheckboxes(edition);
    }
    edition_icon.ShowPicture({ ImageId::edition_banners, { 0, edition * EDITION_BANNER_HEIGHT, EDITION_BANNER_WIDTH,
                                                           (edition + 1) * EDITION_BANNER_HEIGHT } });
    CheckEditionRow(edition);
}

void CivPickerView::CheckEditionRow(int edition)
{
    const int edition_count = model.Catalog().EditionCount();
    if (!c_assert(edition >= 0 && edition < edition_count)) return;
    for (int i = 0; i < edition_count && i < MAX_EDITIONS; i++) edition_rows[i].SetChecked(i == edition);
}

void CivPickerView::ShowControl(HWND control, bool shown)
{
    if (!c_assert(control != NULL)) return;
    // ShowWindow reports the earlier visibility, not success.
    (void)ShowWindow(control, ToShowCommand(shown));
}

// ---------------------------------------------------------------------------------------------------
// Control states requested by the controller

bool CivPickerView::IsCivChecked(int civ_index) const
{
    if (!c_assert(civ_index >= 0 && civ_index < model.Catalog().CivCount())) return false;
    return civ_toggles[civ_index].IsChecked();
}

bool CivPickerView::IsDlcChecked(int dlc) const
{
    if (!c_assert(HasDlcToggle(dlc))) return false;
    return dlc_toggles[dlc].IsChecked();
}

void CivPickerView::SetDlcChecked(int dlc, bool checked)
{
    if (!c_assert(dlc >= 0 && dlc < model.Catalog().DlcCount())) return;
    // A DLC without a toggle (the base game's) has its civs checked one by one.
    if (!HasDlcToggle(dlc)) return;
    dlc_toggles[dlc].SetChecked(checked);
}

// Whether a Civ Pool tab lists the DLC with a toggle of its own (data.json: an edition's dlcToggles).
bool CivPickerView::HasDlcToggle(int dlc) const
{
    if (dlc < 0 || dlc >= model.Catalog().DlcCount()) return false;
    return model.Catalog().DlcAt(dlc).toggle_edition != CivCatalog::NONE;
}

bool CivPickerView::IsAutoToggleChecked() const { return checkbox_autotoggle.IsChecked(); }

void CivPickerView::SetAutoToggleChecked(bool checked) { checkbox_autotoggle.SetChecked(checked); }

bool CivPickerView::IsAutoResetChecked() const { return checkbox_autoreset.IsChecked(); }

void CivPickerView::SetAutoResetChecked(bool checked) { checkbox_autoreset.SetChecked(checked); }

void CivPickerView::ValidateDlcToggle(int dlc) { SetDlcChecked(dlc, !model.IsDlcEmpty(dlc)); }

void CivPickerView::ValidateAllDlcToggles()
{
    const int dlc_count = model.Catalog().DlcCount();
    for (int i = 0; i < dlc_count && i < MAX_DLCS; i++)
    {
        if (HasDlcToggle(i)) ValidateDlcToggle(i);
    }
}

void CivPickerView::ShowRemainingLog(bool shown) { ShowControl(remaining_log.Window(), shown); }

void CivPickerView::ToggleRemainLogCheckbox()
{
    checkbox_showremainlog.SetChecked(!checkbox_showremainlog.IsChecked());
}

void CivPickerView::ClearDrawnLog()
{
    if (!c_assert(drawn_log.Window() != NULL)) return;
    if (!model.IsStartingUp()) drawn_log_linecount = 0;
    SetControlText(drawn_log.Window(), L"");
}

void CivPickerView::ClearLogTexts()
{
    if (!c_assert(drawn_log.Window() != NULL && remaining_log.Window() != NULL)) return;
    SetControlText(remaining_log.Window(), L"");
    SetControlText(drawn_log.Window(), L"");
}

void CivPickerView::RefreshCivIcon()
{
    ShowControl(civ_icon.Window(), model.GetSettings().icons_enabled && current_tab != CIV_POOL_TAB);
}

void CivPickerView::RefreshCivLabel()
{
    if (!model.GetSettings().civ_labels_enabled) ShowControl(label_centre.Window(), false);
    else if (current_tab != CIV_POOL_TAB) ShowControl(label_centre.Window(), true);
}

void CivPickerView::RefreshCornerLabel()
{
    if (!model.GetSettings().iteration_label_enabled) ShowControl(label_corner.Window(), false);
    else if (current_tab == DRAW_TAB) ShowControl(label_corner.Window(), true);
}

void CivPickerView::DeactivateTooltips()
{
    tooltip.Hide();
    shown_tooltip = -1;
}

// ---------------------------------------------------------------------------------------------------
// Rendering model state (Observer notifications, HFDP ch. 2 and 12)

void CivPickerView::OnPoolChanged(int civ_index)
{
    if (!c_assert(civ_index >= 0 && civ_index < model.Catalog().CivCount())) return;
    civ_toggles[civ_index].SetChecked(model.GetCiv(civ_index).enabled);
    SetControlText(label_corner.Window(), IterationText().c_str());
    UpdateRemainingLog();
    UpdateDrawnLog(DrawnLogUpdate::count_only);
    if (!model.IsBatchToggle()) UpdateEnabledCivsCounter();
}

void CivPickerView::OnBulkPoolChanged()
{
    ValidateAllDlcToggles();
    UpdateEnabledCivsCounter();
}

void CivPickerView::OnDlcPoolChanged() { UpdateEnabledCivsCounter(); }

void CivPickerView::OnEditionChanged()
{
    if (!c_assert(main_window != NULL)) return;
    if (!model.IsStartingUp()) PlayAudio(SoundType::view);
    ShowEditionControls(model.GetEdition());
    ValidateAllDlcToggles();
    UpdateEnabledCivsCounter();
}

void CivPickerView::OnResetStarted(bool auto_reset)
{
    if (!model.IsStartingUp() && !auto_reset && !model.GetSettings().jingles_enabled) PlayAudio(SoundType::button);
}

void CivPickerView::OnDrawSequenceBroken() { UpdateDrawnLog(DrawnLogUpdate::blank_line); }

void CivPickerView::OnPoolReset(bool auto_reset)
{
    if (!c_assert(main_window != NULL)) return;
    const Settings& settings = model.GetSettings();
    SetControlText(label_corner.Window(), (L"0/" + std::to_wstring(model.GetCustomMaxCivs())).c_str());
    if (!settings.iteration_label_enabled) ShowControl(label_corner.Window(), false);
    SetControlText(label_centre.Window(), L"?");
    if (!settings.civ_labels_enabled) ShowControl(label_centre.Window(), false);
    ShowCivIconFor(L"Random");

    if (!model.IsStartingUp() && !auto_reset && settings.jingles_enabled && current_tab != CIV_POOL_TAB)
        PlayJingle(model.GetCurrentCiv());
    UpdateRemainingLog();
    UpdateTooltips(false);
}

void CivPickerView::OnDrawStarted()
{
    if (!model.IsStartingUp() && !model.GetSettings().jingles_enabled) PlayAudio(SoundType::button);
}

void CivPickerView::OnEmptyPoolDraw()
{
    PlayAudio(SoundType::error);
    SetControlText(label_centre.Window(), L"Empty pool!");
}

void CivPickerView::OnCivDrawn()
{
    const Settings& settings = model.GetSettings();
    const std::wstring& current_civ = model.GetCurrentCiv();
    if (!c_assert(!current_civ.empty())) return;
    SetControlText(label_corner.Window(), IterationText().c_str());
    if (!settings.iteration_label_enabled) ShowControl(label_corner.Window(), false);
    SetControlText(label_centre.Window(), current_civ.c_str());
    if (!settings.civ_labels_enabled) ShowControl(label_centre.Window(), false);
    ShowCivIconFor(current_civ);

    UpdateDrawnLog(DrawnLogUpdate::entry);
    UpdateRemainingLog();
    if (!model.IsStartingUp()) PlayJingle(current_civ);
    UpdateTooltips(true);
}

void CivPickerView::OnUndrawAccepted() { PlayAudio(SoundType::button); }

void CivPickerView::OnCivUndrawn()
{
    const Settings& settings = model.GetSettings();
    const std::wstring& current_civ = model.GetCurrentCiv();
    if (!c_assert(!current_civ.empty())) return;
    ShowCivIconFor(current_civ);
    PlayJingle(current_civ);
    SetControlText(label_centre.Window(), current_civ == L"Random" ? L"?" : current_civ.c_str());
    if (!settings.civ_labels_enabled) ShowControl(label_centre.Window(), false);

    SetControlText(label_drawncount.Window(), (L"Drawn: " + IterationText()).c_str());
    SetControlText(label_corner.Window(), IterationText().c_str());
    if (!settings.iteration_label_enabled || current_tab != DRAW_TAB) ShowControl(label_corner.Window(), false);
    if (model.GetIterator() == 0) ShowCivIconFor(L"Random");

    RemoveNewestDrawnLogEntry();
    UpdateTooltips(true);
    UpdateRemainingLog();
}

void CivPickerView::OnCivRedrawn()
{
    const std::wstring& current_civ = model.GetCurrentCiv();
    if (!c_assert(!current_civ.empty())) return;
    ShowCivIconFor(current_civ);
    PlayJingle(current_civ);
    SetControlText(label_centre.Window(), current_civ.c_str());
    SetControlText(label_corner.Window(), IterationText().c_str());
    if (!model.GetSettings().iteration_label_enabled || current_tab != DRAW_TAB)
        ShowControl(label_corner.Window(), false);

    UpdateDrawnLog(DrawnLogUpdate::entry);
    UpdateRemainingLog();
    UpdateTooltips(true);
}

void CivPickerView::OnActionRejected() { PlayAudio(SoundType::error); }

void CivPickerView::OnLogLoaded(SaveType type)
{
    if (!c_assert(type == SaveType::automatic || type == SaveType::manual || type == SaveType::quick)) return;
    const Settings& settings = model.GetSettings();
    const std::wstring& current_civ = model.GetCurrentCiv();
    const std::wstring iteration_text = IterationText();
    SetControlText(label_corner.Window(), iteration_text.c_str());
    if (!settings.iteration_label_enabled) ShowControl(label_corner.Window(), false);
    if (model.GetIterator() == 0) SetControlText(label_drawncount.Window(), (L"Drawn: " + iteration_text).c_str());

    ShowCivIconFor(current_civ);
    if (!settings.icons_enabled) ShowControl(civ_icon.Window(), false);
    UpdateRemainingLog();

    const bool nothing_drawn = type == SaveType::automatic && current_civ == L"Random";
    UpdateTooltips(!nothing_drawn);
    SetControlText(label_centre.Window(), nothing_drawn ? L"?" : current_civ.c_str());
    if (!settings.civ_labels_enabled) ShowControl(label_centre.Window(), false);
}

void CivPickerView::OnModelError(const std::wstring& message, bool fatal)
{
    if (!c_assert(!message.empty())) return;
    ShowInformation(message.c_str(), L"Error");
    if (fatal) controller.OnFatalError();
}

// ---------------------------------------------------------------------------------------------------
// Labels, logs and tooltips

std::wstring CivPickerView::IterationText() const
{
    return std::to_wstring(model.GetIterator()) + L"/" + std::to_wstring(model.GetCustomMaxCivs());
}

// The civ's cell in the civ icon atlas, which holds the civs in the order of their IDs and Random after them.
int CivPickerView::CivAtlasCell(const std::wstring& civ_name) const
{
    const int random_cell = model.Catalog().CivCount();
    if (!c_assert(!civ_name.empty())) return random_cell;
    const int civ = model.FindCivIndex(civ_name);
    return civ != CivCatalog::NONE ? civ : random_cell;
}

void CivPickerView::ShowCivIconFor(const std::wstring& civ_name)
{
    const int cell = CivAtlasCell(civ_name);
    if (!c_assert(cell >= 0 && cell <= model.Catalog().CivCount())) return;
    const int column = cell % CIV_ATLAS_COLUMNS;
    const int row = cell / CIV_ATLAS_COLUMNS;
    civ_icon.ShowPicture({ ImageId::civ_atlas, { column * CIV_ATLAS_CELL, row * CIV_ATLAS_CELL,
                                                 (column + 1) * CIV_ATLAS_CELL, (row + 1) * CIV_ATLAS_CELL } });
}

void CivPickerView::SetControlText(HWND control, const wchar_t* text)
{
    if (!c_assert(text != nullptr) || control == NULL) return;
    // A label that didn't take the text keeps showing the old one until the next update.
    (void)SetWindowTextW(control, text);
}

void CivPickerView::UpdateDrawnLog(DrawnLogUpdate update)
{
    if (!c_assert(drawn_log.Window() != NULL)) return;
    SetControlText(label_drawncount.Window(), (L"Drawn: " + IterationText()).c_str());

    const int log_length = GetWindowTextLength(drawn_log.Window());
    if (!c_assert(log_length >= 0)) return;
    std::wstring log_text(static_cast<size_t>(log_length) + 1, L'\0');
    GetWindowText(drawn_log.Window(), &log_text[0], log_length + 1);
    log_text.pop_back();
    const bool starts_with_blank_line = log_text.substr(0, 2) == L"\r\n";

    if (model.IsCacheLoading())
    {
        if (!starts_with_blank_line) return;
        log_text.erase(0, 2);
        SetControlText(drawn_log.Window(), log_text.c_str());
        drawn_log_linecount--;
        return;
    }

    if (update == DrawnLogUpdate::entry)
        AddDrawnLogEntry(log_text, IterationText() + L" - " + model.GetCurrentCiv() + L"\r\n");
    else if (update == DrawnLogUpdate::blank_line && !model.IsResetState() && !starts_with_blank_line)
        AddDrawnLogEntry(log_text, L"\r\n");
}

void CivPickerView::AddDrawnLogEntry(std::wstring& log_text, const std::wstring& entry)
{
    if (!c_assert(!entry.empty())) return;
    if (!c_assert(drawn_log_linecount >= 0 && drawn_log_linecount <= MAX_DRAWNLOG_LINECOUNT)) return;
    log_text = entry + log_text;
    if (drawn_log_linecount == MAX_DRAWNLOG_LINECOUNT)
    {
        const size_t last_newline = log_text.rfind(L"\r");
        if (last_newline != std::wstring::npos) log_text.erase(last_newline);
        SetControlText(drawn_log.Window(), log_text.c_str());
        return;
    }
    SetControlText(drawn_log.Window(), log_text.c_str());
    drawn_log_linecount++;
}

void CivPickerView::RemoveNewestDrawnLogEntry()
{
    if (!c_assert(drawn_log.Window() != NULL)) return;
    const int log_length = GetWindowTextLength(drawn_log.Window());
    if (log_length <= 0) return;

    std::wstring log_text(static_cast<size_t>(log_length) + 1, L'\0');
    GetWindowText(drawn_log.Window(), &log_text[0], log_length + 1);
    log_text.pop_back();
    const size_t first_line_end = log_text.find(L"\r\n");
    if (first_line_end == std::wstring::npos) return;

    log_text = log_text.substr(first_line_end + 2);
    SetControlText(drawn_log.Window(), log_text.c_str());
    drawn_log_linecount--;
}

void CivPickerView::UpdateRemainingLog()
{
    if (!c_assert(remaining_log.Window() != NULL && label_remainingcount.Window() != NULL)) return;
    // In the order of their names, as the Civ Pool tab shows them.
    std::wstring remaining_text;
    const CivCatalog& catalog = model.Catalog();
    for (int position = 0; position < catalog.CivCount() && position < MAX_CIVS; position++)
    {
        const std::wstring& pool_entry = model.GetPoolEntry(catalog.DisplayCiv(position));
        if (model.GetCiv(pool_entry).enabled && pool_entry != L"") remaining_text += pool_entry + L"\r\n";
    }
    const std::wstring remaining_label =
        L"Remaining: " + std::to_wstring(model.GetRemaining()) + L"/" + std::to_wstring(model.GetCustomMaxCivs());
    SetControlText(remaining_log.Window(), remaining_text.c_str());
    SetControlText(label_remainingcount.Window(), remaining_label.c_str());
}

void CivPickerView::UpdateEnabledCivsCounter()
{
    if (!c_assert(label_enabledcount.Window() != NULL)) return;
    const int edition_civ_amount = model.Catalog().EditionCivCount(model.GetEdition());

    const std::wstring enabled_text =
        L"Enabled: " + std::to_wstring(model.GetCustomMaxCivs()) + L"/" + std::to_wstring(edition_civ_amount);
    SetControlText(label_enabledcount.Window(), enabled_text.c_str());
}

void CivPickerView::UpdateTooltips(bool drawn)
{
    if (drawn)
    {
        const std::wstring& current_civ = model.GetCurrentCiv();
        UpdateTooltipText(TOOLTIP_TECHTREE, L"Opens the tech tree for the " + current_civ + TECHTREE_HOTKEY_LINE);
        UpdateTooltipText(TOOLTIP_HISTORY, L"Opens the game's history info for the " + current_civ + L"\nHotkey: H");
        return;
    }
    UpdateTooltipText(TOOLTIP_TECHTREE, std::wstring(L"Opens the tech tree") + TECHTREE_HOTKEY_LINE);
    UpdateTooltipText(TOOLTIP_HISTORY, L"Opens the game's history info\nHotkey: H");
}

// Changes a tooltip's text; a tooltip on show changes with it, as the web version's does.
void CivPickerView::UpdateTooltipText(int tooltip_id, const std::wstring& text)
{
    if (!c_assert(tooltip_id >= 0 && tooltip_id < TOOLTIP_AMOUNT) || !c_assert(!text.empty())) return;
    SetTooltip(tooltip_id, text.c_str());
    POINT cursor = {};
    if (shown_tooltip != tooltip_id || !GetCursorPos(&cursor)) return;
    tooltip.Show({ tooltips[tooltip_id].text, tooltips[tooltip_id].external }, { cursor.x, cursor.y });
}

// ---------------------------------------------------------------------------------------------------
// Sounds and jingles

void CivPickerView::LoadSounds()
{
    for (int i = 0; i < SOUND_AMOUNT; ++i) LoadSound(sound_resources[i]);
    LoadJingles();
}

// The jingles are files named after the civs (data.json's names in lower case), the original game's in the legacy
// folder. They are mapped into memory rather than read: like a resource of the exe, a mapped file takes memory only
// while it plays.
void CivPickerView::LoadJingles()
{
    if (!c_assert(!jingle_folder.empty())) return;
    const CivCatalog& catalog = model.Catalog();
    int missing = 0;
    for (int i = 0; i < catalog.CivCount() && i < MAX_CIVS; i++)
    {
        const CivInfo& civ = catalog.CivAt(i);
        jingles[i] = MapJingle(civ.file_name + L".wav");
        if (jingles[i].data == nullptr) missing++;
        if (!civ.legacy_jingle) continue;
        legacy_jingles[i] = MapJingle(L"legacy\\" + civ.file_name + L".wav");
        if (legacy_jingles[i].data == nullptr) missing++;
    }
    random_jingle = MapJingle(L"random.wav");
    random_legacy_jingle = MapJingle(L"legacy\\random.wav");
    if (random_jingle.data == nullptr) missing++;
    if (random_legacy_jingle.data == nullptr) missing++;
    if (missing == 0) return;
    const std::wstring message = std::to_wstring(missing) + L" civ jingles could not be loaded from " + jingle_folder +
                                 L"\nThe program works without them.";
    MessageBox(NULL, message.c_str(), L"Error", MB_OK | MB_ICONERROR);
}

CivPickerView::MappedJingle CivPickerView::MapJingle(const std::wstring& file_name) const
{
    if (!c_assert(!file_name.empty())) return { nullptr };
    const std::wstring path = jingle_folder + file_name;
    const HANDLE file = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE) return { nullptr };
    const HANDLE mapping = CreateFileMappingW(file, nullptr, PAGE_READONLY, 0, 0, nullptr);
    // The mapping keeps the file open, and the view keeps the mapping; neither handle is needed after this.
    (void)CloseHandle(file);
    if (mapping == NULL) return { nullptr };
    const void* view = MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, 0);
    (void)CloseHandle(mapping);
    return { view };
}

void CivPickerView::LoadSound(SoundResource& sound_resource)
{
    if (!c_assert(instance != NULL) || !c_assert(sound_resource.name != nullptr)) return;
    sound_resource.info = FindResource(instance, sound_resource.name, L"WAVE");
    if (sound_resource.info == NULL)
    {
        MessageBox(NULL, (std::wstring(L"Failed to find resource: ") + sound_resource.name).c_str(), L"Error",
                   MB_OK | MB_ICONERROR);
        return;
    }
    sound_resource.handle = LoadResource(instance, sound_resource.info);
    if (sound_resource.handle == NULL)
    {
        MessageBox(NULL, L"Failed to load resource.", L"Error", MB_OK | MB_ICONERROR);
        return;
    }
    sound_resource.data = LockResource(sound_resource.handle);
    if (sound_resource.data == NULL) MessageBox(NULL, L"Failed to lock resource.", L"Error", MB_OK | MB_ICONERROR);
}

void CivPickerView::UnloadSounds()
{
    for (int i = 0; i < SOUND_AMOUNT; ++i) UnloadSound(sound_resources[i]);
    // Unmapping at the end changes nothing the program still uses; a failure leaves it to Windows at exit.
    StopSound();
    for (MappedJingle& jingle : jingles)
    {
        if (jingle.data != nullptr) (void)UnmapViewOfFile(jingle.data);
        jingle.data = nullptr;
    }
    for (MappedJingle& jingle : legacy_jingles)
    {
        if (jingle.data != nullptr) (void)UnmapViewOfFile(jingle.data);
        jingle.data = nullptr;
    }
    if (random_jingle.data != nullptr) (void)UnmapViewOfFile(random_jingle.data);
    if (random_legacy_jingle.data != nullptr) (void)UnmapViewOfFile(random_legacy_jingle.data);
    random_jingle.data = nullptr;
    random_legacy_jingle.data = nullptr;
}

void CivPickerView::UnloadSound(SoundResource& sound_resource)
{
    if (sound_resource.data)
    {
        UnlockResource(sound_resource.data);
        sound_resource.data = NULL;
    }
    if (sound_resource.handle)
    {
        FreeResource(sound_resource.handle);
        sound_resource.handle = NULL;
    }
}

void CivPickerView::PlayResource(const SoundResource& sound_resource) const { PlayWave(sound_resource.data); }

// Plays a WAVE file that is in memory (a resource of the exe or a mapped jingle file).
void CivPickerView::PlayWave(const void* wave) const
{
    // A sound that failed to load was reported then; playing it would only stop the sound that plays.
    if (!c_assert(wave != nullptr)) return;
    if (!sndPlaySound(static_cast<LPCWSTR>(wave), SND_MEMORY | SND_ASYNC | SND_NODEFAULT))
    {
        MessageBox(NULL, L"Failed to play sound.", L"Error", MB_OK | MB_ICONERROR);
    }
}

void CivPickerView::PlayAudio(SoundType type) const
{
    if (!model.GetSettings().ui_sounds_enabled) return;
    switch (type)
    {
        case SoundType::button: PlayResource(sound_resources[0]); return;
        case SoundType::tabsound: PlayResource(sound_resources[1]); return;
        case SoundType::hover: PlayResource(sound_resources[2]); return;
        case SoundType::error: PlayResource(sound_resources[3]); return;
        case SoundType::view: PlayResource(sound_resources[4]); return;
    }
}

void CivPickerView::PlayJingle(const std::wstring& civ_name) const
{
    if (!c_assert(!civ_name.empty())) return;
    const Settings& settings = model.GetSettings();
    if (!settings.jingles_enabled) return;

    if (civ_name == L"Random")
    {
        PlayWave(settings.legacy_jingle_enabled ? random_legacy_jingle.data : random_jingle.data);
        return;
    }
    const int civ = model.FindCivIndex(civ_name);
    if (civ == CivCatalog::NONE) return;
    const bool legacy = settings.legacy_jingle_enabled && model.GetCiv(civ).legacy;
    PlayWave(legacy ? legacy_jingles[civ].data : jingles[civ].data);
}

void CivPickerView::StopSound() const { PlaySound(NULL, NULL, 0); }
