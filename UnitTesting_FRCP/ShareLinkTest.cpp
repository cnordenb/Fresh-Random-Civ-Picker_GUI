#include "pch.h"

#include <string>

#include "TestData.h"

#include "../Fresh-Random-Civ-Picker_CPPGUI/ShareLink.cpp"

// Share links as the web version writes and opens them, with the program's data.json; so the tests hold for any
// editions and civs it lists.

namespace
{
    const std::wstring ADDRESS = L"https://frcp.net/#p=";
    const std::wstring BASE64URL = L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

    CivCatalog LoadedCatalog()
    {
        CivCatalog catalog;
        EXPECT_TRUE(catalog.Load(ReadProgramDataFile()));
        return catalog;
    }

    // The edition's civs, every third one left out, and up to three draws from the rest in reverse ID order.
    SharedPool SamplePool(const CivCatalog& catalog, int edition)
    {
        SharedPool pool = {};
        pool.edition = edition;
        for (int civ = 0; civ < catalog.CivCount(); civ++)
            pool.enabled[civ] = catalog.InEdition(civ, edition) && civ % 3 != 1;
        for (int civ = catalog.CivCount() - 1; civ >= 0 && pool.draw_count < 3; civ--)
        {
            if (!pool.enabled[civ]) continue;
            pool.draws[pool.draw_count] = civ;
            pool.draw_count++;
        }
        return pool;
    }

    std::wstring Written(const CivCatalog& catalog, const SharedPool& pool)
    {
        return ShareLink::Write(catalog, pool).Characters();
    }

    int BitfieldLength(const CivCatalog& catalog) { return (catalog.CivCount() + 5) / 6; }

    void ExpectSamePool(const CivCatalog& catalog, const SharedPool& expected, const SharedPool& actual)
    {
        EXPECT_EQ(actual.edition, expected.edition);
        for (int civ = 0; civ < catalog.CivCount(); civ++) EXPECT_EQ(actual.enabled[civ], expected.enabled[civ]);
        ASSERT_EQ(actual.draw_count, expected.draw_count);
        for (int i = 0; i < expected.draw_count; i++) EXPECT_EQ(actual.draws[i], expected.draws[i]);
    }
}

TEST(ShareLink, StartsWithTheWebVersionsAddressAndFormatVersion)
{
    const CivCatalog catalog = LoadedCatalog();
    const std::wstring link = Written(catalog, SamplePool(catalog, 0));
    EXPECT_EQ(link.substr(0, ADDRESS.size() + 2), ADDRESS + L"1" + BASE64URL[0]);
}

TEST(ShareLink, WritesAFullPoolAsSetBitsAndDrawsAfterADot)
{
    const CivCatalog catalog = LoadedCatalog();
    for (int edition = 0; edition < catalog.EditionCount(); edition++)
    {
        if (catalog.EditionCivCount(edition) != catalog.CivCount()) continue;
        SharedPool pool = {};
        pool.edition = edition;
        for (int civ = 0; civ < catalog.CivCount(); civ++) pool.enabled[civ] = true;
        const std::wstring bits(static_cast<size_t>(BitfieldLength(catalog)), L'_');
        EXPECT_EQ(Written(catalog, pool), ADDRESS + L"1" + BASE64URL[static_cast<size_t>(edition)] + bits);
        pool.draws[0] = 0;
        pool.draws[1] = catalog.CivCount() - 1;
        pool.draw_count = 2;
        const size_t last_civ = static_cast<size_t>(catalog.CivCount() - 1);
        const std::wstring draws = std::wstring(L".") + BASE64URL[0] + BASE64URL[last_civ];
        EXPECT_EQ(Written(catalog, pool), ADDRESS + L"1" + BASE64URL[static_cast<size_t>(edition)] + bits + draws);
    }
}

TEST(ShareLink, OpensWhatItWritesForEveryEdition)
{
    const CivCatalog catalog = LoadedCatalog();
    for (int edition = 0; edition < catalog.EditionCount(); edition++)
    {
        const SharedPool pool = SamplePool(catalog, edition);
        const ShareLink::Opened opened = ShareLink::Open(catalog, Written(catalog, pool).c_str());
        ASSERT_TRUE(opened.ok);
        ExpectSamePool(catalog, pool, opened.pool);
    }
}

TEST(ShareLink, OpensACodeWithOrWithoutItsAddress)
{
    const CivCatalog catalog = LoadedCatalog();
    const SharedPool pool = SamplePool(catalog, 0);
    const std::wstring code = Written(catalog, pool).substr(ADDRESS.size());
    const std::wstring texts[] = { code, L"p=" + code, L"#p=" + code, L"  \t" + ADDRESS + code + L" \n",
                                   L"https://example.org/index.html?x=1#p=" + code, L"%31" + code.substr(1) };
    for (const std::wstring& text : texts)
    {
        const ShareLink::Opened opened = ShareLink::Open(catalog, text.c_str());
        ASSERT_TRUE(opened.ok) << text;
        ExpectSamePool(catalog, pool, opened.pool);
    }
}

TEST(ShareLink, TurnsOnTheCivsPastAShortBitfield)
{
    const CivCatalog catalog = LoadedCatalog();
    const ShareLink::Opened opened = ShareLink::Open(catalog, (ADDRESS + L"1" + BASE64URL[0] + L"A").c_str());
    ASSERT_TRUE(opened.ok);
    for (int civ = 0; civ < catalog.CivCount(); civ++)
        EXPECT_EQ(opened.pool.enabled[civ], civ >= 6 && catalog.InEdition(civ, 0)) << civ;
    EXPECT_EQ(opened.pool.draw_count, 0);
}

TEST(ShareLink, LeavesOutTheCivsOfNewerEditions)
{
    const CivCatalog catalog = LoadedCatalog();
    for (int edition = 0; edition < catalog.EditionCount(); edition++)
    {
        const std::wstring bits(static_cast<size_t>(BitfieldLength(catalog)), L'_');
        const std::wstring link = ADDRESS + L"1" + BASE64URL[static_cast<size_t>(edition)] + bits;
        const ShareLink::Opened opened = ShareLink::Open(catalog, link.c_str());
        ASSERT_TRUE(opened.ok);
        for (int civ = 0; civ < catalog.CivCount(); civ++)
            EXPECT_EQ(opened.pool.enabled[civ], catalog.InEdition(civ, edition));
    }
}

TEST(ShareLink, RefusesTextThatIsNotALink)
{
    const CivCatalog catalog = LoadedCatalog();
    const std::wstring edition = BASE64URL.substr(0, 1);
    const std::wstring no_edition = BASE64URL.substr(static_cast<size_t>(catalog.EditionCount()), 1);
    const std::wstring no_civ = BASE64URL.substr(static_cast<size_t>(catalog.CivCount()), 1);
    const std::wstring texts[] = {
        L"", L"   ", L"hello", ADDRESS, ADDRESS + L"1", ADDRESS + L"1" + edition, ADDRESS + L"2" + edition + L"_",
        ADDRESS + L"1" + edition + L"_.", ADDRESS + L"1" + edition + L"_..A", ADDRESS + L"1" + edition + L"_!",
        ADDRESS + L"1" + edition + L"_.A.B", ADDRESS + L"1" + no_edition + L"_",
        ADDRESS + L"1" + edition + L"_." + no_civ,
        ADDRESS + L"1" + edition + L"_.AA", ADDRESS + L"1" + edition + L"A.A", ADDRESS + L"%", ADDRESS + L"%3",
        ADDRESS + L"%G1" + edition + L"_", ADDRESS + L"1" + edition + L"_%C3%A4", ADDRESS + L"1" + edition + L"\u00E4",
        ADDRESS + L"1" + edition + L"_ A",
    };
    for (const std::wstring& text : texts)
    {
        EXPECT_FALSE(ShareLink::Open(catalog, text.c_str()).ok) << text;
    }
}

TEST(ShareLink, RefusesTextLongerThanALinkCanBe)
{
    const CivCatalog catalog = LoadedCatalog();
    const std::wstring padding(ShareLink::MAX_INPUT_LENGTH, L' ');
    const std::wstring link = Written(catalog, SamplePool(catalog, 0));
    EXPECT_TRUE(ShareLink::Open(catalog, (padding.substr(0, 100) + link).c_str()).ok);
    EXPECT_FALSE(ShareLink::Open(catalog, (padding + link).c_str()).ok);
}
