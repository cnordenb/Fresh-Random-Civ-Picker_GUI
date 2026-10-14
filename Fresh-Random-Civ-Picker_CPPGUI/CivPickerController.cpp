#include "CivPickerController.h"

#include <shellapi.h>
#include <shlwapi.h>
#include <fstream>
#include <iomanip>
#include <sstream>

#include "Assertions.h"

#include "Resource.h"

#pragma comment(lib, "shlwapi.lib")

namespace
{
    struct Hotkey { UINT modifier; UINT key; int id; };

    const Hotkey HOTKEYS[HOTKEY_AMOUNT] = { { 0, VK_TAB, HOTKEY_ID_TAB },
                                            { 0, VK_SPACE, HOTKEY_ID_SPACE },
                                            { 0, VK_RETURN, HOTKEY_ID_RETURN },
                                            { 0, VK_ESCAPE, HOTKEY_ID_ESC },
                                            { 0, 0x5A, HOTKEY_ID_Z },
                                            { 0, 0x58, HOTKEY_ID_X },
                                            { 0, 0x43, HOTKEY_ID_C },
                                            { 0, 0x56, HOTKEY_ID_V },
                                            { 0, 0x54, HOTKEY_ID_T },
                                            { 0, 0x31, HOTKEY_ID_1 },
                                            { 0, 0x32, HOTKEY_ID_2 },
                                            { 0, 0x33, HOTKEY_ID_3 },
                                            { 0, 0x51, HOTKEY_ID_Q },
                                            { 0, 0x57, HOTKEY_ID_W },
                                            { 0, 0x45, HOTKEY_ID_E },
                                            { 0, 0x41, HOTKEY_ID_A },
                                            { 0, 0x53, HOTKEY_ID_S },
                                            { 0, 0x44, HOTKEY_ID_D },
                                            { 0, 0x46, HOTKEY_ID_F },
                                            { 0, 0x47, HOTKEY_ID_G },
                                            { 0, 0x48, HOTKEY_ID_H },
                                            { 0, 0x52, HOTKEY_ID_R },
                                            { 0, 0x54, HOTKEY_ID_T },
                                            { 0, VK_F1, HOTKEY_ID_F1 },
                                            { 0, 0x42, HOTKEY_ID_B },
                                            { 0, VK_F2, HOTKEY_ID_F2 },
                                            { 0, VK_F3, HOTKEY_ID_F3 },
                                            { 0, VK_F4, HOTKEY_ID_F4 },
                                            { 0, VK_F5, HOTKEY_ID_F5 },
                                            { 0, VK_F6, HOTKEY_ID_F5 },
                                            { 0, VK_F9, HOTKEY_ID_F9 },
                                            { MOD_CONTROL, 0x53, HOTKEY_ID_CTRLS },
                                            { MOD_CONTROL, 0x52, HOTKEY_ID_CTRLR },
                                            { MOD_CONTROL, 0x46, HOTKEY_ID_CTRLF },
                                            { MOD_CONTROL, 0x5A, HOTKEY_ID_CTRLZ },
                                            { MOD_CONTROL, 0x58, HOTKEY_ID_CTRLX },
                                            { MOD_CONTROL, 0x54, HOTKEY_ID_CTRLT },
                                            { 0, 0x4a, HOTKEY_ID_J },
                                            { 0, 0x4b, HOTKEY_ID_K },
                                            { 0, 0x4c, HOTKEY_ID_L },
                                            { 0, 0x50, HOTKEY_ID_P },
                                            { 0, 0x55, HOTKEY_ID_U } };

    // data.json's first edition is the default one.
    constexpr int DEFAULT_EDITION = 0;

    // Cap on user-triggered draws. Each draw starts its jingle as a new asynchronous sound, and Windows
    // frees the interrupted ones only gradually: holding Space at keyboard-repeat speed piled up about
    // 1 MB per draw. At 10 draws per second the memory use was measured to stay flat.
    const ULONGLONG MAX_DRAWS_PER_SECOND = 10;
    const ULONGLONG MIN_DRAW_INTERVAL_MS = 1000 / MAX_DRAWS_PER_SECOND;

    // On the Civ Pool tab these hotkeys change the pool, so auto-reset resets before them.
    const WPARAM POOL_CHANGING_HOTKEYS[] =
    {
        HOTKEY_ID_SPACE, HOTKEY_ID_RETURN, HOTKEY_ID_Q, HOTKEY_ID_W, HOTKEY_ID_E, HOTKEY_ID_A, HOTKEY_ID_S,
        HOTKEY_ID_D, HOTKEY_ID_F, HOTKEY_ID_G, HOTKEY_ID_H, HOTKEY_ID_J, HOTKEY_ID_K, HOTKEY_ID_L
    };

    struct WebLink { int command_id; const wchar_t* url; };
    const WebLink WEB_LINKS[] =
    {
        { IDM_AOE2INSIGHTS, L"https://www.aoe2insights.com/lobbies/" },
        { IDM_SPECTATORDASHBOARD, L"https://aoe2recs.com/" },
        { IDM_GITHUB, L"https://github.com/cnordenb/Fresh-Random-Civ-Picker_CPPGUI" },
        { IDM_WEBVERSION, L"https://frcp.net/" },
        { IDM_DISCORD, L"https://discord.gg/rcFqrBdVUN" },
        { IDM_DONATE, L"https://ko-fi.com/hjoerleif" },
        { IDM_UPDATE, L"https://github.com/cnordenb/Fresh-Random-Civ-Picker_GUI/releases" },
        { IDM_TECHTREE, L"https://aoe2techtree.net/" },
        { IDM_STATS, L"https://aoestats.io/" },
        { IDM_USAGERULES, L"https://www.xbox.com/en-US/developers/rules" }
    };

    std::wstring GetExeFolder()
    {
        wchar_t exe_folder[MAX_PATH];
        GetModuleFileName(NULL, exe_folder, MAX_PATH);
        wchar_t* last_slash = wcsrchr(exe_folder, L'\\');
        if (last_slash != NULL) *last_slash = L'\0';
        return exe_folder;
    }

    void OpenUrl(const wchar_t* url) { ShellExecute(NULL, L"open", url, NULL, NULL, SW_SHOWNORMAL); }

    // Reads the editions, DLCs and civs from data.json in the exe's folder (the build copies it there). If the
    // program can't start with it, says why and returns false.
    bool LoadCatalog(CivCatalog& catalog)
    {
        const std::wstring path = GetExeFolder() + L"\\data.json";
        std::ifstream file(path, std::ios::binary);
        std::stringstream contents;
        if (file) contents << file.rdbuf();
        if (file && catalog.Load(contents.str())) return true;
        const std::wstring problem = file ? catalog.Error() : L"data.json could not be read from " + path;
        (void)MessageBoxW(NULL, (problem + L"\n\nThe program needs data.json to start.").c_str(),
                          L"Fresh Random Civ Picker", MB_OK | MB_ICONERROR);
        return false;
    }
}

CivPickerController::CivPickerController(CivPickerModel& picker_model) :
    model(picker_model),
    view(picker_model, *this)
{
    model.SetObserver(view);
}

// ---------------------------------------------------------------------------------------------------
// Startup, message loop and shutdown

int CivPickerController::Run(HINSTANCE app_instance, int show_command)
{
    view.SetInstance(app_instance);
    GenerateFilePaths();
    view.LoadSounds();
    LoadSettings();
    if (!view.StartGraphics() || !view.CreateMainWindow(show_command))
    {
        view.StopGraphics();
        return FALSE;
    }

    const int exit_code = RunMessageLoop(app_instance);

    view.StopGraphics();
    if (model.GetSettings().persistent_logging) model.SaveAutomaticLog();
    SaveSettings();
    view.UnloadSounds();
    return exit_code;
}

// The Win32 message loop is the program's event loop: it runs until WM_QUIT (CLAUDE2.md I3).
int CivPickerController::RunMessageLoop(HINSTANCE app_instance)
{
    HACCEL accelerators = LoadAccelerators(app_instance, MAKEINTRESOURCE(IDC_WINDOWSPROJECT1));
    MSG message;
    while (GetMessage(&message, nullptr, 0, 0))
    {
        if (view.PreTranslateMessage(message)) continue;
        if (TranslateAccelerator(message.hwnd, accelerators, &message)) continue;
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
    return static_cast<int>(message.wParam);
}

void CivPickerController::OnMainWindowCreated(HWND window)
{
    main_window = window;
    model.SetStartingUp(true);
    model.SetEdition(DEFAULT_EDITION);
    if (model.GetSettings().persistent_logging)
    {
        LoadLog(SaveType::automatic);
        view.ValidateAllDlcToggles();
    }
    ShowTab(CivPickerView::DRAW_TAB);
    EnableHotkeys();
    RunStartupDraw();
    model.SetStartingUp(false);
}

void CivPickerController::RunStartupDraw()
{
    const Settings& settings = model.GetSettings();
    if (settings.draw_on_startup)
    {
        model.DrawCiv(false, L"");
        if (settings.jingles_enabled) view.PlayJingle(model.GetCurrentCiv());
    }
    else if (!settings.persistent_logging) model.ResetProgram(false);
}

void CivPickerController::Quit() { PostQuitMessage(0); }

void CivPickerController::OnFatalError() { PostQuitMessage(-1); }

void CivPickerController::GenerateFilePaths()
{
    const std::wstring exe_folder = GetExeFolder();
    settings_file_path = exe_folder + L"\\settings.ini";
    log_file_path = exe_folder + L"\\log.txt";
    model.SetFilePaths(log_file_path, exe_folder + L"\\Histories\\");
    // The images sit next to the exe (the build copies them to the output folder), so they are found
    // whatever working directory the program was started in.
    view.SetImageFolder(exe_folder + L"\\images\\");
    // The jingles are files there too, named after the civs in data.json.
    view.SetJingleFolder(exe_folder + L"\\sounds\\civ_jingles\\");
}

void CivPickerController::LoadSettings()
{
    Settings& settings = model.EditSettings();
    const wchar_t* ini_file = settings_file_path.c_str();
    settings.persistent_logging = GetPrivateProfileInt(L"Settings", L"PersistentLogging", 1, ini_file) != 0;
    settings.draw_on_startup = GetPrivateProfileInt(L"Settings", L"DrawOnStartup", 1, ini_file) != 0;
    settings.ui_sounds_enabled = GetPrivateProfileInt(L"Settings", L"UISoundsEnabled", 1, ini_file) != 0;
    settings.civ_labels_enabled = GetPrivateProfileInt(L"Settings", L"LabelsEnabled", 1, ini_file) != 0;
    settings.iteration_label_enabled = GetPrivateProfileInt(L"Settings", L"IterationLabelEnabled", 1, ini_file) != 0;
    settings.icons_enabled = GetPrivateProfileInt(L"Settings", L"IconsEnabled", 1, ini_file) != 0;
    settings.jingles_enabled = GetPrivateProfileInt(L"Settings", L"JinglesEnabled", 1, ini_file) != 0;
    settings.tooltips_enabled = GetPrivateProfileInt(L"Settings", L"TooltipsEnabled", 1, ini_file) != 0;
    settings.remainlog_enabled = GetPrivateProfileInt(L"Settings", L"RemainLogEnabled", 1, ini_file) != 0;
    settings.autoreset_enabled = GetPrivateProfileInt(L"Settings", L"AutoResetEnabled", 0, ini_file) != 0;
    settings.autotoggle_enabled = GetPrivateProfileInt(L"Settings", L"AutoToggleEnabled", 0, ini_file) != 0;
    settings.legacy_jingle_enabled = GetPrivateProfileInt(L"Settings", L"LegacyJingleEnabled", 0, ini_file) != 0;
    settings.contfresh_enabled = GetPrivateProfileInt(L"Settings", L"ContFreshEnabled", 0, ini_file) != 0;
    settings.contfresh_strength =
        static_cast<uint8_t>(GetPrivateProfileInt(L"Settings", L"ContFreshStrengthValue", 0, ini_file));
}

void CivPickerController::SaveSettings()
{
    const Settings& settings = model.GetSettings();
    WriteSetting(L"PersistentLogging", settings.persistent_logging);
    WriteSetting(L"DrawOnStartup", settings.draw_on_startup);
    WriteSetting(L"UISoundsEnabled", settings.ui_sounds_enabled);
    WriteSetting(L"LabelsEnabled", settings.civ_labels_enabled);
    WriteSetting(L"IterationLabelEnabled", settings.iteration_label_enabled);
    WriteSetting(L"IconsEnabled", settings.icons_enabled);
    WriteSetting(L"JinglesEnabled", settings.jingles_enabled);
    WriteSetting(L"TooltipsEnabled", settings.tooltips_enabled);
    WriteSetting(L"RemainLogEnabled", settings.remainlog_enabled);
    WriteSetting(L"AutoResetEnabled", settings.autoreset_enabled);
    WriteSetting(L"AutoToggleEnabled", settings.autotoggle_enabled);
    WriteSetting(L"LegacyJingleEnabled", settings.legacy_jingle_enabled);
    WriteSetting(L"ContFreshEnabled", settings.contfresh_enabled);
    WritePrivateProfileString(L"Settings", L"ContFreshStrengthValue",
                              std::to_wstring(settings.contfresh_strength).c_str(), settings_file_path.c_str());
}

void CivPickerController::WriteSetting(const wchar_t* key, bool enabled) const
{
    WritePrivateProfileString(L"Settings", key, enabled ? L"1" : L"0", settings_file_path.c_str());
}

std::wstring CivPickerController::SavesFolder() const
{
    wchar_t saves_folder[MAX_PATH];
    wcscpy_s(saves_folder, GetExeFolder().c_str());
    PathAppend(saves_folder, L"saves");
    return saves_folder;
}

std::wstring CivPickerController::QuicksavePath() const
{
    wchar_t quicksave_path[MAX_PATH];
    wcscpy_s(quicksave_path, SavesFolder().c_str());
    PathAppend(quicksave_path, L"quicksave.txt");
    return quicksave_path;
}

bool CivPickerController::FileExists(const std::wstring& path) const
{
    const DWORD attributes = GetFileAttributes(path.c_str());
    return attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY);
}

// ---------------------------------------------------------------------------------------------------
// Hotkeys

void CivPickerController::EnableHotkeys()
{
    for (const Hotkey& hotkey : HOTKEYS) RegisterHotKey(main_window, hotkey.id, hotkey.modifier, hotkey.key);
}

void CivPickerController::DisableHotkeys()
{
    for (int hotkey_id = 1; hotkey_id < HOTKEY_AMOUNT; hotkey_id++) UnregisterHotKey(main_window, hotkey_id);
}

void CivPickerController::OnActivate(WPARAM activation)
{
    if (activation == WA_ACTIVE || activation == WA_CLICKACTIVE) EnableHotkeys();
    else if (activation == WA_INACTIVE) DisableHotkeys();
}

void CivPickerController::OnHotkey(WPARAM hotkey_id)
{
    hotkey_pressed = true;
    if (GetForegroundWindow() != main_window)
    {
        DisableHotkeys();
        return;
    }
    EnableHotkeys();

    HandleGlobalHotkey(hotkey_id);
    if (view.GetCurrentTab() == CivPickerView::DRAW_TAB) HandleDrawTabHotkey(hotkey_id);
    if (view.GetCurrentTab() != CivPickerView::CIV_POOL_TAB) HandleDrawingHotkey(hotkey_id);
    else HandleCivPoolHotkey(hotkey_id);
    if (view.GetCurrentTab() == CivPickerView::LOG_TAB) HandleLogTabHotkey(hotkey_id);
    HandleTabHotkey(hotkey_id);
    HandleDisplayToggleHotkey(hotkey_id);
}

void CivPickerController::HandleGlobalHotkey(WPARAM hotkey_id)
{
    switch (hotkey_id)
    {
        case HOTKEY_ID_ESC: Quit(); break;
        case HOTKEY_ID_F1: OpenOptions(main_window); break;
        case HOTKEY_ID_F2: OpenHotkeys(main_window); break;
        case HOTKEY_ID_F3: OpenAbout(main_window); break;
        case HOTKEY_ID_F4: OpenTechTree(); break;
        case HOTKEY_ID_F5: SaveLog(SaveType::quick); break;
        case HOTKEY_ID_F9: LoadLog(SaveType::quick); break;
        case HOTKEY_ID_CTRLS: SaveLog(SaveType::manual); break;
        case HOTKEY_ID_CTRLR: LoadLog(SaveType::manual); break;
        case HOTKEY_ID_CTRLF: JoinLobby(); break;
        case HOTKEY_ID_CTRLZ: Undo(); break;
        case HOTKEY_ID_CTRLX: model.RedrawCiv(); break;
        case HOTKEY_ID_CTRLT: OpenSurvapp(); break;
        case HOTKEY_ID_P: CopyRoundLink(); break;
        case HOTKEY_ID_U: OpenShareLink(); break;
    }
}

void CivPickerController::HandleDrawTabHotkey(WPARAM hotkey_id)
{
    switch (hotkey_id)
    {
        case HOTKEY_ID_R: OpenSurvapp(); break;
        case HOTKEY_ID_Q: OpenOptions(main_window); break;
        case HOTKEY_ID_T: OpenTechTree(); break;
        case HOTKEY_ID_H: OpenHistory(main_window); break;
    }
}

void CivPickerController::HandleDrawingHotkey(WPARAM hotkey_id)
{
    if (hotkey_id == HOTKEY_ID_SPACE) DrawCivWithinRateCap();
    if (hotkey_id == HOTKEY_ID_RETURN) model.ResetProgram(false);
}

void CivPickerController::DrawCivWithinRateCap()
{
    const ULONGLONG now = GetTickCount64();
    if (now - last_draw_tick < MIN_DRAW_INTERVAL_MS) return;
    last_draw_tick = now;
    model.DrawCiv(false, L"");
}

void CivPickerController::HandleCivPoolHotkey(WPARAM hotkey_id)
{
    bool changes_pool = false;
    for (WPARAM pool_hotkey : POOL_CHANGING_HOTKEYS) if (pool_hotkey == hotkey_id) changes_pool = true;
    if (changes_pool && model.GetSettings().autoreset_enabled) model.ResetProgram(true);

    switch (hotkey_id)
    {
        case HOTKEY_ID_SPACE: EnableAll(true); break;
        case HOTKEY_ID_RETURN: DisableAll(true); break;
        case HOTKEY_ID_R: ToggleAutoToggle(); break;
        case HOTKEY_ID_T: ToggleAutoReset(); break;
    }
    HandleEditionHotkey(hotkey_id);
    HandleDlcHotkey(hotkey_id);
}

// Q, W and E pick the editions in data.json's order.
void CivPickerController::HandleEditionHotkey(WPARAM hotkey_id)
{
    const int edition_count = model.Catalog().EditionCount();
    for (int edition = 0; edition < edition_count && edition < MAX_EDITIONS; edition++)
    {
        if (hotkey_id == static_cast<WPARAM>(EDITION_HOTKEY_IDS[edition])) model.SetEdition(edition);
    }
}

// A to L toggle the DLCs listed in the current edition's dlcToggles, top to bottom.
void CivPickerController::HandleDlcHotkey(WPARAM hotkey_id)
{
    const EditionInfo& edition = model.Catalog().EditionAt(model.GetEdition());
    for (int slot = 0; slot < edition.dlc_toggle_count && slot < MAX_DLC_TOGGLES; slot++)
    {
        if (hotkey_id == static_cast<WPARAM>(DLC_SLOT_HOTKEY_IDS[slot])) ToggleDlc(edition.dlc_toggles[slot]);
    }
}

void CivPickerController::HandleLogTabHotkey(WPARAM hotkey_id)
{
    if (hotkey_id == HOTKEY_ID_Q) ClearDrawnLog();
    if (hotkey_id == HOTKEY_ID_W) ToggleRemainLog();
}

void CivPickerController::HandleTabHotkey(WPARAM hotkey_id)
{
    if (hotkey_id == HOTKEY_ID_TAB)
    {
        const int selected_tab = view.GetSelectedTab();
        SelectTab(selected_tab < CivPickerView::CIV_POOL_TAB ? selected_tab + 1 : CivPickerView::DRAW_TAB);
    }
    if (hotkey_id == HOTKEY_ID_1) SelectTab(CivPickerView::DRAW_TAB);
    if (hotkey_id == HOTKEY_ID_2) SelectTab(CivPickerView::LOG_TAB);
    if (hotkey_id == HOTKEY_ID_3) SelectTab(CivPickerView::CIV_POOL_TAB);
}

void CivPickerController::HandleDisplayToggleHotkey(WPARAM hotkey_id)
{
    Settings& settings = model.EditSettings();
    if (hotkey_id == HOTKEY_ID_Z)
    {
        view.PlayAudio(SoundType::button);
        settings.civ_labels_enabled = !settings.civ_labels_enabled;
        view.RefreshCivLabel();
    }
    if (hotkey_id == HOTKEY_ID_X)
    {
        view.PlayAudio(SoundType::button);
        settings.icons_enabled = !settings.icons_enabled;
        view.RefreshCivIcon();
    }
    if (hotkey_id == HOTKEY_ID_V)
    {
        settings.jingles_enabled = !settings.jingles_enabled;
        if (settings.jingles_enabled) view.PlayJingle(model.GetCurrentCiv());
        else view.StopSound();
    }
    if (hotkey_id == HOTKEY_ID_B)
    {
        if (!settings.ui_sounds_enabled) view.PlayAudio(SoundType::button);
        settings.ui_sounds_enabled = !settings.ui_sounds_enabled;
    }
    if (hotkey_id == HOTKEY_ID_C)
    {
        view.PlayAudio(SoundType::button);
        settings.iteration_label_enabled = !settings.iteration_label_enabled;
        view.RefreshCornerLabel();
    }
}

// ---------------------------------------------------------------------------------------------------
// Menu, button and checkbox commands

bool CivPickerController::OnCommand(int command_id)
{
    hotkey_pressed = false;
    HandleCivCheckbox(command_id);

    if (model.GetSettings().autoreset_enabled && IsPoolControl(command_id)) model.ResetProgram(true);

    HandleEditionRadio(command_id);
    HandleDlcCheckbox(command_id);
    if (OpenWebPage(command_id) || OpenEditionLink(command_id)) return true;
    if (HandleMenuCommand(command_id)) return true;
    return HandleButtonCommand(command_id);
}

// The controls whose clicks change the pool, so auto-reset resets before them: the civ and DLC toggles, the edition
// rows, and Enable All and Disable All.
bool CivPickerController::IsPoolControl(int command_id) const
{
    const CivCatalog& catalog = model.Catalog();
    const int civ = command_id - CivPickerView::CIV_CHECKBOX_BASE_ID;
    const int dlc = command_id - CivPickerView::DLC_TOGGLE_BASE_ID;
    const int edition = command_id - CivPickerView::EDITION_ROW_BASE_ID;
    return (civ >= 0 && civ < catalog.CivCount()) || (dlc >= 0 && dlc < catalog.DlcCount()) ||
           (edition >= 0 && edition < catalog.EditionCount()) || command_id == IDC_BUTTON_ENABLEALL ||
           command_id == IDC_BUTTON_DISABLEALL;
}

void CivPickerController::HandleCivCheckbox(int command_id)
{
    const int civ_index = command_id - CivPickerView::CIV_CHECKBOX_BASE_ID;
    if (civ_index < 0 || civ_index >= model.Catalog().CivCount()) return;

    view.PlayAudio(SoundType::button);
    const Civ& clicked_civ = model.GetCiv(civ_index);
    if (view.IsCivChecked(civ_index)) model.AddCiv(clicked_civ.name);
    else model.RemoveCiv(clicked_civ.name);
    view.ValidateDlcToggle(clicked_civ.dlc);
}

void CivPickerController::HandleEditionRadio(int command_id)
{
    const int edition = command_id - CivPickerView::EDITION_ROW_BASE_ID;
    if (edition >= 0 && edition < model.Catalog().EditionCount()) model.SetEdition(edition);
}

void CivPickerController::HandleDlcCheckbox(int command_id)
{
    const int dlc = command_id - CivPickerView::DLC_TOGGLE_BASE_ID;
    if (dlc >= 0 && dlc < model.Catalog().DlcCount()) ToggleDlc(dlc);
}

bool CivPickerController::OpenWebPage(int command_id)
{
    for (const WebLink& link : WEB_LINKS)
    {
        if (command_id != link.command_id) continue;
        view.PlayAudio(SoundType::button);
        ShellExecute(0, 0, link.url, 0, 0, SW_SHOW);
        return true;
    }
    return false;
}

// The Game menu's links to the editions' pages (data.json's links).
bool CivPickerController::OpenEditionLink(int command_id)
{
    const int link = command_id - CivPickerView::EDITION_LINK_BASE_ID;
    if (link < 0 || link >= model.Catalog().LinkCount()) return false;
    view.PlayAudio(SoundType::button);
    OpenUrl(model.Catalog().LinkAt(link).url.c_str());
    return true;
}

bool CivPickerController::HandleMenuCommand(int command_id)
{
    switch (command_id)
    {
        case IDM_ABOUT: OpenAbout(main_window); return true;
        case IDM_EXIT: Quit(); return true;
        case IDM_OPTIONS: OpenOptions(main_window); return true;
        case IDM_SAVELOG: SaveLog(SaveType::manual); return true;
        case IDM_LOADLOG: LoadLog(SaveType::manual); return true;
        case IDM_QUICKSAVE: SaveLog(SaveType::quick); return true;
        case IDM_QUICKLOAD: LoadLog(SaveType::quick); return true;
        case IDM_JOINLOBBY: JoinLobby(); return true;
        case IDM_UNDRAW: model.UndrawCiv(); return true;
        case IDM_REDRAW: model.RedrawCiv(); return true;
        case IDM_HOTKEYS: OpenHotkeys(main_window); return true;
        case IDM_SURVAPP: OpenSurvapp(); return true;
        case IDM_COPYLINK: CopyRoundLink(); return true;
        case IDM_SHARE: OpenShare(); return true;
        case IDM_OPENLINK: OpenShareLink(); return true;
    }
    return false;
}

bool CivPickerController::HandleButtonCommand(int command_id)
{
    switch (command_id)
    {
        case IDC_BUTTON_HISTORY: OpenHistory(main_window); return true;
        case IDC_BUTTON_DRAW: DrawCivWithinRateCap(); return true;
        case IDC_BUTTON_RESET: model.ResetProgram(false); return true;
        case IDC_BUTTON_ENABLEALL: EnableAll(true); return true;
        case IDC_BUTTON_DISABLEALL: DisableAll(true); return true;
        case IDC_BUTTON_CLEARLOG: ClearDrawnLog(); return true;
        case IDC_BUTTON_OPTIONS:
            view.DeactivateTooltips();
            OpenOptions(main_window);
            return true;
        case IDC_BUTTON_SURVAPP: OpenSurvapp(); return true;
        case IDC_BUTTON_TECHTREE: OpenTechTree(); return true;
        case IDC_CHECKBOX_REMAINLOG: ToggleRemainLog(); return true;
        case IDC_CHECKBOX_AUTORESET: ToggleAutoReset(); return true;
        case IDC_CHECKBOX_AUTOTOGGLE: ToggleAutoToggle(); return true;
        case IDC_ICON_CIV: view.PlayJingle(model.GetCurrentCiv()); return true;
        case IDC_ICON_EDITION: OpenEditionPage(); return true;
        case IDC_BUTTON_SHARE: CopyRoundLink(); return true;
        case IDC_BUTTON_POOLSHARE: CopyPoolLink(); return true;
    }
    return false;
}

// The edition's store page, or the note data.json has for an edition without one.
void CivPickerController::OpenEditionPage()
{
    const EditionInfo& edition = model.Catalog().EditionAt(model.GetEdition());
    if (!edition.url.empty()) OpenUrl(edition.url.c_str());
    else view.ShowInformation(edition.store_note.c_str(), edition.short_name.c_str());
}

void CivPickerController::OnTabSelected(int tab_index) { ShowTab(tab_index); }

// Showing the Civ Pool tab drops any civ that the current edition does not contain.
void CivPickerController::ShowTab(int tab_index)
{
    if (tab_index == CivPickerView::CIV_POOL_TAB) model.EnforceEditionPool();
    view.ShowTabComponents(tab_index);
}

void CivPickerController::SelectTab(int tab_index)
{
    view.SelectTab(tab_index);
    ShowTab(tab_index);
}

// ---------------------------------------------------------------------------------------------------
// Options dialog

bool CivPickerController::OnOptionsCommand(HWND dialog, const CivPickerView::OptionsEvent& options_event)
{
    PlayComboFeedback(options_event.notification);
    ApplyOptions(dialog, options_event);
    view.RefreshCivIcon();
    if (options_event.notification == BN_CLICKED) HandleOptionsClick(dialog, options_event.command_id);
    view.RefreshCivLabel();
    view.RefreshCornerLabel();

    if (options_event.command_id != IDCANCEL && options_event.command_id != IDOK) return false;
    view.PlayAudio(SoundType::button);
    return true;
}

// Combo box feedback plays before the new jingle type is applied below.
void CivPickerController::PlayComboFeedback(int notification)
{
    if (notification == CBN_DROPDOWN) view.PlayAudio(SoundType::hover);
    if (notification != CBN_SELCHANGE) return;
    if (!model.GetCiv(model.GetCurrentCiv()).legacy) view.PlayAudio(SoundType::view);
    else view.PlayJingle(model.GetCurrentCiv());
}

void CivPickerController::ApplyOptions(HWND dialog, const CivPickerView::OptionsEvent& options_event)
{
    Settings& settings = model.EditSettings();
    settings.ui_sounds_enabled = options_event.ui_sounds;
    settings.civ_labels_enabled = options_event.civ_labels;
    settings.iteration_label_enabled = options_event.iteration_label;
    settings.icons_enabled = options_event.icons;
    settings.jingles_enabled = options_event.jingles;
    view.ShowJingleTypeOption(dialog, settings.jingles_enabled);
    settings.tooltips_enabled = options_event.tooltips;
    settings.draw_on_startup = options_event.draw_on_startup;
    settings.contfresh_enabled = options_event.contfresh;
    view.ShowContfreshOptions(dialog, settings.contfresh_enabled);
    settings.legacy_jingle_enabled = options_event.legacy_jingle;
}

void CivPickerController::HandleOptionsClick(HWND dialog, int command_id)
{
    if (command_id == IDC_CHECKBOX_JINGLES)
    {
        view.StopSound();
        view.PlayJingle(model.GetCurrentCiv());
    }
    switch (command_id)
    {
        case IDC_BUTTON_HOTKEYS:
            OpenHotkeys(dialog);
            break;
        case IDC_CHECKBOX_LABELS:
        case IDC_CHECKBOX_CORNERLABEL:
        case IDC_CHECKBOX_ICONS:
        case IDC_CHECKBOX_SOUNDS:
        case IDC_CHECKBOX_TOOLTIPS:
        case IDC_CHECKBOX_STARTDRAW:
        case IDC_CHECKBOX_CONTFRESH:
            view.PlayAudio(SoundType::button);
            break;
        case IDC_RADIO_LOGGING:
            view.PlayAudio(SoundType::view);
            model.EditSettings().persistent_logging = true;
            break;
        case IDC_RADIO_STARTRESET:
            view.PlayAudio(SoundType::view);
            model.EditSettings().persistent_logging = false;
            break;
    }
}

void CivPickerController::OnContfreshStrengthChanged(int strength)
{
    model.EditSettings().contfresh_strength = static_cast<uint8_t>(strength);
}

// ---------------------------------------------------------------------------------------------------
// Pool toggles, undo and the drawn log

void CivPickerController::ToggleDlc(int dlc)
{
    if (!c_assert(dlc >= 0 && dlc < model.Catalog().DlcCount())) return;
    if (!model.IsStartingUp()) view.PlayAudio(SoundType::button);
    bool checked = view.IsDlcChecked(dlc);
    if (hotkey_pressed)
    {
        checked = !checked;
        view.SetDlcChecked(dlc, checked);
    }
    model.SetDlcEnabled(dlc, checked);
}

void CivPickerController::ToggleAutoToggle()
{
    if (!model.IsStartingUp()) view.PlayAudio(SoundType::button);
    if (hotkey_pressed) view.SetAutoToggleChecked(!view.IsAutoToggleChecked());
    model.SetAutoToggle(view.IsAutoToggleChecked());
}

void CivPickerController::ToggleAutoReset()
{
    if (!model.IsStartingUp()) view.PlayAudio(SoundType::button);
    if (hotkey_pressed) view.SetAutoResetChecked(!view.IsAutoResetChecked());
    model.EditSettings().autoreset_enabled = view.IsAutoResetChecked();
}

void CivPickerController::ToggleRemainLog()
{
    view.PlayAudio(SoundType::button);
    Settings& settings = model.EditSettings();
    settings.remainlog_enabled = !settings.remainlog_enabled;
    view.ShowRemainingLog(settings.remainlog_enabled);
    if (hotkey_pressed) view.ToggleRemainLogCheckbox();
}

void CivPickerController::EnableAll(bool sound_acceptable)
{
    if (sound_acceptable) view.PlayAudio(SoundType::button);
    model.EnableAll();
}

void CivPickerController::DisableAll(bool sound_acceptable)
{
    if (sound_acceptable) view.PlayAudio(SoundType::button);
    model.DisableAll();
}

void CivPickerController::ClearDrawnLog()
{
    view.PlayAudio(SoundType::button);
    view.ClearDrawnLog();
}

// Undo means undrawing the last civ, or restoring the log saved by the last reset.
void CivPickerController::Undo()
{
    const bool cache_restorable = model.GetSettings().persistent_logging && model.IsCacheAvailable();
    switch (model.GetLastAction())
    {
        case LastAction::draw:
            if (model.GetIterator() > 0) model.UndrawCiv();
            else if (cache_restorable) LoadCachedLog();
            return;
        case LastAction::reset:
            if (cache_restorable) LoadCachedLog();
            return;
        case LastAction::pool_change:
            return;
    }
}

void CivPickerController::LoadCachedLog()
{
    model.SetCacheLoading(true);
    LoadLog(SaveType::automatic);
    model.SetCacheAvailable(false);
    model.SetCacheLoading(false);
}

// ---------------------------------------------------------------------------------------------------
// Saving and loading the log

void CivPickerController::SaveLog(SaveType type)
{
    model.DiscardRedoHistory();
    std::wstring save_file_path = log_file_path;
    if (type == SaveType::quick) save_file_path = QuicksavePath();
    else if (type == SaveType::manual)
    {
        const CivPickerView::FileChoice choice = ChooseManualSaveFile();
        if (!choice.chosen) return;
        save_file_path = choice.path;
    }
    // A save that cannot be written is skipped silently, as it always has been.
    (void)model.WriteLogFile(save_file_path);
}

CivPickerView::FileChoice CivPickerController::ChooseManualSaveFile()
{
    view.PlayAudio(SoundType::button);
    SYSTEMTIME now;
    GetLocalTime(&now);

    std::wstringstream file_name;
    file_name << L"FRCP Preset v" << VERSION << L" @"
              << std::setw(4) << std::setfill(L'0') << now.wYear << L"."
              << std::setw(2) << std::setfill(L'0') << now.wMonth << L"."
              << std::setw(2) << std::setfill(L'0') << now.wDay << L" "
              << std::setw(2) << std::setfill(L'0') << now.wHour
              << std::setw(2) << std::setfill(L'0') << now.wMinute
              << std::setw(2) << std::setfill(L'0') << now.wSecond << L".txt";

    CivPickerView::FileChoice choice = view.AskSaveFilePath(file_name.str(), SavesFolder());
    view.PlayAudio(SoundType::button);
    if (choice.chosen && choice.path.find(L".txt") == std::wstring::npos) choice.path.append(L".txt");
    return choice;
}

void CivPickerController::LoadLog(SaveType type)
{
    std::wstring load_file_path = log_file_path;
    if (type != SaveType::automatic)
    {
        const CivPickerView::FileChoice choice = ChooseLogToLoad(type);
        if (!choice.chosen) return;
        load_file_path = choice.path;
        PrepareLogLoad();
    }

    if (!model.LoadLogFile(load_file_path, type)) return;
    if (type != SaveType::automatic || model.IsCacheLoading())
    {
        ShowTab(view.GetCurrentTab());
        view.ValidateAllDlcToggles();
        model.SetStartingUp(false);
    }
}

CivPickerView::FileChoice CivPickerController::ChooseLogToLoad(SaveType type)
{
    view.PlayAudio(SoundType::button);
    if (type == SaveType::quick)
    {
        const std::wstring quicksave_path = QuicksavePath();
        return { FileExists(quicksave_path), quicksave_path };
    }

    CivPickerView::FileChoice choice = view.AskOpenFilePath(SavesFolder());
    if (!choice.chosen)
    {
        view.PlayAudio(SoundType::button);
        model.SetStartingUp(false);
    }
    return choice;
}

// A loaded log replaces the whole session: both logs are emptied and the default edition's full pool is restored
// first.
void CivPickerController::PrepareLogLoad()
{
    view.ClearLogTexts();
    model.SetEdition(DEFAULT_EDITION);
    EnableAll(false);
    model.ResetProgram(true);
}

// ---------------------------------------------------------------------------------------------------
// Share links: the web version's links, which both versions write and open

// P, the Log tab's share button and File > Copy Share Link: the pool with this round's draws.
void CivPickerController::CopyRoundLink()
{
    view.PlayAudio(SoundType::button);
    CopyLink(model.GetSharedPool());
}

// The Civ Pool tab's share button: the pool alone.
void CivPickerController::CopyPoolLink()
{
    view.PlayAudio(SoundType::button);
    SharedPool pool = model.GetSharedPool();
    pool.draw_count = 0;
    CopyLink(pool);
}

// If the clipboard can't take the link, the Share dialog shows the links to copy by hand (as copyShareLink does).
void CivPickerController::CopyLink(const SharedPool& pool)
{
    const ShareLink::Text link = ShareLink::Write(model.Catalog(), pool);
    if (view.CopyText(link.Characters())) view.ShowLinkCopied(pool.draw_count);
    else ShowShareLinks();
}

// File > Share ...: both links, in a dialog.
void CivPickerController::OpenShare()
{
    view.PlayAudio(SoundType::button);
    ShowShareLinks();
}

void CivPickerController::ShowShareLinks()
{
    SharedPool pool = model.GetSharedPool();
    const ShareLink::Text round_link = ShareLink::Write(model.Catalog(), pool);
    const int draw_count = pool.draw_count;
    pool.draw_count = 0;
    const ShareLink::Text pool_link = ShareLink::Write(model.Catalog(), pool);
    view.ShowShareDialog(main_window, { pool_link.Characters(), round_link.Characters(), draw_count });
}

// U and File > Open Link ...: a link someone shared replaces the pool and the round, after a look at what it holds
// unless nothing would be lost (the web version's offerLink). The log of past rounds stays.
void CivPickerController::OpenShareLink()
{
    view.PlayAudio(SoundType::button);
    const ShareLink::Opened opened = view.AskShareLink(main_window);
    if (!opened.ok) return;
    const bool replaces = model.HasCustomState() || view.HasDrawnLog();
    if (replaces && !view.ConfirmSharedPool(main_window, opened.pool)) return;
    // The link's draws come in quietly, as a saved log's do at startup.
    model.SetStartingUp(true);
    model.LoadSharedPool(opened.pool);
    model.SetStartingUp(false);
    ShowTab(view.GetCurrentTab());
    view.ValidateAllDlcToggles();
    if (replaces) view.PlayAudio(SoundType::button);
    view.ShowLinkLoaded();
}

// ---------------------------------------------------------------------------------------------------
// Dialogs and web pages

void CivPickerController::OpenTechTree()
{
    view.PlayAudio(SoundType::button);
    // aoe2techtree.net names some civs differently (data.json's techTree).
    const int civ = model.FindCivIndex(model.GetCurrentCiv());
    std::wstring techtree_url = L"https://aoe2techtree.net/#";
    if (civ != CivCatalog::NONE) techtree_url += model.Catalog().CivAt(civ).tech_tree;
    OpenUrl(techtree_url.c_str());
}

void CivPickerController::OpenSurvapp()
{
    view.PlayAudio(SoundType::button);
    OpenUrl(L"https://aoe2vils.net/");
}

void CivPickerController::OpenAuthorPage() { OpenUrl(L"https://linktr.ee/hjoerleif"); }

void CivPickerController::OpenOptions(HWND owner)
{
    view.PlayAudio(SoundType::button);
    view.ShowOptionsDialog(owner);
}

void CivPickerController::OpenHotkeys(HWND owner)
{
    view.PlayAudio(SoundType::button);
    view.ShowHotkeysDialog(owner);
}

void CivPickerController::OpenAbout(HWND owner)
{
    view.PlayAudio(SoundType::button);
    view.ShowAboutDialog(owner);
}

void CivPickerController::OpenHistory(HWND owner)
{
    view.PlayAudio(SoundType::button);
    view.ShowHistoryDialog(owner);
}

void CivPickerController::JoinLobby()
{
    view.PlayAudio(SoundType::button);
    const CivPickerView::LobbyInput input = view.AskLobbyCode(main_window);
    if (input.result == IDC_BUTTON_OK) LaunchLobby(input.code, L"aoe2de://0/", L"aoe2de://1/");
    else if (input.result == IDC_BUTTON_SPEC) LaunchLobby(input.code, L"aoe2de://1/", L"aoe2de://0/");
}

// A bare 9-digit code gets the prefix; a full link has the other mode's prefix swapped for this one.
void CivPickerController::LaunchLobby(std::wstring lobby_code, const wchar_t* prefix, const wchar_t* other_prefix) const
{
    if (lobby_code.length() == 9)
    {
        OpenUrl((prefix + lobby_code).c_str());
        return;
    }
    if (lobby_code.substr(0, 11) == other_prefix) lobby_code.replace(0, 11, prefix);
    OpenUrl(lobby_code.c_str());
}

bool CivPickerController::IsValidLobbyCode(const std::wstring& lobby_code)
{
    const size_t length = lobby_code.length();
    if (length != 9 && length != 20) return false;

    if (length == 20)
    {
        if (lobby_code.substr(0, 11) != L"aoe2de://0/" && lobby_code.substr(0, 11) != L"aoe2de://1/") return false;
        const std::wstring numbers = lobby_code.substr(12, 20);
        if (numbers.find_first_not_of(L"0123456789") != std::wstring::npos) return false;
    }
    if (length == 9 && lobby_code.find_first_not_of(L"0123456789") != std::wstring::npos) return false;
    return true;
}

// Entry point. As in HFDP ch. 12, main creates the model and the controller; the controller creates the view.
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine,
                      _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Static storage keeps the program's state off the heap and the stack (P10 rule 3). The catalog of editions,
    // DLCs and civs comes first, as everything else is made from it.
    static CivCatalog catalog;
    if (!LoadCatalog(catalog)) return FALSE;
    static CivPickerModel model(catalog);
    static CivPickerController controller(model);
    return controller.Run(hInstance, nCmdShow);
}
