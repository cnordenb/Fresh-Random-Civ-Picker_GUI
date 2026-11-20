#pragma once

#include <string>

#include "CivDataLimits.h"

class DataReader;

// Part of the MODEL: the editions, DLCs and civs. data.json is the only place they are written down (the web
// version reads the same file); the catalog reads it once at startup and checks it, so the rest of the program can
// rely on every reference in it. Plain C++, so it is unit-tested with the real file.
// Editions are numbered in data.json's order, and the first is the default; DLCs in the order of data.json's "dlcs"
// object; civs by their permanent IDs, which run from 0 without gaps.

struct DlcToggleLayout
{
    int left;
    int top;
    int step;
    int size;
    int gap;
};

struct EditionLink
{
    std::wstring text;
    std::wstring url;
};

struct EditionInfo
{
    std::wstring id;
    std::wstring name;
    std::wstring short_name;
    std::wstring series;
    std::wstring url; // the store page; empty for an edition that has a store note instead
    std::wstring store_note;
    std::wstring icon; // names its icon files: icon, icon_hover and icon_pressed
    std::wstring banner; // names its banner file
    int year = 0;
    bool legacy_jingles = false; // its civs also have the original game's jingles
    int dlc_toggles[MAX_DLC_TOGGLES] = {};
    int dlc_toggle_count = 0;
    DlcToggleLayout toggle_layout = {};
    int older_dlcs[MAX_DLCS] = {};
    int older_dlc_count = 0;
    EditionLink links[MAX_EDITION_LINKS];
    int link_count = 0;
};

struct DlcInfo
{
    std::wstring key;
    std::wstring name;
    std::wstring short_name;
    std::wstring released;
    std::wstring icon; // names its icon file; empty for a DLC without one
    int edition = 0;
    int icon_index = -1; // its place among the DLCs that have an icon, or -1
    int toggle_edition = -1; // the edition whose Civ Pool tab has a toggle for it, or -1
    int toggle_slot = -1; // that toggle's place in the edition's list
};

struct CivInfo
{
    std::wstring name;
    std::wstring tech_tree; // its name on aoe2techtree.net
    std::wstring file_name; // its name in lower case, which names its jingles
    int dlc = 0;
    int edition = 0;
    bool legacy_jingle = false;
};

class CivCatalog final
{
public:
    static constexpr int NONE = -1;

    bool Load(const std::string& utf8_json);
    const std::wstring& Error() const;

    int EditionCount() const;
    const EditionInfo& EditionAt(int edition) const;
    int FindEdition(const std::wstring& id) const;
    int DlcCount() const;
    const DlcInfo& DlcAt(int dlc) const;
    int CivCount() const;
    const CivInfo& CivAt(int civ) const;
    int DisplayCiv(int position) const;
    bool InEdition(int civ, int edition) const;
    int EditionCivCount(int edition) const;
    int LinkCount() const;
    const EditionLink& LinkAt(int link) const;

private:
    struct DlcList
    {
        int items[MAX_DLCS];
        int count;
    };

    bool Fail(const std::wstring& problem);
    int FindDlc(const std::wstring& key) const;
    void ReadDlcs(DataReader& reader, int root);
    void ReadDlc(DataReader& reader, int key, int dlc);
    void ReadEditions(DataReader& reader, int root);
    void ReadEdition(DataReader& reader, int object, int edition);
    void ReadEditionToggles(DataReader& reader, int object, int edition);
    void ReadEditionLinks(DataReader& reader, int object, int edition);
    DlcList ReadDlcList(DataReader& reader, int array, const std::wstring& where) const;
    DlcToggleLayout ReadToggleLayout(DataReader& reader, int object, const std::wstring& where) const;
    void ReadDlcEditions(DataReader& reader, int root);
    void ReadCivs(DataReader& reader, int root);
    void ReadCiv(DataReader& reader, int object, int position);
    bool DeriveDlcs();
    bool DeriveCivs();
    void SortDisplayOrder();

    EditionInfo editions[MAX_EDITIONS];
    DlcInfo dlcs[MAX_DLCS];
    CivInfo civs[MAX_CIVS];
    bool civ_read[MAX_CIVS] = {};
    int display_order[MAX_CIVS] = {};
    int edition_count = 0;
    int dlc_count = 0;
    int civ_count = 0;
    std::wstring error;
};
