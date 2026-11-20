# Tests

This project strives to have sensible testing. The goal is not to have 100% test coverage but to have tests which _sufficiently_ ensure the functional completeness, functional correctness, maintainability, and accessibility of Fresh Random Civ Picker. For more guidelines on software quality and accessibility, see [ISO/IEC 25010:2011](https://cdn.standards.iteh.ai/samples/35733/2ca18b477b7845a5b8cae39d6de0c098/ISO-IEC-25010-2011.pdf), [ISO/IEC 25010:2023](https://cdn.standards.iteh.ai/samples/78176/13ff8ea97048443f99318920757df124/ISO-IEC-25010-2023.pdf) and [WCAG 2.2](https://www.w3.org/TR/WCAG22/).

This document lists each test, its purpose, and in which test file it resides. This document is an effort to mitigate redundant tests and ensure test quality. It is intended as an aid in the trouble-shooting of potential false positives.

Test descriptions must remain brief and parameters (eg. Purpose) may not exceed 120 characters. Tests are ordered in order of their execution.

The test report names each test by its number here, such as "Test 15 failed": a test's number is its place in the order
the tests run. So when a test is added, removed or moved, renumber this document to match.

<!--
Each test has an entry of this form, after the table of contents, in the order the tests run:

## Test N

### File residence:
`path/to/test_file`, lines 24-26

### Purpose:
Verify that ... (Req. 3, 4).

### Assertion expectation:
What must hold for the test to pass.
-->

## Table of contents per category

Functional completeness & correctness tests:

- [Test 1](#test-1): `HotkeyDefaultsTest.EveryHotkeyStartsWithTheKeyItAlwaysHad`, default keys are the old fixed keys
- [Test 2](#test-2): `HotkeyDefaultsTest.NoTwoDefaultsClash`, no two defaults clash
- [Test 3](#test-3): `HotkeyDefaultsTest.EachSectionHasItsHotkeys`, hotkeys per section
- [Test 4](#test-4): `HotkeyDefaultsTest.SettingNamesAreDistinctWords`, setting names are distinct words
- [Test 5](#test-5): `HotkeyActionTest.AKeystrokeMeansItsTabsHotkey`, a keystroke means the open tab's hotkey
- [Test 6](#test-6): `HotkeyActionTest.AGlobalKeystrokeMeansTheSameOnEveryTab`, global hotkeys work on every tab
- [Test 7](#test-7): `HotkeyActionTest.AKeystrokeWithoutAHotkeyMeansNothing`, an unbound keystroke does nothing
- [Test 8](#test-8): `HotkeyActionTest.HotkeysDataJsonGivesNothingToDoAreUnused`, hotkeys beyond the data are unused
- [Test 9](#test-9): `HotkeyActionTest.MoreDlcTogglesThanSlotsUseEverySlot`, more toggles than slots use every slot
- [Test 10](#test-10): `HotkeyActionTest.EditionAndDlcHotkeysHaveTheirPlaces`, edition and DLC hotkeys map to places
- [Test 11](#test-11): `HotkeyAssignTest.AFreeKeystrokeChangesOnlyItsHotkey`, a free keystroke changes one hotkey
- [Test 12](#test-12): `HotkeyAssignTest.AGlobalKeystrokeClashesWithEachTabsHotkey`, a global keystroke clashes on each tab
- [Test 13](#test-13): `HotkeyAssignTest.ATabsKeystrokeClashesWithTheGlobalOneAndItsOwnTabs`, a tab keystroke clashes with global and own tab
- [Test 14](#test-14): `HotkeyAssignTest.AHotkeysOwnKeystrokeAndNoKeyClashWithNothing`, own keystroke and no key never clash
- [Test 15](#test-15): `HotkeyAssignTest.AssigningTakesTheKeystrokeFromItsClashes`, assigning takes the key from clashes
- [Test 16](#test-16): `HotkeyAssignTest.AnUnusedHotkeyKeepsItsKeyAndTakesNone`, an unused hotkey blocks and takes no key
- [Test 17](#test-17): `HotkeyAssignTest.AKeystrokeThatCantBeAHotkeyIsRefused`, unusable keystrokes are refused
- [Test 18](#test-18): `HotkeyAssignTest.ClearingLeavesAHotkeyWithoutAKey`, clearing removes a hotkey's key
- [Test 19](#test-19): `HotkeyAssignTest.RestoringGivesEveryHotkeyItsDefault`, restoring gives the defaults back
- [Test 20](#test-20): `KeystrokeNameTest.ModifiersLeadInTheOrderCtrlAltShift`, modifier order in names
- [Test 21](#test-21): `KeystrokeNameTest.KeysHaveTheirUsualNames`, usual key names
- [Test 22](#test-22): `KeystrokeNameTest.EveryUsableKeystrokeIsReadBackFromItsName`, names read back to their keystrokes
- [Test 23](#test-23): `KeystrokeNameTest.NamesAreReadInAnyCase`, names are read in any case
- [Test 24](#test-24): `KeystrokeNameTest.OtherTextIsNoKeystroke`, other text is no keystroke
- [Test 25](#test-25): `KeystrokeNameTest.F12HasANameButCantBeAHotkey`, F12 is named but refused
- [Test 26](#test-26): `KeystrokeUseTest.WindowsKeepsSomeKeystrokes`, keystrokes Windows keeps
- [Test 27](#test-27): `KeystrokeUseTest.KeysWithoutANameCantBeHotkeys`, unnamed keys are no hotkeys
- [Test 28](#test-28): `KeystrokeUseTest.AModifierKeyAloneIsNoHotkey`, a modifier alone is no hotkey
- [Test 29](#test-29): `KeystrokeUseTest.OtherKeystrokesCanBeHotkeys`, ordinary keystrokes are usable
- [Test 30](#test-30): `ShareLink.StartsWithTheWebVersionsAddressAndFormatVersion`, link address and format version
- [Test 31](#test-31): `ShareLink.WritesAFullPoolAsSetBitsAndDrawsAfterADot`, a full pool's bits and draws
- [Test 32](#test-32): `ShareLink.OpensWhatItWritesForEveryEdition`, a written link opens to the same pool
- [Test 33](#test-33): `ShareLink.OpensACodeWithOrWithoutItsAddress`, a code opens with or without address
- [Test 34](#test-34): `ShareLink.TurnsOnTheCivsPastAShortBitfield`, civs past a short bitfield are on
- [Test 35](#test-35): `ShareLink.LeavesOutTheCivsOfNewerEditions`, no civs of newer editions
- [Test 36](#test-36): `ShareLink.RefusesTextThatIsNotALink`, text that is no link is refused
- [Test 37](#test-37): `ShareLink.RefusesTextLongerThanALinkCanBe`, overlong text is refused
- [Test 38](#test-38): `Json.FindsObjectMembersByKey`, members by key
- [Test 39](#test-39): `Json.WalksMembersAndElementsInOrder`, members and elements in order
- [Test 40](#test-40): `Json.ReadsEmptyContainers`, empty object and array
- [Test 41](#test-41): `Json.DecodesEscapes`, escapes in strings
- [Test 42](#test-42): `Json.DecodesUtf8IncludingCharactersBeyondTheBasicPlane`, UTF-8 of two to four bytes
- [Test 43](#test-43): `Json.RefusesMalformedUtf8InText`, malformed UTF-8 is refused
- [Test 44](#test-44): `Json.ReadsWholeNumbersOnly`, whole numbers only
- [Test 45](#test-45): `Json.TellsTypesApart`, value types
- [Test 46](#test-46): `Json.SkipsAByteOrderMark`, byte order mark is skipped
- [Test 47](#test-47): `Json.RefusesMalformedText`, malformed JSON is refused
- [Test 48](#test-48): `Json.RefusesNestingDeeperThanItsLimit`, nesting limit
- [Test 49](#test-49): `Json.RefusesMoreValuesThanItsLimit`, value limit
- [Test 50](#test-50): `Json.ReportsTheLineWhereItStopped`, line of a syntax error
- [Test 51](#test-51): `CivCatalog.LoadsTheProgramsDataFile`, `data.json` loads
- [Test 52](#test-52): `CivCatalog.EveryCivBelongsToItsDlcsEdition`, a civ's edition is its DLC's
- [Test 53](#test-53): `CivCatalog.EachDlcToggleKnowsItsPlace`, DLC toggles know their place
- [Test 54](#test-54): `CivCatalog.NamesTheIconFilesOfEveryEditionAndDlc`, picture files of editions and DLCs
- [Test 55](#test-55): `CivCatalog.TakesDlcTogglesBeyondTheNineWithHotkeys`, more than nine DLC toggles load
- [Test 56](#test-56): `CivCatalog.TakesAsManyDlcsAsItsLimitAndNoMore`, DLC limit
- [Test 57](#test-57): `CivCatalog.AnEditionHasTheCivsOfEveryOlderEdition`, editions include older editions' civs
- [Test 58](#test-58): `CivCatalog.ListsTheCivsInTheOrderOfTheirNames`, civs in order of name
- [Test 59](#test-59): `CivCatalog.NamesTheJinglesAndTechTreesOfEveryCiv`, civ file names and tech trees
- [Test 60](#test-60): `CivCatalog.ListsEveryEditionLinkOnce`, edition links listed once
- [Test 61](#test-61): `CivCatalog.FindsEditionsByTheirIds`, editions by id
- [Test 62](#test-62): `CivCatalog.RefusesAFileWithAProblem`, a faulty `data.json` is refused
- [Test 63](#test-63): `CivCatalog.RefusesTextThatIsNotJson`, a file that is not JSON is refused
- [Test 64](#test-64): `Rects.OverlapWhenTheyShareAPixel`, rectangles sharing a pixel overlap
- [Test 65](#test-65): `Rects.DoNotOverlapWhenTheyOnlyTouch`, touching rectangles do not overlap
- [Test 66](#test-66): `Rects.HaveAnAreaOnlyWhenWideAndTall`, area needs width and height
- [Test 67](#test-67): `Rects.BoundingRectCoversBoth`, bounding rectangle
- [Test 68](#test-68): `DottedLine.OddLengthDotsEveryOtherPixelFromBothEnds`, dots of an odd-length line
- [Test 69](#test-69): `DottedLine.EvenLengthDoublesTheFirstDotToReachTheLastPixel`, dots of an even-length line
- [Test 70](#test-70): `DottedLine.NoDotsOutsideTheLine`, no dots outside the line
- [Test 71](#test-71): `TileLayout.StretchCoversTheAreaWithOneTile`, stretch rule
- [Test 72](#test-72): `TileLayout.RepeatCentresWholeTilesInTheArea`, repeat rule
- [Test 73](#test-73): `TileLayout.RoundScalesTilesToFitAWholeNumber`, round rule
- [Test 74](#test-74): `TileLayout.RoundKeepsAtLeastOneTile`, round rule keeps one tile
- [Test 75](#test-75): `NineSlice.SplitsTheTargetIntoCornersEdgesAndMiddle`, nine-slice areas
- [Test 76](#test-76): `Scrollbar.KnobStartsAtTheTopAndEndsAtTheBottomOfTheTrack`, knob position
- [Test 77](#test-77): `Scrollbar.NoScrollingWhenTheContentFits`, no scrolling when content fits
- [Test 78](#test-78): `Scrollbar.ScrollTopIsClampedToTheContent`, scroll position is clamped
- [Test 79](#test-79): `Scrollbar.DraggingTheKnobScrollsInProportion`, knob drag scrolls in proportion
- [Test 80](#test-80): `Scrollbar.HitTestFindsButtonsTrackAndKnob`, scrollbar hit test
- [Test 81](#test-81): `Slider.FillRunsFromTheMinimumToTheMaximum`, slider fill
- [Test 82](#test-82): `Slider.ArrowsAndBarKeepTheImagesProportionsAtTheHandlesScale`, slider layout proportions
- [Test 83](#test-83): `Slider.HandleStopsWhereItTouchesAnArrow`, slider handle stops at arrows
- [Test 84](#test-84): `Slider.ValueAtFollowsThePointerAndIsClamped`, slider value at the pointer
- [Test 85](#test-85): `Slider.PointerFindsTheArrowsTheHandleAndTheBar`, slider hit test
- [Test 86](#test-86): `ContainedSize.KeepsTheImagesProportionsInTheBox`, fitting keeps proportions
- [Test 87](#test-87): `ContainedSize.EnlargesSmallImagesAndKeepsAPixel`, fitting enlarges and keeps a pixel
- [Test 88](#test-88): `WordWrap.BreaksAtTheLastSpaceThatFits`, wrap at the last space
- [Test 89](#test-89): `WordWrap.KeepsLineBreaksOfTheText`, line breaks are kept
- [Test 90](#test-90): `WordWrap.AWordLongerThanTheLineOverflowsOnItsOwnLine`, an overlong word overflows
- [Test 91](#test-91): `WordWrap.AnEmptyTextHasOneEmptyLine`, empty text is one empty line
- [Test 92](#test-92): `WordWrap.TheWidestLineIsMeasuredWithoutTrailingSpaces`, widest line without trailing spaces
- [Test 93](#test-93): `IconLabels.ALineThatStartsWithALabelGetsItsIcon`, a leading label gets its icon
- [Test 94](#test-94): `IconLabels.ALabelCountsOnlyAsWholeWordsAtTheLinesStart`, labels match whole words at the start
- [Test 95](#test-95): `IconLabels.TheLongestLabelWins`, the longest label wins
- [Test 96](#test-96): `IconLabels.RefusesEmptyOrOverlongLabelsAndCellsBelowZero`, bad labels are refused
- [Test 97](#test-97): `IconLabels.HoldsAFixedNumberOfLabels`, label capacity
- [Test 98](#test-98): `OpaqueRuns.FindsTheFullyOpaqueStretchesOfARow`, opaque runs of a row
- [Test 99](#test-99): `ColourFilter.GrayscaleUsesTheCssLuminanceWeights`, grayscale weights
- [Test 100](#test-100): `ColourFilter.BrightnessScalesAndClamps`, brightness scales and clamps
- [Test 101](#test-101): `ColourFilter.HueRotateByZeroAndSaturateByOneChangeNothing`, neutral hue and saturation
- [Test 102](#test-102): `Tooltip.SitsBelowRightOfTheCursor`, tooltip below right of the cursor
- [Test 103](#test-103): `Tooltip.MovesLeftAndAboveAtTheEdges`, tooltip at the edges
- [Test 104](#test-104): `Blur.SpreadsAnOpaqueBlockAndKeepsItsTotal`, blur spreads a block
- [Test 105](#test-105): `BoxShadow.KeepsTheBoxOpaqueAndShadesOnlyOutsideIt`, box shadow outside the box
- [Test 106](#test-106): `CompletenessTest.AllCivsAreDrawn`, every civ is drawn in a round
- [Test 107](#test-107): `EditionTest.SwitchingEditionLeavesItsCivs`, an edition's civs after switching
- [Test 108](#test-108): `EditionTest.DlcsAreEmptyOnlyWithoutEnabledCivs`, empty DLCs
- [Test 109](#test-109): `ShareTest.ALoadedPoolIsTheSharedOne`, a loaded pool is the shared one
- [Test 110](#test-110): `ShareTest.AnUndrawnCivIsNotShared`, an undrawn civ is not shared
- [Test 111](#test-111): `FreshnessTest.TheLastCivsOfARoundSitOutTheNextRoundsFirstDraws`, last civs sit out the next round's start
- [Test 112](#test-112): `FreshnessTest.TheMostFollowsThePoolsSize`, "the most" follows the pool
- [Test 113](#test-113): `HotkeyTest.TheModelsHotkeysFollowDataJson`, the model's hotkeys follow `data.json`

Accessibility tests:

Maintainability tests:

## Test 1

### File residence:
`UnitTesting_FRCP/tests/HotkeyBindingsTest.cpp`, lines 52-61

### Purpose:
Verify that every hotkey's default key is the key the program had before hotkeys could be changed.

### Assertion expectation:
Each of the 50 defaults has its name in `DEFAULT_KEYS`, and new bindings hold each default.

## Test 2

### File residence:
`UnitTesting_FRCP/tests/HotkeyBindingsTest.cpp`, lines 63-66

### Purpose:
Verify that no two default hotkeys clash.

### Assertion expectation:
With every hotkey in use, `ClashesWith` finds 0 clashes for each hotkey's own default keystroke.

## Test 3

### File residence:
`UnitTesting_FRCP/tests/HotkeyBindingsTest.cpp`, lines 68-77

### Purpose:
Verify that each hotkey belongs to the right section: global or one of the three tabs.

### Assertion expectation:
The scopes count 24 global, 6 draw tab, 4 log tab and 16 civ pool tab hotkeys.

## Test 4

### File residence:
`UnitTesting_FRCP/tests/HotkeyBindingsTest.cpp`, lines 79-92

### Purpose:
Verify that every hotkey's name in the settings file is one distinct word.

### Assertion expectation:
Each of the 50 setting names is non-empty, has only letters and digits, and no two are equal.

## Test 5

### File residence:
`UnitTesting_FRCP/tests/HotkeyBindingsTest.cpp`, lines 94-105

### Purpose:
Verify that a keystroke means the hotkey of the tab that is open.

### Assertion expectation:
Q, Space and L map to 7 expected actions on the draw, log and civ pool tabs (Q on civ pool: `edition_1`).

## Test 6

### File residence:
`UnitTesting_FRCP/tests/HotkeyBindingsTest.cpp`, lines 107-117

### Purpose:
Verify that a global hotkey's keystroke means the same on every tab.

### Assertion expectation:
On each of the 3 tabs F1 is `open_options`, Ctrl+Z is `undo` and Esc is `quit`.

## Test 7

### File residence:
`UnitTesting_FRCP/tests/HotkeyBindingsTest.cpp`, lines 119-126

### Purpose:
Verify that a keystroke that no hotkey has means nothing.

### Assertion expectation:
On the draw tab W, Shift+Q and no keystroke all give `HotkeyAction::none`.

## Test 8

### File residence:
`UnitTesting_FRCP/tests/HotkeyBindingsTest.cpp`, lines 128-139

### Purpose:
Verify that edition and DLC hotkeys beyond what the data holds are unused.

### Assertion expectation:
With 2 editions and 3 DLC toggles: `edition_3`, `dlc_slot_4` are unused, E and F do nothing, D is `dlc_slot_3`.

## Test 9

### File residence:
`UnitTesting_FRCP/tests/HotkeyBindingsTest.cpp`, lines 141-147

### Purpose:
Verify that more DLC toggles than hotkey slots use every slot.

### Assertion expectation:
With 1 edition and `MAX_DLC_TOGGLES` toggles: `dlc_slot_9` is used, `edition_2` is not.

## Test 10

### File residence:
`UnitTesting_FRCP/tests/HotkeyBindingsTest.cpp`, lines 149-162

### Purpose:
Verify that edition and DLC hotkeys map to their places and back.

### Assertion expectation:
`EditionOf`/`DlcSlotOf` invert `EditionHotkey`/`DlcSlotHotkey`; out of range gives `none` or `NO_PLACE`.

## Test 11

### File residence:
`UnitTesting_FRCP/tests/HotkeyBindingsTest.cpp`, lines 164-174

### Purpose:
Verify that assigning a free keystroke changes only that hotkey.

### Assertion expectation:
Draw-tab draw moves to D; Space is free on the draw tab; log-tab Space and civ-pool D are unchanged.

## Test 12

### File residence:
`UnitTesting_FRCP/tests/HotkeyBindingsTest.cpp`, lines 176-186

### Purpose:
Verify that a keystroke for a global hotkey clashes with each tab's hotkey on that keystroke.

### Assertion expectation:
Q for `toggle_civ_name` has 3 clashes: `draw_tab_open_options`, `clear_drawn_log`, `edition_1`.

## Test 13

### File residence:
`UnitTesting_FRCP/tests/HotkeyBindingsTest.cpp`, lines 188-201

### Purpose:
Verify that a tab hotkey's keystroke clashes with global hotkeys and its own tab's, not other tabs'.

### Assertion expectation:
For `clear_drawn_log`: F1 clashes with `open_options`, W with `toggle_remaining_log`; W on the draw tab: 0.

## Test 14

### File residence:
`UnitTesting_FRCP/tests/HotkeyBindingsTest.cpp`, lines 203-209

### Purpose:
Verify that a hotkey's own keystroke and no keystroke clash with nothing.

### Assertion expectation:
`ClashesWith(open_options, F1)` and `ClashesWith(open_options, NO_KEYSTROKE)` both count 0.

## Test 15

### File residence:
`UnitTesting_FRCP/tests/HotkeyBindingsTest.cpp`, lines 211-222

### Purpose:
Verify that assigning a keystroke takes it from the hotkeys it clashes with.

### Assertion expectation:
After Q goes to `toggle_civ_name`, its 3 former owners have no key, Q means it on all tabs, no clash is left.

## Test 16

### File residence:
`UnitTesting_FRCP/tests/HotkeyBindingsTest.cpp`, lines 224-234

### Purpose:
Verify that an unused hotkey neither blocks its key nor can be given one.

### Assertion expectation:
With 2 editions: E goes to `toggle_auto_reset` without clash; `edition_3` refuses Y and still holds E.

## Test 17

### File residence:
`UnitTesting_FRCP/tests/HotkeyBindingsTest.cpp`, lines 236-246

### Purpose:
Verify that a keystroke that cannot be a hotkey is refused.

### Assertion expectation:
6 keystrokes (none, F12, Alt+Tab, `;`, Shift, Win+Q) are refused, `undo` stays Ctrl+Z; `none` takes no key.

## Test 18

### File residence:
`UnitTesting_FRCP/tests/HotkeyBindingsTest.cpp`, lines 248-257

### Purpose:
Verify that clearing a hotkey leaves it without a key.

### Assertion expectation:
After `Clear(draw_tab_draw_civ)` it holds `NO_KEYSTROKE`; Space and no keystroke do nothing on the draw tab.

## Test 19

### File residence:
`UnitTesting_FRCP/tests/HotkeyBindingsTest.cpp`, lines 259-269

### Purpose:
Verify that restoring defaults gives every hotkey its default key.

### Assertion expectation:
After an assignment and a clearing, `RestoreDefaults` makes all 50 hotkeys equal `DefaultKeystrokeOf`.

## Test 20

### File residence:
`UnitTesting_FRCP/tests/HotkeyBindingsTest.cpp`, lines 271-277

### Purpose:
Verify that a keystroke's name lists its modifiers in the order Ctrl, Alt, Shift.

### Assertion expectation:
The names are "Ctrl+Shift+S", "Ctrl+Alt+Shift+Backspace" and "Alt+Num +".

## Test 21

### File residence:
`UnitTesting_FRCP/tests/HotkeyBindingsTest.cpp`, lines 279-287

### Purpose:
Verify that keys have their usual names.

### Assertion expectation:
The names are "7", "Num 5", "F11", "PgDn", "None" for no keystroke, and "" for the unnamed `;` key.

## Test 22

### File residence:
`UnitTesting_FRCP/tests/HotkeyBindingsTest.cpp`, lines 289-304

### Purpose:
Verify that every usable keystroke is read back from its own name.

### Assertion expectation:
All 596 usable keystrokes (77 keys x 8 modifier sets - 20) satisfy `ParseKeystroke(NameKeystroke(k)) == k`.

## Test 23

### File residence:
`UnitTesting_FRCP/tests/HotkeyBindingsTest.cpp`, lines 306-314

### Purpose:
Verify that keystroke names are read in any letter case.

### Assertion expectation:
"ctrl+SHIFT+num 5" reads as Ctrl+Shift+Numpad 5, and "None" reads as `NO_KEYSTROKE`.

## Test 24

### File residence:
`UnitTesting_FRCP/tests/HotkeyBindingsTest.cpp`, lines 316-322

### Purpose:
Verify that text that is not a keystroke's name is refused.

### Assertion expectation:
`ParseKeystroke` fails for 10 texts, such as "", "Ctrl+", "Shift+Ctrl+S", "F13", "Ctrl+S " and "Win+S".

## Test 25

### File residence:
`UnitTesting_FRCP/tests/HotkeyBindingsTest.cpp`, lines 324-332

### Purpose:
Verify that F12 is read from its name but cannot be made a hotkey.

### Assertion expectation:
"F12" parses to the F12 key; assigning it to `open_options` fails, and `open_options` keeps F1.

## Test 26

### File residence:
`UnitTesting_FRCP/tests/HotkeyBindingsTest.cpp`, lines 334-347

### Purpose:
Verify that keystrokes Windows keeps for itself cannot be hotkeys.

### Assertion expectation:
9 keystrokes, such as F12, Alt+Tab, Alt+F4, Ctrl+Esc and Ctrl+Alt+Delete, check as `kept_by_windows`.

## Test 27

### File residence:
`UnitTesting_FRCP/tests/HotkeyBindingsTest.cpp`, lines 349-354

### Purpose:
Verify that a key without a name cannot be a hotkey.

### Assertion expectation:
The `;` key, no keystroke and Win+Q check as `unnamed_key`.

## Test 28

### File residence:
`UnitTesting_FRCP/tests/HotkeyBindingsTest.cpp`, lines 356-361

### Purpose:
Verify that a modifier key alone cannot be a hotkey.

### Assertion expectation:
Shift, Ctrl and Alt alone each check as `unnamed_key`.

## Test 29

### File residence:
`UnitTesting_FRCP/tests/HotkeyBindingsTest.cpp`, lines 363-370

### Purpose:
Verify that ordinary keystrokes can be hotkeys.

### Assertion expectation:
Q, Ctrl+Tab, Shift+Esc, Ctrl+F4, Backspace and Ctrl+Delete check as `usable`.

## Test 30

### File residence:
`UnitTesting_FRCP/tests/ShareLinkTest.cpp`, lines 52-57

### Purpose:
Verify that a share link starts with the web version's address and the format version.

### Assertion expectation:
The link for edition 0 starts with "https://frcp.net/#p=", then "1", then "A".

## Test 31

### File residence:
`UnitTesting_FRCP/tests/ShareLinkTest.cpp`, lines 59-77

### Purpose:
Verify that a link writes a full pool as set bits and the draws after a dot.

### Assertion expectation:
Per edition with every civ: address, "1", edition character, all "_" bits; 2 draws add "." and 2 characters.

## Test 32

### File residence:
`UnitTesting_FRCP/tests/ShareLinkTest.cpp`, lines 79-89

### Purpose:
Verify that opening a written link gives back the same pool, for every edition.

### Assertion expectation:
For each edition, `Open` of the written link succeeds with the same edition, enabled civs and 3 draws.

## Test 33

### File residence:
`UnitTesting_FRCP/tests/ShareLinkTest.cpp`, lines 91-104

### Purpose:
Verify that a share code opens with or without its address.

### Assertion expectation:
6 forms (bare code, "p=", "#p=", padded link, another address, "%31" escape) open to the same pool.

## Test 34

### File residence:
`UnitTesting_FRCP/tests/ShareLinkTest.cpp`, lines 106-114

### Purpose:
Verify that civs past the end of a short bitfield are turned on.

### Assertion expectation:
Bitfield "A" for edition 0: civs 0 to 5 are off, every later civ of the edition is on, 0 draws.

## Test 35

### File residence:
`UnitTesting_FRCP/tests/ShareLinkTest.cpp`, lines 116-128

### Purpose:
Verify that a link never enables civs of editions newer than its own.

### Assertion expectation:
For each edition, an all-set bitfield enables exactly the civs with `InEdition(civ, edition)`.

## Test 36

### File residence:
`UnitTesting_FRCP/tests/ShareLinkTest.cpp`, lines 130-149

### Purpose:
Verify that text that is not a valid link is refused.

### Assertion expectation:
`Open` fails for 21 texts: empty, wrong version, unknown edition or civ, bad characters, bad or repeated draws.

## Test 37

### File residence:
`UnitTesting_FRCP/tests/ShareLinkTest.cpp`, lines 151-158

### Purpose:
Verify that text longer than a link can be is refused.

### Assertion expectation:
A link after 100 spaces opens; the same link after `MAX_INPUT_LENGTH` spaces is refused.

## Test 38

### File residence:
`UnitTesting_FRCP/tests/CivDataTest.cpp`, lines 29-43

### Purpose:
Verify that the JSON reader finds an object's members by key.

### Assertion expectation:
Root is an object of 3 members: "first" is 1, "third" is "x", "fourth" is `NONE`; the inner array is true, null.

## Test 39

### File residence:
`UnitTesting_FRCP/tests/CivDataTest.cpp`, lines 45-57

### Purpose:
Verify that the JSON reader gives members and elements in the order of the text.

### Assertion expectation:
Keys are "a" then "c", a third is `NONE`; "a" has 3 elements, its second has 2; "b" is 4 and "c" is 5.

## Test 40

### File residence:
`UnitTesting_FRCP/tests/CivDataTest.cpp`, lines 59-64

### Purpose:
Verify that the JSON reader reads empty objects and arrays.

### Assertion expectation:
`{}` and `[]` each have `Count` 0.

## Test 41

### File residence:
`UnitTesting_FRCP/tests/CivDataTest.cpp`, lines 66-72

### Purpose:
Verify that the JSON reader decodes escapes in strings.

### Assertion expectation:
The escapes for quote, backslash, slash, tab, line feed and `ö` decode to their characters.

## Test 42

### File residence:
`UnitTesting_FRCP/tests/CivDataTest.cpp`, lines 74-78

### Purpose:
Verify that the JSON reader decodes UTF-8 of two, three and four bytes.

### Assertion expectation:
The bytes for U+00F6, U+20AC and U+1F600 decode to those characters, the last as a surrogate pair.

## Test 43

### File residence:
`UnitTesting_FRCP/tests/CivDataTest.cpp`, lines 80-85

### Purpose:
Verify that the JSON reader refuses malformed UTF-8 in a string.

### Assertion expectation:
`Text` fails for the byte sequences C3 28 and E0 80 80.

## Test 44

### File residence:
`UnitTesting_FRCP/tests/CivDataTest.cpp`, lines 87-97

### Purpose:
Verify that the JSON reader reads whole numbers only, within its range.

### Assertion expectation:
12 and -7 are read and 0 is ok; 1.5, 1e3 and 1234567890 are refused.

## Test 45

### File residence:
`UnitTesting_FRCP/tests/CivDataTest.cpp`, lines 99-111

### Purpose:
Verify that the JSON reader tells value types apart.

### Assertion expectation:
`{}`, `[]`, `""`, `0`, `false` are object, array, string, number, literal; a string is no integer, a number no text.

## Test 46

### File residence:
`UnitTesting_FRCP/tests/CivDataTest.cpp`, lines 113-117

### Purpose:
Verify that the JSON reader skips a UTF-8 byte order mark.

### Assertion expectation:
`{"a": 1}` after the bytes EF BB BF parses, and "a" is 1.

## Test 47

### File residence:
`UnitTesting_FRCP/tests/CivDataTest.cpp`, lines 119-131

### Purpose:
Verify that the JSON reader refuses malformed JSON.

### Assertion expectation:
22 malformed texts each give `syntax_error` and leave `Root()` as `NONE`.

## Test 48

### File residence:
`UnitTesting_FRCP/tests/CivDataTest.cpp`, lines 133-139

### Purpose:
Verify that the JSON reader refuses nesting deeper than its limit.

### Assertion expectation:
`MAX_DEPTH` nested arrays parse as `ok`; one more gives `too_deep`.

## Test 49

### File residence:
`UnitTesting_FRCP/tests/CivDataTest.cpp`, lines 141-147

### Purpose:
Verify that the JSON reader refuses more values than its limit.

### Assertion expectation:
An array of `MAX_VALUES` numbers, one value too many with the array itself, gives `too_many_values`.

## Test 50

### File residence:
`UnitTesting_FRCP/tests/CivDataTest.cpp`, lines 149-154

### Purpose:
Verify that the JSON reader reports the line of a syntax error.

### Assertion expectation:
A trailing comma before `}` on line 3 gives `syntax_error` and `ErrorLine()` 3.

## Test 51

### File residence:
`UnitTesting_FRCP/tests/CivDataTest.cpp`, lines 156-164

### Purpose:
Verify that the program's `data.json` loads into the catalog.

### Assertion expectation:
`Load` succeeds with an empty error, and the edition, DLC and civ counts are above 0.

## Test 52

### File residence:
`UnitTesting_FRCP/tests/CivDataTest.cpp`, lines 166-179

### Purpose:
Verify that every civ belongs to the edition of its DLC.

### Assertion expectation:
Each civ's DLC index is valid, its edition and legacy jingle match the DLC's edition, and it is `InEdition`.

## Test 53

### File residence:
`UnitTesting_FRCP/tests/CivDataTest.cpp`, lines 181-203

### Purpose:
Verify that each DLC toggle knows its edition and slot.

### Assertion expectation:
Each toggle's DLC holds that edition and slot; the DLCs with a toggle edition number as many as the toggles.

## Test 54

### File residence:
`UnitTesting_FRCP/tests/CivDataTest.cpp`, lines 205-219

### Purpose:
Verify that every edition and DLC names its picture files (Rule 98).

### Assertion expectation:
Each edition has an icon and a banner; a DLC's icon is empty exactly when its `icon_index` is `NONE`.

## Test 55

### File residence:
`UnitTesting_FRCP/tests/CivDataTest.cpp`, lines 246-258

### Purpose:
Prevent a failure with more than nine DLC toggles in an edition (regression: a 10th once crashed startup).

### Assertion expectation:
`data.json` with 10 DLC toggles added to edition 0 loads; the count grows by 10, each toggle's slot is its index.

## Test 56

### File residence:
`UnitTesting_FRCP/tests/CivDataTest.cpp`, lines 260-271

### Purpose:
Enforce that the catalog takes `MAX_DLCS` DLCs and refuses more with a message (Rule 99).

### Assertion expectation:
Filling up to `MAX_DLCS` loads with that count; one more fails, and the error contains the limit's number.

## Test 57

### File residence:
`UnitTesting_FRCP/tests/CivDataTest.cpp`, lines 273-289

### Purpose:
Verify that an edition has every civ of each edition that is not newer than it.

### Assertion expectation:
For each pair by year, the older's civ count is not greater and each of its civs is `InEdition` of the newer.

## Test 58

### File residence:
`UnitTesting_FRCP/tests/CivDataTest.cpp`, lines 291-304

### Purpose:
Verify that the catalog lists the civs once each, in the order of their names.

### Assertion expectation:
`DisplayCiv` gives every civ once, and each `file_name` is greater than the one before.

## Test 59

### File residence:
`UnitTesting_FRCP/tests/CivDataTest.cpp`, lines 306-317

### Purpose:
Verify that a civ's file name is its name and that every civ has a tech tree (Rule 98).

### Assertion expectation:
For each civ `file_name` equals `name` ignoring case, with the same length, and `tech_tree` is not empty.

## Test 60

### File residence:
`UnitTesting_FRCP/tests/CivDataTest.cpp`, lines 319-334

### Purpose:
Verify that the catalog lists every edition's links once, in order.

### Assertion expectation:
`LinkAt` gives each edition's link URLs in turn, and `LinkCount` is their sum.

## Test 61

### File residence:
`UnitTesting_FRCP/tests/CivDataTest.cpp`, lines 336-343

### Purpose:
Verify that editions are found by their ids.

### Assertion expectation:
`FindEdition` of each edition's id gives its index; "no such edition" gives `NONE`.

## Test 62

### File residence:
`UnitTesting_FRCP/tests/CivDataTest.cpp`, lines 345-368

### Purpose:
Enforce that a `data.json` with a problem is refused with a message that says what is wrong (Rule 99).

### Assertion expectation:
11 files, each `data.json` with one change, fail to load, with the expected words in the error and 0 civs.

## Test 63

### File residence:
`UnitTesting_FRCP/tests/CivDataTest.cpp`, lines 370-376

### Purpose:
Enforce that a `data.json` that is not JSON is refused with a message (Rule 99).

### Assertion expectation:
The first half of `data.json` fails to load, and the error contains "not valid JSON".

## Test 64

### File residence:
`UnitTesting_FRCP/tests/ParchmentGeometryTest.cpp`, lines 7-11

### Purpose:
Verify that rectangles overlap when they share a pixel.

### Assertion expectation:
(0,0,10,10) overlaps (9,9,20,20) and (2,2,4,4).

## Test 65

### File residence:
`UnitTesting_FRCP/tests/ParchmentGeometryTest.cpp`, lines 13-17

### Purpose:
Verify that rectangles that only touch at an edge do not overlap.

### Assertion expectation:
(0,0,10,10) overlaps neither (10,0,20,10) nor (0,10,10,20).

## Test 66

### File residence:
`UnitTesting_FRCP/tests/ParchmentGeometryTest.cpp`, lines 19-26

### Purpose:
Verify that a rectangle has an area only when it has both width and height.

### Assertion expectation:
A 1x1 pixel and a 0.5x0.5 float rectangle have an area; zero-wide and inverted rectangles have none.

## Test 67

### File residence:
`UnitTesting_FRCP/tests/ParchmentGeometryTest.cpp`, lines 28-35

### Purpose:
Verify that the bounding rectangle covers both rectangles.

### Assertion expectation:
The bounds of (5,20,10,30) and (0,25,8,40) are (0,20,10,40).

## Test 68

### File residence:
`UnitTesting_FRCP/tests/ParchmentGeometryTest.cpp`, lines 37-44

### Purpose:
Verify that a dotted line of odd length has a dot on every other pixel, from both ends.

### Assertion expectation:
Length 5: dots at 0, 2 and 4, none at 1 and 3.

## Test 69

### File residence:
`UnitTesting_FRCP/tests/ParchmentGeometryTest.cpp`, lines 46-54

### Purpose:
Verify that a dotted line of even length doubles its first dot so that the last pixel has a dot.

### Assertion expectation:
Length 424: dots at 0, 1, 3 and 423, none at 2 and 422.

## Test 70

### File residence:
`UnitTesting_FRCP/tests/ParchmentGeometryTest.cpp`, lines 56-60

### Purpose:
Verify that a dotted line has no dots outside its length.

### Assertion expectation:
Length 10: no dot at -1 or at 10.

## Test 71

### File residence:
`UnitTesting_FRCP/tests/ParchmentGeometryTest.cpp`, lines 62-67

### Purpose:
Verify that the stretch rule covers the area with one tile.

### Assertion expectation:
Area 40, tile 6: origin 0, length 40.

## Test 72

### File residence:
`UnitTesting_FRCP/tests/ParchmentGeometryTest.cpp`, lines 69-74

### Purpose:
Verify that the repeat rule centres whole tiles in the area.

### Assertion expectation:
Area 40, tile 6: length 6, origin -1.

## Test 73

### File residence:
`UnitTesting_FRCP/tests/ParchmentGeometryTest.cpp`, lines 76-81

### Purpose:
Verify that the round rule scales tiles so that a whole number of them fits.

### Assertion expectation:
Area 100, tile 38: origin 0, length 100/3.

## Test 74

### File residence:
`UnitTesting_FRCP/tests/ParchmentGeometryTest.cpp`, lines 83-87

### Purpose:
Verify that the round rule keeps at least one tile.

### Assertion expectation:
Area 10, tile 38: length 10.

## Test 75

### File residence:
`UnitTesting_FRCP/tests/ParchmentGeometryTest.cpp`, lines 89-100

### Purpose:
Verify that a nine-slice splits its target into corners, edges and a middle.

### Assertion expectation:
Target (10,20,110,70), border 6: area 0 ends at (16,26), area 4 is (16,26,104,64), area 8 has left 104, bottom 70.

## Test 76

### File residence:
`UnitTesting_FRCP/tests/ParchmentGeometryTest.cpp`, lines 102-107

### Purpose:
Verify that the scrollbar's knob runs from the top to the bottom of its track with the scroll position.

### Assertion expectation:
Content 1000, view 400, track 375: knob offset 0 at scroll 0, 354 at 600 and 177 at 300.

## Test 77

### File residence:
`UnitTesting_FRCP/tests/ParchmentGeometryTest.cpp`, lines 109-114

### Purpose:
Verify that content that fits needs no scrolling.

### Assertion expectation:
Content 300 in view 400: maximum scroll 0 and no scrollbar; content 401 needs one.

## Test 78

### File residence:
`UnitTesting_FRCP/tests/ParchmentGeometryTest.cpp`, lines 116-120

### Purpose:
Verify that the scroll position is kept within the content.

### Assertion expectation:
Content 1000, view 400: 900 becomes 600 and -5 becomes 0.

## Test 79

### File residence:
`UnitTesting_FRCP/tests/ParchmentGeometryTest.cpp`, lines 122-126

### Purpose:
Verify that dragging the knob scrolls in proportion and no further than the end.

### Assertion expectation:
Maximum scroll 600 over 354 px: a 177 px drag from 0 gives 300; a 1000 px drag from 300 gives 600.

## Test 80

### File residence:
`UnitTesting_FRCP/tests/ParchmentGeometryTest.cpp`, lines 128-135

### Purpose:
Verify that a point on the scrollbar is found as a button, the track or the knob.

### Assertion expectation:
5 points on a 429 px bar with the knob at 100: up button, down button, track above, knob, track below.

## Test 81

### File residence:
`UnitTesting_FRCP/tests/ParchmentGeometryTest.cpp`, lines 137-142

### Purpose:
Verify that the slider's fill runs from 0 at the minimum to 1 at the maximum.

### Assertion expectation:
Range 1 to 10: fill 0 at 1, 1 at 10 and 1/3 at 4.

## Test 82

### File residence:
`UnitTesting_FRCP/tests/ParchmentGeometryTest.cpp`, lines 144-153

### Purpose:
Verify that the slider's arrows and bar keep the images' proportions at the handle's scale.

### Assertion expectation:
192x24: handle 24; at scale 24/43 the arrows are 35 wide and 30 tall, the bar 15 tall and centred at 12.

## Test 83

### File residence:
`UnitTesting_FRCP/tests/ParchmentGeometryTest.cpp`, lines 155-160

### Purpose:
Verify that the slider's handle stops where it touches an arrow.

### Assertion expectation:
At value 1 the handle's left edge is the left arrow's right; at 10 its right edge is the right arrow's left.

## Test 84

### File residence:
`UnitTesting_FRCP/tests/ParchmentGeometryTest.cpp`, lines 162-169

### Purpose:
Verify that the slider's value follows the pointer and stays in its range.

### Assertion expectation:
The handle's centre for 7 gives 7, x 89 gives 5, x 0 gives 1 and x 191 gives 10.

## Test 85

### File residence:
`UnitTesting_FRCP/tests/ParchmentGeometryTest.cpp`, lines 171-180

### Purpose:
Verify that a point on the slider is found as an arrow, the handle or the bar.

### Assertion expectation:
x 5 is the left arrow, 185 the right arrow, 31 the handle, 100 the bar; -1 and 192 are `none`.

## Test 86

### File residence:
`UnitTesting_FRCP/tests/ParchmentGeometryTest.cpp`, lines 182-190

### Purpose:
Verify that an image keeps its proportions when fitted into a box.

### Assertion expectation:
In 128x128: 128x128 is 128 wide, 400x300 becomes 128x96 and 300x400 becomes 96x128.

## Test 87

### File residence:
`UnitTesting_FRCP/tests/ParchmentGeometryTest.cpp`, lines 192-197

### Purpose:
Verify that fitting enlarges a small image and never makes a side less than one pixel.

### Assertion expectation:
52x52 in 104x104 is 104 wide; 1000x1 in 128x128 is 1 high; 128x93 in 128x93 is 93 high.

## Test 88

### File residence:
`UnitTesting_FRCP/tests/ParchmentGeometryTest.cpp`, lines 216-226

### Purpose:
Verify that text wraps at the last space that fits.

### Assertion expectation:
"aaa bbb ccc" at 10 per character in width 75: 2 lines, (start 0, length 7) and (start 8, length 3).

## Test 89

### File residence:
`UnitTesting_FRCP/tests/ParchmentGeometryTest.cpp`, lines 228-236

### Purpose:
Verify that wrapping keeps the text's own line breaks.

### Assertion expectation:
"ab", a line feed, "cd" in width 500: 2 lines, the first of length 2, the second starting at 3.

## Test 90

### File residence:
`UnitTesting_FRCP/tests/ParchmentGeometryTest.cpp`, lines 238-246

### Purpose:
Verify that a word longer than the line overflows on a line of its own.

### Assertion expectation:
"abcdefgh ij" in width 50: 2 lines, the first of length 8, the second starting at 9.

## Test 91

### File residence:
`UnitTesting_FRCP/tests/ParchmentGeometryTest.cpp`, lines 248-254

### Purpose:
Verify that an empty text wraps to one empty line.

### Assertion expectation:
`Wrap` of "" succeeds with 1 line of length 0.

## Test 92

### File residence:
`UnitTesting_FRCP/tests/ParchmentGeometryTest.cpp`, lines 256-262

### Purpose:
Verify that the widest line is measured without its trailing spaces.

### Assertion expectation:
"aaa bbb ccc" at 10 per character in width 75: `WidestLine()` is 70.

## Test 93

### File residence:
`UnitTesting_FRCP/tests/ParchmentGeometryTest.cpp`, lines 264-271

### Purpose:
Verify that a line that starts with a label gets the label's icon.

### Assertion expectation:
With "Alpha" at cell 3: "Alpha (1/9)", "Alpha" and "Alpha" before a line break all give 3.

## Test 94

### File residence:
`UnitTesting_FRCP/tests/ParchmentGeometryTest.cpp`, lines 273-281

### Purpose:
Verify that a label counts only as whole words at the start of the line.

### Assertion expectation:
With "Alpha" at cell 3: "Alphas (1/9)", " Alpha", "Alph" and "" all give `NO_ICON`.

## Test 95

### File residence:
`UnitTesting_FRCP/tests/ParchmentGeometryTest.cpp`, lines 283-290

### Purpose:
Verify that the longest matching label wins.

### Assertion expectation:
With "Alpha" at 3 and "Alpha Beta" at 5: "Alpha Beta (2/9)" gives 5 and "Alpha Gamma" gives 3.

## Test 96

### File residence:
`UnitTesting_FRCP/tests/ParchmentGeometryTest.cpp`, lines 292-301

### Purpose:
Verify that empty or overlong labels and cells below zero are refused.

### Assertion expectation:
`Add` fails for "", a label of `MAX_ICON_LABEL_LENGTH` + 1 and cell -1; "Alpha" then gives `NO_ICON`.

## Test 97

### File residence:
`UnitTesting_FRCP/tests/ParchmentGeometryTest.cpp`, lines 303-308

### Purpose:
Verify that the labels hold a fixed number and refuse one more.

### Assertion expectation:
`MAX_ICON_LABELS` labels are added; the next `Add` fails.

## Test 98

### File residence:
`UnitTesting_FRCP/tests/ParchmentGeometryTest.cpp`, lines 310-320

### Purpose:
Verify that the fully opaque stretches of a row of alpha values are found.

### Assertion expectation:
Alpha 0,255,255,128,255,255,255,0 gives 2 runs: 1 to 3 and 4 to 7.

## Test 99

### File residence:
`UnitTesting_FRCP/tests/ParchmentGeometryTest.cpp`, lines 322-328

### Purpose:
Verify that grayscale uses the CSS luminance weights.

### Assertion expectation:
Pure red (255,0,0) becomes (54,54,54).

## Test 100

### File residence:
`UnitTesting_FRCP/tests/ParchmentGeometryTest.cpp`, lines 330-337

### Purpose:
Verify that brightness scales each channel and clamps at 255.

### Assertion expectation:
(200,100,10) at 0.88 is (176,88,9); red 200 at 2.0 is 255.

## Test 101

### File residence:
`UnitTesting_FRCP/tests/ParchmentGeometryTest.cpp`, lines 339-345

### Purpose:
Verify that a hue rotation of 0 and a saturation of 1 change nothing.

### Assertion expectation:
(120,60,30) stays (120,60,30).

## Test 102

### File residence:
`UnitTesting_FRCP/tests/ParchmentGeometryTest.cpp`, lines 347-352

### Purpose:
Verify that a tooltip sits below and to the right of the cursor.

### Assertion expectation:
Cursor (100,100), tooltip 80x40 in a 1000x800 area: placed at (110,120).

## Test 103

### File residence:
`UnitTesting_FRCP/tests/ParchmentGeometryTest.cpp`, lines 354-359

### Purpose:
Verify that a tooltip moves left of and above the cursor at the area's edges.

### Assertion expectation:
Cursor (980,790), tooltip 80x40 in a 1000x800 area: placed at (916,742).

## Test 104

### File residence:
`UnitTesting_FRCP/tests/ParchmentGeometryTest.cpp`, lines 361-372

### Purpose:
Verify that a blur spreads an opaque block outward.

### Assertion expectation:
8x8 block in a 32x32 mask, radius 3: the centre is between 0 and 255, 3 px outside is above 0, the corner is 0.

## Test 105

### File residence:
`UnitTesting_FRCP/tests/ParchmentGeometryTest.cpp`, lines 374-384

### Purpose:
Verify that a box shadow keeps the box opaque and shades only outside it.

### Assertion expectation:
20x20, box (5,5,15,15), offset (2,2): the box pixel is 0xff336699, (16,16) is black with alpha, (0,0) is 0.

## Test 106

### File residence:
`UnitTesting_FRCP/tests/test.cpp`, lines 10-28

### Purpose:
Verify that every civ is drawn once before any is drawn again.

### Assertion expectation:
After as many draws as civs: 0 remain, every pool entry is "" and every civ has been drawn.

## Test 107

### File residence:
`UnitTesting_FRCP/tests/test.cpp`, lines 30-47

### Purpose:
Verify that switching edition enables exactly that edition's civs.

### Assertion expectation:
For each edition: `GetEdition` is it, the maximum is its civ count, a civ is enabled exactly when `InEdition`.

## Test 108

### File residence:
`UnitTesting_FRCP/tests/test.cpp`, lines 49-64

### Purpose:
Verify that a DLC is empty only when none of its civs is enabled.

### Assertion expectation:
Each DLC is empty when disabled; when enabled it is empty only if no civ belongs to it.

## Test 109

### File residence:
`UnitTesting_FRCP/tests/test.cpp`, lines 66-99

### Purpose:
Verify that loading a shared pool gives the model the pool that was shared.

### Assertion expectation:
Loaded edition, enabled civs and 3 draws equal the shared; iterator 3, current civ the last draw, custom state.

## Test 110

### File residence:
`UnitTesting_FRCP/tests/test.cpp`, lines 101-112

### Purpose:
Verify that an undrawn civ is not shared.

### Assertion expectation:
After 4 draws and 1 undo the shared pool has 3 draws, the last being the current civ.

## Test 111

### File residence:
`UnitTesting_FRCP/tests/test.cpp`, lines 114-137

### Purpose:
Verify that the last civs of a round sit out the first draws of the next round.

### Assertion expectation:
With 5 sitting out: draw d (0 to 4) of the next round is none of the previous round's last 5 - d civs.

## Test 112

### File residence:
`UnitTesting_FRCP/tests/test.cpp`, lines 139-156

### Purpose:
Verify that the freshness setting "the most" follows the pool's size.

### Assertion expectation:
Maximum is half the pool; set to it, the setting is `CONTFRESH_MOST`, the limit half each edition's pool; 1 stays 1.

## Test 113

### File residence:
`UnitTesting_FRCP/tests/test.cpp`, lines 158-179

### Purpose:
Enforce that the model's edition and DLC hotkeys follow `data.json` (Rule 98).

### Assertion expectation:
An edition hotkey is used when its edition exists, a DLC slot when below the most toggles; `cycle_tabs` is Tab.
