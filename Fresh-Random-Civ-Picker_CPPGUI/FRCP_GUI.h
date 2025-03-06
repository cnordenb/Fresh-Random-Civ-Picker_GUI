#pragma once
#define IDC_STATIC_TEXT 1001
#define IDC_HYPERLINK 1002
#include "resource.h"

#include "framework.h"
#include "FRCP_GUI.h"
#include <iostream>
#include <fstream>
#include <sstream>
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
#include <map>
#include <thread>





#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")



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
#define HOTKEY_ID_T 9
#define HOTKEY_ID_1 10
#define HOTKEY_ID_2 11
#define HOTKEY_ID_3 12
#define HOTKEY_ID_Q 13
#define HOTKEY_ID_W 14
#define HOTKEY_ID_E 15
#define HOTKEY_ID_A 16
#define HOTKEY_ID_S 17
#define HOTKEY_ID_D 18
#define HOTKEY_ID_F 19
#define HOTKEY_ID_G 20
#define HOTKEY_ID_H 21
#define HOTKEY_ID_R 22


#define DT_UNDERLINE 0x80000000
#define MAX_LENGTH 15 

#define MIN_WIDTH 550
#define MIN_HEIGHT 440

#define BUTTON_WIDTH 100
#define BUTTON_HEIGHT 30

#define TOOLTIP_DRAW                0
#define TOOLTIP_RESET               1
#define TOOLTIP_CLEAR               2
#define TOOLTIP_ENABLEALL           3
#define TOOLTIP_DISABLEALL          4
#define TOOLTIP_TECHTREE            5
#define TOOLTIP_REMAININGTOGGLE     6
#define TOOLTIP_DE                  7
#define TOOLTIP_HD                  8
#define TOOLTIP_AOK                 9
#define TOOLTIP_ROYALS			    10
#define TOOLTIP_ROME				11
#define TOOLTIP_INDIA				12
#define TOOLTIP_DUKES				13
#define TOOLTIP_WEST				14
#define TOOLTIP_KHANS				15
#define TOOLTIP_RAJAS				16
#define TOOLTIP_AFRICANS			17
#define TOOLTIP_FORGOTTEN			18
#define TOOLTIP_AOC				    19
#define TOOLTIP_AUTOTOGGLE  	    20
#define TOOLTIP_AUTORESET  	        21

wchar_t INI_FILE_PATH[MAX_PATH];
wchar_t LOG_FILE_PATH[MAX_PATH];

std::map<HWND, WNDPROC> originalButtonProcs;

// Global Variables
PAINTSTRUCT paint_struct;
HDC device_handling_context;
RECT rectangle_struct;
HINSTANCE instance;                                // current instance
WCHAR title[MAX_LOADSTRING];                  // The title bar text
WCHAR window_class[MAX_LOADSTRING];            // the main window class name


HWND label_corner, label_centre, label_drawncount, label_remainingcount;    // labels

HWND button_draw, button_reset, button_enableall, button_disableall, button_clearlog, button_techtree, button_options;	    // buttons

HWND drawn_log, remaining_log;				 // textfield for log tab


HWND civ_icon, edition_icon;						// icons

HWND checkbox_showremainlog;

HWND hOptionsDlg = NULL;


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

HBITMAP icon_techtree;                                                                             // tech tree button icon


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

bool startup = true;
bool persistent_logging = true;
bool draw_on_startup = false;
bool icons_enabled = true;
bool jingles_enabled = false;
bool labels_enabled = true;
bool legacy_jingle_enabled = false;
bool ui_sounds_enabled = false;
bool tooltips_enabled = true;
bool pool_altered = false;
bool autoreset_enabled = true;
bool autotoggle_enabled = true;
bool remainlog_enabled = true;
bool reset_state = true;
bool hotkey_pressed = false;




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
std::pair<std::wstring, bool> civ_enabled[MAX_CIVS];

std::pair<std::wstring, enum edition> civ_edition[MAX_CIVS] = {
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


std::pair<std::wstring, enum dlc> civ_dlc[MAX_CIVS] = {
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


std::wstring drawn_civs[MAX_CIVS] = { L"" };

bool accessor_out_of_bounds = false;             // for unit testing
int times_drawn[MAX_CIVS] = { 0 };               // for unit testing
HFONT font_underline = NULL;
HWND tab;
std::vector<std::wstring> civs;

// Function declarations
HBITMAP FetchIcon(const std::wstring &);
void CreateTabs(HWND);
void ShowTabComponents(int, HWND);

void ResetProgram();

void DrawCiv();
void KillApplication();

void SaveSettings();
void LoadSettings();

void SaveLog();
void LoadLog(HWND);

void EnableHotkeys(HWND);
void DisableHotkeys(HWND);

void CreateUnderlineFont();
void LoadImages();

void MuteSounds();
void PlayJingle(std::wstring &);
bool VerifiedLegacyCiv(const std::wstring &);

void InitialiseCivs();
void InitialiseCivStates();
void InitialiseCustomPoolCheckboxes(HWND);
void AddCiv(const std::wstring &);
void RemoveCiv(const std::wstring &);
void SetCivStatus(const std::wstring &, bool);
bool GetCivStatus(const std::wstring &);
void ShowAllPoolCheckboxes();
void HideCustomPoolCheckboxes();
void ShowHDPoolCheckboxes();
void ShowAOCPoolCheckboxes();
void DLCToggles(edition);
void OpenTechTree();
void OpenOptions(HWND);
void OpenHotkeys(HWND);

void CreateCheckboxes(HWND);
void CreateImages(HWND);
void CreateButtons(HWND);
void CreateLabels(HWND);
void CreateTextfields(HWND);
void CreateRadiobuttons(HWND);

void SubclassButtons();
void AddTooltips();
void CreateTooltips(HWND);

edition GetCivEdition(const std::wstring &);
dlc GetCivDLC(const std::wstring &);

void SetEditionState(HWND hWnd, edition edition);

void EnableAll(HWND);
void DisableAll(HWND);

void ShowDrawTab(bool, HWND);
void ShowLogTab(bool);
void ShowCustomTab(bool);
void ShowDEDLCCheckboxes(bool);
void ShowHDDLCCheckboxes(bool);
void ShowAOCCheckbox(bool);
int GetDlcCheckboxId(dlc);
HWND GetCivCheckbox(const std::wstring &);


void UpdateDrawnLog(bool, bool, bool);
void UpdateRemainingLog(bool);
void ToggleRemainLog();

bool DlcEmpty(dlc);
void ToggleDlc(dlc, HWND);
void EnableDlc(dlc, HWND);
void DisableDlc(dlc, HWND);
void ValidateDlcToggle(HWND, dlc);
void ValidateAllDlcToggles(HWND);
void ToggleAutoToggle(HWND);
void ToggleAutoReset(HWND);


void AddTooltip(HWND, HWND, LPCWSTR);

void ActivateTooltip(HWND, TOOLINFO *, POINT);



HWND hwndTooltip[22];
int hwnd_length = sizeof(hwndTooltip) / sizeof(hwndTooltip[0]);

void UpdateTooltipText(HWND hwndTool, HWND hwndTip, LPCWSTR newText);


LPCWSTR StringCleaner(const std::wstring &);

HWND lastTooltipControl = NULL;
bool tooltipActive = false;


void SubclassButton(HWND);


int GetWindowWidth(HWND);
int GetWindowHeight(HWND);



std::string ConvertToString(const std::wstring &);
HWND CreateCheckbox(HWND hWnd, HINSTANCE hInstance, int x, int y, int width, int height, int id, LPCWSTR text);

void GenerateFilePaths();

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    AboutDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    OptionsDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    HotkeysDlgProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    HyperlinkProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    ButtonProc(HWND, UINT, WPARAM, LPARAM);
