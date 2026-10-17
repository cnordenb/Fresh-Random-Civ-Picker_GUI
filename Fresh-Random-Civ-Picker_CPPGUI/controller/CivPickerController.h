#pragma once

#include <string>

#include "CivPickerModel.h"
#include "CivPickerView.h"

// CONTROLLER of the Model-View-Controller design (HFDP ch. 12).
// Takes the user's input from the view (menus, buttons, hotkeys, the Options dialog), works out what
// it means and turns it into requests on the model, and tells the view when to change. It holds no
// application logic of its own. As in HFDP, the controller creates its view; Run() is the startup
// sequence that wires model, view and controller together and runs the message loop.
class CivPickerController final
{
public:
    explicit CivPickerController(CivPickerModel& picker_model);

    int Run(HINSTANCE app_instance, int show_command);

    void OnMainWindowCreated(HWND window);
    void OnActivate(WPARAM activation);
    void OnHotkey(Keystroke keystroke);
    bool OnCommand(int command_id);
    void OnTabSelected(int tab_index);
    bool OnOptionsCommand(HWND dialog, const CivPickerView::OptionsEvent& options_event);
    void OnFreshnessChanged(int civs);
    void OnKeyChosen(HWND editor, HotkeyAction action, Keystroke keystroke);
    void OnDefaultKeysChosen(HWND editor);
    void OpenAuthorPage();
    void Quit();
    void OnFatalError();
     
    static bool IsValidLobbyCode(const std::wstring& lobby_code);
    static bool LoadCatalog(CivCatalog& catalog);

private:
    int RunMessageLoop(HINSTANCE app_instance);
    void GenerateFilePaths();
    void LoadSettings();
    void SaveSettings();
    void WriteSetting(const wchar_t* key, bool enabled) const;
    void LoadHotkeys();
    void SaveHotkeys() const;
    std::wstring SavesFolder() const;
    std::wstring QuicksavePath() const;
    bool FileExists(const std::wstring& path) const;
    void EnableHotkeys();
    void DisableHotkeys();
    void RunStartupDraw();
    void ShowTab(int tab_index);
    void SelectTab(int tab_index);

    HotkeyScope ShownTabScope() const;
    void RunHotkey(HotkeyAction action);
    void RunGlobalHotkey(HotkeyAction action);
    void CycleTabs();
    void ToggleDisplaySetting(HotkeyAction action);
    void RunDrawTabHotkey(HotkeyAction action);
    void RunLogTabHotkey(HotkeyAction action);
    void RunCivPoolHotkey(HotkeyAction action);
    void SelectEditionByPlace(int edition);
    void ToggleDlcBySlot(int slot);
    void CustomiseHotkeys(HWND owner);
    bool IsAnotherProgramsHotkey(Keystroke keystroke) const;

    bool IsPoolControl(int command_id) const;
    void HandleCivCheckbox(int command_id);
    void HandleEditionRadio(int command_id);
    void HandleDlcCheckbox(int command_id);
    bool OpenWebPage(int command_id);
    bool OpenEditionLink(int command_id);
    bool HandleMenuCommand(int command_id);
    bool HandleButtonCommand(int command_id);
    void DrawCivWithinRateCap();
    void OpenEditionPage();

    void PlayComboFeedback(int notification);
    void ApplyOptions(HWND dialog, const CivPickerView::OptionsEvent& options_event);
    void HandleOptionsClick(HWND dialog, int command_id);

    void ToggleDlc(int dlc);
    void ToggleAutoToggle();
    void ToggleAutoReset();
    void ToggleRemainLog();
    void EnableAll(bool sound_acceptable);
    void DisableAll(bool sound_acceptable);
    void ClearDrawnLog();
    void Undo();
    void LoadCachedLog();

    void SaveLog(SaveType type);
    CivPickerView::FileChoice ChooseManualSaveFile();
    void LoadLog(SaveType type);
    CivPickerView::FileChoice ChooseLogToLoad(SaveType type);
    void PrepareLogLoad();

    void CopyRoundLink();
    void CopyPoolLink();
    void CopyLink(const SharedPool& pool);
    void OpenShare();
    void ShowShareLinks();
    void OpenShareLink();

    void OpenTechTree();
    void OpenSurvapp();
    void OpenOptions(HWND owner);
    void OpenHotkeys(HWND owner);
    void OpenAbout(HWND owner);
    void OpenHistory(HWND owner);
    void JoinLobby();
    void LaunchLobby(std::wstring lobby_code, const wchar_t* prefix, const wchar_t* other_prefix) const;

    CivPickerModel& model;
    CivPickerView view;
    HWND main_window = NULL;
    std::wstring settings_file_path;
    std::wstring log_file_path;
    bool hotkey_pressed = false;
    ULONGLONG last_draw_tick = 0;
};
