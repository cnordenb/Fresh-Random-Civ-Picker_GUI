#include "framework.h"
#include "Fresh-Random-Civ-Picker_CPPGUI.h"
#include <iostream>
#include <string>
#include <random>
#include <cstdio>
#include <Windows.h>
#include <commctrl.h>
#include <shellapi.h>


#pragma comment(lib, "comctl32.lib")

#define IDM_TOGGLE_CHECK 32771
#define IDM_GITHUB 32772
#define CIVS_MAX 45
#define MAX_LOADSTRING 100
#define HOTKEY_ID_TAB 1
#define HOTKEY_ID_SPACE 2
#define HOTKEY_ID_RETURN 3
#define HOTKEY_ID_ESC 4



// Global Variables
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HWND hLabel;
HWND hCenterLabel;
HWND hGenerateButton;
HWND hResetButton;
HWND hLogTextField;
WCHAR szSizeText[50];
HBRUSH hBrushWhite;
HBRUSH hBrushBlack;
int iterator = 0; // Global variable to keep track of the count
int given_index = 0;
int j = 0;
int length = GetWindowTextLength(hLogTextField);
int remaining = CIVS_MAX;
std::string civ = "";
std::wstring labelText = std::to_wstring(iterator + 1) + L"/" + std::to_wstring(CIVS_MAX);
std::wstring newLogEntry;
std::wstring logText;
std::wstring default_hlabel;
bool available[CIVS_MAX];
bool isChecked = false;

// Function declarations
std::string civ_name(int);
void CreateTabs(HWND);
void ShowTabComponents(int);
int result(int);
void reset();
void draw_civ();
void kill_application();
void enable_hotkeys(HWND);
void disable_hotkeys(HWND);




// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

HWND hTab;

// Function to create tabs
void CreateTabs(HWND hWnd)
{
    // Initialize common controls
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_TAB_CLASSES;
    InitCommonControlsEx(&icex);

    // Create the tab control
    hTab = CreateWindow(WC_TABCONTROL, L"",
        WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE,
        0, 0, 400, 300,
        hWnd, NULL, hInst, NULL);

    // Add tabs
    TCITEM tie;
    tie.mask = TCIF_TEXT;

    // Change the type of pszText to LPCWSTR
    tie.pszText = const_cast<LPWSTR>(L"Draw Civ");
    TabCtrl_InsertItem(hTab, 0, &tie);

    tie.pszText = const_cast<LPWSTR>(L"Log");
    TabCtrl_InsertItem(hTab, 1, &tie);
}

// Function to show/hide components based on the selected tab
void ShowTabComponents(int tabIndex)
{
    if (tabIndex == 0)
    {
        ShowWindow(hGenerateButton, SW_SHOW);
        ShowWindow(hLabel, SW_SHOW);
        ShowWindow(hCenterLabel, SW_SHOW);
		ShowWindow(hLogTextField, SW_HIDE);
		ShowWindow(hResetButton, SW_SHOW);
    }
    else if (tabIndex == 1)
    {
        ShowWindow(hGenerateButton, SW_HIDE);
        ShowWindow(hLabel, SW_HIDE);
        ShowWindow(hCenterLabel, SW_HIDE);
		ShowWindow(hResetButton, SW_HIDE);
		ShowWindow(hLogTextField, SW_SHOW);
        // Add components for the second tab here
    }
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    
    reset();

    /*AllocConsole();
    
    // Redirect standard output to the console
    FILE *fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);

    // Now you can use printf and std::cout to output to the console
    printf("Console window successfully attached.\n");*/


    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WINDOWSPROJECT1, szWindowClass, MAX_LOADSTRING);
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
    wcex.lpszClassName  = szWindowClass;
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
    hInst = hInstance; // Store instance handle in our global variable

	wcscpy_s(szTitle, L"Fresh Random Civ Picker");

    // Set the default window size to 400x300
    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
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

        hGenerateButton = CreateWindow(
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

		hResetButton = CreateWindow(
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

        hLabel = CreateWindow(
            L"STATIC",  // Predefined class; Unicode assumed
            L"",  // Label text from variable..
            WS_VISIBLE | WS_CHILD,  // Styles
            0,  // x position (will be set in WM_SIZE)
            0,  // y position (will be set in WM_SIZE)
            30,  // Label width
            15,  // Label height
            hTab,  // Parent window
            NULL,  // No menu.
            (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
            NULL);  // Pointer not needed.

        hCenterLabel = CreateWindow(
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

		hLogTextField = CreateWindow(
			L"EDIT",  // Predefined class; Unicode assumed
			L"",  // Label text
			WS_VISIBLE | WS_CHILD | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,  // Styles
			10,  // x position (will be set in WM_SIZE)
			25,  // y position (will be set in WM_SIZE)
			350,  // Label width
			200,  // Label height
			hTab,  // Parent window
			NULL,  // No menu.
			(HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
			NULL);  // Pointer not needed.

		ShowWindow(hLogTextField, SW_HIDE);

        //default_hlabel = L"0/" + std::to_wstring(CIVS_MAX);

        reset();

		hBrushBlack = CreateSolidBrush(RGB(0, 0, 0));
		hBrushWhite = CreateSolidBrush(RGB(255, 255, 255));
		
        enable_hotkeys(hWnd);

        break; 

    
        

    case WM_SIZE:
    {
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);

        SetWindowPos(hTab, NULL, 0, 0, width, height, SWP_NOZORDER);

        SetWindowPos(hLabel, NULL, width - 50, height - 20, 40, 15, SWP_NOZORDER);
        SetWindowPos(hCenterLabel, NULL, (width - 80) / 2, (height - 15) / 2, 100, 15, SWP_NOZORDER);  // Centered positio
        SetWindowPos(hGenerateButton, NULL, (width - 100) / 2, (height + 50) / 2, 100, 30, SWP_NOZORDER);
        SetWindowPos(hResetButton, NULL, 10, height - 40, 100, 30, SWP_NOZORDER); // Anchored to bottom left corner
		SetWindowPos(hLogTextField, NULL, 10, 25, width - 20, height - 50, SWP_NOZORDER);

    }
    break;

    case WM_ACTIVATE: // re-enables hotkeys when window returns to foreground
    
		if (wParam == WA_ACTIVE || wParam == WA_CLICKACTIVE) enable_hotkeys(hWnd);
		else if (wParam == WA_INACTIVE) disable_hotkeys(hWnd);

        break;
    
    

    case WM_CTLCOLORSTATIC:
    {
        HDC hdcStatic = (HDC)wParam;
        SetTextColor(hdcStatic, isChecked ? RGB(255, 255, 255) : RGB(0, 0, 0));
        SetBkColor(hdcStatic, isChecked ? RGB(0, 0, 0) : RGB(255, 255, 255));
        return (INT_PTR)(isChecked ? hBrushBlack : hBrushWhite);
    }
    break;

    case WM_ERASEBKGND:
    {
        HDC hdc = (HDC)wParam;
        RECT rect;
        GetClientRect(hWnd, &rect);
        FillRect(hdc, &rect, isChecked ? hBrushBlack : hBrushWhite);
        return 1;
    }
    break;

    case WM_CTLCOLORBTN:
    {
        HDC hdcButton = (HDC)wParam;
        SetTextColor(hdcButton, isChecked ? RGB(255, 255, 255) : RGB(0, 0, 0));
        SetBkColor(hdcButton, isChecked ? RGB(0, 0, 0) : RGB(255, 255, 255));
        return (INT_PTR)(isChecked ? hBrushBlack : hBrushWhite);
    }
    break;

    case WM_NOTIFY:
    {
        LPNMHDR pnmhdr = (LPNMHDR)lParam;
        if (pnmhdr->hwndFrom == hTab && pnmhdr->code == TCN_SELCHANGE)
        {
            int tabIndex = TabCtrl_GetCurSel(hTab);
            ShowTabComponents(tabIndex);
        }
    }
    break;

    case WM_GETMINMAXINFO:
    {
        MINMAXINFO* pmmi = (MINMAXINFO*)lParam;
        pmmi->ptMinTrackSize.x = 400; // Minimum width
        pmmi->ptMinTrackSize.y = 300; // Minimum height

        /*
        pmmi->ptMaxTrackSize.x = 400; // Maximum width
        pmmi->ptMaxTrackSize.y = 300; // Maximum height
*/
    }
        break;

        
    case WM_HOTKEY:
    {
        
		if (GetForegroundWindow() != hWnd) { // disables hotkeys if window is not in foreground
            disable_hotkeys(hWnd);
            break;
        }
		else enable_hotkeys(hWnd);

        if (wParam == HOTKEY_ID_TAB)
        {
            int tabIndex = TabCtrl_GetCurSel(hTab);
            int newTabIndex = tabIndex == 0 ? 1 : 0;
            TabCtrl_SetCurSel(hTab, newTabIndex);
            ShowTabComponents(newTabIndex);
        }

        if (wParam == HOTKEY_ID_SPACE) draw_civ();

        if (wParam == HOTKEY_ID_RETURN) reset();     

        if (wParam == HOTKEY_ID_ESC) kill_application();        

    }
	
    break;


    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // Parse the menu selections:
        
            


        switch (wmId) // action listener for clicks
        {
        case IDM_ABOUT: // "About"
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
		case IDM_EXIT: // "Exit"
            DestroyWindow(hWnd);
            break;
		case IDM_TOGGLE_CHECK: // "Dark Mode (beta)"
            isChecked = !isChecked;
            CheckMenuItem(GetMenu(hWnd), IDM_TOGGLE_CHECK, isChecked ? MF_CHECKED : MF_UNCHECKED);
            InvalidateRect(hWnd, NULL, TRUE); // Force the window to repaint
            EnumChildWindows(hWnd, [](HWND hwnd, LPARAM lParam) -> BOOL {
                InvalidateRect(hwnd, NULL, TRUE);
                return TRUE;
                }, 0);
            break;
		case IDM_GITHUB: // "GitHub"
            ShellExecute(0, 0, L"https://github.com/cnordenb/CPP_Desktop_Application_test/", 0, 0, SW_SHOW);
            break;
        case 1: // "Draw"            
            draw_civ();     
            break;
		case 2: // "Reset"
			reset();
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
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
		kill_application();
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        SetDlgItemText(hDlg, IDC_STATIC_TEXT, L"Chupnis");
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

std::string civ_name(int index)
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

int result(int max) {
    // Create a random device and seed the Mersenne Twister engine
    std::random_device rd;  // Obtain a random number from hardware
    std::mt19937 mt(rd());  // Seed the Mersenne Twister engine
    std::uniform_int_distribution<int> dist(0, max);  // Define the range

    return dist(mt);  // Return a random number between 0 and CIVS_MAX
}

void reset() {

    if (iterator > 0)
    {
        length = GetWindowTextLength(hLogTextField);
        logText.resize(length + 1);
        GetWindowText(hLogTextField, &logText[0], length + 1);
        logText.pop_back(); // Remove the null terminator

        logText = L"\r\n" + logText;
        SetWindowText(hLogTextField, logText.c_str());
    }

    for (int i = 0; i < CIVS_MAX; i++) {
        available[i] = true;
    }
    iterator = 0;
    remaining = CIVS_MAX;

    SetWindowText(hLabel, (L"0/" + std::to_wstring(CIVS_MAX)).c_str());
    SetWindowText(hCenterLabel, L"?");

    printf("\n\nProgram has been reset.\n");

    
}

void draw_civ() {
    if (iterator == CIVS_MAX)
    {
        reset();
        
    }

    
    given_index = result(remaining);

    j = 0;
    for (int i = 0; i < CIVS_MAX; i++) { // the purpose of this for-loop is to check tha the civ is not drawn already


		printf("\niteration %d out of %d\n", i, CIVS_MAX);
        printf("given index: %d\n", given_index);
		bool internal_reset = false;
        
        while (!available[j]) { // at the start, both i and j are 0, separate j has purpose so that a drawn civ is not checked more than once
            if (j == CIVS_MAX) {
                internal_reset = true;
                j = 0;
            }
            printf("drawing civ no. %d...\n", j);
            printf("civ no. %d has already been drawn... Incrementing j from %d to %d.\n", j, j, (j+1));
            j++;
			if (j >= CIVS_MAX && internal_reset == true) {
                internal_reset = false;
				printf("========================================\n");
                printf("\nAll civs have been iterated through.\n");
                printf("========================================\n");
                break;
			}
        }
        if (j >= CIVS_MAX) { // this shouldn't happen. something is wrong
            printf("\n\n=====================\n\n");
            printf("i is now %d\n", i);
            printf("j is now %d\n", j);
            printf("Error: Index j out of bounds\n\n==================== = \n\n\n");
            break;
        }
        if (i == given_index) {
           
            //printf("\nres (i) is now %d\n", given_index);
            //printf("available[%d] is currently %s\n\n", res, available[res] ? "true" : "false");
            given_index = j; // given index updated with increment to skip already drawn civs
            //printf("res (j) is now %d\n", given_index);
            //printf("available[%d] is currently %s\n", res, available[res] ? "true" : "false");
            break; // given drawn civ has not already been drawn
        }

        j++; // j incremented to keep up with i
    }

    civ = civ_name(given_index);

    printf("\n%d. %s (current set: %d/%d)\n",(given_index+1), civ_name(given_index).c_str(), iterator + 1, CIVS_MAX);
	if (given_index >= 0 && given_index < sizeof(available)) available[given_index] = false;
	else printf("\n\n=====================\n\nIndex out of range: %d\n\n=====================\n\n\n", given_index);

    
    iterator++;
    remaining--;



    // Update the labels
    labelText = std::to_wstring(iterator) + L"/" + std::to_wstring(CIVS_MAX);
    SetWindowText(hLabel, labelText.c_str());
    SetWindowTextA(hCenterLabel, civ.c_str());

    length = GetWindowTextLength(hLogTextField);
    logText.resize(length + 1);
    GetWindowText(hLogTextField, &logText[0], length + 1);
    logText.pop_back(); // Remove the null terminator

    newLogEntry = std::wstring(civ.begin(), civ.end()) + L" (" + std::to_wstring(iterator) + L"/" + std::to_wstring(CIVS_MAX) + L")" + L"\r\n";
    logText = newLogEntry + logText;
    if (iterator == CIVS_MAX) logText += L"\r\n";


    SetWindowText(hLogTextField, logText.c_str());

}

void enable_hotkeys(HWND hWnd) {
	RegisterHotKey(hWnd, HOTKEY_ID_TAB, 0, VK_TAB);
	RegisterHotKey(hWnd, HOTKEY_ID_SPACE, 0, VK_SPACE);
	RegisterHotKey(hWnd, HOTKEY_ID_RETURN, 0, VK_RETURN);
	RegisterHotKey(hWnd, HOTKEY_ID_ESC, 0, VK_ESCAPE);
}

void disable_hotkeys(HWND hWnd) {
	UnregisterHotKey(hWnd, HOTKEY_ID_TAB);
	UnregisterHotKey(hWnd, HOTKEY_ID_SPACE);
	UnregisterHotKey(hWnd, HOTKEY_ID_RETURN);
	UnregisterHotKey(hWnd, HOTKEY_ID_ESC);
}

void kill_application() {
    DeleteObject(hBrushWhite);
    DeleteObject(hBrushBlack);
    PostQuitMessage(0);
}