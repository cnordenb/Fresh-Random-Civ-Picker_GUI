#include "CivCatalog.h"

#include <cwctype>

#include "Assertions.h"
#include "JsonReader.h"

namespace
{
    constexpr int MAX_KEY_LENGTH = 64;
    constexpr int MAX_CIV_NAME_LENGTH = 40;
    // The program's name for "no civ drawn yet", which a civ can't have.
    const wchar_t* const NO_CIV_NAME = L"Random";

    std::wstring Widen(const char* ascii)
    {
        std::wstring wide;
        if (!c_assert(ascii != nullptr)) return wide;
        for (int i = 0; i < MAX_KEY_LENGTH && ascii[i] != '\0'; i++) wide.push_back(static_cast<wchar_t>(ascii[i]));
        return wide;
    }

    const wchar_t* TypeName(JsonType type)
    {
        switch (type)
        {
            case JsonType::object: return L"an object";
            case JsonType::array: return L"a list";
            case JsonType::string: return L"a text";
            case JsonType::number: return L"a number";
            case JsonType::literal: return L"true, false or null";
        }
        return L"a value";
    }

    std::wstring ParseProblem(JsonStatus status, int line)
    {
        (void)c_assert(line >= 1);
        const std::wstring at = L" (line " + std::to_wstring(line) + L")";
        switch (status)
        {
            case JsonStatus::too_long: return L"data.json: the file is too long";
            case JsonStatus::too_many_values: return L"data.json: the file holds too many values" + at;
            case JsonStatus::too_deep: return L"data.json: the file nests values too deeply" + at;
            case JsonStatus::syntax_error: return L"data.json: the file is not valid JSON" + at;
            case JsonStatus::ok: break;
        }
        return L"data.json: the file could not be read";
    }

    std::wstring LowerCase(const std::wstring& text)
    {
        std::wstring lower = text;
        for (size_t i = 0; i < lower.size() && i < static_cast<size_t>(MAX_CIV_NAME_LENGTH); i++)
            lower[i] = static_cast<wchar_t>(std::towlower(lower[i]));
        return lower;
    }

    // A civ's name is a word: the log file separates a name from its state with a space.
    bool IsUsableCivName(const std::wstring& name)
    {
        const size_t longest = static_cast<size_t>(MAX_CIV_NAME_LENGTH);
        if (name.empty() || name.size() > longest || name == NO_CIV_NAME) return false;
        for (size_t i = 0; i < name.size() && i < longest; i++)
        {
            if (std::iswspace(name[i]) || std::iswcntrl(name[i])) return false;
        }
        return true;
    }
}

// Reads data.json's values for the catalog and remembers the first thing that is missing or of the wrong kind,
// with the place it was found (Enter). After a problem every read gives an empty value, so reading can go on to
// the end; Load then reports that first problem.
class DataReader final
{
public:
    explicit DataReader(const JsonDocument& json) : document(json) {}

    const JsonDocument& Document() const { return document; }
    bool Failed() const { return !problem.empty(); }
    const std::wstring& Problem() const { return problem; }
    void Enter(const std::wstring& where)
    {
        if (!c_assert(!where.empty())) return;
        place = where;
    }

    void Fail(const std::wstring& what)
    {
        if (!c_assert(!what.empty())) return;
        if (problem.empty()) problem = L"data.json: " + what;
    }

    // A member that must be there, of the given type; NONE (and a problem) otherwise.
    int Required(int object, const char* key, JsonType type)
    {
        if (!c_assert(key != nullptr)) return JsonDocument::NONE;
        const int value = document.Member(object, key);
        if (document.Is(value, type)) return value;
        Fail(place + L" needs \"" + Widen(key) + L"\" (" + TypeName(type) + L")");
        return JsonDocument::NONE;
    }

    std::wstring TextOf(int string_value)
    {
        if (!document.Is(string_value, JsonType::string))
        {
            Fail(place + L" has a value that should be a text");
            return L"";
        }
        const JsonDocument::TextResult text = document.Text(string_value);
        if (!text.ok) Fail(place + L" has a text that is not valid UTF-8");
        return text.text;
    }

    std::wstring Text(int object, const char* key)
    {
        if (!c_assert(key != nullptr)) return L"";
        const int value = Required(object, key, JsonType::string);
        return value != JsonDocument::NONE ? TextOf(value) : L"";
    }

    // A text member that may be missing or null.
    std::wstring OptionalText(int object, const char* key)
    {
        if (!c_assert(key != nullptr)) return L"";
        const int value = document.Member(object, key);
        if (value == JsonDocument::NONE || document.IsNull(value)) return L"";
        return TextOf(value);
    }

    int Integer(int object, const char* key)
    {
        if (!c_assert(key != nullptr)) return 0;
        const int value = Required(object, key, JsonType::number);
        if (value == JsonDocument::NONE) return 0;
        const JsonDocument::IntegerResult number = document.Integer(value);
        if (!number.ok) Fail(place + L"'s \"" + Widen(key) + L"\" is not a whole number");
        return number.ok ? number.value : 0;
    }

    // A member that is either true or missing (false).
    bool Flag(int object, const char* key) const
    {
        if (!c_assert(key != nullptr)) return false;
        return document.IsTrue(document.Member(object, key));
    }

private:
    const JsonDocument& document;
    std::wstring problem;
    std::wstring place = L"the file";
};

// --- Loading ---

bool CivCatalog::Load(const std::string& utf8_json)
{
    edition_count = 0;
    dlc_count = 0;
    civ_count = 0;
    error.clear();
    JsonDocument document;
    const JsonStatus status = document.Parse(utf8_json);
    if (status != JsonStatus::ok) return Fail(ParseProblem(status, document.ErrorLine()));
    const int root = document.Root();
    if (!document.Is(root, JsonType::object)) return Fail(L"data.json: the file holds no object");
    DataReader reader(document);
    // The DLCs first: the editions and civs refer to them by key, and they to the editions by ID.
    ReadDlcs(reader, root);
    ReadEditions(reader, root);
    ReadDlcEditions(reader, root);
    ReadCivs(reader, root);
    if (reader.Failed()) return Fail(reader.Problem());
    if (!DeriveDlcs() || !DeriveCivs()) return false;
    SortDisplayOrder();
    return c_assert(edition_count > 0 && dlc_count > 0 && civ_count > 0);
}

bool CivCatalog::Fail(const std::wstring& problem)
{
    (void)c_assert(!problem.empty());
    error = problem;
    edition_count = 0;
    dlc_count = 0;
    civ_count = 0;
    return false;
}

const std::wstring& CivCatalog::Error() const { return error; }

void CivCatalog::ReadDlcs(DataReader& reader, int root)
{
    if (!c_assert(reader.Document().Is(root, JsonType::object))) return;
    reader.Enter(L"the file");
    const int object = reader.Required(root, "dlcs", JsonType::object);
    const int count = reader.Document().Count(object);
    if (count < 1 || count > MAX_DLCS)
    {
        reader.Fail(L"\"dlcs\" must list 1 to " + std::to_wstring(MAX_DLCS) + L" DLCs");
        return;
    }
    dlc_count = count;
    for (int i = 0; i < count; i++) ReadDlc(reader, reader.Document().MemberKey(object, i), i);
}

// One member of "dlcs": its key, and the object after it (a member's value follows its key).
void CivCatalog::ReadDlc(DataReader& reader, int key, int dlc)
{
    if (!c_assert(dlc >= 0 && dlc < MAX_DLCS) || !c_assert(key != JsonDocument::NONE))
    {
        reader.Fail(L"the list of DLCs could not be read");
        return;
    }
    DlcInfo& info = dlcs[dlc];
    info.key = reader.TextOf(key);
    const std::wstring where = L"DLC \"" + info.key + L"\"";
    reader.Enter(where);
    const int object = key + 1;
    if (!reader.Document().Is(object, JsonType::object))
    {
        reader.Fail(where + L" is not an object");
        return;
    }
    info.name = reader.Text(object, "name");
    info.short_name = reader.OptionalText(object, "short");
    info.released = reader.Text(object, "released");
    // Numbered among the DLCs with icons by DeriveDlcs.
    info.icon_index = reader.OptionalText(object, "icon").empty() ? NONE : 0;
    info.edition = NONE;
    info.toggle_edition = NONE;
    info.toggle_slot = NONE;
    if (info.key.empty() || FindDlc(info.key) != dlc) reader.Fail(where + L" has an empty or repeated key");
}

int CivCatalog::FindDlc(const std::wstring& key) const
{
    for (int i = 0; i < dlc_count && i < MAX_DLCS; i++)
    {
        if (dlcs[i].key == key) return i;
    }
    return NONE;
}

void CivCatalog::ReadEditions(DataReader& reader, int root)
{
    if (!c_assert(reader.Document().Is(root, JsonType::object)) || reader.Failed()) return;
    reader.Enter(L"the file");
    const int array = reader.Required(root, "editions", JsonType::array);
    const int count = reader.Document().Count(array);
    if (count < 1 || count > MAX_EDITIONS)
    {
        reader.Fail(L"\"editions\" must list 1 to " + std::to_wstring(MAX_EDITIONS) + L" editions");
        return;
    }
    edition_count = count;
    for (int i = 0; i < count; i++) ReadEdition(reader, reader.Document().Element(array, i), i);
}

void CivCatalog::ReadEdition(DataReader& reader, int object, int edition)
{
    if (!c_assert(edition >= 0 && edition < MAX_EDITIONS)) return;
    reader.Enter(L"edition " + std::to_wstring(edition + 1));
    EditionInfo& info = editions[edition];
    info.id = reader.Text(object, "id");
    const std::wstring where = L"edition \"" + info.id + L"\"";
    reader.Enter(where);
    info.name = reader.Text(object, "name");
    info.short_name = reader.Text(object, "short");
    info.series = reader.Text(object, "series");
    info.year = reader.Integer(object, "year");
    info.url = reader.OptionalText(object, "url");
    info.store_note = reader.OptionalText(object, "storeNote");
    if (info.url.empty() && info.store_note.empty()) reader.Fail(where + L" needs a \"url\" or a \"storeNote\"");
    info.legacy_jingles = reader.Flag(object, "legacyJingles");
    ReadEditionToggles(reader, object, edition);
    ReadEditionLinks(reader, object, edition);
    if (info.id.empty() || FindEdition(info.id) != edition) reader.Fail(where + L" has an empty or repeated ID");
}

// The edition's DLC toggles on the Civ Pool tab and where they go, and the DLCs auto-toggle switches on with it.
void CivCatalog::ReadEditionToggles(DataReader& reader, int object, int edition)
{
    if (!c_assert(edition >= 0 && edition < edition_count)) return;
    EditionInfo& info = editions[edition];
    const std::wstring where = L"edition \"" + info.id + L"\"";
    const DlcList toggles = ReadDlcList(reader, reader.Required(object, "dlcToggles", JsonType::array), where);
    if (toggles.count > MAX_DLC_TOGGLES)
        reader.Fail(where + L" has more than " + std::to_wstring(MAX_DLC_TOGGLES) + L" DLC toggles");
    info.dlc_toggle_count = toggles.count <= MAX_DLC_TOGGLES ? toggles.count : 0;
    for (int i = 0; i < info.dlc_toggle_count; i++) info.dlc_toggles[i] = toggles.items[i];
    info.toggle_layout = ReadToggleLayout(reader, reader.Required(object, "dlcToggleLayout", JsonType::object), where);
    const DlcList older = ReadDlcList(reader, reader.Required(object, "olderDlcs", JsonType::array), where);
    info.older_dlc_count = older.count;
    for (int i = 0; i < older.count && i < MAX_DLCS; i++) info.older_dlcs[i] = older.items[i];
}

// The Game menu's links for the edition; an edition may have none.
void CivCatalog::ReadEditionLinks(DataReader& reader, int object, int edition)
{
    if (!c_assert(edition >= 0 && edition < edition_count)) return;
    EditionInfo& info = editions[edition];
    info.link_count = 0;
    const JsonDocument& document = reader.Document();
    const int array = document.Member(object, "links");
    if (array == JsonDocument::NONE) return;
    const int count = document.Count(array);
    if (!document.Is(array, JsonType::array) || count > MAX_EDITION_LINKS)
    {
        reader.Fail(L"edition \"" + info.id + L"\" needs \"links\" to be a list of at most " +
                    std::to_wstring(MAX_EDITION_LINKS));
        return;
    }
    for (int i = 0; i < count; i++)
    {
        const int link = document.Element(array, i);
        info.links[i] = { reader.Text(link, "text"), reader.Text(link, "url") };
    }
    info.link_count = count;
}

CivCatalog::DlcList CivCatalog::ReadDlcList(DataReader& reader, int array, const std::wstring& where) const
{
    DlcList list = {};
    if (!c_assert(!where.empty())) return list;
    const JsonDocument& document = reader.Document();
    const int count = document.Count(array);
    if (count > MAX_DLCS)
    {
        reader.Fail(where + L" lists too many DLCs");
        return list;
    }
    for (int i = 0; i < count; i++)
    {
        const std::wstring key = reader.TextOf(document.Element(array, i));
        const int dlc = FindDlc(key);
        if (dlc == NONE)
        {
            reader.Fail(where + L" names an unknown DLC \"" + key + L"\"");
            return list;
        }
        list.items[list.count] = dlc;
        list.count++;
    }
    return list;
}

DlcToggleLayout CivCatalog::ReadToggleLayout(DataReader& reader, int object, const std::wstring& where) const
{
    if (!c_assert(!where.empty())) return { 0, 0, 0, 1, 0 };
    // A braced list is evaluated from left to right, so a problem is reported for the first bad member.
    const DlcToggleLayout layout = { reader.Integer(object, "left"), reader.Integer(object, "top"),
                                     reader.Integer(object, "step"), reader.Integer(object, "size"),
                                     reader.Integer(object, "gap") };
    const bool valid = layout.left >= 0 && layout.top >= 0 && layout.step >= 0 && layout.size > 0 && layout.gap >= 0;
    if (!valid) reader.Fail(where + L" has a DLC toggle layout with a negative or empty measure");
    return layout;
}

void CivCatalog::ReadDlcEditions(DataReader& reader, int root)
{
    if (!c_assert(reader.Document().Is(root, JsonType::object)) || reader.Failed()) return;
    const JsonDocument& document = reader.Document();
    const int object = document.Member(root, "dlcs");
    for (int i = 0; i < dlc_count && i < MAX_DLCS; i++)
    {
        const std::wstring where = L"DLC \"" + dlcs[i].key + L"\"";
        reader.Enter(where);
        const int key = document.MemberKey(object, i);
        if (!c_assert(key != JsonDocument::NONE)) return;
        const std::wstring id = reader.Text(key + 1, "edition");
        dlcs[i].edition = FindEdition(id);
        if (dlcs[i].edition == NONE) reader.Fail(where + L" names an unknown edition \"" + id + L"\"");
    }
}

void CivCatalog::ReadCivs(DataReader& reader, int root)
{
    if (!c_assert(reader.Document().Is(root, JsonType::object)) || reader.Failed()) return;
    reader.Enter(L"the file");
    const int array = reader.Required(root, "civs", JsonType::array);
    const int count = reader.Document().Count(array);
    if (count < 1 || count > MAX_CIVS)
    {
        reader.Fail(L"\"civs\" must list 1 to " + std::to_wstring(MAX_CIVS) + L" civs");
        return;
    }
    civ_count = count;
    for (int i = 0; i < MAX_CIVS; i++) civ_read[i] = false;
    for (int i = 0; i < count; i++) ReadCiv(reader, reader.Document().Element(array, i), i);
}

// A civ goes to the place of its ID. With as many IDs as civs, each below their number and none used twice, every
// place from 0 on gets one civ.
void CivCatalog::ReadCiv(DataReader& reader, int object, int position)
{
    if (!c_assert(position >= 0 && position < civ_count)) return;
    const std::wstring where = L"civ " + std::to_wstring(position + 1);
    reader.Enter(where);
    const int id = reader.Integer(object, "id");
    if (reader.Failed()) return;
    if (id < 0 || id >= civ_count || civ_read[id])
    {
        reader.Fail(where + L" has an ID that is out of range or used twice");
        return;
    }
    civ_read[id] = true;
    CivInfo& civ = civs[id];
    civ.name = reader.Text(object, "name");
    const std::wstring dlc_key = reader.Text(object, "dlc");
    const std::wstring tech_tree = reader.OptionalText(object, "techTree");
    civ.tech_tree = tech_tree.empty() ? civ.name : tech_tree;
    civ.dlc = FindDlc(dlc_key);
    if (civ.dlc == NONE) reader.Fail(where + L" names an unknown DLC \"" + dlc_key + L"\"");
}

// What follows from the DLCs as read: their icons' places among the DLC icons, and where their toggles are.
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
            // A toggle shows the DLC's icon.
            if (dlcs[dlc].icon_index == NONE)
                return Fail(L"data.json: DLC \"" + dlcs[dlc].key + L"\" has a toggle but no \"icon\"");
            dlcs[dlc].toggle_edition = edition;
            dlcs[dlc].toggle_slot = slot;
        }
    }
    return true;
}

// A civ belongs to its DLC's edition; the civs of an edition with legacy jingles have one each.
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

// The order of their names, in which the Civ Pool tab and the History list show the civs (as the web version does).
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

// --- Reading the catalog ---

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

// The civ at a place in the order of names.
int CivCatalog::DisplayCiv(int position) const
{
    if (!c_assert(position >= 0 && position < civ_count)) return 0;
    return display_order[position];
}

// An edition has its own civs and those of every older edition.
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

// The editions' links, one after the other, as the Game menu lists them.
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
