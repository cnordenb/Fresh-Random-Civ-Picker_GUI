#include "CivCatalog.h"
#include "CivPickerController.h"
#include "CivPickerModel.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine,
                      _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    static CivCatalog catalog;
    if (!CivPickerController::LoadCatalog(catalog)) return FALSE;
    static CivPickerModel model(catalog);
    static CivPickerController controller(model);
    return controller.Run(hInstance, nCmdShow);
}
