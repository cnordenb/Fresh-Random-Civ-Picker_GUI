#include "framework.h"
#include "Fresh-Random-Civ-Picker_CPPGUI.h"
#include <iostream>
#include <string>
#include <random>
#include <Windows.h>
#include "Resource.h"
#include <commctrl.h>
#include <shellapi.h>


#pragma comment(lib, "comctl32.lib")

#define IDM_TOGGLE_CHECK 32771
#define CIVS_MAX 45
#define MAX_LOADSTRING 100
#define HOTKEY_ID_TAB 1
#define HOTKEY_ID_SPACE 2
#define HOTKEY_ID_RETURN 3
#define HOTKEY_ID_ESC 4
#define DT_UNDERLINE 0x80000000



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
HBRUSH brush_white;
HBRUSH brush_black;
int iterator = 0; // Global variable to keep track of the count
int length = GetWindowTextLength(textfield_log);
std::wstring label_text = std::to_wstring(iterator + 1) + L"/" + std::to_wstring(CIVS_MAX);
std::wstring log_entry;
std::wstring log_text;
std::wstring hlabel_default;
bool mode_dark = false;
bool accessor_out_of_bounds = false;             // for unit testing
int times_drawn[CIVS_MAX] = { 0 };               // for unit testing
HFONT font_underline = NULL;
HWND tab;
std::vector<std::wstring> civs;

// Function declarations
void CreateTabs(HWND);
void ShowTabComponents(int);
void ResetProgram();
void DrawCiv();
void KillApplication();
void EnableHotkeys(HWND);
void DisableHotkeys(HWND);
void CreateUnderlineFont();
std::string ConvertToString(const std::wstring&);



// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    AboutDlgProc(HWND, UINT, WPARAM, LPARAM);
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
        0, 0, 400, 300,
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
        ShowWindow(button_draw, SW_SHOW);
        ShowWindow(label_corner, SW_SHOW);
        ShowWindow(label_centre, SW_SHOW);
		ShowWindow(textfield_log, SW_HIDE);
		ShowWindow(button_reset, SW_SHOW);
    }
    else if (tabIndex == 1)
    {
        ShowWindow(button_draw, SW_HIDE);
        ShowWindow(label_corner, SW_HIDE);
        ShowWindow(label_centre, SW_HIDE);
		ShowWindow(button_reset, SW_HIDE);
		ShowWindow(textfield_log, SW_SHOW);
        
    }
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

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
        CW_USEDEFAULT, 0, 400, 300, nullptr, nullptr, hInstance, nullptr);

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

        CreateTabs(hWnd);

        button_draw = CreateWindow(
            L"BUTTON",  // Predefined class; Unicode assumed 
            L"Draw",      // Button text 
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
            0,         // x position 
            0,         // y position 
            100,        // Button width
            30,         // Button height
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
			100,        // Button width
			30,         // Button height
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
		SetWindowPos(label_centre, NULL, (width - 80) / 2, (height - 15) / 2, 100, 15, SWP_NOZORDER);       // drawn civ label anchored to centre
		SetWindowPos(button_draw, NULL, (width - 100) / 2, (height + 50) / 2, 100, 30, SWP_NOZORDER);   // draw button anchored to centre
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
        pmmi->ptMinTrackSize.y = 300; // Minimum height

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

    iterator = 0;

    SetWindowText(label_corner, (L"0/" + std::to_wstring(CIVS_MAX)).c_str());     // resets remaining civs label
    SetWindowText(label_centre, L"?");                                      // resets drawn civ label


}

void DrawCiv()
{
	if (size(civs) == 0) ResetProgram();        // if all civs have been drawn, reset progrma and civ vector

    std::random_device rd;      // seeding random number
	std::mt19937 mt(rd());      // with Mersenne Twister

	std::shuffle(civs.begin(), civs.end(), mt); // shuffling civs vector
	std::wstring civ_name = civs.back();		// draws last element
	civs.pop_back();							// removes last element from pool

    std::string civ_name_str = ConvertToString(civ_name);

    iterator++;

    // Update the labels
    label_text = std::to_wstring(iterator) + L"/" + std::to_wstring(CIVS_MAX);
    SetWindowText(label_corner, label_text.c_str());
    SetWindowTextA(label_centre, civ_name_str.c_str());
    

    length = GetWindowTextLength(textfield_log);
    log_text.resize(length + 1);
    GetWindowText(textfield_log, &log_text[0], length + 1);
    log_text.pop_back(); // Remove the null terminator

    log_entry = std::wstring(civ_name.begin(), civ_name.end()) + L" (" + std::to_wstring(iterator) + L"/" + std::to_wstring(CIVS_MAX) + L")" + L"\r\n";
    log_text = log_entry + log_text;
    if (iterator == CIVS_MAX) log_text += L"\r\n";


    SetWindowText(textfield_log, log_text.c_str());

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
    int bufferSize = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string str(bufferSize, 0);
    WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, &str[0], bufferSize, nullptr, nullptr);
    return str;
}