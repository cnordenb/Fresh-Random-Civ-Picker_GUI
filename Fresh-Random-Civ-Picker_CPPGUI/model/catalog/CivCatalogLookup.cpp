#include "CivCatalog.h"

#include <cwctype>

#include "Assertions.h"

namespace
{
    constexpr int MAX_CIV_NAME_LENGTH = 40;
    const wchar_t* const NO_CIV_NAME = L"Random";

    std::wstring LowerCase(const std::wstring& text)
    {
        std::wstring lower = text;
        for (size_t i = 0; i < lower.size() && i < static_cast<size_t>(MAX_CIV_NAME_LENGTH); i++)
            lower[i] = static_cast<wchar_t>(std::towlower(lower[i]));
        return lower;
    }

    bool IsUsableCivName(const std::wstring& name)
    {
        const size_t longest = static_cast<size_t>(MAX_CIV_NAME_LENGTH);
        if (name.empty() || name.size() > longest || name == NO_CIV_NAME) return false;
        for (size_t i = 0; i < name.size() && i < longest; i++)
        {
            if (std::iswspace(name[i]) != 0 || std::iswcntrl(name[i]) != 0) return false;
        }
        return true;
    }
}

bool CivCatalog::DeriveDlcs()
{
    int icons = 0;
    for (int i = 0; i < dlc_count && i < MAX_DLCS; i++)
    {
        if (dlcs[i].icon_index == NONE) continue;
        dlcs[i].icon_index = icons;
        icons++;
    }
    for (int edition = 0; edition < edition_count && edition < MAX_EDITIONS; edition++)
    {
        const EditionInfo& info = editions[edition];
        for (int slot = 0; slot < info.dlc_toggle_count && slot < MAX_DLC_TOGGLES; slot++)
        {
            const int dlc = info.dlc_toggles[slot];
            if (!c_assert(dlc >= 0 && dlc < dlc_count)) return Fail(L"data.json: a DLC toggle names no DLC");
            if (dlcs[dlc].toggle_edition != NONE)
                return Fail(L"data.json: DLC \"" + dlcs[dlc].key + L"\" has two toggles");
            if (dlcs[dlc].icon_index == NONE)
                return Fail(L"data.json: DLC \"" + dlcs[dlc].key + L"\" has a toggle but no \"icon\"");
            dlcs[dlc].toggle_edition = edition;
            dlcs[dlc].toggle_slot = slot;
        }
    }
    return true;
}

bool CivCatalog::DeriveCivs()
{
    for (int i = 0; i < civ_count && i < MAX_CIVS; i++)
    {
        CivInfo& civ = civs[i];
        if (!c_assert(civ.dlc >= 0 && civ.dlc < dlc_count)) return Fail(L"data.json: a civ names no DLC");
        if (!IsUsableCivName(civ.name))
        {
            return Fail(L"data.json: civ " + std::to_wstring(i) + L" needs a name of at most " +
                        std::to_wstring(MAX_CIV_NAME_LENGTH) + L" characters, without spaces, other than \"" +
                        NO_CIV_NAME + L"\"");
        }
        for (int j = 0; j < i; j++)
        {
            if (civs[j].name == civ.name) return Fail(L"data.json: two civs are named \"" + civ.name + L"\"");
        }
        civ.edition = dlcs[civ.dlc].edition;
        civ.legacy_jingle = editions[civ.edition].legacy_jingles;
        civ.file_name = LowerCase(civ.name);
    }
    return true;
}

void CivCatalog::SortDisplayOrder()
{
    if (!c_assert(civ_count >= 0 && civ_count <= MAX_CIVS)) return;
    for (int i = 0; i < civ_count && i < MAX_CIVS; i++) display_order[i] = i;
    for (int i = 1; i < civ_count && i < MAX_CIVS; i++)
    {
        const int civ = display_order[i];
        int place = i;
        for (int j = i; j > 0 && civs[display_order[j - 1]].file_name > civs[civ].file_name; j--)
        {
            display_order[j] = display_order[j - 1];
            place = j - 1;
        }
        display_order[place] = civ;
    }
}

int CivCatalog::EditionCount() const { return edition_count; }

const EditionInfo& CivCatalog::EditionAt(int edition) const
{
    if (!c_assert(edition >= 0 && edition < edition_count)) return editions[0];
    return editions[edition];
}

int CivCatalog::FindEdition(const std::wstring& id) const
{
    for (int i = 0; i < edition_count && i < MAX_EDITIONS; i++)
    {
        if (editions[i].id == id) return i;
    }
    return NONE;
}

int CivCatalog::DlcCount() const { return dlc_count; }

const DlcInfo& CivCatalog::DlcAt(int dlc) const
{
    if (!c_assert(dlc >= 0 && dlc < dlc_count)) return dlcs[0];
    return dlcs[dlc];
}

int CivCatalog::CivCount() const { return civ_count; }

const CivInfo& CivCatalog::CivAt(int civ) const
{
    if (!c_assert(civ >= 0 && civ < civ_count)) return civs[0];
    return civs[civ];
}

int CivCatalog::DisplayCiv(int position) const
{
    if (!c_assert(position >= 0 && position < civ_count)) return 0;
    return display_order[position];
}

bool CivCatalog::InEdition(int civ, int edition) const
{
    if (!c_assert(civ >= 0 && civ < civ_count) || !c_assert(edition >= 0 && edition < edition_count)) return false;
    return editions[civs[civ].edition].year <= editions[edition].year;
}

int CivCatalog::EditionCivCount(int edition) const
{
    if (!c_assert(edition >= 0 && edition < edition_count)) return 0;
    int count = 0;
    for (int i = 0; i < civ_count && i < MAX_CIVS; i++)
    {
        if (InEdition(i, edition)) count++;
    }
    return count;
}

int CivCatalog::LinkCount() const
{
    int count = 0;
    for (int i = 0; i < edition_count && i < MAX_EDITIONS; i++) count += editions[i].link_count;
    return count;
}

const EditionLink& CivCatalog::LinkAt(int link) const
{
    if (!c_assert(link >= 0 && link < LinkCount())) return editions[0].links[0];
    int first = 0;
    for (int i = 0; i < edition_count && i < MAX_EDITIONS; i++)
    {
        const int count = editions[i].link_count;
        if (link < first + count) return editions[i].links[link - first];
        first += count;
    }
    return editions[0].links[0];
}
