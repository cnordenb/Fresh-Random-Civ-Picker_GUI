#pragma once

// Places on the main window's Civ Pool tab that more than one part of the view lays out against
// (CivPickerViewLayout.cpp, CivPickerViewDlcToggles.cpp), in the 534 x 511 client area.
constexpr int CIV_GRID_TOP = 210; // the civ grid's first row
constexpr int DLC_LIST_BOTTOM = CIV_GRID_TOP - 6; // the DLC toggles end 6 px above the civ grid
