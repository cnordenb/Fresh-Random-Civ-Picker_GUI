#pragma once

#include "CivDataLimits.h"

constexpr const wchar_t* VERSION = L"1.4.2";

constexpr int MAX_DRAWNLOG_LINECOUNT = 256;
constexpr int MAX_LOADSTRING = 100;
constexpr int HOTKEY_AMOUNT = 42;
constexpr int SOUND_AMOUNT = 5;

constexpr int HOTKEY_ID_TAB = 1;
constexpr int HOTKEY_ID_SPACE = 2;
constexpr int HOTKEY_ID_RETURN = 3;
constexpr int HOTKEY_ID_ESC = 4;
constexpr int HOTKEY_ID_Z = 5;
constexpr int HOTKEY_ID_X = 6;
constexpr int HOTKEY_ID_C = 7;
constexpr int HOTKEY_ID_V = 8;
constexpr int HOTKEY_ID_T = 9;
constexpr int HOTKEY_ID_1 = 10;
constexpr int HOTKEY_ID_2 = 11;
constexpr int HOTKEY_ID_3 = 12;
constexpr int HOTKEY_ID_Q = 13;
constexpr int HOTKEY_ID_W = 14;
constexpr int HOTKEY_ID_E = 15;
constexpr int HOTKEY_ID_A = 16;
constexpr int HOTKEY_ID_S = 17;
constexpr int HOTKEY_ID_D = 18;
constexpr int HOTKEY_ID_F = 19;
constexpr int HOTKEY_ID_G = 20;
constexpr int HOTKEY_ID_H = 21;
constexpr int HOTKEY_ID_R = 22;
constexpr int HOTKEY_ID_F1 = 23;
constexpr int HOTKEY_ID_B = 24;
constexpr int HOTKEY_ID_F2 = 25;
constexpr int HOTKEY_ID_F3 = 26;
constexpr int HOTKEY_ID_F4 = 27;
constexpr int HOTKEY_ID_F5 = 28;
constexpr int HOTKEY_ID_F9 = 30;
constexpr int HOTKEY_ID_CTRLS = 31;
constexpr int HOTKEY_ID_CTRLR = 32;
constexpr int HOTKEY_ID_CTRLF = 33;
constexpr int HOTKEY_ID_CTRLZ = 34;
constexpr int HOTKEY_ID_CTRLX = 35;
constexpr int HOTKEY_ID_CTRLT = 36;
constexpr int HOTKEY_ID_J = 37;
constexpr int HOTKEY_ID_K = 38;
constexpr int HOTKEY_ID_L = 39;
constexpr int HOTKEY_ID_P = 40;
constexpr int HOTKEY_ID_U = 41;

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
