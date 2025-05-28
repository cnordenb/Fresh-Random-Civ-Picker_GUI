#include "FRCP_GUI.h"

void CreateTabs(HWND hWnd)
{
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_TAB_CLASSES;
    InitCommonControlsEx(&icex);

    tab = CreateWindow(WC_TABCONTROL, L"", WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE, 0, 0, MIN_WIDTH, MIN_HEIGHT, hWnd, NULL, instance, NULL);

    TCITEM tie;
    tie.mask = TCIF_TEXT;

    tie.pszText = const_cast<LPWSTR>(L"Draw Civ");
    TabCtrl_InsertItem(tab, 0, &tie);

    tie.pszText = const_cast<LPWSTR>(L"Log");
    TabCtrl_InsertItem(tab, 1, &tie);

    tie.pszText = const_cast<LPWSTR>(L"Civ Pool");
    TabCtrl_InsertItem(tab, 2, &tie);    
}

void ShowTabComponents(int tabIndex, HWND hWnd)
{
    if (!startup) PlayAudio(tabsound);
    current_tab = tabIndex;
    if (tabIndex == 0)
    {        
		ShowLogTab(false);
		ShowCustomTab(false);
        ShowDrawTab(true, hWnd);
    }
    else if (tabIndex == 1)
    {        
		ShowDrawTab(false, hWnd);
		ShowCustomTab(false);
        ShowLogTab(true);
    }
    else if (tabIndex == 2)
    {        
		ShowDrawTab(false, hWnd);
		ShowLogTab(false);
		ShowCustomTab(true);
    }
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{    
    LoadSounds();
    LoadJingles();


    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    GenerateFilePaths();
    LoadSettings();    

    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    CreateUnderlineFont();
	InitialiseCivStates();

    LoadStringW(hInstance, IDS_APP_TITLE, title, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WINDOWSPROJECT1, window_class, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance (hInstance, nCmdShow)) return FALSE;

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINDOWSPROJECT1));

    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    Gdiplus::GdiplusShutdown(gdiplusToken);

    DeleteObject(font_underline);
    DeleteObject(font_bold);

    if (persistent_logging) SaveLog(automatic);
    SaveSettings();

    UnloadSounds();
    UnloadJingles();

    return (int) msg.wParam;
}

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

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    instance = hInstance;

	wcscpy_s(title, L"Fresh Random Civ Picker");

    HWND hWnd = CreateWindowW(window_class, title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, MIN_WIDTH, MIN_HEIGHT, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd) return FALSE;    

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    
    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    TOOLINFO toolInfo = { 0 };
    toolInfo.cbSize = sizeof(toolInfo);
    toolInfo.hwnd = hWnd;
    toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS | TTF_TRACK;
    switch (message)
    {  
        case WM_DESTROY: PostQuitMessage(0); break;
        case WM_CREATE:
        {
            startup = true;
            INITCOMMONCONTROLSEX iccex;
            iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
            iccex.dwICC = ICC_WIN95_CLASSES | ICC_BAR_CLASSES | ICC_COOL_CLASSES | ICC_TREEVIEW_CLASSES;
            InitCommonControlsEx(&iccex);
            if (!InitCommonControlsEx(&iccex))
            {
                MessageBox(hWnd, L"Failed to initialize common controls.", L"Error", MB_OK | MB_ICONERROR);
                return -1;
            }
            for (int i = 0; i < MAX_CIVS; ++i) civ_name_to_index[civ[i].name] = i;            
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
            if (persistent_logging) {LoadLog(hWnd, automatic); ValidateAllDlcToggles(hWnd);}           
			ShowTabComponents(0, hWnd);            
            EnableHotkeys(hWnd);
            if (draw_on_startup)
            {
                DrawCiv(false, L"");
				if (jingles_enabled) PlayJingle(current_civ);
            }
			else if (!persistent_logging) ResetProgram(false);      
            startup = false;
            break;
        }
        case WM_SIZE:
        {
            PositionComponents(lParam);
            break;
        }
        case WM_ACTIVATE:
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
                if (pnmhdr->code == TCN_SELCHANGE)
                {
                    int tabIndex = TabCtrl_GetCurSel(tab);
                    ShowTabComponents(tabIndex, hWnd);
                    break;
                }
            }
            if (pnmhdr->code == TTN_SHOW)
            {
                LPNMTTDISPINFO lpttd = (LPNMTTDISPINFO)lParam;

                POINT pt;
                GetCursorPos(&pt);

                SendMessage(pnmhdr->hwndFrom, TTM_TRACKPOSITION, 0, (LPARAM)MAKELONG(pt.x + 10, pt.y + 10));
                return TRUE;
            }
            break;
        }
        case WM_MOUSEMOVE:
        {
            if (hwndTooltip)
            {
                POINT pt;
                GetCursorPos(&pt);

                ScreenToClient(hWnd, &pt);

                RECT rect;

                bool tooltipActivated = false;
                HWND button[] = { button_draw, button_reset, button_techtree, button_options, button_clearlog,
                                    button_enableall, button_disableall, checkbox_showremainlog,
                                    radiobutton_de, radiobutton_hd, radiobutton_aok, de_dlc_checkbox[0],
                                de_dlc_checkbox[1], de_dlc_checkbox[2], de_dlc_checkbox[3], de_dlc_checkbox[4], de_dlc_checkbox[5], de_dlc_checkbox[6],
                                hd_dlc_checkbox[0], hd_dlc_checkbox[1], hd_dlc_checkbox[2], checkbox_aoc,
                                checkbox_autotoggle, checkbox_autoreset, button_survapp, button_history };

                int tooltip_id[] = { TOOLTIP_DRAW, TOOLTIP_RESET, TOOLTIP_TECHTREE, TOOLTIP_OPTIONS, TOOLTIP_CLEAR,
                                        TOOLTIP_ENABLEALL, TOOLTIP_DISABLEALL, TOOLTIP_REMAININGTOGGLE,
                                        TOOLTIP_DE, TOOLTIP_HD, TOOLTIP_AOK, de_dlc_tipid[0], de_dlc_tipid[1],
                                    de_dlc_tipid[2], de_dlc_tipid[3], de_dlc_tipid[4], de_dlc_tipid[5], de_dlc_tipid[6],
                                    hd_dlc_tipid[0], hd_dlc_tipid[1], hd_dlc_tipid[2], TOOLTIP_AOC,
                                    TOOLTIP_AUTOTOGGLE, TOOLTIP_AUTORESET, TOOLTIP_SURVAPP, TOOLTIP_HISTORY };
                
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
                if (!tooltipActivated) DeactivateTooltips(toolInfo);
            }
            break;
        }
        case WM_GETMINMAXINFO:
        {
            MINMAXINFO *pmmi = (MINMAXINFO *)lParam;
            pmmi->ptMinTrackSize.x = MIN_WIDTH;
            pmmi->ptMinTrackSize.y = MIN_HEIGHT;
            break;
        }
        case WM_HOTKEY:
        {     
            hotkey_pressed = true;
            if (GetForegroundWindow() != hWnd)
            {
                DisableHotkeys(hWnd);
                break;
            }
            else EnableHotkeys(hWnd);

            if (wParam == HOTKEY_ID_ESC) PostQuitMessage(0);
            if (wParam == HOTKEY_ID_F1) OpenOptions(hWnd);
            if (wParam == HOTKEY_ID_F2) OpenHotkeys(hWnd);
            if (wParam == HOTKEY_ID_F3) OpenAbout(hWnd);
            if (wParam == HOTKEY_ID_F4) OpenTechTree();
            if (wParam == HOTKEY_ID_F5) SaveLog(quick);
			if (wParam == HOTKEY_ID_F9) LoadLog(hWnd, quick);
            if (wParam == HOTKEY_ID_CTRLS) SaveLog(manual);
            if (wParam == HOTKEY_ID_CTRLR) LoadLog(hWnd, manual);
			if (wParam == HOTKEY_ID_CTRLF) JoinLobby(hWnd);
            if (wParam == HOTKEY_ID_CTRLZ) Undo(hWnd);
            if (wParam == HOTKEY_ID_CTRLX) RedrawCiv();
			if (wParam == HOTKEY_ID_CTRLT) OpenSurvapp();
            if (current_tab == 0)
            {
                if (wParam == HOTKEY_ID_R) OpenSurvapp();
                if (wParam == HOTKEY_ID_Q) OpenOptions(hWnd);
                if (wParam == HOTKEY_ID_T) OpenTechTree();
                if (wParam == HOTKEY_ID_H) OpenHistory(hWnd);
            }
            if (current_tab != 2)
            {
                if (wParam == HOTKEY_ID_SPACE) DrawCiv(false, L"");
                if (wParam == HOTKEY_ID_RETURN) ResetProgram(false);
            }
            else if (current_tab == 2)
            {
                if (wParam > 1 && wParam < 4 || wParam > 12 && wParam < 22) if (autoreset_enabled) ResetProgram(true);             
                if (wParam == HOTKEY_ID_SPACE) EnableAll(hWnd, true);           
                if (wParam == HOTKEY_ID_RETURN) DisableAll(hWnd, true);
                if (wParam == HOTKEY_ID_Q) SetEditionState(hWnd, DE);
                if (wParam == HOTKEY_ID_W) SetEditionState(hWnd, HD);
				if (wParam == HOTKEY_ID_E) SetEditionState(hWnd, AOK);
                if (wParam == HOTKEY_ID_R) ToggleAutoToggle(hWnd);
                if (wParam == HOTKEY_ID_T) ToggleAutoReset(hWnd);                        
                if (edition_state == DE)
                {
                    if (wParam == HOTKEY_ID_A) ToggleDlc(kingdoms, hWnd);
					if (wParam == HOTKEY_ID_S) ToggleDlc(royals, hWnd);
					if (wParam == HOTKEY_ID_D) ToggleDlc(rome, hWnd);
					if (wParam == HOTKEY_ID_F) ToggleDlc(india, hWnd);
					if (wParam == HOTKEY_ID_G) ToggleDlc(dukes, hWnd);
					if (wParam == HOTKEY_ID_H) ToggleDlc(west, hWnd);
					if (wParam == HOTKEY_ID_J) ToggleDlc(khans, hWnd);
				}
				else if (edition_state == HD)
				{
					if (wParam == HOTKEY_ID_A) ToggleDlc(rajas, hWnd);
					if (wParam == HOTKEY_ID_S) ToggleDlc(africans, hWnd);
					if (wParam == HOTKEY_ID_D) ToggleDlc(forgotten, hWnd);
				}
                else if (edition_state == AOK) if (wParam == HOTKEY_ID_A) ToggleDlc(aoc, hWnd);                
            }
            if (current_tab == 1)
            {
                if (wParam == HOTKEY_ID_Q) ClearDrawnLog();
				if (wParam == HOTKEY_ID_W) ToggleRemainLog();
            }
            if (wParam == HOTKEY_ID_TAB)
            {
                int tabIndex = TabCtrl_GetCurSel(tab);
                int newTabIndex;
                if (tabIndex < 2) newTabIndex = tabIndex + 1;
                else newTabIndex = 0;
                TabCtrl_SetCurSel(tab, newTabIndex);
                ShowTabComponents(newTabIndex, hWnd);
            }
            if (wParam == HOTKEY_ID_1) {TabCtrl_SetCurSel(tab, 0); ShowTabComponents(0, hWnd);}
			if (wParam == HOTKEY_ID_2) {TabCtrl_SetCurSel(tab, 1); ShowTabComponents(1, hWnd);}
			if (wParam == HOTKEY_ID_3) {TabCtrl_SetCurSel(tab, 2); ShowTabComponents(2, hWnd);}
            if (wParam == HOTKEY_ID_Z)
            {
				PlayAudio(button);
                if (civ_labels_enabled)
                {
					civ_labels_enabled = false;
                    ShowWindow(label_centre, SW_HIDE);
				}
                else
                {
					civ_labels_enabled = true;
					if (current_tab != 2) ShowWindow(label_centre, SW_SHOW);
                }
            }
			if (wParam == HOTKEY_ID_X)
            {
                PlayAudio(button);
                if (icons_enabled)
                {
					icons_enabled = false;
					ShowWindow(civ_icon, SW_HIDE);
				}
				else
                {
					icons_enabled = true;
					if (current_tab != 2) ShowWindow(civ_icon, SW_SHOW);
				}
			}

            if (wParam == HOTKEY_ID_V)
            {
                if (jingles_enabled)
                {
                    jingles_enabled = false;
                    StopSound();
                }
                else
                {
                    jingles_enabled = true;
                    PlayJingle(current_civ);
                }
            }

			if (wParam == HOTKEY_ID_B)
            {
				if (ui_sounds_enabled) ui_sounds_enabled = false;				
                else
                {
                    PlayAudio(button);
                    ui_sounds_enabled = true;
                }                    				
			}             

            if (wParam == HOTKEY_ID_C)
            {
                PlayAudio(button);
                if (iteration_label_enabled)
                {
                    iteration_label_enabled = false;
                    ShowWindow(label_corner, SW_HIDE);
                }
                else
                {
                    iteration_label_enabled = true;
                    if (current_tab == 0) ShowWindow(label_corner, SW_SHOW);
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

            if (wmId >= 5 && wmId <= (MAX_CIVS + 5))
            {
				int i = wmId - 5;
                PlayAudio(button);
                if (IsDlgButtonChecked(hWnd, wmId) == BST_CHECKED) AddCiv(civ[i].name);
                else RemoveCiv(civ[i].name);
                ValidateDlcToggle(hWnd, civ[i].dlc);
            }     

            if (autoreset_enabled) if (wmId > 4 && wmId < 50 || wmId > 50 && wmId < 65) ResetProgram(true);
            
            for (int i = 0; i < EDITION_AMOUNT; i++) if (wmId == every_edition_id[i]) SetEditionState(hWnd, every_edition[i]);
            for (int i = 0; i < DLC_AMOUNT; i++) if (wmId == every_dlc_id[i]) ToggleDlc(every_dlc[i], hWnd);

            switch (wmId)
            {
                case IDM_ABOUT:
                    OpenAbout(hWnd);
                    break;
                case IDM_EXIT:
                    PostQuitMessage(0);
                    break;
                case IDM_OPTIONS:
                    OpenOptions(hWnd);
                    break;
                case IDM_SAVELOG:
                    SaveLog(manual);
                    break;
                case IDM_LOADLOG:
                    LoadLog(hWnd, manual);
                    break;
                case IDM_JOINLOBBY:
					JoinLobby(hWnd);
                    break;
                case IDM_UNDRAW:
                    UndrawCiv();
                    break;
                case IDM_REDRAW:
                    RedrawCiv();
                    break;
                case IDM_AOE2INSIGHTS:
                    PlayAudio(button);
                    ShellExecute(0, 0, L"https://www.aoe2insights.com/lobbies/", 0, 0, SW_SHOW);
                    break;
                case IDM_SPECTATORDASHBOARD:
                    PlayAudio(button);
                    ShellExecute(0, 0, L"https://aoe2recs.com/", 0, 0, SW_SHOW);
                    break;
                case IDM_HOTKEYS:
                    OpenHotkeys(hWnd);
                    break;
                case IDM_GITHUB:
                    PlayAudio(button);
                    ShellExecute(0, 0, L"https://github.com/cnordenb/Fresh-Random-Civ-Picker_CPPGUI", 0, 0, SW_SHOW);
                    break;
                case IDM_WEBVERSION:
                    PlayAudio(button);
                    ShellExecute(0, 0, L"https://cnordenb.github.io/frcp_tsx/", 0, 0, SW_SHOW);
                    break;
				case IDM_DISCORD:
                    PlayAudio(button);
                    ShellExecute(0, 0, L"https://discord.gg/rcFqrBdVUN", 0, 0, SW_SHOW);
					break;
				case IDM_DONATE:
                    PlayAudio(button);
                    ShellExecute(0, 0, L"https://ko-fi.com/hjoerleif", 0, 0, SW_SHOW);
                    break;
				case IDM_UPDATE:
                    PlayAudio(button);
                    ShellExecute(0, 0, L"https://github.com/cnordenb/Fresh-Random-Civ-Picker_GUI/releases", 0, 0, SW_SHOW);
                    break;
				case IDM_TECHTREE:
                    PlayAudio(button);
                    ShellExecute(0, 0, L"https://aoe2techtree.net/", 0, 0, SW_SHOW);
					break;
				case IDM_STATS:
                    PlayAudio(button);
                    ShellExecute(0, 0, L"https://aoestats.io/", 0, 0, SW_SHOW);
					break;
				case IDM_DESTEAM:
                    PlayAudio(button);
                    ShellExecute(0, 0, L"https://store.steampowered.com/app/813780/Age_of_Empires_II_Definitive_Edition/", 0, 0, SW_SHOW);
					break;
				case IDM_HDSTEAM:
                    PlayAudio(button);
                    ShellExecute(0, 0, L"https://store.steampowered.com/app/221380/Age_of_Empires_II_Retired/", 0, 0, SW_SHOW);
					break;
				case IDM_VOOBLY:
                    PlayAudio(button);
                    ShellExecute(0, 0, L"https://www.voobly.com/games/view/Age-of-Empires-II-The-Conquerors", 0, 0, SW_SHOW);
					break;
				case IDM_SURVAPP:
					OpenSurvapp();
					break;
                case IDC_BUTTON_HISTORY:
                    OpenHistory(hWnd);
                    break;
				case IDM_XBOX:
                    PlayAudio(button);
                    ShellExecute(0, 0, L"https://www.xbox.com/games/store/age-of-empires-ii-definitive-edition/9N42SSSX2MTG/0010", 0, 0, SW_SHOW);
					break;
                case IDC_BUTTON_DRAW:
                    DrawCiv(false, L"");
                    break;
                case IDC_BUTTON_RESET:
                    ResetProgram(false);
                    break;
                case IDC_BUTTON_ENABLEALL:
                    EnableAll(hWnd, true);
                    break;
                case IDC_BUTTON_DISABLEALL:
                    DisableAll(hWnd, true);
                    break;
			    case IDC_BUTTON_CLEARLOG:
                    ClearDrawnLog();
                    break;
                case IDC_BUTTON_OPTIONS:
                    DeactivateTooltips(toolInfo);
                    OpenOptions(hWnd);
                    break;
                case IDC_BUTTON_SURVAPP:
                    OpenSurvapp();
                    break;
                case IDC_BUTTON_TECHTREE:
                    OpenTechTree();
                    break;
                case IDC_CHECKBOX_REMAINLOG:
                    ToggleRemainLog();
                    break;
                case IDC_CHECKBOX_AUTORESET:
                    ToggleAutoReset(hWnd);
                    break;
                case IDC_CHECKBOX_AUTOTOGGLE:
                    ToggleAutoToggle(hWnd);
                    break;
                case IDC_ICON_CIV:
                    PlayJingle(current_civ);
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
					    MessageBox(hWnd, L"The original Age of Kings is not available on Steam. It is included in both HD and Definitive editions.", L"Age of Kings", MB_OK | MB_ICONINFORMATION);
                        break;
                    }
                    break;
                default:
                    return DefWindowProc(hWnd, message, wParam, lParam);
            }
            break;
        }  
        default: return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK ButtonProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_NCCREATE) originalButtonProcs[hwnd] = (WNDPROC)GetWindowLongPtr(hwnd, GWLP_WNDPROC);

    switch (uMsg)
    {    
        case WM_MOUSEMOVE:
        {
            if (!tooltips_enabled) break;

            if (hwndTooltip)
            {
                POINT pt;
                GetCursorPos(&pt);

                ScreenToClient(hwnd, &pt);

                RECT rect;
                TOOLINFO toolInfo = { 0 };
                toolInfo.cbSize = sizeof(toolInfo);
                toolInfo.hwnd = hwnd;
                toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS | TTF_TRACK;

                HWND button[] = { button_draw, button_reset, button_techtree, button_options, button_clearlog,
                                    button_enableall, button_disableall, checkbox_showremainlog,
                                    radiobutton_de, radiobutton_hd, radiobutton_aok, de_dlc_checkbox[0],
                                de_dlc_checkbox[1], de_dlc_checkbox[2], de_dlc_checkbox[3], de_dlc_checkbox[4], de_dlc_checkbox[5], de_dlc_checkbox[6],
                                hd_dlc_checkbox[0], hd_dlc_checkbox[1], hd_dlc_checkbox[2], checkbox_aoc,
                                checkbox_autotoggle, checkbox_autoreset, button_survapp, button_history };

                int tooltip_id[] = { TOOLTIP_DRAW, TOOLTIP_RESET, TOOLTIP_TECHTREE, TOOLTIP_OPTIONS, TOOLTIP_CLEAR,
                                        TOOLTIP_ENABLEALL, TOOLTIP_DISABLEALL, TOOLTIP_REMAININGTOGGLE,
                                        TOOLTIP_DE, TOOLTIP_HD, TOOLTIP_AOK, de_dlc_tipid[0], de_dlc_tipid[1],
                                    de_dlc_tipid[2], de_dlc_tipid[3], de_dlc_tipid[4], de_dlc_tipid[5], de_dlc_tipid[6],
                                    hd_dlc_tipid[0], hd_dlc_tipid[1], hd_dlc_tipid[2], TOOLTIP_AOC,
                                    TOOLTIP_AUTOTOGGLE, TOOLTIP_AUTORESET, TOOLTIP_SURVAPP, TOOLTIP_HISTORY };

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
            SendMessage(GetParent(hwnd), WM_MOUSEMOVE, wParam, lParam);
            break;
        }
        
        case WM_SETCURSOR:
            SetCursor(LoadCursor(NULL, IDC_HAND));
            return TRUE;
        default:
            return CallWindowProc(originalButtonProcs[hwnd], hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

void SubclassButton(HWND button) { originalButtonProcs[button] = (WNDPROC)SetWindowLongPtr(button, GWLP_WNDPROC, (LONG_PTR)ButtonProc); }

INT_PTR CALLBACK AboutDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static WNDPROC oldProc;
    switch (message)
    {
        case WM_INITDIALOG:
        {
            HWND hwndHyperlink = GetDlgItem(hDlg, IDC_HYPERLINK);
            oldProc = (WNDPROC)SetWindowLongPtr(hwndHyperlink, GWLP_WNDPROC, (LONG_PTR)HyperlinkProc);
            SetDlgItemText(hDlg, IDC_VERSION, VERSION);
            return(INT_PTR)TRUE;
        }
        case WM_COMMAND:
        {
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
            {
                PlayAudio(button);
                EndDialog(hDlg, LOWORD(wParam));
                return(INT_PTR)TRUE;
            }
            break;
        }
    }
    return (INT_PTR)FALSE;
}

INT_PTR CALLBACK OptionsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:
        {        
            CheckDlgButton(hDlg, IDC_CHECKBOX_LABELS, civ_labels_enabled ? BST_CHECKED : BST_UNCHECKED);
            CheckDlgButton(hDlg, IDC_CHECKBOX_CORNERLABEL, iteration_label_enabled ? BST_CHECKED : BST_UNCHECKED);
            CheckDlgButton(hDlg, IDC_CHECKBOX_ICONS, icons_enabled ? BST_CHECKED : BST_UNCHECKED);
		    CheckDlgButton(hDlg, IDC_CHECKBOX_JINGLES, jingles_enabled ? BST_CHECKED : BST_UNCHECKED);
            CheckDlgButton(hDlg, IDC_CHECKBOX_SOUNDS, ui_sounds_enabled ? BST_CHECKED : BST_UNCHECKED);
            CheckDlgButton(hDlg, IDC_CHECKBOX_TOOLTIPS, tooltips_enabled ? BST_CHECKED : BST_UNCHECKED);
            CheckDlgButton(hDlg, IDC_CHECKBOX_STARTDRAW, draw_on_startup ? BST_CHECKED : BST_UNCHECKED);
            CheckDlgButton(hDlg, IDC_CHECKBOX_CONTFRESH, contfresh_enabled ? BST_CHECKED : BST_UNCHECKED);

		    if (persistent_logging) CheckRadioButton(hDlg, IDC_RADIO_LOGGING, IDC_RADIO_STARTRESET, IDC_RADIO_LOGGING);
		    else CheckRadioButton(hDlg, IDC_RADIO_LOGGING, IDC_RADIO_STARTRESET, IDC_RADIO_STARTRESET);

            hComboBox = GetDlgItem(hDlg, IDC_LEGACY_OPTION);
            SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM)L"Definitive Edition");
            SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM)L"Legacy");
            SendMessage(hComboBox, CB_SETCURSEL, legacy_jingle_enabled ? 1 : 0, 0);

            if (!jingles_enabled)
            {
                ShowWindow(hComboBox, SW_HIDE);
                ShowWindow(GetDlgItem(hDlg, IDC_STATIC_JT), SW_HIDE);
            }

            hSlider = GetDlgItem(hDlg, IDC_SLIDER_CONTFRESHSTRENGTH);
            SendMessage(hSlider, TBM_SETRANGE, TRUE, MAKELPARAM(1, 10));
            SendMessage(hSlider, TBM_SETPOS, TRUE, contfresh_strength);

            if (!contfresh_enabled)
            {
                ShowWindow(hSlider, SW_HIDE);
                ShowWindow(GetDlgItem(hDlg, IDC_STATIC_CONTFRESHVALUE), SW_HIDE);
                ShowWindow(GetDlgItem(hDlg, IDC_STATIC_CFA), SW_HIDE);
                ShowWindow(GetDlgItem(hDlg, IDC_STATIC_CFB), SW_HIDE);
                ShowWindow(GetDlgItem(hDlg, IDC_STATIC_CFC), SW_HIDE);
            }

            int pos = (int)SendMessage(hSlider, TBM_GETPOS, 0, 0);


            wchar_t valueStr[16];
            swprintf_s(valueStr, L"%d", pos);
            SetDlgItemText(hDlg, IDC_STATIC_CONTFRESHVALUE, valueStr);

            return(INT_PTR)TRUE;
        }
        case WM_HSCROLL:
        {
            if ((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER_CONTFRESHSTRENGTH)) {
                int pos = (int)SendMessage((HWND)lParam, TBM_GETPOS, 0, 0);

                // Convert the value to a string
                wchar_t valueStr[16];
                swprintf_s(valueStr, L"%d", pos);

                // Update the static label
                SetDlgItemText(hDlg, IDC_STATIC_CONTFRESHVALUE, valueStr);

                contfresh_strength = pos;
				//MessageBox(hDlg, StringCleaner(L"Continent Freshness Strength set to: " + std::to_wstring(pos)), L"Continent Freshness Strength", MB_OK);
                // Use 'pos' as needed, e.g., update a variable or label
                //MessageBox(hDlg, StringCleaner(L"Continent Freshness Strength set to: " + std::to_wstring(pos)), L"Continent Freshness Strength", MB_OK);

            }
            break;
        }
        case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            int wmEvent = HIWORD(wParam);

            if (IDC_LEGACY_OPTION)
            {
                switch (wmEvent)
                {
                    case CBN_DROPDOWN:
                    {
                        PlayAudio(hover);
                        break;
                    }
                    case CBN_SELCHANGE:
                    {						
                        if (!GetCiv(current_civ).legacy) PlayAudio(view);
                        else PlayJingle(current_civ);
                        break;
                    }
                }
            }

		    ui_sounds_enabled = IsDlgButtonChecked(hDlg, IDC_CHECKBOX_SOUNDS) == BST_CHECKED;
            civ_labels_enabled = IsDlgButtonChecked(hDlg, IDC_CHECKBOX_LABELS) == BST_CHECKED;
            iteration_label_enabled = IsDlgButtonChecked(hDlg, IDC_CHECKBOX_CORNERLABEL) == BST_CHECKED;
		    icons_enabled = IsDlgButtonChecked(hDlg, IDC_CHECKBOX_ICONS) == BST_CHECKED;

            jingles_enabled = IsDlgButtonChecked(hDlg, IDC_CHECKBOX_JINGLES) == BST_CHECKED;
            if (!jingles_enabled)
            {
                ShowWindow(hComboBox, SW_HIDE);
                ShowWindow(GetDlgItem(hDlg, IDC_STATIC_JT), SW_HIDE);
            }
			else
			{
				ShowWindow(hComboBox, SW_SHOW);
				ShowWindow(GetDlgItem(hDlg, IDC_STATIC_JT), SW_SHOW);
			}

            tooltips_enabled = IsDlgButtonChecked(hDlg, IDC_CHECKBOX_TOOLTIPS) == BST_CHECKED;
		    draw_on_startup = IsDlgButtonChecked(hDlg, IDC_CHECKBOX_STARTDRAW) == BST_CHECKED;

			contfresh_enabled = IsDlgButtonChecked(hDlg, IDC_CHECKBOX_CONTFRESH) == BST_CHECKED;
            if (!contfresh_enabled)
            {
                ShowWindow(hSlider, SW_HIDE);
				ShowWindow(GetDlgItem(hDlg, IDC_STATIC_CONTFRESHVALUE), SW_HIDE);
				ShowWindow(GetDlgItem(hDlg, IDC_STATIC_CFA), SW_HIDE);
				ShowWindow(GetDlgItem(hDlg, IDC_STATIC_CFB), SW_HIDE);
				ShowWindow(GetDlgItem(hDlg, IDC_STATIC_CFC), SW_HIDE);
			}
            else
            {
                ShowWindow(hSlider, SW_SHOW);
				ShowWindow(GetDlgItem(hDlg, IDC_STATIC_CONTFRESHVALUE), SW_SHOW);
				ShowWindow(GetDlgItem(hDlg, IDC_STATIC_CFA), SW_SHOW);
				ShowWindow(GetDlgItem(hDlg, IDC_STATIC_CFB), SW_SHOW);
				ShowWindow(GetDlgItem(hDlg, IDC_STATIC_CFC), SW_SHOW);
            }
                


            int selectedIndex = static_cast<int>(SendMessage(hComboBox, CB_GETCURSEL, 0, 0));
            legacy_jingle_enabled = (selectedIndex == 1);



            if (icons_enabled && current_tab != 2) ShowWindow(civ_icon, SW_SHOW);            
            else ShowWindow(civ_icon, SW_HIDE);                    

            if (wmEvent == BN_CLICKED)
            {
                if (wmId == IDC_CHECKBOX_JINGLES)
                {
                    StopSound();
                    PlayJingle(current_civ);
                }
                
                switch (wmId)
                {
                case IDC_BUTTON_HOTKEYS:
				    OpenHotkeys(hDlg);
                    break;
                case IDC_CHECKBOX_LABELS:
                case IDC_CHECKBOX_CORNERLABEL:
                case IDC_CHECKBOX_ICONS:
                case IDC_CHECKBOX_SOUNDS:
                case IDC_CHECKBOX_TOOLTIPS:
			    case IDC_CHECKBOX_STARTDRAW:
                    PlayAudio(button);
                    break;
			    case IDC_RADIO_LOGGING:
                    PlayAudio(view);
                    persistent_logging = true;
                    break;
			    case IDC_RADIO_STARTRESET:
                    PlayAudio(view);
                    persistent_logging = false;
				    break;
                }
            }

            if (!civ_labels_enabled) ShowWindow(label_centre, SW_HIDE);
            else if (current_tab != 2) ShowWindow(label_centre, SW_SHOW);

            if (!iteration_label_enabled) ShowWindow(label_corner, SW_HIDE);
            else if (current_tab == 0) ShowWindow(label_corner, SW_SHOW);
        
            if (wmId == IDCANCEL || wmId == IDOK)
            {
                PlayAudio(button);
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
        {
            device_handling_context = BeginPaint(hwnd, &paint_struct);
            SetTextColor(device_handling_context, RGB(0, 0, 255));
            SetBkMode(device_handling_context, TRANSPARENT);
            SelectObject(device_handling_context, font_underline);
            GetClientRect(hwnd, &rectangle_struct);
            DrawText(device_handling_context, L"Hjörleif", -1, &rectangle_struct, DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX | DT_UNDERLINE);
            EndPaint(hwnd, &paint_struct);
            return 0;
        }
        case WM_LBUTTONDOWN:
        {
            ShellExecute(NULL, L"open", L"https://linktr.ee/hjoerleif", NULL, NULL, SW_SHOWNORMAL);
            return 0;
        }
        case WM_SETCURSOR:
        {
            SetCursor(LoadCursor(NULL, IDC_HAND));
            return 0;
        }
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void ResetProgram(bool auto_reset)
{

    if (!auto_reset && reset_state) return;

    if (persistent_logging && !auto_reset && !reset_state && !startup)
    {
        SaveLog(automatic);
        cache_available = true;
    }        

    if (!startup && !auto_reset && !jingles_enabled) PlayAudio(button);
    if (custom_max_civs < MAX_CIVS) custom_civ_pool = true;

    current_civ = L"Random";
    iterator = 0;
	remaining = custom_max_civs;
    
    UpdateDrawnLog(false, false, true);

    if (custom_civ_pool)
    {
        //civs.clear();
        
        custom_max_civs = 0;
        for (int i = 0; i < MAX_CIVS; i++)
        {
            if (civ[i].enabled)
            {
				available_civs[i] = true;
				civ_list[i] = civ[i].name;
                //civs.push_back(civ[i].name);
                custom_max_civs++;
            }
            else
            {
                available_civs[i] = false;
                civ_list[i] = L"";
            }
        }
    }
    else InitialiseCivs();    
    
	for (int i = 0; i < MAX_CIVS; i++) drawn_civs[i] = L"";

    SetWindowText(label_corner, (L"0/" + std::to_wstring(custom_max_civs)).c_str());
	if (!iteration_label_enabled) ShowWindow(label_corner, SW_HIDE);
    SetWindowText(label_centre, L"?");
	if (!civ_labels_enabled) ShowWindow(label_centre, SW_HIDE);
    SendMessageW(civ_icon, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)icon_random);

    if (!auto_reset && jingles_enabled && current_tab != 2) PlayJingle(current_civ);

    reset_state = true;

    UpdateRemainingLog(true);
    UpdateTooltips(false);
    undrawable = false;

    last_action = reset;



    //if (!auto_reset)
    //{
    //    int tenth = (custom_max_civs / 2) / 10;
    //    int factor = (10 * tenth) - (tenth * contfresh_strength);


    //    std::wstring mb_text = L"";
    //    for (int i = 0; i < MAX_CIVS; i++)
    //    {
    //        if (contfresh_array[i] != L"") mb_text += contfresh_array[i] + L"\n";
    //    }

    //    if (contfresh_strength == 1)
    //    {
    //        factor = 1;
    //        mb_text = contfresh_civ;
    //        MessageBox(NULL, StringCleaner(L"Contfresh civ amount: " + std::to_wstring(factor) + L"\nContfresh civ: " + mb_text), L"contresh info", MB_OK | MB_ICONINFORMATION);
    //    }
    //    else MessageBox(NULL, StringCleaner(L"Contfresh civ amount: " + std::to_wstring(factor) + L"\nContfresh civs:\n" + mb_text), L"contresh info", MB_OK | MB_ICONINFORMATION);

    //}

}

void DrawCiv(bool rigged, const std::wstring &civ_name)
{
    if (!startup && !jingles_enabled) PlayAudio(button);
    reset_state = false;

    if (custom_max_civs == 0)
    {
        PlayAudio(error);
        SetWindowTextA(label_centre, "Empty pool!");
        return;
    }

    if (remaining == 0 || iterator == custom_max_civs || iterator == 0)
    {
        ResetProgram(true);
        reset_state = false;
    }

    if (rigged)
    {
        current_civ = civ_name;
		civ_list[GetCivIndex(current_civ)] = L"";
		available_civs[GetCivIndex(current_civ)] = false;
        //civs.erase(std::remove(civs.begin(), civs.end(), civ_name), civs.end());
    }    
    else
    {
		ValidateRemainCount();
        if (remaining == 0) MessageBox(NULL, StringCleaner(L"remaining == 0 at line 971."), L"Error", MB_OK | MB_ICONERROR);
        int given_index = GetRandomInt(remaining);
        


        if (contfresh_enabled)
        {

            int r_delta = 0;
			for (int i = 0; i < MAX_CIVS; i++) civ_list_copy[i] = civ_list[i];
			for (int i = 0; i < MAX_CIVS; i++) available_civs_copy[i] = available_civs[i];
			//std::vector <std::wstring> civs_copy;
   //         civs_copy.reserve(MAX_CIVS);

   //         civs_copy = civs;
            do
            {
                if ((remaining - r_delta) < 2)
                {
                    for (int i = 0; i < MAX_CIVS; i++) if (available_civs_copy[i]) current_civ = civ_list_copy[i];
                    break;
                }
				ValidateRemainCount();
                if (remaining == 0) MessageBox(NULL, StringCleaner(L"remaining == 0 at line 989."), L"Error", MB_OK | MB_ICONERROR);
                given_index = GetRandomInt(remaining - r_delta);

                int j = 0;
                for (int i = 0; i < MAX_CIVS; i++)
                {
                    while (!available_civs_copy[j]) j++;
                    if (i == given_index)
                    {
                        given_index = j;
                        break;
                    }
                    j++;
                }
                if (given_index < 0 || given_index >= MAX_CIVS)
                {
                    MessageBox(NULL, StringCleaner(L"Invalid index! Given index currently: " + std::to_wstring(given_index)), L"Error", MB_OK | MB_ICONERROR);
                    return;
                }
                //else
                //{
                //    MessageBox(NULL, StringCleaner(L"Valid index! Given index currently: " + std::to_wstring(given_index)), L"Success", MB_OK | MB_ICONINFORMATION);
                //}
                current_civ = civ[given_index].name;



                civ_list_copy[given_index] = L"";
                if (given_index < 0 || given_index >= MAX_CIVS)
                {
					MessageBox(NULL, StringCleaner(L"Invalid index! Given index currently: " + std::to_wstring(given_index)), L"Error", MB_OK | MB_ICONERROR);
					return;
                }
				available_civs_copy[given_index] = false;

                //std::shuffle(civs_copy.begin(), civs_copy.end(), mt);
                // 
                //current_civ = civs_copy.back();
                
                //civs_copy.pop_back();

                //if (IsContfreshCiv(current_civ))
                //{
                //    break;
                //    //civs.erase(std::remove(civs.begin(), civs.end(), current_civ), civs.end());
                //    //MessageBox(NULL, StringCleaner(current_civ + L" is a cont fresh civ!"), L"Contfresh", MB_OK | MB_ICONINFORMATION);
                //}
                //else 
                r_delta++;
			} while (!IsContfreshCiv(current_civ));
        }
        else
        {

            int j = 0;
            for (int i = 0; i < MAX_CIVS; i++)
            {
                while (!available_civs[j]) j++;
                if (i == given_index)
                {
                    given_index = j;
                    break;
                }
                j++;
            }
			current_civ = civ[given_index].name;

        }
        civ_list[GetCivIndex(current_civ)] = L"";
        available_civs[GetCivIndex(current_civ)] = false;
    }



	drawn_civs[iterator] = current_civ;

    std::string civ_name_str = ConvertToString(current_civ);

    iterator++;
    remaining--;

    label_text = std::to_wstring(iterator) + L"/" + std::to_wstring(custom_max_civs);
    SetWindowText(label_corner, label_text.c_str());
	if (!iteration_label_enabled) ShowWindow(label_corner, SW_HIDE);
    SetWindowTextA(label_centre, civ_name_str.c_str());
	if (!civ_labels_enabled) ShowWindow(label_centre, SW_HIDE);

    HBITMAP drawn_civ_icon = FetchCivIcon(current_civ);
    SendMessageW(civ_icon, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)drawn_civ_icon);
    
	civ_list[GetCivIndex(current_civ)] = L"";

    UpdateDrawnLog(false, true, false);
    UpdateRemainingLog(false);


    /*
    std::thread sound_thread(PlayJingle, current_civ);
    sound_thread.detach();*/
    if (!startup) PlayJingle(current_civ);
    

    UpdateTooltips(true);
    redrawable = false;
    undrawable = true;

    last_action = draw;

    if (contfresh_enabled) UpdateContfresh(current_civ);
}

void EnableHotkeys(HWND hWnd) {	for (int i = 0; i < HOTKEY_AMOUNT; i++) RegisterHotKey(hWnd, hotkey[i].id, hotkey[i].modifier, hotkey[i].key); }

void DisableHotkeys(HWND hWnd) { for (int i = 1; i < HOTKEY_AMOUNT; i++) UnregisterHotKey(hWnd, i); }

void CreateUnderlineFont()
{
    LOGFONT lf;
    HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    GetObject(hFont, sizeof(LOGFONT), &lf);
    lf.lfUnderline = TRUE;
    font_underline = CreateFontIndirect(&lf);
}

std::string ConvertToString(const std::wstring& wstr)
{
    std::string str;
    str.reserve(MAX_LENGTH);
    WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, &str[0], MAX_LENGTH, nullptr, nullptr);
    return str;
}

void LoadImages()
{ 
    std::wstring bmp_parsed_civname;
    std::wstring icon_path;
    for (int i = 0; i < MAX_CIVS; i++)
    {
        bmp_parsed_civname = civ[i].name;
        bmp_parsed_civname[0] = std::tolower(bmp_parsed_civname[0]);
        icon_path = L"images\\civ_icons\\" + bmp_parsed_civname + L".bmp";
        civ[i].icon = (HBITMAP)LoadImageW(NULL, icon_path.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    }

    icon_techtree = (HBITMAP)LoadImageW(NULL, L"images\\civ_icons\\techtree.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_survapp = (HBITMAP)LoadImageW(NULL, L"images\\survivalist.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    icon_options = (HBITMAP)LoadImageW(NULL, L"images\\civ_icons\\options.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    icon_history = (HBITMAP)LoadImageW(NULL, L"images\\history.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    icon_random = (HBITMAP)LoadImageW(NULL, L"images\\civ_icons\\random.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

	icon_de = (HBITMAP)LoadImageW(NULL, L"images\\edition_icons\\aoe2de.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_hd = (HBITMAP)LoadImageW(NULL, L"images\\edition_icons\\aoe2hd.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	icon_aok = (HBITMAP)LoadImageW(NULL, L"images\\edition_icons\\aok.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    for (int i = 0; i < de_dlc_amount; i++) de_dlc_bmp[i] = (HBITMAP)LoadImageW(NULL, StringCleaner(L"images\\dlc_icons\\" + de_dlc_bmpstring[i]), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    for (int i = 0; i < hd_dlc_amount; i++) hd_dlc_bmp[i] = (HBITMAP)LoadImageW(NULL, StringCleaner(L"images\\dlc_icons\\" + hd_dlc_bmpstring[i]), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

	icon_aoc = (HBITMAP)LoadImageW(NULL, L"images\\dlc_icons\\aoc.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);  	
}

HBITMAP FetchCivIcon(const std::wstring &civ_name)
{
    if (civ_name == L"Random") return icon_random;
    for (int i = 0; i < MAX_CIVS; i++) if (civ_name == civ[i].name) return civ[i].icon;
	return icon_random;
}

void AddCiv(const std::wstring &civ)
{
    if (GetCiv(civ).enabled) return;

    //civs.push_back(civ);
	civ_list[GetCivIndex(civ)] = civ;
    available_civs[GetCivIndex(civ)] = true;
	GetCiv(civ).SetEnabled(true);
	custom_max_civs++;
    label_text = std::to_wstring(iterator) + L"/" + std::to_wstring(custom_max_civs);
    SetWindowText(label_corner, label_text.c_str());
	if (custom_max_civs == MAX_CIVS) custom_civ_pool = false;
    UpdateRemainingLog(true);
    UpdateDrawnLog(false, false, false);
    if (!batch_toggle) UpdateEnabledCivsCounter();
    last_action = pool_change;
}

void RemoveCiv(const std::wstring &civ)
{
    if (!GetCiv(civ).enabled) return;
    
    if (!available_civs[GetCivIndex(civ)])
    {
        iterator--;
		remaining++;
    }

    custom_civ_pool = true;

	civ_list[GetCivIndex(civ)] = L"";
	available_civs[GetCivIndex(civ)] = false;
	//civs.erase(std::remove(civs.begin(), civs.end(), civ), civs.end());
	GetCiv(civ).SetEnabled(false);
	custom_max_civs--;
    label_text = std::to_wstring(iterator) + L"/" + std::to_wstring(custom_max_civs);
    SetWindowText(label_corner, label_text.c_str());
    UpdateRemainingLog(true);
	UpdateDrawnLog(false, false, false);
    if (!batch_toggle) UpdateEnabledCivsCounter();
    last_action = pool_change;
}

void InitialiseCivStates() { for (int i = 0; i < MAX_CIVS; i++) civ[i].SetEnabled(true); }

HWND CreateCheckbox(HWND hWnd, HINSTANCE hInstance, int x, int y, int width, int height, int id, LPCWSTR text)
{
    HWND checkbox = CreateWindow(L"BUTTON", text, WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, x, y+130, width+2, height, hWnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), hInstance, NULL);
    SendMessage(checkbox, BM_SETCHECK, BST_CHECKED, 0);
    return checkbox;
}

void ShowAllPoolCheckboxes() { for (int i = 0; i < MAX_CIVS; i++) ShowWindow(civ[i].checkbox, SW_SHOW); }

void HideCustomPoolCheckboxes()
{ 
	for (int i = 0; i < MAX_CIVS; i++) ShowWindow(civ[i].checkbox, SW_HIDE);
	ShowDEDLCCheckboxes(false);
	ShowHDDLCCheckboxes(false);
	ShowAOCCheckbox(false);
    ShowWindow(checkbox_aoc, SW_HIDE);
}

void EnableAll(HWND hWnd, bool sound_acceptable)
{
	batch_toggle = true;

	if (sound_acceptable) PlayAudio(button);
	custom_civ_pool = false;

    switch (edition_state)
    {
        case DE:
        {

            for (int i = 0; i < MAX_CIVS; i++)
            {
                SendMessage(civ[i].checkbox, BM_SETCHECK, BST_CHECKED, 0);
                AddCiv(civ[i].name);
            }
            break;
        }
        case HD:
        {
            for (int i = 0; i < MAX_CIVS; i++)
            {
                if (civ[i].edition != DE)
                {
                    SendMessage(civ[i].checkbox, BM_SETCHECK, BST_CHECKED, 0);
                    AddCiv(civ[i].name);
                }
            }
            break;
        }
        case AOK:
        {
            for (int i = 0; i < MAX_CIVS; i++)
            {
                if (civ[i].edition == AOK)
                {
                    SendMessage(civ[i].checkbox, BM_SETCHECK, BST_CHECKED, 0);
                    AddCiv(civ[i].name);
                }
            }
            break;
        }
    }

    if (autoreset_enabled) ResetProgram(true);
    ValidateAllDlcToggles(hWnd);
    UpdateEnabledCivsCounter();
	batch_toggle = false;
}

void DisableAll(HWND hWnd, bool sound_acceptable)
{
	batch_toggle = true;

    if (sound_acceptable) PlayAudio(button);
	custom_civ_pool = true;
    for (int i = 0; i < MAX_CIVS; i++)
    {
        SendMessage(civ[i].checkbox, BM_SETCHECK, BST_UNCHECKED, 0);
		RemoveCiv(civ[i].name);
    }
    if (autoreset_enabled) ResetProgram(true);
    ValidateAllDlcToggles(hWnd);
    UpdateEnabledCivsCounter();
    batch_toggle = false;
}

void ShowDrawTab(bool showing_enabled, HWND hWnd)
{
    int height = GetWindowHeight(hWnd);
    int width = GetWindowWidth(hWnd);
    if (showing_enabled)
    {
        ShowWindow(button_draw, SW_SHOW);
        SetWindowPos(button_reset, NULL, 10, height - 40, 100, 30, SWP_NOZORDER);
	    ShowWindow(button_reset, SW_SHOW);
	    if (civ_labels_enabled) ShowWindow(label_centre, SW_SHOW);
        if (icons_enabled) ShowWindow(civ_icon, SW_SHOW);
	    if (iteration_label_enabled) ShowWindow(label_corner, SW_SHOW);
        ShowWindow(button_survapp, SW_SHOW);
		ShowWindow(button_techtree, SW_SHOW);
        ShowWindow(button_history, SW_SHOW);
        UpdateTooltipText(button_options, hwndTooltip[TOOLTIP_OPTIONS], StringCleaner(L"Opens options\nHotkey: Q (Draw tab only) / F1"));
	}
    else if (!showing_enabled)
    {
        if (current_tab == 2)
        {
            ShowWindow(civ_icon, SW_HIDE);
            ShowWindow(button_draw, SW_HIDE);
            ShowWindow(label_centre, SW_HIDE);
        }
        ShowWindow(button_reset, SW_HIDE);
        SetWindowPos(button_reset, NULL, (width - 100) / 2, height - 40, 100, 30, SWP_NOZORDER);
        ShowWindow(label_corner, SW_HIDE);
		ShowWindow(button_survapp, SW_HIDE);
		ShowWindow(button_techtree, SW_HIDE);
        ShowWindow(button_history, SW_HIDE);
        UpdateTooltipText(button_options, hwndTooltip[TOOLTIP_OPTIONS], StringCleaner(L"Opens options\nHotkey: F1"));
    }	
}

void ShowLogTab(bool showing_enabled)
{
    if (showing_enabled)
    {
        ShowWindow(button_draw, SW_SHOW);
        ShowWindow(button_reset, SW_SHOW);
        if (civ_labels_enabled) ShowWindow(label_centre, SW_SHOW);
        if (icons_enabled) ShowWindow(civ_icon, SW_SHOW);
		ShowWindow(label_drawncount, SW_SHOW);
		ShowWindow(button_clearlog, SW_SHOW);
        ShowWindow(drawn_log, SW_SHOW);
		ShowWindow(label_remainingcount, SW_SHOW);
        ShowWindow(checkbox_showremainlog, SW_SHOW);
		if (remainlog_enabled) ShowWindow(remaining_log, SW_SHOW);        
    }
    else if (!showing_enabled)
    {
        if (current_tab == 2)
        {
            ShowWindow(button_draw, SW_HIDE);
            ShowWindow(label_centre, SW_HIDE);
            ShowWindow(civ_icon, SW_HIDE);
        }
        ShowWindow(button_reset, SW_HIDE);
		ShowWindow(label_drawncount, SW_HIDE);
		ShowWindow(button_clearlog, SW_HIDE);
        ShowWindow(drawn_log, SW_HIDE);
		ShowWindow(label_remainingcount, SW_HIDE);
		ShowWindow(checkbox_showremainlog, SW_HIDE);
        ShowWindow(remaining_log, SW_HIDE);
    }
}

void ShowCustomTab(bool showing_enabled)
{
    if (showing_enabled)
    {
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
		ShowWindow(label_enabledcount, SW_SHOW);

        if (edition_state == DE) ShowDEDLCCheckboxes(true);
		else if (edition_state == HD) ShowHDDLCCheckboxes(true);
        else if (edition_state == AOK) ShowAOCCheckbox(true);
    }
    else if (!showing_enabled)
    {
        ShowWindow(edition_icon, SW_HIDE);
		ShowWindow(label_enabledcount, SW_HIDE);
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

void ShowHDPoolCheckboxes()
{
    for (int i = 0; i < MAX_CIVS; i++)
    {        
        if (civ[i].edition != DE) ShowWindow(civ[i].checkbox, SW_SHOW);
        else
        {
			ShowWindow(civ[i].checkbox, SW_HIDE);
            SendMessage(civ[i].checkbox, BM_SETCHECK, BST_UNCHECKED, 0);
            RemoveCiv(civ[i].name);
        }        
    }
}

void ShowAOCPoolCheckboxes()
{
    for (int i = 0; i < MAX_CIVS; i++)
    {
        if (civ[i].edition == AOK) ShowWindow(civ[i].checkbox, SW_SHOW);
		else
        {
            ShowWindow(civ[i].checkbox, SW_HIDE);
            SendMessage(civ[i].checkbox, BM_SETCHECK, BST_UNCHECKED, 0);
            RemoveCiv(civ[i].name);
        }
    }
}

void ShowDEDLCCheckboxes(bool de_state)
{
    if (de_state)
    {
        for (int i = 0; i < de_dlc_amount; i++)
        {
            ShowWindow(de_dlc_checkbox[i], SW_SHOW);
            ShowWindow(de_dlc_icon[i], SW_SHOW);
        }            
    }
	else
    {
        for (int i = 0; i < de_dlc_amount; i++)
        {
            ShowWindow(de_dlc_checkbox[i], SW_HIDE);
            ShowWindow(de_dlc_icon[i], SW_HIDE);
        }            
	}	
}

void ShowHDDLCCheckboxes(bool hd_state)
{
    if (hd_state)
    {
        for (int i = 0; i < hd_dlc_amount; i++)
        {
            ShowWindow(hd_dlc_checkbox[i], SW_SHOW);
            ShowWindow(hd_dlc_icon[i], SW_SHOW);
        }            
    }
	else
    {
        for (int i = 0; i < hd_dlc_amount; i++)
        {
            ShowWindow(hd_dlc_checkbox[i], SW_HIDE);
            ShowWindow(hd_dlc_icon[i], SW_HIDE);
        }            
	}
}

void ShowAOCCheckbox(bool aok_state)
{
	if (aok_state)
    {
		ShowWindow(checkbox_aoc, SW_SHOW);
		ShowWindow(aoc_icon, SW_SHOW);
	}
	else
    {
		ShowWindow(checkbox_aoc, SW_HIDE);
		ShowWindow(aoc_icon, SW_HIDE);
	}
}

void ToggleDlc(dlc civ_dlc, HWND hWnd)
{
	batch_toggle = true;
    if (!startup) PlayAudio(button);
	int check_id = GetDlcCheckboxId(civ_dlc);

    if (hotkey_pressed)
    {
		if (IsDlgButtonChecked(hWnd, check_id) == BST_CHECKED) CheckDlgButton(hWnd, check_id, BST_UNCHECKED);		
		else CheckDlgButton(hWnd, check_id, BST_CHECKED);		
    }

    if (IsDlgButtonChecked(hWnd, check_id) == BST_CHECKED)
    {
        for (int i = 0; i < MAX_CIVS; i++)
        {
            if (civ[i].dlc == civ_dlc)
            {
                CheckDlgButton(hWnd, i + 5, BST_CHECKED);
                AddCiv(civ[i].name);
            }
        }
    }
    else
    {
        for (int i = 0; i < MAX_CIVS; i++)
        {
            if (civ[i].dlc == civ_dlc)
            {
                CheckDlgButton(hWnd, i + 5, BST_UNCHECKED);
                RemoveCiv(civ[i].name);
            }
        }
    }
    UpdateEnabledCivsCounter();
	batch_toggle = false;
}

void EnableDlc(dlc civ_dlc, HWND hWnd)
{
    for (int i = 0; i < MAX_CIVS; i++)
    {        
        if (civ[i].dlc == civ_dlc)
        {
            CheckDlgButton(hWnd, i + 5, BST_CHECKED);
            AddCiv(civ[i].name);
        }
    }
}

void DisableDlc(dlc civ_dlc, HWND hWnd)
{
    for (int i = 0; i < MAX_CIVS; i++)
    {
        if (civ[i].dlc == civ_dlc)
        {
            CheckDlgButton(hWnd, i + 5, BST_UNCHECKED);
            RemoveCiv(civ[i].name);
        }
    }
}

bool IsDlcEmpty(dlc civ_dlc)
{
	int count = 0;
	for (int i = 0; i < MAX_CIVS; i++) if (civ[i].enabled && civ[i].dlc == civ_dlc) count++;
	if (count == 0) return true;
	else return false;
}

void ValidateDlcToggle(HWND hWnD, dlc civ_dlc)
{
	if (!IsDlcEmpty(civ_dlc)) CheckDlgButton(hWnD, GetDlcCheckboxId(civ_dlc), BST_CHECKED);
    else CheckDlgButton(hWnD, GetDlcCheckboxId(civ_dlc), BST_UNCHECKED);
}

void ValidateAllDlcToggles(HWND hWnd) { for (int i = 0; i < DLC_AMOUNT; i++) ValidateDlcToggle(hWnd, every_dlc[i]); }

void UpdateDrawnLog(bool start_state, bool drawn, bool blankline_wanted)
{
    std::wstring drawn_label = L"Drawn: " + std::to_wstring(iterator) + L"/" + std::to_wstring(custom_max_civs);
    SetWindowText(label_drawncount, drawn_label.c_str());

    drawnlog_length = GetWindowTextLength(drawn_log);
    drawnlog_text.resize(drawnlog_length + 1);
    GetWindowText(drawn_log, &drawnlog_text[0], drawnlog_length + 1);
    drawnlog_text.pop_back();

    if (cache_loading)
    {
        if (drawnlog_text.substr(0, 2) == L"\r\n")
        {
            drawnlog_text.erase(0, 2);
            SetWindowText(drawn_log, drawnlog_text.c_str());
            drawn_log_linecount--;
        }
        return;
    }

    if (drawn)
    {        

        log_entry = std::to_wstring(iterator) + L"/" + std::to_wstring(custom_max_civs) + L" - " + std::wstring(current_civ.begin(), current_civ.end()) + L"\r\n";
        //drawnlog_text = log_entry + drawnlog_text;
        //if (iterator == custom_max_civs)
        //{
        //    //drawnlog_text += L"\r\n";
        //    //drawn_log_linecount++;
        //    //CheckDrawnLogLength();

        //}            
        //drawn_log_linecount++;
        //CheckDrawnLogLength();
        //SetWindowText(drawn_log, drawnlog_text.c_str());
        log_addentry(log_entry);
        if (!start_state || start_state && GetCiv(current_civ).enabled) drawn_civs[iterator-1] = current_civ;

    }    
    else if (!reset_state && blankline_wanted)
    {
        if (drawnlog_text.substr(0, 2) != L"\r\n")
        {
            //drawnlog_length = GetWindowTextLength(drawn_log);
            //drawnlog_text.resize(drawnlog_length + 1);
            //GetWindowText(drawn_log, &drawnlog_text[0], drawnlog_length + 1);
            //drawnlog_text.pop_back();
            //drawnlog_text = L"\r\n" + drawnlog_text;
            //SetWindowText(drawn_log, drawnlog_text.c_str());
			log_addentry(L"\r\n");
        }        
    }

    SetWindowText(label_drawncount, drawn_label.c_str());

    if (start_state)
    {
        if (!icons_enabled) ShowWindow(civ_icon, SW_HIDE);
        if (!civ_labels_enabled) ShowWindow(label_centre, SW_HIDE);
        if (!iteration_label_enabled) ShowWindow(label_corner, SW_HIDE);
    }


}

void UpdateRemainingLog(bool refresh)
{
    if (refresh)
    {
        remaininglog_text.clear();
        for (int i = 0; i < MAX_CIVS; i++)
        {
			if (GetCiv(civ_list[i]).enabled && civ_list[i] != L"")
			{
                remaininglog_text += civ_list[i] + L"\r\n";
			}
        }
    }
    else
    {
        size_t pos = remaininglog_text.find(current_civ + L"\r\n");
        if (pos != std::wstring::npos) remaininglog_text.erase(pos, current_civ.length() + 2);
    }

	ValidateRemainCount();
	std::wstring remain_label = L"Remaining: " + std::to_wstring(remaining) + L"/" + std::to_wstring(custom_max_civs);
    SetWindowText(remaining_log, remaininglog_text.c_str());
	SetWindowText(label_remainingcount, remain_label.c_str());
}

int GetWindowWidth(HWND hWnd) { RECT rect; GetClientRect(hWnd, &rect); return rect.right - rect.left; }

int GetWindowHeight(HWND hWnd) { RECT rect; GetClientRect(hWnd, &rect); return rect.bottom - rect.top; }

void OpenTechTree()
{
    PlayAudio(button);
    std::wstring techtree_path = L"https://aoe2techtree.net/#";

    if (current_civ == L"Maya") techtree_path += L"Mayans";
    else if (current_civ == L"Inca") techtree_path += L"Incas";
    else if (current_civ != L"Random") techtree_path += current_civ;

    ShellExecute(NULL, L"open", techtree_path.c_str(), NULL, NULL, SW_SHOWNORMAL);
}

void OpenSurvapp()
{
    PlayAudio(button);
    ShellExecute(NULL, L"open", L"https://aoe2-de-tools.herokuapp.com/villagers-required/", NULL, NULL, SW_SHOWNORMAL);
}

void AddTooltip(HWND hwndTool, HWND hwndTip, LPCWSTR pszText)
{
    if (!hwndTool || !hwndTip)
    {
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

    if (!SendMessage(hwndTip, TTM_ADDTOOL, 0, (LPARAM)&toolInfo))
    {
		MessageBox(NULL, L"Failed to add tooltip", L"Error", MB_OK);
        OutputDebugString(L"Failed to add tooltip\n");
    }
}

void ActivateTooltip(HWND hwndTip, TOOLINFO *toolInfo, POINT pt)
{
    ClientToScreen(toolInfo->hwnd, &pt);

    SendMessage(hwndTip, TTM_TRACKPOSITION, 0, (LPARAM)MAKELONG(pt.x + 10, pt.y + 20));

    if (!SendMessage(hwndTip, TTM_TRACKACTIVATE, TRUE, (LPARAM)toolInfo)) OutputDebugString(L"Failed to activate tooltip\n");
}

LPCWSTR StringCleaner(const std::wstring &dirty_string) { LPCWSTR clean_string = const_cast<LPCWSTR>(dirty_string.c_str()); return clean_string; }

LPCWSTR StringConjurer(LPVOID dirty_string) { LPCWSTR clean_string = static_cast<LPCWSTR>(dirty_string); return clean_string; }

void UpdateTooltipText(HWND hwndTool, HWND hwndTip, LPCWSTR newText)
{
    if (!hwndTool || !hwndTip)
    {
        MessageBox(NULL, L"Invalid handle(s) provided to UpdateTooltipText", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    TOOLINFO toolInfo = { 0 };
    toolInfo.cbSize = sizeof(toolInfo);
    toolInfo.hwnd = hwndTool;
    toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS | TTF_TRACK;
    toolInfo.uId = (UINT_PTR)hwndTool;
    toolInfo.lpszText = (LPWSTR)newText;

    SendMessage(hwndTip, TTM_UPDATETIPTEXT, 0, (LPARAM)&toolInfo);
}

void SetEditionState(HWND hWnd, edition edition)
{
	batch_toggle = true;
    if (!startup) PlayAudio(view);
    switch (edition)
    {
        case DE:
        {
            ShowDEDLCCheckboxes(true);
            ShowHDDLCCheckboxes(false);
            ShowAOCCheckbox(false);
            SendMessageW(edition_icon, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)icon_de);
            edition_state = DE;
            if (autotoggle_enabled && !startup) for (int i = 0; i < 5; i++) EnableDlc(old_dlc[i], hWnd);
            ShowAllPoolCheckboxes();
            if ((SendMessage(radiobutton_de, BM_GETCHECK, 0, 0)) != BST_CHECKED)
            {
                SendMessage(radiobutton_de, BM_SETCHECK, BST_CHECKED, 0);
                SendMessage(radiobutton_hd, BM_SETCHECK, BST_UNCHECKED, 0);
                SendMessage(radiobutton_aok, BM_SETCHECK, BST_UNCHECKED, 0);
            }
            break;
        }
        case HD:
        {
            ShowDEDLCCheckboxes(false);
            ShowHDDLCCheckboxes(true);
            ShowAOCCheckbox(false);
            if (autotoggle_enabled && !startup) for (int i = 0; i < 2; i++) EnableDlc(old_dlc[i], hWnd);
            SendMessageW(edition_icon, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)icon_hd);
            ShowHDPoolCheckboxes();
            if ((SendMessage(radiobutton_hd, BM_GETCHECK, 0, 0)) != BST_CHECKED)
            {
				SendMessage(radiobutton_hd, BM_SETCHECK, BST_CHECKED, 0);
				SendMessage(radiobutton_de, BM_SETCHECK, BST_UNCHECKED, 0);
				SendMessage(radiobutton_aok, BM_SETCHECK, BST_UNCHECKED, 0);
            }
            break;
        }
        case AOK:
        {
            ShowDEDLCCheckboxes(false);
            ShowHDDLCCheckboxes(false);
            ShowAOCCheckbox(true);
            if (autotoggle_enabled && !startup) EnableDlc(aok, hWnd);
            SendMessageW(edition_icon, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)icon_aok);
            ShowAOCPoolCheckboxes();
            if ((SendMessage(radiobutton_aok, BM_GETCHECK, 0, 0)) != BST_CHECKED)
            {
                SendMessage(radiobutton_de, BM_SETCHECK, BST_UNCHECKED, 0);
                SendMessage(radiobutton_hd, BM_SETCHECK, BST_UNCHECKED, 0);
                SendMessage(radiobutton_aok, BM_SETCHECK, BST_CHECKED, 0);
            }
            break;
        }
    }
    edition_state = edition;
    ValidateAllDlcToggles(hWnd);
    UpdateEnabledCivsCounter();
    batch_toggle = false;
}
       
void ToggleRemainLog()
{	
    PlayAudio(button);
    if (remainlog_enabled)
    {
        ShowWindow(remaining_log, SW_HIDE);
        remainlog_enabled = false;
        if (hotkey_pressed)
        {
            if ((SendMessage(checkbox_showremainlog, BM_GETCHECK, 0, 0)) != BST_UNCHECKED) SendMessage(checkbox_showremainlog, BM_SETCHECK, BST_UNCHECKED, 0);            
            else SendMessage(checkbox_showremainlog, BM_SETCHECK, BST_CHECKED, 0);
        }        
    }
	else
	{
		ShowWindow(remaining_log, SW_SHOW);
		remainlog_enabled = true;

        if (hotkey_pressed)
        {
            if ((SendMessage(checkbox_showremainlog, BM_GETCHECK, 0, 0)) != BST_CHECKED) SendMessage(checkbox_showremainlog, BM_SETCHECK, BST_CHECKED, 0);            
            else SendMessage(checkbox_showremainlog, BM_SETCHECK, BST_UNCHECKED, 0);
        }        
	}
}

int GetDlcCheckboxId(dlc dlc)
{
	for (int i = 0; i < DLC_AMOUNT; i++) if (every_dlc[i] == dlc) return every_dlc_id[i];
    return -1;
}

void ToggleAutoToggle(HWND hWnd)
{
    if (!startup) PlayAudio(button);
    if (hotkey_pressed)
    {
        if (IsDlgButtonChecked(hWnd, IDC_CHECKBOX_AUTOTOGGLE) == BST_CHECKED) CheckDlgButton(hWnd, IDC_CHECKBOX_AUTOTOGGLE, BST_UNCHECKED);		
        else CheckDlgButton(hWnd, IDC_CHECKBOX_AUTOTOGGLE, BST_CHECKED);
    }

    if (IsDlgButtonChecked(hWnd, IDC_CHECKBOX_AUTOTOGGLE) == BST_CHECKED)
    {
        autotoggle_enabled = true;
        switch (edition_state)
        {
            case DE: for (int i = 0; i < 5; i++) EnableDlc(old_dlc[i], hWnd); break;
            case HD: for (int i = 0; i < 2; i++) EnableDlc(old_dlc[i], hWnd); break;
            case AOK: EnableDlc(aok, hWnd); break;
        }
    }
    else if (IsDlgButtonChecked(hWnd, IDC_CHECKBOX_AUTOTOGGLE) == BST_UNCHECKED)
    {
        autotoggle_enabled = false;
        switch (edition_state)
        {
            case DE: for (int i = 0; i < 5; i++) DisableDlc(old_dlc[i], hWnd); break;
            case HD: for (int i = 0; i < 2; i++) DisableDlc(old_dlc[i], hWnd); break;
            case AOK: DisableDlc(aok, hWnd); break;
        }
    }
}

void ToggleAutoReset(HWND hWnd)
{
    if (!startup) PlayAudio(button);
	if (hotkey_pressed)
	{
		if (IsDlgButtonChecked(hWnd, IDC_CHECKBOX_AUTORESET) == BST_CHECKED) CheckDlgButton(hWnd, IDC_CHECKBOX_AUTORESET, BST_UNCHECKED);		
		else CheckDlgButton(hWnd, IDC_CHECKBOX_AUTORESET, BST_CHECKED);
	}
    if (IsDlgButtonChecked(hWnd, IDC_CHECKBOX_AUTORESET) == BST_CHECKED) autoreset_enabled = true;
    else if (IsDlgButtonChecked(hWnd, IDC_CHECKBOX_AUTORESET) == BST_UNCHECKED) autoreset_enabled = false;
}

void SaveSettings()
{                       
    WritePrivateProfileString(L"Settings", L"PersistentLogging", persistent_logging ? L"1" : L"0", INI_FILE_PATH);
	WritePrivateProfileString(L"Settings", L"DrawOnStartup", draw_on_startup ? L"1" : L"0", INI_FILE_PATH);
    WritePrivateProfileString(L"Settings", L"UISoundsEnabled", ui_sounds_enabled ? L"1" : L"0", INI_FILE_PATH);
    WritePrivateProfileString(L"Settings", L"LabelsEnabled", civ_labels_enabled ? L"1" : L"0", INI_FILE_PATH);
    WritePrivateProfileString(L"Settings", L"IterationLabelEnabled", iteration_label_enabled ? L"1" : L"0", INI_FILE_PATH);
    WritePrivateProfileString(L"Settings", L"IconsEnabled", icons_enabled ? L"1" : L"0", INI_FILE_PATH);
    WritePrivateProfileString(L"Settings", L"JinglesEnabled", jingles_enabled ? L"1" : L"0", INI_FILE_PATH);
    WritePrivateProfileString(L"Settings", L"TooltipsEnabled", tooltips_enabled ? L"1" : L"0", INI_FILE_PATH);
    WritePrivateProfileString(L"Settings", L"RemainLogEnabled", remainlog_enabled ? L"1" : L"0", INI_FILE_PATH);
    WritePrivateProfileString(L"Settings", L"AutoResetEnabled", autoreset_enabled ? L"1" : L"0", INI_FILE_PATH);
    WritePrivateProfileString(L"Settings", L"AutoToggleEnabled", autotoggle_enabled ? L"1" : L"0", INI_FILE_PATH);
    WritePrivateProfileString(L"Settings", L"LegacyJingleEnabled", legacy_jingle_enabled ? L"1" : L"0", INI_FILE_PATH);
    WritePrivateProfileString(L"Settings", L"ContFreshEnabled", contfresh_enabled ? L"1" : L"0", INI_FILE_PATH);
    WritePrivateProfileString(L"Settings", L"ContFreshStrengthValue", StringCleaner(std::to_wstring(contfresh_strength)), INI_FILE_PATH);
}

void LoadSettings()
{
	persistent_logging = GetPrivateProfileInt(L"Settings", L"PersistentLogging", 1, INI_FILE_PATH);
	draw_on_startup = GetPrivateProfileInt(L"Settings", L"DrawOnStartup", 1, INI_FILE_PATH);
    ui_sounds_enabled = GetPrivateProfileInt(L"Settings", L"UISoundsEnabled", 1, INI_FILE_PATH);
    civ_labels_enabled = GetPrivateProfileInt(L"Settings", L"LabelsEnabled", 1, INI_FILE_PATH);
    iteration_label_enabled = GetPrivateProfileInt(L"Settings", L"IterationLabelEnabled", 1, INI_FILE_PATH);
    icons_enabled = GetPrivateProfileInt(L"Settings", L"IconsEnabled", 1, INI_FILE_PATH);
    jingles_enabled = GetPrivateProfileInt(L"Settings", L"JinglesEnabled", 1, INI_FILE_PATH);
    tooltips_enabled = GetPrivateProfileInt(L"Settings", L"TooltipsEnabled", 1, INI_FILE_PATH);
    remainlog_enabled = GetPrivateProfileInt(L"Settings", L"RemainLogEnabled", 1, INI_FILE_PATH);
    autoreset_enabled = GetPrivateProfileInt(L"Settings", L"AutoResetEnabled", 0, INI_FILE_PATH);
    autotoggle_enabled = GetPrivateProfileInt(L"Settings", L"AutoToggleEnabled", 0, INI_FILE_PATH);
    legacy_jingle_enabled = GetPrivateProfileInt(L"Settings", L"LegacyJingleEnabled", 0, INI_FILE_PATH);
    contfresh_enabled = GetPrivateProfileInt(L"Settings", L"ContFreshEnabled", 0, INI_FILE_PATH);
    contfresh_strength = GetPrivateProfileInt(L"Settings", L"ContFreshStrengthValue", 0, INI_FILE_PATH);
}

void SaveLog(savetype type)
{
    bool current_found = false;
    for (int i = 0; i < MAX_CIVS; i++)
    {
        if (drawn_civs[i] == current_civ) current_found = true;
        else if (!current_found) continue;
		if (drawn_civs[i] != current_civ) drawn_civs[i] = L"";
    }

    std::wstring saveFilePath;

    wchar_t exePath[MAX_PATH];
    GetModuleFileName(NULL, exePath, MAX_PATH);

    wchar_t* lastSlash = wcsrchr(exePath, L'\\');
    if (lastSlash != NULL) *lastSlash = L'\0';

    PathAppend(exePath, L"saves");

    switch (type)
    {
        case automatic:
        {
            saveFilePath = LOG_FILE_PATH;
            break;
        }
        case quick:
        {
            PathAppend(exePath, L"quicksave.txt");

            saveFilePath = exePath;
            break;
        }

        case manual:
        {
            PlayAudio(button);

            SYSTEMTIME st;
            GetLocalTime(&st);

            std::wstringstream wss;
            wss << L"FRCP Preset v" << VERSION << " @"
                << std::setw(4) << std::setfill(L'0') << st.wYear << L"."
                << std::setw(2) << std::setfill(L'0') << st.wMonth << L"."
                << std::setw(2) << std::setfill(L'0') << st.wDay << L" "
                << std::setw(2) << std::setfill(L'0') << st.wHour
                << std::setw(2) << std::setfill(L'0') << st.wMinute
                << std::setw(2) << std::setfill(L'0') << st.wSecond << L".txt";

            std::wstring defaultFileName = wss.str();

            OPENFILENAME ofn;

            wchar_t szFile[MAX_PATH] = L"";
            wcscpy_s(szFile, defaultFileName.c_str());

            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = NULL;
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = sizeof(szFile);
            ofn.lpstrFilter = L"Text Files\0*.txt\0All Files\0*.*\0";
            ofn.nFilterIndex = 1;
            ofn.lpstrFileTitle = NULL;
            ofn.nMaxFileTitle = 0;
            ofn.lpstrInitialDir = exePath;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;

            if (GetSaveFileName(&ofn) == TRUE)
            {
                PlayAudio(button);
                saveFilePath = ofn.lpstrFile;
                if (saveFilePath.find(L".txt") == std::wstring::npos)
                {
                    saveFilePath.append(L".txt");
                }
            }
            else
            {
                PlayAudio(button);
                return;
            }
            break;
        }
    }

    std::wofstream outFile(saveFilePath);

    if (!outFile) return;
    

    outFile << L"CivStates:" << std::endl;
	for (int i = 0; i < MAX_CIVS; i++) outFile << civ[i].name << L" " << (civ[i].enabled ? L"1" : L"0") << std::endl;
    outFile << L"DrawnCivs:" << std::endl;
	for (int i = 0; i < MAX_CIVS; i++)
	{
        if (drawn_civs[i] != L"") outFile << drawn_civs[i] << std::endl;        
        else break;
	}
    outFile << L"EditionState:" << std::endl;
    switch (edition_state)
    {
	    case DE: outFile << L"DE" << std::endl; break;
	    case HD: outFile << L"HD" << std::endl; break;
	    case AOK: outFile << L"AOK" << std::endl; break;
    }
    if (contfresh_enabled)
    {
        outFile << L"ContfreshCivArray:" << std::endl;
        for (int i = 0; i < MAX_CIVS; i++)
        {
            if (contfresh_array[i] != L"") outFile << contfresh_array[i] << std::endl;
            else break;
        }
    }

    outFile.close();
}

void LoadLog(HWND hWnd, savetype type)
{
    std::wstring loadFilePath = LOG_FILE_PATH;

    if (type != automatic)
    {
        PlayAudio(button);

        wchar_t exePath[MAX_PATH];
        GetModuleFileName(NULL, exePath, MAX_PATH);

        wchar_t* lastSlash = wcsrchr(exePath, L'\\');
        if (lastSlash != NULL) *lastSlash = L'\0';

        PathAppend(exePath, L"saves");

        if (type == quick)
        {

            PathAppend(exePath, L"quicksave.txt");

            loadFilePath = exePath;

			if (!FileExists(loadFilePath))return;

        }
        else if (type == manual)
        {
            OPENFILENAME ofn;
            wchar_t szFile[MAX_PATH] = L"";
            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hWnd;
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = sizeof(szFile);
            ofn.lpstrFilter = L"Text Files\0*.txt\0All Files\0*.*\0";
            ofn.nFilterIndex = 1;
            ofn.lpstrFileTitle = NULL;
            ofn.nMaxFileTitle = 0;
            ofn.lpstrInitialDir = exePath;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;

            if (GetOpenFileName(&ofn) == TRUE) loadFilePath = ofn.lpstrFile;
            else
            {
                PlayAudio(button);
                startup = false;
                return;
            }
        }

        SetWindowText(remaining_log, L"");
        SetWindowText(drawn_log, L"");
        SetEditionState(hWnd, DE);
        EnableAll(hWnd, false);
        ResetProgram(true);
    }

    std::wifstream inFile(loadFilePath);

    if (!inFile) return;

    std::wstring line;
    bool readingDrawnCivs = false;
    bool readingCivStates = false;
    bool readingEditionState = false;
    bool readingContfreshArray = false;
    if (type == automatic) InitialiseCivs();

    bool stateread_commenced = false;
	bool drawnread_commenced = false;
    bool editionread_commenced = false;
    bool contfreshread_commenced = false;
    int i = 0;
    int j = 0;
    int stop = MAX_CIVS*2+4;
    while (std::getline(inFile, line))
    {
		if (i >= stop || line == L"") break;
        if (line == L"CivStates:" && !stateread_commenced)
        {
            j = 0;
            readingCivStates = true;
            readingDrawnCivs = false;
			readingEditionState = false;
            readingContfreshArray = false;
			stateread_commenced = true;
            continue;
        }
        else if (contfresh_enabled && line == L"ContfreshCivArray:" && !contfreshread_commenced)
        {
            j = 0;
            readingCivStates = false;
            readingDrawnCivs = false;
            readingEditionState = false;
            readingContfreshArray = true;
            contfreshread_commenced = true;
            continue;
        }
        else if (line == L"DrawnCivs:" && !drawnread_commenced)
        {
            j = 0;
            readingCivStates = false;
            readingDrawnCivs = true;
			readingEditionState = false;
            readingContfreshArray = false;
			drawnread_commenced = true;
            continue;
        }
        else if (line == L"EditionState:" && !editionread_commenced)
        {
            readingCivStates = false;
            readingDrawnCivs = false;
            readingEditionState = true;
            readingContfreshArray = false;
            continue;
        }

		else if (line == L"") continue;
        if (readingCivStates)
        {
			if (j >= MAX_CIVS) break;
            std::wistringstream iss(line);
            std::wstring civName;
            int state;
            if (iss >> civName >> state)
            {
                if (state == 0)
                {
                    RemoveCiv(civName);
                    SendMessage(GetCivCheckbox(civName), BM_SETCHECK, BST_UNCHECKED, 0);
                }
                else SendMessage(GetCivCheckbox(civName), BM_SETCHECK, BST_CHECKED, 0);
            }
            j++;
        }
        else if (contfresh_enabled && readingContfreshArray)
        {
            if (j >= MAX_CIVS) break;
			contfresh_array[j] = line;
            j++;
        }
        else if (readingDrawnCivs)
        {
            if (j >= MAX_CIVS) break;
            DrawCiv(true, line); 
            j++;
		}
		else if (readingEditionState)
		{
            bool temp_toggleoff = false;
            if (autotoggle_enabled)
            {
                temp_toggleoff = true;
                autotoggle_enabled = false;
            }
			if (line == L"DE") SetEditionState(hWnd, DE);			
			else if (line == L"HD") SetEditionState(hWnd, HD);			
			else if (line == L"AOK") SetEditionState(hWnd, AOK);
            if (temp_toggleoff && !autotoggle_enabled) autotoggle_enabled = true;
		}
		i++;
    }

    label_text = std::to_wstring(iterator) + L"/" + std::to_wstring(custom_max_civs);
    std::wstring remain_text = std::to_wstring(custom_max_civs - iterator) + L"/" + std::to_wstring(custom_max_civs);
    SetWindowText(label_corner, label_text.c_str());
	if (!iteration_label_enabled) ShowWindow(label_corner, SW_HIDE);
    std::wstring drawn_label = L"Drawn: " + label_text;
    if (iterator == 0) SetWindowText(label_drawncount, drawn_label.c_str());

    HBITMAP drawn_civ_icon = FetchCivIcon(current_civ);
    SendMessageW(civ_icon, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)drawn_civ_icon);
	if (!icons_enabled) ShowWindow(civ_icon, SW_HIDE);

    UpdateRemainingLog(true);

    if (type == automatic && current_civ == L"Random")
    {
        UpdateTooltips(false);
        SetWindowText(label_centre, L"?");
    }        
    else
    {
        UpdateTooltips(true);
        SetWindowText(label_centre, current_civ.c_str());		
    }
    if (!civ_labels_enabled) ShowWindow(label_centre, SW_HIDE);

    inFile.close();

    if (type != automatic || cache_loading)
    {
        ShowTabComponents(current_tab, hWnd);
        ValidateAllDlcToggles(hWnd);
        startup = false;
    }
    if (iterator > 0) reset_state = false;
}

void GenerateFilePaths()
{
    wchar_t exePathS[MAX_PATH];
    wchar_t exePathL[MAX_PATH];
	wchar_t exePathH[MAX_PATH];
    GetModuleFileName(NULL, exePathS, MAX_PATH);
    GetModuleFileName(NULL, exePathL, MAX_PATH);
	GetModuleFileName(NULL, exePathH, MAX_PATH);

    wchar_t *lastSlashS = wcsrchr(exePathS, L'\\');
    if (lastSlashS != NULL) *lastSlashS = L'\0';
    wchar_t *lastSlashL = wcsrchr(exePathL, L'\\');
    if (lastSlashL != NULL) *lastSlashL = L'\0';
	wchar_t* lastSlash = wcsrchr(exePathH, L'\\');
	if (lastSlash != NULL) *lastSlash = L'\0';
    
    wcscat_s(exePathS, MAX_PATH, L"\\settings.ini");
    wcscat_s(exePathL, MAX_PATH, L"\\log.txt");
	wcscat_s(exePathH, MAX_PATH, L"\\Histories\\");

    wcscpy_s(INI_FILE_PATH, exePathS);
    wcscpy_s(LOG_FILE_PATH, exePathL);
	wcscpy_s(HIS_FILE_PATH, exePathH);
}

HWND GetCivCheckbox(const std::wstring &civ_name) { for (int i = 0; i < MAX_CIVS; i++) if (civ[i].name == civ_name) return civ[i].checkbox; return NULL; }

void InitialiseCivs()
{
    for (int i = 0; i < MAX_CIVS; i++)
    {
        civ_list[i] = civ[i].name;
        available_civs[i] = true;
    }
        
    //if (!civs.empty()) civs.clear();
    //for (int i = 0; i < MAX_CIVS; i++) civs.push_back(civ[i].name);
}

void OpenOptions(HWND hWnd) { PlayAudio(button); DialogBox(instance, MAKEINTRESOURCE(IDD_OPTIONS), hWnd, OptionsDlgProc); }

void OpenHotkeys(HWND hWnd) { PlayAudio(button); DialogBox(instance, MAKEINTRESOURCE(IDD_HOTKEYS), hWnd, HotkeysDlgProc); }

void OpenAbout(HWND hWnd) { PlayAudio(button); DialogBox(instance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, AboutDlgProc); }

INT_PTR CALLBACK HotkeysDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:
        {
            CreateBoldFont();

            SendMessage(GetDlgItem(hDlg, IDC_STATIC_GLOBAL_HOTKEYS), WM_SETFONT, (WPARAM)font_bold, TRUE);
            SendMessage(GetDlgItem(hDlg, IDC_STATIC_DRAW_CIV_HOTKEYS), WM_SETFONT, (WPARAM)font_bold, TRUE);
            SendMessage(GetDlgItem(hDlg, IDC_STATIC_LOG_TAB_HOTKEYS), WM_SETFONT, (WPARAM)font_bold, TRUE);
            SendMessage(GetDlgItem(hDlg, IDC_STATIC_CIV_POOL_HOTKEYS), WM_SETFONT, (WPARAM)font_bold, TRUE);

            return(INT_PTR)TRUE;
        }
        case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            int wmEvent = HIWORD(wParam);

            if (wmId == IDCANCEL || wmId == IDOK)
            {
                PlayAudio(button);
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
    SubclassButton(button_survapp);
	SubclassButton(button_options);
    SubclassButton(button_history);

    SubclassButton(button_enableall);
    SubclassButton(button_disableall);

    SubclassButton(radiobutton_de);
    SubclassButton(radiobutton_hd);
    SubclassButton(radiobutton_aok);

    SubclassButton(checkbox_autoreset);
    SubclassButton(checkbox_autotoggle);

    for (int i = 0; i < de_dlc_amount; i++) SubclassButton(de_dlc_checkbox[i]);
    for (int i = 0; i < hd_dlc_amount; i++) SubclassButton(hd_dlc_checkbox[i]);
    SubclassButton(checkbox_aoc);
}

void CreateTooltips(HWND hWnd)
{
    for (int i = 0; i < hwnd_length; i++) hwndTooltip[i] = CreateWindowEx(0, TOOLTIPS_CLASS, NULL, WS_POPUP | TTS_ALWAYSTIP | TTS_NOPREFIX, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hWnd, NULL, instance, NULL);
    
    if (!hwndTooltip)
    {
        MessageBox(hWnd, L"Failed to create tooltip window.", L"Error", MB_OK | MB_ICONERROR);
        return;
    }
    for (int i = 0; i < hwnd_length; i++) SendMessage(hwndTooltip[i], TTM_SETMAXTIPWIDTH, 0, 300);
}

void AddTooltips()
{
    AddTooltip(button_draw, hwndTooltip[TOOLTIP_DRAW], StringCleaner(L"Draws a fresh random civilisation\nHotkey: Space"));
    AddTooltip(button_reset, hwndTooltip[TOOLTIP_RESET], StringCleaner(L"Resets the pool of drawn civs and renders all enabled civs available\nHotkey: Enter"));
    AddTooltip(button_options, hwndTooltip[TOOLTIP_OPTIONS], StringCleaner(L"Opens options\nHotkey: F1"));
    AddTooltip(button_techtree, hwndTooltip[TOOLTIP_TECHTREE], StringCleaner(L"Opens the tech tree\nHotkey: T (Draw tab only) / F4"));
    AddTooltip(button_survapp, hwndTooltip[TOOLTIP_SURVAPP], StringCleaner(L"Opens Survivalist's webapp to calculate villagers required for sustainable production\nHotkey: R (Draw tab only) / Ctrl + T"));
    AddTooltip(button_history, hwndTooltip[TOOLTIP_HISTORY], StringCleaner(L"Opens the game's History info\nHotkey: H (Draw tab only)"));

    AddTooltip(button_clearlog, hwndTooltip[TOOLTIP_CLEAR], StringCleaner(L"Clears the log of previously drawn civs\nHotkey: Q"));
    AddTooltip(checkbox_showremainlog, hwndTooltip[TOOLTIP_REMAININGTOGGLE], StringCleaner(L"Toggles the display of the remaining civs log\nHotkey: W"));

    AddTooltip(checkbox_autotoggle, hwndTooltip[TOOLTIP_AUTOTOGGLE], StringCleaner(L"Toggles older civs as well as the automatic enabling of older civs when switching between editions\nHotkey: R"));
    AddTooltip(checkbox_autoreset, hwndTooltip[TOOLTIP_AUTORESET], StringCleaner(L"Automatically resets the pool of drawn civs when the civilisation pool is changed\nHotkey: T"));

    AddTooltip(button_enableall, hwndTooltip[TOOLTIP_ENABLEALL], StringCleaner(L"Enables all civilisations so that they are made available in the pool of civilisations for drawing\nHotkey: Space"));
    AddTooltip(button_disableall, hwndTooltip[TOOLTIP_DISABLEALL], StringCleaner(L"Disables all civilisations so that they are removed from the pool of civilisations for drawing\nHotkey: Enter"));

    AddTooltip(radiobutton_de, hwndTooltip[TOOLTIP_DE], StringCleaner(L"Selects the Definitive Edition (2019) civilisation pool\nHotkey: Q"));
    AddTooltip(radiobutton_hd, hwndTooltip[TOOLTIP_HD], StringCleaner(L"Selects the HD Edition (2013) civilisation pool\nHotkey: W"));
    AddTooltip(radiobutton_aok, hwndTooltip[TOOLTIP_AOK], StringCleaner(L"Selects the Age of Kings (1999) civilisation pool\nHotkey: E"));

    for (int i = 0; i < de_dlc_amount; i++) AddTooltip(de_dlc_checkbox[i], hwndTooltip[de_dlc_tipid[i]], StringCleaner(de_dlc_tooltip[i]));
    for (int i = 0; i < hd_dlc_amount; i++) AddTooltip(hd_dlc_checkbox[i], hwndTooltip[hd_dlc_tipid[i]], StringCleaner(hd_dlc_tooltip[i]));
    AddTooltip(checkbox_aoc, hwndTooltip[TOOLTIP_AOC], StringCleaner(L"Toggles The Conquerors civilisations\n(Aztecs, Huns, Koreans, Mayans, Spanish)\nHotkey: A"));
}

void CreateCheckboxes(HWND hWnd)
{
    checkbox_showremainlog = CreateCheckbox(hWnd, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), 10, 230, 180, 20, IDC_CHECKBOX_REMAINLOG, L"Show");
    CheckDlgButton(hWnd, IDC_CHECKBOX_REMAINLOG, remainlog_enabled ? BST_CHECKED : BST_UNCHECKED);

    int row[] = { 30, 50, 70, 90, 110, 130, 150, 170, 190, 210 };
    int column[] = { 10, 112, 214, 316, 418 };

    for (int i = 0; i < MAX_CIVS; i++) civ[i].checkbox = CreateCheckbox(hWnd, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), column[i % 5], row[i / 5] + 10, 100, 20, i + 5, civ[i].name.c_str());

    checkbox_autoreset = CreateCheckbox(hWnd, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), 310, 260, 180, 20, IDC_CHECKBOX_AUTORESET, L"Auto-reset upon change");
    checkbox_autotoggle = CreateCheckbox(hWnd, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), 10, 10, 170, 20, IDC_CHECKBOX_AUTOTOGGLE, L"Auto-toggle older civs");

    if (!autoreset_enabled) CheckDlgButton(hWnd, IDC_CHECKBOX_AUTORESET, BST_UNCHECKED);
    if (!autotoggle_enabled) CheckDlgButton(hWnd, IDC_CHECKBOX_AUTOTOGGLE, BST_UNCHECKED);

    for (int i = 0; i < de_dlc_amount; i++) de_dlc_checkbox[i] = CreateCheckbox(hWnd, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), 368, (de_dlc_boxrow[i] - 10), 160, 20, de_dlc_id[i], StringCleaner(de_dlc_name[i]));
    for (int i = 0; i < hd_dlc_amount; i++) hd_dlc_checkbox[i] = CreateCheckbox(hWnd, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), 383, (hd_dlc_boxrow[i] - 10), 160, 20, hd_dlc_id[i], StringCleaner(hd_dlc_name[i]));

    checkbox_aoc = CreateCheckbox(hWnd, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), 388, -46, 140, 20, IDC_CHECKBOX_AOC, L"The Conquerors");
}

void CreateImages(HWND hWnd)
{
    civ_icon = CreateWindow(L"BUTTON", L"", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | BS_BITMAP, 0, 0, 60, 60, hWnd, (HMENU)IDC_ICON_CIV, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);

    edition_icon = CreateWindowW(L"BUTTON", L"", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | BS_BITMAP, 0, 0, 60, 60, hWnd, (HMENU)IDC_ICON_EDITION, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);

    for (int i = 0; i < de_dlc_amount; i++) de_dlc_icon[i] = CreateWindow(L"STATIC", NULL, WS_VISIBLE | WS_CHILD | SS_BITMAP, 338, 0, 0, 0, hWnd, NULL, NULL, NULL);
    for (int i = 0; i < hd_dlc_amount; i++) hd_dlc_icon[i] = CreateWindow(L"STATIC", NULL, WS_VISIBLE | WS_CHILD | SS_BITMAP, 338, 10, 0, 0, hWnd, NULL, NULL, NULL);
    aoc_icon = CreateWindow(L"STATIC", NULL, WS_VISIBLE | WS_CHILD | SS_BITMAP, 338, 10, 0, 0, hWnd, NULL, NULL, NULL);

    for (int i = 0; i < de_dlc_amount; i++) SendMessage(de_dlc_icon[i], STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)de_dlc_bmp[i]);
    for (int i = 0; i < hd_dlc_amount; i++) SendMessage(hd_dlc_icon[i], STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hd_dlc_bmp[i]);
    SendMessage(aoc_icon, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)icon_aoc);
}

void CreateButtons(HWND hWnd)
{
    button_history = CreateWindow(L"BUTTON", L"History", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | BS_BITMAP,
        60, 60, 83, 60, hWnd, (HMENU)IDC_BUTTON_HISTORY, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
    SendMessageW(button_history, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)icon_history);


    button_draw = CreateWindow(L"BUTTON", L"Draw", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | TTF_TRACK, 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT, hWnd, (HMENU)IDC_BUTTON_DRAW, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
    button_reset = CreateWindow(L"BUTTON", L"Reset", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 10, 200, BUTTON_WIDTH, BUTTON_HEIGHT, hWnd, (HMENU)IDC_BUTTON_RESET, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
    button_survapp = CreateWindow(L"BUTTON", L"", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | BS_BITMAP, 0, 0, 50, 50, hWnd, (HMENU)IDC_BUTTON_SURVAPP, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
    SendMessageW(button_survapp, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)icon_survapp);
    button_techtree = CreateWindow(L"BUTTON", L"", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | BS_BITMAP, 0, 0, 60, 60, hWnd, (HMENU)IDC_BUTTON_TECHTREE, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
    SendMessageW(button_techtree, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)icon_techtree);
    button_options = CreateWindow(L"BUTTON", L"", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | BS_BITMAP, 0, 0, 25, 25, hWnd, (HMENU)IDC_BUTTON_OPTIONS, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
    SendMessageW(button_options, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)icon_options); 
    button_clearlog = CreateWindow(L"BUTTON", L"Clear Log", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT, hWnd, (HMENU)IDC_BUTTON_CLEARLOG, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
    button_enableall = CreateWindow(L"BUTTON", L"Enable All", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 10, 380, BUTTON_WIDTH, BUTTON_HEIGHT, hWnd, (HMENU)IDC_BUTTON_ENABLEALL, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
    button_disableall = CreateWindow(L"BUTTON", L"Disable All", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 125, 380, BUTTON_WIDTH, BUTTON_HEIGHT, hWnd, (HMENU)IDC_BUTTON_DISABLEALL, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
}

void CreateLabels(HWND hWnd)
{
    label_corner = CreateWindow(L"STATIC", L"", WS_VISIBLE | WS_CHILD, 0, 0, 30, 15, tab, NULL, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
    label_centre = CreateWindow(L"STATIC", L"?", WS_VISIBLE | WS_CHILD, 0, 0, 90, 15, hWnd, NULL, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
    label_drawncount = CreateWindow(L"STATIC", L"", WS_VISIBLE | WS_CHILD, 100, 25, 100, 15, hWnd, NULL, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
    label_remainingcount = CreateWindow(L"STATIC", L"", WS_VISIBLE | WS_CHILD, 0, 25, 100, 15, hWnd, NULL, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
	label_enabledcount = CreateWindow(L"STATIC", StringCleaner(L"Enabled: " + enabledcount), WS_VISIBLE | WS_CHILD, 200, 140, 100, 15, hWnd, NULL, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
}

void CreateTextfields(HWND hWnd)
{
    drawn_log = CreateWindow(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY, 10, 25, 350, 200, tab, NULL, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
    remaining_log = CreateWindow(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY, 10, 25, 350, 200, tab, NULL, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
}

void CreateRadiobuttons(HWND hWnd)
{
    radiobutton_de = CreateWindow(L"BUTTON", L"Definitive Edition (2019)", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON, 10, 40, 175, 20, hWnd, (HMENU)IDC_RADIO_DE, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
    radiobutton_hd = CreateWindow(L"BUTTON", L"HD Edition (2013)", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON, 10, 70, 175, 20, hWnd, (HMENU)IDC_RADIO_HD, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
    radiobutton_aok = CreateWindow(L"BUTTON", L"Age of Kings (1999)", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON, 10, 100, 175, 20, hWnd, (HMENU)IDC_RADIO_AOK, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
    CheckRadioButton(hWnd, IDC_RADIO_DE, IDC_RADIO_AOK, IDC_RADIO_DE);
}

void DeactivateTooltips(TTTOOLINFOW toolinfo) { for (int i = 0; i < hwnd_length; i++) SendMessage(hwndTooltip[i], TTM_TRACKACTIVATE, FALSE, (LPARAM)&toolinfo); }

void CreateBoldFont()
{
    LOGFONT lf;
    HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    GetObject(hFont, sizeof(LOGFONT), &lf);
    lf.lfWeight = FW_BOLD;
    font_bold = CreateFontIndirect(&lf);
}

void PositionComponents(LPARAM lParam)
{
    int width = LOWORD(lParam);
    int height = HIWORD(lParam);

    SetWindowPos(button_draw, NULL, (width - 100) / 2, (height + 100) / 2, 100, 30, SWP_NOZORDER);
    SetWindowPos(button_techtree, NULL, width - 150, height - 120, 60, 60, SWP_NOZORDER);
    SetWindowPos(button_survapp, NULL, width - 100, 50, 50, 50, SWP_NOZORDER);
    SetWindowPos(button_options, NULL, width - 35, 0, 25, 25, SWP_NOZORDER);


    if (current_tab == 0)  SetWindowPos(button_reset, NULL, 10, height - 40, 100, 30, SWP_NOZORDER);
    else SetWindowPos(button_reset, NULL, (width - 100) / 2, height - 40, 100, 30, SWP_NOZORDER);


    SetWindowPos(tab, NULL, 0, 0, width, height, SWP_NOZORDER);
    SetWindowPos(label_corner, NULL, width - 50, height - 20, 40, 15, SWP_NOZORDER);
    SetWindowPos(label_centre, NULL, (width - 80) / 2, (height + 35) / 2, 90, 15, SWP_NOZORDER);
    SetWindowPos(civ_icon, NULL, (width - 100) / 2, (height - 180) / 2, 104, 104, SWP_NOZORDER);

    SetWindowPos(label_drawncount, NULL, 10, 25, 90, 15, SWP_NOZORDER);
    SetWindowPos(button_clearlog, NULL, 110, 25, 100, 30, SWP_NOZORDER);
    SetWindowPos(drawn_log, NULL, 10, 60, width - (width / 2) - 69, height - 70, SWP_NOZORDER);

    SetWindowPos(label_remainingcount, NULL, (width / 2) + 60, 25, 130, 15, SWP_NOZORDER);
    SetWindowPos(checkbox_showremainlog, NULL, (width / 2) + 190, 25, 60, 15, SWP_NOZORDER);
    SetWindowPos(remaining_log, NULL, (width / 2) + 60, 60, width - (width / 2) - 65, height - 70, SWP_NOZORDER);


    for (int i = 0; i < de_dlc_amount; i++) SetWindowPos(de_dlc_icon[i], NULL, 345, (de_dlc_row[i]), 18, 18, SWP_NOZORDER);
    for (int i = 0; i < hd_dlc_amount; i++) SetWindowPos(hd_dlc_icon[i], NULL, 350, (hd_dlc_row[i]), 30, 30, SWP_NOZORDER);
    SetWindowPos(aoc_icon, NULL, 340, 70, 45, 45, SWP_NOZORDER);

    SetWindowPos(edition_icon, NULL, 190, 30, 128, 93, SWP_NOZORDER);
}

void ClearDrawnLog() { if (!startup) drawn_log_linecount = 0; PlayAudio(button); SetWindowText(drawn_log, L""); }

void JoinLobby(HWND hWnd)
{
    PlayAudio(button);
    std::wstring lobbyCode;
    int result = DialogBoxParam(instance, MAKEINTRESOURCE(IDD_JOINLOBBY_DIALOG), hWnd, JoinLobbyDlgProc, reinterpret_cast<LPARAM>(&lobbyCode));
    if (result == IDC_BUTTON_OK)
    {
		if (lobbyCode.length() == 9) ShellExecute(NULL, L"open", StringCleaner(L"aoe2de://0/" + lobbyCode), NULL, NULL, SW_SHOWNORMAL);
        else
        {
			if (lobbyCode.substr(0, 11) == L"aoe2de://1/") lobbyCode.replace(0, 11, L"aoe2de://0/");
            ShellExecute(NULL, L"open", lobbyCode.c_str(), NULL, NULL, SW_SHOWNORMAL);
        }
    }
    else if (result == IDC_BUTTON_SPEC)
    {
        if (lobbyCode.length() == 9) ShellExecute(NULL, L"open", StringCleaner(L"aoe2de://1/" + lobbyCode), NULL, NULL, SW_SHOWNORMAL);
        else
        {
            if (lobbyCode.substr(0, 11) == L"aoe2de://0/") lobbyCode.replace(0, 11, L"aoe2de://1/");
            ShellExecute(NULL, L"open", lobbyCode.c_str(), NULL, NULL, SW_SHOWNORMAL);
        }
    }
}



INT_PTR CALLBACK JoinLobbyDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static std::wstring *lobby_code;

    switch (message)
    {
        case WM_INITDIALOG:
            lobby_code = reinterpret_cast<std::wstring *>(lParam);
            return (INT_PTR)TRUE;
        case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            int wmEvent = HIWORD(wParam);
            if (wmId == IDCANCEL)
            {
                PlayAudio(button);
                EndDialog(hDlg, wmId);
                return(INT_PTR)TRUE;
            }
            if (LOWORD(wParam) == IDC_BUTTON_OK)
            {
                PlayAudio(button);
                wchar_t buffer[256];
                GetDlgItemText(hDlg, IDC_EDIT_LOBBYCODE, buffer, 256);
                *lobby_code = buffer;
                if (!IsValidLobbyCode(*lobby_code))
                {
                    MessageBox(hDlg, L"Invalid lobby code entered.\n\nInput code must be of a format which is either 'aoe2de://0/123456789' or '123456789'. ", L"Error: invalid input", MB_OK | MB_ICONERROR);
                    return (INT_PTR)FALSE;
                }
                EndDialog(hDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            }
            else if (LOWORD(wParam) == IDC_BUTTON_SPEC)
            {
				PlayAudio(button);
				wchar_t buffer[256];
				GetDlgItemText(hDlg, IDC_EDIT_LOBBYCODE, buffer, 256);
				*lobby_code = buffer;
				if (!IsValidLobbyCode(*lobby_code))
				{
					MessageBox(hDlg, L"Invalid lobby code entered.\n\nInput code must be of a format which is either 'aoe2de://1/123456789' or '123456789'. ", L"Error: invalid input", MB_OK | MB_ICONERROR);
					return (INT_PTR)FALSE;
				}
				EndDialog(hDlg, LOWORD(wParam));
				return (INT_PTR)TRUE;
            }
            else if (LOWORD(wParam) == IDC_BUTTON_CANCEL)
            {
                PlayAudio(button);
                EndDialog(hDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            }
            break;
        }

    }
    return (INT_PTR)FALSE;
}

bool IsValidLobbyCode(const std::wstring &lobby_code)
{
    size_t lngth = lobby_code.length();
    switch (lngth)
    {
        case 9: break;
        case 20: break;
        default: return false;
    }

    if (lngth == 20)
    {
        if (lobby_code.substr(0, 11) != L"aoe2de://0/" && lobby_code.substr(0, 11) != L"aoe2de://1/") return false;
        std::wstring numbers = lobby_code.substr(12, 20);
        if (numbers.find_first_not_of(L"0123456789") != std::wstring::npos) return false;
    }

    if (lngth == 9) if (lobby_code.find_first_not_of(L"0123456789") != std::wstring::npos) return false;   

	return true;
}

void Undo(HWND hWnd)
{
    switch (last_action)
    {
    case draw:
        if (iterator > 0) UndrawCiv();
        else if (persistent_logging && cache_available)
        {
            cache_loading = true;
            LoadLog(hWnd, automatic);
            cache_available = false;
            cache_loading = false;
        }
        return;
    case reset:
        if (persistent_logging && cache_available)
        {
            cache_loading = true;
            LoadLog(hWnd, automatic);
            cache_available = false;
            cache_loading = false;
        }
        return;
    case pool_change:
        return;
    default:
        return;
    }
}

void UndrawCiv()
{
    if (!undrawable)
    {
		PlayAudio(error);
        return;
    }
	PlayAudio(button);
    if (iterator > 0)
    {
        iterator--;
        remaining++;
    }
    else return;
    civ_list[GetCivIndex(current_civ)] = current_civ;
    available_civs[GetCivIndex(current_civ)] = true;
    if (iterator >= 1) current_civ = drawn_civs[iterator - 1];
    else current_civ = L"Random";
	HBITMAP drawn_civ_icon = FetchCivIcon(current_civ);
	SendMessageW(civ_icon, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)drawn_civ_icon);
    PlayJingle(current_civ);
    if (current_civ == L"Random") SetWindowText(label_centre, L"?");
    else SetWindowText(label_centre, current_civ.c_str());	
    if (!civ_labels_enabled) ShowWindow(label_centre, SW_HIDE);
    SetWindowText(label_drawncount, StringCleaner(L"Drawn: " + std::to_wstring(iterator) + L"/" + std::to_wstring(custom_max_civs)));
    SetWindowText(label_corner, StringCleaner(std::to_wstring(iterator) + L"/" + std::to_wstring(custom_max_civs)));
    if (!iteration_label_enabled || current_tab != 0) ShowWindow(label_corner, SW_HIDE);
    if (iterator == 0) SendMessageW(civ_icon, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)icon_random);

    int drawnlog_length = GetWindowTextLength(drawn_log);
    if (drawnlog_length > 0)
    {
        std::wstring drawnlog_text(drawnlog_length, L'\0');
        GetWindowText(drawn_log, &drawnlog_text[0], drawnlog_length + 1);
        size_t pos = drawnlog_text.find(L"\r\n");
        if (pos != std::wstring::npos)
        {
            drawnlog_text = drawnlog_text.substr(pos + 2);
            SetWindowText(drawn_log, drawnlog_text.c_str());
            drawn_log_linecount--;
            //drawn_log_linecount++;
        }
    }
    UpdateTooltips(true);

	UpdateRemainingLog(true);
    redrawable = true;
}

void RedrawCiv()
{
    if (!redrawable || drawn_civs[iterator] == L"" || !available_civs[GetCivIndex(drawn_civs[iterator])])
    {
		PlayAudio(error);
        return;
    }
    current_civ = drawn_civs[iterator];
    civ_list[GetCivIndex(current_civ)] = L"";
    available_civs[GetCivIndex(current_civ)] = false;
    iterator++;
    remaining--;
	HBITMAP drawn_civ_icon = FetchCivIcon(current_civ);
	SendMessageW(civ_icon, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)drawn_civ_icon);
    PlayJingle(current_civ);
	SetWindowText(label_centre, current_civ.c_str());
    SetWindowText(label_corner, StringCleaner(std::to_wstring(iterator) + L"/" + std::to_wstring(custom_max_civs)));
    if (!iteration_label_enabled || current_tab != 0) ShowWindow(label_corner, SW_HIDE);
    UpdateDrawnLog(false, true, false);
    UpdateRemainingLog(false);

    UpdateTooltips(true);


    if (iterator == custom_max_civs) redrawable = false;
}

Civ &GetCiv(const std::wstring &name) { for (int i = 0; i < MAX_CIVS; i++) if (civ[i].name == name) return civ[i]; return random; }

void PlayAudio(sound_type type)
{
    if (!ui_sounds_enabled) return;
    switch (type) {
    case button: PlayResource(soundResources[0]); return;
    case tabsound: PlayResource(soundResources[1]); return;
    case hover: PlayResource(soundResources[2]); return;
    case error: PlayResource(soundResources[3]); return;
    case view: PlayResource(soundResources[4]); return;
    }
}

void PlayJingle(const std::wstring& civ_name)
{

    //TODO: make jingle a member variable of Civ so loops in this function are redundant



    if (!jingles_enabled) return;

    if (civ_name == L"Random")
    {
        if (legacy_jingle_enabled) PlayResource(random_legacy_jingle);
        else PlayResource(random_jingle);
        return;
    }

    if (legacy_jingle_enabled && GetCiv(civ_name).legacy)
    {
        std::wstring legacy_civ[LEGACY_JINGLE_AMOUNT] = { L"" };
        int j = 0;
        for (int i = 0; i < MAX_CIVS; i++)
        {
            if (civ[i].legacy)
            {
                legacy_civ[j] = civ[i].name;
                j++;
            }
        }
		for (int i = 0; i < LEGACY_JINGLE_AMOUNT; i++) if (legacy_civ[i] == civ_name) PlayResource(legacyJingle[i]);
    }
    else for (int i = 0; i < MAX_CIVS; i++) if (civ[i].name == civ_name) PlayResource(jingleResource[i]);



    //std::wstring processed_civ_name = civ_name;
    //processed_civ_name[0] = std::tolower(processed_civ_name[0]);
    //std::wstring jingle_path;

    //if (legacy_jingle_enabled && GetCiv(civ_name).legacy) jingle_path = L"sounds\\civ_jingles\\legacy\\" + processed_civ_name + L".wav";
    //else jingle_path = L"sounds\\civ_jingles\\" + processed_civ_name + L".wav";

    //PlaySound(jingle_path.c_str(), NULL, SND_FILENAME | SND_ASYNC);
}

void StopSound() { PlaySound(NULL, NULL, 0); }

BOOL PlayResource(const SoundResource &sound_resource) 
{ 
    BOOL bRtn = sndPlaySound(static_cast<LPCWSTR>(sound_resource.lpRes), SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
    if (!bRtn) {
        MessageBox(NULL, L"Failed to play sound.", L"Error", MB_OK | MB_ICONERROR);
    }
    return bRtn;
}

bool LoadSound(SoundResource& soundResource)
{
    soundResource.hResInfo = FindResource(instance, soundResource.name, L"WAVE");
    if (soundResource.hResInfo == NULL)
    {
        MessageBox(NULL, L"Failed to find resource.", L"Error", MB_OK | MB_ICONERROR);
        return false;
    }

    soundResource.hRes = LoadResource(instance, soundResource.hResInfo);
    if (soundResource.hRes == NULL)
    {
        MessageBox(NULL, L"Failed to load resource.", L"Error", MB_OK | MB_ICONERROR);
        return false;
    }

    soundResource.lpRes = LockResource(soundResource.hRes);
    if (soundResource.lpRes == NULL)
    {
        MessageBox(NULL, L"Failed to lock resource.", L"Error", MB_OK | MB_ICONERROR);
        return false;
    }
    return true;
}


void UnloadSound(SoundResource& soundResource) {
    if (soundResource.lpRes) {
        UnlockResource(soundResource.lpRes);
        soundResource.lpRes = NULL;
    }
    if (soundResource.hRes) {
        FreeResource(soundResource.hRes);
        soundResource.hRes = NULL;
    }
}
void LoadSounds() { for (int i = 0; i < SOUND_AMOUNT; ++i) LoadSound(soundResources[i]); }

void UnloadSounds() { for (int i = 0; i < SOUND_AMOUNT; ++i) UnloadSound(soundResources[i]); }

void LoadJingles()
{

    int j = 0;
    for (int i = 0; i < MAX_CIVS; i++)
    {
        jingleResource[i] = { NULL, NULL, NULL, StringCleaner(civ[i].name) };
        LoadSound(jingleResource[i]);
        if (civ[i].legacy)
        {
			std::wstring l_str = L"l_" + civ[i].name;
			legacyJingle[j] = { NULL, NULL, NULL, StringCleaner(l_str)};
			LoadSound(legacyJingle[j]);
            j++;
        }
    }
	random_jingle = { NULL, NULL, NULL, L"Random" };
	random_legacy_jingle = { NULL, NULL, NULL, L"l_Random" };
    LoadSound(random_jingle);
	LoadSound(random_legacy_jingle);
}

void UnloadJingles()
{
    for (int i = 0; i < MAX_CIVS; i++) UnloadSound(jingleResource[i]);
	for (int i = 0; i < LEGACY_JINGLE_AMOUNT; i++) UnloadSound(legacyJingle[i]);
    UnloadSound(random_jingle);
    UnloadSound(random_legacy_jingle);
}

void UpdateEnabledCivsCounter()
{
    switch (edition_state)
    {
    case DE:
        SetWindowText(label_enabledcount, (L"Enabled: " + std::to_wstring(custom_max_civs) + L"/" + std::to_wstring(MAX_CIVS)).c_str());
        return;
    case HD:
        SetWindowText(label_enabledcount, (L"Enabled: " + std::to_wstring(custom_max_civs) + L"/" + std::to_wstring(MAX_CIVS_HD)).c_str());
        return;
    case AOK:
        SetWindowText(label_enabledcount, (L"Enabled: " + std::to_wstring(custom_max_civs) + L"/" + std::to_wstring(MAX_CIVS_AOK)).c_str());
        return;
    }
}

bool FileExists(const std::wstring &filePath)
{
    DWORD fileAttributes = GetFileAttributes(filePath.c_str());
    return (fileAttributes != INVALID_FILE_ATTRIBUTES &&
        !(fileAttributes & FILE_ATTRIBUTE_DIRECTORY));
}

void UpdateContfresh(const std::wstring& civ)
{
    contfresh_civ = civ;
    if (contfresh_strength == 1 || custom_max_civs < 2) return;
    SetContfreshStrength();
  

    if (contfresh_count >= contfresh_max)
    {
        int decrement = 0;
        std::wstring temp_array[MAX_CIVS] = { L"" };
        

        while (contfresh_count > contfresh_max)
        {
            contfresh_count--;
            decrement++;
        }
        int j = 0;
        for (int i = 0; i < MAX_CIVS; i++)
        {
            if (decrement > 0)
            {
                decrement--;
            }
            else
            {
                temp_array[j] = contfresh_array[i];
                j++;
            }
            contfresh_array[i] = L"";
        }
        for (int i = 0; i < contfresh_max; i++) contfresh_array[i] = temp_array[i];
    }

    for (int i = 0; i < MAX_CIVS; i++)
    {
        if (contfresh_array[i] != L"") continue;
        else
        {
            contfresh_array[i] = civ;
            break;
        }
    }
    contfresh_count++;

    // text string which prints out whole array below
	//std::wstring mb_text = L"";
	//for (int i = 0; i < MAX_CIVS; i++)
	//{
	//	if (contfresh_array[i] != L"") mb_text += contfresh_array[i] + L"\n";
	//}
	// MessageBox to show the contents of the contfresh array
    //if (!startup)
    //{
    //    contfresh_civ = current_civ;
    //    if (contfresh_strength == 1) MessageBox(NULL, StringCleaner(L"Contfresh civ: " + contfresh_civ), L"Info", MB_OK | MB_ICONINFORMATION);
    //    else MessageBox(NULL, StringCleaner(L"Contfresh array:\n" + mb_text), L"Info", MB_OK | MB_ICONINFORMATION);

    //}

}

bool IsContfreshCiv(const std::wstring& civ)
{
    if (custom_max_civs < 2) return true;
    
    if (contfresh_strength == 1)
    {
        if (civ == contfresh_civ) return false;
        else return true;
    }    
    else
    {
        for (int i = 0; i < MAX_CIVS; i++)
        {
            if (contfresh_array[i] == civ) return false;
            if (contfresh_array[i] == L"") break;
        }
        return true;
    }
}

void SetContfreshStrength()
{
    if (contfresh_strength == 10) contfresh_max = (custom_max_civs / 2) - 1;
    else
    {
        float half = custom_max_civs / 2.0f;
        float tenth = half / 10.0f;
        float factor = tenth * contfresh_strength;
        float fl_contfresh_max = factor;
        contfresh_max = static_cast<int>(fl_contfresh_max);
    }

}

void CheckDrawnLogLength()
{
    if (drawn_log_linecount > MAX_DRAWNLOG_LINECOUNT)
    {
        size_t last_newline = drawnlog_text.rfind(L"\r");
        
        if (last_newline != std::wstring::npos)
        {
            drawnlog_text.erase(last_newline);
            //if (reset_state) drawnlog_text.erase(last_newline);

            //if (drawn_log_linecount > 0) drawn_log_linecount--;

			//MessageBox(NULL, StringCleaner(L"drawn_log_linecount: " + std::to_wstring(drawn_log_linecount)), L"Error", MB_OK | MB_ICONINFORMATION);
    
        }
        
    }
}


void log_addentry(std::wstring message)
{
    drawnlog_text = message + drawnlog_text;
    if (drawn_log_linecount == MAX_DRAWNLOG_LINECOUNT)
    {
        size_t last_newline = drawnlog_text.rfind(L"\r");

        if (last_newline != std::wstring::npos)
        {
            drawnlog_text.erase(last_newline);
        }
        SetWindowText(drawn_log, drawnlog_text.c_str());
    }
	else
	{
		SetWindowText(drawn_log, drawnlog_text.c_str());
		drawn_log_linecount++;
		//MessageBox(NULL, StringCleaner(L"drawn_log_linecount: " + std::to_wstring(drawn_log_linecount)), L"Error", MB_OK | MB_ICONINFORMATION);
	}
}

void log_removelastentry() {
	size_t last_newline = drawnlog_text.rfind(L"\r");
	if (last_newline != std::wstring::npos)
	{
		drawnlog_text.erase(last_newline);
		SetWindowText(drawn_log, drawnlog_text.c_str());
	}
	drawn_log_linecount--;
}

int GetRandomInt(int max)
{
    while (max < 1) max++;
	if (max == 0) MessageBox(NULL, L"Error: max is 0", L"Error", MB_OK | MB_ICONERROR);
    std::random_device rd;
    std::mt19937 mt(rd());
	std::uniform_int_distribution<> dis(0, max - 1);
	return dis(mt);
}

int GetCivIndex(const std::wstring& civ_name)
{
    auto it = civ_name_to_index.find(civ_name);
    if (it != civ_name_to_index.end())
    {
        int index = it->second;
        return index;
    }
	return -1;
}

void ValidateRemainCount() { remaining = custom_max_civs - iterator; }

INT_PTR CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    HWND hist_combo_box;
    HWND hBanner = GetDlgItem(hDlg, IDC_CIV_ICON);
    std::wstring selected_civ = current_civ;

    switch (message) {
    case WM_INITDIALOG:
        if (selected_civ == L"Random") selected_civ = L"Armenians";
        UpdateCivAddedInfo(hDlg, GetCiv(selected_civ).dlc);
        SetDlgItemTextW(hDlg, IDC_HISTORY_EDIT, StringCleaner(FetchHistory(selected_civ)));

        hist_combo_box = GetDlgItem(hDlg, IDC_HISTORY_COMBO);
        for (int i = 0; i < MAX_CIVS; i++) SendMessage(hist_combo_box, CB_ADDSTRING, 0, (LPARAM)reinterpret_cast<LPARAM>(civ[i].name.c_str()));
        SendMessage(hBanner, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)GetCiv(selected_civ).icon);
        SetWindowPos(hBanner, NULL, 230, 8, 50, 50, SWP_NOZORDER);
        SendMessage(hist_combo_box, CB_SETCURSEL, GetCivIndex(selected_civ), 0);
        return (INT_PTR)TRUE;
    case WM_COMMAND:

        

        // fill text field with txt contents from /Histories/ of selected civ, eg. Armenians.txt
		if (LOWORD(wParam) == IDC_HISTORY_COMBO && HIWORD(wParam) == CBN_SELCHANGE)
        {
            SetWindowPos(hBanner, NULL, 230, 8, 50, 50, SWP_NOZORDER);
			hist_combo_box = GetDlgItem(hDlg, IDC_HISTORY_COMBO);
			int selected_index = SendMessage(hist_combo_box, CB_GETCURSEL, 0, 0);
			if (selected_index != CB_ERR)
            {
                
				wchar_t buffer[256];
				SendMessage(hist_combo_box, CB_GETLBTEXT, selected_index, reinterpret_cast<LPARAM>(buffer));
				selected_civ = buffer;
				UpdateCivAddedInfo(hDlg, GetCiv(selected_civ).dlc);
                ShowWindow(hBanner, SW_HIDE);
                SendMessage(hBanner, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)GetCiv(selected_civ).icon);
                
                SetWindowPos(hBanner, NULL, 230, 8, 50, 50, SWP_NOZORDER);
                ShowWindow(hBanner, SW_SHOW);
                // make sure it downsizes to fit 50x50
                
                
                SetDlgItemText(hDlg, IDC_HISTORY_EDIT, StringCleaner(FetchHistory(selected_civ)));
                
                /*SetWindowPos(hBanner, NULL, 225, 8, 50, 50, SWP_NOZORDER);
                SetWindowPos(hBanner, NULL, 230, 8, 50, 50, SWP_NOZORDER);*/
			}
            //SetWindowPos(hBanner, NULL, 225, 8, 50, 50, SWP_NOZORDER);

		}

        if (LOWORD(wParam) == IDC_HIS_SEARCH && HIWORD(wParam) == EN_CHANGE) {
            HWND hSearch = GetDlgItem(hDlg, IDC_HIS_SEARCH);
            HWND hCombo = GetDlgItem(hDlg, IDC_HISTORY_COMBO);
            wchar_t searchText[256];
            GetWindowText(hSearch, searchText, 256);

            // Search for civ name in combo box
            int count = (int)SendMessage(hCombo, CB_GETCOUNT, 0, 0);
            for (int i = 0; i < count; ++i) {
                wchar_t itemText[256];
                SendMessage(hCombo, CB_GETLBTEXT, i, (LPARAM)itemText);
                if (_wcsnicmp(itemText, searchText, wcslen(searchText)) == 0) {
                    SendMessage(hCombo, CB_SETCURSEL, i, 0);
                    // Optionally trigger selection change
                    SendMessage(hDlg, WM_COMMAND, MAKELONG(IDC_HISTORY_COMBO, CBN_SELCHANGE), (LPARAM)hCombo);
                    break;
                }
            }
        }


        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void OpenHistory(HWND hParent) { DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_HISTORY_DIALOG), hParent, DialogProc); }

std::wstring NormaliseLineEndings(const std::wstring& input) {
    std::wstring output;
    output.reserve(input.size());
    for (size_t i = 0; i < input.size(); ++i) {
        if (input[i] == L'\n') {
            if (i == 0 || input[i - 1] != L'\r')
                output += L"\r\n";
            else
                output += L"\n";
        }
        else {
            output += input[i];
        }
    }
    return output;
}

std::wstring FetchHistory(const std::wstring& civ_name)
{
    // get filename from civ name
	std::wstring filename = HIS_FILE_PATH + civ_name + L".txt";
    std::ifstream file(filename, std::ios::binary);
    if (!file) return L"History not found.";

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string utf8str = buffer.str();

    // Remove BOM if present
    if (utf8str.size() >= 3 &&
        (unsigned char)utf8str[0] == 0xEF &&
        (unsigned char)utf8str[1] == 0xBB &&
        (unsigned char)utf8str[2] == 0xBF) {
        utf8str = utf8str.substr(3);
    }

    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
    return conv.from_bytes(utf8str);
}

void UpdateTooltips(bool drawn)
{
    if (drawn)
    {
        UpdateTooltipText(button_techtree, hwndTooltip[TOOLTIP_TECHTREE], StringCleaner(L"Opens the tech tree for the " + current_civ + L"\nHotkey: T"));
        UpdateTooltipText(button_history, hwndTooltip[TOOLTIP_HISTORY], StringCleaner(L"Opens the game's history info for the " + current_civ + L"\nHotkey: H"));
    }
    else
    {
        UpdateTooltipText(button_techtree, hwndTooltip[TOOLTIP_TECHTREE], StringCleaner(L"Opens the tech tree\nHotkey: T"));
        UpdateTooltipText(button_history, hwndTooltip[TOOLTIP_HISTORY], StringCleaner(L"Opens the game's history info\nHotkey: H"));
    }
}

void UpdateCivAddedInfo(HWND hDlg, dlc dlc)
{
    switch (dlc)
    {
    case aok:
        SetDlgItemText(hDlg, IDC_HIS_INFO, L"1999-09-27 (Age of Kings)");
        break;
    case aoc:
        SetDlgItemText(hDlg, IDC_HIS_INFO, L"2000-08-25 (The Conquerors)");
        break;
    case forgotten:
        SetDlgItemText(hDlg, IDC_HIS_INFO, L"2013-11-07 (The Forgotten)");
        break;
    case africans:
        SetDlgItemText(hDlg, IDC_HIS_INFO, L"2015-11-05 (The African Kingdoms)");
        break;
    case rajas:
        SetDlgItemText(hDlg, IDC_HIS_INFO, L"2016-12-19 (Rise of the Rajas)");
        break;
    case khans:
        SetDlgItemText(hDlg, IDC_HIS_INFO, L"2019-11-14 (The Last Khans)");
        break;
    case west:
        SetDlgItemText(hDlg, IDC_HIS_INFO, L"2021-01-26 (Lords of the West)");
        break;
    case dukes:
        SetDlgItemText(hDlg, IDC_HIS_INFO, L"2021-08-10 (Dawn of the Dukes)");
        break;
    case india:
        SetDlgItemText(hDlg, IDC_HIS_INFO, L"2022-04-28 (Dynasties of India)");
        break;
    case rome:
        SetDlgItemText(hDlg, IDC_HIS_INFO, L"2023-05-16 (Return of Rome)");
        break;
    case royals:
        SetDlgItemText(hDlg, IDC_HIS_INFO, L"2023-10-31 (The Mountain Royals)");
        break;
    case kingdoms:
        SetDlgItemText(hDlg, IDC_HIS_INFO, L"2025-05-06 (The Three Kingdoms)");
        break;
    }
}