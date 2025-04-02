#pragma once
#define IDC_STATIC_TEXT 1001
#define IDC_HYPERLINK 1002

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
#include <commdlg.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <strsafe.h>
#include <gdiplus.h>
#include <mmsystem.h>
#include <map>
#include <thread>
#include <iomanip>
#include <libloaderapi.h>





#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define VERSION L"1.3.2"

#define MAX_CIVS 45
#define DLC_AMOUNT 10
#define EDITION_AMOUNT 3
#define MAX_LOADSTRING 100
#define HOTKEY_AMOUNT 37

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
#define HOTKEY_ID_F1 23
#define HOTKEY_ID_B 24
#define HOTKEY_ID_F2 25
#define HOTKEY_ID_F3 26
#define HOTKEY_ID_F4 27
#define HOTKEY_ID_F5 28
#define HOTKEY_ID_F6 29
#define HOTKEY_ID_F9 30
#define HOTKEY_ID_CTRLS 31
#define HOTKEY_ID_CTRLR 32
#define HOTKEY_ID_CTRLF 33
#define HOTKEY_ID_CTRLZ 34
#define HOTKEY_ID_CTRLX 35
#define HOTKEY_ID_CTRLT 36


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
#define TOOLTIP_OPTIONS             22
#define TOOLTIP_SURVAPP			    23

class Hotkey
{
public:
	UINT modifier;
	UINT key;
	UINT id;

	Hotkey(UINT modifier, UINT key, UINT id) : modifier(modifier), key(key), id(id) {}
};

Hotkey hotkey[HOTKEY_AMOUNT] = { Hotkey(0, VK_TAB, HOTKEY_ID_TAB), Hotkey(0, VK_SPACE, HOTKEY_ID_SPACE),
Hotkey(0, VK_RETURN, HOTKEY_ID_RETURN), Hotkey(0, VK_ESCAPE, HOTKEY_ID_ESC), Hotkey(0, 0x5A, HOTKEY_ID_Z),
Hotkey(0, 0x58, HOTKEY_ID_X), Hotkey(0, 0x43, HOTKEY_ID_C), Hotkey(0, 0x56, HOTKEY_ID_V), Hotkey(0, 0x54, HOTKEY_ID_T),
Hotkey(0, 0x31, HOTKEY_ID_1), Hotkey(0, 0x32, HOTKEY_ID_2), Hotkey(0, 0x33, HOTKEY_ID_3), Hotkey(0, 0x51, HOTKEY_ID_Q),
Hotkey(0, 0x57, HOTKEY_ID_W), Hotkey(0, 0x45, HOTKEY_ID_E), Hotkey(0, 0x41, HOTKEY_ID_A), Hotkey(0, 0x53, HOTKEY_ID_S),
Hotkey(0, 0x44, HOTKEY_ID_D), Hotkey(0, 0x46, HOTKEY_ID_F), Hotkey(0, 0x47, HOTKEY_ID_G), Hotkey(0, 0x48, HOTKEY_ID_H),
Hotkey(0, 0x52, HOTKEY_ID_R), Hotkey(0, 0x54, HOTKEY_ID_T), Hotkey(0, VK_F1, HOTKEY_ID_F1), Hotkey(0, 0x42, HOTKEY_ID_B),
Hotkey(0, VK_F2, HOTKEY_ID_F2), Hotkey(0, VK_F3, HOTKEY_ID_F3), Hotkey(0, VK_F4, HOTKEY_ID_F4), Hotkey(0, VK_F5, HOTKEY_ID_F5),
Hotkey(0, VK_F6, HOTKEY_ID_F5), Hotkey(0, VK_F9, HOTKEY_ID_F9), Hotkey(MOD_CONTROL, 0x53, HOTKEY_ID_CTRLS),
Hotkey(MOD_CONTROL, 0x52, HOTKEY_ID_CTRLR), Hotkey(MOD_CONTROL, 0x46, HOTKEY_ID_CTRLF), Hotkey(MOD_CONTROL, 0x5A, HOTKEY_ID_CTRLZ),
Hotkey(MOD_CONTROL, 0x58, HOTKEY_ID_CTRLX), Hotkey(MOD_CONTROL, 0x54, HOTKEY_ID_CTRLT)};

class Civ
{
public:
	std::wstring name;
	bool enabled;
	enum edition edition;
	enum dlc dlc;
	HWND checkbox;

	Civ(std::wstring name, bool enabled, enum edition edition, enum dlc dlc, HWND checkbox) : name(name), enabled(enabled), edition(edition), dlc(dlc), checkbox(checkbox)  {}

    void SetEnabled(bool enabled) { this->enabled = enabled; }
};


wchar_t INI_FILE_PATH[MAX_PATH];
wchar_t LOG_FILE_PATH[MAX_PATH];

std::map<HWND, WNDPROC> originalButtonProcs;

PAINTSTRUCT paint_struct;
HDC device_handling_context;
RECT rectangle_struct;
HINSTANCE instance;
WCHAR title[MAX_LOADSTRING];
WCHAR window_class[MAX_LOADSTRING];


HWND label_corner, label_centre, label_drawncount, label_remainingcount;

HWND button_draw, button_reset, button_enableall, button_disableall, button_clearlog, button_techtree, button_options, button_survapp;

HWND drawn_log, remaining_log;

HWND checkbox_showremainlog;

HWND hOptionsDlg = NULL;


HWND checkbox_khans, checkbox_dukes, checkbox_west, checkbox_india, checkbox_rome, checkbox_royals,
checkbox_forgotten, checkbox_africans, checkbox_rajas,
checkbox_aoc;

HWND checkbox_autoreset, checkbox_autotoggle;

HWND radiobutton_de, radiobutton_hd, radiobutton_aok;

HFONT font_bold;

HBRUSH brush_white;
HBRUSH brush_black;


int iterator = 0;
int drawnlog_length = GetWindowTextLength(drawn_log);
int remaininglog_length = GetWindowTextLength(remaining_log);
int current_tab = 0;
int custom_max_civs = MAX_CIVS;
std::wstring label_text = std::to_wstring(iterator + 1) + L"/" + std::to_wstring(MAX_CIVS);
std::wstring log_entry;
std::wstring drawnlog_text, remaininglog_text;
std::wstring hlabel_default;
std::wstring current_civ = L"Random";


bool startup = true;
bool persistent_logging = true;
bool draw_on_startup = false;
bool civ_labels_enabled = true;
bool iteration_label_enabled = true;
bool tooltips_enabled = true;
bool autoreset_enabled = true;
bool autotoggle_enabled = true;
bool remainlog_enabled = true;
bool reset_state = true;
bool hotkey_pressed = false;
bool redrawable = false;
bool undrawable = false;

enum edition
{
    DE,
    HD,
    AOK
};

enum dlc
{
    aok,
    aoc,

    forgotten,
    africans,
    rajas,

    khans,
    west,
    dukes,
    india,
    royals,
    rome    
};

int de_dlc_row[] = { 25, 45, 65, 85, 105, 125 };
int hd_dlc_row[] = { 30, 65, 100 };

int de_dlc_boxrow[] = { -96, -76, -56, -36, -16, 4 };
int hd_dlc_boxrow[] = { -85, -51, -16 };

int de_dlc_id[] = { IDC_CHECKBOX_ROYALS, IDC_CHECKBOX_ROME, IDC_CHECKBOX_INDIA, IDC_CHECKBOX_DUKES, IDC_CHECKBOX_WEST, IDC_CHECKBOX_KHANS };
int hd_dlc_id[] = { IDC_CHECKBOX_RAJAS, IDC_CHECKBOX_AFRICANS, IDC_CHECKBOX_FORGOTTEN };

HWND de_dlc_checkbox[] = { checkbox_royals, checkbox_rome, checkbox_india, checkbox_dukes, checkbox_west, checkbox_khans };
HWND hd_dlc_checkbox[] = { checkbox_rajas, checkbox_africans, checkbox_forgotten };

std::wstring de_dlc_name[] = { L"The Mountain Royals", L"Return of Rome", L"Dynasties of India", L"Dawn of the Dukes", L"Lords of the West", L"The Last Khans" };
std::wstring hd_dlc_name[] = { L"Rise of the Rajas", L"African Kingdoms", L"The Forgotten" };

std::wstring de_dlc_tooltip[] = { L"Toggles The Mountain Royals civilisations\n(Armenians, Georgians)\nHotkey: A",
                                    L"Toggles Return of Rome civilisation\n(Romans)\nHotkey: S",
                                    L"Toggles Dynasties of India civilisations\n(Bengalis, Dravidians, Gurjaras)\nHotkey: D",
                                    L"Toggles Dawn of the Dukes civilisations\n(Bohemians, Poles)\nHotkey: F",
                                    L"Toggles Lords of the West civilisations\n(Burgundians, Sicilians)\nHotkey: G",
                                    L"Toggles The Last Khans civilisations\n(Bulgarians, Cumans, Lithuanians, Tatars)\nHotkey: H" };
std::wstring hd_dlc_tooltip[] = { L"Toggles Rise of Rajas civilisations\n(Burmese, Khmer, Malay, Vietnamese)\nHotkey: A",
                                   L"Toggles African Kingdoms civilisations\n(Berbers, Ethiopians, Malians, Portuguese)\nHotkey: S" ,
                                   L"Toggles The Forgotten civilisations\n(Incas, Hindustanis, Italians, Magyars, Slavs)\nHotkey: D" };

int de_dlc_tipid[] = { TOOLTIP_ROYALS, TOOLTIP_ROME, TOOLTIP_INDIA, TOOLTIP_DUKES, TOOLTIP_WEST, TOOLTIP_KHANS };
int hd_dlc_tipid[] = { TOOLTIP_RAJAS, TOOLTIP_AFRICANS, TOOLTIP_FORGOTTEN };

dlc de_dlc[] = { royals, rome, india, dukes, west, khans };
dlc hd_dlc[] = { rajas, africans, forgotten };

const int de_dlc_amount = 6;
const int hd_dlc_amount = 3;

dlc old_dlc[] = { aok, aoc, forgotten, africans, rajas };
dlc every_dlc[] = { aoc, forgotten, africans, rajas, khans, west, dukes, india, royals, rome };
int every_dlc_id[] = { IDC_CHECKBOX_AOC, IDC_CHECKBOX_FORGOTTEN, IDC_CHECKBOX_AFRICANS, IDC_CHECKBOX_RAJAS,
                        IDC_CHECKBOX_KHANS, IDC_CHECKBOX_WEST, IDC_CHECKBOX_DUKES, IDC_CHECKBOX_INDIA, IDC_CHECKBOX_ROYALS,
                    IDC_CHECKBOX_ROME };

edition every_edition[] = { AOK, HD, DE };
int every_edition_id[] = { IDC_RADIO_AOK, IDC_RADIO_HD, IDC_RADIO_DE };

HWND checkbox_armenians, checkbox_aztecs, checkbox_bengalis, checkbox_berbers, checkbox_bohemians,
checkbox_britons, checkbox_bulgarians, checkbox_burgundians, checkbox_burmese, checkbox_byzantines,
checkbox_celts, checkbox_chinese, checkbox_cumans, checkbox_dravidians, checkbox_ethiopians,
checkbox_franks, checkbox_georgians, checkbox_goths, checkbox_gurjaras, checkbox_huns, checkbox_incas,
checkbox_hindustanis, checkbox_italians, checkbox_japanese, checkbox_khmer, checkbox_koreans,
checkbox_lithuanians, checkbox_magyars, checkbox_malay, checkbox_malians, checkbox_mayans,
checkbox_mongols, checkbox_persians, checkbox_poles, checkbox_portuguese, checkbox_romans,
checkbox_saracens, checkbox_sicilians, checkbox_slavs, checkbox_spanish, checkbox_tatars,
checkbox_teutons, checkbox_turks, checkbox_vietnamese, checkbox_vikings;

HWND placeholder_hwnd;
Civ random(L"Random", false, AOK, aok, placeholder_hwnd);

Civ civ[MAX_CIVS] = 
{Civ(L"Armenians", true, DE, royals, checkbox_armenians), 
Civ(L"Aztecs", true, AOK, aoc, checkbox_aztecs), 
Civ(L"Bengalis", true, DE, india, checkbox_bengalis), 
Civ(L"Berbers", true, HD, africans, checkbox_berbers), 
Civ(L"Bohemians", true, DE, dukes, checkbox_bohemians), 
Civ(L"Britons", true, AOK, aok, checkbox_britons), 
Civ(L"Bulgarians", true, DE, khans, checkbox_bulgarians), 
Civ(L"Burgundians", true, DE, west, checkbox_burgundians), 
Civ(L"Burmese", true, HD, rajas, checkbox_burmese), 
Civ(L"Byzantines", true, AOK, aok, checkbox_byzantines), 
Civ(L"Celts", true, AOK, aok, checkbox_celts), 
Civ(L"Chinese", true, AOK, aok, checkbox_chinese), 
Civ(L"Cumans", true, DE, khans, checkbox_cumans), 
Civ(L"Dravidians", true, DE, india, checkbox_dravidians), 
Civ(L"Ethiopians", true, HD, africans, checkbox_ethiopians), 
Civ(L"Franks", true, AOK, aok, checkbox_franks), 
Civ(L"Georgians", true, DE, royals, checkbox_georgians), 
Civ(L"Goths", true, AOK, aok, checkbox_goths), 
Civ(L"Gurjaras", true, DE, india, checkbox_gurjaras), 
Civ(L"Hindustanis", true, HD, forgotten, checkbox_hindustanis), 
Civ(L"Huns", true, AOK, aoc, checkbox_huns), 
Civ(L"Incas", true, HD, forgotten, checkbox_incas), 
Civ(L"Italians", true, HD, forgotten, checkbox_italians), 
Civ(L"Japanese", true, AOK, aok, checkbox_japanese), 
Civ(L"Khmer", true, HD, rajas, checkbox_khmer), 
Civ(L"Koreans", true, AOK, aoc, checkbox_koreans), 
Civ(L"Lithuanians", true, DE, khans, checkbox_lithuanians), 
Civ(L"Magyars", true, HD, forgotten, checkbox_magyars), 
Civ(L"Malay", true, HD, rajas, checkbox_malay), 
Civ(L"Malians", true, HD, africans, checkbox_malians), 
Civ(L"Mayans", true, AOK, aoc, checkbox_mayans), 
Civ(L"Mongols", true, AOK, aok, checkbox_mongols), 
Civ(L"Persians", true, AOK, aok, checkbox_persians), 
Civ(L"Poles", true, DE, dukes, checkbox_poles), 
Civ(L"Portuguese", true, HD, africans, checkbox_portuguese), 
Civ(L"Romans", true, DE, rome, checkbox_romans), 
Civ(L"Saracens", true, AOK, aoc, checkbox_saracens), 
Civ(L"Sicilians", true, DE, west, checkbox_sicilians), 
Civ(L"Slavs", true, HD, forgotten, checkbox_slavs), 
Civ(L"Spanish", true, AOK, aoc, checkbox_spanish), 
Civ(L"Tatars", true, DE, khans, checkbox_tatars), 
Civ(L"Teutons", true, AOK, aok, checkbox_teutons), 
Civ(L"Turks", true, AOK, aok, checkbox_turks), 
Civ(L"Vietnamese", true, HD, rajas, checkbox_vietnamese), 
Civ(L"Vikings", true, AOK, aok, checkbox_vikings)};

edition edition_state = DE;

bool custom_civ_pool = false;

Civ& GetCiv(const std::wstring &name);

std::wstring drawn_civs[MAX_CIVS] = { L"" };

bool accessor_out_of_bounds = false;
int times_drawn[MAX_CIVS] = { 0 };
HFONT font_underline = NULL;
HWND tab;
std::vector<std::wstring> civs;

enum savetype { automatic, manual, quick };

void CreateBoldFont();
void CreateTabs(HWND);
void ShowTabComponents(int, HWND);

void ResetProgram(bool);

void DrawCiv();

void UndrawCiv();
void RedrawCiv();

void SaveSettings();
void LoadSettings();

void SaveLog(savetype);
void LoadLog(HWND, savetype);

void EnableHotkeys(HWND);
void DisableHotkeys(HWND);

void CreateUnderlineFont();

void InitialiseCivs();
void InitialiseCivStates();
void InitialiseCustomPoolCheckboxes(HWND);
void AddCiv(const std::wstring &);
void RemoveCiv(const std::wstring &);

void ShowAllPoolCheckboxes();
void HideCustomPoolCheckboxes();
void ShowHDPoolCheckboxes();
void ShowAOCPoolCheckboxes();
void DLCToggles(edition);
void OpenSurvapp();
void OpenTechTree();
void OpenOptions(HWND);
void OpenHotkeys(HWND);
void OpenAbout(HWND);

void CreateCheckboxes(HWND);
void CreateButtons(HWND);
void CreateLabels(HWND);
void CreateTextfields(HWND);
void CreateRadiobuttons(HWND);
void PositionComponents(LPARAM);

void SubclassButtons();
void AddTooltips();
void CreateTooltips(HWND);

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
void UpdateRemainingLog(bool, bool);
void ToggleRemainLog();
void ClearDrawnLog();

bool IsDlcEmpty(dlc);
void ToggleDlc(dlc, HWND);
void EnableDlc(dlc, HWND);
void DisableDlc(dlc, HWND);
void ValidateDlcToggle(HWND, dlc);
void ValidateAllDlcToggles(HWND);
void ToggleAutoToggle(HWND);
void ToggleAutoReset(HWND);

void AddTooltip(HWND, HWND, LPCWSTR);

void ActivateTooltip(HWND, TOOLINFO *, POINT);
void DeactivateTooltips(TTTOOLINFOW);

HWND hwndTooltip[24];
int hwnd_length = sizeof(hwndTooltip) / sizeof(hwndTooltip[0]);

void UpdateTooltipText(HWND hwndTool, HWND hwndTip, LPCWSTR newText);

LPCWSTR StringConjurer(LPVOID);
LPCWSTR StringCleaner(const std::wstring&);

HWND lastTooltipControl = NULL;
bool tooltipActive = false;

void SubclassButton(HWND);

int GetWindowWidth(HWND);
int GetWindowHeight(HWND);

void JoinLobby(HWND);
bool IsValidLobbyCode(const std::wstring &);

std::string ConvertToString(const std::wstring &);
HWND CreateCheckbox(HWND hWnd, HINSTANCE hInstance, int x, int y, int width, int height, int id, LPCWSTR text);

void GenerateFilePaths();

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    AboutDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    OptionsDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    HotkeysDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    JoinLobbyDlgProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    HyperlinkProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    ButtonProc(HWND, UINT, WPARAM, LPARAM);
