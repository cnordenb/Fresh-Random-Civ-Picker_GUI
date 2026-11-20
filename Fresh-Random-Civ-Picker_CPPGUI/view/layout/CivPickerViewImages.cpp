#include "CivPickerView.h"

#include <ShellScalingApi.h>
#include <cwchar>

#include "Assertions.h"

namespace
{
    const wchar_t* const EDITION_LOOK_SUFFIXES[EDITION_LOOKS] = { L"", L"_hover", L"_pressed" };

    ImageFile PngFile(const std::wstring& name)
    {
        ImageFile file = {};
        if (!c_assert(!name.empty())) return file;
        (void)_snwprintf_s(file.path, ImageFile::MAX_PATH_LENGTH, _TRUNCATE, L"%ls.png", name.c_str());
        return file;
    }
}

ImageSources CivPickerView::CollectImageSources()
{
    const CivCatalog& catalog = model.Catalog();
    int civ_icon_count = 0;
    for (int civ = 0; civ < catalog.CivCount() && civ < MAX_CIVS; civ++)
        civ_icon_files[civ_icon_count++] = PngFile(L"civ\\" + catalog.CivAt(civ).file_name);
    civ_icon_files[civ_icon_count++] = PngFile(L"civ\\random");
    int dlc_icon_count = 0;
    for (int edition = 0; edition < catalog.EditionCount() && edition < MAX_EDITIONS; edition++)
    {
        const EditionInfo& info = catalog.EditionAt(edition);
        for (const wchar_t* suffix : EDITION_LOOK_SUFFIXES)
            dlc_icon_files[dlc_icon_count++] = PngFile(L"dlc\\" + info.icon + suffix);
        edition_banner_files[edition] = PngFile(L"dlc\\" + info.banner);
    }
    for (int dlc = 0; dlc < catalog.DlcCount() && dlc < MAX_DLCS; dlc++)
    {
        const DlcInfo& info = catalog.DlcAt(dlc);
        if (info.icon_index != CivCatalog::NONE) dlc_icon_files[dlc_icon_count++] = PngFile(L"dlc\\" + info.icon);
    }
    return { image_folder.c_str(), { civ_icon_files, civ_icon_count }, { dlc_icon_files, dlc_icon_count },
             { edition_banner_files, catalog.EditionCount() } };
}

void CivPickerView::ReportMissingImages() const
{
    const int missing = renderer.MissingImageCount();
    if (missing == 0) return;
    const std::wstring first = renderer.FirstMissingImage();
    const std::wstring message =
        missing == 1 ? L"The image " + first + L" could not be loaded from " + image_folder +
                           L".\nThe program works without it."
                     : std::to_wstring(missing) + L" images could not be loaded from " + image_folder +
                           L", among them " + first + L".\nThe program works without them.";
    (void)MessageBox(NULL, message.c_str(), L"Error", MB_OK | MB_ICONERROR);
}

void CivPickerView::UpdateCursorSize()
{
    if (!c_assert(main_window != NULL)) return;
    const HMONITOR monitor = MonitorFromWindow(main_window, MONITOR_DEFAULTTONEAREST);
    if (monitor == NULL || monitor == cursor_monitor) return;
    cursor_monitor = monitor;
    const DPI_AWARENESS_CONTEXT unaware = SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    UINT dpi_x = USER_DEFAULT_SCREEN_DPI;
    UINT dpi_y = USER_DEFAULT_SCREEN_DPI;
    const HRESULT result = GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &dpi_x, &dpi_y);
    if (unaware != NULL) (void)SetThreadDpiAwarenessContext(unaware);
    const bool large = SUCCEEDED(result) && dpi_x >= LARGE_CURSOR_DPI;
    renderer.SelectCursorSize(large ? CursorSize::large : CursorSize::standard);
}
