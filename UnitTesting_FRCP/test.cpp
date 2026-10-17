#include "pch.h"

#include <algorithm>

#include "TestData.h"

#include "CivPickerModel.cpp"

// The model with the editions, DLCs and civs of the program's data.json.

TEST(CompletenessTest, AllCivsAreDrawn)
{
    CivCatalog catalog;
    ASSERT_TRUE(catalog.Load(ReadProgramDataFile()));
    CivPickerModel model(catalog);
    model.ResetProgram(false);

    for (int i = 0; i < catalog.CivCount(); i++)
    {
        model.DrawCiv(false, L"");
    }

    EXPECT_EQ(model.GetRemaining(), 0);
    for (int i = 0; i < catalog.CivCount(); i++)
    {
        EXPECT_EQ(model.GetPoolEntry(i), L"");
        EXPECT_TRUE(model.HasBeenDrawn(model.GetCiv(i).name));
    }
}

TEST(EditionTest, SwitchingEditionLeavesItsCivs)
{
    CivCatalog catalog;
    ASSERT_TRUE(catalog.Load(ReadProgramDataFile()));
    for (int edition = 0; edition < catalog.EditionCount(); edition++)
    {
        CivPickerModel model(catalog);
        model.ResetProgram(false);
        model.SetEdition(edition);

        EXPECT_EQ(model.GetEdition(), edition);
        EXPECT_EQ(model.GetCustomMaxCivs(), catalog.EditionCivCount(edition));
        for (int civ = 0; civ < catalog.CivCount(); civ++)
        {
            EXPECT_EQ(model.GetCiv(civ).enabled, catalog.InEdition(civ, edition));
        }
    }
}

TEST(EditionTest, DlcsAreEmptyOnlyWithoutEnabledCivs)
{
    CivCatalog catalog;
    ASSERT_TRUE(catalog.Load(ReadProgramDataFile()));
    CivPickerModel model(catalog);
    model.ResetProgram(false);
    for (int dlc = 0; dlc < catalog.DlcCount(); dlc++)
    {
        model.SetDlcEnabled(dlc, false);
        EXPECT_TRUE(model.IsDlcEmpty(dlc));
        model.SetDlcEnabled(dlc, true);
        bool has_civs = false;
        for (int civ = 0; civ < catalog.CivCount(); civ++) has_civs = has_civs || catalog.CivAt(civ).dlc == dlc;
        EXPECT_EQ(model.IsDlcEmpty(dlc), !has_civs);
    }
}

TEST(ShareTest, ALoadedPoolIsTheSharedOne)
{
    CivCatalog catalog;
    ASSERT_TRUE(catalog.Load(ReadProgramDataFile()));
    const int edition = catalog.EditionCount() - 1;
    CivPickerModel sharing(catalog);
    sharing.ResetProgram(false);
    sharing.SetEdition(edition);
    for (int civ = 0; civ < catalog.CivCount(); civ += 4) sharing.RemoveCiv(catalog.CivAt(civ).name);
    sharing.ResetProgram(true);
    for (int i = 0; i < 3; i++) sharing.DrawCiv(false, L"");
    const SharedPool shared = sharing.GetSharedPool();
    ASSERT_EQ(shared.draw_count, 3);

    CivPickerModel loading(catalog);
    loading.ResetProgram(false);
    EXPECT_FALSE(loading.HasCustomState());
    loading.LoadSharedPool(shared);
    const SharedPool loaded = loading.GetSharedPool();
    EXPECT_EQ(loaded.edition, edition);
    for (int civ = 0; civ < catalog.CivCount(); civ++)
    {
        EXPECT_EQ(loaded.enabled[civ], shared.enabled[civ]);
    }
    ASSERT_EQ(loaded.draw_count, 3);
    for (int i = 0; i < 3; i++)
    {
        EXPECT_EQ(loaded.draws[i], shared.draws[i]);
    }
    EXPECT_EQ(loading.GetIterator(), 3);
    EXPECT_EQ(loading.GetCurrentCiv(), catalog.CivAt(shared.draws[2]).name);
    EXPECT_EQ(loading.GetCustomMaxCivs(), sharing.GetCustomMaxCivs());
    EXPECT_TRUE(loading.HasCustomState());
}

TEST(ShareTest, AnUndrawnCivIsNotShared)
{
    CivCatalog catalog;
    ASSERT_TRUE(catalog.Load(ReadProgramDataFile()));
    CivPickerModel model(catalog);
    model.ResetProgram(false);
    for (int i = 0; i < 4; i++) model.DrawCiv(false, L"");
    model.UndrawCiv();
    const SharedPool shared = model.GetSharedPool();
    ASSERT_EQ(shared.draw_count, 3);
    EXPECT_EQ(catalog.CivAt(shared.draws[2]).name, model.GetCurrentCiv());
}

// Continuous freshness counts civs, as the web version does: the last N civs drawn sit out, N at most half the pool.
TEST(FreshnessTest, TheLastCivsOfARoundSitOutTheNextRoundsFirstDraws)
{
    CivCatalog catalog;
    ASSERT_TRUE(catalog.Load(ReadProgramDataFile()));
    CivPickerModel model(catalog);
    model.EditSettings().contfresh_enabled = true;
    model.ResetProgram(false);
    const int pool = model.GetCustomMaxCivs();
    const int sitting_out = 5;
    ASSERT_GT(pool, 2 * sitting_out);
    model.SetFreshness(sitting_out);
    ASSERT_EQ(model.FreshnessLimit(), sitting_out);
    std::wstring round[MAX_CIVS];
    for (int i = 0; i < pool; i++)
    {
        model.DrawCiv(false, L"");
        round[i] = model.GetCurrentCiv();
    }
    // Each draw of the next round moves the window on by one civ.
    for (int draw = 0; draw < sitting_out; draw++)
    {
        model.DrawCiv(false, L"");
        for (int i = pool - sitting_out + draw; i < pool; i++) EXPECT_NE(model.GetCurrentCiv(), round[i]);
    }
}

TEST(FreshnessTest, TheMostFollowsThePoolsSize)
{
    CivCatalog catalog;
    ASSERT_TRUE(catalog.Load(ReadProgramDataFile()));
    CivPickerModel model(catalog);
    model.ResetProgram(false);
    EXPECT_EQ(model.FreshnessMaximum(), model.GetCustomMaxCivs() / 2);
    model.SetFreshness(model.FreshnessMaximum());
    EXPECT_EQ(model.GetSettings().contfresh_civs, CONTFRESH_MOST);
    for (int edition = 0; edition < catalog.EditionCount(); edition++)
    {
        model.SetEdition(edition);
        EXPECT_EQ(model.FreshnessLimit(), model.GetCustomMaxCivs() / 2);
    }
    model.SetFreshness(1);
    EXPECT_EQ(model.FreshnessLimit(), 1);
    EXPECT_EQ(model.GetSettings().contfresh_civs, 1);
}

// The Civ Pool tab's edition and DLC hotkeys are in use as far as data.json gives them editions and DLC toggles.
TEST(HotkeyTest, TheModelsHotkeysFollowDataJson)
{
    CivCatalog catalog;
    ASSERT_TRUE(catalog.Load(ReadProgramDataFile()));
    const CivPickerModel model(catalog);
    int most_toggles = 0;
    for (int edition = 0; edition < catalog.EditionCount(); edition++)
        most_toggles = std::max(most_toggles, catalog.EditionAt(edition).dlc_toggle_count);

    for (int edition = 0; edition < MAX_EDITIONS; edition++)
    {
        const HotkeyAction action = static_cast<HotkeyAction>(static_cast<int>(HotkeyAction::edition_1) + edition);
        EXPECT_EQ(model.GetHotkeys().IsUsed(action), edition < catalog.EditionCount()) << edition;
    }
    for (int slot = 0; slot < DLC_HOTKEY_COUNT; slot++)
    {
        const HotkeyAction action = static_cast<HotkeyAction>(static_cast<int>(HotkeyAction::dlc_slot_1) + slot);
        EXPECT_EQ(model.GetHotkeys().IsUsed(action), slot < most_toggles) << slot;
    }
    const Keystroke tab = { 0, KEY_TAB };
    EXPECT_TRUE(model.GetHotkeys().KeystrokeOf(HotkeyAction::cycle_tabs) == tab);
}
