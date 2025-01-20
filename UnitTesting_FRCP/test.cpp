#include "pch.h"
#include <gtest/gtest.h>
#include "../Fresh-Random-Civ-Picker_CPPGUI/Fresh-Random-Civ-Picker_CPPGUI.cpp"

TEST(CompletenessTest, AllCivsAreDrawn) {
	ResetProgram();

	for (int i = 0; i < CIVS_MAX; i++) {
		DrawCiv();
	}

	for (int i = 0; i < CIVS_MAX; i++) {
		EXPECT_EQ(size(civs), 0);
	}
}

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}