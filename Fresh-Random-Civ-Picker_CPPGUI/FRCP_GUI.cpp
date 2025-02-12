#include "framework.h"
#include "FRCP_GUI.h"
#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <Windows.h>
#include "Resource.h"
#include <commctrl.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <strsafe.h>
#include <gdiplus.h>
#include <mmsystem.h>


#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "comctl32.lib")


#define IDM_TOGGLE_CHECK 32771
#define MAX_CIVS 45
#define MAX_LOADSTRING 100
#define HOTKEY_ID_TAB 1
#define HOTKEY_ID_SPACE 2
#define HOTKEY_ID_RETURN 3
#define HOTKEY_ID_ESC 4
#define DT_UNDERLINE 0x80000000
#define MAX_LENGTH 10 // currently longest civ name is 10 characters (portuguese)
#define MIN_WIDTH 400
#define MIN_HEIGHT 300
#define BUTTON_WIDTH 100
#define BUTTON_HEIGHT 30


// Global Variables
PAINTSTRUCT paint_struct;
HDC device_handling_context;
RECT rectangle_struct;
HINSTANCE instance;                                // current instance
WCHAR title[MAX_LOADSTRING];                  // The title bar text
WCHAR window_class[MAX_LOADSTRING];            // the main window class name
HWND label_corner;
HWND label_centre;
HWND button_draw;
HWND button_reset;
HWND textfield_log;
HWND civ_icon;
HBRUSH brush_white;
HBRUSH brush_black;

HBITMAP icon_armenians, icon_aztecs, icon_bengalis, icon_berber, icon_bohemians,
        icon_britons, icon_bulgarians, icon_burgundians, icon_burmese, icon_byzantines,
        icon_celts, icon_chinese, icon_cumans, icon_dravidians, icon_ethiopians,
        icon_franks, icon_georgians, icon_goths, icon_gurjaras, icon_huns, icon_incas,
        icon_hindustanis, icon_italians, icon_japanese, icon_khmer, icon_koreans,
        icon_lithuanians, icon_magyars, icon_malay, icon_malians, icon_mayans,
        icon_mongols, icon_persians, icon_poles, icon_portuguese, icon_romans,
        icon_saracens, icon_sicilians, icon_slavs, icon_spanish, icon_tatars,
        icon_teutons, icon_turks, icon_vietnamese, icon_vikings, icon_random;

int iterator = 0; // Global variable to keep track of the count
int length = GetWindowTextLength(textfield_log);
int tab_current = 0;
std::wstring label_text = std::to_wstring(iterator + 1) + L"/" + std::to_wstring(MAX_CIVS);
std::wstring log_entry;
std::wstring log_text;
std::wstring hlabel_default;
std::wstring current_civ = L"Random";
bool mode_dark = false;
bool icons_enabled = false;
bool jingles_enabled = false;
bool accessor_out_of_bounds = false;             // for unit testing
int times_drawn[MAX_CIVS] = { 0 };               // for unit testing
HFONT font_underline = NULL;
HWND tab;
std::vector<std::wstring> civs;

// Function declarations
HBITMAP FetchIcon(std::wstring);
void CreateTabs(HWND);
void ShowTabComponents(int);
void ResetProgram();
void DrawCiv();
void KillApplication();
void EnableHotkeys(HWND);
void DisableHotkeys(HWND);
void CreateUnderlineFont();
void LoadImages();
void PlayJingle(std::wstring);
std::string ConvertToString(const std::wstring&);



// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    AboutDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    OptionsDlgProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    HyperlinkProc(HWND, UINT, WPARAM, LPARAM);


// Function to create tabs
void CreateTabs(HWND hWnd)
{
    // Initialize common controls
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_TAB_CLASSES;
    InitCommonControlsEx(&icex);

    // Create the tab control
    tab = CreateWindow(WC_TABCONTROL, L"",
        WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE,
        0, 0, MIN_WIDTH, MIN_HEIGHT,
        hWnd, NULL, instance, NULL);

    // Add tabs
    TCITEM tie;
    tie.mask = TCIF_TEXT;

    // Change the type of pszText to LPCWSTR
    tie.pszText = const_cast<LPWSTR>(L"Draw Civ");
    TabCtrl_InsertItem(tab, 0, &tie);

    tie.pszText = const_cast<LPWSTR>(L"Log");
    TabCtrl_InsertItem(tab, 1, &tie);
}

// Function to show/hide components based on the selected tab
void ShowTabComponents(int tabIndex)
{
    if (tabIndex == 0)
    {
        tab_current = 0;
        ShowWindow(button_draw, SW_SHOW);
        ShowWindow(label_corner, SW_SHOW);
        ShowWindow(label_centre, SW_SHOW);
		ShowWindow(textfield_log, SW_HIDE);
		ShowWindow(button_reset, SW_SHOW);
        if (icons_enabled) ShowWindow(civ_icon, SW_SHOW);
    }
    else if (tabIndex == 1)
    {
        tab_current = 1;
        ShowWindow(button_draw, SW_HIDE);
        ShowWindow(label_corner, SW_HIDE);
        ShowWindow(label_centre, SW_HIDE);
		ShowWindow(button_reset, SW_HIDE);
		ShowWindow(textfield_log, SW_SHOW);
        if (icons_enabled) ShowWindow(civ_icon, SW_HIDE);
        
    }
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize GDI+
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);


    CreateUnderlineFont();
    ResetProgram();


    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, title, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WINDOWSPROJECT1, window_class, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINDOWSPROJECT1));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    // Shutdown GDI+
    Gdiplus::GdiplusShutdown(gdiplusToken);


    DeleteObject(font_underline);

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINDOWSPROJECT1));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WINDOWSPROJECT1);
    wcex.lpszClassName  = window_class;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    instance = hInstance; // Store instance handle in our global variable

	wcscpy_s(title, L"Fresh Random Civ Picker");

    // Set the default window size to 400x300
    HWND hWnd = CreateWindowW(window_class, title, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, 400, 350, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{


    switch (message)
    {

       
    case WM_CREATE:

        LoadImages();

        CreateTabs(hWnd);

        button_draw = CreateWindow(
            L"BUTTON",  // Predefined class; Unicode assumed 
            L"Draw",      // Button text 
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
            0,         // x position 
            0,         // y position 
            BUTTON_WIDTH,      
            BUTTON_HEIGHT,      
            hWnd,       // Parent window
            (HMENU)1,       // No menu.
            (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
            NULL);      // Pointer not needed.

		button_reset = CreateWindow(
			L"BUTTON",  // Predefined class; Unicode assumed 
			L"Reset",      // Button text 
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
		    10,         // x position 
			200,         // y position 
			BUTTON_WIDTH,       
			BUTTON_HEIGHT,         
			hWnd,       // Parent window
			(HMENU)2,       // No menu.
			(HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
			NULL);      // Pointer not needed.

        label_corner = CreateWindow(
            L"STATIC",  // Predefined class; Unicode assumed
            L"",  // Label text from variable..
            WS_VISIBLE | WS_CHILD,  // Styles
            0,  // x position (will be set in WM_SIZE)
            0,  // y position (will be set in WM_SIZE)
            30,  // Label width
            15,  // Label height
            tab,  // Parent window
            NULL,  // No menu.
            (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
            NULL);  // Pointer not needed.

        label_centre = CreateWindow(
            L"STATIC",  // Predefined class; Unicode assumed
            L"?",  // Label text
            WS_VISIBLE | WS_CHILD,  // Styles
            0,  // x position (will be set in WM_SIZE)
            0,  // y position (will be set in WM_SIZE)
            100,  // Label width
            15,  // Label height
            hWnd,  // Parent window
            NULL,  // No menu.
            (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
            NULL);  // Pointer not needed.

		textfield_log = CreateWindow(
			L"EDIT",  // Predefined class; Unicode assumed
			L"",  // Label text
			WS_VISIBLE | WS_CHILD | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,  // Styles
			10,  // x position (will be set in WM_SIZE)
			25,  // y position (will be set in WM_SIZE)
			350,  // Label width
			200,  // Label height
			tab,  // Parent window
			NULL,  // No menu.
			(HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
			NULL);  // Pointer not needed.

        civ_icon = CreateWindowW(
            L"STATIC",  // Predefined class; Unicode assumed
            NULL,  // Label text
            WS_VISIBLE | WS_CHILD | SS_BITMAP,  // Styles
            0,  // x position (will be set in WM_SIZE)
            0,  // y position (will be set in WM_SIZE)
            104,  // Label width
            104,  // Label height
            hWnd,  // Parent window
            NULL,
            NULL,
            NULL);  // Pointer not needed.


		

        ShowWindow(civ_icon, SW_HIDE);
		ShowWindow(textfield_log, SW_HIDE);


		ResetProgram();    // resetter is called in order to enable remaining civ indicator label (hLabel)

		brush_black = CreateSolidBrush(RGB(0, 0, 0));
		brush_white = CreateSolidBrush(RGB(255, 255, 255));
		
        EnableHotkeys(hWnd);

        break; 

    
        

    case WM_SIZE:
    {
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);

        SetWindowPos(tab, NULL, 0, 0, width, height, SWP_NOZORDER);                                        // tab size anchored to window size
		SetWindowPos(label_corner, NULL, width - 50, height - 20, 40, 15, SWP_NOZORDER);                          // remaining civ indicator anchored to bottom right corner 
        SetWindowPos(label_centre, NULL, (width - 80) / 2, (height + 35) / 2, 100, 15, SWP_NOZORDER);       // drawn civ label anchored to centre
        SetWindowPos(civ_icon, NULL, (width - 100) / 2, (height - 180) / 2, 104, 104, SWP_NOZORDER);       // civ icon anchored to centre
        SetWindowPos(button_draw, NULL, (width - 100) / 2, (height + 100) / 2, 100, 30, SWP_NOZORDER);   // draw button anchored to centre
        SetWindowPos(button_reset, NULL, 10, height - 40, 100, 30, SWP_NOZORDER);                           // reset button anchored to bottom left corner
		SetWindowPos(textfield_log, NULL, 10, 25, width - 20, height - 50, SWP_NOZORDER);                   // log text field anchored to window size

    }
        break;

    case WM_ACTIVATE: // re-enables hotkeys when window returns to foreground 
    
		if (wParam == WA_ACTIVE || wParam == WA_CLICKACTIVE) EnableHotkeys(hWnd);
		else if (wParam == WA_INACTIVE) DisableHotkeys(hWnd);

        break;
    
    

    case WM_CTLCOLORSTATIC:
    {
        HDC hdcStatic = (HDC)wParam;
        SetTextColor(hdcStatic, mode_dark ? RGB(255, 255, 255) : RGB(0, 0, 0));
        SetBkColor(hdcStatic, mode_dark ? RGB(0, 0, 0) : RGB(255, 255, 255));
        return (INT_PTR)(mode_dark ? brush_black : brush_white);
    }
        break;

    case WM_ERASEBKGND:
    {
        HDC hdc = (HDC)wParam;
        RECT rect;
        GetClientRect(hWnd, &rect);
        FillRect(hdc, &rect, mode_dark ? brush_black : brush_white);
        return 1;
    }
        break;

    case WM_CTLCOLORBTN:
    {
        HDC hdcButton = (HDC)wParam;
        SetTextColor(hdcButton, mode_dark ? RGB(255, 255, 255) : RGB(0, 0, 0));
        SetBkColor(hdcButton, mode_dark ? RGB(0, 0, 0) : RGB(255, 255, 255));
        return (INT_PTR)(mode_dark ? brush_black : brush_white);
    }
        break;

    case WM_NOTIFY:
    {
        LPNMHDR pnmhdr = (LPNMHDR)lParam;
        if (pnmhdr->hwndFrom == tab && pnmhdr->code == TCN_SELCHANGE)
        {
            int tabIndex = TabCtrl_GetCurSel(tab);
            ShowTabComponents(tabIndex);
        }
    }
        break;

	case WM_GETMINMAXINFO:                                  // minimum window size
    {
        MINMAXINFO* pmmi = (MINMAXINFO*)lParam;
        pmmi->ptMinTrackSize.x = 400; // Minimum width
        pmmi->ptMinTrackSize.y = 350; // Minimum height

    }
        break;

        
    case WM_HOTKEY:
    {
        
		if (GetForegroundWindow() != hWnd)  // disables hotkeys if window is not in foreground
        {                
            DisableHotkeys(hWnd);
            break;
        }
		else EnableHotkeys(hWnd);

        if (wParam == HOTKEY_ID_TAB)                        // tab for switching tabs
        {
            int tabIndex = TabCtrl_GetCurSel(tab);
            int newTabIndex = tabIndex == 0 ? 1 : 0;
            TabCtrl_SetCurSel(tab, newTabIndex);
            ShowTabComponents(newTabIndex);
        }

		if (wParam == HOTKEY_ID_SPACE) DrawCiv();           // space for drawing civ

		if (wParam == HOTKEY_ID_RETURN) ResetProgram();            // return for resetting

		if (wParam == HOTKEY_ID_ESC) KillApplication();                // escape for exiting

    }
	
        break;


    case WM_COMMAND:                                        // action listener for clicks
    {
        int wmId = LOWORD(wParam);
        // Parse the menu selections:     
            


        switch (wmId) 
        {
        case IDM_ABOUT:                                     // "About"
            DialogBox(instance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, AboutDlgProc);
            break;
		case IDM_EXIT:                                      // "Exit"
            DestroyWindow(hWnd);
            break;
        case IDM_OPTIONS:
            DialogBox(instance, MAKEINTRESOURCE(IDD_OPTIONS), hWnd, OptionsDlgProc);
            break;
		case IDM_TOGGLE_CHECK:                              // "Dark Mode (beta)"
           
            
            mode_dark = !mode_dark;
            CheckMenuItem(GetMenu(hWnd), IDM_TOGGLE_CHECK, mode_dark ? MF_CHECKED : MF_UNCHECKED);
            InvalidateRect(hWnd, NULL, TRUE);
            EnumChildWindows(hWnd, [](HWND hwnd, LPARAM lParam) -> BOOL {
                InvalidateRect(hwnd, NULL, TRUE);
                return TRUE;
                }, 0);
            
            break;
		case IDM_GITHUB:                                    // "GitHub"
            ShellExecute(0, 0, L"https://github.com/cnordenb/Fresh-Random-Civ-Picker_CPPGUI", 0, 0, SW_SHOW);
            break;
		case IDM_WEBVERSION:                                // "Web Version"
			ShellExecute(0, 0, L"https://cnordenb.github.io/Fresh-Random-Civ-Picker_web/", 0, 0, SW_SHOW);
			break;
        case 1:                                             // "Draw"            
            DrawCiv();     
            break;
		case 2:                                             // "Reset"
			ResetProgram();
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
        break;
    
    case WM_PAINT:
        {       
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
		KillApplication();
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

INT_PTR CALLBACK AboutDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static WNDPROC oldProc;
    switch (message)
    {
    case WM_INITDIALOG:
    {
        
        HWND hwndHyperlink = GetDlgItem(hDlg, IDC_HYPERLINK);
        oldProc = (WNDPROC)SetWindowLongPtr(hwndHyperlink, GWLP_WNDPROC, (LONG_PTR)HyperlinkProc);
        return(INT_PTR)TRUE;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return(INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}


INT_PTR CALLBACK OptionsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    //static WNDPROC oldProc;

    switch (message)
    {
    case WM_INITDIALOG:
    {
        
        //oldProc = (WNDPROC)SetWindowLongPtr(hwndHyperlink, GWLP_WNDPROC, (LONG_PTR)HyperlinkProc);
        CheckDlgButton(hDlg, IDC_CHECKBOX_ICONS, icons_enabled ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHECKBOX_JINGLES, jingles_enabled ? BST_CHECKED : BST_UNCHECKED);

        return(INT_PTR)TRUE;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
			icons_enabled = IsDlgButtonChecked(hDlg, IDC_CHECKBOX_ICONS) == BST_CHECKED;
			jingles_enabled = IsDlgButtonChecked(hDlg, IDC_CHECKBOX_JINGLES) == BST_CHECKED;

            // Handle the checkbox state as needed
            if (icons_enabled && tab_current == 0)
            {
                ShowWindow(civ_icon, SW_SHOW);
            }
            else
            {
                ShowWindow(civ_icon, SW_HIDE);
            }

            // Handle the checkbox state as needed
            if (jingles_enabled) PlayJingle(current_civ);
            
            



            EndDialog(hDlg, LOWORD(wParam));
            return(INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}


LRESULT CALLBACK HyperlinkProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    
    switch (msg)
    {
    case WM_PAINT:    
        device_handling_context = BeginPaint(hwnd, &paint_struct);
        SetTextColor(device_handling_context, RGB(0, 0, 255)); // Blue color
        SetBkMode(device_handling_context, TRANSPARENT);
        SelectObject(device_handling_context, font_underline);        
        GetClientRect(hwnd, &rectangle_struct);
        DrawText(device_handling_context, L"Hjörleif", -1, &rectangle_struct, DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX | DT_UNDERLINE);
        EndPaint(hwnd, &paint_struct);
        return 0;    
    case WM_LBUTTONDOWN:    
        ShellExecute(NULL, L"open", L"https://linktr.ee/hjoerleif", NULL, NULL, SW_SHOWNORMAL);
        return 0;    
    case WM_SETCURSOR:
		SetCursor(LoadCursor(NULL, IDC_HAND));
		return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}





void ResetProgram()
{ 
    if (iterator > 0)                       // adds blank line to log before next iteration of civ drawing
    {
        length = GetWindowTextLength(textfield_log);
        log_text.resize(length + 1);
        GetWindowText(textfield_log, &log_text[0], length + 1);
        log_text.pop_back();

        log_text = L"\r\n" + log_text;
        SetWindowText(textfield_log, log_text.c_str());
    }


	civs = { L"Armenians", L"Aztecs", L"Bengalis", L"Berbers", L"Bohemians", L"Britons",
        L"Bulgarians", L"Burgundians", L"Burmese", L"Byzantines", L"Celts", L"Chinese", 
        L"Cumans", L"Dravidians", L"Ethiopians", L"Franks", L"Georgians", L"Goths", L"Gurjaras",
        L"Hindustanis", L"Huns", L"Incas", L"Italians", L"Japanese", L"Khmer", L"Koreans", 
        L"Lithuanians", L"Magyars", L"Malay", L"Malians", L"Mayans", L"Mongols", L"Persians",
        L"Poles", L"Portuguese", L"Romans", L"Saracens", L"Sicilians", L"Slavs", L"Spanish", 
        L"Tatars", L"Teutons", L"Turks", L"Vietnamese", L"Vikings" };

	current_civ = L"Random";
    iterator = 0;

    SetWindowText(label_corner, (L"0/" + std::to_wstring(MAX_CIVS)).c_str());     // resets remaining civs label
    SetWindowText(label_centre, L"?");                                      // resets drawn civ label
    SendMessageW(civ_icon, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)icon_random);
	if (jingles_enabled) PlayJingle(current_civ);


}

void DrawCiv()
{
	if (size(civs) == 0) ResetProgram();        // if all civs have been drawn, reset progrma and civ vector

    std::random_device rd;      // seeding random number
	std::mt19937 mt(rd());      // with Mersenne Twister

	std::shuffle(civs.begin(), civs.end(), mt); // shuffling civs vector
	current_civ = civs.back();		// draws last element
	civs.pop_back();							// removes last element from pool

    std::string civ_name_str = ConvertToString(current_civ);

    iterator++;

    // Update the labels
    label_text = std::to_wstring(iterator) + L"/" + std::to_wstring(MAX_CIVS);
    SetWindowText(label_corner, label_text.c_str());
    SetWindowTextA(label_centre, civ_name_str.c_str());

    HBITMAP drawn_civ_icon = FetchIcon(current_civ);
    SendMessageW(civ_icon, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)drawn_civ_icon);

    
   


    length = GetWindowTextLength(textfield_log);
    log_text.resize(length + 1);
    GetWindowText(textfield_log, &log_text[0], length + 1);
    log_text.pop_back(); // Remove the null terminator

    log_entry = std::wstring(current_civ.begin(), current_civ.end()) + L" (" + std::to_wstring(iterator) + L"/" + std::to_wstring(MAX_CIVS) + L")" + L"\r\n";
    log_text = log_entry + log_text;
    if (iterator == MAX_CIVS) log_text += L"\r\n";



    SetWindowText(textfield_log, log_text.c_str());
    if (tab_current == 1) ShowWindow(civ_icon, SW_HIDE);

    if (jingles_enabled) PlayJingle(current_civ);
    



}

void EnableHotkeys(HWND hWnd)
{
	RegisterHotKey(hWnd, HOTKEY_ID_TAB, 0, VK_TAB);
	RegisterHotKey(hWnd, HOTKEY_ID_SPACE, 0, VK_SPACE);
	RegisterHotKey(hWnd, HOTKEY_ID_RETURN, 0, VK_RETURN);
	RegisterHotKey(hWnd, HOTKEY_ID_ESC, 0, VK_ESCAPE);
}

void DisableHotkeys(HWND hWnd)
{
	UnregisterHotKey(hWnd, HOTKEY_ID_TAB);
	UnregisterHotKey(hWnd, HOTKEY_ID_SPACE);
	UnregisterHotKey(hWnd, HOTKEY_ID_RETURN);
	UnregisterHotKey(hWnd, HOTKEY_ID_ESC);
}

void KillApplication()
{
    DeleteObject(brush_white);
    DeleteObject(brush_black);
    PostQuitMessage(0);
}


void CreateUnderlineFont()
{
    LOGFONT lf;
    HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    GetObject(hFont, sizeof(LOGFONT), &lf);
    lf.lfUnderline = TRUE;
    font_underline = CreateFontIndirect(&lf);
}

std::string ConvertToString(const std::wstring& wstr) {
    std::string str;
    str.reserve(MAX_LENGTH);
    WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, &str[0], MAX_LENGTH, nullptr, nullptr);
    return str;
}

void LoadImages() { 
    icon_armenians = (HBITMAP)LoadImageW(NULL, L"civ_icons\\armenians.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_aztecs = (HBITMAP)LoadImageW(NULL, L"civ_icons\\aztecs.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_bengalis = (HBITMAP)LoadImageW(NULL, L"civ_icons\\bengalis.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_berber = (HBITMAP)LoadImageW(NULL, L"civ_icons\\berber.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_bohemians = (HBITMAP)LoadImageW(NULL, L"civ_icons\\bohemians.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_britons = (HBITMAP)LoadImageW(NULL, L"civ_icons\\britons.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_bulgarians = (HBITMAP)LoadImageW(NULL, L"civ_icons\\bulgarians.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_burgundians = (HBITMAP)LoadImageW(NULL, L"civ_icons\\burgundians.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_burmese = (HBITMAP)LoadImageW(NULL, L"civ_icons\\burmese.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_byzantines = (HBITMAP)LoadImageW(NULL, L"civ_icons\\byzantines.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_celts = (HBITMAP)LoadImageW(NULL, L"civ_icons\\celts.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_chinese = (HBITMAP)LoadImageW(NULL, L"civ_icons\\chinese.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_cumans = (HBITMAP)LoadImageW(NULL, L"civ_icons\\cumans.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_dravidians = (HBITMAP)LoadImageW(NULL, L"civ_icons\\dravidians.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_ethiopians = (HBITMAP)LoadImageW(NULL, L"civ_icons\\ethiopians.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_franks = (HBITMAP)LoadImageW(NULL, L"civ_icons\\franks.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_georgians = (HBITMAP)LoadImageW(NULL, L"civ_icons\\georgians.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_goths = (HBITMAP)LoadImageW(NULL, L"civ_icons\\goths.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_gurjaras = (HBITMAP)LoadImageW(NULL, L"civ_icons\\gurjaras.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_huns = (HBITMAP)LoadImageW(NULL, L"civ_icons\\huns.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_incas = (HBITMAP)LoadImageW(NULL, L"civ_icons\\incas.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_hindustanis = (HBITMAP)LoadImageW(NULL, L"civ_icons\\hindustanis.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_italians = (HBITMAP)LoadImageW(NULL, L"civ_icons\\italians.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_japanese = (HBITMAP)LoadImageW(NULL, L"civ_icons\\japanese.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_khmer = (HBITMAP)LoadImageW(NULL, L"civ_icons\\khmer.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_koreans = (HBITMAP)LoadImageW(NULL, L"civ_icons\\koreans.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_lithuanians = (HBITMAP)LoadImageW(NULL, L"civ_icons\\lithuanians.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_magyars = (HBITMAP)LoadImageW(NULL, L"civ_icons\\magyars.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_malay = (HBITMAP)LoadImageW(NULL, L"civ_icons\\malay.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_malians = (HBITMAP)LoadImageW(NULL, L"civ_icons\\malians.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_mayans = (HBITMAP)LoadImageW(NULL, L"civ_icons\\mayans.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_mongols = (HBITMAP)LoadImageW(NULL, L"civ_icons\\mongols.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_persians = (HBITMAP)LoadImageW(NULL, L"civ_icons\\persians.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_poles = (HBITMAP)LoadImageW(NULL, L"civ_icons\\poles.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_portuguese = (HBITMAP)LoadImageW(NULL, L"civ_icons\\portuguese.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_romans = (HBITMAP)LoadImageW(NULL, L"civ_icons\\romans.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_saracens = (HBITMAP)LoadImageW(NULL, L"civ_icons\\saracens.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_sicilians = (HBITMAP)LoadImageW(NULL, L"civ_icons\\sicilians.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_slavs = (HBITMAP)LoadImageW(NULL, L"civ_icons\\slavs.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_spanish = (HBITMAP)LoadImageW(NULL, L"civ_icons\\spanish.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_tatars = (HBITMAP)LoadImageW(NULL, L"civ_icons\\tatars.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_teutons = (HBITMAP)LoadImageW(NULL, L"civ_icons\\teutons.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_turks = (HBITMAP)LoadImageW(NULL, L"civ_icons\\turks.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_vietnamese = (HBITMAP)LoadImageW(NULL, L"civ_icons\\vietnamese.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_vikings = (HBITMAP)LoadImageW(NULL, L"civ_icons\\vikings.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    icon_random = (HBITMAP)LoadImageW(NULL, L"civ_icons\\random.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
}

HBITMAP FetchIcon(std::wstring civ_name) {
	if (civ_name == L"Armenians") return icon_armenians;
	if (civ_name == L"Aztecs") return icon_aztecs;
	if (civ_name == L"Bengalis") return icon_bengalis;
	if (civ_name == L"Berbers") return icon_berber;
	if (civ_name == L"Bohemians") return icon_bohemians;
	if (civ_name == L"Britons") return icon_britons;
	if (civ_name == L"Bulgarians") return icon_bulgarians;
	if (civ_name == L"Burgundians") return icon_burgundians;
	if (civ_name == L"Burmese") return icon_burmese;
	if (civ_name == L"Byzantines") return icon_byzantines;
	if (civ_name == L"Celts") return icon_celts;
	if (civ_name == L"Chinese") return icon_chinese;
	if (civ_name == L"Cumans") return icon_cumans;
	if (civ_name == L"Dravidians") return icon_dravidians;
	if (civ_name == L"Ethiopians") return icon_ethiopians;
	if (civ_name == L"Franks") return icon_franks;
	if (civ_name == L"Georgians") return icon_georgians;
	if (civ_name == L"Goths") return icon_goths;
	if (civ_name == L"Gurjaras") return icon_gurjaras;
	if (civ_name == L"Hindustanis") return icon_hindustanis;
	if (civ_name == L"Huns") return icon_huns;
	if (civ_name == L"Incas") return icon_incas;
	if (civ_name == L"Italians") return icon_italians;
	if (civ_name == L"Japanese") return icon_japanese;
	if (civ_name == L"Khmer") return icon_khmer;
	if (civ_name == L"Koreans") return icon_koreans;
	if (civ_name == L"Lithuanians") return icon_lithuanians;
	if (civ_name == L"Magyars") return icon_magyars;
	if (civ_name == L"Malay") return icon_malay;
	if (civ_name == L"Malians") return icon_malians;
	if (civ_name == L"Mayans") return icon_mayans;
	if (civ_name == L"Mongols") return icon_mongols;
	if (civ_name == L"Persians") return icon_persians;
	if (civ_name == L"Poles") return icon_poles;
	if (civ_name == L"Portuguese") return icon_portuguese;
	if (civ_name == L"Romans") return icon_romans;
	if (civ_name == L"Saracens") return icon_saracens;
	if (civ_name == L"Sicilians") return icon_sicilians;
	if (civ_name == L"Slavs") return icon_slavs;
	if (civ_name == L"Spanish") return icon_spanish;
	if (civ_name == L"Tatars") return icon_tatars;
	if (civ_name == L"Teutons") return icon_teutons;
	if (civ_name == L"Turks") return icon_turks;
	if (civ_name == L"Vietnamese") return icon_vietnamese;
	if (civ_name == L"Vikings") return icon_vikings;
}

void PlayJingle(std::wstring civ_name) {



    std::wstring processed_civ_name = civ_name;
    processed_civ_name[0] = std::tolower(processed_civ_name[0]);
	std::wstring jingle_path = L"jingles\\" + processed_civ_name + L".wav";

    
    
    PlaySound(jingle_path.c_str(), NULL, SND_FILENAME | SND_ASYNC);


    

	
    
    
}