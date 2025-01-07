// WindowsProject1.cpp : Defines the entry point for the application.
//


#include "framework.h"
#include "WindowsProject1.h"
#include <iostream>
#include <string>
#include <random>
#include <cstdio>
#include <Windows.h>


#define MAX_LOADSTRING 100
int iterator = 0; // Global variable to keep track of the count

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HWND hLabel;
HWND hCenterLabel;
HWND hGenerateButton;
HWND hSizeLabel;
WCHAR szSizeText[50];

std::string civ_name(int);
int result(int);
bool available[45];
void resetter();



int remaining = 45;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.
    resetter();

    AllocConsole();

    // Redirect standard output to the console
    FILE *fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);

    // Now you can use printf and std::cout to output to the console
    printf("Console window successfully attached.\n");


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
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
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
        

        hGenerateButton = CreateWindow(
            L"BUTTON",  // Predefined class; Unicode assumed 
            L"Generate",      // Button text 
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
            0,         // x position 
            0,         // y position 
            100,        // Button width
            30,         // Button height
            hWnd,       // Parent window
            (HMENU)1,       // No menu.
            (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
            NULL);      // Pointer not needed.

        hLabel = CreateWindow(
            L"STATIC",  // Predefined class; Unicode assumed
            L"0/45",  // Label text from variable..
            WS_VISIBLE | WS_CHILD,  // Styles
            0,  // x position (will be set in WM_SIZE)
            0,  // y position (will be set in WM_SIZE)
            30,  // Label width
            15,  // Label height
            hWnd,  // Parent window
            NULL,  // No menu.
            (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
            NULL);  // Pointer not needed.


        hCenterLabel = CreateWindow(
            L"STATIC",  // Predefined class; Unicode assumed
            L"",  // Label text
            WS_VISIBLE | WS_CHILD,  // Styles
            0,  // x position (will be set in WM_SIZE)
            0,  // y position (will be set in WM_SIZE)
            100,  // Label width
            15,  // Label height
            hWnd,  // Parent window
            NULL,  // No menu.
            (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
            NULL);  // Pointer not needed.

        hSizeLabel = CreateWindow(
            L"STATIC",  // Predefined class; Unicode assumed
            L"",  // Initial text (empty)
            WS_VISIBLE | WS_CHILD,  // Styles
            0,  // x position (will be set in WM_SIZE)
            0,  // y position (will be set in WM_SIZE)
            100,  // Label width
            30,  // Label height
            hWnd,  // Parent window
            NULL,  // No menu.
            (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
            NULL);  // Pointer not needed.

        break;
    case WM_SIZE:
    {
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);

        SetWindowPos(hLabel, NULL, width - 50, height - 20, 40, 15, SWP_NOZORDER);
        SetWindowPos(hCenterLabel, NULL, (width - 100) / 2, (height - 15) / 2, 100, 15, SWP_NOZORDER);  // Centered positio
        SetWindowPos(hGenerateButton, NULL, (width - 100) / 2, (height + 25) / 2, 100, 30, SWP_NOZORDER);
        SetWindowPos(hSizeLabel, NULL, 10, height - 40, 100, 30, SWP_NOZORDER);  // Bottom left position

        wsprintf(szSizeText, L"%dx%d", width, height);
        SetWindowText(hSizeLabel, szSizeText);

    }
        break;

    case WM_GETMINMAXINFO:
    {
        MINMAXINFO* pmmi = (MINMAXINFO*)lParam;
        pmmi->ptMinTrackSize.x = 400; // Minimum width
        pmmi->ptMinTrackSize.y = 300; // Minimum height
    }
        break;

    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            int res = 0;
            int j = 0;
            
            std::string civ = "";
            std::wstring labelText = std::to_wstring(iterator + 1) + L"/45";

            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            case 1: // Handle Generate button click
                

                if (iterator == 45)
                {
                    printf("\nProgram has been reset.\n");
                    iterator = 0;

                    resetter();
                    remaining = 45;
                }

                 // Increment count and reset to 0 after 45

                res = result(remaining);

                
                j = 0;
                for (int i = 0; i < 45; i++) {
                    
                    if (i == res) {
                        printf("res (i) is now %d\n", res);
                        printf("available[%d] is currently %s\n", res, available[res] ? "true" : "false");
                        res = j;
                        printf("res (j) is now %d\n", res);
                        printf("available[%d] is currently %s\n", res, available[res] ? "true" : "false");
                        break;
                    }
                    while (!available[j]) j++;
                    j++;

                }

                

                printf("\n%s (current set: %d/%d)\n", civ_name(res), iterator + 1, 45);
                available[res] = 0;
                iterator++;
                remaining--;

                civ = civ_name(res);

                labelText = std::to_wstring(iterator) + L"/45";
                SetWindowText(hLabel, labelText.c_str());
                SetWindowTextA(hCenterLabel, civ.c_str());

                
                

                

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
        PostQuitMessage(0);
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

    return dist(mt);  // Return a random number between 0 and 44
}

void resetter() {
    for (int i = 0; i < 45; i++) {
        available[i] = true;
    }
}
