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
int index_given = 0;
int slot_jumper = 0;
int length = GetWindowTextLength(textfield_log);
int civs_remaining = CIVS_MAX;
std::string civ_name = "";
std::wstring label_text = std::to_wstring(iterator + 1) + L"/" + std::to_wstring(CIVS_MAX);
std::wstring log_entry;
std::wstring log_text;
std::wstring hlabel_default;
bool civ_is_available[CIVS_MAX];
bool mode_dark = false;
bool accessor_out_of_bounds = false;             // for unit testing
int times_drawn[CIVS_MAX] = { 0 };               // for unit testing
HFONT font_underline = NULL;
HWND tab;

// Function declarations
std::string GetCivName(int);
void CreateTabs(HWND);
void ShowTabComponents(int);
int GetRandomIndex(int);
void ResetProgram();
void DrawCiv(HWND);
void KillApplication();
void EnableHotkeys(HWND);
void DisableHotkeys(HWND);
void DrawCivLogic();
void CreateUnderlineFont();




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

		if (wParam == HOTKEY_ID_SPACE) DrawCiv(hWnd);           // space for drawing civ

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
            DrawCiv(hWnd);     
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






std::string GetCivName(int index)         // returns the name of the civ based on the index
{
    switch (index)
    {
    case 0:
        return "Armenians";
    case 1:
        return "Aztecs";
    case 2:
        return "Bengalis";
    case 3:
        return "Berbers";
    case 4:
        return "Bohemians";
    case 5:
        return "Britons";
    case 6:
        return "Bulgarians";
    case 7:
        return "Burgundians";
    case 8:
        return "Burmese";
    case 9:
        return "Byzantines";
    case 10:
        return "Celts";
    case 11:
        return "Chinese";
    case 12:
        return "Cumans";
    case 13:
        return "Dravidians";
    case 14:
        return "Ethiopians";
    case 15:
        return "Franks";
    case 16:
        return "Georgians";
    case 17:
        return "Goths";
    case 18:
        return "Gurjaras";
    case 19:
        return "Hindustanis";
    case 20:
        return "Huns";
    case 21:
        return "Incas";
    case 22:
        return "Italians";
    case 23:
        return "Japanese";
    case 24:
        return "Khmer";
    case 25:
        return "Koreans";
    case 26:
        return "Lithuanians";
    case 27:
        return "Magyars";
    case 28:
        return "Malay";
    case 29:
        return "Malians";
    case 30:
        return "Mayans";
    case 31:
        return "Mongols";
    case 32:
        return "Persians";
    case 33:
        return "Poles";
    case 34:
        return "Portuguese";
    case 35:
        return "Romans";
    case 36:
        return "Saracens";
    case 37:
        return "Sicilians";
    case 38:
        return "Slavs";
    case 39:
        return "Spanish";
    case 40:
        return "Tatars";
    case 41:
        return "Teutons";
    case 42:
        return "Turks";
    case 43:
        return "Vietnamese";
    case 44:
        return "Vikings";
    default:
        return "(no civ chosen)";
    }
}

int GetRandomIndex(int max)
{                       // returns a random number between 0 and max
    
    std::random_device rd;  
    std::mt19937 mt(rd());  
    std::uniform_int_distribution<int> dist(0, max);

    return dist(mt);  // Return a random number between 0 and CIVS_MAX
}

void ResetProgram()
{ 							    // resets the program

    if (iterator > 0)                       // adds blank line to log before next iteration of civ drawing
    {
        length = GetWindowTextLength(textfield_log);
        log_text.resize(length + 1);
        GetWindowText(textfield_log, &log_text[0], length + 1);
        log_text.pop_back();

        log_text = L"\r\n" + log_text;
        SetWindowText(textfield_log, log_text.c_str());
    }

    for (int i = 0; i < CIVS_MAX; i++) {    // marks all civs as available
        civ_is_available[i] = true;
		times_drawn[i] = 0;                 // for unit testing
    }
	iterator = 0;                           // resets iterator
	civs_remaining = CIVS_MAX;				    // resets remaining civs

	SetWindowText(label_corner, (L"0/" + std::to_wstring(CIVS_MAX)).c_str());     // resets remaining civs label
	SetWindowText(label_centre, L"?");                                      // resets drawn civ label

    
}

void DrawCiv(HWND hWnd)
{
    if (iterator == CIVS_MAX)
    {
        ResetProgram();
        
    }

    
    index_given = GetRandomIndex(civs_remaining);

    slot_jumper = 0; // i keeps track of total civ elements, j keeps track of available civ elements (how many unavailable civs to skip)
	for (int i = 0; i < CIVS_MAX; i++)  // finds fresh random civ on. Only iterates through remaining amount of civs (see line 657)
    { 
		bool reset_internal = false;
        
		while (!civ_is_available[slot_jumper])  // defines amount of empty elements (already drawn civs) to jump over in one step instead of iterating through all of them
        { 
            if (slot_jumper == CIVS_MAX)  // in rare case j exceeds number of elements, restarts from 0 to prevent out of bounds exception
            {    
                reset_internal = true;
                slot_jumper = 0;
            }
            slot_jumper++;
			if (slot_jumper >= CIVS_MAX && reset_internal == true)  // ensures while loop exits after less than two iterations through the array
            {  
                reset_internal = false;
				break;
			}
        }
        if (i == index_given)
        {
           
            index_given = slot_jumper; // given index updated with increment to skip already drawn civs
        
			break; // fresh random civ found; end search for undrawn civ
        }

        slot_jumper++; // j incremented to keep up with i
    }

    civ_name = GetCivName(index_given);

    if (index_given < 0 || index_given >= sizeof(civ_is_available)) MessageBox(hWnd, L"Out of bounds! Line 677.\nRedrawing...", L"Error", MB_OK);
    if (index_given >= 0 && index_given < sizeof(civ_is_available)) civ_is_available[index_given] = false; // marks civ as unavailable
	else  // in rare case of out of bounds exception, restarts function.
    { 
        DrawCiv(hWnd);
        return;
    }

    
    iterator++;
    civs_remaining--;



    // Update the labels
    label_text = std::to_wstring(iterator) + L"/" + std::to_wstring(CIVS_MAX);
    SetWindowText(label_corner, label_text.c_str());
    SetWindowTextA(label_centre, civ_name.c_str());

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

void DrawCivLogic()
{                         // for unit testing
    if (iterator == CIVS_MAX)
    {
        ResetProgram();

    }


    index_given = GetRandomIndex(civs_remaining);

    slot_jumper = 0; 
    for (int i = 0; i < CIVS_MAX; i++)
    {
        bool internal_reset = false;

        while (!civ_is_available[slot_jumper])
        { 
            if (slot_jumper == CIVS_MAX)
            {    
                internal_reset = true;
                slot_jumper = 0;
            }
            slot_jumper++;
            if (slot_jumper >= CIVS_MAX && internal_reset == true)
            {  
                internal_reset = false;
                break;
            }
        }
        if (i == index_given)
        {

            index_given = slot_jumper; 

            break; 
        }

        slot_jumper++; 
    }

    civ_name = GetCivName(index_given);

    if (index_given < 0 || index_given >= sizeof(civ_is_available)) accessor_out_of_bounds = true;
	else accessor_out_of_bounds = false;
    if (index_given >= 0 && index_given < sizeof(civ_is_available))
    {
        civ_is_available[index_given] = false; 
		times_drawn[index_given]++; 
    }
    else
    { 
        DrawCivLogic();
        return;
    }


    iterator++;
    civs_remaining--;



    

}

void CreateUnderlineFont()
{
    LOGFONT lf;
    HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    GetObject(hFont, sizeof(LOGFONT), &lf);
    lf.lfUnderline = TRUE;
    font_underline = CreateFontIndirect(&lf);
}