#include "CivCatalog.h"

#include "Assertions.h"
#include "JsonReader.h"

namespace
{
    constexpr int MAX_KEY_LENGTH = 64;

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
            default: (void)c_assert(false); break;
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
            default: (void)c_assert(false); break;
        }
        return L"data.json: the file could not be read";
    }
}

class DataReader final
{
public:
    explicit DataReader(const JsonDocument& json);

    const JsonDocument& Document() const;
    bool Failed() const;
    const std::wstring& Problem() const;
    void Enter(const std::wstring& where);
    void Fail(const std::wstring& what);
    int Required(int object, const char* key, JsonType type);
    std::wstring TextOf(int string_value);
    std::wstring Text(int object, const char* key);
    std::wstring OptionalText(int object, const char* key);
    int Integer(int object, const char* key);
    bool Flag(int object, const char* key) const;

private:
    const JsonDocument& document;
    std::wstring problem;
    std::wstring place = L"the file";
};

DataReader::DataReader(const JsonDocument& json) :
    document(json)
{
}

const JsonDocument& DataReader::Document() const { return document; }

bool DataReader::Failed() const { return !problem.empty(); }

const std::wstring& DataReader::Problem() const { return problem; }

void DataReader::Enter(const std::wstring& where)
{
    if (!c_assert(!where.empty())) return;
    place = where;
}

void DataReader::Fail(const std::wstring& what)
{
    if (!c_assert(!what.empty())) return;
    if (problem.empty()) problem = L"data.json: " + what;
}

int DataReader::Required(int object, const char* key, JsonType type)
{
    if (!c_assert(key != nullptr)) return JsonDocument::NONE;
    const int value = document.Member(object, key);
    if (document.Is(value, type)) return value;
    Fail(place + L" needs \"" + Widen(key) + L"\" (" + TypeName(type) + L")");
    return JsonDocument::NONE;
}

std::wstring DataReader::TextOf(int string_value)
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

std::wstring DataReader::Text(int object, const char* key)
{
    if (!c_assert(key != nullptr)) return L"";
    const int value = Required(object, key, JsonType::string);
    return value != JsonDocument::NONE ? TextOf(value) : L"";
}

std::wstring DataReader::OptionalText(int object, const char* key)
{
    if (!c_assert(key != nullptr)) return L"";
    const int value = document.Member(object, key);
    if (value == JsonDocument::NONE || document.IsNull(value)) return L"";
    return TextOf(value);
}

int DataReader::Integer(int object, const char* key)
{
    if (!c_assert(key != nullptr)) return 0;
    const int value = Required(object, key, JsonType::number);
    if (value == JsonDocument::NONE) return 0;
    const JsonDocument::IntegerResult number = document.Integer(value);
    if (!number.ok) Fail(place + L"'s \"" + Widen(key) + L"\" is not a whole number");
    return number.ok ? number.value : 0;
}

bool DataReader::Flag(int object, const char* key) const
{
    if (!c_assert(key != nullptr)) return false;
    return document.IsTrue(document.Member(object, key));
}

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
    info.icon = reader.OptionalText(object, "icon");
    info.icon_index = info.icon.empty() ? NONE : 0;
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
    info.icon = reader.Text(object, "icon");
    info.banner = reader.Text(object, "banner");
    info.url = reader.OptionalText(object, "url");
    info.store_note = reader.OptionalText(object, "storeNote");
    if (info.url.empty() && info.store_note.empty()) reader.Fail(where + L" needs a \"url\" or a \"storeNote\"");
    info.legacy_jingles = reader.Flag(object, "legacyJingles");
    ReadEditionToggles(reader, object, edition);
    ReadEditionLinks(reader, object, edition);
    if (info.id.empty() || FindEdition(info.id) != edition) reader.Fail(where + L" has an empty or repeated ID");
}

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
