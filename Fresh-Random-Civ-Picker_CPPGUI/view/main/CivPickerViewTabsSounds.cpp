#include "CivPickerView.h"

#include <mmsystem.h>

#include "Assertions.h"

namespace
{
    int ToShowCommand(bool shown) { return shown ? SW_SHOW : SW_HIDE; }
}

int CivPickerView::GetCurrentTab() const { return current_tab; }

int CivPickerView::GetSelectedTab() const { return tab_strip.Selected(); }

void CivPickerView::SelectTab(int tab_index)
{
    if (!c_assert(tab_index >= DRAW_TAB && tab_index <= CIV_POOL_TAB)) return;
    tab_strip.Select(tab_index);
}

void CivPickerView::ShowTabComponents(int tab_index)
{
    if (!c_assert(tab_index >= DRAW_TAB && tab_index <= CIV_POOL_TAB) || !c_assert(main_window != NULL)) return;
    if (!model.IsStartingUp()) PlayAudio(SoundType::tabsound);
    current_tab = tab_index;
    tab_strip.Select(tab_index);
    DeactivateTooltips();
    if (tab_index == DRAW_TAB)
    {
        ShowLogTab(false);
        ShowCustomTab(false);
        ShowDrawTab(true);
    }
    else if (tab_index == LOG_TAB)
    {
        ShowDrawTab(false);
        ShowCustomTab(false);
        ShowLogTab(true);
    }
    else if (tab_index == CIV_POOL_TAB)
    {
        ShowDrawTab(false);
        ShowLogTab(false);
        ShowCustomTab(true);
    }
    ShowTabTooltips(tab_index);
}

void CivPickerView::ShowDrawTab(bool showing_enabled)
{
    if (!c_assert(main_window != NULL)) return;
    const Settings& settings = model.GetSettings();
    const HWND draw_tab_only[] = { button_survapp.Window(), button_techtree.Window(), button_history.Window(),
                                   button_options.Window() };
    if (showing_enabled)
    {
        ShowControl(button_draw.Window(), true);
        MoveResetButton(DRAW_TAB);
        ShowControl(button_reset.Window(), true);
        if (settings.civ_labels_enabled) ShowControl(label_centre.Window(), true);
        if (settings.icons_enabled) ShowControl(civ_icon.Window(), true);
        if (settings.iteration_label_enabled) ShowControl(label_corner.Window(), true);
        for (HWND control : draw_tab_only) ShowControl(control, true);
        return;
    }

    if (current_tab == CIV_POOL_TAB)
    {
        ShowControl(civ_icon.Window(), false);
        ShowControl(button_draw.Window(), false);
        ShowControl(label_centre.Window(), false);
    }
    ShowControl(button_reset.Window(), false);
    MoveResetButton(LOG_TAB);
    ShowControl(label_corner.Window(), false);
    for (HWND control : draw_tab_only) ShowControl(control, false);
}

void CivPickerView::ShowLogTab(bool showing_enabled)
{
    if (!c_assert(main_window != NULL)) return;
    const Settings& settings = model.GetSettings();
    const HWND log_tab_only[] = { label_drawncount.Window(), button_clearlog.Window(), drawn_log.Window(),
                                  label_remainingcount.Window(), checkbox_showremainlog.Window(),
                                  button_share.Window() };
    if (showing_enabled)
    {
        ShowControl(button_draw.Window(), true);
        ShowControl(button_reset.Window(), true);
        if (settings.civ_labels_enabled) ShowControl(label_centre.Window(), true);
        if (settings.icons_enabled) ShowControl(civ_icon.Window(), true);
        for (HWND control : log_tab_only) ShowControl(control, true);
        if (settings.remainlog_enabled) ShowControl(remaining_log.Window(), true);
        return;
    }

    if (current_tab == CIV_POOL_TAB)
    {
        ShowControl(button_draw.Window(), false);
        ShowControl(label_centre.Window(), false);
        ShowControl(civ_icon.Window(), false);
    }
    ShowControl(button_reset.Window(), false);
    for (HWND control : log_tab_only) ShowControl(control, false);
    ShowControl(remaining_log.Window(), false);
}

void CivPickerView::ShowCustomTab(bool showing_enabled)
{
    if (!c_assert(main_window != NULL)) return;
    ShowControl(edition_icon.Window(), showing_enabled);
    ShowControl(label_enabledcount.Window(), showing_enabled);
    const HWND pool_controls[] = { button_enableall.Window(), button_disableall.Window(), checkbox_autoreset.Window(),
                                   checkbox_autotoggle.Window(), button_poolshare.Window() };
    for (HWND control : pool_controls) ShowControl(control, showing_enabled);
    const int edition_count = model.Catalog().EditionCount();
    for (int i = 0; i < edition_count && i < MAX_EDITIONS; i++) ShowControl(edition_rows[i].Window(), showing_enabled);
    if (!showing_enabled)
    {
        HideCustomPoolCheckboxes();
        return;
    }
    ShowPoolCheckboxes(model.GetEdition());
    ShowDlcToggles(model.GetEdition());
}

void CivPickerView::ShowPoolCheckboxes(int edition)
{
    if (!c_assert(edition >= 0 && edition < model.Catalog().EditionCount())) return;
    const int civ_count = model.Catalog().CivCount();
    for (int i = 0; i < civ_count && i < MAX_CIVS; i++)
        ShowControl(civ_toggles[i].Window(), model.BelongsToEdition(model.GetCiv(i), edition));
}

void CivPickerView::HideCustomPoolCheckboxes()
{
    const int civ_count = model.Catalog().CivCount();
    for (int i = 0; i < civ_count && i < MAX_CIVS; i++) ShowControl(civ_toggles[i].Window(), false);
    const int edition_count = model.Catalog().EditionCount();
    for (int edition = 0; edition < edition_count && edition < MAX_EDITIONS; edition++) HideDlcToggles(edition);
}

void CivPickerView::ShowEditionControls(int edition)
{
    if (!c_assert(edition >= 0 && edition < model.Catalog().EditionCount())) return;
    if (current_tab == CIV_POOL_TAB)
    {
        ShowDlcToggles(edition);
        ShowPoolCheckboxes(edition);
    }
    edition_icon.ShowPicture({ ImageId::edition_banners, { 0, edition * EDITION_BANNER_HEIGHT, EDITION_BANNER_WIDTH,
                                                           (edition + 1) * EDITION_BANNER_HEIGHT } });
    CheckEditionRow(edition);
}

void CivPickerView::CheckEditionRow(int edition)
{
    const int edition_count = model.Catalog().EditionCount();
    if (!c_assert(edition >= 0 && edition < edition_count)) return;
    for (int i = 0; i < edition_count && i < MAX_EDITIONS; i++) edition_rows[i].SetChecked(i == edition);
}

void CivPickerView::ShowControl(HWND control, bool shown)
{
    if (!c_assert(control != NULL)) return;
    (void)ShowWindow(control, ToShowCommand(shown));
}

bool CivPickerView::IsCivChecked(int civ_index) const
{
    if (!c_assert(civ_index >= 0 && civ_index < model.Catalog().CivCount())) return false;
    return civ_toggles[civ_index].IsChecked();
}

bool CivPickerView::IsDlcChecked(int dlc) const
{
    if (!c_assert(HasDlcToggle(dlc))) return false;
    return dlc_toggles[dlc].IsChecked();
}

void CivPickerView::SetDlcChecked(int dlc, bool checked)
{
    if (!c_assert(dlc >= 0 && dlc < model.Catalog().DlcCount())) return;
    if (!HasDlcToggle(dlc)) return;
    dlc_toggles[dlc].SetChecked(checked);
}

bool CivPickerView::HasDlcToggle(int dlc) const
{
    if (dlc < 0 || dlc >= model.Catalog().DlcCount()) return false;
    return model.Catalog().DlcAt(dlc).toggle_edition != CivCatalog::NONE;
}

bool CivPickerView::IsAutoToggleChecked() const { return checkbox_autotoggle.IsChecked(); }

void CivPickerView::SetAutoToggleChecked(bool checked) { checkbox_autotoggle.SetChecked(checked); }

bool CivPickerView::IsAutoResetChecked() const { return checkbox_autoreset.IsChecked(); }

void CivPickerView::SetAutoResetChecked(bool checked) { checkbox_autoreset.SetChecked(checked); }

void CivPickerView::ValidateDlcToggle(int dlc) { SetDlcChecked(dlc, !model.IsDlcEmpty(dlc)); }

void CivPickerView::ValidateAllDlcToggles()
{
    const int dlc_count = model.Catalog().DlcCount();
    for (int i = 0; i < dlc_count && i < MAX_DLCS; i++)
    {
        if (HasDlcToggle(i)) ValidateDlcToggle(i);
    }
}

void CivPickerView::ShowRemainingLog(bool shown) { ShowControl(remaining_log.Window(), shown); }

void CivPickerView::ToggleRemainLogCheckbox()
{
    checkbox_showremainlog.SetChecked(!checkbox_showremainlog.IsChecked());
}

void CivPickerView::ClearDrawnLog()
{
    if (!c_assert(drawn_log.Window() != NULL)) return;
    if (!model.IsStartingUp()) drawn_log_linecount = 0;
    SetControlText(drawn_log.Window(), L"");
}

void CivPickerView::ClearLogTexts()
{
    if (!c_assert(drawn_log.Window() != NULL && remaining_log.Window() != NULL)) return;
    SetControlText(remaining_log.Window(), L"");
    SetControlText(drawn_log.Window(), L"");
}

void CivPickerView::RefreshCivIcon()
{
    ShowControl(civ_icon.Window(), model.GetSettings().icons_enabled && current_tab != CIV_POOL_TAB);
}

void CivPickerView::RefreshCivLabel()
{
    if (!model.GetSettings().civ_labels_enabled) ShowControl(label_centre.Window(), false);
    else if (current_tab != CIV_POOL_TAB) ShowControl(label_centre.Window(), true);
}

void CivPickerView::RefreshCornerLabel()
{
    if (!model.GetSettings().iteration_label_enabled) ShowControl(label_corner.Window(), false);
    else if (current_tab == DRAW_TAB) ShowControl(label_corner.Window(), true);
}

void CivPickerView::DeactivateTooltips()
{
    tooltip.Hide();
    shown_tooltip = -1;
}

void CivPickerView::LoadSounds()
{
    for (int i = 0; i < SOUND_AMOUNT; ++i) LoadSound(sound_resources[i]);
    LoadJingles();
}

void CivPickerView::LoadJingles()
{
    if (!c_assert(!jingle_folder.empty())) return;
    const CivCatalog& catalog = model.Catalog();
    int missing = 0;
    for (int i = 0; i < catalog.CivCount() && i < MAX_CIVS; i++)
    {
        const CivInfo& civ = catalog.CivAt(i);
        jingles[i] = MapJingle(civ.file_name + L".wav");
        if (jingles[i].data == nullptr) missing++;
        if (!civ.legacy_jingle) continue;
        legacy_jingles[i] = MapJingle(L"legacy\\" + civ.file_name + L".wav");
        if (legacy_jingles[i].data == nullptr) missing++;
    }
    random_jingle = MapJingle(L"random.wav");
    random_legacy_jingle = MapJingle(L"legacy\\random.wav");
    if (random_jingle.data == nullptr) missing++;
    if (random_legacy_jingle.data == nullptr) missing++;
    if (missing == 0) return;
    const std::wstring message = std::to_wstring(missing) + L" civ jingles could not be loaded from " + jingle_folder +
                                 L"\nThe program works without them.";
    MessageBox(NULL, message.c_str(), L"Error", MB_OK | MB_ICONERROR);
}

MappedJingle CivPickerView::MapJingle(const std::wstring& file_name) const
{
    if (!c_assert(!file_name.empty())) return { nullptr };
    const std::wstring path = jingle_folder + file_name;
    const HANDLE file = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE) return { nullptr };
    const HANDLE mapping = CreateFileMappingW(file, nullptr, PAGE_READONLY, 0, 0, nullptr);
    (void)CloseHandle(file);
    if (mapping == NULL) return { nullptr };
    const void* view = MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, 0);
    (void)CloseHandle(mapping);
    return { view };
}

void CivPickerView::LoadSound(SoundResource& sound_resource)
{
    if (!c_assert(instance != NULL) || !c_assert(sound_resource.name != nullptr)) return;
    sound_resource.info = FindResource(instance, sound_resource.name, L"WAVE");
    if (sound_resource.info == NULL)
    {
        MessageBox(NULL, (std::wstring(L"Failed to find resource: ") + sound_resource.name).c_str(), L"Error",
                   MB_OK | MB_ICONERROR);
        return;
    }
    sound_resource.handle = LoadResource(instance, sound_resource.info);
    if (sound_resource.handle == NULL)
    {
        MessageBox(NULL, L"Failed to load resource.", L"Error", MB_OK | MB_ICONERROR);
        return;
    }
    sound_resource.data = LockResource(sound_resource.handle);
    if (sound_resource.data == NULL) MessageBox(NULL, L"Failed to lock resource.", L"Error", MB_OK | MB_ICONERROR);
}

void CivPickerView::UnloadSounds()
{
    for (int i = 0; i < SOUND_AMOUNT; ++i) UnloadSound(sound_resources[i]);
    StopSound();
    for (MappedJingle& jingle : jingles)
    {
        if (jingle.data != nullptr) (void)UnmapViewOfFile(jingle.data);
        jingle.data = nullptr;
    }
    for (MappedJingle& jingle : legacy_jingles)
    {
        if (jingle.data != nullptr) (void)UnmapViewOfFile(jingle.data);
        jingle.data = nullptr;
    }
    if (random_jingle.data != nullptr) (void)UnmapViewOfFile(random_jingle.data);
    if (random_legacy_jingle.data != nullptr) (void)UnmapViewOfFile(random_legacy_jingle.data);
    random_jingle.data = nullptr;
    random_legacy_jingle.data = nullptr;
}

void CivPickerView::UnloadSound(SoundResource& sound_resource)
{
    if (sound_resource.data != nullptr)
    {
        UnlockResource(sound_resource.data);
        sound_resource.data = nullptr;
    }
    if (sound_resource.handle != NULL)
    {
        FreeResource(sound_resource.handle);
        sound_resource.handle = NULL;
    }
}

void CivPickerView::PlayResource(const SoundResource& sound_resource) const { PlayWave(sound_resource.data); }

void CivPickerView::PlayWave(const void* wave) const
{
    if (!c_assert(wave != nullptr)) return;
    if (!sndPlaySound(static_cast<LPCWSTR>(wave), SND_MEMORY | SND_ASYNC | SND_NODEFAULT))
    {
        MessageBox(NULL, L"Failed to play sound.", L"Error", MB_OK | MB_ICONERROR);
    }
}

void CivPickerView::PlayAudio(SoundType type) const
{
    if (!model.GetSettings().ui_sounds_enabled) return;
    switch (type)
    {
        case SoundType::button: PlayResource(sound_resources[0]); return;
        case SoundType::tabsound: PlayResource(sound_resources[1]); return;
        case SoundType::hover: PlayResource(sound_resources[2]); return;
        case SoundType::error: PlayResource(sound_resources[3]); return;
        case SoundType::view: PlayResource(sound_resources[4]); return;
        default: (void)c_assert(false); return;
    }
}

void CivPickerView::PlayJingle(const std::wstring& civ_name) const
{
    if (!c_assert(!civ_name.empty())) return;
    const Settings& settings = model.GetSettings();
    if (!settings.jingles_enabled) return;

    if (civ_name == L"Random")
    {
        PlayWave(settings.legacy_jingle_enabled ? random_legacy_jingle.data : random_jingle.data);
        return;
    }
    const int civ = model.FindCivIndex(civ_name);
    if (civ == CivCatalog::NONE) return;
    const bool legacy = settings.legacy_jingle_enabled && model.GetCiv(civ).legacy;
    PlayWave(legacy ? legacy_jingles[civ].data : jingles[civ].data);
}

void CivPickerView::StopSound() const { PlaySound(NULL, NULL, 0); }
