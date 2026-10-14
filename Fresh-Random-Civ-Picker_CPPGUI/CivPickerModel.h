#pragma once

#include <cstdint>
#include <iosfwd>
#include <string>
#include <unordered_map>

#include "CivCatalog.h"
#include "GlobalConstants.h"
#include "ShareLink.h"

// MODEL of the Model-View-Controller design (HFDP ch. 12).
// Holds all data, state and application logic of the civ picker: the civ pool, the draw sequence,
// the settings and the save files. It knows nothing about the view or the controller and contains
// no Win32 code; every state change is reported to its observer (HFDP ch. 2), and the view pulls
// the state it displays through the const queries below. The editions, DLCs and civs come from its
// catalog (data.json); an edition or a DLC is its number there, a civ its index (its ID).

enum class LastAction { draw, reset, pool_change };

enum class SaveType { automatic, manual, quick };

struct Civ
{
    std::wstring name;
    bool enabled;
    int edition;
    int dlc;
    bool legacy;
};

struct Settings
{
    bool persistent_logging = true;
    bool draw_on_startup = false;
    bool icons_enabled = true;
    bool jingles_enabled = false;
    bool civ_labels_enabled = true;
    bool iteration_label_enabled = true;
    bool legacy_jingle_enabled = false;
    bool ui_sounds_enabled = false;
    bool tooltips_enabled = true;
    bool autoreset_enabled = true;
    bool autotoggle_enabled = true;
    bool remainlog_enabled = true;
    bool contfresh_enabled = true;
    uint8_t contfresh_strength = 1;
};

// Observer interface (HFDP ch. 2) through which the model announces its state changes.
// The notifications happen at the exact points of each operation where the display has to follow.
class CivPickerObserver
{
public:
    virtual void OnPoolChanged(int civ_index) = 0;
    virtual void OnBulkPoolChanged() = 0;
    virtual void OnDlcPoolChanged() = 0;
    virtual void OnEditionChanged() = 0;
    virtual void OnResetStarted(bool auto_reset) = 0;
    virtual void OnDrawSequenceBroken() = 0;
    virtual void OnPoolReset(bool auto_reset) = 0;
    virtual void OnDrawStarted() = 0;
    virtual void OnEmptyPoolDraw() = 0;
    virtual void OnCivDrawn() = 0;
    virtual void OnUndrawAccepted() = 0;
    virtual void OnCivUndrawn() = 0;
    virtual void OnCivRedrawn() = 0;
    virtual void OnActionRejected() = 0;
    virtual void OnLogLoaded(SaveType type) = 0;
    virtual void OnModelError(const std::wstring& message, bool fatal) = 0;

protected:
    ~CivPickerObserver() = default;
};

class CivPickerModel
{
public:
    explicit CivPickerModel(const CivCatalog& civ_catalog);

    const CivCatalog& Catalog() const;
    bool BelongsToEdition(const Civ& civ, int edition) const;

    void SetObserver(CivPickerObserver& new_observer);
    void SetFilePaths(const std::wstring& log_path, const std::wstring& history_folder);

    bool IsStartingUp() const;
    void SetStartingUp(bool starting_up);
    const Settings& GetSettings() const;
    Settings& EditSettings();

    const Civ& GetCiv(int civ_index) const;
    const Civ& GetCiv(const std::wstring& civ_name) const;
    uint8_t GetCivIndex(const std::wstring& civ_name) const;
    int FindCivIndex(const std::wstring& civ_name) const;
    const std::wstring& GetPoolEntry(int civ_index) const;
    const std::wstring& GetCurrentCiv() const;
    uint8_t GetIterator() const;
    uint8_t GetRemaining() const;
    uint8_t GetCustomMaxCivs() const;
    int GetEdition() const;
    LastAction GetLastAction() const;
    bool IsResetState() const;
    bool IsBatchToggle() const;
    bool IsCacheLoading() const;
    bool IsCacheAvailable() const;
    bool IsDlcEmpty(int dlc) const;
    bool HasBeenDrawn(const std::wstring& civ_name) const;
    std::wstring FetchHistory(const std::wstring& civ_name) const;
    SharedPool GetSharedPool() const;
    bool HasCustomState() const;

    void SetCacheLoading(bool loading);
    void SetCacheAvailable(bool available);

    void ResetProgram(bool auto_reset);
    void DrawCiv(bool rigged, const std::wstring& civ_name);
    void UndrawCiv();
    void RedrawCiv();

    void AddCiv(const std::wstring& civ_name);
    void RemoveCiv(const std::wstring& civ_name);
    void EnableAll();
    void DisableAll();
    void SetDlcEnabled(int dlc, bool enabled);
    void SetEdition(int edition);
    void EnforceEditionPool();
    void LoadSharedPool(const SharedPool& shared);
    void SetAutoToggle(bool enabled);

    void DiscardRedoHistory();
    bool WriteLogFile(const std::wstring& path) const;
    void SaveAutomaticLog();
    bool LoadLogFile(const std::wstring& path, SaveType type);

private:
    enum class LogSection { none, civ_states, drawn_civs, edition, contfresh };

    struct LogReader
    {
        LogSection section = LogSection::none;
        bool states_read = false;
        bool drawn_read = false;
        bool contfresh_read = false;
        int section_line = 0;
    };

    Civ& FindCiv(const std::wstring& civ_name);
    uint8_t GetRandomInt(uint8_t max) const;
    void ValidateRemainCount();
    void InitialiseCivs();
    void RebuildCustomPool();

    bool DrawRiggedCiv(const std::wstring& civ_name);
    bool DrawRandomCiv();
    bool DrawFreshCiv();
    void RecordDrawnCiv();

    void EnableDlc(int dlc);
    void DisableDlc(int dlc);
    void SetOldDlcsEnabled(int edition, bool enabled);
    void RemoveCivsOutsideEdition(int edition);

    void UpdateContfresh(const std::wstring& drawn_civ);
    bool IsContfreshCiv(const std::wstring& candidate_civ) const;
    void SetContfreshStrength();

    void ParseLog(std::wistream& in_file);
    bool StartLogSection(const std::wstring& line, LogReader& reader) const;
    bool ApplyLogLine(const std::wstring& line, LogReader& reader);
    void ApplyCivStateLine(const std::wstring& line);
    void ApplyEditionLine(const std::wstring& line);

    const CivCatalog& catalog;
    int civ_count;
    Civ civ[MAX_CIVS];
    Civ random;
    std::unordered_map<std::wstring, int> civ_name_to_index;
    CivPickerObserver* observer;
    Settings settings;

    std::wstring civ_list[MAX_CIVS];
    bool available_civs[MAX_CIVS];
    std::wstring drawn_civs[MAX_CIVS];
    std::wstring contfresh_array[MAX_CIVS];
    std::wstring contfresh_civ;
    std::wstring current_civ;
    std::wstring log_file_path;
    std::wstring history_folder_path;

    uint8_t iterator;
    uint8_t remaining;
    uint8_t custom_max_civs;
    uint8_t contfresh_max;
    uint8_t contfresh_count;
    int edition_state;
    LastAction last_action;

    bool startup;
    bool reset_state;
    bool redrawable;
    bool undrawable;
    bool cache_loading;
    bool cache_available;
    bool batch_toggle;
    bool custom_civ_pool;
};
