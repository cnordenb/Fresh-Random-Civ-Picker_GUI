#include "GlobalConstants.h"
#include "History.h"
#include "Resource.h"
#include <string>

void History::Show(HWND hParent) {
    DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_HISTORY_DIALOG), hParent, History::DialogProc);
}

INT_PTR CALLBACK History::DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    HWND combo_box;
    std::wstring test[MAX_CIVS];
    for (int i = 0; i < MAX_CIVS; i++) {
        test[i] = L"Civilization " + std::to_wstring(i);
    }
    LPARAM converted_test[MAX_CIVS];
	for (int i = 0; i < MAX_CIVS; i++) {
		converted_test[i] = reinterpret_cast<LPARAM>(test[i].c_str());
	}
    switch (message) {
    case WM_INITDIALOG:
		combo_box = GetDlgItem(hDlg, IDC_HISTORY_COMBO);
        for (int i = 0; i < MAX_CIVS; i++) SendMessage(combo_box, CB_ADDSTRING, 0, (LPARAM)converted_test[i]);
        return (INT_PTR)TRUE;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}