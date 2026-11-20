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
    constexpr int DEFAULT_EDITION = 0;

    std::wstring GetExeFolder()
    {
        wchar_t exe_folder[MAX_PATH] = {};
        GetModuleFileName(NULL, exe_folder, MAX_PATH);
        wchar_t* last_slash = wcsrchr(exe_folder, L'\\');
        if (last_slash != NULL) *last_slash = L'\0';
        return exe_folder;
    }

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

bool CivPickerController::LoadCatalog(CivCatalog& catalog)
{
    const std::wstring path = GetExeFolder() + L"\\data.json";
    std::ifstream file(path.c_str(), std::ios::binary);
    std::stringstream contents;
    if (file) contents << file.rdbuf();
    if (file && catalog.Load(contents.str())) return true;
    const std::wstring problem = file ? catalog.Error() : L"data.json could not be read from " + path;
    (void)MessageBoxW(NULL, (problem + L"\n\nThe program needs data.json to start.").c_str(),
                      L"Fresh Random Civ Picker", MB_OK | MB_ICONERROR);
    return false;
}

int CivPickerController::Run(HINSTANCE app_instance, int show_command)
{
    view.SetInstance(app_instance);
    GenerateFilePaths();
    view.LoadSounds();
    LoadSettings();
    if (!view.StartGraphics() || !view.CreateMainWindow(show_command))
    {
        view.StopGraphics();
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
    view.SetImageFolder(exe_folder + L"\\images\\");
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

void CivPickerController::OnTabSelected(int tab_index) { ShowTab(tab_index); }

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

void CivPickerController::PrepareLogLoad()
{
    view.ClearLogTexts();
    model.SetEdition(DEFAULT_EDITION);
    EnableAll(false);
    model.ResetProgram(true);
}
