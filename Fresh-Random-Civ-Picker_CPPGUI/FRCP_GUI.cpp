/*
TODO

- implement tooltips
- add tech tree link button to draw tab
    - fetch tech tree icon
- add hotkeys for option toggles
- implement ini file for persistent settings
- implement log file for persistent log and civ states


*/


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
#define HOTKEY_ID_Z 5
#define HOTKEY_ID_X 6
#define HOTKEY_ID_C 7
#define HOTKEY_ID_V 8
#define DT_UNDERLINE 0x80000000
#define MAX_LENGTH 15 
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

HWND label_corner, label_centre, label_drawncount, label_remainingcount;    // labels

HWND button_draw, button_reset, button_enableall, button_disableall, button_clearlog;	    // buttons

HWND drawn_log, remaining_log;				 // textfield for log tab


HWND civ_icon, edition_icon;						// icons

HWND checkbox_showremainlog;


HWND checkbox_armenians, checkbox_aztecs, checkbox_bengalis, checkbox_berbers, checkbox_bohemians,       // checkboxes for custom civ pool tab
checkbox_britons, checkbox_bulgarians, checkbox_burgundians, checkbox_burmese, checkbox_byzantines,
checkbox_celts, checkbox_chinese, checkbox_cumans, checkbox_dravidians, checkbox_ethiopians,
checkbox_franks, checkbox_georgians, checkbox_goths, checkbox_gurjaras, checkbox_huns, checkbox_incas,
checkbox_hindustanis, checkbox_italians, checkbox_japanese, checkbox_khmer, checkbox_koreans,
checkbox_lithuanians, checkbox_magyars, checkbox_malay, checkbox_malians, checkbox_mayans,
checkbox_mongols, checkbox_persians, checkbox_poles, checkbox_portuguese, checkbox_romans,
checkbox_saracens, checkbox_sicilians, checkbox_slavs, checkbox_spanish, checkbox_tatars,
checkbox_teutons, checkbox_turks, checkbox_vietnamese, checkbox_vikings;


HWND civ_checkbox[] = { checkbox_armenians, checkbox_aztecs, checkbox_bengalis, checkbox_berbers, checkbox_bohemians,       // array of checkboxes
                        checkbox_britons, checkbox_bulgarians, checkbox_burgundians, checkbox_burmese, checkbox_byzantines,
                        checkbox_celts, checkbox_chinese, checkbox_cumans, checkbox_dravidians, checkbox_ethiopians,
                        checkbox_franks, checkbox_georgians, checkbox_goths, checkbox_gurjaras, checkbox_huns, checkbox_incas,
                        checkbox_hindustanis, checkbox_italians, checkbox_japanese, checkbox_khmer, checkbox_koreans,
                        checkbox_lithuanians, checkbox_magyars, checkbox_malay, checkbox_malians, checkbox_mayans,
                        checkbox_mongols, checkbox_persians, checkbox_poles, checkbox_portuguese, checkbox_romans,
                        checkbox_saracens, checkbox_sicilians, checkbox_slavs, checkbox_spanish, checkbox_tatars,
                        checkbox_teutons, checkbox_turks, checkbox_vietnamese, checkbox_vikings };

HWND checkbox_khans, checkbox_dukes, checkbox_west, checkbox_india, checkbox_rome, checkbox_royals, // dlc checkboxes
        checkbox_forgotten, checkbox_africans, checkbox_rajas,
        checkbox_aoc;

HWND khans_icon, dukes_icon, west_icon, india_icon, rome_icon, royals_icon,     // dlc icons
    forgotten_icon, africans_icon, rajas_icon,
    aoc_icon;


HWND autoreset_checkbox, autotoggle_checkbox;			 // autoreset checkbox

HWND radiobutton_de, radiobutton_hd, radiobutton_aok;	 // radio buttons

HBRUSH brush_white;
HBRUSH brush_black;

HBITMAP icon_armenians, icon_aztecs, icon_bengalis, icon_berber, icon_bohemians,	                        // civ icons
icon_britons, icon_bulgarians, icon_burgundians, icon_burmese, icon_byzantines,
icon_celts, icon_chinese, icon_cumans, icon_dravidians, icon_ethiopians,
icon_franks, icon_georgians, icon_goths, icon_gurjaras, icon_huns, icon_incas,
icon_hindustanis, icon_italians, icon_japanese, icon_khmer, icon_koreans,
icon_lithuanians, icon_magyars, icon_malay, icon_malians, icon_mayans,
icon_mongols, icon_persians, icon_poles, icon_portuguese, icon_romans,
icon_saracens, icon_sicilians, icon_slavs, icon_spanish, icon_tatars,
icon_teutons, icon_turks, icon_vietnamese, icon_vikings, icon_random;

HBITMAP civ_icon_array[] = { icon_armenians, icon_aztecs, icon_bengalis, icon_berber, icon_bohemians,	                        // civ icons
                    icon_britons, icon_bulgarians, icon_burgundians, icon_burmese, icon_byzantines,
                    icon_celts, icon_chinese, icon_cumans, icon_dravidians, icon_ethiopians,
                    icon_franks, icon_georgians, icon_goths, icon_gurjaras, icon_huns, icon_incas,
                    icon_hindustanis, icon_italians, icon_japanese, icon_khmer, icon_koreans,
                    icon_lithuanians, icon_magyars, icon_malay, icon_malians, icon_mayans,
                    icon_mongols, icon_persians, icon_poles, icon_portuguese, icon_romans,
                    icon_saracens, icon_sicilians, icon_slavs, icon_spanish, icon_tatars,
                    icon_teutons, icon_turks, icon_vietnamese, icon_vikings, icon_random };

HBITMAP icon_de, icon_hd, icon_aok;	                                                            // edition icons

HBITMAP icon_khans, icon_dukes, icon_west, icon_india, icon_rome, icon_royals,                      // dlc icons
            icon_forgotten, icon_africans, icon_rajas,
            icon_aoc;
        

int iterator = 0; // Global variable to keep track of the count
int drawnlog_length = GetWindowTextLength(drawn_log);
int remaininglog_length = GetWindowTextLength(remaining_log);
int current_tab = 0;
int custom_max_civs = MAX_CIVS;
std::wstring label_text = std::to_wstring(iterator + 1) + L"/" + std::to_wstring(MAX_CIVS);
std::wstring log_entry;
std::wstring drawnlog_text, remaininglog_text;
std::wstring hlabel_default;
std::wstring current_civ = L"Random";

std::wstring civ_index[] = { L"Armenians", L"Aztecs", L"Bengalis", L"Berbers", L"Bohemians",       // array of civs for easy indexing
							L"Britons", L"Bulgarians", L"Burgundians", L"Burmese", L"Byzantines",
							L"Celts", L"Chinese", L"Cumans", L"Dravidians", L"Ethiopians",
							L"Franks", L"Georgians", L"Goths", L"Gurjaras", L"Huns", L"Incas",
							L"Hindustanis", L"Italians", L"Japanese", L"Khmer", L"Koreans",
							L"Lithuanians", L"Magyars", L"Malay", L"Malians", L"Mayans",
							L"Mongols", L"Persians", L"Poles", L"Portuguese", L"Romans",
							L"Saracens", L"Sicilians", L"Slavs", L"Spanish", L"Tatars",
							L"Teutons", L"Turks", L"Vietnamese", L"Vikings" };

bool mode_dark = false;
bool icons_enabled = true;
bool jingles_enabled = false;
bool labels_enabled = true;
bool legacy_jingle_enabled = false;
bool ui_sounds_enabled = false;
bool pool_altered = false;
bool autoreset_enabled = true;
bool autotoggle_enabled = true;
bool remainlog_enabled = true;
bool reset_state = true;


enum edition {
	DE,     // Definitive Edition
	HD,     // HD Edition / (2013)
	AOK     // Age of Kings
};

enum dlc {
	aok,   // Age of Kings
	aoc,   // The Conquerors

	forgotten,   // The Forgotten
	africans,   // African Kingdoms
	rajas,   // Rise of Rajas

	khans,   // Last Khans
	west,   // Lords of the West
	dukes,   // Dawn of the Dukes
	india,   // Dynasties of India
	royals,   // The Mountain Royals
    rome    // Return of Rome

};

dlc old_dlc[] = { aok, aoc, forgotten, africans, rajas };

edition edition_state = DE;

bool custom_civ_pool = false;
std::vector<std::pair<std::wstring, bool>> civ_enabled;
std::vector<std::pair<std::wstring, enum edition>> civ_edition;
std::vector<std::pair<std::wstring, enum dlc>> civ_dlc;

bool accessor_out_of_bounds = false;             // for unit testing
int times_drawn[MAX_CIVS] = { 0 };               // for unit testing
HFONT font_underline = NULL;
HWND tab;
std::vector<std::wstring> civs;

// Function declarations
HBITMAP FetchIcon(std::wstring);
void CreateTabs(HWND);
void ShowTabComponents(int, HWND);

void ResetProgram();

void DrawCiv();
void KillApplication();

void EnableHotkeys(HWND);
void DisableHotkeys(HWND);

void CreateUnderlineFont();
void LoadImages();

void PlayJingle(std::wstring);
bool VerifiedLegacyCiv(std::wstring);

void InitialiseCivStates();
void InitialiseCivEditions();
void InitialiseCivDLCs();
void InitialiseCustomPoolCheckboxes(HWND);
void AddCiv(std::wstring);
void RemoveCiv(std::wstring);
void SetCivStatus(const std::wstring, bool);
bool GetCivStatus(const std::wstring);
void ShowAllPoolCheckboxes();
void HideCustomPoolCheckboxes();
void ShowHDPoolCheckboxes();
void ShowAOCPoolCheckboxes();
void DLCToggles(edition);

edition GetCivEdition(const std::wstring&);
dlc GetCivDLC(const std::wstring&);

void EnableAll();
void DisableAll();

void ShowDrawTab(bool, HWND);
void ShowLogTab(bool);
void ShowCustomTab(bool);
void ShowDEDLCCheckboxes(bool);
void ShowHDDLCCheckboxes(bool);
void ShowAOCCheckbox(bool);

void UpdateDrawnLog(bool, bool);
void UpdateRemainingLog();

bool DlcFull(dlc);
void ToggleDlc(dlc, bool, HWND);
void ValidateDlcToggle(HWND, dlc);
void ValidateAllDlcToggles(HWND);

void CreateTooltips(HWND);
void AddTooltip(HWND, HWND, LPCWSTR);

int GetWindowWidth(HWND);
int GetWindowHeight(HWND);


std::string ConvertToString(const std::wstring&);
HWND CreateCheckbox(HWND hWnd, HINSTANCE hInstance, int x, int y, int width, int height, int id, LPCWSTR text);



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

    tie.pszText = const_cast<LPWSTR>(L"Civ Pool");
    TabCtrl_InsertItem(tab, 2, &tie);
}

// Function to show/hide components based on the selected tab
void ShowTabComponents(int tabIndex, HWND hWnd)
{
    if (ui_sounds_enabled) PlaySound(L"tab_sound.wav", NULL, SND_FILENAME | SND_ASYNC);
    if (tabIndex == 0)
    {
        current_tab = 0;
		ShowLogTab(false);
		ShowCustomTab(false);

        ShowDrawTab(true, hWnd);
    }
    else if (tabIndex == 1)
    {
        current_tab = 1;
		ShowDrawTab(false, hWnd);
		ShowCustomTab(false);

        ShowLogTab(true);
    }
    else if (tabIndex == 2)
    {
        current_tab = 2;
		ShowDrawTab(false, hWnd);
		ShowLogTab(false);

		ShowCustomTab(true);

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
	InitialiseCivStates();
    InitialiseCivEditions();
	InitialiseCivDLCs();


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
        CW_USEDEFAULT, 0, 550, 440, nullptr, nullptr, hInstance, nullptr);

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
    static HWND hwndTooltip;

    switch (message)
    {
        case WM_CREATE:
        {
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

            label_drawncount = CreateWindow(
                L"STATIC",  // Predefined class; Unicode assumed
                L"",  // Label text
                WS_VISIBLE | WS_CHILD,  // Styles
                100,  // x position (will be set in WM_SIZE)
                25,  // y position (will be set in WM_SIZE)
                100,  // Label width
                15,  // Label height
                hWnd,  // Parent window
                NULL,  // No menu.
                (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
                NULL);  // Pointer not needed.

            label_remainingcount = CreateWindow(
                L"STATIC",  // Predefined class; Unicode assumed
                L"",  // Label text
                WS_VISIBLE | WS_CHILD,  // Styles
                0,  // x position (will be set in WM_SIZE)
                25,  // y position (will be set in WM_SIZE)
                100,  // Label width
                15,  // Label height
                hWnd,  // Parent window
                NULL,  // No menu.
                (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
                NULL);  // Pointer not needed.

            drawn_log = CreateWindow(
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

            remaining_log = CreateWindow(
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


            edition_icon = CreateWindowW(
                L"STATIC",  // Predefined class; Unicode assumed
                NULL,  // Label text
                WS_VISIBLE | WS_CHILD | SS_BITMAP,  // Styles
                200,  // x position (will be set in WM_SIZE)
                40,  // y position (will be set in WM_SIZE)
                70,  // Label width
                51,  // Label height
                hWnd,  // Parent window
                NULL,
                NULL,
                NULL);  // Pointer not needed.

            button_clearlog = CreateWindow(
                L"BUTTON",  // Predefined class; Unicode assumed 
                L"Clear",      // Button text 
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
				0,         // WM_SIZE will set x position
				0,         // WM_SIZE will set y position
                BUTTON_WIDTH,
                BUTTON_HEIGHT,
                hWnd,       // Parent window
                (HMENU)IDC_BUTTON_CLEARLOG,       
                (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
                NULL);      // Pointer not needed.

            button_enableall = CreateWindow(
                L"BUTTON",  // Predefined class; Unicode assumed 
                L"Enable All",      // Button text 
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
                10,         // x position 
                340,         // y position 
                BUTTON_WIDTH,
                BUTTON_HEIGHT,
                hWnd,       // Parent window
                (HMENU)3,       // No menu.
                (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
                NULL);      // Pointer not needed.

            button_disableall = CreateWindow(
                L"BUTTON",  // Predefined class; Unicode assumed 
                L"Disable All",      // Button text 
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
                125,         // x position 
                340,         // y position 
                BUTTON_WIDTH,
                BUTTON_HEIGHT,
                hWnd,       // Parent window
                (HMENU)4,       // No menu.
                (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
                NULL);      // Pointer not needed.

            radiobutton_de = CreateWindow(
                L"BUTTON",  // Predefined class; Unicode assumed 
                L"Definitive Edition (2019)",      // Button text 
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,  // Styles 
                10,         // x position 
                30,         // y position 
                175,         // Button width
                20,         // Button height
                hWnd,       // Parent window
                (HMENU)IDC_RADIO_DE,       // No menu.
                (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
                NULL);      // Pointer not needed.

            radiobutton_hd = CreateWindow(
                L"BUTTON",  // Predefined class; Unicode assumed 
                L"HD Edition (2013)",      // Button text 
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,  // Styles 
                10,         // x position 
                60,         // y position 
                175,         // Button width
                20,         // Button height
                hWnd,
                (HMENU)IDC_RADIO_HD,
                (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
                NULL);

            radiobutton_aok = CreateWindow(
                L"BUTTON",  // Predefined class; Unicode assumed
                L"Age of Kings (1999)",      // Button text
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,  // Styles
                10,         // x position
                90,         // y position
                175,         // Button width
                20,         // Button height
                hWnd,       // Parent window
                (HMENU)IDC_RADIO_AOK,       // No menu.
                (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
                NULL);      // Pointer not needed.


			checkbox_showremainlog = CreateCheckbox(hWnd, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), 10, 230, 180, 20, IDC_CHECKBOX_REMAINLOG, L"Show");
            CheckDlgButton(hWnd, IDC_CHECKBOX_REMAINLOG, remainlog_enabled ? BST_CHECKED : BST_UNCHECKED);

 
            CheckRadioButton(hWnd, IDC_RADIO_DE, IDC_RADIO_AOK, IDC_RADIO_DE);

            // defined spot coordinates for the 45 individual civ checkboxes in custom tab
			int row[] = { 30, 50, 70, 90, 110, 130, 150, 170, 190 };
			int column[] = { 10, 112, 214, 316, 418 };

            // creating individual civ checkboxes for custom tab
			for (int i = 0; i < MAX_CIVS; i++)
			{
				civ_checkbox[i] = CreateCheckbox(hWnd, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), column[i % 5], row[i / 5], 100, 20, i + 5, civ_index[i].c_str());
			}

            autoreset_checkbox = CreateCheckbox(hWnd, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), 310, 230, 180, 20, IDC_CHECKBOX_AUTORESET, L"Auto-reset upon change");
            autotoggle_checkbox = CreateCheckbox(hWnd, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), 10, 0, 180, 20, IDC_CHECKBOX_AUTOTOGGLE, L"Auto-toggle older civs");

            checkbox_royals = CreateCheckbox(hWnd, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), 368, -96, 160, 20, IDC_CHECKBOX_ROYALS, L"The Mountain Royals");
            checkbox_rome = CreateCheckbox(hWnd, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), 368, -76, 160, 20, IDC_CHECKBOX_ROME, L"Return of Rome");
            checkbox_india = CreateCheckbox(hWnd, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), 368, -56, 160, 20, IDC_CHECKBOX_INDIA, L"Dynasties of India");
            checkbox_dukes = CreateCheckbox(hWnd, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), 368, -36, 160, 20, IDC_CHECKBOX_DUKES, L"Dawn of the Dukes");
            checkbox_west = CreateCheckbox(hWnd, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), 368, -16, 160, 20, IDC_CHECKBOX_WEST, L"Lords of the West");
            checkbox_khans = CreateCheckbox(hWnd, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), 368, 4, 160, 20, IDC_CHECKBOX_KHANS, L"The Last Khans");

            checkbox_rajas = CreateCheckbox(hWnd, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), 383, -84, 145, 20, IDC_CHECKBOX_RAJAS, L"Rise of Rajas");
            checkbox_africans = CreateCheckbox(hWnd, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), 383, -51, 145, 20, IDC_CHECKBOX_AFRICANS, L"African Kingdoms");
            checkbox_forgotten = CreateCheckbox(hWnd, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), 383, -16, 145, 20, IDC_CHECKBOX_FORGOTTEN, L"The Forgotten");

            checkbox_aoc = CreateCheckbox(hWnd, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), 388, -56, 140, 20, IDC_CHECKBOX_AOC, L"The Conquerors");

            royals_icon = CreateWindow(L"STATIC", NULL, WS_VISIBLE | WS_CHILD | SS_BITMAP, 338, 0, 0, 0, hWnd, NULL, NULL, NULL);
            rome_icon = CreateWindow(L"STATIC", NULL, WS_VISIBLE | WS_CHILD | SS_BITMAP, 338, 0, 0, 0, hWnd, NULL, NULL, NULL);
            india_icon = CreateWindow(L"STATIC", NULL, WS_VISIBLE | WS_CHILD | SS_BITMAP, 338, 0, 0, 0, hWnd, NULL, NULL, NULL);
            dukes_icon = CreateWindow(L"STATIC", NULL, WS_VISIBLE | WS_CHILD | SS_BITMAP, 338, 0, 0, 0, hWnd, NULL, NULL, NULL);
            west_icon = CreateWindow(L"STATIC", NULL, WS_VISIBLE | WS_CHILD | SS_BITMAP, 338, 0, 0, 0, hWnd, NULL, NULL, NULL);
            khans_icon = CreateWindow(L"STATIC", NULL, WS_VISIBLE | WS_CHILD | SS_BITMAP, 338, 0, 0, 0, hWnd, NULL, NULL, NULL);

            rajas_icon = CreateWindow(L"STATIC", NULL, WS_VISIBLE | WS_CHILD | SS_BITMAP, 338, 0, 0, 0, hWnd, NULL, NULL, NULL);
            africans_icon = CreateWindow(L"STATIC", NULL, WS_VISIBLE | WS_CHILD | SS_BITMAP, 338, 0, 0, 0, hWnd, NULL, NULL, NULL);
            forgotten_icon = CreateWindow(L"STATIC", NULL, WS_VISIBLE | WS_CHILD | SS_BITMAP, 338, 0, 0, 0, hWnd, NULL, NULL, NULL);

            aoc_icon = CreateWindow(L"STATIC", NULL, WS_VISIBLE | WS_CHILD | SS_BITMAP, 338, 0, 0, 0, hWnd, NULL, NULL, NULL);


            SendMessage(royals_icon, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)icon_royals);
            SendMessage(rome_icon, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)icon_rome);
            SendMessage(india_icon, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)icon_india);
            SendMessage(dukes_icon, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)icon_dukes);
            SendMessage(west_icon, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)icon_west);
            SendMessage(khans_icon, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)icon_khans);

            SendMessage(rajas_icon, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)icon_rajas);
            SendMessage(africans_icon, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)icon_africans);
            SendMessage(forgotten_icon, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)icon_forgotten);

            SendMessage(aoc_icon, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)icon_aoc);


            SetWindowPos(royals_icon, NULL, 345, 25, 18, 18, SWP_NOZORDER);
            SetWindowPos(rome_icon, NULL, 345, 45, 18, 18, SWP_NOZORDER);
            SetWindowPos(india_icon, NULL, 345, 65, 18, 18, SWP_NOZORDER);
            SetWindowPos(dukes_icon, NULL, 345, 85, 18, 18, SWP_NOZORDER);
            SetWindowPos(west_icon, NULL, 345, 105, 18, 18, SWP_NOZORDER);
            SetWindowPos(khans_icon, NULL, 345, 125, 18, 18, SWP_NOZORDER);

            SetWindowPos(rajas_icon, NULL, 350, 30, 30, 30, SWP_NOZORDER);
            SetWindowPos(africans_icon, NULL, 350, 65, 30, 30, SWP_NOZORDER);
            SetWindowPos(forgotten_icon, NULL, 350, 100, 30, 30, SWP_NOZORDER);

            SetWindowPos(aoc_icon, NULL, 340, 50, 45, 45, SWP_NOZORDER);


            SendMessageW(edition_icon, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)icon_de);
            HideCustomPoolCheckboxes();
            ShowWindow(civ_icon, SW_HIDE);
            SetWindowPos(edition_icon, NULL, 190, 30, 128, 93, SWP_NOZORDER);
            

			
            ShowLogTab(false);
            ShowCustomTab(false);
            ShowDrawTab(true, hWnd);






            ResetProgram();    // resetter is called in order to enable remaining civ indicator label (hLabel)

            brush_black = CreateSolidBrush(RGB(0, 0, 0));
            brush_white = CreateSolidBrush(RGB(255, 255, 255));

            CreateTooltips(hWnd);

            EnableHotkeys(hWnd);

            break;
        }
        case WM_SIZE:
        {
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);

            SetWindowPos(button_draw, NULL, (width - 100) / 2, (height + 100) / 2, 100, 30, SWP_NOZORDER);   // draw button anchored to centre


            if (current_tab == 0) {
                SetWindowPos(button_reset, NULL, 10, height - 40, 100, 30, SWP_NOZORDER);
            }
            else {
                SetWindowPos(button_reset, NULL, (width - 100) / 2, height - 40, 100, 30, SWP_NOZORDER);
            }


            SetWindowPos(tab, NULL, 0, 0, width, height, SWP_NOZORDER);                                        // tab size anchored to window size
            SetWindowPos(label_corner, NULL, width - 50, height - 20, 40, 15, SWP_NOZORDER);                          // remaining civ indicator anchored to bottom right corner 
            SetWindowPos(label_centre, NULL, (width - 80) / 2, (height + 35) / 2, 100, 15, SWP_NOZORDER);       // drawn civ label anchored to centre
            SetWindowPos(civ_icon, NULL, (width - 100) / 2, (height - 180) / 2, 104, 104, SWP_NOZORDER);       // civ icon anchored to centre
                          // reset button anchored to bottom left corner
			
            SetWindowPos(label_drawncount, NULL, 10, 25, 90, 15, SWP_NOZORDER);                           // drawn civ label anchored to top left corner
			SetWindowPos(button_clearlog, NULL, 110, 25, 100, 30, SWP_NOZORDER);                           // clear log button anchored to top left corner
            SetWindowPos(drawn_log, NULL, 10, 60, width - (width / 2) - 60, height - 25, SWP_NOZORDER);                   // log text field anchored to window size
			SetWindowPos(label_remainingcount, NULL, (width / 2) + 60, 25, 130, 15, SWP_NOZORDER);                           // drawn civ label anchored to top right corner
			SetWindowPos(checkbox_showremainlog, NULL, (width / 2) + 190, 25, 60, 15, SWP_NOZORDER);                           // drawn civ label anchored to top right corner
            SetWindowPos(remaining_log, NULL, (width / 2) + 60, 60, width - (width / 2) - 60, height - 25, SWP_NOZORDER);                   // log text field anchored to window size

            break;
        }
        case WM_ACTIVATE: // re-enables hotkeys when window returns to foreground 
        {
            if (wParam == WA_ACTIVE || wParam == WA_CLICKACTIVE) EnableHotkeys(hWnd);
            else if (wParam == WA_INACTIVE) DisableHotkeys(hWnd);

            break;
        }
        case WM_CTLCOLORSTATIC:
        {
            HDC hdcStatic = (HDC)wParam;
            SetTextColor(hdcStatic, mode_dark ? RGB(255, 255, 255) : RGB(0, 0, 0));
            SetBkColor(hdcStatic, mode_dark ? RGB(0, 0, 0) : RGB(255, 255, 255));
            return (INT_PTR)(mode_dark ? brush_black : brush_white);
        }
        case WM_ERASEBKGND:
        {
            HDC hdc = (HDC)wParam;
            RECT rect;
            GetClientRect(hWnd, &rect);
            FillRect(hdc, &rect, mode_dark ? brush_black : brush_white);
            return 1;
        }  
        case WM_CTLCOLORBTN:
        {
            HDC hdcButton = (HDC)wParam;
            SetTextColor(hdcButton, mode_dark ? RGB(255, 255, 255) : RGB(0, 0, 0));
            SetBkColor(hdcButton, mode_dark ? RGB(0, 0, 0) : RGB(255, 255, 255));
            return (INT_PTR)(mode_dark ? brush_black : brush_white);
		}   
        case WM_NOTIFY:
        {


            LPNMHDR pnmhdr = (LPNMHDR)lParam;
            if (pnmhdr->hwndFrom == tab)
            {
                if (pnmhdr->code == TCN_SELCHANGE) {
                    int tabIndex = TabCtrl_GetCurSel(tab);
                    ShowTabComponents(tabIndex, hWnd);
                    break;
                }
            }
            else if (pnmhdr->code == TTN_SHOW)
            {
                // Get the tooltip info
                LPNMTTDISPINFO lpttd = (LPNMTTDISPINFO)lParam;

                // Get the cursor position
                POINT pt;
                GetCursorPos(&pt);

                // Set the tooltip position
                SendMessage(pnmhdr->hwndFrom, TTM_TRACKPOSITION, 0, (LPARAM)MAKELONG(pt.x + 10, pt.y + 10));
                return TRUE;
            }
            break;
        }

        case WM_MOUSEMOVE:
        {
            if (hwndTooltip)
            {
                // Get the cursor position
                POINT pt;
                GetCursorPos(&pt);

                // Convert screen coordinates to client coordinates
                ScreenToClient(hWnd, &pt);

                // Check if the cursor is over the button
                RECT rect;
                if (GetWindowRect(button_draw, &rect))
                {
                    ScreenToClient(hWnd, (LPPOINT)&rect.left);
                    ScreenToClient(hWnd, (LPPOINT)&rect.right);

                    if (PtInRect(&rect, pt))
                    {
                        // Set the tooltip position
                        ClientToScreen(hWnd, &pt);
                        SendMessage(hwndTooltip, TTM_TRACKPOSITION, 0, (LPARAM)MAKELONG(pt.x + 10, pt.y + 10));

                        // Activate the tooltip
                        TOOLINFO toolInfo = { 0 };
                        toolInfo.cbSize = sizeof(toolInfo);
                        toolInfo.hwnd = hWnd;
                        toolInfo.uId = (UINT_PTR)button_draw;
                        SendMessage(hwndTooltip, TTM_TRACKACTIVATE, TRUE, (LPARAM)&toolInfo);
                    }
                    else
                    {
                        // Deactivate the tooltip
                        TOOLINFO toolInfo = { 0 };
                        toolInfo.cbSize = sizeof(toolInfo);
                        toolInfo.hwnd = hWnd;
                        toolInfo.uId = (UINT_PTR)button_draw;
                        SendMessage(hwndTooltip, TTM_TRACKACTIVATE, FALSE, (LPARAM)&toolInfo);
                    }
                }
            }
            break;
        }

        case WM_GETMINMAXINFO:                                  // minimum window size
        {
            MINMAXINFO *pmmi = (MINMAXINFO *)lParam;
            pmmi->ptMinTrackSize.x = 550; // Minimum width
            pmmi->ptMinTrackSize.y = 440; // Minimum height


            break;

        }
        case WM_HOTKEY:
        {

            

            if (GetForegroundWindow() != hWnd)  // disables hotkeys if window is not in foreground
            {
                DisableHotkeys(hWnd);
                break;
            }
            else EnableHotkeys(hWnd);

            if (wParam == HOTKEY_ID_ESC) {
                if (ui_sounds_enabled) {
                    PlaySound(L"exit.wav", NULL, SND_FILENAME | SND_ASYNC);
                    Sleep(200);
                }

                KillApplication();                // escape for exiting
            }

            

            if (current_tab != 2) {
                if (wParam == HOTKEY_ID_SPACE) DrawCiv();           // space for drawing civ
                if (wParam == HOTKEY_ID_RETURN) ResetProgram();            // return for resetting
            }

            if (wParam == HOTKEY_ID_TAB)                        // tab for switching tabs
            {
                int tabIndex = TabCtrl_GetCurSel(tab);
                int newTabIndex;
                if (tabIndex < 2) newTabIndex = tabIndex + 1;
                else newTabIndex = 0;
                TabCtrl_SetCurSel(tab, newTabIndex);
                ShowTabComponents(newTabIndex, hWnd);
            }




            if (wParam == HOTKEY_ID_Z) {
                if (labels_enabled) {
					labels_enabled = false;
                    ShowWindow(label_centre, SW_HIDE);
				}
                else {
					labels_enabled = true;
					if (current_tab == 0) ShowWindow(label_centre, SW_SHOW);
                }
            }

			if (wParam == HOTKEY_ID_X) {
				if (icons_enabled) {
					icons_enabled = false;
					ShowWindow(civ_icon, SW_HIDE);
				}
				else {
					icons_enabled = true;
					if (current_tab == 0) ShowWindow(civ_icon, SW_SHOW);
				}
			}

            if (wParam == HOTKEY_ID_C) {
                if (jingles_enabled) {
                    jingles_enabled = false;
                }
                else {
                    jingles_enabled = true;
                    PlayJingle(current_civ);
                }
            }

			if (wParam == HOTKEY_ID_V) {
				if (ui_sounds_enabled) {
					ui_sounds_enabled = false;
				}
				else {
					ui_sounds_enabled = true;
				}
			}


            
                
            break;
        }

        
        

        case WM_COMMAND:                                        // action listener for clicks
        {
            int wmId = LOWORD(wParam);





            // individual civ checkbox civ handlers in custom tab
            if (wmId >= 5 && wmId <= 49) {

                if (IsDlgButtonChecked(hWnd, wmId) == BST_CHECKED) AddCiv(civ_index[wmId-5]);
                else RemoveCiv(civ_index[wmId - 5]);
                ValidateDlcToggle(hWnd, GetCivDLC(civ_index[wmId - 5]));
				

            }         


            // pool altered state
            if (wmId > 4 && wmId < 50 || wmId > 53 && wmId < 64 || autotoggle_enabled && wmId > 50 && wmId < 54)
            {
                pool_altered = true;
            }

			// button sounds in tab views
            if (ui_sounds_enabled) {
                if (wmId > 2 && wmId < 66) {
					if (wmId > 50 && wmId < 54) PlaySound(L"view_sound.wav", NULL, SND_FILENAME | SND_ASYNC);
					else PlaySound(L"button_sound.wav", NULL, SND_FILENAME | SND_ASYNC);
                }
            }

            // auto-reset
            if (autoreset_enabled) {
				if (wmId > 4 && wmId < 50 || wmId > 50 && wmId < 64) ResetProgram();
            }

            switch (wmId)
            {
            case IDM_ABOUT:                                     // "About"
                if (ui_sounds_enabled) PlaySound(L"button_sound.wav", NULL, SND_FILENAME | SND_ASYNC);
                DialogBox(instance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, AboutDlgProc);
                break;
            case IDM_EXIT:                                      // "Exit"
                if (ui_sounds_enabled) PlaySound(L"exit.wav", NULL, SND_FILENAME | SND_ASYNC);
                Sleep(200);
                DestroyWindow(hWnd);
                break;
            case IDM_OPTIONS:								   // "Options"
                if (ui_sounds_enabled) PlaySound(L"button_sound.wav", NULL, SND_FILENAME | SND_ASYNC);
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
                if (ui_sounds_enabled) PlaySound(L"button_sound.wav", NULL, SND_FILENAME | SND_ASYNC);
                ShellExecute(0, 0, L"https://github.com/cnordenb/Fresh-Random-Civ-Picker_CPPGUI", 0, 0, SW_SHOW);
                break;
            case IDM_WEBVERSION:                                // "Web Version"
                if (ui_sounds_enabled) PlaySound(L"button_sound.wav", NULL, SND_FILENAME | SND_ASYNC);
                ShellExecute(0, 0, L"https://cnordenb.github.io/Fresh-Random-Civ-Picker_web/", 0, 0, SW_SHOW);
                break;
            case 1:                                             // "Draw"     
                if (ui_sounds_enabled && !jingles_enabled) PlaySound(L"button_sound.wav", NULL, SND_FILENAME | SND_ASYNC);
                DrawCiv();
                break;
            case 2:                                             // "Reset"
                if (ui_sounds_enabled && !jingles_enabled) PlaySound(L"button_sound.wav", NULL, SND_FILENAME | SND_ASYNC);
                ResetProgram();
                break;
            case 3:                                             // "Enable All"
                if (ui_sounds_enabled) PlaySound(L"button_sound.wav", NULL, SND_FILENAME | SND_ASYNC);
                EnableAll();
                ValidateAllDlcToggles(hWnd);
                break;
            case 4:                                             // "Disable All"
                if (ui_sounds_enabled) PlaySound(L"button_sound.wav", NULL, SND_FILENAME | SND_ASYNC);
                DisableAll();
                ValidateAllDlcToggles(hWnd);
                break;
			case IDC_BUTTON_CLEARLOG:                           // "Clear"
				SetWindowText(drawn_log, L"");
                break;
            case IDC_CHECKBOX_REMAINLOG:                        // "Show Remaining Civs Log"
                if (!remainlog_enabled) {
                    remainlog_enabled = true;
                    ShowWindow(remaining_log, SW_SHOW);
                }
                else {
					remainlog_enabled = false;
					ShowWindow(remaining_log, SW_HIDE);
                }
                break;
            case IDC_CHECKBOX_AUTORESET:										                            // Auto-reset Checkbox
                if (IsDlgButtonChecked(hWnd, IDC_CHECKBOX_AUTORESET) == BST_CHECKED) autoreset_enabled = true;
                else if (IsDlgButtonChecked(hWnd, IDC_CHECKBOX_AUTORESET) == BST_UNCHECKED) autoreset_enabled = false;
                break;
            case IDC_CHECKBOX_AUTOTOGGLE:											                            // Auto-reset Checkbox
                
                if (IsDlgButtonChecked(hWnd, IDC_CHECKBOX_AUTOTOGGLE) == BST_CHECKED) {
                    autotoggle_enabled = true;
                    switch (edition_state) {
                    case DE:
                        for (int i = 0; i < 5; i++) {
                            ToggleDlc(old_dlc[i], true, hWnd);
                        }
                        break;
                    case HD:
                        for (int i = 0; i < 2; i++) {
                            ToggleDlc(old_dlc[i], true, hWnd);
						}
						break;
                    case AOK:
                        ToggleDlc(aok, true, hWnd);
                        break;
                    }
                }               
                else if (IsDlgButtonChecked(hWnd, IDC_CHECKBOX_AUTOTOGGLE) == BST_UNCHECKED) {
                    autotoggle_enabled = false;
                    switch (edition_state) {
                    case DE:
                        for (int i = 0; i < 5; i++) {
                            ToggleDlc(old_dlc[i], false, hWnd);
                        }
                        break;
                    case HD:
                        for (int i = 0; i < 2; i++) {
                            ToggleDlc(old_dlc[i], false, hWnd);
                        }
                        break;
                    case AOK:
				        ToggleDlc(aok, false, hWnd);
				        break;
                    }
			        
                }
                    
                    
                break;
            case IDC_RADIO_DE:
                ShowDEDLCCheckboxes(true);
                ShowHDDLCCheckboxes(false);
                ShowAOCCheckbox(false);

                


                SendMessageW(edition_icon, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)icon_de);
                
                edition_state = DE;
                ShowAllPoolCheckboxes();                
				ValidateAllDlcToggles(hWnd);

                if (autotoggle_enabled) {
                    for (int i = 0; i < 5; i++) {
                        ToggleDlc(old_dlc[i], true, hWnd);
                    }
                }


                break;
            case IDC_RADIO_HD:
                ShowDEDLCCheckboxes(false);
                ShowHDDLCCheckboxes(true);
                ShowAOCCheckbox(false);

                if (autotoggle_enabled) {
                    for (int i = 0; i < 2; i++) {
                        ToggleDlc(old_dlc[i], true, hWnd);
                    }
                }
                

                SendMessageW(edition_icon, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)icon_hd);
                if (ui_sounds_enabled) PlaySound(L"view_sound.wav", NULL, SND_FILENAME | SND_ASYNC);
                edition_state = HD;
                ShowHDPoolCheckboxes();
                ValidateAllDlcToggles(hWnd);



                break;
            case IDC_RADIO_AOK:
                ShowDEDLCCheckboxes(false);
                ShowHDDLCCheckboxes(false);
                ShowAOCCheckbox(true);

                if (autotoggle_enabled) {
                    ToggleDlc(aok, true, hWnd);
                }
                

                SendMessageW(edition_icon, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)icon_aok);
                if (ui_sounds_enabled) PlaySound(L"view_sound.wav", NULL, SND_FILENAME | SND_ASYNC);
                edition_state = AOK;
                ShowAOCPoolCheckboxes();
                ValidateAllDlcToggles(hWnd);



                break;
            case IDC_CHECKBOX_ROYALS:
                
                if (IsDlgButtonChecked(hWnd, IDC_CHECKBOX_ROYALS) == BST_CHECKED) {
                    ToggleDlc(royals, true, hWnd);
                }
                else if (IsDlgButtonChecked(hWnd, IDC_CHECKBOX_ROYALS) == BST_UNCHECKED) {
                    ToggleDlc(royals, false, hWnd);
                }
                break;
            case IDC_CHECKBOX_ROME:
				if (IsDlgButtonChecked(hWnd, IDC_CHECKBOX_ROME) == BST_CHECKED) {
					ToggleDlc(rome, true, hWnd);
				}
				else if (IsDlgButtonChecked(hWnd, IDC_CHECKBOX_ROME) == BST_UNCHECKED) {
					ToggleDlc(rome, false, hWnd);
				}
                break;
            case IDC_CHECKBOX_INDIA:
				if (IsDlgButtonChecked(hWnd, IDC_CHECKBOX_INDIA) == BST_CHECKED) {
					ToggleDlc(india, true, hWnd);
				}
				else if (IsDlgButtonChecked(hWnd, IDC_CHECKBOX_INDIA) == BST_UNCHECKED) {
					ToggleDlc(india, false, hWnd);
				}
                break;
            case IDC_CHECKBOX_DUKES:
				if (IsDlgButtonChecked(hWnd, IDC_CHECKBOX_DUKES) == BST_CHECKED) {
					ToggleDlc(dukes, true, hWnd);
				}
				else if (IsDlgButtonChecked(hWnd, IDC_CHECKBOX_DUKES) == BST_UNCHECKED) {
					ToggleDlc(dukes, false, hWnd);
				}
                break;
            case IDC_CHECKBOX_WEST:
				if (IsDlgButtonChecked(hWnd, IDC_CHECKBOX_WEST) == BST_CHECKED) {
					ToggleDlc(west, true, hWnd);
				}
				else if (IsDlgButtonChecked(hWnd, IDC_CHECKBOX_WEST) == BST_UNCHECKED) {
					ToggleDlc(west, false, hWnd);
				}
                break;
            case IDC_CHECKBOX_KHANS:
				if (IsDlgButtonChecked(hWnd, IDC_CHECKBOX_KHANS) == BST_CHECKED) {
					ToggleDlc(khans, true, hWnd);
				}
				else if (IsDlgButtonChecked(hWnd, IDC_CHECKBOX_KHANS) == BST_UNCHECKED) {
					ToggleDlc(khans, false, hWnd);
				}
                break;
            case IDC_CHECKBOX_RAJAS:
				if (IsDlgButtonChecked(hWnd, IDC_CHECKBOX_RAJAS) == BST_CHECKED) {
					ToggleDlc(rajas, true, hWnd);
				}
				else if (IsDlgButtonChecked(hWnd, IDC_CHECKBOX_RAJAS) == BST_UNCHECKED) {
					ToggleDlc(rajas, false, hWnd);
				}
                break;
            case IDC_CHECKBOX_AFRICANS:
				if (IsDlgButtonChecked(hWnd, IDC_CHECKBOX_AFRICANS) == BST_CHECKED) {
					ToggleDlc(africans, true, hWnd);
				}
				else if (IsDlgButtonChecked(hWnd, IDC_CHECKBOX_AFRICANS) == BST_UNCHECKED) {
					ToggleDlc(africans, false, hWnd);
				}
                break;
            case IDC_CHECKBOX_FORGOTTEN:
				if (IsDlgButtonChecked(hWnd, IDC_CHECKBOX_FORGOTTEN) == BST_CHECKED) {
					ToggleDlc(forgotten, true, hWnd);
				}
				else if (IsDlgButtonChecked(hWnd, IDC_CHECKBOX_FORGOTTEN) == BST_UNCHECKED) {
					ToggleDlc(forgotten, false, hWnd);
				}
                break;
            case IDC_CHECKBOX_AOC:
				if (IsDlgButtonChecked(hWnd, IDC_CHECKBOX_AOC) == BST_CHECKED) {
					ToggleDlc(aoc, true, hWnd);
				}
				else if (IsDlgButtonChecked(hWnd, IDC_CHECKBOX_AOC) == BST_UNCHECKED) {
					ToggleDlc(aoc, false, hWnd);
				}
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
            break;
            
        }
        case WM_PAINT:
        {
        
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            EndPaint(hWnd, &ps);
        
            break;
        }
        case WM_DESTROY:
        {
            KillApplication();
            break;
        }
        default:
        {
            return DefWindowProc(hWnd, message, wParam, lParam);
        }            
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
        CheckDlgButton(hDlg, IDC_CHECKBOX_LABELS, labels_enabled ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hDlg, IDC_CHECKBOX_ICONS, icons_enabled ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHECKBOX_JINGLES, jingles_enabled ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hDlg, IDC_CHECKBOX_SOUNDS, ui_sounds_enabled ? BST_CHECKED : BST_UNCHECKED);

        HWND hComboBox = GetDlgItem(hDlg, IDC_LEGACY_OPTION);
        SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM)L"Definitive Edition");
        SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM)L"Legacy");
        SendMessage(hComboBox, CB_SETCURSEL, legacy_jingle_enabled ? 1 : 0, 0); // Set default selection based on current setting


        return(INT_PTR)TRUE;
    }
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        int wmEvent = HIWORD(wParam);


        if (ui_sounds_enabled && IDC_LEGACY_OPTION) {
            switch (wmEvent)
            {
            case CBN_DROPDOWN:
                // Handle dropdown open event
                PlaySound(L"hover_sound.wav", NULL, SND_FILENAME | SND_ASYNC);
                break;

            case CBN_SELCHANGE:
                // Handle selection change event
                PlaySound(L"view_sound.wav", NULL, SND_FILENAME | SND_ASYNC);
                break;
            }
        }

		ui_sounds_enabled = IsDlgButtonChecked(hDlg, IDC_CHECKBOX_SOUNDS) == BST_CHECKED;
		labels_enabled = IsDlgButtonChecked(hDlg, IDC_CHECKBOX_LABELS) == BST_CHECKED;
		icons_enabled = IsDlgButtonChecked(hDlg, IDC_CHECKBOX_ICONS) == BST_CHECKED;
		jingles_enabled = IsDlgButtonChecked(hDlg, IDC_CHECKBOX_JINGLES) == BST_CHECKED;

        // Handle the combobox selection
        HWND hComboBox = GetDlgItem(hDlg, IDC_LEGACY_OPTION);
        int selectedIndex = SendMessage(hComboBox, CB_GETCURSEL, 0, 0);
        legacy_jingle_enabled = (selectedIndex == 1);


        // Handle the checkbox state as needed
        if (icons_enabled && current_tab == 0)
        {
            ShowWindow(civ_icon, SW_SHOW);
        }
        else
        {
            ShowWindow(civ_icon, SW_HIDE);
        }



        if (wmEvent == BN_CLICKED)
        {
            switch (wmId)
            {
            case IDC_CHECKBOX_LABELS:
            case IDC_CHECKBOX_ICONS:
            case IDC_CHECKBOX_JINGLES:
            case IDC_CHECKBOX_SOUNDS:
                if (ui_sounds_enabled)
                {
                    PlaySound(L"button_sound.wav", NULL, SND_FILENAME | SND_ASYNC);
                }
                break;
            }
        }


		if (!labels_enabled) ShowWindow(label_centre, SW_HIDE);
		else if (current_tab == 0) ShowWindow(label_centre, SW_SHOW);

            
			


  
        
        if (wmId == IDCANCEL || wmId == IDOK) {
			if (jingles_enabled) PlayJingle(current_civ);
            EndDialog(hDlg, wmId);
            return(INT_PTR)TRUE;
        }
         
   

        
        break;

	}
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
    if (custom_max_civs < MAX_CIVS) custom_civ_pool = true;

    current_civ = L"Random";
    iterator = 0;

	UpdateDrawnLog(false, true);
   

    if (custom_civ_pool) {
        civs.clear();
        custom_max_civs = 0;
        for (int i = 0; i < MAX_CIVS; i++) {
            if (GetCivStatus(civ_index[i])) {
                civs.push_back(civ_index[i]);
                custom_max_civs++;
            }
        }

    }


    else {
        civs = { L"Armenians", L"Aztecs", L"Bengalis", L"Berbers", L"Bohemians", L"Britons",
                L"Bulgarians", L"Burgundians", L"Burmese", L"Byzantines", L"Celts", L"Chinese", 
                L"Cumans", L"Dravidians", L"Ethiopians", L"Franks", L"Georgians", L"Goths", L"Gurjaras",
                L"Hindustanis", L"Huns", L"Incas", L"Italians", L"Japanese", L"Khmer", L"Koreans", 
                L"Lithuanians", L"Magyars", L"Malay", L"Malians", L"Mayans", L"Mongols", L"Persians",
                L"Poles", L"Portuguese", L"Romans", L"Saracens", L"Sicilians", L"Slavs", L"Spanish", 
                L"Tatars", L"Teutons", L"Turks", L"Vietnamese", L"Vikings" };
    }
	
    

    SetWindowText(label_corner, (L"0/" + std::to_wstring(custom_max_civs)).c_str());     // resets remaining civs label
    SetWindowText(label_centre, L"?");                                      // resets drawn civ label
    SendMessageW(civ_icon, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)icon_random);
	if (jingles_enabled && !pool_altered) PlayJingle(current_civ);
    pool_altered = false;
    reset_state = true;

	UpdateRemainingLog();


}

void DrawCiv()
{
    reset_state = false;
    
    if (custom_max_civs == 0) {
        if (ui_sounds_enabled) PlaySound(L"error_sound.wav", NULL, SND_FILENAME | SND_ASYNC);
		SetWindowTextA(label_centre, "Empty pool!");
        return;        // if no civs are selected, return
    }

    if (civs.empty() || iterator == custom_max_civs || iterator == 0) ResetProgram();        // if all civs have been drawn, reset program and civ vector



    std::random_device rd;      // seeding random number
	std::mt19937 mt(rd());      // with Mersenne Twister 

	std::shuffle(civs.begin(), civs.end(), mt); // shuffling civs vector
	current_civ = civs.back();		// draws last element
	civs.pop_back();							// removes last element from pool

    std::string civ_name_str = ConvertToString(current_civ);

    iterator++;

    // Update the labels
    label_text = std::to_wstring(iterator) + L"/" + std::to_wstring(custom_max_civs);
    SetWindowText(label_corner, label_text.c_str());
    SetWindowTextA(label_centre, civ_name_str.c_str());


    HBITMAP drawn_civ_icon = FetchIcon(current_civ);
    SendMessageW(civ_icon, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)drawn_civ_icon);

   

    UpdateDrawnLog(true, false);
    UpdateRemainingLog();



    if (current_tab != 0) {
        ShowWindow(civ_icon, SW_HIDE);
        ShowWindow(label_centre, SW_HIDE);
    }
        

    if (jingles_enabled) PlayJingle(current_civ);
    


    
}

void EnableHotkeys(HWND hWnd)
{
	RegisterHotKey(hWnd, HOTKEY_ID_TAB, 0, VK_TAB);
	RegisterHotKey(hWnd, HOTKEY_ID_SPACE, 0, VK_SPACE);
	RegisterHotKey(hWnd, HOTKEY_ID_RETURN, 0, VK_RETURN);
	RegisterHotKey(hWnd, HOTKEY_ID_ESC, 0, VK_ESCAPE);
    RegisterHotKey(hWnd, HOTKEY_ID_Z, 0, 0x5A);
	RegisterHotKey(hWnd, HOTKEY_ID_X, 0, 0x58);
	RegisterHotKey(hWnd, HOTKEY_ID_C, 0, 0x43);
	RegisterHotKey(hWnd, HOTKEY_ID_V, 0, 0x56);


}

void DisableHotkeys(HWND hWnd)
{
	UnregisterHotKey(hWnd, HOTKEY_ID_TAB);
	UnregisterHotKey(hWnd, HOTKEY_ID_SPACE);
	UnregisterHotKey(hWnd, HOTKEY_ID_RETURN);
	UnregisterHotKey(hWnd, HOTKEY_ID_ESC);
    UnregisterHotKey(hWnd, HOTKEY_ID_Z);
	UnregisterHotKey(hWnd, HOTKEY_ID_X);
	UnregisterHotKey(hWnd, HOTKEY_ID_C);
	UnregisterHotKey(hWnd, HOTKEY_ID_V);

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


    /*                                  // can't get this to work yet, some icons disappear
    std::wstring bmp_parsed_civname;
    std::wstring icon_path;


    for (int i = 0; i < MAX_CIVS; i++) {
        bmp_parsed_civname = civ_index[i];
        bmp_parsed_civname[0] = std::tolower(bmp_parsed_civname[0]);

        icon_path = L"civ_icons\\" + bmp_parsed_civname + L".bmp";

        civ_icon_array[i] = (HBITMAP)LoadImageW(NULL, icon_path.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    }
*/

    
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

	icon_de = (HBITMAP)LoadImageW(NULL, L"edition_icons\\aoe2de.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_hd = (HBITMAP)LoadImageW(NULL, L"edition_icons\\aoe2hd.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_aok = (HBITMAP)LoadImageW(NULL, L"edition_icons\\aok.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

	icon_africans = (HBITMAP)LoadImageW(NULL, L"dlc_icons\\african.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_aoc = (HBITMAP)LoadImageW(NULL, L"dlc_icons\\aoc.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_dukes = (HBITMAP)LoadImageW(NULL, L"dlc_icons\\dukes.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_forgotten = (HBITMAP)LoadImageW(NULL, L"dlc_icons\\forgotten.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_india = (HBITMAP)LoadImageW(NULL, L"dlc_icons\\india.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_khans = (HBITMAP)LoadImageW(NULL, L"dlc_icons\\khans.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_rajas = (HBITMAP)LoadImageW(NULL, L"dlc_icons\\rajas.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_rome = (HBITMAP)LoadImageW(NULL, L"dlc_icons\\rome.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_royals = (HBITMAP)LoadImageW(NULL, L"dlc_icons\\royals.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_west = (HBITMAP)LoadImageW(NULL, L"dlc_icons\\west.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
}

HBITMAP FetchIcon(std::wstring civ_name) {



                                        // can't get this to work yet, some icons disappear
    /*
    for (int i = 0; i < MAX_CIVS; i++) {
        if (civ_name == civ_index[i]) {
            return civ_icon_array[i];
        }
    }*/
        

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
    std::wstring jingle_path;

    if (legacy_jingle_enabled && VerifiedLegacyCiv(civ_name)) {
        jingle_path = L"civ_jingles\\legacy\\" + processed_civ_name + L".wav";
    }
	else jingle_path = L"civ_jingles\\" + processed_civ_name + L".wav";

    
    
    PlaySound(jingle_path.c_str(), NULL, SND_FILENAME | SND_ASYNC);

       
    
}

bool VerifiedLegacyCiv(std::wstring civ) {
    if (civ == L"Aztecs" ||
        civ == L"Britons" ||
        civ == L"Byzantines" ||
        civ == L"Celts" ||
        civ == L"Chinese" ||
        civ == L"Franks" ||
        civ == L"Goths" ||
        civ == L"Huns" ||
        civ == L"Japanese" ||
        civ == L"Koreans" ||
        civ == L"Mayans" ||
        civ == L"Mongols" ||
        civ == L"Persians" ||
        civ == L"Saracens" ||
        civ == L"Spanish" ||
        civ == L"Teutons" ||
        civ == L"Turks" ||
        civ == L"Vikings" ||
        civ == L"Random") return true;
    else return false;
}

void AddCiv(std::wstring civ) {

    if (!GetCivStatus(civ)) {
        civs.push_back(civ);
        SetCivStatus(civ, true);
	    custom_max_civs++;
        label_text = std::to_wstring(iterator) + L"/" + std::to_wstring(custom_max_civs);
        SetWindowText(label_corner, label_text.c_str());
	    if (custom_max_civs == MAX_CIVS) custom_civ_pool = false;
        UpdateRemainingLog();
        UpdateDrawnLog(false, false);
    }
}

void RemoveCiv(std::wstring civ) {
    if (GetCivStatus(civ)) {
        if (std::find(civs.begin(), civs.end(), civ) == civs.end()) iterator--;
        custom_civ_pool = true;
	    civs.erase(std::remove(civs.begin(), civs.end(), civ), civs.end());
	    SetCivStatus(civ, false);
	    custom_max_civs--;
        label_text = std::to_wstring(iterator) + L"/" + std::to_wstring(custom_max_civs);
        SetWindowText(label_corner, label_text.c_str());
        UpdateRemainingLog();
		UpdateDrawnLog(false, false);
    }    
}

void InitialiseCivStates() {
    civ_enabled = {
        {L"Armenians", true}, {L"Aztecs", true}, {L"Bengalis", true}, {L"Berbers", true},
        {L"Bohemians", true}, {L"Britons", true}, {L"Bulgarians", true}, {L"Burgundians", true},
        {L"Burmese", true}, {L"Byzantines", true}, {L"Celts", true}, {L"Chinese", true},
        {L"Cumans", true}, {L"Dravidians", true}, {L"Ethiopians", true}, {L"Franks", true},
        {L"Georgians", true}, {L"Goths", true}, {L"Gurjaras", true}, {L"Hindustanis", true},
        {L"Huns", true}, {L"Incas", true}, {L"Italians", true}, {L"Japanese", true},
        {L"Khmer", true}, {L"Koreans", true}, {L"Lithuanians", true}, {L"Magyars", true},
        {L"Malay", true}, {L"Malians", true}, {L"Mayans", true}, {L"Mongols", true},
        {L"Persians", true}, {L"Poles", true}, {L"Portuguese", true}, {L"Romans", true},
        {L"Saracens", true}, {L"Sicilians", true}, {L"Slavs", true}, {L"Spanish", true},
        {L"Tatars", true}, {L"Teutons", true}, {L"Turks", true}, {L"Vietnamese", true},
        {L"Vikings", true}
    };
}

void SetCivStatus(const std::wstring civ_name, bool status) {
    for (auto &civ : civ_enabled) {
        if (civ.first == civ_name) {
            civ.second = status;
            break;
        }
    }
}

bool GetCivStatus(const std::wstring civ_name) {
    for (const auto &civ : civ_enabled) {
        if (civ.first == civ_name) {
            return civ.second;
        }
    }
    return false;
}

HWND CreateCheckbox(HWND hWnd, HINSTANCE hInstance, int x, int y, int width, int height, int id, LPCWSTR text) {
    HWND checkbox = CreateWindow(
        L"BUTTON",          // Predefined class; Unicode assumed
        text,               // Button text
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, // Styles
        x,                  // x position
        y+120,                  // y position
        width+2,              // Button width
        height,             // Button height
        hWnd,               // Parent window
        (HMENU)id,          // Control ID
        hInstance,          // Instance handle
        NULL                // Pointer not needed
    );
    SendMessage(checkbox, BM_SETCHECK, BST_CHECKED, 0);

    return checkbox;
}

void ShowAllPoolCheckboxes() {
    for (int i = 0; i < MAX_CIVS; i++) {
        ShowWindow(civ_checkbox[i], SW_SHOW);
    }
    if (edition_state == DE) {
        ShowWindow(checkbox_khans, SW_SHOW);
		ShowWindow(checkbox_dukes, SW_SHOW);
        ShowWindow(checkbox_west, SW_SHOW);
		ShowWindow(checkbox_india, SW_SHOW);
		ShowWindow(checkbox_rome, SW_SHOW);
		ShowWindow(checkbox_royals, SW_SHOW);

    }
}

void HideCustomPoolCheckboxes() { 
	for (int i = 0; i < MAX_CIVS; i++) {
		ShowWindow(civ_checkbox[i], SW_HIDE);
	}
	ShowDEDLCCheckboxes(false);
	ShowHDDLCCheckboxes(false);
	ShowAOCCheckbox(false);
    ShowWindow(checkbox_aoc, SW_HIDE);
}

void EnableAll() {
    pool_altered = true;
	custom_civ_pool = false;

    if (edition_state == DE) {
        for (int i = 0; i < MAX_CIVS; i++) {        
            SendMessage(civ_checkbox[i], BM_SETCHECK, BST_CHECKED, 0);
            AddCiv(civ_index[i]);
        }
	}
	else if (edition_state == HD) {
		for (int i = 0; i < MAX_CIVS; i++) {
			if (GetCivEdition(civ_index[i]) == HD || GetCivEdition(civ_index[i]) == AOK) {
				SendMessage(civ_checkbox[i], BM_SETCHECK, BST_CHECKED, 0);
				AddCiv(civ_index[i]);
			}
		}
	}
	else if (edition_state == AOK) {
		for (int i = 0; i < MAX_CIVS; i++) {
			if (GetCivEdition(civ_index[i]) == AOK) {
				SendMessage(civ_checkbox[i], BM_SETCHECK, BST_CHECKED, 0);
				AddCiv(civ_index[i]);
			}
		}
	}

    
    if (autoreset_enabled) ResetProgram();
}

void DisableAll() {
    pool_altered = true;
	custom_civ_pool = true;
    for (int i = 0; i < MAX_CIVS; i++) {
        SendMessage(civ_checkbox[i], BM_SETCHECK, BST_UNCHECKED, 0);
		RemoveCiv(civ_index[i]);
    }
    if (autoreset_enabled) ResetProgram();
}

void ShowDrawTab(bool state, HWND hWnd) {
    int height = GetWindowHeight(hWnd);
    int width = GetWindowWidth(hWnd);
    if (state == true) {
        if (icons_enabled) ShowWindow(civ_icon, SW_SHOW);
        ShowWindow(button_draw, SW_SHOW);
        SetWindowPos(button_reset, NULL, 10, height - 40, 100, 30, SWP_NOZORDER);
	    ShowWindow(button_reset, SW_SHOW);
	    if (labels_enabled) ShowWindow(label_centre, SW_SHOW);
	    ShowWindow(label_corner, SW_SHOW);
	}
    else if (state == false) {
        ShowWindow(civ_icon, SW_HIDE);
        ShowWindow(button_draw, SW_HIDE);
        ShowWindow(button_reset, SW_HIDE);
        SetWindowPos(button_reset, NULL, (width - 100) / 2, height - 40, 100, 30, SWP_NOZORDER);
        ShowWindow(label_centre, SW_HIDE);
        ShowWindow(label_corner, SW_HIDE);
    }
	
}

void ShowLogTab(bool state) {
    if (state == true) {
        ShowWindow(button_draw, SW_SHOW);
        ShowWindow(button_reset, SW_SHOW);
		ShowWindow(label_drawncount, SW_SHOW);
		ShowWindow(button_clearlog, SW_SHOW);
        ShowWindow(drawn_log, SW_SHOW);
		ShowWindow(label_remainingcount, SW_SHOW);
        ShowWindow(checkbox_showremainlog, SW_SHOW);
		if (remainlog_enabled) ShowWindow(remaining_log, SW_SHOW);        
    }

    else if (state == false) {
		ShowWindow(label_drawncount, SW_HIDE);
		ShowWindow(button_clearlog, SW_HIDE);
        ShowWindow(drawn_log, SW_HIDE);
		ShowWindow(label_remainingcount, SW_HIDE);
		ShowWindow(checkbox_showremainlog, SW_HIDE);
        ShowWindow(remaining_log, SW_HIDE);
        ShowWindow(button_draw, SW_HIDE);
        ShowWindow(button_reset, SW_HIDE);
    }
}

void ShowCustomTab(bool state) {

    if (state == true) {
        ShowWindow(edition_icon, SW_SHOW);
		if (edition_state == DE) ShowAllPoolCheckboxes();
		else if (edition_state == HD) ShowHDPoolCheckboxes();
        else if (edition_state == AOK) ShowAOCPoolCheckboxes();
       
	    ShowWindow(button_enableall, SW_SHOW);
	    ShowWindow(button_disableall, SW_SHOW);		
		ShowWindow(radiobutton_de, SW_SHOW);
		ShowWindow(radiobutton_hd, SW_SHOW);
        ShowWindow(radiobutton_aok, SW_SHOW);
        ShowWindow(autoreset_checkbox, SW_SHOW);
        ShowWindow(autotoggle_checkbox, SW_SHOW);
		ShowWindow(edition_icon, SW_SHOW);

        if (edition_state == DE) {
			ShowDEDLCCheckboxes(true);
		}
		else if (edition_state == HD) {
			ShowHDDLCCheckboxes(true);
        }
        else if (edition_state == AOK) {
            ShowAOCCheckbox(true);
        }
    }

    else if (state == false) {
        ShowWindow(edition_icon, SW_HIDE);
        HideCustomPoolCheckboxes();
        ShowWindow(button_enableall, SW_HIDE);
        ShowWindow(button_disableall, SW_HIDE);
		ShowWindow(radiobutton_de, SW_HIDE);
		ShowWindow(radiobutton_hd, SW_HIDE);
		ShowWindow(radiobutton_aok, SW_HIDE);
        ShowWindow(autoreset_checkbox, SW_HIDE);
		ShowWindow(autotoggle_checkbox, SW_HIDE);
		ShowDEDLCCheckboxes(false);
    }
	
}


void ShowHDPoolCheckboxes() {
    for (int i = 0; i < MAX_CIVS; i++) {
        if (GetCivEdition(civ_index[i]) == HD || GetCivEdition(civ_index[i]) == AOK) {
            ShowWindow(civ_checkbox[i], SW_SHOW);
        }
        else {
			ShowWindow(civ_checkbox[i], SW_HIDE);
            SendMessage(civ_checkbox[i], BM_SETCHECK, BST_UNCHECKED, 0);
            RemoveCiv(civ_index[i]);
        }
        
    }
}

void ShowAOCPoolCheckboxes() {
    for (int i = 0; i < MAX_CIVS; i++) {
        if (GetCivEdition(civ_index[i]) == AOK) {
            ShowWindow(civ_checkbox[i], SW_SHOW);
		}
        else {
            ShowWindow(civ_checkbox[i], SW_HIDE);
            SendMessage(civ_checkbox[i], BM_SETCHECK, BST_UNCHECKED, 0);
            RemoveCiv(civ_index[i]);
        }
    }
}


edition GetCivEdition(const std::wstring& civ_name) {
    for (const auto& civ : civ_edition) {
        if (civ.first == civ_name) {
            return civ.second;
        }
    }
    return DE; // Default edition if not found
}

dlc GetCivDLC(const std::wstring &civ_name) {
	for (const auto& civ : civ_dlc) {
		if (civ.first == civ_name) {
			return civ.second;
		}
	}
	return aok; // Default if not found
}


void InitialiseCivEditions() {
	civ_edition = {
		{L"Armenians", DE}, {L"Aztecs", AOK}, {L"Bengalis", DE}, {L"Berbers", HD},
		{L"Bohemians", DE}, {L"Britons", AOK}, {L"Bulgarians", DE}, {L"Burgundians", DE},
		{L"Burmese", HD}, {L"Byzantines", AOK}, {L"Celts", AOK}, {L"Chinese", AOK},
		{L"Cumans", DE}, {L"Dravidians", DE}, {L"Ethiopians", HD}, {L"Franks", AOK},
		{L"Georgians", DE}, {L"Goths", AOK}, {L"Gurjaras", DE}, {L"Hindustanis", HD},
		{L"Huns", AOK}, {L"Incas", HD}, {L"Italians", HD}, {L"Japanese", AOK},
		{L"Khmer", HD}, {L"Koreans", AOK}, {L"Lithuanians", DE}, {L"Magyars", HD},
		{L"Malay", HD}, {L"Malians", HD}, {L"Mayans", AOK}, {L"Mongols", AOK},
		{L"Persians", AOK}, {L"Poles", DE}, {L"Portuguese", HD}, {L"Romans", DE},
		{L"Saracens", AOK}, {L"Sicilians", DE}, {L"Slavs", HD}, {L"Spanish", AOK},
		{L"Tatars", DE}, {L"Teutons", AOK}, {L"Turks", AOK}, {L"Vietnamese", HD},
		{L"Vikings", AOK}
	};
}




void InitialiseCivDLCs() {
	civ_dlc = {
		{L"Armenians", royals}, {L"Aztecs", aoc}, {L"Bengalis", india}, {L"Berbers", africans},
		{L"Bohemians", dukes}, {L"Britons", aok}, {L"Bulgarians", khans}, {L"Burgundians", west},
		{L"Burmese", rajas}, {L"Byzantines", aok}, {L"Celts", aok}, {L"Chinese", aok},
		{L"Cumans", khans}, {L"Dravidians", india}, {L"Ethiopians", africans}, {L"Franks", aok},
		{L"Georgians", royals}, {L"Goths", aok}, {L"Gurjaras", india}, {L"Hindustanis", forgotten},
		{L"Huns", aoc}, {L"Incas", forgotten}, {L"Italians", forgotten}, {L"Japanese", aok},
		{L"Khmer", rajas}, {L"Koreans", aoc}, {L"Lithuanians", khans}, {L"Magyars", forgotten},
		{L"Malay", rajas}, {L"Malians", africans}, {L"Mayans", aoc}, {L"Mongols", aok},
		{L"Persians", aok}, {L"Poles", dukes}, {L"Portuguese", africans}, {L"Romans", rome},
		{L"Saracens", aok}, {L"Sicilians", west}, {L"Slavs", forgotten}, {L"Spanish", aoc},
		{L"Tatars", khans}, {L"Teutons", aok}, {L"Turks", aok}, {L"Vietnamese", rajas},
		{L"Vikings", aok}
	};
}

void ShowDEDLCCheckboxes(bool de_state) {
    if (de_state) {
        ShowWindow(checkbox_khans, SW_SHOW);
	    ShowWindow(checkbox_dukes, SW_SHOW);
	    ShowWindow(checkbox_west, SW_SHOW);
	    ShowWindow(checkbox_india, SW_SHOW);
	    ShowWindow(checkbox_rome, SW_SHOW);
	    ShowWindow(checkbox_royals, SW_SHOW);
        
		ShowWindow(khans_icon, SW_SHOW);
		ShowWindow(dukes_icon, SW_SHOW);
		ShowWindow(west_icon, SW_SHOW);
		ShowWindow(india_icon, SW_SHOW);
		ShowWindow(rome_icon, SW_SHOW);
		ShowWindow(royals_icon, SW_SHOW);
    }
	else {
		ShowWindow(checkbox_khans, SW_HIDE);
		ShowWindow(checkbox_dukes, SW_HIDE);
		ShowWindow(checkbox_west, SW_HIDE);
		ShowWindow(checkbox_india, SW_HIDE);
		ShowWindow(checkbox_rome, SW_HIDE);
		ShowWindow(checkbox_royals, SW_HIDE);

		ShowWindow(khans_icon, SW_HIDE);
		ShowWindow(dukes_icon, SW_HIDE);
		ShowWindow(west_icon, SW_HIDE);
		ShowWindow(india_icon, SW_HIDE);
		ShowWindow(rome_icon, SW_HIDE);
		ShowWindow(royals_icon, SW_HIDE);
	}
	
}

void ShowHDDLCCheckboxes(bool hd_state) {
    if (hd_state) {
        ShowWindow(checkbox_forgotten, SW_SHOW);
		ShowWindow(checkbox_africans, SW_SHOW);
		ShowWindow(checkbox_rajas, SW_SHOW);

		ShowWindow(forgotten_icon, SW_SHOW);
		ShowWindow(africans_icon, SW_SHOW);
		ShowWindow(rajas_icon, SW_SHOW);
    }
	else {
		ShowWindow(checkbox_forgotten, SW_HIDE);
		ShowWindow(checkbox_africans, SW_HIDE);
		ShowWindow(checkbox_rajas, SW_HIDE);

		ShowWindow(forgotten_icon, SW_HIDE);
		ShowWindow(africans_icon, SW_HIDE);
		ShowWindow(rajas_icon, SW_HIDE);
	}
}

void ShowAOCCheckbox(bool aok_state) {
	if (aok_state) {
		ShowWindow(checkbox_aoc, SW_SHOW);

		ShowWindow(aoc_icon, SW_SHOW);
	}
	else {
		ShowWindow(checkbox_aoc, SW_HIDE);

		ShowWindow(aoc_icon, SW_HIDE);
	}
}

void ToggleDlc(dlc civ_dlc, bool toggle_state, HWND hWnd) {
    if (toggle_state) {
        for (int i = 0; i < MAX_CIVS; i++) {
            if (GetCivDLC(civ_index[i]) == civ_dlc) {
                CheckDlgButton(hWnd, i + 5, BST_CHECKED);
                AddCiv(civ_index[i]);
            }
        }
    }
    else {
        for (int i = 0; i < MAX_CIVS; i++) {
            if (GetCivDLC(civ_index[i]) == civ_dlc) {
                CheckDlgButton(hWnd, i + 5, BST_UNCHECKED);
                RemoveCiv(civ_index[i]);
            }
        }
    }
    remaininglog_text.clear();
    for (const auto& civ : civs) {
        remaininglog_text += civ + L"\r\n";
    }
    SetWindowText(remaining_log, remaininglog_text.c_str());
}

bool DlcFull(dlc civ_dlc) {
	int count = 0;
	for (int i = 0; i < MAX_CIVS; i++) {
		if (GetCivStatus(civ_index[i]) && GetCivDLC(civ_index[i]) == civ_dlc) {
			count++;
		}
	}

    if (count == 1 && civ_dlc == rome) return true;

    if (count == 3 && civ_dlc == india) return true;

    if (count == 2) {
        switch (civ_dlc) {
        case royals:
            return true;
		case dukes:
            return true;
        case west:
            return true;
        }
    }

    if (count == 4) {
        switch (civ_dlc) {
        case khans:
            return true;
        case rajas:
            return true;
        case africans:
            return true;
        }
    }

    if (count == 5) {
        switch (civ_dlc) {
        case forgotten:
            return true;
        case aoc:
            return true;
        }
    }

    return false;
}

void ValidateDlcToggle(HWND hWnD, dlc civ_dlc) {
	if (DlcFull(civ_dlc)) {
        switch (civ_dlc) {
        case khans:
            CheckDlgButton(hWnD, IDC_CHECKBOX_KHANS, BST_CHECKED);
            break;
        case dukes:
            CheckDlgButton(hWnD, IDC_CHECKBOX_DUKES, BST_CHECKED);
            break;
        case west:
            CheckDlgButton(hWnD, IDC_CHECKBOX_WEST, BST_CHECKED);
            break;
        case india:
            CheckDlgButton(hWnD, IDC_CHECKBOX_INDIA, BST_CHECKED);
            break;
        case rome:
            CheckDlgButton(hWnD, IDC_CHECKBOX_ROME, BST_CHECKED);
            break;
        case royals:
            CheckDlgButton(hWnD, IDC_CHECKBOX_ROYALS, BST_CHECKED);
            break;
        case forgotten:
            CheckDlgButton(hWnD, IDC_CHECKBOX_FORGOTTEN, BST_CHECKED);
            break;
        case africans:
            CheckDlgButton(hWnD, IDC_CHECKBOX_AFRICANS, BST_CHECKED);
            break;
        case rajas:
            CheckDlgButton(hWnD, IDC_CHECKBOX_RAJAS, BST_CHECKED);
            break;
        case aoc:
            CheckDlgButton(hWnD, IDC_CHECKBOX_AOC, BST_CHECKED);
            break;
        }
	}
	else {
        switch (civ_dlc) {
        case khans:
            CheckDlgButton(hWnD, IDC_CHECKBOX_KHANS, BST_UNCHECKED);
            break;
        case dukes:
            CheckDlgButton(hWnD, IDC_CHECKBOX_DUKES, BST_UNCHECKED);
            break;
        case west:
            CheckDlgButton(hWnD, IDC_CHECKBOX_WEST, BST_UNCHECKED);
            break;
        case india:
            CheckDlgButton(hWnD, IDC_CHECKBOX_INDIA, BST_UNCHECKED);
            break;
        case rome:
            CheckDlgButton(hWnD, IDC_CHECKBOX_ROME, BST_UNCHECKED);
            break;
        case royals:
            CheckDlgButton(hWnD, IDC_CHECKBOX_ROYALS, BST_UNCHECKED);
            break;
        case forgotten:
            CheckDlgButton(hWnD, IDC_CHECKBOX_FORGOTTEN, BST_UNCHECKED);
            break;
        case africans:
            CheckDlgButton(hWnD, IDC_CHECKBOX_AFRICANS, BST_UNCHECKED);
            break;
        case rajas:
            CheckDlgButton(hWnD, IDC_CHECKBOX_RAJAS, BST_UNCHECKED);
            break;
        case aoc:
            CheckDlgButton(hWnD, IDC_CHECKBOX_AOC, BST_UNCHECKED);
            break;
        }
	}
}

void ValidateAllDlcToggles(HWND hWnd) {
	ValidateDlcToggle(hWnd, khans);
	ValidateDlcToggle(hWnd, dukes);
	ValidateDlcToggle(hWnd, west);
	ValidateDlcToggle(hWnd, india);
	ValidateDlcToggle(hWnd, rome);
	ValidateDlcToggle(hWnd, royals);
	ValidateDlcToggle(hWnd, forgotten);
	ValidateDlcToggle(hWnd, africans);
	ValidateDlcToggle(hWnd, rajas);
	ValidateDlcToggle(hWnd, aoc);
}

void AddTooltip(HWND hwndTool, HWND hwndTip, LPCWSTR pszText)
{
    TOOLINFO toolInfo = { 0 };
    toolInfo.cbSize = sizeof(toolInfo);
    toolInfo.hwnd = hwndTool;
    toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS | TTF_TRACK;
    toolInfo.uId = (UINT_PTR)hwndTool;
    toolInfo.lpszText = (LPWSTR)pszText;
    SendMessage(hwndTip, TTM_ADDTOOL, 0, (LPARAM)&toolInfo);
}



void CreateTooltips(HWND hWnd)
{
    // Create the tooltip window
    HWND hwndTooltip = CreateWindowEx(0, TOOLTIPS_CLASS, NULL,
        WS_POPUP | TTS_ALWAYSTIP | TTS_NOPREFIX,
        CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT,
        hWnd, NULL, instance, NULL);

    // Set the maximum width for the tooltip window
    SendMessage(hwndTooltip, TTM_SETMAXTIPWIDTH, 0, 300);

    // Add tooltips for buttons
    AddTooltip(button_draw, hwndTooltip, L"Draw a random civilization");
    AddTooltip(button_reset, hwndTooltip, L"Reset the program");
    AddTooltip(button_enableall, hwndTooltip, L"Enable all civilizations");
    AddTooltip(button_disableall, hwndTooltip, L"Disable all civilizations");
}

void UpdateDrawnLog(bool drawn, bool blankline_wanted) {

    
    if (drawn) {
        drawnlog_length = GetWindowTextLength(drawn_log);
        drawnlog_text.resize(drawnlog_length + 1);
        GetWindowText(drawn_log, &drawnlog_text[0], drawnlog_length + 1);
        drawnlog_text.pop_back(); // Remove the null terminator
        log_entry = std::wstring(current_civ.begin(), current_civ.end()) + L"\r\n";
        drawnlog_text = log_entry + drawnlog_text;
        if (iterator == custom_max_civs) drawnlog_text += L"\r\n";
        SetWindowText(drawn_log, drawnlog_text.c_str());
    }

    else {
        if (!reset_state && blankline_wanted) {
            if (pool_altered || iterator >= 0)                       // adds blank line to log before next iteration of civ drawing
            {
                drawnlog_length = GetWindowTextLength(drawn_log);
                drawnlog_text.resize(drawnlog_length + 1);
                GetWindowText(drawn_log, &drawnlog_text[0], drawnlog_length + 1);
                drawnlog_text.pop_back();

                drawnlog_text = L"\r\n" + drawnlog_text;
                SetWindowText(drawn_log, drawnlog_text.c_str());

            }
        }
    }
    std::wstring drawn_label = L"Drawn: " + std::to_wstring(iterator) + L"/" + std::to_wstring(custom_max_civs);
    SetWindowText(label_drawncount, drawn_label.c_str());
}

void UpdateRemainingLog() {

    if (reset_state) {
        std::vector<std::wstring> sorted_civs = civs;
        std::sort(sorted_civs.begin(), sorted_civs.end());
        remaininglog_text.clear();
        for (const auto& civ : sorted_civs) {
            remaininglog_text += civ + L"\r\n";
        }
    }
    else {
        size_t pos = remaininglog_text.find(current_civ + L"\r\n");
        if (pos != std::wstring::npos) {
            remaininglog_text.erase(pos, current_civ.length() + 2); // +2 for "\r\n"
        }
    }

	std::wstring remain_label = L"Remaining: " + std::to_wstring(custom_max_civs - iterator) + L"/" + std::to_wstring(custom_max_civs);
    SetWindowText(remaining_log, remaininglog_text.c_str());
	SetWindowText(label_remainingcount, remain_label.c_str());
}

int GetWindowWidth(HWND hWnd) {
    RECT rect;
    GetClientRect(hWnd, &rect);
    return rect.right - rect.left;
}

int GetWindowHeight(HWND hWnd) {
    RECT rect;
    GetClientRect(hWnd, &rect);
    return rect.bottom - rect.top;
}