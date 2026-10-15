#pragma once

#include "CivDataLimits.h"

#define VERSION L"1.4.2"

#define MAX_DRAWNLOG_LINECOUNT 256
#define MAX_LOADSTRING 100
#define HOTKEY_AMOUNT 42
#define SOUND_AMOUNT 5

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
#define HOTKEY_ID_J 37
#define HOTKEY_ID_K 38
#define HOTKEY_ID_L 39
#define HOTKEY_ID_P 40
#define HOTKEY_ID_U 41


#define DT_UNDERLINE 0x80000000
#define MAX_LENGTH 15 

#define MIN_WIDTH 550
#define MIN_HEIGHT 570

#define BUTTON_WIDTH 100
#define BUTTON_HEIGHT 30

// The Civ Pool tab's hotkeys that pick an edition, by the edition's place in data.json, and that toggle a DLC, by
// its toggle's place in the edition's list; with the keys the program shows for them. Toggles after the first
// DLC_HOTKEY_COUNT of an edition have no hotkey.
constexpr int DLC_HOTKEY_COUNT = 9;
constexpr int EDITION_HOTKEY_IDS[MAX_EDITIONS] = { HOTKEY_ID_Q, HOTKEY_ID_W, HOTKEY_ID_E };
constexpr const wchar_t* EDITION_HOTKEY_KEYS[MAX_EDITIONS] = { L"Q", L"W", L"E" };
constexpr int DLC_SLOT_HOTKEY_IDS[DLC_HOTKEY_COUNT] = { HOTKEY_ID_A, HOTKEY_ID_S, HOTKEY_ID_D, HOTKEY_ID_F, HOTKEY_ID_G,
                                                        HOTKEY_ID_H, HOTKEY_ID_J, HOTKEY_ID_K, HOTKEY_ID_L };
constexpr const wchar_t* DLC_SLOT_HOTKEY_KEYS[DLC_HOTKEY_COUNT] = { L"A", L"S", L"D", L"F", L"G",
                                                                    L"H", L"J", L"K", L"L" };
