#include "pch.h"

#include <set>
#include <string>

#include "HotkeyBindings.cpp"

// The program's hotkeys: the keys they start with, what a keystroke does on each tab, changing them as the
// Customise Hotkeys dialog does, and their names in settings.ini.

namespace
{
    constexpr HotkeyScope TABS[] = { HotkeyScope::draw_tab, HotkeyScope::log_tab, HotkeyScope::civ_pool_tab };
    constexpr uint8_t MODIFIER_SETS[] = { 0,
                                          MODIFIER_CONTROL,
                                          MODIFIER_ALT,
                                          MODIFIER_SHIFT,
                                          MODIFIER_CONTROL | MODIFIER_ALT,
                                          MODIFIER_CONTROL | MODIFIER_SHIFT,
                                          MODIFIER_ALT | MODIFIER_SHIFT,
                                          MODIFIER_CONTROL | MODIFIER_ALT | MODIFIER_SHIFT };

    // The keys the program had before they could be changed, in the Hotkeys dialog's order.
    const wchar_t* const DEFAULT_KEYS[HOTKEY_ACTION_COUNT] = {
        L"Tab", L"1", L"2", L"3", L"F1", L"F2", L"F3", L"F4", L"F5", L"F9", L"Ctrl+S", L"Ctrl+R", L"P", L"U",
        L"Ctrl+F", L"Ctrl+Z", L"Ctrl+X", L"Ctrl+T", L"Z", L"X", L"C", L"V", L"B", L"Esc",
        L"Space", L"Enter", L"Q", L"R", L"T", L"H",
        L"Space", L"Enter", L"Q", L"W",
        L"Space", L"Enter", L"Q", L"W", L"E", L"R", L"T", L"A", L"S", L"D", L"F", L"G", L"H", L"J", L"K", L"L" };
    constexpr uint8_t KEY_F4 = KEY_F1 + 3;
    constexpr uint8_t KEY_NUMPAD_5 = KEY_NUMPAD_0 + 5;
    constexpr uint8_t KEY_SEMICOLON = 0xBA; // a punctuation key, which depends on the keyboard's layout
    constexpr uint8_t MODIFIER_WINDOWS = 0x8;

    HotkeyBindings AllInUse() { return HotkeyBindings(MAX_EDITIONS, DLC_HOTKEY_COUNT); }

    HotkeyAction ActionAt(int index) { return static_cast<HotkeyAction>(index); }

    std::wstring NameOf(Keystroke keystroke) { return NameKeystroke(keystroke).characters; }

    Keystroke Key(uint8_t key) { return { 0, key }; }

    Keystroke Held(uint8_t modifiers, uint8_t key) { return { modifiers, key }; }

    // Every pair of hotkeys that can work at the same time has two keystrokes, or at least one without a key.
    bool NoClashAnywhere(const HotkeyBindings& hotkeys)
    {
        for (int i = 0; i < HOTKEY_ACTION_COUNT; i++)
        {
            if (!hotkeys.IsUsed(ActionAt(i)) || hotkeys.KeystrokeOf(ActionAt(i)) == NO_KEYSTROKE) continue;
            if (hotkeys.ClashesWith(ActionAt(i), hotkeys.KeystrokeOf(ActionAt(i))).count != 0) return false;
        }
        return true;
    }
}

TEST(HotkeyDefaultsTest, EveryHotkeyStartsWithTheKeyItAlwaysHad)
{
    const HotkeyBindings hotkeys = AllInUse();

    for (int i = 0; i < HOTKEY_ACTION_COUNT; i++)
    {
        EXPECT_EQ(NameOf(HotkeyBindings::DefaultKeystrokeOf(ActionAt(i))), DEFAULT_KEYS[i]) << i;
        EXPECT_TRUE(hotkeys.KeystrokeOf(ActionAt(i)) == HotkeyBindings::DefaultKeystrokeOf(ActionAt(i))) << i;
    }
}

TEST(HotkeyDefaultsTest, NoTwoDefaultsClash)
{
    EXPECT_TRUE(NoClashAnywhere(AllInUse()));
}

TEST(HotkeyDefaultsTest, EachSectionHasItsHotkeys)
{
    int counts[HOTKEY_SCOPE_COUNT] = {};
    for (int i = 0; i < HOTKEY_ACTION_COUNT; i++) counts[static_cast<int>(HotkeyBindings::ScopeOf(ActionAt(i)))]++;

    EXPECT_EQ(counts[static_cast<int>(HotkeyScope::global)], 24);
    EXPECT_EQ(counts[static_cast<int>(HotkeyScope::draw_tab)], 6);
    EXPECT_EQ(counts[static_cast<int>(HotkeyScope::log_tab)], 4);
    EXPECT_EQ(counts[static_cast<int>(HotkeyScope::civ_pool_tab)], 16);
}

TEST(HotkeyDefaultsTest, SettingNamesAreDistinctWords)
{
    std::set<std::wstring> names;
    for (int i = 0; i < HOTKEY_ACTION_COUNT; i++)
    {
        const std::wstring name = HotkeyBindings::SettingNameOf(ActionAt(i));
        EXPECT_FALSE(name.empty()) << i;
        EXPECT_EQ(name.find_first_not_of(L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"),
                  std::wstring::npos)
            << i;
        names.insert(name);
    }
    EXPECT_EQ(static_cast<int>(names.size()), HOTKEY_ACTION_COUNT);
}

TEST(HotkeyActionTest, AKeystrokeMeansItsTabsHotkey)
{
    const HotkeyBindings hotkeys = AllInUse();

    EXPECT_EQ(hotkeys.ActionFor(Key('Q'), HotkeyScope::draw_tab), HotkeyAction::draw_tab_open_options);
    EXPECT_EQ(hotkeys.ActionFor(Key('Q'), HotkeyScope::log_tab), HotkeyAction::clear_drawn_log);
    EXPECT_EQ(hotkeys.ActionFor(Key('Q'), HotkeyScope::civ_pool_tab), HotkeyAction::edition_1);
    EXPECT_EQ(hotkeys.ActionFor(Key(KEY_SPACE), HotkeyScope::draw_tab), HotkeyAction::draw_tab_draw_civ);
    EXPECT_EQ(hotkeys.ActionFor(Key(KEY_SPACE), HotkeyScope::log_tab), HotkeyAction::log_tab_draw_civ);
    EXPECT_EQ(hotkeys.ActionFor(Key(KEY_SPACE), HotkeyScope::civ_pool_tab), HotkeyAction::enable_all_civs);
    EXPECT_EQ(hotkeys.ActionFor(Key('L'), HotkeyScope::civ_pool_tab), HotkeyAction::dlc_slot_9);
}

TEST(HotkeyActionTest, AGlobalKeystrokeMeansTheSameOnEveryTab)
{
    const HotkeyBindings hotkeys = AllInUse();

    for (HotkeyScope tab : TABS)
    {
        EXPECT_EQ(hotkeys.ActionFor(Key(KEY_F1), tab), HotkeyAction::open_options);
        EXPECT_EQ(hotkeys.ActionFor(Held(MODIFIER_CONTROL, 'Z'), tab), HotkeyAction::undo);
        EXPECT_EQ(hotkeys.ActionFor(Key(KEY_ESCAPE), tab), HotkeyAction::quit);
    }
}

TEST(HotkeyActionTest, AKeystrokeWithoutAHotkeyMeansNothing)
{
    const HotkeyBindings hotkeys = AllInUse();

    EXPECT_EQ(hotkeys.ActionFor(Key('W'), HotkeyScope::draw_tab), HotkeyAction::none);
    EXPECT_EQ(hotkeys.ActionFor(Held(MODIFIER_SHIFT, 'Q'), HotkeyScope::draw_tab), HotkeyAction::none);
    EXPECT_EQ(hotkeys.ActionFor(NO_KEYSTROKE, HotkeyScope::draw_tab), HotkeyAction::none);
}

TEST(HotkeyActionTest, HotkeysDataJsonGivesNothingToDoAreUnused)
{
    // Two editions, and at most three DLC toggles in an edition.
    const HotkeyBindings hotkeys(2, 3);

    EXPECT_TRUE(hotkeys.IsUsed(HotkeyAction::edition_2));
    EXPECT_FALSE(hotkeys.IsUsed(HotkeyAction::edition_3));
    EXPECT_TRUE(hotkeys.IsUsed(HotkeyAction::dlc_slot_3));
    EXPECT_FALSE(hotkeys.IsUsed(HotkeyAction::dlc_slot_4));
    EXPECT_EQ(hotkeys.ActionFor(Key('E'), HotkeyScope::civ_pool_tab), HotkeyAction::none);
    EXPECT_EQ(hotkeys.ActionFor(Key('F'), HotkeyScope::civ_pool_tab), HotkeyAction::none);
    EXPECT_EQ(hotkeys.ActionFor(Key('D'), HotkeyScope::civ_pool_tab), HotkeyAction::dlc_slot_3);
}

TEST(HotkeyActionTest, MoreDlcTogglesThanSlotsUseEverySlot)
{
    const HotkeyBindings hotkeys(1, MAX_DLC_TOGGLES);

    EXPECT_TRUE(hotkeys.IsUsed(HotkeyAction::dlc_slot_9));
    EXPECT_FALSE(hotkeys.IsUsed(HotkeyAction::edition_2));
}

TEST(HotkeyActionTest, EditionAndDlcHotkeysHaveTheirPlaces)
{
    for (int edition = 0; edition < MAX_EDITIONS; edition++)
        EXPECT_EQ(HotkeyBindings::EditionOf(HotkeyBindings::EditionHotkey(edition)), edition);
    for (int slot = 0; slot < DLC_HOTKEY_COUNT; slot++)
        EXPECT_EQ(HotkeyBindings::DlcSlotOf(HotkeyBindings::DlcSlotHotkey(slot)), slot);

    EXPECT_EQ(HotkeyBindings::EditionHotkey(0), HotkeyAction::edition_1);
    EXPECT_EQ(HotkeyBindings::DlcSlotHotkey(DLC_HOTKEY_COUNT - 1), HotkeyAction::dlc_slot_9);
    EXPECT_EQ(HotkeyBindings::EditionHotkey(MAX_EDITIONS), HotkeyAction::none);
    EXPECT_EQ(HotkeyBindings::DlcSlotHotkey(-1), HotkeyAction::none);
    EXPECT_EQ(HotkeyBindings::EditionOf(HotkeyAction::dlc_slot_1), HotkeyBindings::NO_PLACE);
    EXPECT_EQ(HotkeyBindings::DlcSlotOf(HotkeyAction::toggle_auto_reset), HotkeyBindings::NO_PLACE);
}

TEST(HotkeyAssignTest, AFreeKeystrokeChangesOnlyItsHotkey)
{
    HotkeyBindings hotkeys = AllInUse();

    EXPECT_TRUE(hotkeys.Assign(HotkeyAction::draw_tab_draw_civ, Key('D')));

    EXPECT_EQ(hotkeys.ActionFor(Key('D'), HotkeyScope::draw_tab), HotkeyAction::draw_tab_draw_civ);
    EXPECT_EQ(hotkeys.ActionFor(Key(KEY_SPACE), HotkeyScope::draw_tab), HotkeyAction::none);
    EXPECT_EQ(hotkeys.ActionFor(Key(KEY_SPACE), HotkeyScope::log_tab), HotkeyAction::log_tab_draw_civ);
    EXPECT_EQ(hotkeys.ActionFor(Key('D'), HotkeyScope::civ_pool_tab), HotkeyAction::dlc_slot_3);
}

TEST(HotkeyAssignTest, AGlobalKeystrokeClashesWithEachTabsHotkey)
{
    const HotkeyBindings hotkeys = AllInUse();

    const HotkeyBindings::Clashes clashes = hotkeys.ClashesWith(HotkeyAction::toggle_civ_name, Key('Q'));

    ASSERT_EQ(clashes.count, 3);
    EXPECT_EQ(clashes.actions[0], HotkeyAction::draw_tab_open_options);
    EXPECT_EQ(clashes.actions[1], HotkeyAction::clear_drawn_log);
    EXPECT_EQ(clashes.actions[2], HotkeyAction::edition_1);
}

TEST(HotkeyAssignTest, ATabsKeystrokeClashesWithTheGlobalOneAndItsOwnTabs)
{
    const HotkeyBindings hotkeys = AllInUse();

    const HotkeyBindings::Clashes global = hotkeys.ClashesWith(HotkeyAction::clear_drawn_log, Key(KEY_F1));
    const HotkeyBindings::Clashes same_tab = hotkeys.ClashesWith(HotkeyAction::clear_drawn_log, Key('W'));
    const HotkeyBindings::Clashes other_tab = hotkeys.ClashesWith(HotkeyAction::draw_tab_open_options, Key('W'));

    ASSERT_EQ(global.count, 1);
    EXPECT_EQ(global.actions[0], HotkeyAction::open_options);
    ASSERT_EQ(same_tab.count, 1);
    EXPECT_EQ(same_tab.actions[0], HotkeyAction::toggle_remaining_log);
    EXPECT_EQ(other_tab.count, 0);
}

TEST(HotkeyAssignTest, AHotkeysOwnKeystrokeAndNoKeyClashWithNothing)
{
    const HotkeyBindings hotkeys = AllInUse();

    EXPECT_EQ(hotkeys.ClashesWith(HotkeyAction::open_options, Key(KEY_F1)).count, 0);
    EXPECT_EQ(hotkeys.ClashesWith(HotkeyAction::open_options, NO_KEYSTROKE).count, 0);
}

TEST(HotkeyAssignTest, AssigningTakesTheKeystrokeFromItsClashes)
{
    HotkeyBindings hotkeys = AllInUse();

    EXPECT_TRUE(hotkeys.Assign(HotkeyAction::toggle_civ_name, Key('Q')));

    EXPECT_TRUE(hotkeys.KeystrokeOf(HotkeyAction::draw_tab_open_options) == NO_KEYSTROKE);
    EXPECT_TRUE(hotkeys.KeystrokeOf(HotkeyAction::clear_drawn_log) == NO_KEYSTROKE);
    EXPECT_TRUE(hotkeys.KeystrokeOf(HotkeyAction::edition_1) == NO_KEYSTROKE);
    for (HotkeyScope tab : TABS) EXPECT_EQ(hotkeys.ActionFor(Key('Q'), tab), HotkeyAction::toggle_civ_name);
    EXPECT_TRUE(NoClashAnywhere(hotkeys));
}

TEST(HotkeyAssignTest, AnUnusedHotkeyKeepsItsKeyAndTakesNone)
{
    HotkeyBindings hotkeys(2, DLC_HOTKEY_COUNT);

    EXPECT_EQ(hotkeys.ClashesWith(HotkeyAction::toggle_auto_reset, Key('E')).count, 0);
    EXPECT_TRUE(hotkeys.Assign(HotkeyAction::toggle_auto_reset, Key('E')));
    EXPECT_FALSE(hotkeys.Assign(HotkeyAction::edition_3, Key('Y')));

    EXPECT_TRUE(hotkeys.KeystrokeOf(HotkeyAction::edition_3) == Key('E'));
    EXPECT_EQ(hotkeys.ActionFor(Key('E'), HotkeyScope::civ_pool_tab), HotkeyAction::toggle_auto_reset);
}

TEST(HotkeyAssignTest, AKeystrokeThatCantBeAHotkeyIsRefused)
{
    HotkeyBindings hotkeys = AllInUse();
    const Keystroke refused[] = { NO_KEYSTROKE, Key(KEY_F12), Held(MODIFIER_ALT, KEY_TAB), Key(KEY_SEMICOLON),
                                  Key(KEY_SHIFT), Held(MODIFIER_WINDOWS, 'Q') };

    for (const Keystroke& keystroke : refused) EXPECT_FALSE(hotkeys.Assign(HotkeyAction::undo, keystroke));

    EXPECT_TRUE(hotkeys.KeystrokeOf(HotkeyAction::undo) == Held(MODIFIER_CONTROL, 'Z'));
    EXPECT_FALSE(hotkeys.Assign(HotkeyAction::none, Key('Y')));
}

TEST(HotkeyAssignTest, ClearingLeavesAHotkeyWithoutAKey)
{
    HotkeyBindings hotkeys = AllInUse();

    hotkeys.Clear(HotkeyAction::draw_tab_draw_civ);

    EXPECT_TRUE(hotkeys.KeystrokeOf(HotkeyAction::draw_tab_draw_civ) == NO_KEYSTROKE);
    EXPECT_EQ(hotkeys.ActionFor(Key(KEY_SPACE), HotkeyScope::draw_tab), HotkeyAction::none);
    EXPECT_EQ(hotkeys.ActionFor(NO_KEYSTROKE, HotkeyScope::draw_tab), HotkeyAction::none);
}

TEST(HotkeyAssignTest, RestoringGivesEveryHotkeyItsDefault)
{
    HotkeyBindings hotkeys = AllInUse();
    EXPECT_TRUE(hotkeys.Assign(HotkeyAction::toggle_civ_name, Key('Q')));
    hotkeys.Clear(HotkeyAction::quit);

    hotkeys.RestoreDefaults();

    for (int i = 0; i < HOTKEY_ACTION_COUNT; i++)
        EXPECT_TRUE(hotkeys.KeystrokeOf(ActionAt(i)) == HotkeyBindings::DefaultKeystrokeOf(ActionAt(i))) << i;
}

TEST(KeystrokeNameTest, ModifiersLeadInTheOrderCtrlAltShift)
{
    EXPECT_EQ(NameOf(Held(MODIFIER_SHIFT | MODIFIER_CONTROL, 'S')), L"Ctrl+Shift+S");
    EXPECT_EQ(NameOf(Held(MODIFIER_SHIFT | MODIFIER_ALT | MODIFIER_CONTROL, KEY_BACKSPACE)),
              L"Ctrl+Alt+Shift+Backspace");
    EXPECT_EQ(NameOf(Held(MODIFIER_ALT, KEY_ADD)), L"Alt+Num +");
}

TEST(KeystrokeNameTest, KeysHaveTheirUsualNames)
{
    EXPECT_EQ(NameOf(Key('7')), L"7");
    EXPECT_EQ(NameOf(Key(KEY_NUMPAD_5)), L"Num 5");
    EXPECT_EQ(NameOf(Key(KEY_F11)), L"F11");
    EXPECT_EQ(NameOf(Key(KEY_PAGE_DOWN)), L"PgDn");
    EXPECT_EQ(NameOf(NO_KEYSTROKE), L"None");
    EXPECT_EQ(NameOf(Key(KEY_SEMICOLON)), L"");
}

TEST(KeystrokeNameTest, EveryUsableKeystrokeIsReadBackFromItsName)
{
    int usable = 0;
    for (int key = 1; key <= 0xFE; key++)
    {
        for (uint8_t modifiers : MODIFIER_SETS)
        {
            const Keystroke keystroke = Held(modifiers, static_cast<uint8_t>(key));
            if (CheckKeystroke(keystroke) != KeystrokeUse::usable) continue;
            usable++;
            const ReadKeystroke read = ParseKeystroke(NameKeystroke(keystroke).characters);
            EXPECT_TRUE(read.ok && read.keystroke == keystroke) << NameOf(keystroke).c_str();
        }
    }
    // 26 letters, 10 digits, 11 function keys, 10 number pad digits and 20 other keys, each with the 8 sets of
    // modifiers, less the 20 Windows keeps: Tab, F4 and Space with the 4 sets that hold Alt, Esc with the 6 that
    // hold Alt or Ctrl, and Delete with the 2 that hold Ctrl and Alt.
    EXPECT_EQ(usable, 77 * 8 - 20);
}

TEST(KeystrokeNameTest, NamesAreReadInAnyCase)
{
    const ReadKeystroke read = ParseKeystroke(L"ctrl+SHIFT+num 5");

    EXPECT_TRUE(read.ok);
    EXPECT_TRUE(read.keystroke == Held(MODIFIER_CONTROL | MODIFIER_SHIFT, KEY_NUMPAD_5));
    EXPECT_TRUE(ParseKeystroke(L"None").ok);
    EXPECT_TRUE(ParseKeystroke(L"None").keystroke == NO_KEYSTROKE);
}

TEST(KeystrokeNameTest, OtherTextIsNoKeystroke)
{
    const wchar_t* const texts[] = { L"", L"Ctrl+", L"Ctrl", L"Shift+Ctrl+S", L"Ctrl+Ctrl+S", L"S+Ctrl", L"F13",
                                     L"Ctrl+S ", L"Spacebar", L"Win+S" };

    for (const wchar_t* text : texts) EXPECT_FALSE(ParseKeystroke(text).ok) << text;
}

TEST(KeystrokeNameTest, F12HasANameButCantBeAHotkey)
{
    HotkeyBindings hotkeys = AllInUse();
    const ReadKeystroke read = ParseKeystroke(L"F12");

    EXPECT_TRUE(read.ok && read.keystroke == Key(KEY_F12));
    EXPECT_FALSE(hotkeys.Assign(HotkeyAction::open_options, read.keystroke));
    EXPECT_TRUE(hotkeys.KeystrokeOf(HotkeyAction::open_options) == Key(KEY_F1));
}

TEST(KeystrokeUseTest, WindowsKeepsSomeKeystrokes)
{
    const Keystroke kept[] = { Key(KEY_F12),
                               Held(MODIFIER_CONTROL, KEY_F12),
                               Held(MODIFIER_ALT, KEY_TAB),
                               Held(MODIFIER_ALT | MODIFIER_SHIFT, KEY_TAB),
                               Held(MODIFIER_ALT, KEY_ESCAPE),
                               Held(MODIFIER_CONTROL, KEY_ESCAPE),
                               Held(MODIFIER_ALT, KEY_F4),
                               Held(MODIFIER_ALT, KEY_SPACE),
                               Held(MODIFIER_CONTROL | MODIFIER_ALT, KEY_DELETE) };

    for (const Keystroke& keystroke : kept) EXPECT_EQ(CheckKeystroke(keystroke), KeystrokeUse::kept_by_windows);
}

TEST(KeystrokeUseTest, KeysWithoutANameCantBeHotkeys)
{
    EXPECT_EQ(CheckKeystroke(Key(KEY_SEMICOLON)), KeystrokeUse::unnamed_key);
    EXPECT_EQ(CheckKeystroke(NO_KEYSTROKE), KeystrokeUse::unnamed_key);
    EXPECT_EQ(CheckKeystroke(Held(MODIFIER_WINDOWS, 'Q')), KeystrokeUse::unnamed_key);
}

TEST(KeystrokeUseTest, AModifierKeyAloneIsNoHotkey)
{
    const uint8_t modifier_keys[] = { KEY_SHIFT, KEY_CONTROL, KEY_ALT };

    for (uint8_t key : modifier_keys) EXPECT_EQ(CheckKeystroke(Key(key)), KeystrokeUse::unnamed_key) << key;
}

TEST(KeystrokeUseTest, OtherKeystrokesCanBeHotkeys)
{
    const Keystroke usable[] = { Key('Q'), Held(MODIFIER_CONTROL, KEY_TAB), Held(MODIFIER_SHIFT, KEY_ESCAPE),
                                 Held(MODIFIER_CONTROL, KEY_F4), Key(KEY_BACKSPACE),
                                 Held(MODIFIER_CONTROL, KEY_DELETE) };

    for (const Keystroke& keystroke : usable) EXPECT_EQ(CheckKeystroke(keystroke), KeystrokeUse::usable);
}
