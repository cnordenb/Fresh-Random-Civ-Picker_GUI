#pragma once
#define IDC_STATIC_TEXT 1001
#define IDC_HYPERLINK 1002

#include "GlobalConstants.h"
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
#include <unordered_map>
#include <codecvt>
#include <locale>





#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")



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
Hotkey(MOD_CONTROL, 0x58, HOTKEY_ID_CTRLX), Hotkey(MOD_CONTROL, 0x54, HOTKEY_ID_CTRLT), Hotkey(0, 0x4a, HOTKEY_ID_J)};

class Civ
{
public:
	std::wstring name;
	bool enabled;
	enum edition edition;
	enum dlc dlc;
    bool legacy;
	HWND checkbox;
	HBITMAP icon;

	Civ(std::wstring name, bool enabled, enum edition edition, enum dlc dlc, bool legacy, HWND checkbox, HBITMAP icon) : name(name), enabled(enabled), edition(edition), dlc(dlc), legacy(legacy), checkbox(checkbox), icon(icon)  {}

    void SetEnabled(bool enabled) { this->enabled = enabled; }
};


std::unordered_map<std::wstring, int> civ_name_to_index;
std::wstring civ_list[MAX_CIVS];
bool available_civs[MAX_CIVS];

std::wstring civ_list_copy[MAX_CIVS];
bool available_civs_copy[MAX_CIVS];

wchar_t INI_FILE_PATH[MAX_PATH];
wchar_t LOG_FILE_PATH[MAX_PATH];
wchar_t HIS_FILE_PATH[MAX_PATH];

std::map<HWND, WNDPROC> originalButtonProcs;

PAINTSTRUCT paint_struct;
HDC device_handling_context;
RECT rectangle_struct;
HINSTANCE instance;
WCHAR title[MAX_LOADSTRING];
WCHAR window_class[MAX_LOADSTRING];


HWND label_corner, label_centre, label_drawncount, label_remainingcount, label_enabledcount;

HWND button_draw, button_reset, button_enableall, button_disableall, button_history,
button_clearlog, button_techtree, button_options, button_survapp;

HWND drawn_log, remaining_log;

HWND hSlider;
HWND hComboBox;

HWND civ_icon, edition_icon;

HWND checkbox_showremainlog;

HWND hOptionsDlg = NULL;

struct SoundResource {
    HRSRC hResInfo;
    HGLOBAL hRes;
    LPVOID lpRes;
    LPCWSTR name;
};

SoundResource soundResources[SOUND_AMOUNT] = {
    {NULL, NULL, NULL, L"button_sound"},
    {NULL, NULL, NULL, L"tab_sound"},
    {NULL, NULL, NULL, L"hover_sound"},
    {NULL, NULL, NULL, L"error_sound"},
    {NULL, NULL, NULL, L"view_sound"}
};


SoundResource jingleResource[MAX_CIVS];
SoundResource legacyJingle[LEGACY_JINGLE_AMOUNT];

SoundResource random_jingle;
SoundResource random_legacy_jingle;

HWND checkbox_khans, checkbox_dukes, checkbox_west, checkbox_india, checkbox_rome, checkbox_royals, checkbox_kingdoms,
checkbox_forgotten, checkbox_africans, checkbox_rajas,
checkbox_aoc;

HWND khans_icon, dukes_icon, west_icon, india_icon, rome_icon, royals_icon, kingdoms_icon,
forgotten_icon, africans_icon, rajas_icon,
aoc_icon;


HWND checkbox_autoreset, checkbox_autotoggle;

HWND radiobutton_de, radiobutton_hd, radiobutton_aok;

HFONT font_bold;

HBRUSH brush_white;
HBRUSH brush_black;

HBITMAP icon_de, icon_hd, icon_aok;

HBITMAP icon_khans, icon_dukes, icon_west, icon_india, icon_rome, icon_royals, icon_kingdoms,
icon_forgotten, icon_africans, icon_rajas,
icon_aoc;

HBITMAP icon_survapp, icon_techtree, icon_options, icon_history;


uint8_t iterator = 0;
uint8_t remaining = MAX_CIVS;
uint16_t drawnlog_length = GetWindowTextLength(drawn_log);
uint16_t remaininglog_length = GetWindowTextLength(remaining_log);
uint8_t current_tab = 0;
uint8_t custom_max_civs = MAX_CIVS;
std::wstring label_text = std::to_wstring(iterator + 1) + L"/" + std::to_wstring(MAX_CIVS);
std::wstring log_entry;
std::wstring drawnlog_text, remaininglog_text;
std::wstring hlabel_default;
std::wstring current_civ = L"Random";
std::wstring enabledcount = std::to_wstring(custom_max_civs);


bool startup = true;
bool persistent_logging = true;
bool draw_on_startup = false;
bool icons_enabled = true;
bool jingles_enabled = false;
bool civ_labels_enabled = true;
bool iteration_label_enabled = true;
bool legacy_jingle_enabled = false;
bool ui_sounds_enabled = false;
bool tooltips_enabled = true;
bool autoreset_enabled = true;
bool autotoggle_enabled = true;
bool remainlog_enabled = true;
bool reset_state = true;
bool hotkey_pressed = false;
bool redrawable = false;
bool undrawable = false;
bool cache_loading = false;
bool cache_available = false;
bool batch_toggle = false;
bool contfresh_enabled = true;

std::wstring contfresh_civ = L"";
uint8_t contfresh_max = (custom_max_civs / 2) - 1;
uint8_t contfresh_strength = 1;

enum sound_type
{
	button,
	tabsound,
    hover,
    view,
	error
};

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
    rome,
    kingdoms
};

enum last_action_type
{
    draw,
    reset,
    pool_change    
};

last_action_type last_action;

int de_dlc_row[] = { 25, 45, 65, 85, 105, 125, 145 };
int hd_dlc_row[] = { 30, 65, 100 };

int de_dlc_boxrow[] = { -96, -76, -56, -36, -16, 4, 24 };
int hd_dlc_boxrow[] = { -85, -51, -16 };

int de_dlc_id[] = { IDC_CHECKBOX_KINGDOMS, IDC_CHECKBOX_ROYALS, IDC_CHECKBOX_ROME, IDC_CHECKBOX_INDIA, IDC_CHECKBOX_DUKES, IDC_CHECKBOX_WEST, IDC_CHECKBOX_KHANS };
int hd_dlc_id[] = { IDC_CHECKBOX_RAJAS, IDC_CHECKBOX_AFRICANS, IDC_CHECKBOX_FORGOTTEN };

HWND de_dlc_checkbox[] = { checkbox_kingdoms, checkbox_royals, checkbox_rome, checkbox_india, checkbox_dukes, checkbox_west, checkbox_khans };
HWND hd_dlc_checkbox[] = { checkbox_rajas, checkbox_africans, checkbox_forgotten };

std::wstring de_dlc_name[] = { L"The Three Kingdoms", L"The Mountain Royals", L"Return of Rome", L"Dynasties of India", L"Dawn of the Dukes", L"Lords of the West", L"The Last Khans"};
std::wstring hd_dlc_name[] = { L"Rise of the Rajas", L"African Kingdoms", L"The Forgotten" };

std::wstring de_dlc_tooltip[] = { L"Toggles The Three Kingdoms civilisations\n(Jurchens, Khitans, Shu, Wei, Wu)\nHotkey: A",
                                    L"Toggles The Mountain Royals civilisations\n(Armenians, Georgians)\nHotkey: S",
                                    L"Toggles Return of Rome civilisation\n(Romans)\nHotkey: D",
                                    L"Toggles Dynasties of India civilisations\n(Bengalis, Dravidians, Gurjaras)\nHotkey: F",
                                    L"Toggles Dawn of the Dukes civilisations\n(Bohemians, Poles)\nHotkey: G",
                                    L"Toggles Lords of the West civilisations\n(Burgundians, Sicilians)\nHotkey: H",
                                    L"Toggles The Last Khans civilisations\n(Bulgarians, Cumans, Lithuanians, Tatars)\nHotkey: J" };
std::wstring hd_dlc_tooltip[] = { L"Toggles Rise of Rajas civilisations\n(Burmese, Khmer, Malay, Vietnamese)\nHotkey: A",
                                   L"Toggles African Kingdoms civilisations\n(Berbers, Ethiopians, Malians, Portuguese)\nHotkey: S" ,
                                   L"Toggles The Forgotten civilisations\n(Incas, Hindustanis, Italians, Magyars, Slavs)\nHotkey: D" };

int de_dlc_tipid[] = { TOOLTIP_KINGDOMS, TOOLTIP_ROYALS, TOOLTIP_ROME, TOOLTIP_INDIA, TOOLTIP_DUKES, TOOLTIP_WEST, TOOLTIP_KHANS };
int hd_dlc_tipid[] = { TOOLTIP_RAJAS, TOOLTIP_AFRICANS, TOOLTIP_FORGOTTEN };

dlc de_dlc[] = { kingdoms, royals, rome, india, dukes, west, khans };
dlc hd_dlc[] = { rajas, africans, forgotten };

std::wstring de_dlc_bmpstring[] = { L"kingdoms.bmp", L"royals.bmp", L"rome.bmp", L"india.bmp", L"dukes.bmp", L"west.bmp", L"khans.bmp"};
std::wstring hd_dlc_bmpstring[] = { L"rajas.bmp", L"african.bmp", L"forgotten.bmp" };

HBITMAP de_dlc_bmp[] = { icon_kingdoms, icon_royals, icon_rome, icon_india, icon_dukes, icon_west, icon_khans};
HBITMAP hd_dlc_bmp[] = { icon_rajas, icon_africans, icon_forgotten };

HWND de_dlc_icon[] = { kingdoms_icon, royals_icon, rome_icon, india_icon, dukes_icon, west_icon, khans_icon };
HWND hd_dlc_icon[] = { rajas_icon, africans_icon, forgotten_icon };

const uint8_t de_dlc_amount = 7;
const uint8_t hd_dlc_amount = 3;

dlc old_dlc[] = { aok, aoc, forgotten, africans, rajas };
dlc every_dlc[] = { aoc, forgotten, africans, rajas, khans, west, dukes, india, royals, rome, kingdoms };
int every_dlc_id[] = { IDC_CHECKBOX_AOC, IDC_CHECKBOX_FORGOTTEN, IDC_CHECKBOX_AFRICANS, IDC_CHECKBOX_RAJAS,
                        IDC_CHECKBOX_KHANS, IDC_CHECKBOX_WEST, IDC_CHECKBOX_DUKES, IDC_CHECKBOX_INDIA, IDC_CHECKBOX_ROYALS,
                    IDC_CHECKBOX_ROME, IDC_CHECKBOX_KINGDOMS };

edition every_edition[] = { AOK, HD, DE };
int every_edition_id[] = { IDC_RADIO_AOK, IDC_RADIO_HD, IDC_RADIO_DE };

HWND checkbox_armenians, checkbox_aztecs, checkbox_bengalis, checkbox_berbers, checkbox_bohemians,
checkbox_britons, checkbox_bulgarians, checkbox_burgundians, checkbox_burmese, checkbox_byzantines,
checkbox_celts, checkbox_chinese, checkbox_cumans, checkbox_dravidians, checkbox_ethiopians,
checkbox_franks, checkbox_georgians, checkbox_goths, checkbox_gurjaras, checkbox_huns, checkbox_incas,
checkbox_hindustanis, checkbox_italians, checkbox_japanese, checkbox_jurchens, checkbox_khitans, checkbox_khmer, checkbox_koreans,
checkbox_lithuanians, checkbox_magyars, checkbox_malay, checkbox_malians, checkbox_mayans,
checkbox_mongols, checkbox_persians, checkbox_poles, checkbox_portuguese, checkbox_romans,
checkbox_saracens, checkbox_shu, checkbox_sicilians, checkbox_slavs, checkbox_spanish, checkbox_tatars,
checkbox_teutons, checkbox_turks, checkbox_vietnamese, checkbox_vikings, checkbox_wei, checkbox_wu;

HBITMAP icon_armenians, icon_aztecs, icon_bengalis, icon_berber, icon_bohemians,
icon_britons, icon_bulgarians, icon_burgundians, icon_burmese, icon_byzantines,
icon_celts, icon_chinese, icon_cumans, icon_dravidians, icon_ethiopians,
icon_franks, icon_georgians, icon_goths, icon_gurjaras, icon_huns, icon_incas,
icon_hindustanis, icon_italians, icon_japanese, icon_jurchens, icon_khitans, icon_khmer, icon_koreans,
icon_lithuanians, icon_magyars, icon_malay, icon_malians, icon_mayans,
icon_mongols, icon_persians, icon_poles, icon_portuguese, icon_romans,
icon_saracens, icon_shu, icon_sicilians, icon_slavs, icon_spanish, icon_tatars,
icon_teutons, icon_turks, icon_vietnamese, icon_vikings, icon_wei, icon_wu, icon_random;

HWND placeholder_hwnd;
Civ random(L"Random", false, AOK, aok, true, placeholder_hwnd, icon_random);

Civ civ[MAX_CIVS] = 
{Civ(L"Armenians", true, DE, royals, false, checkbox_armenians, icon_armenians), 
Civ(L"Aztecs", true, AOK, aoc, true, checkbox_aztecs, icon_aztecs), 
Civ(L"Bengalis", true, DE, india, false, checkbox_bengalis, icon_bengalis), 
Civ(L"Berbers", true, HD, africans, false, checkbox_berbers, icon_berber), 
Civ(L"Bohemians", true, DE, dukes, false, checkbox_bohemians, icon_bohemians), 
Civ(L"Britons", true, AOK, aok, true, checkbox_britons, icon_britons), 
Civ(L"Bulgarians", true, DE, khans, false, checkbox_bulgarians, icon_bulgarians), 
Civ(L"Burgundians", true, DE, west, false, checkbox_burgundians, icon_burgundians), 
Civ(L"Burmese", true, HD, rajas, false, checkbox_burmese, icon_burmese), 
Civ(L"Byzantines", true, AOK, aok, true, checkbox_byzantines, icon_byzantines), 
Civ(L"Celts", true, AOK, aok, true, checkbox_celts, icon_celts), 
Civ(L"Chinese", true, AOK, aok, true, checkbox_chinese, icon_chinese), 
Civ(L"Cumans", true, DE, khans, false, checkbox_cumans, icon_cumans), 
Civ(L"Dravidians", true, DE, india, false, checkbox_dravidians, icon_dravidians), 
Civ(L"Ethiopians", true, HD, africans, false, checkbox_ethiopians, icon_ethiopians), 
Civ(L"Franks", true, AOK, aok, true, checkbox_franks, icon_franks), 
Civ(L"Georgians", true, DE, royals, false, checkbox_georgians, icon_georgians), 
Civ(L"Goths", true, AOK, aok, true, checkbox_goths, icon_goths), 
Civ(L"Gurjaras", true, DE, india, false, checkbox_gurjaras, icon_gurjaras), 
Civ(L"Hindustanis", true, HD, forgotten, false, checkbox_hindustanis, icon_hindustanis), 
Civ(L"Huns", true, AOK, aoc, true, checkbox_huns, icon_huns), 
Civ(L"Inca", true, HD, forgotten, false, checkbox_incas, icon_incas), 
Civ(L"Italians", true, HD, forgotten, false, checkbox_italians, icon_italians), 
Civ(L"Japanese", true, AOK, aok, true, checkbox_japanese, icon_japanese),
Civ(L"Jurchens", true, DE, kingdoms, false, checkbox_jurchens, icon_jurchens),
Civ(L"Khitans", true, DE, kingdoms, false, checkbox_khitans, icon_khitans),
Civ(L"Khmer", true, HD, rajas, false, checkbox_khmer, icon_khmer), 
Civ(L"Koreans", true, AOK, aoc, true, checkbox_koreans, icon_koreans), 
Civ(L"Lithuanians", true, DE, khans, false, checkbox_lithuanians, icon_lithuanians), 
Civ(L"Magyars", true, HD, forgotten, false, checkbox_magyars, icon_magyars), 
Civ(L"Malay", true, HD, rajas, false, checkbox_malay, icon_malay), 
Civ(L"Malians", true, HD, africans, false, checkbox_malians, icon_malians), 
Civ(L"Maya", true, AOK, aoc, true, checkbox_mayans, icon_mayans), 
Civ(L"Mongols", true, AOK, aok, true, checkbox_mongols, icon_mongols), 
Civ(L"Persians", true, AOK, aok, true, checkbox_persians, icon_persians), 
Civ(L"Poles", true, DE, dukes, false, checkbox_poles, icon_poles), 
Civ(L"Portuguese", true, HD, africans, false, checkbox_portuguese, icon_portuguese), 
Civ(L"Romans", true, DE, rome, false, checkbox_romans, icon_romans), 
Civ(L"Saracens", true, AOK, aok, true, checkbox_saracens, icon_saracens),
Civ(L"Shu", true, DE, kingdoms, false, checkbox_shu, icon_shu),
Civ(L"Sicilians", true, DE, west, false, checkbox_sicilians, icon_sicilians), 
Civ(L"Slavs", true, HD, forgotten, false, checkbox_slavs, icon_slavs), 
Civ(L"Spanish", true, AOK, aoc, true, checkbox_spanish, icon_spanish), 
Civ(L"Tatars", true, DE, khans, false, checkbox_tatars, icon_tatars), 
Civ(L"Teutons", true, AOK, aok, true, checkbox_teutons, icon_teutons), 
Civ(L"Turks", true, AOK, aok, true, checkbox_turks, icon_turks), 
Civ(L"Vietnamese", true, HD, rajas, false, checkbox_vietnamese, icon_vietnamese), 
Civ(L"Vikings", true, AOK, aok, true, checkbox_vikings, icon_vikings),
Civ(L"Wei", true, DE, kingdoms, false, checkbox_wei, icon_wei),
Civ(L"Wu", true, DE, kingdoms, false, checkbox_wu, icon_wu)};

edition edition_state = DE;

bool custom_civ_pool = false;

Civ& GetCiv(const std::wstring &name);

std::wstring drawn_civs[MAX_CIVS] = { L"" };
std::wstring contfresh_array[MAX_CIVS] = { L"" };
uint8_t contfresh_count = 0;

bool accessor_out_of_bounds = false;
uint8_t times_drawn[MAX_CIVS] = { 0 };
HFONT font_underline = NULL;
HWND tab;

uint16_t drawn_log_linecount = 0;

enum savetype { automatic, manual, quick };

uint8_t GetRandomInt(uint8_t);
uint8_t GetCivIndex(const std::wstring&);


void ValidateRemainCount();
void CreateBoldFont();
HBITMAP FetchCivIcon(const std::wstring &);
void CreateTabs(HWND);
void ShowTabComponents(int, HWND);

void ResetProgram(bool);

void DrawCiv(bool, const std::wstring &);

void Undo(HWND);

void UndrawCiv();
void RedrawCiv();

void SaveSettings();
void LoadSettings();

void SaveLog(savetype);
void LoadLog(HWND, savetype);

void EnableHotkeys(HWND);
void DisableHotkeys(HWND);

void CreateUnderlineFont();
void LoadImages();

BOOL PlayResource(const SoundResource &);

bool LoadSound(SoundResource &);
void LoadSounds();
void LoadJingles();
void UnloadSound(HGLOBAL);
void UnloadSounds();
void UnloadJingles();

void StopSound();
void PlayAudio(sound_type);
void PlayJingle(const std::wstring &);
bool IsLegacyCiv(const std::wstring &);
bool IsContfreshCiv(const std::wstring&);
void SetContfreshStrength();

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
void CreateImages(HWND);
void CreateButtons(HWND);
void CreateLabels(HWND);
void CreateTextfields(HWND);
void CreateRadiobuttons(HWND);
void PositionComponents(LPARAM);

void SubclassButtons();
void AddTooltips();
void CreateTooltips(HWND);

void SetEditionState(HWND hWnd, edition edition);

void EnableAll(HWND, bool);
void DisableAll(HWND, bool);

void ShowDrawTab(bool, HWND);
void ShowLogTab(bool);
void ShowCustomTab(bool);
void ShowDEDLCCheckboxes(bool);
void ShowHDDLCCheckboxes(bool);
void ShowAOCCheckbox(bool);
int GetDlcCheckboxId(dlc);
HWND GetCivCheckbox(const std::wstring &);

void UpdateEnabledCivsCounter();
void UpdateDrawnLog(bool, bool, bool);
void UpdateRemainingLog(bool);
void ToggleRemainLog();
void ClearDrawnLog();
void UpdateContfresh(const std::wstring &);
void CheckDrawnLogLength();
void AddDrawnLogEntry(std::wstring);
void RemovePreviousDrawnLogEntry();


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

HWND hwndTooltip[TOOLTIP_AMOUNT];
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

bool FileExists(const std::wstring &filePath);

std::string ConvertToString(const std::wstring &);
HWND CreateCheckbox(HWND hWnd, HINSTANCE hInstance, int x, int y, int width, int height, int id, LPCWSTR text);

void GenerateFilePaths();
void OpenHistory(HWND);
std::wstring NormaliseLineEndings(const std::wstring& input);
std::wstring FetchHistory(const std::wstring& civ_name);

void UpdateTooltips(bool);
void UpdateCivAddedInfo(HWND, dlc);
void UpdateCivPoolInfo(HWND, const std::wstring &);
void HidePoolInfo(HWND);

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    AboutDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    OptionsDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    HotkeysDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    JoinLobbyDlgProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    HyperlinkProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    ButtonProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);