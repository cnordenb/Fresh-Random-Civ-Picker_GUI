# Development log, part 1

- [2026-10-05](#2026-10-05)
- [2026-10-06](#2026-10-06)
- [2026-10-13](#2026-10-13)
- [2026-10-14](#2026-10-14)
- [2026-10-15](#2026-10-15)

Part 1 of the development log. It goes on in [part 2](DEVLOG2.md). The decisions made in the log are listed in
[DECISIONS.md](../DECISIONS.md).

## 2026-10-05

### Known issues found by reviewing the code

#### What changed

- **Why.** The code was read through at commit `10b2ab4` to find latent bugs and quirks. The whole program was then
  in `FRCP_GUI.cpp` and `FRCP_GUI.h`; the function names below are that file's.
- **Found.** Nothing in the code was changed. The issues were listed in `ISSUES.md`, a file of tracked and fixed
  issues. Those still open are under Not done.
- **Found and fixed later.** Auto-reset skipped after the Slavs checkbox and after the J, K and L hotkeys, see
  [Auto-reset after Slavs and after J, K and L](#auto-reset-after-slavs-and-after-j-k-and-l);
  the test project did not compile, see
  [No file over 1000 lines, no folder over 20 files](DEVLOG2.md#no-file-over-1000-lines-no-folder-over-20-files);
  F6 also quicksaved and T was registered twice, see
  [Hotkeys the user can change](DEVLOG2.md#hotkeys-the-user-can-change).

#### Not done

- **DLC checkboxes and edition radio buttons by mouse.** Toggling one with the mouse does not trigger auto-reset,
  while the hotkeys that do the same (A to H; Q, W and E) do. Their control ids fall outside the range of ids that
  the main window's `WM_COMMAND` handler resets for, and `ToggleDlc` and `SetEditionState` never reset.
- **Options, jingle type.** `if (IDC_LEGACY_OPTION)` in `OptionsDlgProc` is always true; it should compare `wmId`
  with it. Any control's notification code equal to `CBN_DROPDOWN` or `CBN_SELCHANGE` is treated as coming from the
  jingle-type dropdown. No visible effect.
- **Hotkeys forwarded to Options.** `hOptionsDlg` is never assigned, so the code that forwards `WM_HOTKEY` to the
  Options dialog never runs.
- **`ConvertToString`.** It writes into a `std::string` that is reserved but empty (undefined behaviour that happens
  to work) and cuts the output at `MAX_LENGTH`, 15 bytes, so a civ name of more than 14 characters would be cut off
  or garbled in the label of the drawn civ. Calling `SetWindowTextW` with `current_civ` would avoid both.
- **Civ checkbox range.** The range of civ checkbox ids in `WM_COMMAND`, `wmId <= (MAX_CIVS + 5)`, is off by one:
  id `MAX_CIVS + 5` would index past the end of `civ[]`.
- **`LoadJingles`.** It stores `StringCleaner(l_str)`, a pointer into a string local to the loop, in
  `legacyJingle[j].name`. The pointer dangles after each turn of the loop; nothing reads it later.
- **Save Preset and Load Preset dialogs.** `nMaxFile` is set to `sizeof(szFile)`, which is bytes where characters
  are meant. The Save dialog also passes `OFN_FILEMUSTEXIST`, a flag for Open dialogs.
- **`IsValidLobbyCode`.** For a link of the form `aoe2de://0/...` it checks `substr(12, 20)`, which skips the first
  digit of the code; the digits start at index 11.
- **History texts.** Those of Mapuche, Muisca, Tupi, Danes, Saxons and Varangians are missing from the local
  `Debug\Histories\`, so the History dialog shows "History not found." for them. The game ships them as
  `<civ>-utf8.txt` (`danes-utf8.txt`); they must be copied and renamed `<Civ>.txt`, and the release zip must hold
  them.
- **Hotkeys dialog.** `IDD_HOTKEYS` in `FRCP_GUI.rc` does not list "H - Open History" of the Draw tab.
- **GitHub Repository menu item.** It opens the repository's old address, `.../Fresh-Random-Civ-Picker_CPPGUI`,
  which works only because GitHub redirects a renamed repository.
- **Sound.** Sound playback works poorly on slower computers (carried over from the known bugs in `README.md`).
- **Files committed by accident.** `Fresh-Random-Civ-Picker_CPPGUI/RCa23700` is a temporary file of the resource
  compiler, and `Fresh-Random-Civ-Picker_CPPGUI.filters` is a leftover of the Visual Studio template.

## 2026-10-06

### Auto-reset after Slavs and after J, K and L

#### What changed

- **Slavs checkbox.** Unticking or ticking Slavs did not trigger auto-reset. The `WM_COMMAND` handler reset only
  for control ids `wmId > 4 && wmId < 50 || wmId > 50 && wmId < 65`, a range left from when there were fewer civs.
  A civ checkbox's id is its index + 5, so id 50 is the civ at index 45: Slavs since The Viking Sagas, Tatars
  before. The range was mended (commit `bf19d68`).
- **J, K and L.** The hotkeys of the Definitive Edition's DLCs Dawn of the Dukes, Lords of the West and The Last
  Khans did not trigger auto-reset. The Civ Pool tab's `WM_HOTKEY` handler reset for
  `wParam > 1 && wParam < 4 || wParam > 12 && wParam < 22`, which covers A to H but not `HOTKEY_ID_J` (37),
  `HOTKEY_ID_K` (38) and `HOTKEY_ID_L` (39). The range was mended (commit `8cfd1a1`).

#### Not done

- How the two fixes were checked was not recorded.
- Toggling a DLC or an edition with the mouse still does not trigger auto-reset, see
  [Known issues found by reviewing the code](#known-issues-found-by-reviewing-the-code).

## 2026-10-13

### Development rules compiled from three books

#### What changed

- **Rules.** `CLAUDE2.md`, beside the code, was written as the binding rules of every change. It was compiled from
  three books kept in `materials/` (not in git): *The Power of Ten: Rules for Developing Safety Critical Code*,
  *Clean Code* and *Head First Design Patterns*. The log names them by their titles.
- **The Power of Ten, in short.** (1) Simple control flow: no `goto`, `setjmp` or recursion. (2) Every loop has a
  fixed upper bound. (3) No dynamic memory allocation after initialization. (4) No function over about 60 lines.
  (5) Two assertions per function on average, each with a recovery. (6) Data at the smallest possible scope.
  (7) Every return value checked, every parameter validated. (8) The preprocessor only for `#include` and simple
  macros. (9) One level of dereferencing, no function pointers. (10) All warnings on at the most pedantic setting
  and static analysis, both with zero findings.
- **Precedence.** It was decided that the Power of Ten overrides the other two books where they conflict. Where the
  books leave a detail open (brace style, naming case, file layout), the code as it is sets the standard: functions
  in `PascalCase`, variables and parameters in `snake_case`, constants in `UPPER_SNAKE_CASE`. A rule may be broken
  only with explicit approval, justified where it happens.
- **Where the books conflict.** Decided so:
  - No `throw` in the project's code. Failure is reported through return values that every caller checks; code that
    can throw is wrapped at the boundary.
  - A command may return a status of success or failure and nothing else; a query returns data and changes nothing.
  - An operation that can fail returns a small result struct, not output arguments, except where Win32 forces them.
  - A variable's scope is not widened to save arguments.
  - Virtual functions are allowed on small, closed hierarchies. Function pointers, pointers to member functions,
    `std::function`, stored lambdas and callback tables are not.
  - Every object is made and wired at startup. After startup: no `new`, no growing containers, no allocating
    string operations, no new threads; storage has fixed capacities named by constants. Nothing is made lazily.
  - Trees are walked with an explicit, bounded stack, not by recursion.
  - 60 lines is the ceiling of a function; *Clean Code*'s smaller sizes are the target.
  - Assertions and parameter checks are kept even where they make a function longer.
- **Readings for C++ and Win32.** Decided so:
  - Virtual functions are not function pointers, since a closed program's overriders are known when it is compiled.
    Hierarchies stay shallow, leaf classes are `final`, and no virtual call may re-enter its caller.
  - The function pointers Win32 demands (window, dialog and subclass procedures, enumeration callbacks, thread
    entry points) are the only ones allowed. Each stays thin: it validates its parameters and calls an ordinary
    function.
  - The message loop, and the modal loops Windows runs, are the only loops without a fixed bound.
  - "One level of dereferencing" counts the dereferences written in the source; the implicit `this` does not count.
  - "Initialization" is everything in `wWinMain` before the message loop, the main window's `WM_CREATE` included.
    Dialogs opened later run after initialization.
  - The Power of Ten governs the program's code. The test code follows *Clean Code* and compiles without warnings,
    but may use the heap in its fixtures.
  - Re-entrancy that Windows causes (`CreateWindow` sending `WM_CREATE`) is a platform boundary, not recursion. The
    code must not cause it on purpose: no `SendMessage` to a window from inside that window's handler.
  - On MSVC the most pedantic setting is `/Wall` with `/WX`, with the Windows SDK's headers set apart by
    `/external:anglebrackets /external:W0`. An informational `/Wall` warning that cannot be mended in the source is
    never disabled silently.
- **Model, view and controller.** The roles were laid down. The model holds all data, state and logic, knows
  nothing of the view or the controller, is plain C++ without `<windows.h>` so that it can be unit-tested, and
  notifies its one observer when its state changes. The view only presents: when notified it reads from the model,
  it never changes the model, and it knows how to enable a control but never when. The controller takes the input
  from the view, calls the model and may tell the view to change; it holds no logic of the application. Everything
  is built and wired in `wWinMain`; there are no globals, and a window procedure reaches its object through the
  window's user data.

#### Not done

- The code did not follow the rules when they were written. It was brought in line by the sections that follow;
  what stayed open is under Not done in
  [Model, view and controller in place of one file](#model-view-and-controller-in-place-of-one-file).

### Model, view and controller in place of one file

#### What changed

- **Why.** `FRCP_GUI.cpp` and `FRCP_GUI.h` held data, logic, windows and input together, against the rules of
  [Development rules compiled from three books](#development-rules-compiled-from-three-books).
- **Split.** They were split into `CivPickerModel`, `CivPickerView` and `CivPickerController` (a `.h` and a `.cpp`
  each). The program looks and works as before.
- **Fixed in the split.** Undefined behaviour and leaks without a visible effect: the narrow-string conversion of
  the centre label (`ConvertToString`); the file dialogs' buffer sizes given in bytes where characters are meant;
  the bold font made anew, and leaked, each time the Hotkeys dialog opened; the History search sending
  `WM_COMMAND` to its own dialog; civ indices out of range, which now stop the operation and no longer write past
  the arrays. Dead code was removed: unused globals and functions, and code that was commented out.
- **Tests.** The unit test project `UnitTesting_FRCP` was rewritten for the model.
- **Checked.** Zero warnings at `/W4` and zero findings of MSVC Code Analysis (`/analyze`); see the correction on
  how these two were measured in
  [No file over 1000 lines, no folder over 20 files](DEVLOG2.md#no-file-over-1000-lines-no-folder-over-20-files).
  Every function is under 60 lines. An automated comparison of the old program with the new one found no
  difference: 55 scripted steps, at each the text, visibility, check state and position of every control, and at
  the end the saved `log.txt` and `settings.ini`. The unit test project builds and its tests pass.

#### Not done

- **Warnings.** The project files do not enforce the warning level or the analysis: no code analysis is configured,
  `/Wall` is not used, and `LanguageStandard` is not set.
- **Assertions.** The average of two per function is not reached.
- **Return values.** Those of Win32 calls in the view and the controller are mostly neither checked nor cast to
  `(void)`.
- **Heap after startup.** `std::wstring` is used for state, log text and labels while the program runs. The model's
  index of names (`std::unordered_map`) is built once at startup and only read afterwards.
- **Constants.** `GlobalConstants.h` uses `#define`, against *Clean Code*.
- **Quirks kept on purpose**, since mending them would change the behaviour:
  - removing a civ before the pool was ever initialised underflows `iterator` in `CivPickerModel::RemoveCiv`;
  - the History dialog's search remembers the length of the previous text from one opening to the next;
  - changing the jingle type in Options plays the jingle with the previous setting;
  - `IsValidLobbyCode` does not check the first digit of a lobby link of 20 characters;
  - the B hotkey never plays its button sound when it turns the interface's sounds on;
  - `T`, and `F5` with `F6`, are registered twice under the same hotkey ids;
  - the dialog for saving a preset uses `OFN_FILEMUSTEXIST`.

### At most 10 draws a second, and images found beside the exe

#### What changed

- **Why.** Holding Space made the program's memory reach 1.5 GB. Each jingle is a new asynchronous
  `sndPlaySound`, and Windows keeps about 1 MB for each interrupted jingle and frees it slowly.
- **Draw rate.** Space and the Draw button draw at most 10 times a second
  (`CivPickerController::DrawCivWithinRateCap`).
- **Images.** They are loaded from `<exe folder>\images\`, no longer from the working directory. The target
  `CopyImagesToOutDir` of the project file copies `images\` beside the exe on every build.
- **aoe2vils.net button.** It shows `images\aoe2vils.png`, pixel art of 12 x 12, enlarged without smoothing.
- **Checked.** Memory stayed flat at 10 draws a second and climbed at about 16 a second.

### Parchment interface, as the web version's

#### What changed

- **Why.** The program was to look like the web version at frcp.net after that version's new interface.
- **View.** The view was rewritten. The model and the controller keep their roles; the controller gained the menu
  commands Quicksave and Quickload and the new links.
- **Drawing.** `ParchmentRenderer` is the only place that talks to Direct2D, DirectWrite and WIC. Two software
  render targets share every resource: an opaque one with ClearType text, and a premultiplied one for layered
  windows. `ParchmentImages` decodes the PNGs and builds the nine-slices and cursors; `ParchmentFonts` holds the
  text formats and measures text. Every image is a PNG decoded by WIC; GDI+ is no longer used.
- **Layout arithmetic.** `ParchmentGeometry` is plain C++ without Win32 or Direct2D: CSS border-image tiling,
  nine-slices, scrollbars, sliders, text wrapping, tooltip placement, box shadows, image filters, dotted outlines.
- **Window shape.** `ParchmentShell` gives a window the web version's shape. The window's region is the fully
  opaque pixels of its picture, and an owned, click-through layered window, the skin, shows the burnt edges or a
  dialog's shadow. A second layer dims a window while a dialog is open over it, 15 % a dialog. The main window is
  a borderless popup that is moved by its title row.
- **Controls.** `ParchmentControl` is the base of every control (button, checkbox, tab strip, civ and DLC toggle,
  label, slider, link, picture). Towards the controller it behaves as a Win32 button (`BM_*`, `WM_COMMAND` with
  `BN_CLICKED`, `TBM_*` and `WM_HSCROLL` for the slider), so the controller's control ids and logic are unchanged.
  `ParchmentLog` is the scrolling log, `ParchmentMenuBar` the menu bar with its lists, `ParchmentTooltip` the
  tooltip. `ParchmentDialogs` builds every dialog from a layout of data (`DialogLayout`) over an empty template in
  memory.
- **View files.** `CivPickerView.cpp` (window, observer notifications, sounds; 985 lines),
  `CivPickerViewLayout.cpp` (the main window's controls and menus) and `CivPickerDialogs.cpp` (the dialogs'
  layouts and procedures; 811 lines).
- **Painting.** A control draws over a copy of its parent's backdrop into a shared back buffer and is shown in one
  step. Sibling controls that wait for their paint are drawn in the same Direct2D frame, and a window paints its
  waiting controls before itself, so a change of tab appears at once. A window that has just been shown fills
  itself with its backdrop first, so it never shows black where controls are still to paint.
- **Assets.** `tools/convert_web_assets.py` converts the web version's WebP images in `..\frcp_html` to PNG
  (`images\ui`, `images\civ`, `images\dlc`). The CSS filters of the web version (grey icons, dimmed tabs, the
  tooltip's tint) are applied once, when the images load.
- **Memory.** Windows are shaped one at a time in one `ShellWorkspace` allocated at startup. Images are decoded in
  strips of 48 K pixels straight into their Direct2D bitmaps, and the parchment's picture is released once the
  main window's shape and backdrop hold it. After 4 s without input the view hands its working set back to Windows
  (`SetProcessWorkingSetSize(-1, -1)`); the next paint pages back what it needs. A key, click, hotkey or closed
  dialog starts the 4 s again.
- **Assertions.** `common\Assertions.h` has the macro `c_assert`, which logs the file, line and expression and
  gives `false`, so that the function can recover.
- **Tests.** 34 unit tests, with `ParchmentGeometryTest.cpp` for the layout arithmetic.
- **Checked, pixels.** Against screenshots of frcp.net from headless Chrome at 100 %: the Draw, Log and Civ Pool
  tabs and the Options, Hotkeys, About, Join Lobby and message dialogs line up without an offset. What differs is
  the rasterization of text below a pixel (DirectWrite against Skia, a mean of about 3 of 255 a channel inside the
  window), content only the desktop has, and dialogs whose height on the web is fractional (Chrome rounds them up
  or down by a pixel; the desktop matches the case of an even height). Menus and tooltips were compared by eye.
- **Checked, behaviour.** The 55 scripted steps of
  [Model, view and controller in place of one file](#model-view-and-controller-in-place-of-one-file), compared by
  control id against the build before. `log.txt` and `settings.ini` are identical byte for byte; every other
  difference is intended, see Decisions. The lobby dialog's error line and growth were driven apart from these.
  No assertion fired.
- **Checked, builds.** x64 and Win32, Debug and Release: zero warnings at `/W4` and zero `/analyze` findings (see
  the correction named in the section before). All 34 unit tests passed.
- **Checked, memory.** Release x64, idle, after the 4 s: the private working set (Task Manager's "Memory") is
  0.1 MB; it was 2.8 MB before the new interface and 41 to 48 MB with it before the memory work. Committed memory
  is 44 MB; it was 4 MB before and 68 to 70 MB with it. Most of what stays committed is Direct2D's software
  renderer, its surfaces and the decoded images (about 13 MB). Captures of the tabs and dialogs before and after
  the memory work match, but for at most 1 of 255 in a few edge pixels of icons, from rounding in the strips.
- **Checked, rules.** Every new function is at most 60 lines, no line is over 120 characters, and functions
  validate their parameters. The new view code has 1.1 assertions a function (1.25 counting only functions of more
  than one line), up from 0.5.

#### Decisions

- **Size.** The main window has the web version's fixed size, 578 x 605 with the burnt edges. It can be moved by
  its title and minimized, not resized or maximized.
- **Desktop-only features** keep their place, with the parchment look: History (a book at the top right of the
  Draw tab), aoe2vils.net (a framed pixel-art button at the bottom left), the menu items Save Preset, Load Preset,
  Quicksave (F5), Quickload (F9) and Exit, and the desktop's hotkeys.
- **Options button.** It is shown on the Draw tab only, as on the web; the old gear was on every tab. Options
  stays one key (F1) or one menu item away on every tab.
- **Left out as web-only.** The switch of hotkey mode, and "When the page opens" (the desktop says "When the
  program starts").
- **Options.** The desktop's options, in the web's words where an option is the same (Show civ name, Show round
  counter, Draw a civ, Show Hotkeys). Options that do not apply are disabled, not hidden, as on the web.
- **Dialogs.** About and Hotkeys keep the desktop's content in the web's layout. Message boxes use the web's
  message dialog. A lobby code that is not valid shows the web's error line.
- **Keyboard.** A dialog opens with its first control focused. Tab follows the layout. A focused control shows the
  web's dotted outline.
- **Sounds.** They are the desktop's; the web's hover and tab sounds were not added.
- **Tooltips.** They use the web's words where the hotkeys are the same. The tech tree's reads "Hotkey: T (Draw tab
  only) / F4" on both versions; the desktop said "Hotkey: T".

#### Obstacles met, and how they were solved

- **Garbled text.** The About dialog's alias and copyright sign were garbled: a UTF-8 source file without a byte
  order mark is read in the system's code page. The text now uses `\u` escapes.
- **Focus.** Dialogs focused their last control and tabbed backwards, since controls were raised when created.
  The dotted focus outline drew nothing (a DOT dash with flat caps). Both were mended.
- **Painting.** A dialog showed black where its controls had not painted yet, and the Civ Pool tab took 250 ms to
  repaint; see Painting above. A disabled dropdown blended over whatever the shared back buffer last held, so the
  main window's menu bar showed through Options' "Jingle type"; it now copies the dialog's backdrop under itself
  first.
- **Other faults found while checking.** Two expressions dereferenced twice; a released control could still answer
  its dying window. Both were mended.
- **Button font.** A blackletter: "Lucida Blackletter", else "Old English Text MT" (installed with Office), else
  Times New Roman, which is the web's font stack. Without the first two the buttons fall back as the web does.
- **Screenshots on a scaled display.** The program is not DPI-aware, as before, so Windows stretches its window.
  `PrintWindow` with `PW_RENDERFULLCONTENT` then gives a resampled picture. Copying from the window's device
  context gives the real pixels, but only while the window is on a screen. `SetWindowPlacement` moves a window
  that would be off every monitor back onto one; `SetWindowPos` keeps it off-screen. On a busy machine the first
  `PrintWindow` of a window just shown off-screen can come back black however long the wait, so a capture is taken
  and thrown away first.
- **Speed.** Direct2D draws in software here. A frame costs about 0.5 ms and a scaled image of high quality 0.3 to
  0.6 ms, so the Civ Pool tab, about 80 controls, repaints in about 50 ms (Release). Scaling the icons once at
  startup would cut that further if needed.
- **Allocation after initialization.** Dialogs create their child windows when they open, Direct2D, DirectWrite
  and GDI allocate while drawing, and Ctrl+C in a log hands the clipboard a `GlobalAlloc` block, as the clipboard
  demands. The program's own code allocates nothing after startup.
- **Lobby field.** `EM_SETCUEBANNER` is not used: the field's placeholder is drawn by a subclass, to get the web's
  colour.

#### Not done

- `CivPickerView.cpp`, `CivPickerDialogs.cpp`, `ParchmentDialog.cpp` (626 lines) and `ParchmentMenu.cpp` (512) are
  over *Clean Code*'s desirable 500 lines, as the controller and the model already were. The next split would move
  the observer notifications out of `CivPickerView.cpp`.
- Two assertions a function on average are not reached (question 7).
- The open dropdown list keeps the system's border of 1 px.
- The folders `images\civ_icons`, `images\dlc_icons` and `images\edition_icons` are no longer used (question 5).

#### Questions

1. **Boolean setters.** `SetChecked(bool)`, `ShowControl(HWND, bool)` and the view's `ShowDrawTab(bool)` and
   `SetDlcChecked(..., bool)` were read as setters of a value, not as flag arguments, which *Clean Code* forbids.
   Should they become pairs such as `Check` and `Uncheck`? Proposal: keep them as they are.
2. **Cases of a message switch.** Each case is one line, `case X: Handler(); return 0;`, as the dispatchers of the
   split already were. Should every handler return the result, so that a case is a single `return Handler();`?
   Proposal: keep the form in use.
3. **Allocations inside the APIs.** After startup Windows, Direct2D and the clipboard allocate on the program's
   behalf (see Obstacles). Should these count as a platform boundary, as the function pointers Win32 demands do?
   Proposal: yes, as one more reading of the rules.
4. **`/Wall`.** It leaves only informational warnings: C4820 (padding), C5045 (Spectre mitigation), C4625, C4626,
   C5026, C5027 and C4623 (implicitly deleted copy, move or default members), C4514 (an unused inline function
   removed) and C4355 (`this` in the controller's member initializers). Should these be disabled, or stay visible?
   Proposal: disable these by name once `/Wall` is set in the project files.
5. **Old icon folders.** `images\civ_icons`, `images\dlc_icons` and `images\edition_icons` hold BMPs that nothing
   uses, yet every build copies them beside the exe. Should they be deleted? Proposal: delete them.
6. **Resizing.** Should the window be resizable like a browser's window, or stay at the web version's size?
   Proposal: keep the fixed size.
7. **Assertion density.** The new view code has 1.1 assertions a function where the Power of Ten asks for 2. Every
   function checks its parameters and the state it relies on; a second check in most small functions would test
   something that cannot fail, which does not count. Should this be accepted for drawing code, or the count be
   raised anyway? Proposal: accept it.
8. **Committed memory.** 0.1 MB shows at idle, but 44 MB stay committed (4 MB in the old build), most of it
   Direct2D's. Storing the large images at the size they are drawn (Options button, tabs, grey civ icons, DLC
   icons) would commit about 8 MB less and paint faster, with a small risk of resampling differences from the web
   version. Should that be done, and is 4 s without input the right wait before the working set is handed back?
   Proposal: left to be decided; nothing was changed.

## 2026-10-14

### Editions, DLCs and civs in data.json only

#### What changed

- **Rules.** A rule was added to `CLAUDE2.md`, binding as the Power of Ten and before the books: strict separation
  of data and code, with `data.json` the only place where editions, DLCs and civs may be. It is the same file as
  the web version's `frcp_html/data.json`, and the two are kept identical.
- **Reading the file.** `JsonReader` tokenizes JSON in one loop with an explicit stack: no recursion, every loop
  bounded, at startup only. `CivCatalog::Load` reads and checks the file: every reference resolves, civ ids run
  from 0 without gaps, a DLC has at most one toggle and a DLC with a toggle has an icon, civ names are unique
  single words other than "Random" (the log file parts a name from its state with a space, and "Random" means no
  civ drawn).
- **Use.** `wWinMain` loads the catalog from the exe's folder before it makes the model. The model, view and
  controller take everything from it: the edition rows ("short (year)"), the DLC toggles (`dlcToggles` at
  `dlcToggleLayout`), the civ grid, the remaining log and the History list (ordered by name, as on the web), the
  tooltips, the Civ Pool list of the Hotkeys dialog, the Game menu (`links`), the edition banner's store page or
  note (`url`, `storeNote`), the tech tree's name (`techTree`), auto-toggle (`olderDlcs`), the edition saved in a
  log (`id`), History's "Added" line (`released`), the About notice (`series` and the first edition's `name`) and
  the jingle type option (the first edition's name).
- **Fields added to `data.json`**, in both copies; the web version ignores them: a DLC's `released` date and an
  edition's `links`, the entries of the Game menu with text and address.
- **Jingles.** They are files in `sounds/civ_jingles`, the legacy ones in `legacy` under it, mapped into memory at
  startup and played from there as the resources were. The Release exe is 0.7 MB; with the jingles as resources
  it was 86 MB. The build copies `data.json` and the jingles beside the exe (`CopyProgramDataToOutDir`); a release
  zip must hold them together with `images`.
- **Removed.** The enums `Edition` and `Dlc`, the table of 56 civs, the control, tooltip and resource ids of each
  DLC, the jingle resources, the unused menu and dialog templates of the `.rc` file (they named editions and
  DLCs), and the stale items of the project files (`civ_icons` entries, two history texts in a Steam folder).
- **Control ids.** DLC toggles, edition rows and Game menu links have ids counted from a base (7900 + DLC,
  7950 + edition, 32800 + link) in place of a constant each.
- **Intended changes of text.** The DLC toggles' tooltips use the web version's words and list the civs as
  `data.json` names them ("Toggles The Last Chieftains", "Rise of the Rajas", "Inca", "Maya"). History says
  "African Kingdoms", `data.json`'s name, where the code said "The African Kingdoms".
- **Tests.** 59 unit tests, 25 of them new, for the reader, the catalog and the model.
- **Checked, names.** A scan with every name in `data.json` finds 232 uses in the sources before the change and
  none after, in sources, resources, project files and tests.
- **Checked, behaviour.** The 55 scripted steps against the build before, its control ids mapped to the new ones:
  no difference, and `log.txt` and `settings.ini` identical byte for byte. The three tabs, the Game menu and the
  Options, Hotkeys, About, Join Lobby and History dialogs are identical pixel for pixel. History picked by place
  in the list and by search shows the same civs and dates as before.
- **Checked, files.** Jingles were drawn with both jingle types, with silent copies of the files: no error. On a
  hidden desktop, without `data.json`, with a broken one, and without the jingles: the expected message each
  time; the program ends without `data.json` and runs without jingles.
- **Checked, builds.** As for the parchment interface, and all 59 unit tests passed. Idle memory is unchanged: a
  private working set of 0.1 MB, and committed memory within the spread of about 1 MB between runs.

#### Decisions

- **No data in code.** No edition, DLC or civ name, id, key, count, list, constant, resource entry, control id or
  layout in code, resources, project files or tests. Code reads them from `CivCatalog` or derives them: a civ's
  jingle and icon files are its name in lower case; an edition's and a DLC's icon files are named by their `icon`
  and `banner`.
- **Limits are capacities, not data** (`CivDataLimits.h`): 3 editions, 9 DLC toggles an edition, 64 civs and 4
  links an edition, set by the room in the main window. A file beyond them, or with a broken reference, is refused
  at startup with a message that says what is wrong, and the program ends.
- **Files named after civs** (jingles, history texts) are data files beside the exe, not code.
- **Tests and data.** The unit tests use the real `data.json`; a file with a fault is `data.json` with one thing
  changed.

#### Not done

- The new files have about 0.8 assertions a function; the functions without one accept every input. See question 7
  of [Parchment interface, as the web version's](#parchment-interface-as-the-web-versions).

#### Questions

1. **"AoE2DE" in the Join Lobby dialog.** The dialog says "Enter AoE2DE lobby code" and "AoE2DE must be already
   running", as the web version does. Lobbies open through the `aoe2de://` links of that game's client, so the
   word was read as the name of the program the feature starts, not as data of an edition. Should it come from
   `data.json` too? Proposal: leave it in the code.
2. **Room for editions and civs.** `data.json` may list at most 3 editions and 64 civs, since the main window has
   room for no more. A fourth edition or a 65th civ would need a change of layout first. Is that accepted?
   Proposal: accept it until such data exists.

### Share links made and opened as on the web version

#### What changed

- **Why.** Links were to work across the two versions: a link made by either opens in the other with the same
  edition, civ pool and draws.
- **Format.** `https://frcp.net/#p=` and a code: the format's version "1", the edition by its place in
  `data.json`, the pool as a bitfield (six civs a base64url digit; bit b of digit c is the civ with id 6c + b),
  and optionally "." and the round's draws, a digit each, in the order drawn. Civs past the end of a link's
  bitfield count as enabled, and civs outside the edition as left out. The format allows 64 civs, which is
  `MAX_CIVS`.
- **Model.** `ShareLink` (plain C++) writes and opens links exactly as `shareCode` and `parseShare` of the web
  version do: a link is read from any text that holds one (trimmed, from after its "#", "p=" optional, percent
  escapes decoded), and anything else is refused. The model gives what a link carries (`GetSharedPool`: the
  edition, the pool, and the round's draws that are still drawn and in the pool), says whether loading one would
  replace something (`HasCustomState`), and loads one (`LoadSharedPool`: the edition without auto-toggle, the
  pool, then a new round with the link's draws). The log of past rounds stays, as on the web.
- **Menu.** File has Copy Share Link (P), Share ... and Open Link ... (U).
- **Buttons.** Share buttons stand where the web version has them: on the Log tab (the pool and this round's
  draws) and on the Civ Pool tab (the pool alone), the wooden nine-slice button with `ui/share_icon.png`.
- **Copying.** The link goes to the clipboard, and the web version's status message shows below the window
  (`ParchmentToast`: "Link copied: civ pool + 3 drawn civs", 2.2 s, fading in and out). If the clipboard refuses,
  the Share dialog opens with the links to copy by hand.
- **Dialogs**, laid out from the web version's CSS: Share (both links in read-only fields with Copy buttons, and a
  note); Open Link (a field, the error line "That isn't a Fresh Random Civ Picker link." that makes the dialog
  grow, Open and Cancel); Open Shared Link (what the link holds, Load Link and Cancel). The preview shows only if
  loading would replace something: another edition than the first, a civ left out, a draw, or anything in the
  drawn log. A loaded link's draws come in without their jingles; then the button sound and "Loaded the shared
  link".
- **Parts made for this.** `PartKind::readonly_field` in the dialogs; `DialogMetrics.h`, the measures that
  `CivPickerDialogs.cpp` and `CivPickerShare.cpp` share; a smooth or pixelated icon in `FramedIconButton`.
- **Tests.** 69 unit tests, 10 of them new, for links and sharing.
- **Checked, against the web version's code.** A scratch harness ran `shareCode` and `parseShare` from
  `frcp_html/script.js` in Node beside `ShareLink` built into a small test program: 3000 random pools gave
  identical links, the desktop opened all 3000 web links to the same pool, and 6010 mutated texts (3589 of them
  still links) were accepted or refused alike, with the same result.
- **Checked, in the program.** A round of three draws, shared, read from the Share dialog and opened by the web
  version's parser gives the same civs in order. A link made by the web version's code, opened through Open Link
  (after the error line for a text that is no link, and the preview), leaves the edition, every civ's check state
  and the round as the link says, and sharing again gives the same link. The status message shows centred 14 px
  below the window and is gone after about 2.4 s. The share buttons and the File menu match screenshots of the
  web version; the Hotkeys dialog lists P and U at its old size. The 55 scripted steps against the build before:
  no difference, and no assertion fired.
- **Checked, builds.** As for the parchment interface; all 69 unit tests passed; idle memory unchanged.

#### Decisions

- **Keys.** P copies the share link and U opens one, as in the web version's browser key set. Ctrl+S and Ctrl+R
  stay Save Preset and Load Preset.

#### Not done

- The write to the clipboard itself was not tested automatically: copying would replace the clipboard of the
  machine at work, and a window station with a clipboard of its own could not be made. It uses the same calls as
  Ctrl+C in the drawn log.

## 2026-10-15

### DLCs added by data.json alone, and original images

#### What changed

- **Why.** Adding a DLC was to need nothing but `data.json`, its jingles and its icons, so the limit of 9 DLCs was
  to rise to 128. The game's original images were to replace the converted WebP images as they became available,
  first the cursors (32 and 48 px), the parchment, the slider and the dialogs' panel. The parchment was never to
  be stretched or resized.
- **Adding a DLC.** In `data.json`: the DLC in `dlcs` with its `icon`, its key at the top of its edition's
  `dlcToggles`, its civs in `civs` with the next ids. Files: `images\dlc\<icon>.png`,
  `images\civ\<civ in lower case>.png`, `sounds\civ_jingles\<civ in lower case>.wav` (and in `legacy\` for an
  edition with legacy jingles), `Histories\<Civ>.txt`. Nothing in code, resources or project files.
- **Icons.** One file each, of any size. Each is fitted into its cell of an atlas with its proportions kept
  (`ContainedSize`; WIC's Fant scaler on premultiplied pixels) when `ParchmentImages::BuildAtlas` builds the
  atlases at startup. The civ atlas holds the civs in the order of their ids and Random after them; the DLC atlas
  three looks of each edition's icon, then the DLCs' icons; the edition banners are stacked. A file that cannot be
  loaded leaves its cell empty, and a message at startup names the first such file.
- **Limits.** `MAX_DLCS` and `MAX_DLC_TOGGLES` are 128. An edition's toggles that do not fit above the civ grid
  scroll (`CivPickerDlcToggles.cpp`): how many show follows from its `dlcToggleLayout`, the rows between its top
  and 6 px above the civ grid (`CivPickerLayout.h`). A `ScrollBar` control with the logs' look, counting rows,
  sits at the right edge of the client area, and the wheel scrolls the list under the mouse.
- **Hotkeys of toggles.** A to L toggle an edition's first nine (`DLC_HOTKEY_COUNT`). Later toggles have no hotkey,
  their tooltips name none, and the Hotkeys dialog lists the nine.
- **Control ids.** DLC toggles moved to 8000 + DLC, since 7900 + 128 would run into the edition rows at 7950; the
  scrollbars are 7960 + edition. `static_assert`s keep the ranges apart.
- **Fixed, a crash at startup.** A 10th DLC toggle made the program vanish at startup, reported as "When adding a
  civ, binary neither opens nor throws error message" and reproduced with the `x64\Release` build and a
  `data.json` with a DLC "new" as the Definitive Edition's 10th toggle. With `MAX_DLC_TOGGLES` raised to 10,
  `CivPickerView::DlcTooltipText` appended `DLC_SLOT_HOTKEY_KEYS[9]`, a null pointer since only A to L exist, to
  a `std::wstring`: an access violation while the main window was built. Toggles past the ninth now have no
  hotkey.
- **Failed startup.** A startup that fails without crashing now says so in a message box; it ended silently.
- **Parchment.** `ui/background.png` (1024 x 1167) is drawn unstretched. Its visible part (alpha above 3: x 26 to
  995, y 13 to 1151, the paper and its burnt edge without the transparent margin) fills the window's width of
  578 px, so the window is 679 px tall, up from 605 (`ParchmentTheme.h`). The content keeps its place, so the
  74 px gained are bare parchment below the client area (question 1).
- **Cursors.** `ui/cursor_x1.png` and `ui/cursor_pointing_x1.png` (32 px) and the `_x2` files (48 px); hotspots
  (0, 0) for the arrow, (2, 0) and (3, 0) for the hand. They are created on a thread switched to per-monitor DPI
  awareness, so Windows shows them at their own size and does not enlarge them for the DPI-unaware window. The
  view asks for the real DPI of the window's monitor (`GetDpiForMonitor`, again on such a thread) when the window
  is made and when it moves to another monitor, and picks 48 px from 144 DPI (150 %) on.
- **Slider.** `ui/slider/` holds the arrows, bar, progress and handle, the arrows and the handle in normal, hover
  and active looks. `LayOutSlider` in `ParchmentGeometry` scales the images so that the handle is as tall as the
  slider (24 px in Options): the arrows at the ends, the bar from arrow centre to arrow centre under them, the
  handle travelling between the arrows. Bar and progress are drawn as three-slices with unstretched ends of 4 px;
  the progress is clipped at the handle's centre. An arrow steps the value, a press on the bar moves the handle
  there, the handle drags.
- **Panel.** `ui/9slice/boxstyle3a_*.png`, a file a tile (the same art as the old `panel_9slice.png` of 192 px,
  alpha identical), loaded by `ParchmentImages::CreatePanelSlices`.
- **Icon files.** `images\civ\*.png` and `images\dlc\*.png` were cut losslessly from the old atlases, so they look
  as before until originals replace them. `tools/convert_web_assets.py` no longer lists the images that are
  originals, and copies a web icon only where none of that name exists, so it never overwrites an original.
- **Scrollbar.** `DrawScrollbar` in `ParchmentScrollBar.cpp` draws the scrollbar of the logs and the DLC lists
  alike.
- **Tests.** 76 unit tests, 7 of them new: the slider's layout and hit test, contained size, icon file names, more
  than nine toggles, the limit of 128 DLCs, an edition's icon and banner required.
- **Checked, builds.** x64 and Win32, Debug and Release: zero warnings at `/W4`; zero `/analyze` findings (x64
  Debug, 28 analysis logs); see the correction on how these were measured in
  [No file over 1000 lines, no folder over 20 files](DEVLOG2.md#no-file-over-1000-lines-no-folder-over-20-files).
  A probe with `/Wall` shows only the warnings of question 4 of
  [Parchment interface, as the web version's](#parchment-interface-as-the-web-versions) and one in a
  header of the Windows SDK. All 76 unit tests passed.
- **Checked, in the program.** On a hidden desktop, with a scratch harness posting clicks and wheel turns to the
  windows: the Draw and Civ Pool tabs and the Options and Hotkeys dialogs; 12 Definitive Edition toggles and 3
  more civs added by `data.json`, icon files and jingle files alone, the list scrolled by wheel, buttons and
  track; 128 DLCs (123 toggles of one edition); a missing icon named at startup; the `data.json` reported as
  failing starts. The slider's arrows step the value and show their active look. No assertion fired.

#### Decisions

- **128 DLCs and 128 DLC toggles an edition**, replacing the 9 of
  [Editions, DLCs and civs in data.json only](#editions-dlcs-and-civs-in-datajson-only). `MAX_CIVS` stays 64.
- **The parchment is never stretched or resized**; the window takes its height from the picture.
- **Windows 10 1607 or later is needed**, since `SetThreadDpiAwarenessContext` is linked directly (question 5).

#### Not done

- **Not seen, since a hidden desktop has no pointer** (controls get `WM_MOUSELEAVE` at once). To check by hand: the
  hover looks of the slider's arrows and handle; dragging the slider's handle and the DLC list's scrollbar knob;
  the cursors on a 4K display at 175 %, which should be 48 px and sharp, not enlarged to about 84 px (if they are
  too big, Windows scales cursors made on a per-monitor aware thread after all); the window on a 1080p display at
  150 %, where at 679 px it only just fits the work area.
- **Files to delete by hand.** Unused since this change, but still copied beside the exe by every build; they were
  left in place: `images\ui\New folder\` (where the originals were staged; its files are copied into place);
  `images\ui\cursor.png`, `cursor_pointing.png`, `slider.png`, `slider_knob.png` and `panel_9slice.png`;
  `images\civ\civicon_atlas.png`, `images\dlc\dlc_atlas.png` and `images\dlc\edition_banners_atlas.png`.
- **64 civs** is still a limit, see question 2.

#### Questions

1. **Bare parchment below the content.** The parchment made the window 74 px taller, all of it bare. Keep it, or
   let the client area use it (taller logs, more rows in the civ grid, more DLC rows before the list scrolls)?
   Proposal: left to be decided; the content was not moved.
2. **64 civs.** It is the next limit a DLC can hit: 56 are used, so three more DLCs of three civs reach it. It is
   set by the share-link format (a draw is one base64url digit) and by the civ grid (13 rows of five). Should it
   be raised with a new version of the share-link format, shared with the web version? Proposal: yes, before the
   third such DLC.
3. **Hotkeys past the ninth toggle.** A to L take an edition's first nine toggles, newest first, so older DLCs
   lose theirs as new ones arrive. Another set of keys for them, or none? Proposal: none.
4. **Cursor size at 125 %.** The 48 px cursors are used from 150 % scaling on. At 125 % the 32 px ones show at
   their own size, smaller than the system's cursors of 40 px. Move the threshold, or let Windows enlarge the
   32 px ones there? Proposal: keep the threshold until the cursors were seen at 125 %.
5. **Windows before 10 1607.** The program no longer starts there. Is that accepted, or should it still start on
   older Windows? Proposal: accept it.

### Continuous freshness counted in civs

#### What changed

- **Why.** The web version's slider steps in civs. The desktop had a strength from 1 to 10, each step a tenth of
  half the pool.
- **Setting.** `contfresh_civs`: how many of the civs drawn last sit out the next draws, at most half the pool
  (`FreshnessMaximum`, at least 1; `FreshnessLimit` is the setting within it). The top of the slider stores
  `CONTFRESH_MOST` (which is `MAX_CIVS`), "the most", which follows the pool's size as `FRESH_MOST` of the web
  version does. The default is the most, as on the web.
- **`settings.ini`.** The key is `ContFreshCivs`. `ContFreshStrengthValue` is read once more, turned into civs (1
  is 1 civ, 10 the most, the steps between a tenth of half the civs) and deleted on the next save.
- **Fixed, the default.** `ContFreshStrengthValue` defaulted to 0 although the slider ran from 1 to 10. The
  strength is gone with this change.
- **Options.** The slider runs from 1 to half the pool. Its label reads "Exclude the last N civs", with "(the
  most)" at the top, or "Needs at least 2 civs in the pool" with the slider greyed. The hint is the web's: "When a
  round ends, these civs sit out the first draws of the next round."
- **Model.** The history keeps the draws of the last half pool, oldest first; the setting picks how many of the
  newest sit out, so a new setting holds from the next draw on, in both directions. The special case for strength
  1 (the previous civ only) is gone: 1 civ does the same. Loading a log replaces the history that the replayed
  draws built with the saved one.
- **Fixed, loading a save.** Loading could drop the newest entries of the freshness history: the log reader
  stopped after `MAX_CIVS*2+4` lines, fewer than civ states + draws + edition + the history, which is written
  oldest first. `LOG_LINE_LIMIT` in `CivPickerModel.cpp` now covers all of them, with half the civs for the
  history.
- **Tests.** 78 unit tests, 2 of them new: the last N civs of a round sit out the first draws of the next round,
  repeated 500 times; the most follows the pool.
- **Checked.** Zero warnings at `/W4` and zero `/analyze` findings. All 78 unit tests passed. On a hidden desktop:
  "Exclude the last 28 civs (the most)" with the full pool, 27 after a click on the left arrow, an old strength of
  4 read as 11, and `settings.ini` saved with `ContFreshCivs=11` and without the old key.

#### Not done

- Two differences from the web version were left as they were, see the questions.

#### Questions

1. **Every civ left sits out.** This happens only after the pool changes in the middle of a round. The desktop
   then draws as it did before this change; the web version lets the oldest ones back in first. Match the web?
   Proposal: yes, so that the two versions draw alike.
2. **Undone draws.** An undone draw stays in the desktop's history of recent civs, while the web version's history
   is its log, which loses the undone draw. Match the web? Proposal: yes, for the same reason.

### Program icon before the title

#### What changed

- **Why.** The window had its icon before the title until the parchment interface; the title row was to read
  "[icon] Fresh Random Civ Picker" again. The web version has no icon there.
- **Drawing.** `CivPickerView::DrawTitleIcon` drew the 16 px frame of the icon `IDI_SMALL` with GDI (`DrawIconEx`,
  which blends its alpha) onto the main window's backdrop, centred in the title row of 30 px where the title used
  to start; the title follows 6 px after it (`TITLE_ICON_SIZE`, `TITLE_ICON_GAP`). The icon is an opaque black
  square with the white question mark, as on the taskbar. A failed load or draw is reported and leaves the title
  alone.
- **Picture.** Later the same day the icon came to be drawn from `images\ui\frcp.png` (500 px, scaled down) in
  place of the `.ico`'s frame; it is cleaner at 100 %.
- **Checked.** Zero warnings at `/W4`, zero `/analyze` findings; captured on a hidden desktop.

#### Not done

- `images\ui\frcp.png` was not tracked by git and was left to be added.
- The icon stays soft on a scaled display, see the question.

#### Questions

1. **A sharp icon on a scaled display.** The program is not DPI-aware, so at 175 % Windows enlarges the whole
   window, the icon with it. A truly sharp icon needs either a per-monitor DPI-aware program, a large change, or
   a small per-monitor aware layered window for the icon over the title row. Which, if any? Proposal: neither for
   the icon alone.
