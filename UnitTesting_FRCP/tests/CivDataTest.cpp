#include "pch.h"

#include "TestData.h"

#include "CivCatalog.cpp"
#include "JsonReader.cpp"

// JsonDocument reads JSON as RFC 8259 defines it. CivCatalog reads the program's own data.json, so these tests hold
// for whatever editions, DLCs and civs it lists; the files with problems are data.json with one thing changed.

namespace
{
    JsonDocument Parsed(const std::string& text)
    {
        JsonDocument document;
        EXPECT_EQ(document.Parse(text), JsonStatus::ok) << text;
        return document;
    }

    std::wstring TextOf(const JsonDocument& document, int value)
    {
        const JsonDocument::TextResult text = document.Text(value);
        EXPECT_TRUE(text.ok);
        return text.text;
    }

    bool Contains(const std::wstring& text, const std::wstring& part) { return text.find(part) != std::wstring::npos; }
}

// --- JsonDocument ---

TEST(Json, FindsObjectMembersByKey)
{
    const JsonDocument document = Parsed(R"({"first": 1, "second": {"inner": [true, null]}, "third": "x"})");
    const int root = document.Root();
    EXPECT_TRUE(document.Is(root, JsonType::object));
    EXPECT_EQ(document.Count(root), 3);
    EXPECT_EQ(document.Integer(document.Member(root, "first")).value, 1);
    EXPECT_EQ(TextOf(document, document.Member(root, "third")), L"x");
    EXPECT_EQ(document.Member(root, "fourth"), JsonDocument::NONE);
    const int inner = document.Member(document.Member(root, "second"), "inner");
    EXPECT_EQ(document.Count(inner), 2);
    EXPECT_TRUE(document.IsTrue(document.Element(inner, 0)));
    EXPECT_TRUE(document.IsNull(document.Element(inner, 1)));
    EXPECT_EQ(document.Element(inner, 2), JsonDocument::NONE);
}

TEST(Json, WalksMembersAndElementsInOrder)
{
    const JsonDocument document = Parsed(R"({"a": [1, [2, 3], {"b": 4}], "c": 5})");
    const int root = document.Root();
    EXPECT_EQ(TextOf(document, document.MemberKey(root, 0)), L"a");
    EXPECT_EQ(TextOf(document, document.MemberKey(root, 1)), L"c");
    EXPECT_EQ(document.MemberKey(root, 2), JsonDocument::NONE);
    const int list = document.Member(root, "a");
    EXPECT_EQ(document.Count(list), 3);
    EXPECT_EQ(document.Count(document.Element(list, 1)), 2);
    EXPECT_EQ(document.Integer(document.Member(document.Element(list, 2), "b")).value, 4);
    EXPECT_EQ(document.Integer(document.Member(root, "c")).value, 5);
}

TEST(Json, ReadsEmptyContainers)
{
    const JsonDocument document = Parsed(R"({"none": {}, "empty": []})");
    EXPECT_EQ(document.Count(document.Member(document.Root(), "none")), 0);
    EXPECT_EQ(document.Count(document.Member(document.Root(), "empty")), 0);
}

TEST(Json, DecodesEscapes)
{
    const JsonDocument document = Parsed(R"(["quote \" backslash \\ slash \/ tab \t line \n", "Hj\u00f6rleif"])");
    EXPECT_EQ(TextOf(document, document.Element(document.Root(), 0)),
              L"quote \" backslash \\ slash / tab \t line \n");
    EXPECT_EQ(TextOf(document, document.Element(document.Root(), 1)), L"Hj\u00F6rleif");
}

TEST(Json, DecodesUtf8IncludingCharactersBeyondTheBasicPlane)
{
    // U+00F6 takes two bytes, U+20AC three and U+1F600 four, which UTF-16 writes as a surrogate pair.
    const JsonDocument document = Parsed("[\"\xC3\xB6 \xE2\x82\xAC \xF0\x9F\x98\x80\"]");
    EXPECT_EQ(TextOf(document, document.Element(document.Root(), 0)), L"\u00F6 \u20AC \xD83D\xDE00");
}

TEST(Json, RefusesMalformedUtf8InText)
{
    const JsonDocument document = Parsed("[\"\xC3\x28\", \"\xE0\x80\x80\"]");
    EXPECT_FALSE(document.Text(document.Element(document.Root(), 0)).ok);
    EXPECT_FALSE(document.Text(document.Element(document.Root(), 1)).ok); // an overlong encoding
}

TEST(Json, ReadsWholeNumbersOnly)
{
    const JsonDocument document = Parsed("[12, -7, 0, 1.5, 1e3, 1234567890]");
    const int root = document.Root();
    EXPECT_EQ(document.Integer(document.Element(root, 0)).value, 12);
    EXPECT_EQ(document.Integer(document.Element(root, 1)).value, -7);
    EXPECT_TRUE(document.Integer(document.Element(root, 2)).ok);
    EXPECT_FALSE(document.Integer(document.Element(root, 3)).ok);
    EXPECT_FALSE(document.Integer(document.Element(root, 4)).ok);
    EXPECT_FALSE(document.Integer(document.Element(root, 5)).ok); // more digits than an int is sure to hold
}

TEST(Json, TellsTypesApart)
{
    const JsonDocument document = Parsed(R"([{}, [], "", 0, false])");
    const int root = document.Root();
    EXPECT_TRUE(document.Is(document.Element(root, 0), JsonType::object));
    EXPECT_TRUE(document.Is(document.Element(root, 1), JsonType::array));
    EXPECT_TRUE(document.Is(document.Element(root, 2), JsonType::string));
    EXPECT_TRUE(document.Is(document.Element(root, 3), JsonType::number));
    EXPECT_TRUE(document.Is(document.Element(root, 4), JsonType::literal));
    EXPECT_FALSE(document.IsTrue(document.Element(root, 4)));
    EXPECT_FALSE(document.Integer(document.Element(root, 2)).ok);
    EXPECT_FALSE(document.Text(document.Element(root, 3)).ok);
}

TEST(Json, SkipsAByteOrderMark)
{
    const JsonDocument document = Parsed("\xEF\xBB\xBF{\"a\": 1}");
    EXPECT_EQ(document.Integer(document.Member(document.Root(), "a")).value, 1);
}

TEST(Json, RefusesMalformedText)
{
    const char* const malformed[] = {
        "", " ", "{", "[1,]", "{\"a\" 1}", "{\"a\": 1,}", "[01]", "[1.]", "[-]", "tru", "[\"\\x\"]", "[\"\\u12G4\"]",
        "[1 2]", "{} {}", "\"open", "[\"\x01\"]", "{1: 2}", "[}", "{]", "nul", "[+1]", "{\"a\": }",
    };
    for (const char* text : malformed)
    {
        JsonDocument document;
        EXPECT_EQ(document.Parse(text), JsonStatus::syntax_error) << '"' << text << '"';
        EXPECT_EQ(document.Root(), JsonDocument::NONE);
    }
}

TEST(Json, RefusesNestingDeeperThanItsLimit)
{
    JsonDocument document;
    const std::string deepest(JsonDocument::MAX_DEPTH, '[');
    EXPECT_EQ(document.Parse(deepest + std::string(JsonDocument::MAX_DEPTH, ']')), JsonStatus::ok);
    EXPECT_EQ(document.Parse("[" + deepest + std::string(JsonDocument::MAX_DEPTH + 1, ']')), JsonStatus::too_deep);
}

TEST(Json, RefusesMoreValuesThanItsLimit)
{
    std::string many = "[0";
    for (int i = 1; i < JsonDocument::MAX_VALUES; i++) many += ",0";
    JsonDocument document;
    EXPECT_EQ(document.Parse(many + "]"), JsonStatus::too_many_values);
}

TEST(Json, ReportsTheLineWhereItStopped)
{
    JsonDocument document;
    EXPECT_EQ(document.Parse("{\n  \"a\": 1,\n}"), JsonStatus::syntax_error);
    EXPECT_EQ(document.ErrorLine(), 3);
}

// --- CivCatalog with the program's data.json ---

TEST(CivCatalog, LoadsTheProgramsDataFile)
{
    CivCatalog catalog;
    ASSERT_TRUE(catalog.Load(ReadProgramDataFile())) << catalog.Error();
    EXPECT_TRUE(catalog.Error().empty());
    EXPECT_GT(catalog.EditionCount(), 0);
    EXPECT_GT(catalog.DlcCount(), 0);
    EXPECT_GT(catalog.CivCount(), 0);
}

TEST(CivCatalog, EveryCivBelongsToItsDlcsEdition)
{
    CivCatalog catalog;
    ASSERT_TRUE(catalog.Load(ReadProgramDataFile()));
    for (int civ = 0; civ < catalog.CivCount(); civ++)
    {
        const CivInfo& info = catalog.CivAt(civ);
        ASSERT_GE(info.dlc, 0);
        ASSERT_LT(info.dlc, catalog.DlcCount());
        EXPECT_EQ(info.edition, catalog.DlcAt(info.dlc).edition);
        EXPECT_EQ(info.legacy_jingle, catalog.EditionAt(info.edition).legacy_jingles);
        EXPECT_TRUE(catalog.InEdition(civ, info.edition));
    }
}

TEST(CivCatalog, EachDlcToggleKnowsItsPlace)
{
    CivCatalog catalog;
    ASSERT_TRUE(catalog.Load(ReadProgramDataFile()));
    int toggles = 0;
    for (int edition = 0; edition < catalog.EditionCount(); edition++)
    {
        const EditionInfo& info = catalog.EditionAt(edition);
        for (int slot = 0; slot < info.dlc_toggle_count; slot++)
        {
            const DlcInfo& dlc = catalog.DlcAt(info.dlc_toggles[slot]);
            EXPECT_EQ(dlc.toggle_edition, edition);
            EXPECT_EQ(dlc.toggle_slot, slot);
            toggles++;
        }
    }
    int dlcs_with_toggles = 0;
    for (int dlc = 0; dlc < catalog.DlcCount(); dlc++)
    {
        if (catalog.DlcAt(dlc).toggle_edition >= 0) dlcs_with_toggles++;
    }
    EXPECT_EQ(dlcs_with_toggles, toggles);
}

// The icon files are named in data.json: every edition's icon and banner, and the icon of every DLC that has one.
TEST(CivCatalog, NamesTheIconFilesOfEveryEditionAndDlc)
{
    CivCatalog catalog;
    ASSERT_TRUE(catalog.Load(ReadProgramDataFile()));
    for (int edition = 0; edition < catalog.EditionCount(); edition++)
    {
        EXPECT_FALSE(catalog.EditionAt(edition).icon.empty());
        EXPECT_FALSE(catalog.EditionAt(edition).banner.empty());
    }
    for (int dlc = 0; dlc < catalog.DlcCount(); dlc++)
    {
        const DlcInfo& info = catalog.DlcAt(dlc);
        EXPECT_EQ(info.icon.empty(), info.icon_index == CivCatalog::NONE) << dlc;
    }
}

namespace
{
    // data.json with extra DLCs of the first edition, toggled at the top of its list, as a new DLC is added.
    std::string ProgramDataFileWithNewDlcs(int count)
    {
        CivCatalog original;
        EXPECT_TRUE(original.Load(ReadProgramDataFile()));
        std::string edition_id;
        for (wchar_t character : original.EditionAt(0).id) edition_id.push_back(static_cast<char>(character));
        std::string dlcs;
        std::string toggles;
        for (int i = 0; i < count; i++)
        {
            const std::string key = "new-dlc-" + std::to_string(i);
            dlcs += "\"" + key + "\": {\"name\": \"New " + std::to_string(i) + "\", \"edition\": \"" + edition_id +
                    "\", \"icon\": \"" + key + "\", \"released\": \"2030-01-01\"}, ";
            toggles += "\"" + key + "\", ";
        }
        std::string data = ProgramDataFileWith("\"dlcToggles\": [", "\"dlcToggles\": [" + toggles);
        const std::string dlcs_start = "\"dlcs\": {";
        const size_t found = data.find(dlcs_start);
        if (found != std::string::npos) data.insert(found + dlcs_start.size(), dlcs);
        return data;
    }
}

TEST(CivCatalog, TakesDlcTogglesBeyondTheNineWithHotkeys)
{
    CivCatalog original;
    ASSERT_TRUE(original.Load(ReadProgramDataFile()));
    const int added = 10;
    CivCatalog catalog;
    ASSERT_TRUE(catalog.Load(ProgramDataFileWithNewDlcs(added))) << catalog.Error();
    const EditionInfo& first = catalog.EditionAt(0);
    ASSERT_EQ(first.dlc_toggle_count, original.EditionAt(0).dlc_toggle_count + added);
    EXPECT_GT(first.dlc_toggle_count, 9);
    for (int slot = 0; slot < first.dlc_toggle_count; slot++)
        EXPECT_EQ(catalog.DlcAt(first.dlc_toggles[slot]).toggle_slot, slot);
}

TEST(CivCatalog, TakesAsManyDlcsAsItsLimitAndNoMore)
{
    CivCatalog original;
    ASSERT_TRUE(original.Load(ReadProgramDataFile()));
    const int room = MAX_DLCS - original.DlcCount();
    CivCatalog full;
    EXPECT_TRUE(full.Load(ProgramDataFileWithNewDlcs(room))) << full.Error();
    EXPECT_EQ(full.DlcCount(), MAX_DLCS);
    CivCatalog over;
    EXPECT_FALSE(over.Load(ProgramDataFileWithNewDlcs(room + 1)));
    EXPECT_TRUE(Contains(over.Error(), std::to_wstring(MAX_DLCS)));
}

TEST(CivCatalog, AnEditionHasTheCivsOfEveryOlderEdition)
{
    CivCatalog catalog;
    ASSERT_TRUE(catalog.Load(ReadProgramDataFile()));
    for (int older = 0; older < catalog.EditionCount(); older++)
    {
        for (int newer = 0; newer < catalog.EditionCount(); newer++)
        {
            if (catalog.EditionAt(older).year > catalog.EditionAt(newer).year) continue;
            EXPECT_LE(catalog.EditionCivCount(older), catalog.EditionCivCount(newer));
            for (int civ = 0; civ < catalog.CivCount(); civ++)
            {
                if (catalog.InEdition(civ, older)) EXPECT_TRUE(catalog.InEdition(civ, newer));
            }
        }
    }
}

TEST(CivCatalog, ListsTheCivsInTheOrderOfTheirNames)
{
    CivCatalog catalog;
    ASSERT_TRUE(catalog.Load(ReadProgramDataFile()));
    std::vector<bool> listed(static_cast<size_t>(catalog.CivCount()), false);
    for (int position = 0; position < catalog.CivCount(); position++)
    {
        const int civ = catalog.DisplayCiv(position);
        EXPECT_FALSE(listed[static_cast<size_t>(civ)]);
        listed[static_cast<size_t>(civ)] = true;
        if (position == 0) continue;
        EXPECT_LT(catalog.CivAt(catalog.DisplayCiv(position - 1)).file_name, catalog.CivAt(civ).file_name);
    }
}

TEST(CivCatalog, NamesTheJinglesAndTechTreesOfEveryCiv)
{
    CivCatalog catalog;
    ASSERT_TRUE(catalog.Load(ReadProgramDataFile()));
    for (int civ = 0; civ < catalog.CivCount(); civ++)
    {
        const CivInfo& info = catalog.CivAt(civ);
        EXPECT_EQ(info.file_name.size(), info.name.size());
        EXPECT_EQ(_wcsicmp(info.file_name.c_str(), info.name.c_str()), 0);
        EXPECT_FALSE(info.tech_tree.empty());
    }
}

TEST(CivCatalog, ListsEveryEditionLinkOnce)
{
    CivCatalog catalog;
    ASSERT_TRUE(catalog.Load(ReadProgramDataFile()));
    int links = 0;
    for (int edition = 0; edition < catalog.EditionCount(); edition++)
    {
        const EditionInfo& info = catalog.EditionAt(edition);
        for (int i = 0; i < info.link_count; i++)
        {
            EXPECT_EQ(catalog.LinkAt(links).url, info.links[i].url);
            links++;
        }
    }
    EXPECT_EQ(catalog.LinkCount(), links);
}

TEST(CivCatalog, FindsEditionsByTheirIds)
{
    CivCatalog catalog;
    ASSERT_TRUE(catalog.Load(ReadProgramDataFile()));
    for (int edition = 0; edition < catalog.EditionCount(); edition++)
        EXPECT_EQ(catalog.FindEdition(catalog.EditionAt(edition).id), edition);
    EXPECT_EQ(catalog.FindEdition(L"no such edition"), CivCatalog::NONE);
}

TEST(CivCatalog, RefusesAFileWithAProblem)
{
    struct Problem { std::string original; std::string replacement; std::wstring reported; };
    const Problem problems[] = {
        { "\"dlc\": \"", "\"dlc\": \"no-such-dlc-", L"unknown DLC" },
        { "\"civs\"", "\"civz\"", L"needs \"civs\"" },
        { "\"id\": 1,", "\"id\": 0,", L"used twice" },
        { "\"edition\": \"", "\"edition\": \"no-such-edition-", L"unknown edition" },
        { "\"dlcToggles\": [", "\"dlcToggles\": [\"no-such-dlc\", ", L"unknown DLC" },
        { "\"year\": ", "\"year\": \"", L"(line" },
        { "\"id\": 0, \"name\": \"", "\"id\": 0, \"name\": \"Two words ", L"without spaces" },
        { "\"editions\": [", "\"editions\": [], \"x\": [", L"1 to" },
        { ", \"icon\": \"", ", \"no-icon\": \"", L"no \"icon\"" },
        { "\"icon\": \"", "\"no-icon\": \"", L"needs \"icon\"" },
        { "\"banner\": \"", "\"no-banner\": \"", L"needs \"banner\"" },
    };
    for (const Problem& problem : problems)
    {
        CivCatalog catalog;
        EXPECT_FALSE(catalog.Load(ProgramDataFileWith(problem.original, problem.replacement))) << problem.original;
        EXPECT_TRUE(Contains(catalog.Error(), problem.reported)) << problem.original;
        EXPECT_EQ(catalog.CivCount(), 0);
    }
}

TEST(CivCatalog, RefusesTextThatIsNotJson)
{
    CivCatalog catalog;
    const std::string data = ReadProgramDataFile();
    EXPECT_FALSE(catalog.Load(data.substr(0, data.size() / 2)));
    EXPECT_TRUE(Contains(catalog.Error(), L"not valid JSON"));
}
