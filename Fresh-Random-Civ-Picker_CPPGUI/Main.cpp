#include "CivCatalog.h"
#include "CivPickerController.h"
#include "CivPickerModel.h"

// Entry point. As in HFDP ch. 12, main creates the model and the controller; the controller creates the view.
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine,
                      _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Static storage keeps the program's state off the heap and the stack (P10 rule 3). The catalog of editions,
    // DLCs and civs comes first, as everything else is made from it.
    static CivCatalog catalog;
    if (!CivPickerController::LoadCatalog(catalog)) return FALSE;
    static CivPickerModel model(catalog);
    static CivPickerController controller(model);
    return controller.Run(hInstance, nCmdShow);
}
