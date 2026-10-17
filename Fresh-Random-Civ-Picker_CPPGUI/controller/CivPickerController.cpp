#include "CivPickerController.h"

#include <shellapi.h>
#include <shlwapi.h>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>

#include "Assertions.h"
#include "Resource.h"

namespace
{
    // data.json's first edition is the default one.
    constexpr int DEFAULT_EDITION = 0;

    std::wstring GetExeFolder()
    {
        wchar_t exe_folder[MAX_PATH] = {};
        GetModuleFileName(NULL, exe_folder, MAX_PATH);
        wchar_t* last_slash = wcsrchr(exe_folder, L'\\');
        if (last_slash != NULL) *last_slash = L'\0';
        return exe_folder;
    }

    void OpenUrl(const wchar_t* url) { ShellExecute(NULL, L"open", url, NULL, NULL, SW_SHOWNORMAL); }

    // How many civs continuous freshness keeps out (ContFreshCivs), at least 1 and at most CONTFRESH_MOST. Settings
    // saved before have a strength from 1 to 10 (ContFreshStrengthValue): 1 kept out the previous civ, 10 the most,
    // and each step between a tenth of half the civs. Without either, the most, as in the web version.
    uint8_t ReadFreshness(const wchar_t* ini_file, int civ_count)
    {
        constexpr int NOT_SAVED = -1;
        constexpr int STRONGEST = 10;
        if (!c_assert(ini_file != nullptr) || !c_assert(civ_count > 0)) return CONTFRESH_MOST;
        int civs = static_cast<int>(GetPrivateProfileInt(L"Settings", L"ContFreshCivs", NOT_SAVED, ini_file));
        if (civs == NOT_SAVED)
        {
            const int strength =
                static_cast<int>(GetPrivateProfileInt(L"Settings", L"ContFreshStrengthValue", NOT_SAVED, ini_file));
            const bool strongest = strength == NOT_SAVED || strength >= STRONGEST;
            const int stepped = strength <= 1 ? 1 : civ_count / 2 * strength / STRONGEST;
            civs = strongest ? CONTFRESH_MOST : stepped;
        }
        if (civs < 1) return 1;
        return static_cast<uint8_t>(std::min(civs, static_cast<int>(CONTFRESH_MOST)));
    }
}

CivPickerController::CivPickerController(CivPickerModel& picker_model) :
    model(picker_model),
    view(picker_model, *this)
{
    model.SetObserver(view);
}

// Reads the editions, DLCs and civs from data.json in the exe's folder (the build copies it there). If the program
// can't start with it, says why and returns false.
bool CivPickerController::LoadCatalog(CivCatalog& catalog)
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
        // The program says that it stops rather than vanish; the debugger output names the check that failed.
        (void)MessageBoxW(NULL, L"The program could not build its window and stops.", L"Fresh Random Civ Picker",
                          MB_OK | MB_ICONERROR);
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
    MSG message = {};
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
    settings.contfresh_civs = ReadFreshness(ini_file, model.Catalog().CivCount());
    LoadHotkeys();
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
    WritePrivateProfileString(L"Settings", L"ContFreshCivs", std::to_wstring(settings.contfresh_civs).c_str(),
                              settings_file_path.c_str());
    // The strength the number of civs replaced goes; a key that is already gone leaves nothing to do.
    (void)WritePrivateProfileString(L"Settings", L"ContFreshStrengthValue", nullptr, settings_file_path.c_str());
    SaveHotkeys();
}

void CivPickerController::WriteSetting(const wchar_t* key, bool enabled) const
{
    WritePrivateProfileString(L"Settings", key, enabled ? L"1" : L"0", settings_file_path.c_str());
}

std::wstring CivPickerController::SavesFolder() const
{
    wchar_t saves_folder[MAX_PATH] = {};
    wcscpy_s(saves_folder, GetExeFolder().c_str());
    PathAppend(saves_folder, L"saves");
    return saves_folder;
}

std::wstring CivPickerController::QuicksavePath() const
{
    wchar_t quicksave_path[MAX_PATH] = {};
    wcscpy_s(quicksave_path, SavesFolder().c_str());
    PathAppend(quicksave_path, L"quicksave.txt");
    return quicksave_path;
}

bool CivPickerController::FileExists(const std::wstring& path) const
{
    const DWORD attributes = GetFileAttributes(path.c_str());
    return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

// ---------------------------------------------------------------------------------------------------
// Tabs

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
        case IDC_BUTTON_CUSTOMISEHOTKEYS:
            CustomiseHotkeys(dialog);
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
        default:
            break;
    }
}

void CivPickerController::OnFreshnessChanged(int civs) { model.SetFreshness(civs); }

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
        default:
            (void)c_assert(false);
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
    SYSTEMTIME now = {};
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

// The Game menu's links to the editions' pages (data.json's links).
bool CivPickerController::OpenEditionLink(int command_id)
{
    const int link = command_id - CivPickerView::EDITION_LINK_BASE_ID;
    if (link < 0 || link >= model.Catalog().LinkCount()) return false;
    view.PlayAudio(SoundType::button);
    OpenUrl(model.Catalog().LinkAt(link).url.c_str());
    return true;
}

// The edition's store page, or the note data.json has for an edition without one.
void CivPickerController::OpenEditionPage()
{
    const EditionInfo& edition = model.Catalog().EditionAt(model.GetEdition());
    if (!edition.url.empty()) OpenUrl(edition.url.c_str());
    else view.ShowInformation(edition.store_note.c_str(), edition.short_name.c_str());
}

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
