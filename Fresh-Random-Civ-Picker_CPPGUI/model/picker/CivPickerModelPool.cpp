#include "CivPickerModel.h"

#include <algorithm>
#include <fstream>
#include <sstream>

#include "Assertions.h"

namespace
{
    const int LOG_SECTION_AMOUNT = 4;
    const int LOG_LINE_LIMIT = MAX_CIVS + MAX_CIVS + 1 + MAX_CIVS / 2;
}

void CivPickerModel::UndrawCiv()
{
    if (!undrawable)
    {
        observer->OnActionRejected();
        return;
    }
    observer->OnUndrawAccepted();
    if (iterator == 0) return;

    iterator--;
    remaining++;
    const uint8_t civ_index = GetCivIndex(current_civ);
    if (civ_index >= MAX_CIVS) return;

    civ_list[civ_index] = current_civ;
    available_civs[civ_index] = true;
    current_civ = (iterator >= 1) ? drawn_civs[iterator - 1] : L"Random";
    ValidateRemainCount();
    observer->OnCivUndrawn();
    redrawable = true;
}

void CivPickerModel::RedrawCiv()
{
    if (!redrawable || iterator >= MAX_CIVS || drawn_civs[iterator] == L"")
    {
        observer->OnActionRejected();
        return;
    }
    const uint8_t civ_index = GetCivIndex(drawn_civs[iterator]);
    if (civ_index >= MAX_CIVS || !available_civs[civ_index])
    {
        observer->OnActionRejected();
        return;
    }

    current_civ = drawn_civs[iterator];
    civ_list[civ_index] = L"";
    available_civs[civ_index] = false;
    iterator++;
    remaining--;
    ValidateRemainCount();
    observer->OnCivRedrawn();

    if (iterator == custom_max_civs) redrawable = false;
}

void CivPickerModel::AddCiv(const std::wstring& civ_name)
{
    if (GetCiv(civ_name).enabled) return;
    const uint8_t civ_index = GetCivIndex(civ_name);
    if (civ_index >= MAX_CIVS) return;

    civ_list[civ_index] = civ_name;
    available_civs[civ_index] = true;
    FindCiv(civ_name).enabled = true;
    custom_max_civs++;
    if (custom_max_civs == civ_count) custom_civ_pool = false;

    ValidateRemainCount();
    observer->OnPoolChanged(civ_index);
    last_action = LastAction::pool_change;
}

void CivPickerModel::RemoveCiv(const std::wstring& civ_name)
{
    if (!GetCiv(civ_name).enabled) return;
    const uint8_t civ_index = GetCivIndex(civ_name);
    if (civ_index >= MAX_CIVS) return;

    if (!available_civs[civ_index])
    {
        iterator--;
        remaining++;
    }

    custom_civ_pool = true;
    civ_list[civ_index] = L"";
    available_civs[civ_index] = false;
    FindCiv(civ_name).enabled = false;
    custom_max_civs--;

    ValidateRemainCount();
    observer->OnPoolChanged(civ_index);
    last_action = LastAction::pool_change;
}

void CivPickerModel::EnableAll()
{
    batch_toggle = true;
    custom_civ_pool = false;
    for (int i = 0; i < civ_count && i < MAX_CIVS; i++)
    {
        if (BelongsToEdition(civ[i], edition_state)) AddCiv(civ[i].name);
    }

    if (settings.autoreset_enabled) ResetProgram(true);
    observer->OnBulkPoolChanged();
    batch_toggle = false;
}

void CivPickerModel::DisableAll()
{
    batch_toggle = true;
    custom_civ_pool = true;
    for (int i = 0; i < civ_count && i < MAX_CIVS; i++) RemoveCiv(civ[i].name);

    if (settings.autoreset_enabled) ResetProgram(true);
    observer->OnBulkPoolChanged();
    batch_toggle = false;
}

void CivPickerModel::SetDlcEnabled(int dlc, bool enabled)
{
    if (!c_assert(dlc >= 0 && dlc < catalog.DlcCount())) return;
    batch_toggle = true;
    if (enabled) EnableDlc(dlc);
    else DisableDlc(dlc);
    observer->OnDlcPoolChanged();
    batch_toggle = false;
}

void CivPickerModel::EnableDlc(int dlc)
{
    if (!c_assert(dlc >= 0 && dlc < catalog.DlcCount())) return;
    for (int i = 0; i < civ_count && i < MAX_CIVS; i++) if (civ[i].dlc == dlc) AddCiv(civ[i].name);
}

void CivPickerModel::DisableDlc(int dlc)
{
    if (!c_assert(dlc >= 0 && dlc < catalog.DlcCount())) return;
    for (int i = 0; i < civ_count && i < MAX_CIVS; i++) if (civ[i].dlc == dlc) RemoveCiv(civ[i].name);
}

void CivPickerModel::SetOldDlcsEnabled(int edition, bool enabled)
{
    if (!c_assert(edition >= 0 && edition < catalog.EditionCount())) return;
    const EditionInfo& info = catalog.EditionAt(edition);
    for (int i = 0; i < info.older_dlc_count && i < MAX_DLCS; i++)
    {
        if (enabled) EnableDlc(info.older_dlcs[i]);
        else DisableDlc(info.older_dlcs[i]);
    }
}

void CivPickerModel::RemoveCivsOutsideEdition(int edition)
{
    if (!c_assert(edition >= 0 && edition < catalog.EditionCount())) return;
    for (int i = 0; i < civ_count && i < MAX_CIVS; i++)
    {
        if (!BelongsToEdition(civ[i], edition)) RemoveCiv(civ[i].name);
    }
}

void CivPickerModel::SetEdition(int edition)
{
    if (!c_assert(edition >= 0 && edition < catalog.EditionCount())) return;
    batch_toggle = true;
    if (settings.autotoggle_enabled && !startup) SetOldDlcsEnabled(edition, true);
    RemoveCivsOutsideEdition(edition);
    edition_state = edition;
    observer->OnEditionChanged();
    batch_toggle = false;
}

void CivPickerModel::EnforceEditionPool() { RemoveCivsOutsideEdition(edition_state); }

void CivPickerModel::LoadSharedPool(const SharedPool& shared)
{
    if (!c_assert(shared.edition >= 0 && shared.edition < catalog.EditionCount())) return;
    if (!c_assert(shared.draw_count >= 0 && shared.draw_count <= civ_count)) return;
    ResetProgram(true);
    const bool autotoggle_was_enabled = settings.autotoggle_enabled;
    settings.autotoggle_enabled = false;
    SetEdition(shared.edition);
    settings.autotoggle_enabled = autotoggle_was_enabled;
    batch_toggle = true;
    for (int i = 0; i < civ_count && i < MAX_CIVS; i++)
    {
        if (shared.enabled[i]) AddCiv(civ[i].name);
        else RemoveCiv(civ[i].name);
    }
    batch_toggle = false;
    observer->OnBulkPoolChanged();
    ResetProgram(true);
    for (int i = 0; i < shared.draw_count && i < MAX_CIVS; i++)
    {
        const int drawn = shared.draws[i];
        if (!c_assert(drawn >= 0 && drawn < civ_count && civ[drawn].enabled)) return;
        DrawCiv(true, civ[drawn].name);
    }
}

void CivPickerModel::SetAutoToggle(bool enabled)
{
    settings.autotoggle_enabled = enabled;
    SetOldDlcsEnabled(edition_state, enabled);
}

void CivPickerModel::UpdateContfresh(const std::wstring& drawn_civ)
{
    if (!c_assert(!drawn_civ.empty())) return;
    const int capacity = FreshnessMaximum();
    int length = ContfreshHistoryLength();
    if (length >= capacity)
    {
        const int dropped = length - capacity + 1;
        for (int i = 0; i + dropped < length && i < MAX_CIVS; i++)
            contfresh_array[i].swap(contfresh_array[i + dropped]);
        for (int i = length - dropped; i < length && i < MAX_CIVS; i++) contfresh_array[i].clear();
        length -= dropped;
    }
    if (!c_assert(length >= 0 && length < MAX_CIVS)) return;
    contfresh_array[length] = drawn_civ;
}

bool CivPickerModel::IsContfreshCiv(const std::wstring& candidate_civ) const
{
    if (custom_max_civs < 2) return true;
    const int length = ContfreshHistoryLength();
    const int first_sitting_out = length > FreshnessLimit() ? length - FreshnessLimit() : 0;
    for (int i = first_sitting_out; i < length && i < MAX_CIVS; i++)
    {
        if (contfresh_array[i] == candidate_civ) return false;
    }
    return true;
}

int CivPickerModel::ContfreshHistoryLength() const
{
    for (int i = 0; i < MAX_CIVS; i++)
    {
        if (contfresh_array[i].empty()) return i;
    }
    return MAX_CIVS;
}

int CivPickerModel::FreshnessMaximum() const { return std::max(custom_max_civs / 2, 1); }

int CivPickerModel::FreshnessLimit() const
{
    const int maximum = FreshnessMaximum();
    if (!c_assert(settings.contfresh_civs >= 1)) return 1;
    return std::min(static_cast<int>(settings.contfresh_civs), maximum);
}

void CivPickerModel::SetFreshness(int civs)
{
    if (!c_assert(civs >= 1 && civs <= MAX_CIVS)) return;
    settings.contfresh_civs = civs >= FreshnessMaximum() ? CONTFRESH_MOST : static_cast<uint8_t>(civs);
}

void CivPickerModel::DiscardRedoHistory()
{
    bool current_found = false;
    for (int i = 0; i < MAX_CIVS; i++)
    {
        if (drawn_civs[i] == current_civ) current_found = true;
        else if (!current_found) continue;
        if (drawn_civs[i] != current_civ) drawn_civs[i] = L"";
    }
}

bool CivPickerModel::WriteLogFile(const std::wstring& path) const
{
    std::wofstream out_file(path.c_str());
    if (!out_file) return false;

    out_file << L"CivStates:" << std::endl;
    for (int i = 0; i < civ_count && i < MAX_CIVS; i++)
        out_file << civ[i].name << L" " << (civ[i].enabled ? L"1" : L"0") << std::endl;

    out_file << L"DrawnCivs:" << std::endl;
    for (int i = 0; i < MAX_CIVS && drawn_civs[i] != L""; i++) out_file << drawn_civs[i] << std::endl;

    out_file << L"EditionState:" << std::endl;
    out_file << catalog.EditionAt(edition_state).id << std::endl;

    if (settings.contfresh_enabled)
    {
        out_file << L"ContfreshCivArray:" << std::endl;
        for (int i = 0; i < MAX_CIVS && contfresh_array[i] != L""; i++) out_file << contfresh_array[i] << std::endl;
    }
    out_file.close();
    return true;
}

void CivPickerModel::SaveAutomaticLog()
{
    DiscardRedoHistory();
    (void)WriteLogFile(log_file_path);
}

bool CivPickerModel::LoadLogFile(const std::wstring& path, SaveType type)
{
    std::wifstream in_file(path.c_str());
    if (!in_file) return false;

    if (type == SaveType::automatic) InitialiseCivs();
    ParseLog(in_file);
    in_file.close();

    ValidateRemainCount();
    observer->OnLogLoaded(type);
    if (iterator > 0) reset_state = false;
    return true;
}

void CivPickerModel::ParseLog(std::wistream& in_file)
{
    LogReader reader;
    int lines_applied = 0;
    std::wstring line;

    for (int lines_read = 0; lines_read < LOG_LINE_LIMIT + LOG_SECTION_AMOUNT; lines_read++)
    {
        if (!std::getline(in_file, line)) return;
        if (lines_applied >= LOG_LINE_LIMIT || line == L"") return;
        if (StartLogSection(line, reader)) continue;
        if (!ApplyLogLine(line, reader)) return;
        lines_applied++;
    }
}

bool CivPickerModel::StartLogSection(const std::wstring& line, LogReader& reader) const
{
    if (line == L"CivStates:" && !reader.states_read)
    {
        reader.section = LogSection::civ_states;
        reader.states_read = true;
    }
    else if (settings.contfresh_enabled && line == L"ContfreshCivArray:" && !reader.contfresh_read)
    {
        reader.section = LogSection::contfresh;
        reader.contfresh_read = true;
    }
    else if (line == L"DrawnCivs:" && !reader.drawn_read)
    {
        reader.section = LogSection::drawn_civs;
        reader.drawn_read = true;
    }
    else if (line == L"EditionState:") reader.section = LogSection::edition;
    else return false;

    reader.section_line = 0;
    return true;
}

bool CivPickerModel::ApplyLogLine(const std::wstring& line, LogReader& reader)
{
    switch (reader.section)
    {
        case LogSection::civ_states:
            if (reader.section_line >= MAX_CIVS) return false;
            ApplyCivStateLine(line);
            break;
        case LogSection::contfresh:
            if (reader.section_line >= MAX_CIVS) return false;
            if (reader.section_line == 0)
            {
                for (std::wstring& entry : contfresh_array) entry.clear();
            }
            contfresh_array[reader.section_line] = line;
            break;
        case LogSection::drawn_civs:
            if (reader.section_line >= MAX_CIVS) return false;
            DrawCiv(true, line);
            break;
        case LogSection::edition:
            ApplyEditionLine(line);
            break;
        case LogSection::none:
            break;
        default:
            (void)c_assert(false);
            break;
    }
    reader.section_line++;
    return true;
}

void CivPickerModel::ApplyCivStateLine(const std::wstring& line)
{
    std::wistringstream line_stream(line);
    std::wstring civ_name;
    int state = 0;
    if (line_stream >> civ_name >> state && state == 0) RemoveCiv(civ_name);
}

void CivPickerModel::ApplyEditionLine(const std::wstring& line)
{
    const bool autotoggle_was_enabled = settings.autotoggle_enabled;
    settings.autotoggle_enabled = false;

    const int edition = catalog.FindEdition(line);
    if (edition != CivCatalog::NONE) SetEdition(edition);

    if (autotoggle_was_enabled) settings.autotoggle_enabled = true;
}
