#pragma once

#include "CivDataLimits.h"

constexpr const wchar_t* VERSION = L"1.4.2";

constexpr int MAX_DRAWNLOG_LINECOUNT = 256;
constexpr int MAX_LOADSTRING = 100;
constexpr int SOUND_AMOUNT = 5;

// The Civ Pool tab's hotkeys that toggle a DLC, by its toggle's place in the edition's list (A to L by default):
// toggles after the first DLC_HOTKEY_COUNT of an edition have no hotkey (HotkeyBindings).
constexpr int DLC_HOTKEY_COUNT = 9;
