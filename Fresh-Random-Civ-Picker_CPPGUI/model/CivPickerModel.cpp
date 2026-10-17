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
    // Special Case object (CC ch. 7) for "no civ drawn yet": it belongs to no edition or DLC, and like the civs
    // of the original game it has a legacy jingle.
    const Civ RANDOM_CIV = { L"Random", false, CivCatalog::NONE, CivCatalog::NONE, true };
    // What fills the places past the last civ.
    const Civ NO_CIV = { L"", false, CivCatalog::NONE, CivCatalog::NONE, false };

    const int LOG_SECTION_AMOUNT = 4;
    // A civ state and a draw for every civ, the edition, and continuous freshness's history of half the civs.
    const int LOG_LINE_LIMIT = MAX_CIVS + MAX_CIVS + 1 + MAX_CIVS / 2;
    const int INVALID_INDEX = -1;

    // Special Case object (CC ch. 7) that lets the model run without a view, e.g. in unit tests.
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

    // The most DLC toggles an edition has, which is how many DLC slots on the Civ Pool tab have a use.
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
    // The catalog is loaded before the model is made (wWinMain), and it holds 1 to MAX_CIVS civs.
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

// An edition has its own civs and those of every older edition.
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

// The index of the civ of that name, or CivCatalog::NONE for any other name ("Random" too).
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
    std::ifstream file(history_folder_path + civ_name + L".txt", std::ios::binary);
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

// What a share link carries: the edition, the civ pool, and this round's draws in draw order. drawn_civs holds
// the round's draws and, after an undraw, the ones that can be redrawn; a civ counts while it is drawn (no longer
// available) and still in the pool, as the web version shares only draws that are in the pool.
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

// Whether loading a share link would replace anything: another edition than the first, a civ left out, or a draw.
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

// Draws until a civ outside the recent-civ window turns up. Each attempt removes one candidate, so the
// fallback for fewer than two candidates ends the loop before the explicit MAX_CIVS bound (P10 rule 2).
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

// The DLCs of older editions that come with this one (data.json's olderDlcs), which auto-toggle switches on.
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

// A share link's edition and civ pool, then a new round with the link's draws in their order. The round that was
// going on ends, as a reset ends it.
void CivPickerModel::LoadSharedPool(const SharedPool& shared)
{
    if (!c_assert(shared.edition >= 0 && shared.edition < catalog.EditionCount())) return;
    if (!c_assert(shared.draw_count >= 0 && shared.draw_count <= civ_count)) return;
    ResetProgram(true);
    // The link says which civs are on, so its edition comes without switching older civs on.
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

// The history of recent draws, oldest first, keeps as many civs as can ever sit out of this pool: half of it. The
// setting decides how many of the newest ones do (IsContfreshCiv), so a new setting holds from the next draw on.
void CivPickerModel::UpdateContfresh(const std::wstring& drawn_civ)
{
    if (!c_assert(!drawn_civ.empty())) return;
    const int capacity = FreshnessMaximum();
    int length = ContfreshHistoryLength();
    if (length >= capacity)
    {
        // The oldest civs make room, so the newest capacity - 1 stay.
        const int dropped = length - capacity + 1;
        for (int i = 0; i + dropped < length && i < MAX_CIVS; i++)
            contfresh_array[i].swap(contfresh_array[i + dropped]);
        for (int i = length - dropped; i < length && i < MAX_CIVS; i++) contfresh_array[i].clear();
        length -= dropped;
    }
    if (!c_assert(length >= 0 && length < MAX_CIVS)) return;
    contfresh_array[length] = drawn_civ;
}

// A civ may be drawn unless it is among the newest civs of the history, as many as the setting lets sit out.
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

// The most civs that can sit out: half the pool, and at least one.
int CivPickerModel::FreshnessMaximum() const { return std::max(custom_max_civs / 2, 1); }

// How many civs sit out: the setting, but no more than half the pool.
int CivPickerModel::FreshnessLimit() const
{
    const int maximum = FreshnessMaximum();
    if (!c_assert(settings.contfresh_civs >= 1)) return 1;
    return std::min(static_cast<int>(settings.contfresh_civs), maximum);
}

// The Options slider's choice: a number of civs, where its top (half the pool) means the most.
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
    std::wofstream out_file(path);
    if (!out_file) return false;

    out_file << L"CivStates:" << std::endl;
    for (int i = 0; i < civ_count && i < MAX_CIVS; i++)
        out_file << civ[i].name << L" " << (civ[i].enabled ? L"1" : L"0") << std::endl;

    out_file << L"DrawnCivs:" << std::endl;
    for (int i = 0; i < MAX_CIVS && drawn_civs[i] != L""; i++) out_file << drawn_civs[i] << std::endl;

    // The edition is saved by its ID in data.json.
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
    // A failed automatic save is skipped silently, as it always has been; the next save tries again.
    (void)WriteLogFile(log_file_path);
}

bool CivPickerModel::LoadLogFile(const std::wstring& path, SaveType type)
{
    std::wifstream in_file(path);
    if (!in_file) return false;

    if (type == SaveType::automatic) InitialiseCivs();
    ParseLog(in_file);
    in_file.close();

    ValidateRemainCount();
    observer->OnLogLoaded(type);
    if (iterator > 0) reset_state = false;
    return true;
}

// Reads at most LOG_LINE_LIMIT entry lines plus the section headers (P10 rule 2).
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
            // The saved history takes the place of the one the replayed draws built.
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
