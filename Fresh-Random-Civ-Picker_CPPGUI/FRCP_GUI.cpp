/*
TODO

- add options button on draw tab
- add hotkeys list info to options
- civ info button and pages
- additional resources and links page
- make subclassed button component graphics react to hover again

*/
#include "FRCP_GUI.h"




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
    if (startup && jingles_enabled) PlayJingle(current_civ);
    else if (ui_sounds_enabled) PlaySound(L"tab_sound.wav", NULL, SND_FILENAME | SND_ASYNC);
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

    GenerateFilePaths();
    LoadSettings();
    

    // Initialize GDI+
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);


    CreateUnderlineFont();
	InitialiseCivStates();


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

    if (persistent_logging) SaveLog();
    SaveSettings();

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
        CW_USEDEFAULT, 0, MIN_WIDTH, MIN_HEIGHT, nullptr, nullptr, hInstance, nullptr);

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
        {
            INITCOMMONCONTROLSEX iccex;
            iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
            iccex.dwICC = ICC_WIN95_CLASSES | ICC_BAR_CLASSES | ICC_COOL_CLASSES | ICC_TREEVIEW_CLASSES;
            InitCommonControlsEx(&iccex);
            if (!InitCommonControlsEx(&iccex)) {
                MessageBox(hWnd, L"Failed to initialize common controls.", L"Error", MB_OK | MB_ICONERROR);
                return -1;
            }
            LoadImages();
            CreateTabs(hWnd);        
            CreateImages(hWnd);
            CreateCheckboxes(hWnd);
            CreateButtons(hWnd);
            CreateLabels(hWnd);
            CreateTextfields(hWnd);
            CreateRadiobuttons(hWnd);			
            SubclassButtons();
            CreateTooltips(hWnd);
            AddTooltips();
            SetEditionState(hWnd, DE);   
            if (persistent_logging)
            {
				LoadLog(hWnd);
                ValidateAllDlcToggles(hWnd);
            }           
			ShowTabComponents(0, hWnd);
            startup = false;
            EnableHotkeys(hWnd);
            if (draw_on_startup) DrawCiv();
			else if (!persistent_logging) ResetProgram();            
            break;
        }
        case WM_SIZE:
        {
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);

            SetWindowPos(button_draw, NULL, (width - 100) / 2, (height + 100) / 2, 100, 30, SWP_NOZORDER);   // draw button anchored to centre
            SetWindowPos(button_techtree, NULL, width - 150, height - 120, 60, 60, SWP_NOZORDER);   // draw button anchored to centre


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
            SetWindowPos(drawn_log, NULL, 10, 60, width - (width / 2) - 69, height - 70, SWP_NOZORDER);        // log text field anchored to window size
            
			SetWindowPos(label_remainingcount, NULL, (width / 2) + 60, 25, 130, 15, SWP_NOZORDER);                           // drawn civ label anchored to top right corner
			SetWindowPos(checkbox_showremainlog, NULL, (width / 2) + 190, 25, 60, 15, SWP_NOZORDER);                           // drawn civ label anchored to top right corner
            SetWindowPos(remaining_log, NULL, (width / 2) + 60, 60, width - (width / 2) - 65, height - 70, SWP_NOZORDER);                   // log text field anchored to window size

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

            SetWindowPos(edition_icon, NULL, 190, 30, 128, 93, SWP_NOZORDER);


            break;
        }
        case WM_ACTIVATE: // re-enables hotkeys when window returns to foreground 
        {
            if (wParam == WA_ACTIVE || wParam == WA_CLICKACTIVE) EnableHotkeys(hWnd);
            else if (wParam == WA_INACTIVE) DisableHotkeys(hWnd);
            break;
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

            if (pnmhdr->code == TTN_SHOW)
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
            if (!tooltips_enabled) break;
            if (hwndTooltip)
            {
                // Get the cursor position
                POINT pt;
                GetCursorPos(&pt);

                // Convert screen coordinates to client coordinates
                ScreenToClient(hWnd, &pt);

                // Check if the cursor is over any button and update the tooltip accordingly
                RECT rect;
                TOOLINFO toolInfo = { 0 };
                toolInfo.cbSize = sizeof(toolInfo);
                toolInfo.hwnd = hWnd;
                toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS | TTF_TRACK;

                bool tooltipActivated = false;

                HWND button[] = { button_draw, button_reset, button_techtree, button_options, button_clearlog,
                    button_enableall, button_disableall, checkbox_showremainlog,
                    radiobutton_de, radiobutton_hd, radiobutton_aok, checkbox_royals,
                checkbox_rome, checkbox_india, checkbox_dukes, checkbox_west, checkbox_khans,
                checkbox_rajas, checkbox_africans, checkbox_forgotten, checkbox_aoc,
                checkbox_autotoggle, checkbox_autoreset };

                int tooltip_id[] = { TOOLTIP_DRAW, TOOLTIP_RESET, TOOLTIP_TECHTREE, TOOLTIP_OPTIONS, TOOLTIP_CLEAR,
                                        TOOLTIP_ENABLEALL, TOOLTIP_DISABLEALL, TOOLTIP_REMAININGTOGGLE,
                                        TOOLTIP_DE, TOOLTIP_HD, TOOLTIP_AOK, TOOLTIP_ROYALS, TOOLTIP_ROME,
                                    TOOLTIP_INDIA, TOOLTIP_DUKES, TOOLTIP_WEST, TOOLTIP_KHANS,
                                    TOOLTIP_RAJAS, TOOLTIP_AFRICANS, TOOLTIP_FORGOTTEN, TOOLTIP_AOC,
                                    TOOLTIP_AUTOTOGGLE, TOOLTIP_AUTORESET };

                
				for (int i = 0; i < sizeof(button) / sizeof(button[0]); i++)
				{
					if (GetWindowRect(button[i], &rect))
					{
						MapWindowPoints(HWND_DESKTOP, hWnd, (LPPOINT)&rect, 2);
						if (PtInRect(&rect, pt))
						{
							toolInfo.uId = (UINT_PTR)button[i];
							ActivateTooltip(hwndTooltip[tooltip_id[i]], &toolInfo, pt);
							tooltipActivated = true;
						}
                        else SendMessage(hwndTooltip[tooltip_id[i]], TTM_TRACKACTIVATE, FALSE, (LPARAM)&toolInfo);
					}                    
				}             
                
                // Deactivate the tooltip if the cursor is not over any button
                if (!tooltipActivated)
                {
                    for (int i = 0; i < hwnd_length; i++)
                    {
                        SendMessage(hwndTooltip[i], TTM_TRACKACTIVATE, FALSE, (LPARAM)&toolInfo);
                    }                   

                }
            }
            break;
        }
        case WM_GETMINMAXINFO:                                  // minimum window size
        {
            MINMAXINFO *pmmi = (MINMAXINFO *)lParam;
            pmmi->ptMinTrackSize.x = MIN_WIDTH; // Minimum width
            pmmi->ptMinTrackSize.y = MIN_HEIGHT; // Minimum height
            break;
        }
        case WM_HOTKEY:
        {     
            hotkey_pressed = true;
            if (GetForegroundWindow() != hWnd)  // disables hotkeys if window is not in foreground
            {
                DisableHotkeys(hWnd);
                break;
            }
            else EnableHotkeys(hWnd);

            if (wParam == HOTKEY_ID_ESC) {              
                KillApplication();                // escape for exiting
            }            

            if (current_tab == 0)
            {
                if (wParam == HOTKEY_ID_Q)
                {
                    OpenOptions(hWnd);
                }
                if (wParam == HOTKEY_ID_T)
                {
                    if (ui_sounds_enabled) PlaySound(L"button_sound.wav", NULL, SND_FILENAME | SND_ASYNC);
                    OpenTechTree();                    
                }
            }

            if (current_tab != 2) {
                if (wParam == HOTKEY_ID_SPACE) DrawCiv();           // space for drawing civ
                if (wParam == HOTKEY_ID_RETURN) ResetProgram();            // return for resetting
            }
            else if (current_tab == 2)
            {
                if (wParam > 1 && wParam < 4 || wParam > 12 && wParam < 22)
                {                    
                    if (autoreset_enabled) ResetProgram();
                }
                    
                if (wParam == HOTKEY_ID_SPACE) EnableAll(hWnd);           
                if (wParam == HOTKEY_ID_RETURN) DisableAll(hWnd);
                if (wParam == HOTKEY_ID_Q) SetEditionState(hWnd, DE);
                if (wParam == HOTKEY_ID_W) SetEditionState(hWnd, HD);
				if (wParam == HOTKEY_ID_E) SetEditionState(hWnd, AOK);
                if (wParam == HOTKEY_ID_R) ToggleAutoToggle(hWnd);
                if (wParam == HOTKEY_ID_T) ToggleAutoReset(hWnd);
                
                if (ui_sounds_enabled)
                {
                    if (wParam > 15 && wParam < 23 ||
                        wParam == HOTKEY_ID_T) PlaySound(L"button_sound.wav", NULL, SND_FILENAME | SND_ASYNC);
                }
                   

                if (edition_state == DE)
                {
					if (wParam == HOTKEY_ID_A) ToggleDlc(royals, hWnd);
					if (wParam == HOTKEY_ID_S) ToggleDlc(rome, hWnd);
					if (wParam == HOTKEY_ID_D) ToggleDlc(india, hWnd);
					if (wParam == HOTKEY_ID_F) ToggleDlc(dukes, hWnd);
					if (wParam == HOTKEY_ID_G) ToggleDlc(west, hWnd);
					if (wParam == HOTKEY_ID_H) ToggleDlc(khans, hWnd);
				}
				else if (edition_state == HD)
				{
					if (wParam == HOTKEY_ID_A) ToggleDlc(rajas, hWnd);
					if (wParam == HOTKEY_ID_S) ToggleDlc(africans, hWnd);
					if (wParam == HOTKEY_ID_D) ToggleDlc(forgotten, hWnd);
				}
                else if (edition_state == AOK)
                {
                    if (wParam == HOTKEY_ID_A) ToggleDlc(aoc, hWnd);
                }

            }

            if (current_tab == 1)
            {
                if (wParam == HOTKEY_ID_Q)
                {
                    SetWindowText(drawn_log, L"");
                    if (ui_sounds_enabled) PlaySound(L"button_sound.wav", NULL, SND_FILENAME | SND_ASYNC);
                }
				if (wParam == HOTKEY_ID_W)
				{
                    ToggleRemainLog();
				}
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

            if (wParam == HOTKEY_ID_1)
            {
                TabCtrl_SetCurSel(tab, 0);
				ShowTabComponents(0, hWnd);
            }

			if (wParam == HOTKEY_ID_2)
			{
                TabCtrl_SetCurSel(tab, 1);
				ShowTabComponents(1, hWnd);
			}

			if (wParam == HOTKEY_ID_3)
			{
                TabCtrl_SetCurSel(tab, 2);
				ShowTabComponents(2, hWnd);
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
					MuteSounds();
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


            if (hOptionsDlg != NULL)
            {
                SendMessage(hOptionsDlg, WM_HOTKEY, wParam, lParam);
                return 0;
            }
            break;
        }    
        case WM_COMMAND:                                     
        {
            hotkey_pressed = false;

            int wmId = LOWORD(wParam);            


            if (wmId >= 5 && wmId <= 49) {

                if (IsDlgButtonChecked(hWnd, wmId) == BST_CHECKED) AddCiv(civ_index[wmId-5]);
                else RemoveCiv(civ_index[wmId - 5]);
                ValidateDlcToggle(hWnd, GetCivDLC(civ_index[wmId - 5]));
				

            }         


			// button sounds in tab views
            if (ui_sounds_enabled) {
                if (wmId > 2 && wmId < 51 || wmId > 53 && wmId < 68) {
                    PlaySound(L"button_sound.wav", NULL, SND_FILENAME | SND_ASYNC);
                }
            }

            // auto-reset
            if (autoreset_enabled) {
				if (wmId > 4 && wmId < 50 || wmId > 50 && wmId < 65) ResetProgram();
            }

            switch (wmId)
            {
            case IDM_ABOUT:                                     // "About"
                if (ui_sounds_enabled) PlaySound(L"button_sound.wav", NULL, SND_FILENAME | SND_ASYNC);
                DialogBox(instance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, AboutDlgProc);
                break;
            case IDM_EXIT:                                     // "Exit"
                KillApplication();
                break;
            case IDM_OPTIONS:								   // "Options"
                if (ui_sounds_enabled) PlaySound(L"button_sound.wav", NULL, SND_FILENAME | SND_ASYNC);
                DialogBox(instance, MAKEINTRESOURCE(IDD_OPTIONS), hWnd, OptionsDlgProc);
                break;
            case IDM_HOTKEYS:
                OpenHotkeys(hWnd);
                break;
            case IDM_GITHUB:                                    // "GitHub"
                if (ui_sounds_enabled) PlaySound(L"button_sound.wav", NULL, SND_FILENAME | SND_ASYNC);
                ShellExecute(0, 0, L"https://github.com/cnordenb/Fresh-Random-Civ-Picker_CPPGUI", 0, 0, SW_SHOW);
                break;
            case IDM_WEBVERSION:                                // "Web Version"
                if (ui_sounds_enabled) PlaySound(L"button_sound.wav", NULL, SND_FILENAME | SND_ASYNC);
                ShellExecute(0, 0, L"https://cnordenb.github.io/Fresh-Random-Civ-Picker_web/", 0, 0, SW_SHOW);
                break;
            case IDC_BUTTON_DRAW:                                             // "Draw"     
                if (ui_sounds_enabled && !jingles_enabled) PlaySound(L"button_sound.wav", NULL, SND_FILENAME | SND_ASYNC);
                DrawCiv();
                break;
            case IDC_BUTTON_RESET:                                             // "Reset"
                if (ui_sounds_enabled && !jingles_enabled) PlaySound(L"button_sound.wav", NULL, SND_FILENAME | SND_ASYNC);
                ResetProgram();
                break;
            case IDC_BUTTON_ENABLEALL:                                             // "Enable All"                
                EnableAll(hWnd);
                break;
            case IDC_BUTTON_DISABLEALL:                                             // "Disable All"
                DisableAll(hWnd);
                break;
			case IDC_BUTTON_CLEARLOG:                           // "Clear"
				SetWindowText(drawn_log, L"");
                if (ui_sounds_enabled) PlaySound(L"button_sound.wav", NULL, SND_FILENAME | SND_ASYNC);
                break;
            case IDC_BUTTON_OPTIONS:
                OpenOptions(hWnd);
                break;
            case IDC_BUTTON_TECHTREE:
                OpenTechTree();
                break;
            case IDC_CHECKBOX_REMAINLOG:                        // "Show Remaining Civs Log"
                ToggleRemainLog();
                break;
            case IDC_CHECKBOX_AUTORESET:										                            // Auto-reset Checkbox
                ToggleAutoReset(hWnd);
                break;
            case IDC_CHECKBOX_AUTOTOGGLE:											                            // Auto-reset Checkbox
                ToggleAutoToggle(hWnd);
                break;
            case IDC_RADIO_DE:
                SetEditionState(hWnd, DE);
                break;
            case IDC_RADIO_HD:
                SetEditionState(hWnd, HD);
                break;
            case IDC_RADIO_AOK:
                SetEditionState(hWnd, AOK);
                break;            
            case IDC_CHECKBOX_ROYALS:
                ToggleDlc(royals, hWnd);
                break;
			case IDC_CHECKBOX_ROME:
				ToggleDlc(rome, hWnd);
				break;
            case IDC_CHECKBOX_INDIA:
				ToggleDlc(india, hWnd);
                break;
            case IDC_CHECKBOX_DUKES:
				ToggleDlc(dukes, hWnd);
                break;
            case IDC_CHECKBOX_WEST:
				ToggleDlc(west, hWnd);
                break;
            case IDC_CHECKBOX_KHANS:
				ToggleDlc(khans, hWnd);
                break;
            case IDC_CHECKBOX_RAJAS:
				ToggleDlc(rajas, hWnd);
                break;
            case IDC_CHECKBOX_AFRICANS:
				ToggleDlc(africans, hWnd);
                break;
            case IDC_CHECKBOX_FORGOTTEN:
				ToggleDlc(forgotten, hWnd);
                break;
            case IDC_CHECKBOX_AOC:
				ToggleDlc(aoc, hWnd);
                break;
            case IDC_ICON_CIV:
                if (jingles_enabled) PlayJingle(current_civ);
                break;
            case IDC_ICON_EDITION:
                switch (edition_state)
                {
                case DE:
                    ShellExecute(NULL, L"open", L"https://store.steampowered.com/app/813780/Age_of_Empires_II_Definitive_Edition/", NULL, NULL, SW_SHOWNORMAL);
                    break;
                case HD:
                    ShellExecute(NULL, L"open", L"https://store.steampowered.com/app/221380/Age_of_Empires_II_Retired/", NULL, NULL, SW_SHOWNORMAL);
                    break;
                case AOK:
                    break;
                }
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
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

LRESULT CALLBACK ButtonProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_NCCREATE)
    {        
        // Store the original window procedure
        originalButtonProcs[hwnd] = (WNDPROC)GetWindowLongPtr(hwnd, GWLP_WNDPROC);        
    }

    switch (uMsg)
    {    
    case WM_MOUSEMOVE:
    {
        if (!tooltips_enabled) break;

        if (hwndTooltip)
        {
            // Get the cursor position
            POINT pt;
            GetCursorPos(&pt);

            // Convert screen coordinates to client coordinates
            ScreenToClient(hwnd, &pt);

            // Check if the cursor is over any button and update the tooltip accordingly
            RECT rect;
            TOOLINFO toolInfo = { 0 };
            toolInfo.cbSize = sizeof(toolInfo);
            toolInfo.hwnd = hwnd;
            toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS | TTF_TRACK;

            HWND button[] = { button_draw, button_reset, button_techtree, button_options, button_clearlog,
                    button_enableall, button_disableall, checkbox_showremainlog,
                    radiobutton_de, radiobutton_hd, radiobutton_aok, checkbox_royals,
                checkbox_rome, checkbox_india, checkbox_dukes, checkbox_west, checkbox_khans,
                checkbox_rajas, checkbox_africans, checkbox_forgotten, checkbox_aoc,
                checkbox_autotoggle, checkbox_autoreset };

            int tooltip_id[] = { TOOLTIP_DRAW, TOOLTIP_RESET, TOOLTIP_TECHTREE, TOOLTIP_OPTIONS, TOOLTIP_CLEAR,
                                    TOOLTIP_ENABLEALL, TOOLTIP_DISABLEALL, TOOLTIP_REMAININGTOGGLE,
                                    TOOLTIP_DE, TOOLTIP_HD, TOOLTIP_AOK, TOOLTIP_ROYALS, TOOLTIP_ROME,
                                TOOLTIP_INDIA, TOOLTIP_DUKES, TOOLTIP_WEST, TOOLTIP_KHANS,
                                TOOLTIP_RAJAS, TOOLTIP_AFRICANS, TOOLTIP_FORGOTTEN, TOOLTIP_AOC,
                                TOOLTIP_AUTOTOGGLE, TOOLTIP_AUTORESET };



            for (int i = 0; i < sizeof(button) / sizeof(button[0]); i++)
			{
				if (GetWindowRect(button[i], &rect))
				{
					MapWindowPoints(HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2);
					if (PtInRect(&rect, pt))
					{
						toolInfo.uId = (UINT_PTR)button[i];
						ActivateTooltip(hwndTooltip[tooltip_id[i]], &toolInfo, pt);
					}
				}
			}



        }


        // Forward the WM_MOUSEMOVE message to the parent window
        SendMessage(GetParent(hwnd), WM_MOUSEMOVE, wParam, lParam);

        break;
    }
        
    case WM_SETCURSOR:
        // Set the cursor to a hand cursor when the mouse is over the button
        SetCursor(LoadCursor(NULL, IDC_HAND));
        return TRUE;
    default:
        return CallWindowProc(originalButtonProcs[hwnd], hwnd, uMsg, wParam, lParam);
    }



    return 0;
    // Call the original window procedure for default processing
    
}


void SubclassButton(HWND button)
{
    // Store the original window procedure
    originalButtonProcs[button] = (WNDPROC)SetWindowLongPtr(button, GWLP_WNDPROC, (LONG_PTR)ButtonProc);
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
        CheckDlgButton(hDlg, IDC_CHECKBOX_TOOLTIPS, tooltips_enabled ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHECKBOX_STARTDRAW, draw_on_startup ? BST_CHECKED : BST_UNCHECKED);


		if (persistent_logging) CheckRadioButton(hDlg, IDC_RADIO_LOGGING, IDC_RADIO_STARTRESET, IDC_RADIO_LOGGING);
		else CheckRadioButton(hDlg, IDC_RADIO_LOGGING, IDC_RADIO_STARTRESET, IDC_RADIO_STARTRESET);

        HWND hComboBox = GetDlgItem(hDlg, IDC_LEGACY_OPTION);
        SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM)L"Definitive Edition");
        SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM)L"Legacy");
        SendMessage(hComboBox, CB_SETCURSEL, legacy_jingle_enabled ? 1 : 0, 0); // Set default selection based on current setting


        return(INT_PTR)TRUE;
    }
        case WM_HOTKEY:
        {
			if (wParam == HOTKEY_ID_Q) OpenOptions(hDlg);
            break;
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
        tooltips_enabled = IsDlgButtonChecked(hDlg, IDC_CHECKBOX_TOOLTIPS) == BST_CHECKED;
		draw_on_startup = IsDlgButtonChecked(hDlg, IDC_CHECKBOX_STARTDRAW) == BST_CHECKED;

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
            if (wmId == IDC_CHECKBOX_JINGLES)
            {
                if (!jingles_enabled) MuteSounds();
                else PlayJingle(current_civ);
            }
                
            switch (wmId)
            {
            case IDC_BUTTON_HOTKEYS:
				OpenHotkeys(hDlg);
                break;
            case IDC_CHECKBOX_LABELS:
            case IDC_CHECKBOX_ICONS:
            case IDC_CHECKBOX_SOUNDS:
            case IDC_CHECKBOX_TOOLTIPS:
			case IDC_CHECKBOX_STARTDRAW:
                if (ui_sounds_enabled)
                {
                    PlaySound(L"button_sound.wav", NULL, SND_FILENAME | SND_ASYNC);
                }
                break;
			case IDC_RADIO_LOGGING:
				persistent_logging = true;
                break;
			case IDC_RADIO_STARTRESET:
				persistent_logging = false;
				break;
            }
        }


		if (!labels_enabled) ShowWindow(label_centre, SW_HIDE);
		else if (current_tab == 0) ShowWindow(label_centre, SW_SHOW); 
        
        if (wmId == IDCANCEL || wmId == IDOK) {
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

    UpdateDrawnLog(false, false, true);


    if (custom_civ_pool)
    {
        civs.clear();
        custom_max_civs = 0;
        for (int i = 0; i < MAX_CIVS; i++) {
            if (GetCivStatus(civ_index[i])) {
                civs.push_back(civ_index[i]);
                custom_max_civs++;
            }
        }

    }
    else
    {
        InitialiseCivs();        
    }


    
	for (int i = 0; i < MAX_CIVS; i++)
    {
        if (drawn_civs[i] == L"") break;
		drawn_civs[i] = L"";
	}
    


    SetWindowText(label_corner, (L"0/" + std::to_wstring(custom_max_civs)).c_str());     // resets remaining civs label
    SetWindowText(label_centre, L"?");                                      // resets drawn civ label
    SendMessageW(civ_icon, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)icon_random);
    if (jingles_enabled && current_tab != 2) PlayJingle(current_civ);
    reset_state = true;

    UpdateRemainingLog(false);
    UpdateTooltipText(button_techtree, hwndTooltip[TOOLTIP_TECHTREE], StringCleaner(L"Opens the tech tree\nHotkey: T"));


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
    SendMessageW(civ_icon, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)drawn_civ_icon);



    UpdateDrawnLog(false, true, false);
    UpdateRemainingLog(false);



    if (current_tab != 0) {
        ShowWindow(civ_icon, SW_HIDE);
        ShowWindow(label_centre, SW_HIDE);
    }


    if (jingles_enabled)
    {/*
        std::thread sound_thread(PlayJingle, current_civ);
        sound_thread.detach();*/
        PlayJingle(current_civ);
    }


    UpdateTooltipText(button_techtree, hwndTooltip[TOOLTIP_TECHTREE], StringCleaner(L"Opens the tech tree for the " + current_civ + L"\nHotkey: T"));


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
	RegisterHotKey(hWnd, HOTKEY_ID_T, 0, 0x54);
	RegisterHotKey(hWnd, HOTKEY_ID_1, 0, 0x31);
	RegisterHotKey(hWnd, HOTKEY_ID_2, 0, 0x32);
	RegisterHotKey(hWnd, HOTKEY_ID_3, 0, 0x33);
	RegisterHotKey(hWnd, HOTKEY_ID_Q, 0, 0x51);
	RegisterHotKey(hWnd, HOTKEY_ID_W, 0, 0x57);
	RegisterHotKey(hWnd, HOTKEY_ID_E, 0, 0x45);
	RegisterHotKey(hWnd, HOTKEY_ID_A, 0, 0x41);
	RegisterHotKey(hWnd, HOTKEY_ID_S, 0, 0x53);
	RegisterHotKey(hWnd, HOTKEY_ID_D, 0, 0x44);
	RegisterHotKey(hWnd, HOTKEY_ID_F, 0, 0x46);
	RegisterHotKey(hWnd, HOTKEY_ID_G, 0, 0x47);
	RegisterHotKey(hWnd, HOTKEY_ID_H, 0, 0x48);
	RegisterHotKey(hWnd, HOTKEY_ID_R, 0, 0x52);
	RegisterHotKey(hWnd, HOTKEY_ID_T, 0, 0x54);
}

void DisableHotkeys(HWND hWnd)
{
	for (int i = 1; i < 24; i++) {
		UnregisterHotKey(hWnd, i);
	}
}

void KillApplication()
{    
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
    icon_techtree = (HBITMAP)LoadImageW(NULL, L"civ_icons\\techtree.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    
    
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

HBITMAP FetchIcon(const std::wstring &civ_name) {



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
	if (civ_name == L"Random") return icon_random;

    
}

void PlayJingle(std::wstring &civ_name) {



    std::wstring processed_civ_name = civ_name;
    processed_civ_name[0] = std::tolower(processed_civ_name[0]);
    std::wstring jingle_path;

    if (legacy_jingle_enabled && VerifiedLegacyCiv(civ_name)) {
        jingle_path = L"civ_jingles\\legacy\\" + processed_civ_name + L".wav";
    }
	else jingle_path = L"civ_jingles\\" + processed_civ_name + L".wav";

    
    
    PlaySound(jingle_path.c_str(), NULL, SND_FILENAME | SND_ASYNC);

       
    
}

bool VerifiedLegacyCiv(const std::wstring &civ) {
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

void AddCiv(const std::wstring &civ) {

    if (!GetCivStatus(civ)) {
        civs.push_back(civ);
        SetCivStatus(civ, true);
	    custom_max_civs++;
        label_text = std::to_wstring(iterator) + L"/" + std::to_wstring(custom_max_civs);
        SetWindowText(label_corner, label_text.c_str());
	    if (custom_max_civs == MAX_CIVS) custom_civ_pool = false;
        UpdateRemainingLog(true);
        UpdateDrawnLog(false, false, false);
    }
}

void RemoveCiv(const std::wstring &civ) {
    if (GetCivStatus(civ)) {
        if (std::find(civs.begin(), civs.end(), civ) == civs.end()) iterator--;
        custom_civ_pool = true;
	    civs.erase(std::remove(civs.begin(), civs.end(), civ), civs.end());
	    SetCivStatus(civ, false);
	    custom_max_civs--;
        label_text = std::to_wstring(iterator) + L"/" + std::to_wstring(custom_max_civs);
        SetWindowText(label_corner, label_text.c_str());
        UpdateRemainingLog(true);
		UpdateDrawnLog(false, false, false);
    }    
}

void InitialiseCivStates() {
    std::pair<std::wstring, bool> temp_civ_enabled[MAX_CIVS] = {
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
    std::copy(std::begin(temp_civ_enabled), std::end(temp_civ_enabled), std::begin(civ_enabled));
}

void SetCivStatus(const std::wstring &civ_name, bool status) {
    for (auto &civ : civ_enabled) {
        if (civ.first == civ_name) {
            civ.second = status;
            break;
        }
    }
}

bool GetCivStatus(const std::wstring &civ_name) {
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

void EnableAll(HWND hWnd) {
    if (ui_sounds_enabled) PlaySound(L"button_sound.wav", NULL, SND_FILENAME | SND_ASYNC);
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
    ValidateAllDlcToggles(hWnd);
}

void DisableAll(HWND hWnd) {
    if (ui_sounds_enabled) PlaySound(L"button_sound.wav", NULL, SND_FILENAME | SND_ASYNC);
	custom_civ_pool = true;
    for (int i = 0; i < MAX_CIVS; i++) {
        SendMessage(civ_checkbox[i], BM_SETCHECK, BST_UNCHECKED, 0);
		RemoveCiv(civ_index[i]);
    }
    if (autoreset_enabled) ResetProgram();
    ValidateAllDlcToggles(hWnd);
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
		ShowWindow(button_techtree, SW_SHOW);
        ShowWindow(button_options, SW_SHOW);
	}
    else if (state == false) {
        ShowWindow(civ_icon, SW_HIDE);
        ShowWindow(button_draw, SW_HIDE);
        ShowWindow(button_reset, SW_HIDE);
        SetWindowPos(button_reset, NULL, (width - 100) / 2, height - 40, 100, 30, SWP_NOZORDER);
        ShowWindow(label_centre, SW_HIDE);
        ShowWindow(label_corner, SW_HIDE);
		ShowWindow(button_techtree, SW_HIDE);
        ShowWindow(button_options, SW_HIDE);
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
        ShowWindow(checkbox_autoreset, SW_SHOW);
        ShowWindow(checkbox_autotoggle, SW_SHOW);
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
        ShowWindow(checkbox_autoreset, SW_HIDE);
		ShowWindow(checkbox_autotoggle, SW_HIDE);
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

void ToggleDlc(dlc civ_dlc, HWND hWnd)
{

	int check_id = GetDlcCheckboxId(civ_dlc);


    if (hotkey_pressed)
    {
		if (IsDlgButtonChecked(hWnd, check_id) == BST_CHECKED) {
			CheckDlgButton(hWnd, check_id, BST_UNCHECKED);
		}
		else {
			CheckDlgButton(hWnd, check_id, BST_CHECKED);
		}
    }

    if (IsDlgButtonChecked(hWnd, check_id) == BST_CHECKED) {
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
}

void EnableDlc(dlc civ_dlc, HWND hWnd)
{
    for (int i = 0; i < MAX_CIVS; i++) {
        if (GetCivDLC(civ_index[i]) == civ_dlc) {
            CheckDlgButton(hWnd, i + 5, BST_CHECKED);
            AddCiv(civ_index[i]);
        }
    }
}

void DisableDlc(dlc civ_dlc, HWND hWnd)
{
    for (int i = 0; i < MAX_CIVS; i++) {
        if (GetCivDLC(civ_index[i]) == civ_dlc) {
            CheckDlgButton(hWnd, i + 5, BST_UNCHECKED);
            RemoveCiv(civ_index[i]);
        }
    }
}

bool DlcEmpty(dlc civ_dlc) {
	int count = 0;
	for (int i = 0; i < MAX_CIVS; i++) {
		if (GetCivStatus(civ_index[i]) && GetCivDLC(civ_index[i]) == civ_dlc) {
			count++;
		}
	}

	if (count == 0) return true;
	else return false;
}

void ValidateDlcToggle(HWND hWnD, dlc civ_dlc) {
	if (!DlcEmpty(civ_dlc)) {
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

void UpdateDrawnLog(bool start_state, bool drawn, bool blankline_wanted) {

    
    if (drawn)
    {
        drawnlog_length = GetWindowTextLength(drawn_log);
        drawnlog_text.resize(drawnlog_length + 1);
        GetWindowText(drawn_log, &drawnlog_text[0], drawnlog_length + 1);
        drawnlog_text.pop_back(); // Remove the null terminator
        log_entry = std::to_wstring(iterator) + L"/" + std::to_wstring(custom_max_civs) + L" - " + std::wstring(current_civ.begin(), current_civ.end()) + L"\r\n";
        drawnlog_text = log_entry + drawnlog_text;
        if (iterator == custom_max_civs) drawnlog_text += L"\r\n";
        SetWindowText(drawn_log, drawnlog_text.c_str());
        if (!start_state || start_state && GetCivStatus(current_civ)) drawn_civs[iterator-1] = current_civ;
    }

    else if (!reset_state && blankline_wanted)
    {
        if (iterator >= 0)                       // adds blank line to log before next iteration of civ drawing
        {
            drawnlog_length = GetWindowTextLength(drawn_log);
            drawnlog_text.resize(drawnlog_length + 1);
            GetWindowText(drawn_log, &drawnlog_text[0], drawnlog_length + 1);
            drawnlog_text.pop_back();

            drawnlog_text = L"\r\n" + drawnlog_text;
            SetWindowText(drawn_log, drawnlog_text.c_str());

        }        
    }


    std::wstring drawn_label = L"Drawn: " + std::to_wstring(iterator) + L"/" + std::to_wstring(custom_max_civs);
    SetWindowText(label_drawncount, drawn_label.c_str());
}

void UpdateRemainingLog(bool civ_pool_changed) {

    if (reset_state || civ_pool_changed) {
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

void OpenTechTree() {
    std::wstring techtree_path = L"https://aoe2techtree.net/#";
    if (current_civ != L"Random") techtree_path += current_civ;
	ShellExecute(NULL, L"open", techtree_path.c_str(), NULL, NULL, SW_SHOWNORMAL);
}

void AddTooltip(HWND hwndTool, HWND hwndTip, LPCWSTR pszText)
{

    if (!hwndTool || !hwndTip) {
        MessageBox(NULL, L"Invalid handle(s) provided to AddTooltip", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    TOOLINFO toolInfo = { 0 };
    toolInfo.cbSize = sizeof(toolInfo);
    toolInfo.hwnd = hwndTool;
    toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS | TTF_TRACK;
    toolInfo.uId = (UINT_PTR)hwndTool;
    toolInfo.lpszText = (LPWSTR)pszText;
    toolInfo.rect.left = 0;
    toolInfo.rect.top = 0;
    toolInfo.rect.right = 0;
    toolInfo.rect.bottom = 0;



    if (!SendMessage(hwndTip, TTM_ADDTOOL, 0, (LPARAM)&toolInfo)) {
		MessageBox(NULL, L"Failed to add tooltip", L"Error", MB_OK);
        OutputDebugString(L"Failed to add tooltip\n");
    }
}

void ActivateTooltip(HWND hwndTip, TOOLINFO *toolInfo, POINT pt)
{



    // Convert client coordinates to screen coordinates
    ClientToScreen(toolInfo->hwnd, &pt);

    // Set the tooltip position to be next to the mouse cursor
    SendMessage(hwndTip, TTM_TRACKPOSITION, 0, (LPARAM)MAKELONG(pt.x + 10, pt.y + 20));



    if (!SendMessage(hwndTip, TTM_TRACKACTIVATE, TRUE, (LPARAM)toolInfo)) {
        OutputDebugString(L"Failed to activate tooltip\n");
    }
}

LPCWSTR StringCleaner(const std::wstring &dirty_string)
{
	LPCWSTR clean_string = const_cast<LPCWSTR>(dirty_string.c_str());
    return clean_string;
}

void UpdateTooltipText(HWND hwndTool, HWND hwndTip, LPCWSTR newText)
{
    if (!hwndTool || !hwndTip) {
        MessageBox(NULL, L"Invalid handle(s) provided to UpdateTooltipText", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    TOOLINFO toolInfo = { 0 };
    toolInfo.cbSize = sizeof(toolInfo);
    toolInfo.hwnd = hwndTool;
    toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS | TTF_TRACK;
    toolInfo.uId = (UINT_PTR)hwndTool;
    toolInfo.lpszText = (LPWSTR)newText;

    // Update the tooltip text
    SendMessage(hwndTip, TTM_UPDATETIPTEXT, 0, (LPARAM)&toolInfo);
}

void SetEditionState(HWND hWnd, edition edition)
{
    if (ui_sounds_enabled && !startup) PlaySound(L"view_sound.wav", NULL, SND_FILENAME | SND_ASYNC);
    switch (edition)
    {
        case DE:
        {
            ShowDEDLCCheckboxes(true);
            ShowHDDLCCheckboxes(false);
            ShowAOCCheckbox(false);
            SendMessageW(edition_icon, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)icon_de);
            edition_state = DE;
            ShowAllPoolCheckboxes();
            ValidateAllDlcToggles(hWnd);
            if (autotoggle_enabled && !startup)
            {
                for (int i = 0; i < 5; i++)
                {
                    EnableDlc(old_dlc[i], hWnd);
                }
            }
            if ((SendMessage(radiobutton_de, BM_GETCHECK, 0, 0)) != BST_CHECKED)
            {
                SendMessage(radiobutton_de, BM_SETCHECK, BST_CHECKED, 0);
                SendMessage(radiobutton_hd, BM_SETCHECK, BST_UNCHECKED, 0);
                SendMessage(radiobutton_aok, BM_SETCHECK, BST_UNCHECKED, 0);
            }
            return;
        }
        case HD:
        {
            ShowDEDLCCheckboxes(false);
            ShowHDDLCCheckboxes(true);
            ShowAOCCheckbox(false);
            if (autotoggle_enabled && !startup) {
                for (int i = 0; i < 2; i++) {
                    EnableDlc(old_dlc[i], hWnd);
                }
            }
            SendMessageW(edition_icon, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)icon_hd);
            edition_state = HD;
            ShowHDPoolCheckboxes();
            ValidateAllDlcToggles(hWnd);
            if ((SendMessage(radiobutton_hd, BM_GETCHECK, 0, 0)) != BST_CHECKED)
            {
				SendMessage(radiobutton_hd, BM_SETCHECK, BST_CHECKED, 0);
				SendMessage(radiobutton_de, BM_SETCHECK, BST_UNCHECKED, 0);
				SendMessage(radiobutton_aok, BM_SETCHECK, BST_UNCHECKED, 0);
            }
            return;
        }
        case AOK:
        {
            ShowDEDLCCheckboxes(false);
            ShowHDDLCCheckboxes(false);
            ShowAOCCheckbox(true);
            if (autotoggle_enabled && !startup) {
                EnableDlc(aok, hWnd);
            }
            SendMessageW(edition_icon, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)icon_aok);
            edition_state = AOK;
            ShowAOCPoolCheckboxes();
            ValidateAllDlcToggles(hWnd);
            if ((SendMessage(radiobutton_aok, BM_GETCHECK, 0, 0)) != BST_CHECKED)
            {
                SendMessage(radiobutton_de, BM_SETCHECK, BST_UNCHECKED, 0);
                SendMessage(radiobutton_hd, BM_SETCHECK, BST_UNCHECKED, 0);
                SendMessage(radiobutton_aok, BM_SETCHECK, BST_CHECKED, 0);
            }
            return;
        }
    }
        
}
       
void ToggleRemainLog()
{	
    if (remainlog_enabled)
    {
        ShowWindow(remaining_log, SW_HIDE);
        remainlog_enabled = false;
        if (hotkey_pressed)
        {
            if ((SendMessage(checkbox_showremainlog, BM_GETCHECK, 0, 0)) != BST_UNCHECKED)
            {
                SendMessage(checkbox_showremainlog, BM_SETCHECK, BST_UNCHECKED, 0);
            }
            else
            {
                SendMessage(checkbox_showremainlog, BM_SETCHECK, BST_CHECKED, 0);
            }
        }
        
    }

	else
	{
		ShowWindow(remaining_log, SW_SHOW);
		remainlog_enabled = true;

        if (hotkey_pressed)
        {
            if ((SendMessage(checkbox_showremainlog, BM_GETCHECK, 0, 0)) != BST_CHECKED)
            {
                SendMessage(checkbox_showremainlog, BM_SETCHECK, BST_CHECKED, 0);
            }
            else
            {
                SendMessage(checkbox_showremainlog, BM_SETCHECK, BST_UNCHECKED, 0);
            }
        }
        
	}

    if (ui_sounds_enabled) PlaySound(L"button_sound.wav", NULL, SND_FILENAME | SND_ASYNC);
}

int GetDlcCheckboxId(dlc dlc)
{
    switch (dlc)
    {
    case royals:
		return IDC_CHECKBOX_ROYALS;
	case khans:
		return IDC_CHECKBOX_KHANS;
	case dukes:
		return IDC_CHECKBOX_DUKES;
	case west:
		return IDC_CHECKBOX_WEST;
	case india:
		return IDC_CHECKBOX_INDIA;
	case rome:
		return IDC_CHECKBOX_ROME;
	case forgotten:
		return IDC_CHECKBOX_FORGOTTEN;
	case africans:
		return IDC_CHECKBOX_AFRICANS;
	case rajas:
		return IDC_CHECKBOX_RAJAS;
	case aoc:
		return IDC_CHECKBOX_AOC;
    }
}

void ToggleAutoToggle(HWND hWnd)
{

    if (hotkey_pressed)
    {
        if (IsDlgButtonChecked(hWnd, IDC_CHECKBOX_AUTOTOGGLE) == BST_CHECKED)
        {
			CheckDlgButton(hWnd, IDC_CHECKBOX_AUTOTOGGLE, BST_UNCHECKED);
		}
        else
        {
            CheckDlgButton(hWnd, IDC_CHECKBOX_AUTOTOGGLE, BST_CHECKED);
        }
    }

    if (IsDlgButtonChecked(hWnd, IDC_CHECKBOX_AUTOTOGGLE) == BST_CHECKED) {
        autotoggle_enabled = true;
        switch (edition_state) {
        case DE:
            for (int i = 0; i < 5; i++) {
                EnableDlc(old_dlc[i], hWnd);
            }
            break;
        case HD:
            for (int i = 0; i < 2; i++) {
                EnableDlc(old_dlc[i], hWnd);
            }
            break;
        case AOK:
            EnableDlc(aok, hWnd);
            break;
        }
    }
    else if (IsDlgButtonChecked(hWnd, IDC_CHECKBOX_AUTOTOGGLE) == BST_UNCHECKED) {
        autotoggle_enabled = false;
        switch (edition_state) {
        case DE:
            for (int i = 0; i < 5; i++) {
                DisableDlc(old_dlc[i], hWnd);
            }
            break;
        case HD:
            for (int i = 0; i < 2; i++) {
                DisableDlc(old_dlc[i], hWnd);
            }
            break;
        case AOK:
            DisableDlc(aok, hWnd);
            break;
        }

    }


}

void ToggleAutoReset(HWND hWnd)
{

	if (hotkey_pressed)
	{
		if (IsDlgButtonChecked(hWnd, IDC_CHECKBOX_AUTORESET) == BST_CHECKED)
		{
			CheckDlgButton(hWnd, IDC_CHECKBOX_AUTORESET, BST_UNCHECKED);
		}
		else
		{
			CheckDlgButton(hWnd, IDC_CHECKBOX_AUTORESET, BST_CHECKED);
		}
	}

    if (IsDlgButtonChecked(hWnd, IDC_CHECKBOX_AUTORESET) == BST_CHECKED) autoreset_enabled = true;
    else if (IsDlgButtonChecked(hWnd, IDC_CHECKBOX_AUTORESET) == BST_UNCHECKED) autoreset_enabled = false;
}

void SaveSettings()
{
    WritePrivateProfileString(L"Settings", L"PersistentLogging", persistent_logging ? L"1" : L"0", INI_FILE_PATH);
	WritePrivateProfileString(L"Settings", L"DrawOnStartup", draw_on_startup ? L"1" : L"0", INI_FILE_PATH);
    WritePrivateProfileString(L"Settings", L"UISoundsEnabled", ui_sounds_enabled ? L"1" : L"0", INI_FILE_PATH);
    WritePrivateProfileString(L"Settings", L"LabelsEnabled", labels_enabled ? L"1" : L"0", INI_FILE_PATH);
    WritePrivateProfileString(L"Settings", L"IconsEnabled", icons_enabled ? L"1" : L"0", INI_FILE_PATH);
    WritePrivateProfileString(L"Settings", L"JinglesEnabled", jingles_enabled ? L"1" : L"0", INI_FILE_PATH);
    WritePrivateProfileString(L"Settings", L"TooltipsEnabled", tooltips_enabled ? L"1" : L"0", INI_FILE_PATH);
    WritePrivateProfileString(L"Settings", L"RemainLogEnabled", remainlog_enabled ? L"1" : L"0", INI_FILE_PATH);
    WritePrivateProfileString(L"Settings", L"AutoResetEnabled", autoreset_enabled ? L"1" : L"0", INI_FILE_PATH);
    WritePrivateProfileString(L"Settings", L"AutoToggleEnabled", autotoggle_enabled ? L"1" : L"0", INI_FILE_PATH);
    WritePrivateProfileString(L"Settings", L"LegacyJingleEnabled", legacy_jingle_enabled ? L"1" : L"0", INI_FILE_PATH);

}

void LoadSettings()
{
    // Load boolean settings
	persistent_logging = GetPrivateProfileInt(L"Settings", L"PersistentLogging", 1, INI_FILE_PATH);
	draw_on_startup = GetPrivateProfileInt(L"Settings", L"DrawOnStartup", 1, INI_FILE_PATH);
    ui_sounds_enabled = GetPrivateProfileInt(L"Settings", L"UISoundsEnabled", 1, INI_FILE_PATH);
    labels_enabled = GetPrivateProfileInt(L"Settings", L"LabelsEnabled", 1, INI_FILE_PATH);
    icons_enabled = GetPrivateProfileInt(L"Settings", L"IconsEnabled", 1, INI_FILE_PATH);
    jingles_enabled = GetPrivateProfileInt(L"Settings", L"JinglesEnabled", 1, INI_FILE_PATH);
    tooltips_enabled = GetPrivateProfileInt(L"Settings", L"TooltipsEnabled", 1, INI_FILE_PATH);
    remainlog_enabled = GetPrivateProfileInt(L"Settings", L"RemainLogEnabled", 1, INI_FILE_PATH);
    autoreset_enabled = GetPrivateProfileInt(L"Settings", L"AutoResetEnabled", 0, INI_FILE_PATH);
    autotoggle_enabled = GetPrivateProfileInt(L"Settings", L"AutoToggleEnabled", 0, INI_FILE_PATH);
    legacy_jingle_enabled = GetPrivateProfileInt(L"Settings", L"LegacyJingleEnabled", 0, INI_FILE_PATH);

}

void SaveLog()
{
    std::wofstream outFile(LOG_FILE_PATH);
    if (!outFile)
    {
        return;
    }

// Save the civilization states
    outFile << L"CivStates:" << std::endl;
    for (const auto &civ : civ_enabled)
    {
        outFile << civ.first << L" " << (civ.second ? L"1" : L"0") << std::endl;
    }

    // Save the drawn civilisations
    outFile << L"DrawnCivs:" << std::endl;
	for (int i = 0; i < MAX_CIVS; i++)
	{
        if (drawn_civs[i] != L"")
        {
            outFile << drawn_civs[i] << std::endl;
        }
        else break;
	}

    outFile << L"EditionState:" << std::endl;
    switch (edition_state)
    {
	case DE:
		outFile << L"DE" << std::endl;
		break;
	case HD:
		outFile << L"HD" << std::endl;
		break;
	case AOK:
		outFile << L"AOK" << std::endl;
		break;
    }

    

    outFile.close();
}

void LoadLog(HWND hWnd)
{
    std::wifstream inFile(LOG_FILE_PATH);

    if (!inFile) return;

    std::wstring line;
    bool readingDrawnCivs = false;
    bool readingCivStates = false;
	bool readingEditionState = false;
    InitialiseCivs();

    while (std::getline(inFile, line))
    {

        if (line == L"CivStates:")
        {
            readingCivStates = true;
            readingDrawnCivs = false;
			readingEditionState = false;
            continue;
        }
        else if (line == L"DrawnCivs:")
        {
            readingCivStates = false;
            readingDrawnCivs = true;
			readingEditionState = false;
            continue;
        }
        else if (line == L"EditionState:")
        {
            readingCivStates = false;
            readingDrawnCivs = false;
            readingEditionState = true;
            continue;
        }
        if (readingCivStates)
        {
            std::wistringstream iss(line);
            std::wstring civName;
            int state;
            if (iss >> civName >> state)
            {
                if (state == 0) {
                    RemoveCiv(civName);
                    SendMessage(GetCivCheckbox(civName), BM_SETCHECK, BST_UNCHECKED, 0);
                }

                else {
                    SendMessage(GetCivCheckbox(civName), BM_SETCHECK, BST_CHECKED, 0);
                }
            }
        }
        else if (readingDrawnCivs)
        {
            civs.erase(std::remove(civs.begin(), civs.end(), line), civs.end());
            current_civ = line;
            if (GetCivStatus(current_civ)) iterator++;
            UpdateDrawnLog(true, true, false);       
		}
		else if (readingEditionState)
		{
			if (line == L"DE")
			{
				SetEditionState(hWnd, DE);
			}
			else if (line == L"HD")
			{
				SetEditionState(hWnd, HD);
			}
			else if (line == L"AOK")
			{
				SetEditionState(hWnd, AOK);
			}
		}
        
    }


    // Update the labels
    label_text = std::to_wstring(iterator) + L"/" + std::to_wstring(custom_max_civs);
    std::wstring remain_text = std::to_wstring(custom_max_civs - iterator) + L"/" + std::to_wstring(custom_max_civs);
    SetWindowText(label_corner, label_text.c_str());
    SetWindowText(label_centre, current_civ.c_str());


    HBITMAP drawn_civ_icon = FetchIcon(current_civ);
    SendMessageW(civ_icon, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)drawn_civ_icon);



    reset_state = true;
    UpdateRemainingLog(false);

    UpdateTooltipText(button_techtree, hwndTooltip[TOOLTIP_TECHTREE], StringCleaner(L"Opens the tech tree for the " + current_civ + L"\nHotkey: T"));



    
    inFile.close();
}

void GenerateFilePaths()
{
    // Get the path of the executable
    wchar_t exePathS[MAX_PATH];
    wchar_t exePathL[MAX_PATH];
    GetModuleFileName(NULL, exePathS, MAX_PATH);
    GetModuleFileName(NULL, exePathL, MAX_PATH);

    // Remove the executable name from the path
    wchar_t *lastSlashS = wcsrchr(exePathS, L'\\');
    if (lastSlashS != NULL)
    {
        *lastSlashS = L'\0';
    }
    wchar_t *lastSlashL = wcsrchr(exePathL, L'\\');
    if (lastSlashL != NULL)
    {
        *lastSlashL = L'\0';
    }
    
    wcscat_s(exePathS, MAX_PATH, L"\\settings.ini");
    wcscat_s(exePathL, MAX_PATH, L"\\log.txt");


    wcscpy_s(INI_FILE_PATH, exePathS);
    wcscpy_s(LOG_FILE_PATH, exePathL);
}

HWND GetCivCheckbox(const std::wstring &civ_name)
{
	for (int i = 0; i < MAX_CIVS; i++)
	{
		if (civ_index[i] == civ_name)
		{
			return civ_checkbox[i];
		}
	}
	return NULL;
}

void InitialiseCivs()
{
    civs = { L"Armenians", L"Aztecs", L"Bengalis", L"Berbers", L"Bohemians", L"Britons",
                L"Bulgarians", L"Burgundians", L"Burmese", L"Byzantines", L"Celts", L"Chinese",
                L"Cumans", L"Dravidians", L"Ethiopians", L"Franks", L"Georgians", L"Goths", L"Gurjaras",
                L"Hindustanis", L"Huns", L"Incas", L"Italians", L"Japanese", L"Khmer", L"Koreans",
                L"Lithuanians", L"Magyars", L"Malay", L"Malians", L"Mayans", L"Mongols", L"Persians",
                L"Poles", L"Portuguese", L"Romans", L"Saracens", L"Sicilians", L"Slavs", L"Spanish",
                L"Tatars", L"Teutons", L"Turks", L"Vietnamese", L"Vikings" };
}

void MuteSounds()
{
    PlaySound(L"mute.wav", NULL, SND_FILENAME | SND_ASYNC);
}

void OpenOptions(HWND hWnd)
{
    if (ui_sounds_enabled) PlaySound(L"button_sound.wav", NULL, SND_FILENAME | SND_ASYNC);
    DialogBox(instance, MAKEINTRESOURCE(IDD_OPTIONS), hWnd, OptionsDlgProc);    
}

void OpenHotkeys(HWND hWnd)
{
    if (ui_sounds_enabled) PlaySound(L"button_sound.wav", NULL, SND_FILENAME | SND_ASYNC);
	DialogBox(instance, MAKEINTRESOURCE(IDD_HOTKEYS), hWnd, HotkeysDlgProc);	
}



INT_PTR CALLBACK HotkeysDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

{
    switch (message)
    {
        case WM_INITDIALOG:
        {
            return(INT_PTR)TRUE;
        }
        case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            int wmEvent = HIWORD(wParam);

            if (wmId == IDCANCEL || wmId == IDOK) {
                EndDialog(hDlg, wmId);
                return(INT_PTR)TRUE;
            }
            break;

        }
    }    
    return (INT_PTR)FALSE;
}


void SubclassButtons()
{
    SubclassButton(button_draw);
    SubclassButton(button_reset);
    SubclassButton(button_clearlog);
    SubclassButton(checkbox_showremainlog);
    SubclassButton(button_techtree);
	SubclassButton(button_options);

    SubclassButton(button_enableall);
    SubclassButton(button_disableall);

    SubclassButton(radiobutton_de);
    SubclassButton(radiobutton_hd);
    SubclassButton(radiobutton_aok);

    SubclassButton(checkbox_autoreset);
    SubclassButton(checkbox_autotoggle);

    SubclassButton(checkbox_royals);
    SubclassButton(checkbox_rome);
    SubclassButton(checkbox_india);
    SubclassButton(checkbox_dukes);
    SubclassButton(checkbox_west);
    SubclassButton(checkbox_khans);

    SubclassButton(checkbox_rajas);
    SubclassButton(checkbox_africans);
    SubclassButton(checkbox_forgotten);

    SubclassButton(checkbox_aoc);
}

void CreateTooltips(HWND hWnd)
{
    for (int i = 0; i < hwnd_length; i++)
    {
        hwndTooltip[i] = CreateWindowEx(0, TOOLTIPS_CLASS, NULL,
            WS_POPUP | TTS_ALWAYSTIP | TTS_NOPREFIX,
            CW_USEDEFAULT, CW_USEDEFAULT,
            CW_USEDEFAULT, CW_USEDEFAULT,
            hWnd, NULL, instance, NULL);
    }

    if (!hwndTooltip) {
        MessageBox(hWnd, L"Failed to create tooltip window.", L"Error", MB_OK | MB_ICONERROR);
        return;
    }
    for (int i = 0; i < hwnd_length; i++)
    {
        SendMessage(hwndTooltip[i], TTM_SETMAXTIPWIDTH, 0, 300);
    }
}

void AddTooltips()
{
    AddTooltip(button_draw, hwndTooltip[TOOLTIP_DRAW], StringCleaner(L"Draws a fresh random civilisation\nHotkey: Space"));
    AddTooltip(button_reset, hwndTooltip[TOOLTIP_RESET], StringCleaner(L"Resets the pool of drawn civs and renders all enabled civs available\nHotkey: Enter"));
    AddTooltip(button_options, hwndTooltip[TOOLTIP_OPTIONS], StringCleaner(L"Opens options\nHotkey: Q"));
    AddTooltip(button_techtree, hwndTooltip[TOOLTIP_TECHTREE], StringCleaner(L"Opens the tech tree\nHotkey: T"));

    AddTooltip(button_clearlog, hwndTooltip[TOOLTIP_CLEAR], StringCleaner(L"Clears the log of previously drawn civs\nHotkey: Q"));
    AddTooltip(checkbox_showremainlog, hwndTooltip[TOOLTIP_REMAININGTOGGLE], StringCleaner(L"Toggles the display of the remaining civs log\nHotkey: W"));

    AddTooltip(checkbox_autotoggle, hwndTooltip[TOOLTIP_AUTOTOGGLE], StringCleaner(L"Toggles older civs as well as the automatic enabling of older civs when switching between editions\nHotkey: R"));
    AddTooltip(checkbox_autoreset, hwndTooltip[TOOLTIP_AUTORESET], StringCleaner(L"Automatically resets the pool of drawn civs when the civilisation pool is changed\nHotkey: T"));

    AddTooltip(button_enableall, hwndTooltip[TOOLTIP_ENABLEALL], StringCleaner(L"Enables all civilisations so that they are made available in the pool of civilisations for drawing\nHotkey: Space"));
    AddTooltip(button_disableall, hwndTooltip[TOOLTIP_DISABLEALL], StringCleaner(L"Disables all civilisations so that they are removed from the pool of civilisations for drawing\nHotkey: Enter"));

    AddTooltip(radiobutton_de, hwndTooltip[TOOLTIP_DE], StringCleaner(L"Selects the Definitive Edition (2019) civilisation pool\nHotkey: Q"));
    AddTooltip(radiobutton_hd, hwndTooltip[TOOLTIP_HD], StringCleaner(L"Selects the HD Edition (2013) civilisation pool\nHotkey: W"));
    AddTooltip(radiobutton_aok, hwndTooltip[TOOLTIP_AOK], StringCleaner(L"Selects the Age of Kings (1999) civilisation pool\nHotkey: E"));

    AddTooltip(checkbox_royals, hwndTooltip[TOOLTIP_ROYALS], StringCleaner(L"Toggles The Mountain Royals civilisations\n(Armenians, Georgians)\nHotkey: A"));
    AddTooltip(checkbox_rome, hwndTooltip[TOOLTIP_ROME], StringCleaner(L"Toggles Return of Rome civilisation\n(Romans)\nHotkey: S"));
    AddTooltip(checkbox_india, hwndTooltip[TOOLTIP_INDIA], StringCleaner(L"Toggles Dynasties of India civilisations\n(Bengalis, Dravidians, Gurjaras)\nHotkey: D"));
    AddTooltip(checkbox_dukes, hwndTooltip[TOOLTIP_DUKES], StringCleaner(L"Toggles Dawn of the Dukes civilisations\n(Bohemians, Poles)\nHotkey: F"));
    AddTooltip(checkbox_west, hwndTooltip[TOOLTIP_WEST], StringCleaner(L"Toggles Lords of the West civilisations\n(Burgundians, Sicilians)\nHotkey: G"));
    AddTooltip(checkbox_khans, hwndTooltip[TOOLTIP_KHANS], StringCleaner(L"Toggles The Last Khans civilisations\n(Bulgarians, Cumans, Lithuanians, Tatars)\nHotkey: H"));

    AddTooltip(checkbox_rajas, hwndTooltip[TOOLTIP_RAJAS], StringCleaner(L"Toggles Rise of Rajas civilisations\n(Burmese, Khmer, Malay, Vietnamese)\nHotkey: A"));
    AddTooltip(checkbox_africans, hwndTooltip[TOOLTIP_AFRICANS], StringCleaner(L"Toggles African Kingdoms civilisations\n(Berbers, Ethiopians, Malians, Portuguese)\nHotkey: S"));
    AddTooltip(checkbox_forgotten, hwndTooltip[TOOLTIP_FORGOTTEN], StringCleaner(L"Toggles The Forgotten civilisations\n(Incas, Hindustanis, Italians, Magyars, Slavs)\nHotkey: D"));

    AddTooltip(checkbox_aoc, hwndTooltip[TOOLTIP_AOC], StringCleaner(L"Toggles The Conquerors civilisations\n(Aztecs, Huns, Koreans, Mayans, Spanish)\nHotkey: A"));

}



void CreateCheckboxes(HWND hWnd)
{
    checkbox_showremainlog = CreateCheckbox(hWnd, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), 10, 230, 180, 20, IDC_CHECKBOX_REMAINLOG, L"Show");
    CheckDlgButton(hWnd, IDC_CHECKBOX_REMAINLOG, remainlog_enabled ? BST_CHECKED : BST_UNCHECKED);

    int row[] = { 30, 50, 70, 90, 110, 130, 150, 170, 190 };
    int column[] = { 10, 112, 214, 316, 418 };

    for (int i = 0; i < MAX_CIVS; i++)
    {
        civ_checkbox[i] = CreateCheckbox(hWnd, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), column[i % 5], row[i / 5], 100, 20, i + 5, civ_index[i].c_str());
    }

    checkbox_autoreset = CreateCheckbox(hWnd, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), 310, 230, 180, 20, IDC_CHECKBOX_AUTORESET, L"Auto-reset upon change");
    checkbox_autotoggle = CreateCheckbox(hWnd, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), 10, 0, 170, 20, IDC_CHECKBOX_AUTOTOGGLE, L"Auto-toggle older civs");

    if (!autoreset_enabled) CheckDlgButton(hWnd, IDC_CHECKBOX_AUTORESET, BST_UNCHECKED);
    if (!autotoggle_enabled) CheckDlgButton(hWnd, IDC_CHECKBOX_AUTOTOGGLE, BST_UNCHECKED);


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
}

void CreateImages(HWND hWnd)
{
    civ_icon = CreateWindow(L"BUTTON", L"", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | BS_BITMAP, 0, 0, 60, 60, hWnd, (HMENU)IDC_ICON_CIV, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);

    edition_icon = CreateWindowW(L"BUTTON", L"", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | BS_BITMAP, 0, 0, 60, 60, hWnd, (HMENU)IDC_ICON_EDITION, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);

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
}

void CreateButtons(HWND hWnd)
{
    button_draw = CreateWindow(L"BUTTON", L"Draw", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | TTF_TRACK, 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT, hWnd, (HMENU)IDC_BUTTON_DRAW, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
    button_reset = CreateWindow(L"BUTTON", L"Reset", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 10, 200, BUTTON_WIDTH, BUTTON_HEIGHT, hWnd, (HMENU)IDC_BUTTON_RESET, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
    button_techtree = CreateWindow(L"BUTTON", L"", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | BS_BITMAP, 0, 0, 60, 60, hWnd, (HMENU)IDC_BUTTON_TECHTREE, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
    SendMessageW(button_techtree, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)icon_techtree);
    button_options = CreateWindow(L"BUTTON", L"", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | BS_BITMAP, 89, 85, 60, 60, hWnd, (HMENU)IDC_BUTTON_OPTIONS, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
    //SendMessageW(button_techtree, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)icon_techtree); 
    button_clearlog = CreateWindow(L"BUTTON", L"Clear Log", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT, hWnd, (HMENU)IDC_BUTTON_CLEARLOG, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
    button_enableall = CreateWindow(L"BUTTON", L"Enable All", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 10, 340, BUTTON_WIDTH, BUTTON_HEIGHT, hWnd, (HMENU)IDC_BUTTON_ENABLEALL, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
    button_disableall = CreateWindow(L"BUTTON", L"Disable All", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 125, 340, BUTTON_WIDTH, BUTTON_HEIGHT, hWnd, (HMENU)IDC_BUTTON_DISABLEALL, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
}

void CreateLabels(HWND hWnd)
{
    label_corner = CreateWindow(L"STATIC", L"", WS_VISIBLE | WS_CHILD, 0, 0, 30, 15, tab, NULL, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
    label_centre = CreateWindow(L"STATIC", L"?", WS_VISIBLE | WS_CHILD, 0, 0, 100, 15, hWnd, NULL, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
    label_drawncount = CreateWindow(L"STATIC", L"", WS_VISIBLE | WS_CHILD, 100, 25, 100, 15, hWnd, NULL, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
    label_remainingcount = CreateWindow(
        L"STATIC", L"", WS_VISIBLE | WS_CHILD, 0, 25, 100, 15, hWnd, NULL, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
}

void CreateTextfields(HWND hWnd)
{
    drawn_log = CreateWindow(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY, 10, 25, 350, 200, tab, NULL, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
    remaining_log = CreateWindow(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY, 10, 25, 350, 200, tab, NULL, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
}

void CreateRadiobuttons(HWND hWnd)
{
    radiobutton_de = CreateWindow(L"BUTTON", L"Definitive Edition (2019)", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON, 10, 30, 175, 20, hWnd, (HMENU)IDC_RADIO_DE, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
    radiobutton_hd = CreateWindow(L"BUTTON", L"HD Edition (2013)", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON, 10, 60, 175, 20, hWnd, (HMENU)IDC_RADIO_HD, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
    radiobutton_aok = CreateWindow(L"BUTTON", L"Age of Kings (1999)", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON, 10, 90, 175, 20, hWnd, (HMENU)IDC_RADIO_AOK, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
    CheckRadioButton(hWnd, IDC_RADIO_DE, IDC_RADIO_AOK, IDC_RADIO_DE);
}
