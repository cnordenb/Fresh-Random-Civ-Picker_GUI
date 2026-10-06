# Known issues

Latent bugs and quirks in Fresh Random Civ Picker, found by reviewing the code on 2026-10-05.
Line numbers refer to commit `10b2ab4` and will drift. When an issue is fixed, move it to **Fixed**.

## Tracked

### Behaviour
- **Mouse-toggling a DLC checkbox or an edition radio button doesn't trigger auto-reset**, while the equivalent
  hotkeys (A–H, Q/W/E) do. Their control ids fall outside the range at `FRCP_GUI.cpp:441`, and
  `ToggleDlc`/`SetEditionState` (`:443–444`) never reset.
- **F6 also quicksaves, and T is registered twice.** `hotkey[]` registers `VK_F6` with `HOTKEY_ID_F5`
  (`FRCP_GUI.h:61`; `HOTKEY_ID_F6` is unused and F6 isn't documented) and registers `T` at both `FRCP_GUI.h:55` and
  `:59`. Careful when fixing: `DisableHotkeys()` (`FRCP_GUI.cpp:1108`) unregisters ids `1..HOTKEY_AMOUNT-1`, so if
  `HOTKEY_AMOUNT` is simply decremented, the highest id (currently L, 39) would stay registered system-wide after
  FRCP loses focus.
- **Loading a save can drop the newest continuous-freshness entries.** `LoadLog` stops after `MAX_CIVS*2+4` (110)
  data lines (`FRCP_GUI.cpp:2083`, `:2086`). 53 civ states + drawn civs + the edition line + `ContfreshCivArray`
  exceeds that once roughly 30+ civs are drawn at maximum strength, and the array is written oldest first, so the
  most recent civs are the ones lost.
- **`ContFreshStrengthValue` defaults to 0** in `LoadSettings` (`FRCP_GUI.cpp:1884`) although the slider range is
  1–10 (`:717`). 0 behaves like 1 until the slider is moved.

### Latent (no visible effect today)
- `if (IDC_LEGACY_OPTION)` in `OptionsDlgProc` (`FRCP_GUI.cpp:759`) is always true; it should compare `wmId`. Any
  control's notification code equal to `CBN_DROPDOWN`/`CBN_SELCHANGE` is treated as coming from the jingle-type combo.
- `hOptionsDlg` is never assigned, so forwarding `WM_HOTKEY` to the Options dialog (`FRCP_GUI.cpp:419`) is dead code.
- `ConvertToString` (`FRCP_GUI.cpp:1119`) writes into a `reserve()`d but empty `std::string` (undefined behaviour
  that happens to work) and caps the output at `MAX_LENGTH` (15 bytes), so a civ name longer than 14 characters
  would be cut off or garbled in the draw label. Calling `SetWindowTextW` with `current_civ` would avoid both.
- The civ-checkbox range in `WM_COMMAND` (`wmId <= (MAX_CIVS + 5)`, `FRCP_GUI.cpp:432`) is off by one: id
  `MAX_CIVS + 5` would index past the end of `civ[]`.
- `LoadJingles` stores `StringCleaner(l_str)`, a pointer into a loop-local string, in `legacyJingle[j].name`
  (`FRCP_GUI.cpp:2799–2800`). It dangles after each iteration (nothing reads it later).
- The Save/Load preset dialogs set `nMaxFile = sizeof(szFile)`, which is bytes rather than characters
  (`FRCP_GUI.cpp:1949`, `:2042`). The Save dialog also passes `OFN_FILEMUSTEXIST` (`:1955`), an Open-dialog flag.
- `IsValidLobbyCode` validates `substr(12, 20)` for the `aoe2de://0/…` form (`FRCP_GUI.cpp:2578`), which skips the
  first digit of the code (the digits start at index 11).

### Content & docs
- History texts for Mapuche, Muisca, Tupi, Danes, Saxons and Varangians are missing from the local
  `Debug\Histories\` (the History dialog shows "History not found."). The game now ships them as
  `<civ>-utf8.txt` (e.g. `danes-utf8.txt`), so they must be copied and renamed to `<Civ>.txt`. Make sure the
  release zip includes them.
- The Hotkeys dialog (`IDD_HOTKEYS` in `FRCP_GUI.rc`) doesn't list **H – Open History** (Draw tab).
- The "GitHub Repository" menu item opens the old repo URL `…/Fresh-Random-Civ-Picker_CPPGUI`
  (`FRCP_GUI.cpp:486`). It works only through GitHub's rename redirect.
- Sound playback works poorly on slower computers (carried over from the README's known bugs).

### Tooling & housekeeping
- `UnitTesting_FRCP` no longer compiles: `test.cpp` calls `ResetProgram()`/`DrawCiv()` without arguments and uses
  the removed `civs` container. The solution builds it in every configuration, so **Build Solution fails**; build
  the `FRCP_GUI` project on its own.
- `Fresh-Random-Civ-Picker_CPPGUI/RCa23700` is a resource-compiler temp file that was committed by accident.
- Stale project files: `Fresh-Random-Civ-Picker_CPPGUI.filters` (Visual Studio template leftover); ~3,800
  duplicate `ClCompile` entries for non-existent `civ_icons\*.png` in `Fresh-Random-Civ-Picker_CPPGUI.vcxproj.filters`;
  `<Image>`/`<Text>` items in the `.vcxproj` that point at old `civ_icons\` paths and at a local Steam install.

## Fixed
- **Slavs checkbox doesn't trigger auto-reset.** `WM_COMMAND` only resets for control ids
  `wmId > 4 && wmId < 50 || wmId > 50 && wmId < 65` (`FRCP_GUI.cpp:441`), a range left over from when there were
  fewer civs. Civ checkbox ids are index + 5, so id 50 is whichever civ sits at index 45: Slavs since The Viking
  Sagas (Tatars before that). Fixed 2026-10-06.
- **DE DLC hotkeys J, K and L don't trigger auto-reset.** The Civ Pool tab's `WM_HOTKEY` reset range
  `wParam > 1 && wParam < 4 || wParam > 12 && wParam < 22` (`FRCP_GUI.cpp:307`) covers A–H but not `HOTKEY_ID_J` (37),
  `HOTKEY_ID_K` (38) or `HOTKEY_ID_L` (39), i.e. Dawn of the Dukes, Lords of the West and The Last Khans. Fixed 2026-10-06.