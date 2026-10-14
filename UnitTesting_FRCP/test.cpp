#include "pch.h"
#include <gtest/gtest.h>
#include "TestData.h"
#include "../Fresh-Random-Civ-Picker_CPPGUI/CivPickerModel.cpp"

// The model with the editions, DLCs and civs of the program's data.json.

TEST(CompletenessTest, AllCivsAreDrawn) {
	CivCatalog catalog;
	ASSERT_TRUE(catalog.Load(ReadProgramDataFile()));
	CivPickerModel model(catalog);
	model.ResetProgram(false);

	for (int i = 0; i < catalog.CivCount(); i++) {
		model.DrawCiv(false, L"");
	}

	EXPECT_EQ(model.GetRemaining(), 0);
	for (int i = 0; i < catalog.CivCount(); i++) {
		EXPECT_EQ(model.GetPoolEntry(i), L"");
		EXPECT_TRUE(model.HasBeenDrawn(model.GetCiv(i).name));
	}
}

TEST(EditionTest, SwitchingEditionLeavesItsCivs) {
	CivCatalog catalog;
	ASSERT_TRUE(catalog.Load(ReadProgramDataFile()));
	for (int edition = 0; edition < catalog.EditionCount(); edition++) {
		CivPickerModel model(catalog);
		model.ResetProgram(false);
		model.SetEdition(edition);

		EXPECT_EQ(model.GetEdition(), edition);
		EXPECT_EQ(model.GetCustomMaxCivs(), catalog.EditionCivCount(edition));
		for (int civ = 0; civ < catalog.CivCount(); civ++) {
			EXPECT_EQ(model.GetCiv(civ).enabled, catalog.InEdition(civ, edition));
		}
	}
}

TEST(EditionTest, DlcsAreEmptyOnlyWithoutEnabledCivs) {
	CivCatalog catalog;
	ASSERT_TRUE(catalog.Load(ReadProgramDataFile()));
	CivPickerModel model(catalog);
	model.ResetProgram(false);
	for (int dlc = 0; dlc < catalog.DlcCount(); dlc++) {
		model.SetDlcEnabled(dlc, false);
		EXPECT_TRUE(model.IsDlcEmpty(dlc));
		model.SetDlcEnabled(dlc, true);
		bool has_civs = false;
		for (int civ = 0; civ < catalog.CivCount(); civ++) has_civs = has_civs || catalog.CivAt(civ).dlc == dlc;
		EXPECT_EQ(model.IsDlcEmpty(dlc), !has_civs);
	}
}

TEST(ShareTest, ALoadedPoolIsTheSharedOne) {
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
	for (int civ = 0; civ < catalog.CivCount(); civ++) {
		EXPECT_EQ(loaded.enabled[civ], shared.enabled[civ]);
	}
	ASSERT_EQ(loaded.draw_count, 3);
	for (int i = 0; i < 3; i++) {
		EXPECT_EQ(loaded.draws[i], shared.draws[i]);
	}
	EXPECT_EQ(loading.GetIterator(), 3);
	EXPECT_EQ(loading.GetCurrentCiv(), catalog.CivAt(shared.draws[2]).name);
	EXPECT_EQ(loading.GetCustomMaxCivs(), sharing.GetCustomMaxCivs());
	EXPECT_TRUE(loading.HasCustomState());
}

TEST(ShareTest, AnUndrawnCivIsNotShared) {
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

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
