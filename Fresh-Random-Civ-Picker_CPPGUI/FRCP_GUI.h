#pragma once
#define IDC_STATIC_TEXT 1001
#define IDC_HYPERLINK 1002
#include "resource.h"

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
#include <map>




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

std::map<HWND, WNDPROC> originalButtonProcs;
// Global Variables
PAINTSTRUCT paint_struct;
HDC device_handling_context;
RECT rectangle_struct;
HINSTANCE instance;                                // current instance
WCHAR title[MAX_LOADSTRING];                  // The title bar text
WCHAR window_class[MAX_LOADSTRING];            // the main window class name


HWND label_corner, label_centre, label_drawncount, label_remainingcount;    // labels

HWND button_draw, button_reset, button_enableall, button_disableall, button_clearlog, button_techtree;	    // buttons

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
void OpenTechTree();

edition GetCivEdition(const std::wstring &);
dlc GetCivDLC(const std::wstring &);

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

bool DlcEmpty(dlc);
void ToggleDlc(dlc, bool, HWND);
void ValidateDlcToggle(HWND, dlc);
void ValidateAllDlcToggles(HWND);

void CreateTooltip(HWND);
void AddTooltip(HWND, HWND, LPCWSTR);
void ActivateTooltip(HWND, TOOLINFO *, POINT);
HWND hwndTooltip;

void SubclassButton(HWND);

int GetWindowWidth(HWND);
int GetWindowHeight(HWND);


std::string ConvertToString(const std::wstring &);
HWND CreateCheckbox(HWND hWnd, HINSTANCE hInstance, int x, int y, int width, int height, int id, LPCWSTR text);



// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    AboutDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    OptionsDlgProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    HyperlinkProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    ButtonProc(HWND, UINT, WPARAM, LPARAM);

