#pragma once
#include <windows.h>

class History {
public:
    static INT_PTR CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static void Show(HWND hParent);
};
