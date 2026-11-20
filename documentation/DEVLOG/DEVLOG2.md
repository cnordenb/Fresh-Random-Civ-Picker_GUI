# Development log, part 2

- [2026-10-16](#2026-10-16)
- [2026-10-17](#2026-10-17)
- [2026-10-18](#2026-10-18)
- [2026-11-20](#2026-11-20)

Part 2 of the development log. It follows [part 1](DEVLOG1.md). The decisions made in the log are listed in
[DECISIONS.md](../DECISIONS.md).

## 2026-10-16

### Content never under the skin

#### What changed

- **Why.** A piece of parchment over the drawn log lagged behind the window as it moved. The burnt edge of the
  original parchment reaches into the content: a notch over the drawn log (x 32 to 44, y 292 to 323, alpha 228 to
  254), a smaller one at the right edge of the remaining log, the sides of the client frame and the top right
  corner of the close button. Those pixels were not fully opaque, so they fell outside the window's region and the
  skin showed them, and the skin follows the window a moment late (it moves in `WM_WINDOWPOSCHANGED`).
- **Underlay.** `BuildMainShell` lays the parchment on an opaque underlay (`CONTENT_UNDERLAY_COLOUR`, black) under
  the title row, the menu bar and the client area, so all of the content is the window's own. Where the edge
  reaches in, it shows a little darker, like the backdrop the controls already copied.
- **Checked.** On the Log tab, pixels of the content outside the window's region: 885 before (278 in the drawn
  log, 38 in the remaining log), 0 after. Zero warnings at `/W4` (Debug and Release, x64), zero `/analyze`
  findings, and all 78 unit tests passed.

#### Not done

- The burnt edge outside the content still belongs to the skin and still trails a moving window.

### Civ icons in the logs

#### What changed

- **Why.** The web version's logs show each civ's icon (`drawnRows`, `fillLog`, `.log-row`); the Log tab was to
  show them too.
- **Drawn log.** An entry reads "<civ> (<k>/<n>)" with its icon before it, as "[icon] Celts (1/56)"; it read
  "<k>/<n> - <civ>". The remaining log's text is unchanged. Only what is shown changed: `log.txt` stores names,
  and the drawn log is rebuilt from it.
- **`ParchmentLog::ShowIcons`** turns a log's icons on. A line that starts with one of the labels as whole words
  is led by that label's cell of the atlas, 18 px, centred in a line of 20 px, the text 4 px after it. The rest
  of a wrapped line has none; the text keeps one column, so it lines up. Scrolling steps a line of 20 px.
- **Labels.** They are the civs' names from the catalog, each with its own cell
  (`CivPickerView::ShowCivIconsInLogs`). `IconLabels` in `ParchmentGeometry` keeps copies and finds the longest
  label a line starts with. Copying text from a log gives the text alone.
- **History dialog.** Its log keeps its lines of 16.2 px and has no icons; the web version has no such log.
- **Tests.** 83 unit tests, 5 of them new: a label leads a line, only as whole words at its start, the longest
  wins, bad labels are refused, a fixed capacity.
- **Checked.** In a window 578 px wide the rows sit where the web version's do (row 1 of the drawn log at x 46,
  y 144, rows 20 px apart; the remaining log at x 363), measured in headless Chrome against a copy of `frcp_html`
  with a probe. Icon and text baseline match in a capture side by side. Zero warnings at `/W4` (Debug x64 and
  Win32, Release x64), zero `/analyze` findings, and all 83 unit tests passed.

#### Not done

- `ParchmentLog.cpp` is now 528 lines, past *Clean Code*'s desirable 500.

### Open dropdown list framed as on the web version

#### What changed

- **Why.** An open dropdown in Options showed a blue outline. The list (the combobox's `ComboLBox` popup, with
  `WS_BORDER`) had the theme's frame: #7a7a7a, #171717 while focused and #0078d7 while hot, which it is under the
  mouse whenever an item is picked.
- **Frame.** `StyleDropdown` now also subclasses the list (`DropdownListProc`). After the system paints the
  non-client area, a long list's scrollbar included, the frame of 1 px is painted over in `MENU_BORDER_COLOUR`
  (#3d2c1c), as the web version's open list has, on `WM_NCPAINT` and in `WM_PRINT`.
- **Checked.** On a hidden desktop, with Options' jingle dropdown and the History dialog's long list opened by a
  posted click: the frame reads #3d2c1c from the list's window DC and in `PrintWindow`, #7a7a7a before; the long
  list keeps its scrollbar. The theme's colours for each state were read with `DrawThemeBackground` (Listbox,
  `LBCP_BORDER_NOSCROLL`). Zero warnings at `/W4` (Debug and Release, x64), zero `/analyze` findings, and all 83
  unit tests passed.

#### Not done

- The hot, blue look itself could not be produced: a hidden desktop has no pointer.

### Goddard C++ standard adopted, and the code brought in line

#### What changed

- **Rules.** A fourth source was added to `CLAUDE2.md`: the *C++ Coding Standards and Style Guide* of NASA
  Goddard Space Flight Center, as modified for the GMAT project (the draft of 2005-05-24, `mab583.pdf` in
  `materials/`). The log calls it the Goddard standard. It ranks below every earlier rule.
- **How the code was checked.** Every C++ file of the program and the tests: by a scratch checker for the
  mechanical rules (spacing, switches, includes, classes, comments, conditionals, nesting), by clang-tidy 22
  (implicit conversions in conditions, uninitialized variables, parameter names, virtual destructors, large
  parameters by value, unused includes), and by reading.
- **Switches.** A default case in the 16 that had none.
- **Conditions.** 7 implicit tests of pointers, handles and integers made explicit.
- **Initialization.** 5 buffers, `MSG`, `SYSTEMTIME`, `OptionsEvent` and a local array initialized where they are
  declared; `Civ`'s fields and `CivPickerModel::available_civs` given member initializers. Four declarations
  renamed their parameters to match the definitions.
- **Classes.** `final` on `CivPickerController`, `CivPickerModel`, `WrappedLines`, `IconLabels`, `OpaqueRuns`,
  `ShadowBlur` and `BoxShadow`; virtual destructors on the two base classes, `ParchmentControl` and
  `CivPickerObserver`. Every virtual method of `ParchmentControl` is overridden somewhere.
- **Constants.** Of the 52 `#define`s in `GlobalConstants.h`, 45 are `constexpr` now and 7 unused ones were
  deleted (`DT_UNDERLINE`, `MAX_LENGTH`, `MIN_WIDTH`, `MIN_HEIGHT`, `BUTTON_WIDTH`, `BUTTON_HEIGHT`,
  `HOTKEY_ID_F6`).
- **Standard library.** 36 hand-written minimums and maximums, and the helper `Larger` with its 4 uses, became
  `std::min` and `std::max`. `NOMINMAX` is defined in both project files, so that `<windows.h>` does not hide
  them.
- **Includes.** `framework.h` lost the template's four C runtime headers (`<stdlib.h>`, `<malloc.h>`,
  `<memory.h>`, `<tchar.h>`; one `memcpy` became `std::memcpy` from `<cstring>`). Unused includes were removed
  (`<cmath>`, `<cwctype>`, `CivPickerController.h`, `<d2d1_1helper.h>`). System headers were put in order in 8
  files. The tests include the program's sources by name, with the program's folder as an include directory of
  the test project.
- **Files.** `wWinMain` moved to `Main.cpp`, and `LoadCatalog` became a static member of `CivPickerController`.
  `CivPickerShare.cpp`, `CivPickerDialogs.cpp` and `CivPickerDlcToggles.cpp` were renamed
  `CivPickerViewShare.cpp`, `CivPickerViewDialogs.cpp` and `CivPickerViewDlcToggles.cpp`, after their class. The
  test runner's `main` moved to `TestMain.cpp`; `test.cpp` lost its tabs and took the project's brace style.
- **Linking.** The ten `#pragma comment` lines became `AdditionalDependencies` and
  `AdditionalManifestDependencies` of the project file. The unused `gdiplus.lib` was removed. The WIC factory is
  created with `IID_IWICImagingFactory`. The embedded manifest and the imported DLLs are the same as before.
- **Formatting.** 8 lines of arguments aligned with their first argument.
- **Found to comply without changes.** Names and abbreviations (`Dlc` and `Id` throughout; only the SDK's COM
  interfaces use capitals), the order of a class's sections, no function bodies in class definitions (in headers;
  see the next section), no public data in classes, no block comments, no embedded assignments, no `goto`, no
  global variables (the one mutable variable at file scope, `control_class_atom`, is local to its file), no large
  parameters by value, no real nesting of conditional operators, one declaration a line.
- **Checked.** Zero warnings at `/W4` in Debug and Release, x64 and Win32; zero `/analyze` findings (x64 and
  Win32); see the correction in the next section. The `/Wall` probe shows no warning beyond the informational
  ones. All 83 unit tests passed on x64 and Win32. No names of data in code. The 55 scripted steps against the
  build before: no difference, no assertion fired. The three tabs on a fresh start are identical to it pixel for
  pixel.
- **Documents.** `README.md`: a section on how the code is organized; its compile steps named a file that no
  longer exists.

#### Decisions

- **What was adopted.** A rule of the Goddard standard is left out when it contradicts the three books, their
  readings for Win32, the rule on `data.json`, or the project's own convention. A convention is one form used
  throughout the code: `T* p` (246 uses to 0), conditions of `?:` without parentheses (211 of 216). Where the code
  was mixed (switches with and without a default case, 10 to 16), the standard's rule was adopted. What the
  standard itself calls optional stays optional. Its rules on GMAT, CVS, C code and Doxygen do not apply.
- **Left out**, each for the earlier rule it contradicts: camelCase variables and enumerators in capitals
  (snake_case is the convention); classes named after their parent, struct names ending in `Type`, a project
  prefix on names, typedefs for built-in types; anything on exceptions; `*` and `&` set with the name, and `NULL`
  for null pointers (`nullptr`; `NULL` for handles); `.hpp` files and one class a file pair; the first brace of
  every block on its own line; variables and comments lined up in columns; explicit comparison of booleans
  (against `if (!c_assert(...))`); lines of 80 characters (120 stays); file and method prologs, banners and
  mandated comments; Get and Set methods for data; include guard macros (`#pragma once` stays); default and copy
  constructors, destructor and assignment in every class; definitions in the order of their declarations;
  template definitions behind conditional compilation; modifiable arguments by reference; optimizations from the
  start.
- **Readings.**
  - The program is not a library, so its classes stay at global scope, without a namespace.
  - A single statement under an `if` or a loop may stay on the same line: `if (!c_assert(x)) return false;`.
  - A Win32 `BOOL` is a boolean by meaning and is tested directly (`if (!GetWindowRect(...))`). A Win32 function
    that returns other values in a `BOOL` (`TrackPopupMenu` with `TPM_RETURNCMD`) is compared explicitly.
  - A default case comes last in every switch. Stacked labels (`case A: case B:`) share one body and need no
    comment.
  - A struct may be returned by value; the rule against large values is about parameters.
  - C++14 has no array-length function, so the `CountOf` helpers stay.
  - System headers are ordered "lower levels first"; the project's own headers alphabetically, as they were.
  - Generated files keep their tool's format: `Resource.h` keeps its tabs and has no `#pragma once`.
  - A main procedure has a file of its own. The test runner's is `TestMain.cpp`, named apart from `Main.cpp` so
    that their object files cannot collide in a shared build folder.
  - No directory in an include: includes name files only.

### No file over 1000 lines, no folder over 20 files

#### What changed

- **Rules.** Two rules were added to `CLAUDE2.md`, binding as the rule on `data.json`: no file may be over 1000
  lines, and no folder may hold over 20 files, unless it holds assets such as the civ icons.
- **Split files.** Along their section banners, the code moved verbatim. `CivPickerView.cpp` (1100 lines) gave
  `CivPickerViewObserver.cpp` its observer notifications and the labels, logs and tooltips they update (808 and
  299 lines now). `CivPickerController.cpp` (1021) gave `CivPickerControllerInput.cpp` its hotkeys and the
  commands of the menus, buttons and toggles, with the tables only they use (656 and 379). `OpenEditionLink` and
  `OpenEditionPage` stayed with the other actions that open pages, so `OpenUrl` stays local to its file.
- **Folders.** The 60 sources moved, with `git mv`, from one folder of 71 files into folders by their role:
  `common\` (`Assertions`, `CivDataLimits.h`, `GlobalConstants.h`), `model\` (`CivCatalog`, `JsonReader`,
  `CivPickerModel`, `ShareLink`), `controller\`, `view\` (with `CivPickerLayout.h` and `DialogMetrics.h`),
  `parchment\graphics\` (renderer, images, fonts, layout arithmetic, theme, `PixelSurface`),
  `parchment\controls\` (the control base, every control, the logs and the scrollbar) and `parchment\windows\`
  (window shapes, dialogs, menu bar, tooltip and toast). The project's own folder keeps the project and resource
  files, `Main.cpp`, `Resource.h`, `targetver.h`, `framework.h`, `data.json`, the rules and the asset folders.
- **Project files.** Each folder is an include directory of the program's project file in all four
  configurations; the test project lists the folders whose sources it compiles (`common`, `model`,
  `parchment\graphics`). The `.vcxproj.filters` file mirrors the folders.
- **Function bodies in class definitions.** The pass of the section before had checked headers only. Three classes
  in `.cpp` files had bodies in their definitions: `DataReader` (`CivCatalog.cpp`, 13 bodies), `SilentObserver`
  (`CivPickerModel.cpp`, 16) and `LinkOpener` (`ShareLink.cpp`, its constructor). The bodies now follow the class
  definitions.
- **Warning level.** Both project files set `WarningLevel` to `Level4`; it was `Level3`.
- **Fixed, the solution builds.** `ISSUES.md` said that `UnitTesting_FRCP` did not compile, so that Build Solution
  failed. The tests had been rewritten with the model on 2026-10-13, and the stale entries of the project files
  (duplicate `civ_icons\*.png` items in the `.vcxproj.filters`, `<Image>` and `<Text>` items for old paths) went
  on 2026-10-14. The solution builds as it stands: the program's project uses toolset v145, the test project v143
  (MSVC 14.44) with Windows SDK 10.0.22621.0.
- **Checked, builds.** x64 and Win32, Debug and Release: zero warnings at `/W4`, set by the project files and
  confirmed in the compiler's command logs. Zero `/analyze` findings (x64 and Win32 Debug; 31 analysis logs each,
  none with a defect). All 83 unit tests passed in all four configurations, and in the solution as it stands
  (Debug, x64 and x86, zero warnings).
- **Checked, verbatim moves.** A scratch tool compared every definition, with its comment, before and after: 761
  of 764 are identical, and the 3 that differ are the three classes above. clang-tidy 22 finds nothing in the
  touched files that it did not find before: the same 13 tests of a Win32 `BOOL`, which are allowed, and the base
  classes' protected virtual destructors.
- **Checked, mechanical rules.** By a scratch checker that was shown to catch a planted breach of each: no file
  over 1000 lines, no folder over 20 files besides the asset folders, no function over 60 lines, no line over 120
  characters, no tab, every switch with a default case last, no `#define` constant, no conditional compilation,
  no `goto`, `setjmp`, `throw`, `new` or `malloc`, no function pointer, no block comment, no directory in an
  include, no `#pragma comment`, every source in its project file, no name from `data.json` in code.
- **Checked, behaviour.** On a hidden desktop, with silent jingle files and the interface's sounds off, the build
  before against the build after. A scenario through the three tabs, DLC, civ and edition toggles, Enable All and
  Disable All, auto-toggle and auto-reset, undo, redo, reset, quicksave and quickload, and the Options, Hotkeys,
  About, History, Join Lobby and Open Link dialogs gave 56 pictures and 44 snapshots of every control (class, id,
  visibility, check state, place, text). The snapshots and the `log.txt`, `settings.ini` and quicksave written
  were identical in every run. The pictures of the build before differ from one of its runs to the next, in up
  to 425 pixels of some; the build after matched its own second run pixel for pixel and never differed from
  either run of the build before by more than that. 70 draws through an exhausted pool: the same controls, round
  (26 of 56) and freshness history (28). No assertion fired, and both builds exited normally.
- **Documents.** `CLAUDE2.md` (1387 lines) became the rules (`CLAUDE2.md`, 856 lines) and a work log beside it,
  `CLAUDE2_LOG.md`.

#### Decisions

- **Every file.** The limit of 1000 lines holds for every file in the repository, code or not: sources, headers,
  project files, data and the rule files. Below it *Clean Code*'s sizes still apply: about 200 lines typical,
  500 desirable.
- **Counting a folder.** The files directly in it count, tracked or not; a subfolder does not count towards its
  parent. Exempt: asset folders (`images\...`, `sounds\...`, which grow with `data.json`) and folders that only
  builds and tools fill (`Debug`, `Release`, `x64`, `packages`, `.vs`).
- **Small files stay.** A file under 250 lines stays a file of its own; merging small files was decided against.
- **How to split.** A file that grows past the limit is split along its sections into files named after its
  class (`CivPickerView*.cpp`, `CivPickerController*.cpp`), the code moved verbatim. A folder that grows past it
  is divided by what its files depend on.
- **Dependencies between folders.** Nothing in `model\` or `common\` includes from the view, the controller or
  Parchment, and no Parchment folder includes from one that builds on it. A new folder goes into both project
  files and into the `.vcxproj.filters` file.
- **Backlog.** The open points under Not done in
  [Model, view and controller in place of one file](DEVLOG1.md#model-view-and-controller-in-place-of-one-file) were to
  be recorded, not worked off, in this change; only the warning level was set.
- **`CLAUDE.md`** was to be written anew from the two rule files.

#### Obstacles met, and how they were solved

- **Correction: earlier `/W4` and `/analyze` results.** In the sections of 2026-10-13 to this day those results
  were taken with the options in the `CL` environment variable (msbuild's `/p:AdditionalOptions=` does not reach
  the compiler). `CL` goes before the project's own options, so the project's `/W3` won; the compiler said so
  (D9025, overriding '/W4' with '/W3'). The builds of this section are `/W4` for real, and were clean. Options
  for a single build go in `_CL_`, which goes after the project's.
- **Tests with toolset v145.** Building the tests with v145, to test both projects with one compiler, needs
  `/p:Force-Enable-Microsoft-googletest-v140-windesktop-msvcstl-static-rt-dyn=true`: the googletest package
  switches itself off for toolsets after v143. From a copy of the repository in the temporary folder the gtest
  include path passes 260 characters, and `cl.exe` cannot open it.
- **Rebuild deletes outputs.** A Rebuild clears what the last build wrote from its intermediate folder. One
  Rebuild ran in the default intermediate folders with the output folder pointed elsewhere, and so deleted the
  exes and test binaries in `x64\Debug` and `Debug`; data, logs, settings, saves and histories stayed. A Build
  into the default folders put them back. Scratch builds now point both folders elsewhere.
- **Hotkeys on a hidden desktop.** They cannot be tried there: the controller ignores a hotkey unless its window
  is the foreground window, and `GetForegroundWindow` answers for the input desktop only. The scenario clicks the
  controls and sends the menu commands; the hotkey code was shown to be moved verbatim. The program should not be
  tried on a desktop in use either: while it runs, Space, Tab, Enter and the letter keys are its system-wide
  hotkeys.

#### Not done

- The backlog named under Decisions, with this change to it: `/W4` is now enforced by the project files, but no
  code analysis is configured in them, `/Wall` waits for question 4 of
  [Parchment interface, as the web version's](DEVLOG1.md#parchment-interface-as-the-web-versions), and
  `LanguageStandard` is not set.
- The questions of the earlier sections are all still open.
- To check by hand: the solution in Visual Studio after the move (Solution Explorer shows the folders as filters,
  F5 builds and runs as before), and a few hotkeys.

## 2026-10-17

### Hotkeys the user can change

#### What changed

- **Why.** A new feature was asked for: the user sets their own hotkeys in Options. The web version has nothing
  to match (it has two fixed sets of keys), so the design is the desktop's own.
- **Model.** `HotkeyBindings` (`HotkeyBindings.h` and `.cpp` in `model\`, plain C++). Every line of the Hotkeys
  dialog is a hotkey (`HotkeyAction`, 50 of them) with a scope (global, or the Draw Civ, Log or Civ Pool tab), a
  default key (the key the program always had), a name in `settings.ini`, and the key the user gave it
  (`Keystroke`: a virtual-key code with the modifiers `MOD_ALT`, `MOD_CONTROL` and `MOD_SHIFT`; the controller
  `static_assert`s that the numbers are Windows'). `CivPickerModel` holds the bindings (`GetHotkeys`,
  `EditHotkeys`) as it holds the settings.
- **Clashes.** Two hotkeys that can work at the same time (on the same tab, or one of them global) never share a
  keystroke: giving a hotkey a keystroke takes it from the hotkeys it clashes with (`ClashesWith`, at most one on
  each tab).
- **Unused hotkeys.** The hotkeys of editions and DLC slots that `data.json` gives nothing to do (past its
  editions, past the most DLC toggles an edition has) are not registered, not shown, and never clash.
- **Names of keystrokes.** "Ctrl+S", "Num 5", "None"; `ParseKeystroke` reads them back. A keystroke can be a
  hotkey when its key has a name (letters, digits, F1 to F11, the editing and arrow keys, the number pad) and
  Windows does not keep it (F12; Alt with Tab, F4 or Space; Esc with Alt or Ctrl; Ctrl+Alt+Delete).
- **Controller.** `CivPickerControllerHotkeys.cpp` (new) registers each keystroke of the hotkeys in use once (the
  id is the action's number + 1), turns the keystroke of a `WM_HOTKEY` into the hotkey for the tab shown
  (`ActionFor`) and runs it, one switch a scope. The table of ids and its handlers left
  `CivPickerControllerInput.cpp`; `GlobalConstants.h` lost `HOTKEY_AMOUNT`, the `HOTKEY_ID_*` constants and the
  id and key tables of the editions and DLC slots.
- **`settings.ini`.** A section `[Hotkeys]` has a line for each hotkey whose key is not its default
  (`DrawTabDrawCiv=D`, `Exit=None`), written anew on exit, so with default keys there is no section. A line that
  names no keystroke the hotkey can have is left out at startup.
- **Customise Hotkeys dialog.** Options > Hotkeys has a Customise button beside Show Hotkeys. It opens the dialog
  (desktop only, `CivPickerViewHotkeys.cpp`): a hint, a radio button for each section, the section's hotkeys in
  rows of a key (`KeyButton`) and what it does (one column, or two even ones past 12 rows), two status lines, and
  Defaults, Cancel and OK. A click sets a key listening (gold, an ellipsis): it takes every key, Tab, Enter and
  Esc too, and reports the first that is not a modifier, with the modifiers held. Backspace leaves the hotkey
  without a key (a dash). Another click, the focus leaving or another section stops the listening.
- **Status lines.** They say what became of the key: the hotkey's new key; the hotkeys it was taken from ("Q was
  taken from Open Options (Draw Civ tab), Clear drawn log (Log tab) and Definitive Edition (Civ Pool tab)."); a
  keystroke Windows keeps or one without a name; and that another program has the keystroke as a hotkey (the
  controller tries to register it; the program's own hotkeys are unregistered while the dialog is open).
- **When keys change.** At once. Cancel or Esc gives back the keys the dialog opened with, and Defaults gives
  every hotkey its default.
- **Texts.** The menus' shortcuts, the "Hotkey:" lines of the tooltips and the Hotkeys dialog show the keys the
  hotkeys have (`RefreshHotkeyTexts`, once the dialog closes). A hotkey without a key has no shortcut and no
  hotkey line, and a dash in the dialogs. The Hotkeys dialog's layout is made from the bindings (it moved out of
  `CivPickerViewDialogs.cpp`). The Draw and Reset buttons, shown on the Draw Civ and Log tabs, say the key of
  the tab shown.
- **Menus.** With a menu open, a hotkey's keystroke goes to the menu first as its own key (Esc, Enter, the
  arrows) and otherwise closes it, as Esc and Enter did before.
- **Parchment.** `KeyButton` and `PaintKeyCap` (the Hotkeys dialog's keys are drawn with it too);
  `PartKind::key_button` with 24 key buttons a dialog level, and 32 labels, up from 16;
  `ParchmentControl::OnFocusLost` and `NotifyParentLater` (posted, so that a key's parent changes the key after
  the key's own message, without re-entrancy); `ParchmentMenuBar::Remeasure`.
- **Fixed, F6 and T.** The fixed table of hotkeys registered `VK_F6` under the quicksave hotkey's id (F6 was never
  documented) and registered `T` twice. The table is gone: each hotkey has one key, each keystroke is registered
  once, and every id is unregistered when the program loses focus. F6 no longer quicksaves unless the user gives
  Quicksave that key.
- **Tooltip.** The aoe2vils.net tooltip says "Ctrl+T" where it said "Ctrl + T", as keys are named everywhere
  else.
- **Tests.** 113 unit tests, 30 of them new: every default is the key the program had and none clash; what a
  keystroke means on each tab; clashes and taking a key; unused slots; names read back for all 596 usable
  keystrokes; F12 and the keystrokes Windows keeps refused; the model's hotkeys follow `data.json`.
- **Checked, tests.** All 113 passed on x64 and Win32. With the clash rule broken on purpose (global keystrokes
  not clashing with the tabs'), 3 of them failed; the breakage was undone.
- **Checked, builds.** x64 and Win32, Debug and Release: zero warnings at `/W4`; zero `/analyze` findings (x64
  Debug, 34 analysis logs). clang-tidy 22 finds nothing new in the touched files but one test of a Win32 `BOOL`,
  which is allowed. A scratch checker of the touched files: none over 1000 lines (the largest,
  `CivPickerModel.cpp`, has 872), no folder over 20 files, no function over 60 lines, no line over 120
  characters, every switch with a default case last, ASCII and CRLF throughout.
- **Checked, in the program.** On a hidden desktop (a new scratch harness: posted clicks and keys, `PrintWindow`,
  the debug output read for failed assertions), Debug x64, Release x64 and Release Win32: the sections; listening
  and stopping; a global keystroke taken from three tabs' hotkeys; Backspace; F12; a key without a name; F10 (a
  system key), Esc, Tab and Enter taken as keys while listening, and Esc cancelling otherwise; Ctrl+Shift+Y with
  the modifiers held; Defaults, Cancel and OK; another program's hotkey (the harness's own) warned of.
  `settings.ini` kept the changed lines only, and at the next start the menus, the Hotkeys dialog and the
  Customise Hotkeys dialog showed them; a refused F12 and broken lines were left out. No assertion fired, and the
  program exited normally each time.
- **Checked, default keys.** Against the build before, each build run twice (the first run of the build before
  differed from its second in up to 49 pixels): the three tabs, all six menus and the Hotkeys and About dialogs
  identical pixel for pixel, Options different only inside its new button, `settings.ini` identical byte for
  byte.

#### Decisions

- **Where.** Options > Hotkeys > Customise, a dialog of its own; the Hotkeys dialog stays the read-only summary.
  Every hotkey it lists can change, those of the editions and DLC slots too, and each has one key.
- **A keystroke in use** where the hotkey would also work is taken from the hotkeys that had it, and the status
  line names them; it is not refused. Swapping two keys takes two presses, and Cancel undoes everything
  (question 2).
- **Backspace.** While a key listens, Backspace means "no key", so plain Backspace cannot be a hotkey;
  Ctrl+Backspace can. Esc, Tab and Enter can.
- **Punctuation keys** (; = , - . / ` [ ] \ ') cannot be hotkeys: their characters depend on the keyboard's
  layout (question 3).
- **Draw civ and Reset** of the Draw Civ tab and of the Log tab are separate hotkeys, as the Hotkeys dialog lists
  them.
- **F6** no longer quicksaves.

#### Not done

- **Options' radio buttons.** The two radio buttons under "When the program starts" do not untick each other.
  A click on "Automatic civ pool logging" while "Reset civ pool on startup" is ticked leaves both ticked until
  Options is opened again; the setting itself changes correctly. A parchment radio button only ever ticks itself
  (`Checkbox::OnClicked`), and nothing unticks the other one after a click; `InitialiseOptionsDialog` calls
  `CheckRadioButton` only when the dialog opens. Found on the build before this change, by a click on
  `IDC_RADIO_LOGGING` on a hidden desktop: both report `BST_CHECKED`. It was not changed. The section radio
  buttons of the Customise Hotkeys dialog untick each other with `CheckRadioButton`.
- **Not checked:** pressing the hotkeys themselves (a hidden desktop is not the input desktop); the keys' hover
  look; keys typed on a real keyboard into the dialog (the harness posted them to the key button, which the
  dialog manager handles the same way).
- **Pictures changed from outside this work.** While it ran, `images\ui\background.png` was moved to
  `images\ui\obsolete\` and `images\ui\new_background.png` appeared; they were left as they were. The scratch
  test folders used the moved file.

#### Questions

1. **The web version.** Should it get hotkeys the user can change too? Proposal: left to be decided; it is
   another project.
2. **Taking a keystroke over.** Keep it, refuse a keystroke in use, or swap the keys of the two hotkeys?
   Proposal: keep taking over.
3. **Punctuation keys.** Should they be allowed, named after the characters of the keyboard's layout? Proposal:
   not until it is asked for.

## 2026-10-18

### Shorter parchment

#### What changed

- **Why.** A new `ui/background.png` (1024 x 1070) was supplied, to fit more neatly and without the excess height
  below the content.
- **Picture.** It is the picture of
  [DLCs added by data.json alone, and original images](DEVLOG1.md#dlcs-added-by-datajson-alone-and-original-images)
  with its rows 0 to 886 unchanged and a new burnt bottom edge 97 px higher. Its visible part (alpha above 3) is
  x 26 to 995, y 13 to 1054.
- **Window.** `BACKGROUND_VISIBLE_BOTTOM` is 1054; it was 1151. The window, whose height follows from it, is
  621 px tall, down from 679: 38 px of parchment below the client area where there were 96 (the web version has
  22). This leaves more room on a 1080p display at 150 %.
- **Answers.** Of the questions of 2026-10-15, in
  [DLCs added by data.json alone, and original images](DEVLOG1.md#dlcs-added-by-datajson-alone-and-original-images):
  the bare parchment below the content is cut by the shorter picture, and the client area stays as it was (1).
- **Checked.** Zero warnings at `/W4` in Debug and Release, x64 and Win32; all 113 unit tests passed. On a hidden
  desktop the window is 578 x 621; no pixel of the content lies outside the window's region, which reaches row
  609 in the middle; the paper is fully opaque under the content's columns down to row 596, and the content ends
  at row 583. The menus and the Options, Hotkeys and About dialogs are identical pixel for pixel to the build
  before (the Hotkeys dialog once showed its OK button's focus outline, which Windows shows or hides by the keys
  of the run). No assertion fired.

#### Not done

- **Pictures in git.** `images\ui\new_background.png` and `images\ui\obsolete\background.png`, committed in
  `5b78ec3`, are deleted in the working tree, and `images\ui\background.png` is untracked; nothing was staged.

## 2026-11-20

### 07:07:00: the repository in the form of the repository template

#### What changed

- **Why.** The repository was to comply in full with the template at github.com/cnordenb/repo_template: its files,
  and its rules for the code.
- **Files from the template.** `AGENTS.md`, `CLAUDE.md`, `.claude/` (settings and the seven skills), `documentation/`
  and, in `.github/`, `ACCESSIBILITY.md`, `CODE_OF_CONDUCT.md`, `CONTRIBUTING.md`, `SECURITY.md`, the issue and pull
  request forms, `scripts/check-rules.sh` and `workflows/checks.yml`. `FUNDING.yml` and `README.md` were kept as
  they were, apart from the README's language badge (C++ instead of C) and a line of credit in its Acknowledgements.
- **Placeholders.** The project's name, description, owner, repository, chief developer, stack and test command were
  filled in. `{{SECURITY_CONTACT}}` was left, to be given before the first release.
- **Files renamed, moved and deleted.** `LICENSE.md` is `LICENSE`. `build.bat` and `FRCP_GUI.manifest` are in
  `Fresh-Random-Civ-Picker_CPPGUI/tools/`, the two icon files and `social-preview.png` in `images/`, `framework.h`
  in `common/`. Deleted: the main folder's `social-preview.png` and `CppProperties.json`, `RCa23700`, and
  `Fresh-Random-Civ-Picker_CPPGUI.filters`. `.gitignore` no longer leaves out `CLAUDE.md` and has the template's
  lines; `.gitattributes` keeps `*.sh` with LF endings.
- **Earlier documents handed over.** `CLAUDE2.md` and `formatting_notes.md` became Rules 27 to 105 of `RULES.md`,
  condensed, with their source tags. `CLAUDE2_LOG.md` and `ISSUES.md` became the days 2026-10-05 to 2026-10-18 of
  this log, rewritten in its form, and the list in `DECISIONS.md`; the times of those days were never recorded, so
  their sections carry none. The four files were then deleted.
- **Rules.** Rule 14 and Rule 15 do not apply to the asset folders `images/` and `sounds/`. Rule 26 also names
  `FRCP_GUI.rc`, `Resource.h`, `*.sln`, `*.vcxproj`, `*.filters`, `build.bat` and `convert_web_assets.py` as not
  code. Rules 27 to 105 are new. Where the earlier rules contradicted Rules 0 to 26 they were dropped: every rule
  that allowed or required a comment, the limits of 1000 lines a file and 20 files a folder, and the parts of the
  testing rules that differ from Rules 5 and 25.
- **No comments (Rule 10).** All 1240 comments were removed from the `.cpp` and `.h` files, `Resource.h` excepted.
  What a maintainer needs of them is in `MAINTAINER.md`.
- **No code file over 400 lines (Rule 13).** Fourteen files were split in two, the code moved as it was. New:
  `CivCatalogLookup.cpp`, `JsonReaderAccess.cpp`, `CivPickerModelPool.cpp`, `CivPickerControllerExternal.cpp`,
  `ParchmentLogSelection.cpp`, `ParchmentGeometryPixels.cpp`, `ParchmentImagesLoading.cpp`,
  `ParchmentDialogLevels.cpp`, `ParchmentMenuInput.cpp`, `CivPickerViewTabsSounds.cpp`,
  `CivPickerViewFixedDialogs.cpp`, `CivPickerViewHotkeyEditor.cpp`, `CivPickerViewTooltips.cpp` and
  `CivPickerViewTypes.h`. The header holds what was in `CivPickerView.h` outside the class's members; the types
  and constants that were private there are now at file level, and `SECTION_TITLES` is in the namespace
  `CivPickerViewDetail`.
- **No folder over 7 files (Rule 14).** The code is in `common/`, `controller/`, `model/catalog/`, `model/picker/`,
  `view/main/`, `view/dialogs/`, `view/layout/` and, under `parchment/`, `controls/base/`, `controls/lists/`,
  `graphics/drawing/`, `graphics/layout/`, `windows/dialogs/` and `windows/popups/`. The five test files are in
  `UnitTesting_FRCP/tests/`. Both project files, the filters file, `build.bat` and the icon paths in
  `FRCP_GUI.rc` follow the new places.
- **Tests.** 113, unchanged but for the include lines of the new parts. `UnitTesting_FRCP/run-tests.sh` is new: it
  fetches GoogleTest 1.14.0 into `temp/`, builds the tests with g++ and runs them. `checks.yml` runs it, and
  `check-rules.sh`, on a Windows runner.
- **`check-rules.sh`.** It knows the asset folders, takes `.cpp`, `.h`, `.py` and `.bat` as code apart from the
  files Rule 26 names, and does not read `Resource.h` for placeholders.
- **Checked.** Before any change the program was built with `build.bat` and the 113 tests passed under g++. After
  the comments were removed, every file's tokens were compared with the file before, with g++'s preprocessor: no
  difference. Each split was compared line by line with its original: no line lost or changed, apart from those
  named above. At the end the program was built with `build.bat`, started and stayed open, the 113 tests passed,
  the longest function counted 37 lines, and `check-rules.sh` passed.
- **Documents.** `TESTS.md`: all 113 tests, in the order they run under `run-tests.sh`. `MAINTAINER.md`: its
  chapters filled in for this project. `TERMINOLOGY.md`: the project's terms. `DECISIONS.md`.
- **Replaced.** The earlier limits of 1000 lines a file and 20 files a folder, in
  [No file over 1000 lines, no folder over 20 files](#no-file-over-1000-lines-no-folder-over-20-files), by Rules 13
  and 14; and every earlier decision that a rule's exception is justified in a comment, by Rule 10.

#### Not done

- **Visual Studio.** The solution was not built: MSBuild was not found on the machine. The project files were
  changed by script and only checked to be well-formed XML. The order in which the tests run there may differ from
  the order under g++, which `TESTS.md` follows.
- **GitHub.** `checks.yml` was not run; it has never run on a Windows runner.
- **`REQUIREMENTS.md`.** Still the template's first line; to be written by the chief developer.
- **Tests against Rules 5 and 25.** Left as they are, as decided. 102 of the 113 make more than one check, and
  several test the same thing, among them Tests 7 and 18, 12 and 15, 30 and 31, 32 and 33, 34 and 35, 86 and 87.
  Only Test 55 is tied to an earlier bug; no test can name a requirement yet.
- **`ACCESSIBILITY.md`.** No item was checked or marked.
- **ub-mode.** Its tools do not list C++, so crapper, dryer and mutator were not run.
- **Unused includes.** Some split files keep includes they no longer need, such as `<shellapi.h>` and `<iomanip>`
  in `CivPickerController.cpp`; removing them was not tried under Visual Studio.
- **Known issues of 2026-10-05.** Not looked into. Some name code that is gone (`ConvertToString`,
  `hOptionsDlg`); `RCa23700` and the stale filters file are now deleted.
- **Two file names cover two topics.** `CivPickerViewTabsSounds.cpp` and `CivPickerModelPool.cpp`, which also
  holds undo, redo and the log file; no cut in two gave one topic a file within 400 lines.
- **History texts.** `Histories/` is not in the repository and no build copies it; `MAINTAINER.md` says so.
- **Older BMP folders.** `images/civ_icons/`, `dlc_icons/` and `edition_icons/` are read by no code; whether
  to delete them is still the open question of 2026-10-13.

#### Questions

1. **Exceptions to a rule.** The earlier rule that a rule may be broken with approval, justified in a comment, is
   gone, so no rule allows an exception. Proposal: keep it so; an exception is a change of `RULES.md`.
2. **Justifications that were comments.** Six earlier rules asked for a comment saying why (a `(void)` cast, a
   Win32 function pointer, conditional compilation, a case that falls through, a pattern adapted). Proposal: they
   go in `MAINTAINER.md`.
3. **A case that falls through.** No rule covers it now. Proposal: require `[[fallthrough]]`.
4. **Warnings.** Rule 45 says no warning is ever disabled, stricter than before. Proposal: keep.
5. **The language standard.** `build.bat` builds as C++17; the project files set none, which is C++14 in Visual
   Studio. Proposal: set C++17 in both project files.
6. **Build folders and Rule 14.** `Debug`, `x64`, `packages` and `.vs` are no longer named as exempt;
   `check-rules.sh` counts only files Git does not leave out. Proposal: say so in Rule 14.
7. **Variables are nouns (Rule 49).** Booleans such as `is_open` break it. Proposal: "nouns, or Is/Has/Can".
8. **Includes between the new folders (Rule 102).** Only `model/picker/` from `model/catalog/` is stated; the
   directions between `base` and `lists`, `drawing` and `layout`, `dialogs` and `popups`, and the three `view/`
   folders are not. Proposal: state them as the code has them now.
9. **Indentation of at most two levels (Rule 52).** A target or a limit? Proposal: a target.
10. **The tests.** Should those that break Rules 5 and 25 be split and the repeated ones removed, once
    `REQUIREMENTS.md` exists? Proposal: yes, then.
11. **The order of the tests.** `TESTS.md` numbers them as g++ runs them. Should the Visual Studio project stay
    as a second way to build the tests? Proposal: keep it, with `run-tests.sh` as the one that numbers them.
12. **Proposals in the rewritten log.** Three proposals in the days of 2026-10-15 were added in the rewriting and
    were in no earlier document: matching the web version on both questions of continuous freshness, and raising
    the limit of 64 civs before a third DLC needs it. Should they stand?
