#pragma once

#include "framework.h"
#include <commctrl.h>
#include <string>

#include "CivPickerModel.h"
#include "ParchmentButtons.h"
#include "ParchmentDialog.h"
#include "ParchmentLog.h"
#include "ParchmentMenu.h"
#include "ParchmentRenderer.h"
#include "ParchmentScrollBar.h"
#include "ParchmentShell.h"
#include "ParchmentToast.h"
#include "ParchmentTooltip.h"
#include "ParchmentWidgets.h"
#include "PixelSurface.h"

class CivPickerController;

// VIEW of the Model-View-Controller design (HFDP ch. 12).
// Owns every window, control, dialog, image, font and sound. It observes the model and pulls the state it
// displays from it, but can only read it (it holds a const reference). User input is handed to the controller
// (its Strategy); the view knows how to change the display, the controller decides when. Dialogs that only
// present model data (About, Hotkeys, History) handle their own presentation-only interactions.
// Everything is drawn in the parchment theme of the web version (frcp.net): the Parchment* classes are the
// View's components (HFDP ch. 9 Composite of windows), and this class arranges them.

enum class SoundType { button, tabsound, hover, view, error };

class CivPickerView final : public CivPickerObserver
{
public:
    static constexpr int DRAW_TAB = 0;
    static constexpr int LOG_TAB = 1;
    static constexpr int CIV_POOL_TAB = 2;
    // The controls and menu items made from data.json have IDs from a base plus their number there: a civ toggle
    // its civ's ID, a DLC toggle its DLC's number, an edition row its edition's, a Game menu link its link's.
    static constexpr int CIV_CHECKBOX_BASE_ID = 5;
    static constexpr int EDITION_ROW_BASE_ID = 7950;
    static constexpr int DLC_SCROLLBAR_BASE_ID = 7960;
    static constexpr int DLC_TOGGLE_BASE_ID = 8000;
    static constexpr int EDITION_LINK_BASE_ID = 32800;
    static_assert(EDITION_ROW_BASE_ID + MAX_EDITIONS <= DLC_SCROLLBAR_BASE_ID, "edition rows and scrollbars apart");
    static_assert(DLC_SCROLLBAR_BASE_ID + MAX_EDITIONS <= DLC_TOGGLE_BASE_ID, "scrollbars and DLC toggles apart");
    static_assert(DLC_TOGGLE_BASE_ID + MAX_DLCS <= EDITION_LINK_BASE_ID, "DLC toggles and links apart");

    // A WM_COMMAND of the Options dialog together with the state of all its controls.
    struct OptionsEvent
    {
        int command_id;
        int notification;
        bool ui_sounds;
        bool civ_labels;
        bool iteration_label;
        bool icons;
        bool jingles;
        bool tooltips;
        bool draw_on_startup;
        bool contfresh;
        bool legacy_jingle;
    };
    struct LobbyInput { INT_PTR result; std::wstring code; };

    // A pool's two share links, and the number of draws the second one carries (the Share dialog).
    struct ShareLinks
    {
        const wchar_t* pool;
        const wchar_t* round;
        int draw_count;
    };
    struct FileChoice { bool chosen; std::wstring path; };

    // A line of the Hotkeys dialog: a key and what it does.
    struct HotkeyItem
    {
        const wchar_t* key;
        const wchar_t* text;
    };

    CivPickerView(const CivPickerModel& picker_model, CivPickerController& picker_controller);

    void SetInstance(HINSTANCE app_instance);
    void SetImageFolder(const std::wstring& folder);
    void SetJingleFolder(const std::wstring& folder);
    void LoadSounds();
    void UnloadSounds();
    bool StartGraphics();
    void StopGraphics();
    bool CreateMainWindow(int show_command);
    bool PreTranslateMessage(const MSG& message);

    void PlayAudio(SoundType type) const;
    void PlayJingle(const std::wstring& civ_name) const;
    void StopSound() const;

    int GetCurrentTab() const;
    int GetSelectedTab() const;
    void SelectTab(int tab_index);
    void ShowTabComponents(int tab_index);

    bool IsCivChecked(int civ_index) const;
    bool IsDlcChecked(int dlc) const;
    void SetDlcChecked(int dlc, bool checked);
    bool IsAutoToggleChecked() const;
    void SetAutoToggleChecked(bool checked);
    bool IsAutoResetChecked() const;
    void SetAutoResetChecked(bool checked);
    void ValidateDlcToggle(int dlc);
    void ValidateAllDlcToggles();

    void ShowRemainingLog(bool shown);
    void ToggleRemainLogCheckbox();
    void ClearDrawnLog();
    void ClearLogTexts();
    void RefreshCivIcon();
    void RefreshCivLabel();
    void RefreshCornerLabel();
    void DeactivateTooltips();
    void ShowInformation(const wchar_t* text, const wchar_t* caption);

    void ShowOptionsDialog(HWND owner);
    void ShowHotkeysDialog(HWND owner);
    void ShowAboutDialog(HWND owner);
    void ShowHistoryDialog(HWND owner);
    LobbyInput AskLobbyCode(HWND owner);
    void ShowShareDialog(HWND owner, const ShareLinks& links);
    ShareLink::Opened AskShareLink(HWND owner);
    bool ConfirmSharedPool(HWND owner, const SharedPool& pool);
    bool CopyText(const wchar_t* text) const;
    void ShowLinkCopied(int draw_count);
    void ShowLinkLoaded();
    bool HasDrawnLog() const;
    FileChoice AskSaveFilePath(const std::wstring& default_name, const std::wstring& initial_folder) const;
    FileChoice AskOpenFilePath(const std::wstring& initial_folder) const;
    void ShowJingleTypeOption(HWND dialog, bool shown) const;
    void ShowContfreshOptions(HWND dialog, bool shown) const;

    void OnPoolChanged(int civ_index) override;
    void OnBulkPoolChanged() override;
    void OnDlcPoolChanged() override;
    void OnEditionChanged() override;
    void OnResetStarted(bool auto_reset) override;
    void OnDrawSequenceBroken() override;
    void OnPoolReset(bool auto_reset) override;
    void OnDrawStarted() override;
    void OnEmptyPoolDraw() override;
    void OnCivDrawn() override;
    void OnUndrawAccepted() override;
    void OnCivUndrawn() override;
    void OnCivRedrawn() override;
    void OnActionRejected() override;
    void OnLogLoaded(SaveType type) override;
    void OnModelError(const std::wstring& message, bool fatal) override;

private:
    static constexpr int MAX_TOOLTIP_TEXT = 256;
    // The tooltips: these, then one per edition row and one per DLC.
    enum FixedTooltip
    {
        TOOLTIP_DRAW,
        TOOLTIP_RESET,
        TOOLTIP_CLEAR,
        TOOLTIP_ENABLEALL,
        TOOLTIP_DISABLEALL,
        TOOLTIP_TECHTREE,
        TOOLTIP_REMAININGTOGGLE,
        TOOLTIP_AUTOTOGGLE,
        TOOLTIP_AUTORESET,
        TOOLTIP_OPTIONS,
        TOOLTIP_SURVAPP,
        TOOLTIP_HISTORY,
        TOOLTIP_SHARE,
        TOOLTIP_POOLSHARE,
        FIXED_TOOLTIP_COUNT
    };
    static constexpr int EDITION_TOOLTIP_BASE = FIXED_TOOLTIP_COUNT;
    static constexpr int DLC_TOOLTIP_BASE = EDITION_TOOLTIP_BASE + MAX_EDITIONS;
    static constexpr int TOOLTIP_AMOUNT = DLC_TOOLTIP_BASE + MAX_DLCS;
    static constexpr int MENU_COUNT = 6;
    static constexpr int MAX_GAME_ITEMS = MAX_EDITIONS * MAX_EDITION_LINKS + 1;
    static constexpr int MAX_MENU_LABEL = 64;
    static constexpr int MAX_POOL_HOTKEYS = 4 + MAX_EDITIONS + DLC_HOTKEY_COUNT;
    static constexpr int MAX_HOTKEY_TEXT = 96;
    static constexpr int MAX_ABOUT_LEGAL = 512;
    static constexpr UINT_PTR TOAST_TIMER = 2;
    static constexpr int MAX_SHARE_NOTE = 128;
    static constexpr int MAX_PREVIEW_DECORATIONS = 8;
    static constexpr int MAX_PREVIEW_POOL = 768;
    static constexpr int MAX_PREVIEW_ROUND = 3072;
    static constexpr const wchar_t* TECHTREE_HOTKEY_LINE = L"\nHotkey: T (Draw tab only) / F4";
    static constexpr int MAX_LOG_TEXT = ParchmentLog::MAX_TEXT_LENGTH;
    static constexpr int MAX_HOTKEY_DECORATIONS = 160;
    static constexpr int MAX_MESSAGE_TEXT = 512;
    static constexpr int MAX_MESSAGE_TITLE = 64;
    static constexpr int ABOUT_DECORATIONS = 5;
    static constexpr int ABOUT_LINKS = 3;

    enum class DrawnLogUpdate { count_only, entry, blank_line };

    struct SoundResource
    {
        HRSRC info;
        HGLOBAL handle;
        LPVOID data;
        LPCWSTR name;
    };

    // One line of the Open Shared Link dialog's facts (dl.facts): a term and its text, which wraps.
    struct PreviewFact
    {
        const wchar_t* term;
        const wchar_t* text;
    };

    // A jingle file mapped into memory, where it stays until the program ends; null if it couldn't be.
    struct MappedJingle
    {
        const void* data;
    };

    struct TooltipTarget { HWND control; int tooltip_id; };

    // Where one DLC toggle goes (data.json dlcToggleLayout): its top-left corner, icon size and gap.
    struct DlcToggleSpot
    {
        int left;
        int top;
        int size;
        int gap;
    };

    struct Tooltip
    {
        wchar_t text[MAX_TOOLTIP_TEXT];
        bool external;
    };

    struct HotkeyItemSpan
    {
        const HotkeyItem* items;
        int count;
    };

    struct HotkeySection
    {
        const wchar_t* title;
        HotkeyItemSpan items;
        float left;
        float top;
        float column_width;
        int rows;
    };

    static LRESULT CALLBACK MainWindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam);
    static INT_PTR CALLBACK AboutDialogProc(HWND dialog, UINT message, WPARAM wParam, LPARAM lParam) noexcept;
    static INT_PTR CALLBACK OptionsDialogProc(HWND dialog, UINT message, WPARAM wParam, LPARAM lParam) noexcept;
    static INT_PTR CALLBACK HotkeysDialogProc(HWND dialog, UINT message, WPARAM wParam, LPARAM lParam) noexcept;
    static INT_PTR CALLBACK JoinLobbyDialogProc(HWND dialog, UINT message, WPARAM wParam, LPARAM lParam) noexcept;
    static INT_PTR CALLBACK HistoryDialogProc(HWND dialog, UINT message, WPARAM wParam, LPARAM lParam) noexcept;
    static INT_PTR CALLBACK MessageDialogProc(HWND dialog, UINT message, WPARAM wParam, LPARAM lParam) noexcept;
    static INT_PTR CALLBACK ShareDialogProc(HWND dialog, UINT message, WPARAM wParam, LPARAM lParam) noexcept;
    static INT_PTR CALLBACK OpenLinkDialogProc(HWND dialog, UINT message, WPARAM wParam, LPARAM lParam) noexcept;
    static INT_PTR CALLBACK PreviewDialogProc(HWND dialog, UINT message, WPARAM wParam, LPARAM lParam) noexcept;
    static CivPickerView* FromWindow(HWND window);
    static CivPickerView* AttachDialog(HWND dialog, UINT message, LPARAM lParam);

    // --- The main window (CivPickerView.cpp) ---
    LRESULT HandleMainWindowMessage(const MSG& message);
    LRESULT OnDestroy();
    LRESULT OnCreate(HWND window);
    LRESULT OnPaint();
    LRESULT OnHitTest(LPARAM lParam);
    LRESULT OnSetCursor(WPARAM wParam, LPARAM lParam);
    LRESULT OnWindowMoved(WPARAM wParam, LPARAM lParam);
    LRESULT OnActivate(WPARAM wParam);
    LRESULT OnHotkey(WPARAM hotkey_id);
    LRESULT OnCommand(WPARAM wParam, LPARAM lParam);
    LRESULT OnTimer(WPARAM timer_id);
    LRESULT OnMouseWheel(WPARAM wParam, LPARAM lParam);
    LRESULT OnScroll(LPARAM lParam);
    void NoteActivity();
    LRESULT OnMouseLeave();
    LRESULT OnMainWindowMouseMove();
    bool RegisterWindowClasses();
    PixelPoint CentredWindowPosition() const;

    // --- Building the main window (CivPickerViewLayout.cpp) ---
    bool BuildMainShell();
    bool DecorateMainBackdrop();
    bool CreateMainControls();
    bool CreateFrameControls();
    bool CreateDrawTabControls();
    bool CreateLogTabControls();
    bool ShowCivIconsInLogs();
    bool RaiseLogsOverCivName();
    bool CreatePoolTabControls();
    bool CreateEditionControls();
    bool CreateCivToggles();
    bool CreateLabel(Label& label, const ControlPlacement& placement, const Label::Style& style);
    ControlPlacement PlaceInClient(int id, const PixelRect& client_bounds, const wchar_t* text) const;
    bool BuildMenus();
    void SetUpTooltips();
    void SetUpDataTooltips();
    std::wstring DlcTooltipText(int dlc) const;
    void SetTooltip(int tooltip_id, const wchar_t* text);
    void CollectTooltipTargets();

    // --- Tabs and control visibility ---
    void ShowDrawTab(bool showing_enabled);
    void ShowLogTab(bool showing_enabled);
    void ShowCustomTab(bool showing_enabled);
    void ShowPoolCheckboxes(int edition);
    void HideCustomPoolCheckboxes();
    void ShowEditionControls(int edition);
    void CheckEditionRow(int edition);
    void MoveResetButton(int tab_index);

    // --- Labels, logs and tooltips ---
    std::wstring IterationText() const;
    int CivAtlasCell(const std::wstring& civ_name) const;
    void ShowCivIconFor(const std::wstring& civ_name);
    void UpdateDrawnLog(DrawnLogUpdate update);
    void AddDrawnLogEntry(std::wstring& log_text, const std::wstring& entry);
    void RemoveNewestDrawnLogEntry();
    void UpdateRemainingLog();
    void UpdateEnabledCivsCounter();
    void UpdateTooltips(bool drawn);
    void UpdateTooltipText(int tooltip_id, const std::wstring& text);
    void SetControlText(HWND control, const wchar_t* text);
    void ShowControl(HWND control, bool shown);
    bool HasDlcToggle(int dlc) const;

    // --- The DLC toggles' lists, which scroll once they don't fit (CivPickerDlcToggles.cpp) ---
    bool CreateDlcToggles();
    bool CreateDlcToggle(int dlc, const DlcToggleSpot& spot);
    bool CreateDlcScrollbar(int edition);
    int VisibleDlcRows(int edition) const;
    int DlcListHeight(int edition) const;
    PixelRect DlcListArea(int edition) const;
    void ShowDlcToggles(int edition);
    void LayOutDlcToggles(int edition);
    void HideDlcToggles(int edition);

    // --- Images and cursors (CivPickerViewImages.cpp) ---
    ImageSources CollectImageSources();
    void ReportMissingImages() const;
    void UpdateCursorSize();

    // --- Sounds and jingles ---
    void LoadSound(SoundResource& sound_resource);
    void LoadJingles();
    MappedJingle MapJingle(const std::wstring& file_name) const;
    void UnloadSound(SoundResource& sound_resource);
    void PlayResource(const SoundResource& sound_resource) const;
    void PlayWave(const void* wave) const;

    // --- Dialogs (CivPickerDialogs.cpp) ---
    INT_PTR RunDialog(const ParchmentDialogs::Run& run);
    INT_PTR BeginDialog(HWND dialog);
    bool BuildDialogLayouts();
    bool BuildHotkeysLayout();
    bool BuildPoolHotkeys();
    void AddPoolHotkey(const wchar_t* key, const std::wstring& text);
    std::wstring DlcSlotNames(int slot) const;
    float KeyWidth(const wchar_t* key);
    float HotkeyItemWidth(const HotkeyItemSpan& items);
    float AddHotkeySection(const HotkeySection& section);
    void AddHotkeyDecoration(const DialogDecoration& decoration);
    bool BuildAboutLayout();
    void ComposeAboutLegal();
    bool BuildMessageLayout(const wchar_t* text, const wchar_t* caption);
    float ParagraphHeight(const wchar_t* text, float width);
    void ShowHistoryCivIcon(HWND dialog);
    int HistoryListPosition(const std::wstring& civ_name) const;
    void InitialiseOptionsDialog(HWND dialog);
    void SetFreshnessText(HWND dialog, int civs) const;
    void OnContfreshSlider(HWND dialog, HWND slider);
    bool OnOptionsCommand(HWND dialog, int command_id, int notification);
    bool SubmitLobbyCode(HWND dialog, int command_id);
    void InitialiseHistoryDialog(HWND dialog);
    bool OnHistoryCommand(HWND dialog, int command_id, int notification);
    void OnHistorySelectionChanged(HWND dialog, bool show_pool_info, bool history_jingle);
    void OnHistorySearchChanged(HWND dialog);
    void UpdateCivAddedInfo(HWND dialog, int dlc) const;
    void UpdateCivPoolInfo(HWND dialog, const std::wstring& civ_name) const;
    void HidePoolInfo(HWND dialog) const;
    bool OnAboutCommand(HWND dialog, int command_id);

    // --- Share links (CivPickerShare.cpp) ---
    bool BuildShareLayout(const ShareLinks& links);
    INT_PTR BeginShareDialog(HWND dialog);
    bool OnShareCommand(HWND dialog, int command_id);
    void CopyFieldLink(HWND dialog, int field_id);
    bool SubmitShareLink(HWND dialog);
    bool BuildPreviewLayout(const SharedPool& pool);
    void ComposePreviewPool(const SharedPool& pool);
    void ComposePreviewRound(const SharedPool& pool);
    float AddPreviewFact(const PreviewFact& fact, float top, float term_width);
    void AddPreviewDecoration(const DialogDecoration& decoration);
    void ShowToast(const wchar_t* text);

    const CivPickerModel& model;
    CivPickerController& controller;
    HINSTANCE instance = NULL;
    std::wstring image_folder;
    std::wstring jingle_folder;
    WCHAR title[MAX_LOADSTRING] = {};
    WCHAR window_class[MAX_LOADSTRING] = {};
    int current_tab = DRAW_TAB;
    uint16_t drawn_log_linecount = 0;

    ParchmentRenderer renderer;
    PixelSurface back_buffer;
    ShellWorkspace shell_workspace;
    ParchmentShell main_shell;
    ParchmentTooltip tooltip;
    ParchmentToast toast;
    ParchmentDialogs dialogs;
    HMENU text_menu = NULL;

    HWND main_window = NULL;
    CloseButton close_button;
    MenuItemSpec game_items[MAX_GAME_ITEMS] = {};
    wchar_t game_item_labels[MAX_GAME_ITEMS][MAX_MENU_LABEL] = {};
    MenuSpec menus[MENU_COUNT] = {};
    ParchmentMenuBar menu_bar;
    TabStrip tab_strip;
    Label label_corner;
    Label label_centre;
    Label label_drawncount;
    Label label_remainingcount;
    Label label_enabledcount;
    TextButton button_draw;
    TextButton button_reset;
    TextButton button_enableall;
    TextButton button_disableall;
    TextButton button_clearlog;
    PictureButton button_history;
    FramedIconButton button_share;
    FramedIconButton button_poolshare;
    AtlasButton button_techtree;
    AtlasButton button_options;
    FramedIconButton button_survapp;
    ParchmentLog drawn_log;
    ParchmentLog remaining_log;
    IconLabels civ_labels;
    PictureButton civ_icon;
    PictureButton edition_icon;
    Checkbox checkbox_showremainlog;
    Checkbox checkbox_autoreset;
    Checkbox checkbox_autotoggle;
    EditionRow edition_rows[MAX_EDITIONS];
    IconToggle civ_toggles[MAX_CIVS];
    IconToggle dlc_toggles[MAX_DLCS];
    ScrollBar dlc_scrollbars[MAX_EDITIONS];
    HMONITOR cursor_monitor = NULL;
    ImageFile civ_icon_files[MAX_CIVS + 1] = {};
    ImageFile dlc_icon_files[EDITION_LOOKS * MAX_EDITIONS + MAX_DLCS] = {};
    ImageFile edition_banner_files[MAX_EDITIONS] = {};
    TooltipTarget tooltip_targets[TOOLTIP_AMOUNT] = {};
    Tooltip tooltips[TOOLTIP_AMOUNT] = {};
    int shown_tooltip = -1;

    SoundResource sound_resources[SOUND_AMOUNT] = {};
    MappedJingle jingles[MAX_CIVS] = {};
    MappedJingle legacy_jingles[MAX_CIVS] = {};
    MappedJingle random_jingle = {};
    MappedJingle random_legacy_jingle = {};

    std::wstring history_selected_civ;
    int history_search_length = 0;
    std::wstring lobby_code_input;
    bool lobby_error_shown = false;

    WrappedLines paragraph_lines;
    DialogDecoration hotkey_decorations[MAX_HOTKEY_DECORATIONS] = {};
    int hotkey_decoration_count = 0;
    DialogPart hotkeys_parts[1] = {};
    HotkeyItem pool_hotkeys[MAX_POOL_HOTKEYS] = {};
    wchar_t pool_hotkey_texts[MAX_POOL_HOTKEYS][MAX_HOTKEY_TEXT] = {};
    int pool_hotkey_count = 0;
    DialogLayout hotkeys_layout = {};
    wchar_t about_version[64] = L"";
    wchar_t about_legal[MAX_ABOUT_LEGAL] = L"";
    DialogDecoration about_decorations[ABOUT_DECORATIONS] = {};
    DialogLink about_links[ABOUT_LINKS] = {};
    DialogPart about_parts[1] = {};
    DialogLayout about_layout = {};
    wchar_t share_pool_link[ShareLink::MAX_LINK_LENGTH] = L"";
    wchar_t share_round_link[ShareLink::MAX_LINK_LENGTH] = L"";
    wchar_t share_note[MAX_SHARE_NOTE] = L"";
    DialogDecoration share_decorations[4] = {};
    DialogPart share_parts[5] = {};
    DialogLayout share_layout = {};
    wchar_t share_link_input[ShareLink::MAX_INPUT_LENGTH + 1] = L"";
    ShareLink::Opened opened_link = {};
    bool link_error_shown = false;
    wchar_t preview_edition[MAX_MESSAGE_TITLE] = L"";
    wchar_t preview_pool[MAX_PREVIEW_POOL] = L"";
    wchar_t preview_round[MAX_PREVIEW_ROUND] = L"";
    DialogDecoration preview_decorations[MAX_PREVIEW_DECORATIONS] = {};
    int preview_decoration_count = 0;
    DialogPart preview_parts[2] = {};
    DialogLayout preview_layout = {};
    wchar_t message_text[MAX_MESSAGE_TEXT] = L"";
    wchar_t message_title[MAX_MESSAGE_TITLE] = L"";
    DialogDecoration message_decorations[1] = {};
    DialogPart message_parts[1] = {};
    DialogLayout message_layout = {};
};
