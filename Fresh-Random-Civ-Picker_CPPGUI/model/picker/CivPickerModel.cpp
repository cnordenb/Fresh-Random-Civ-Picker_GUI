#include "CivPickerModel.h"

#include <algorithm>
#include <codecvt>
#include <fstream>
#include <locale>
#include <random>
#include <sstream>

#include "Assertions.h"

namespace
{
    const Civ RANDOM_CIV = { L"Random", false, CivCatalog::NONE, CivCatalog::NONE, true };
    const Civ NO_CIV = { L"", false, CivCatalog::NONE, CivCatalog::NONE, false };

    const int INVALID_INDEX = -1;

    class SilentObserver final : public CivPickerObserver
    {
    public:
        void OnPoolChanged(int) override;
        void OnBulkPoolChanged() override;
        void OnDlcPoolChanged() override;
        void OnEditionChanged() override;
        void OnResetStarted(bool) override;
        void OnDrawSequenceBroken() override;
        void OnPoolReset(bool) override;
        void OnDrawStarted() override;
        void OnEmptyPoolDraw() override;
        void OnCivDrawn() override;
        void OnUndrawAccepted() override;
        void OnCivUndrawn() override;
        void OnCivRedrawn() override;
        void OnActionRejected() override;
        void OnLogLoaded(SaveType) override;
        void OnModelError(const std::wstring&, bool) override;
    };

    void SilentObserver::OnPoolChanged(int) {}
    void SilentObserver::OnBulkPoolChanged() {}
    void SilentObserver::OnDlcPoolChanged() {}
    void SilentObserver::OnEditionChanged() {}
    void SilentObserver::OnResetStarted(bool) {}
    void SilentObserver::OnDrawSequenceBroken() {}
    void SilentObserver::OnPoolReset(bool) {}
    void SilentObserver::OnDrawStarted() {}
    void SilentObserver::OnEmptyPoolDraw() {}
    void SilentObserver::OnCivDrawn() {}
    void SilentObserver::OnUndrawAccepted() {}
    void SilentObserver::OnCivUndrawn() {}
    void SilentObserver::OnCivRedrawn() {}
    void SilentObserver::OnActionRejected() {}
    void SilentObserver::OnLogLoaded(SaveType) {}
    void SilentObserver::OnModelError(const std::wstring&, bool) {}

    CivPickerObserver& GetSilentObserver()
    {
        static SilentObserver silent_observer;
        return silent_observer;
    }

    int FindNthAvailable(const bool available[MAX_CIVS], int wanted_position)
    {
        int position = 0;
        for (int i = 0; i < MAX_CIVS; i++)
        {
            if (!available[i]) continue;
            if (position == wanted_position) return i;
            position++;
        }
        return INVALID_INDEX;
    }

    int MostDlcToggles(const CivCatalog& catalog)
    {
        int most = 0;
        for (int edition = 0; edition < catalog.EditionCount() && edition < MAX_EDITIONS; edition++)
            most = std::max(most, catalog.EditionAt(edition).dlc_toggle_count);
        return most;
    }
}

CivPickerModel::CivPickerModel(const CivCatalog& civ_catalog) :
    catalog(civ_catalog),
    civ_count(civ_catalog.CivCount()),
    random(RANDOM_CIV),
    observer(&GetSilentObserver()),
    hotkeys(civ_catalog.EditionCount(), MostDlcToggles(civ_catalog)),
    current_civ(L"Random"),
    iterator(0),
    remaining(static_cast<uint8_t>(civ_catalog.CivCount())),
    custom_max_civs(static_cast<uint8_t>(civ_catalog.CivCount())),
    edition_state(0),
    last_action(LastAction::draw),
    startup(true),
    reset_state(true),
    redrawable(false),
    undrawable(false),
    cache_loading(false),
    cache_available(false),
    batch_toggle(false),
    custom_civ_pool(false)
{
    (void)c_assert(civ_count > 0 && civ_count <= MAX_CIVS);
    for (int i = 0; i < MAX_CIVS; i++)
    {
        civ[i] = NO_CIV;
        if (i >= civ_count) continue;
        const CivInfo& info = catalog.CivAt(i);
        civ[i] = { info.name, true, info.edition, info.dlc, info.legacy_jingle };
        civ_name_to_index[civ[i].name] = i;
    }
}

const CivCatalog& CivPickerModel::Catalog() const { return catalog; }

bool CivPickerModel::BelongsToEdition(const Civ& candidate, int edition) const
{
    const int edition_count = catalog.EditionCount();
    if (!c_assert(edition >= 0 && edition < edition_count)) return false;
    if (candidate.edition < 0 || candidate.edition >= edition_count) return false;
    return catalog.EditionAt(candidate.edition).year <= catalog.EditionAt(edition).year;
}

void CivPickerModel::SetObserver(CivPickerObserver& new_observer) { observer = &new_observer; }

void CivPickerModel::SetFilePaths(const std::wstring& log_path, const std::wstring& history_folder)
{
    log_file_path = log_path;
    history_folder_path = history_folder;
}

bool CivPickerModel::IsStartingUp() const { return startup; }

void CivPickerModel::SetStartingUp(bool starting_up) { startup = starting_up; }

const Settings& CivPickerModel::GetSettings() const { return settings; }

Settings& CivPickerModel::EditSettings() { return settings; }

const HotkeyBindings& CivPickerModel::GetHotkeys() const { return hotkeys; }

HotkeyBindings& CivPickerModel::EditHotkeys() { return hotkeys; }

const Civ& CivPickerModel::GetCiv(int civ_index) const
{
    if (civ_index < 0 || civ_index >= civ_count) return random;
    return civ[civ_index];
}

const Civ& CivPickerModel::GetCiv(const std::wstring& civ_name) const
{
    for (int i = 0; i < civ_count && i < MAX_CIVS; i++) if (civ[i].name == civ_name) return civ[i];
    return random;
}

Civ& CivPickerModel::FindCiv(const std::wstring& civ_name)
{
    for (int i = 0; i < civ_count && i < MAX_CIVS; i++) if (civ[i].name == civ_name) return civ[i];
    return random;
}

uint8_t CivPickerModel::GetCivIndex(const std::wstring& civ_name) const
{
    auto found = civ_name_to_index.find(civ_name);
    if (found != civ_name_to_index.end()) return static_cast<uint8_t>(found->second);
    observer->OnModelError(
        L"Error: GetCivIndex (line 3090) called and civ name '" + civ_name + L"' was not found in index map.", true);
    return MAX_CIVS;
}

int CivPickerModel::FindCivIndex(const std::wstring& civ_name) const
{
    const auto found = civ_name_to_index.find(civ_name);
    return found != civ_name_to_index.end() ? found->second : CivCatalog::NONE;
}

const std::wstring& CivPickerModel::GetPoolEntry(int civ_index) const
{
    if (civ_index < 0 || civ_index >= civ_count) return random.name;
    return civ_list[civ_index];
}

const std::wstring& CivPickerModel::GetCurrentCiv() const { return current_civ; }

uint8_t CivPickerModel::GetIterator() const { return iterator; }

uint8_t CivPickerModel::GetRemaining() const { return remaining; }

uint8_t CivPickerModel::GetCustomMaxCivs() const { return custom_max_civs; }

int CivPickerModel::GetEdition() const { return edition_state; }

LastAction CivPickerModel::GetLastAction() const { return last_action; }

bool CivPickerModel::IsResetState() const { return reset_state; }

bool CivPickerModel::IsBatchToggle() const { return batch_toggle; }

bool CivPickerModel::IsCacheLoading() const { return cache_loading; }

bool CivPickerModel::IsCacheAvailable() const { return cache_available; }

bool CivPickerModel::IsDlcEmpty(int dlc) const
{
    if (!c_assert(dlc >= 0 && dlc < catalog.DlcCount())) return true;
    for (int i = 0; i < civ_count && i < MAX_CIVS; i++) if (civ[i].enabled && civ[i].dlc == dlc) return false;
    return true;
}

bool CivPickerModel::HasBeenDrawn(const std::wstring& civ_name) const
{
    for (int i = 0; i < MAX_CIVS; i++) if (drawn_civs[i] == civ_name) return true;
    return false;
}

std::wstring CivPickerModel::FetchHistory(const std::wstring& civ_name) const
{
    std::ifstream file((history_folder_path + civ_name + L".txt").c_str(), std::ios::binary);
    if (!file) return L"History not found.";

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string utf8_text = buffer.str();

    const bool has_bom = utf8_text.size() >= 3 &&
        static_cast<unsigned char>(utf8_text[0]) == 0xEF &&
        static_cast<unsigned char>(utf8_text[1]) == 0xBB &&
        static_cast<unsigned char>(utf8_text[2]) == 0xBF;
    if (has_bom) utf8_text = utf8_text.substr(3);

    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(utf8_text);
}

SharedPool CivPickerModel::GetSharedPool() const
{
    SharedPool shared = {};
    shared.edition = edition_state;
    for (int i = 0; i < civ_count && i < MAX_CIVS; i++) shared.enabled[i] = civ[i].enabled;
    bool listed[MAX_CIVS] = {};
    for (int i = 0; i < MAX_CIVS && !drawn_civs[i].empty() && shared.draw_count < iterator; i++)
    {
        const int drawn = FindCivIndex(drawn_civs[i]);
        if (drawn == CivCatalog::NONE || listed[drawn] || !civ[drawn].enabled || available_civs[drawn]) continue;
        listed[drawn] = true;
        shared.draws[shared.draw_count] = drawn;
        shared.draw_count++;
    }
    return shared;
}

bool CivPickerModel::HasCustomState() const
{
    if (!c_assert(civ_count > 0)) return false;
    return edition_state != 0 || custom_max_civs < civ_count || iterator > 0;
}

void CivPickerModel::SetCacheLoading(bool loading) { cache_loading = loading; }

void CivPickerModel::SetCacheAvailable(bool available) { cache_available = available; }

uint8_t CivPickerModel::GetRandomInt(uint8_t max) const
{
    if (max < 1)
    {
        observer->OnModelError(L"Error: GetRandomInt (line 3074) called and max is less than 1", true);
        observer->OnModelError(L"Error: max is 0", false);
        return 0;
    }
    std::random_device seed_source;
    std::mt19937 generator(seed_source());
    std::uniform_int_distribution<> distribution(0, max - 1);
    return static_cast<uint8_t>(distribution(generator));
}

void CivPickerModel::ValidateRemainCount() { remaining = static_cast<uint8_t>(custom_max_civs - iterator); }

void CivPickerModel::InitialiseCivs()
{
    for (int i = 0; i < civ_count && i < MAX_CIVS; i++)
    {
        civ_list[i] = civ[i].name;
        available_civs[i] = true;
    }
}

void CivPickerModel::RebuildCustomPool()
{
    custom_max_civs = 0;
    for (int i = 0; i < civ_count && i < MAX_CIVS; i++)
    {
        available_civs[i] = civ[i].enabled;
        civ_list[i] = civ[i].enabled ? civ[i].name : L"";
        if (civ[i].enabled) custom_max_civs++;
    }
}

void CivPickerModel::ResetProgram(bool auto_reset)
{
    if (!startup && !auto_reset && reset_state) return;

    if (settings.persistent_logging && !auto_reset && !reset_state && !startup)
    {
        SaveAutomaticLog();
        cache_available = true;
    }

    observer->OnResetStarted(auto_reset);
    if (custom_max_civs < civ_count) custom_civ_pool = true;

    current_civ = L"Random";
    iterator = 0;
    remaining = custom_max_civs;
    observer->OnDrawSequenceBroken();

    if (custom_civ_pool) RebuildCustomPool();
    else InitialiseCivs();
    for (int i = 0; i < MAX_CIVS; i++) drawn_civs[i] = L"";

    reset_state = true;
    ValidateRemainCount();
    observer->OnPoolReset(auto_reset);
    undrawable = false;
    last_action = LastAction::reset;
}

void CivPickerModel::DrawCiv(bool rigged, const std::wstring& civ_name)
{
    observer->OnDrawStarted();
    reset_state = false;

    if (custom_max_civs == 0)
    {
        observer->OnEmptyPoolDraw();
        return;
    }

    if (remaining == 0 || iterator == custom_max_civs || iterator == 0)
    {
        ResetProgram(true);
        reset_state = false;
    }

    const bool drawn = rigged ? DrawRiggedCiv(civ_name) : DrawRandomCiv();
    if (drawn) RecordDrawnCiv();
}

bool CivPickerModel::DrawRiggedCiv(const std::wstring& civ_name)
{
    current_civ = civ_name;
    const uint8_t civ_index = GetCivIndex(current_civ);
    if (civ_index >= MAX_CIVS) return false;

    civ_list[civ_index] = L"";
    available_civs[civ_index] = false;
    return true;
}

bool CivPickerModel::DrawRandomCiv()
{
    ValidateRemainCount();
    if (remaining == 0) observer->OnModelError(L"remaining == 0 at line 971.", false);
    const int position = GetRandomInt(remaining);

    if (settings.contfresh_enabled)
    {
        if (!DrawFreshCiv()) return false;
    }
    else
    {
        const int civ_index = FindNthAvailable(available_civs, position);
        if (civ_index == INVALID_INDEX)
        {
            observer->OnModelError(L"Invalid index! Given index currently: " + std::to_wstring(civ_index), false);
            return false;
        }
        current_civ = civ[civ_index].name;
    }

    const uint8_t drawn_index = GetCivIndex(current_civ);
    if (drawn_index >= MAX_CIVS) return false;
    civ_list[drawn_index] = L"";
    available_civs[drawn_index] = false;
    return true;
}

bool CivPickerModel::DrawFreshCiv()
{
    std::wstring civ_list_copy[MAX_CIVS];
    bool available_civs_copy[MAX_CIVS] = {};
    for (int i = 0; i < MAX_CIVS; i++)
    {
        civ_list_copy[i] = civ_list[i];
        available_civs_copy[i] = available_civs[i];
    }

    for (int r_delta = 0; r_delta <= MAX_CIVS; r_delta++)
    {
        if ((remaining - r_delta) < 2)
        {
            for (int i = 0; i < MAX_CIVS; i++) if (available_civs_copy[i]) current_civ = civ_list_copy[i];
            return true;
        }

        ValidateRemainCount();
        if (remaining == 0) observer->OnModelError(L"remaining == 0 at line 989.", false);
        const int position = GetRandomInt(static_cast<uint8_t>(remaining - r_delta));
        const int civ_index = FindNthAvailable(available_civs_copy, position);
        if (civ_index == INVALID_INDEX)
        {
            observer->OnModelError(L"Invalid index! Given index currently: " + std::to_wstring(civ_index), false);
            return false;
        }

        current_civ = civ[civ_index].name;
        civ_list_copy[civ_index] = L"";
        available_civs_copy[civ_index] = false;
        if (IsContfreshCiv(current_civ)) return true;
    }
    return true;
}

void CivPickerModel::RecordDrawnCiv()
{
    if (iterator >= MAX_CIVS) return;

    drawn_civs[iterator] = current_civ;
    iterator++;
    remaining--;
    ValidateRemainCount();
    observer->OnCivDrawn();

    redrawable = false;
    undrawable = true;
    last_action = LastAction::draw;
    if (settings.contfresh_enabled) UpdateContfresh(current_civ);
}
