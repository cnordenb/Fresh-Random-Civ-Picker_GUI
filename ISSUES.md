# Known issues

Latent bugs and quirks in Fresh Random Civ Picker, found by reviewing the code on 2026-10-05.
Line numbers refer to commit `10b2ab4` and will drift. When an issue is fixed, move it to **Fixed**.

## Tracked

### Behaviour
- **Mouse-toggling a DLC checkbox or an edition radio button doesn't trigger auto-reset**, while the equivalent
  hotkeys (A–H, Q/W/E) do. Their control ids fall outside the range at `FRCP_GUI.cpp:441`, and
  `ToggleDlc`/`SetEditionState` (`:443–444`) never reset.
- **Options' two "When the program starts" radio buttons don't untick each other.** Clicking "Automatic civ pool
  logging" while "Reset civ pool on startup" is ticked leaves both ticked until Options is opened again (the setting
  itself changes correctly). A parchment radio button only ever ticks itself (`Checkbox::OnClicked`), and nothing
  unticks the other one after a click; `InitialiseOptionsDialog` calls `CheckRadioButton` only when the dialog opens.
  Found 2026-10-17 on the build before customisable hotkeys (a hidden-desktop click on IDC_RADIO_LOGGING: both
  report BST_CHECKED). The Customise Hotkeys dialog's section radio buttons untick each other with
  `CheckRadioButton`.

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
- `Fresh-Random-Civ-Picker_CPPGUI/RCa23700` is a resource-compiler temp file that was committed by accident.
- Stale project file: `Fresh-Random-Civ-Picker_CPPGUI.filters` (Visual Studio template leftover).

## Fixed
- **F6 also quicksaved, and T was registered twice.** The fixed table of hotkeys registered `VK_F6` under the
  quicksave hotkey's id (F6 was never documented) and registered `T` twice. With customisable hotkeys (2026-10-17)
  the table is gone: each hotkey has one key (`HotkeyBindings`), each keystroke is registered once, and every id is
  unregistered when FRCP loses focus. F6 no longer quicksaves unless the user gives Quicksave that key.
- **`UnitTesting_FRCP` didn't compile, so Build Solution failed.** The tests were rewritten with the model in the
  MVC split (2026-10-13), and the stale entries of the project files (duplicate `civ_icons\*.png` items in the
  `.vcxproj.filters`, `<Image>`/`<Text>` items for old paths) went with the data separation (2026-10-14). Checked
  2026-10-16: the solution builds as it stands (Debug, x64 and x86) and its 83 tests pass.
- **Slavs checkbox doesn't trigger auto-reset.** `WM_COMMAND` only resets for control ids
  `wmId > 4 && wmId < 50 || wmId > 50 && wmId < 65` (`FRCP_GUI.cpp:441`), a range left over from when there were
  fewer civs. Civ checkbox ids are index + 5, so id 50 is whichever civ sits at index 45: Slavs since The Viking
  Sagas (Tatars before that). Fixed 2026-10-06.
- **DE DLC hotkeys J, K and L don't trigger auto-reset.** The Civ Pool tab's `WM_HOTKEY` reset range
  `wParam > 1 && wParam < 4 || wParam > 12 && wParam < 22` (`FRCP_GUI.cpp:307`) covers A–H but not `HOTKEY_ID_J` (37),
  `HOTKEY_ID_K` (38) or `HOTKEY_ID_L` (39), i.e. Dawn of the Dukes, Lords of the West and The Last Khans. Fixed 2026-10-06.
- **A 10th DLC toggle made the program vanish at startup** (reported 2026-10-15: "When adding a civ, binary neither
  opens nor throws error message"; reproduced with the `x64\Release` build and its `data.json`, which adds a DLC "new"
  as the 10th DE toggle). With `MAX_DLC_TOGGLES` raised to 10, `CivPickerView::DlcTooltipText` appended
  `DLC_SLOT_HOTKEY_KEYS[9]`, a null pointer (only A to L exist), to a `std::wstring`: an access violation while the
  main window was built. Toggles after an edition's ninth now simply have no hotkey (`DLC_HOTKEY_COUNT`), an edition
  may have up to 128 (`MAX_DLC_TOGGLES`) and they scroll, and a startup that fails without crashing now says so in a
  message box. Fixed 2026-10-15.
- **Loading a save could drop the newest continuous-freshness entries**: the log reader stopped after
  `MAX_CIVS*2+4` lines, fewer than civ states + draws + edition + the freshness history, which is written oldest
  first. `LOG_LINE_LIMIT` now covers all of them (`CivPickerModel.cpp`). Fixed 2026-10-15.
- **`ContFreshStrengthValue` defaulted to 0** although the slider ran from 1 to 10. The strength is gone: the setting
  is now a number of civs (`ContFreshCivs`, default the most, as on the web version), and an old strength is turned
  into civs once. Fixed 2026-10-15.
