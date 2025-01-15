#include "pch.h"
#include "../Fresh-Random-Civ-Picker_CPPGUI/Fresh-Random-Civ-Picker_CPPGUI.cpp"

TEST(CompletenessTest, AllCivsAreDrawn) {
	reset();

	for (int i = 0; i < CIVS_MAX; i++) {
		draw_civ_logic();
	}

	for (int i = 0; i < CIVS_MAX; i++) {
		EXPECT_EQ(available[i], false);
	}
}

TEST(FreshnessTest, NoCivIsDrawnMoreThanOnce) {
	reset();
	for (int i = 0; i < CIVS_MAX; i++) {
		draw_civ_logic();
	}
	for (int i = 0; i < CIVS_MAX; i++) {
		EXPECT_EQ(times_drawn[i], 1);
	}
}

TEST(ExtraTest, NoOutOfBoundsError) {
	reset();
	for (int i = 0; i < 1000; i++) {
		draw_civ_logic();
		ASSERT_NE(isOutOfBounds, true);
	}
}

TEST(RandomnessTest, IsNotPredictable) {
	int predictable[CIVS_MAX];
	for (int i = 0; i < CIVS_MAX; i++) {
		predictable[i] = i;
	}
	int supposed_random[CIVS_MAX];
	for (int i = 0; i < CIVS_MAX; i++) {
		supposed_random[i] = result(CIVS_MAX);
	}
	int check = 0;
	for (int i = 0; i < CIVS_MAX; i++) {
		if (predictable[i] != supposed_random[i]) {
			check++;
		}
	}
	bool random_enough;
	if (check > 34) random_enough = true;				// arbitrarily decided criterion: at least 75% of cases do not match a sequential order
	else random_enough = false;
	EXPECT_EQ(random_enough, true);
}

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}