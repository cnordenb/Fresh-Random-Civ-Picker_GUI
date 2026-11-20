# Decisions

- [Rules, and how they are read](#rules-and-how-they-are-read)
- [Requirements, and how they are read](#requirements-and-how-they-are-read)
- [Code and files](#code-and-files)
- [Tests](#tests)
- [Documents](#documents)
- [The earlier rules in CLAUDE2.md, and how they were read](#the-earlier-rules-in-claude2md-and-how-they-were-read)
- [The interface](#the-interface)
- [Hotkeys](#hotkeys)
- [Drawing, sharing and settings](#drawing-sharing-and-settings)

The decisions recorded in the development log in [DEVLOG/](DEVLOG/DEVLOG1.md), by subject (Rule 23). Each ends with the
day it was made, and the time where the log has one, as a link to its entry in the log. Where a later decision replaced
an earlier one, the earlier one is named after it, with its own link.

## Rules, and how they are read

- **The template's rules win.** Rules 0 to 26 stand; an earlier rule that contradicted one was dropped.
  [2026-11-20 07:07:00](DEVLOG/DEVLOG2.md#070700-the-repository-in-the-form-of-the-repository-template)
- **The earlier rules are Rules 27 to 105,** condensed, one paragraph each, with their source tags.
  [2026-11-20 07:07:00](DEVLOG/DEVLOG2.md#070700-the-repository-in-the-form-of-the-repository-template)
- **Asset folders.** Rules 14 and 15 do not apply to `images/` and `sounds/` of the program's folder.
  [2026-11-20 07:07:00](DEVLOG/DEVLOG2.md#070700-the-repository-in-the-form-of-the-repository-template)
- **Not code (Rule 26).** `FRCP_GUI.rc`, `Resource.h`, `*.sln`, `*.vcxproj`, `*.filters`, `build.bat` and
  `convert_web_assets.py`.
  [2026-11-20 07:07:00](DEVLOG/DEVLOG2.md#070700-the-repository-in-the-form-of-the-repository-template)

## Requirements, and how they are read

## Code and files

- **Folders of at most 7 files.** The code is in `common/`, `controller/`, `model/catalog/`, `model/picker/`,
  three folders under `view/` and six under `parchment/`. Replaced: at most 20 files a folder.
  [2026-10-16](DEVLOG/DEVLOG2.md#no-file-over-1000-lines-no-folder-over-20-files)
  [2026-11-20 07:07:00](DEVLOG/DEVLOG2.md#070700-the-repository-in-the-form-of-the-repository-template)
- **Code files of at most 400 lines,** split into parts named after their class and a topic, the code moved as
  it is. Replaced: at most 1000 lines a file.
  [2026-10-16](DEVLOG/DEVLOG2.md#no-file-over-1000-lines-no-folder-over-20-files)
  [2026-11-20 07:07:00](DEVLOG/DEVLOG2.md#070700-the-repository-in-the-form-of-the-repository-template)
- **No comments.** What a maintainer needs of the removed comments is in `MAINTAINER.md`.
  [2026-11-20 07:07:00](DEVLOG/DEVLOG2.md#070700-the-repository-in-the-form-of-the-repository-template)
- **Model, view and controller.** The model holds all data, state and logic, is plain C++ without `<windows.h>` and
  knows nothing of the other two. The view only presents and never changes the model. The controller takes the
  input and holds no logic of the application. Everything is built and wired in `wWinMain`; there are no globals.
  [2026-10-13](DEVLOG/DEVLOG1.md#development-rules-compiled-from-three-books)
- **Editions, DLCs and civs are in `data.json` only.** No name, id, key, count, list, constant, resource entry,
  control id or layout of theirs in code, resources, project files or tests; code reads them from `CivCatalog`.
  The file is kept identical to the web version's.
  [2026-10-14](DEVLOG/DEVLOG1.md#editions-dlcs-and-civs-in-datajson-only)
- **Files named after civs are data.** A civ's jingle and icon files are its name in lower case; jingles and
  history texts are files beside the exe.
  [2026-10-14](DEVLOG/DEVLOG1.md#editions-dlcs-and-civs-in-datajson-only)
- **Limits are capacities, not data.** 3 editions, 128 DLCs, 128 DLC toggles an edition, 64 civs, 4 links an
  edition. A `data.json` beyond them, or with a broken reference, is refused at startup with a message, and the
  program ends. Replaced: 9 DLC toggles an edition.
  [2026-10-14](DEVLOG/DEVLOG1.md#editions-dlcs-and-civs-in-datajson-only)
  [2026-10-15](DEVLOG/DEVLOG1.md#dlcs-added-by-datajson-alone-and-original-images)
- **Windows 10 1607 or later is needed**, for the cursors' per-monitor DPI awareness.
  [2026-10-15](DEVLOG/DEVLOG1.md#dlcs-added-by-datajson-alone-and-original-images)
- **No file over 1000 lines**, whatever it holds: sources, headers, project files, data and rule files.
  [2026-10-16](DEVLOG/DEVLOG2.md#no-file-over-1000-lines-no-folder-over-20-files)
- **No folder over 20 files.** The files directly in it count, tracked or not. Asset folders and folders that
  only builds and tools fill are exempt.
  [2026-10-16](DEVLOG/DEVLOG2.md#no-file-over-1000-lines-no-folder-over-20-files)
- **Small files stay.** A file under 250 lines is not merged into another.
  [2026-10-16](DEVLOG/DEVLOG2.md#no-file-over-1000-lines-no-folder-over-20-files)
- **How a file or folder is split.** A file along its sections, into files named after its class, the code moved
  verbatim; a folder by what its files depend on.
  [2026-10-16](DEVLOG/DEVLOG2.md#no-file-over-1000-lines-no-folder-over-20-files)
- **Folders by role.** `common`, `model`, `controller`, `view` and three Parchment folders. Nothing in `model` or
  `common` includes from the view, the controller or Parchment, and no Parchment folder includes from one that
  builds on it. [2026-10-16](DEVLOG/DEVLOG2.md#no-file-over-1000-lines-no-folder-over-20-files)
- **The warning level is `/W4` in the project files.** The rest of the backlog of the split (code analysis in the
  build, `/Wall`, assertions, return values, heap after startup) was recorded, not worked off.
  [2026-10-16](DEVLOG/DEVLOG2.md#no-file-over-1000-lines-no-folder-over-20-files)

## Tests

- **The tests are documented as they are.** None was changed to meet Rules 5 and 25.
  [2026-11-20 07:07:00](DEVLOG/DEVLOG2.md#070700-the-repository-in-the-form-of-the-repository-template)
- **The tests run with `bash UnitTesting_FRCP/run-tests.sh`,** under g++ with GoogleTest 1.14.0, locally and on
  GitHub; its order numbers the tests in `TESTS.md`.
  [2026-11-20 07:07:00](DEVLOG/DEVLOG2.md#070700-the-repository-in-the-form-of-the-repository-template)
- **Test code may use the heap** in its fixtures, but follows every other rule of cleanliness and compiles without
  warnings. [2026-10-13](DEVLOG/DEVLOG1.md#development-rules-compiled-from-three-books)
- **The unit tests use the real `data.json`.** A file with a fault is `data.json` with one thing changed.
  [2026-10-14](DEVLOG/DEVLOG1.md#editions-dlcs-and-civs-in-datajson-only)

## Documents

- **The earlier log and known issues are in the log,** rewritten in its form; `CLAUDE2.md`, `CLAUDE2_LOG.md`,
  `formatting_notes.md` and `ISSUES.md` are deleted.
  [2026-11-20 07:07:00](DEVLOG/DEVLOG2.md#070700-the-repository-in-the-form-of-the-repository-template)
- **The security contact** stays a placeholder until the first release.
  [2026-11-20 07:07:00](DEVLOG/DEVLOG2.md#070700-the-repository-in-the-form-of-the-repository-template)
- **`CLAUDE.md` is written from the two rule files**, `CLAUDE2.md` and its work log.
  [2026-10-16](DEVLOG/DEVLOG2.md#no-file-over-1000-lines-no-folder-over-20-files)

## The earlier rules in CLAUDE2.md, and how they were read

- **The Power of Ten comes first.** It overrides *Clean Code* and *Head First Design Patterns* where they conflict.
  The rules on `data.json` and on files and folders bind as it does; the Goddard standard ranks below all of them.
  [2026-10-13](DEVLOG/DEVLOG1.md#development-rules-compiled-from-three-books)
  [2026-10-16](DEVLOG/DEVLOG2.md#goddard-c-standard-adopted-and-the-code-brought-in-line)
- **The code as it is sets the standard** where the books leave a detail open: functions in `PascalCase`, variables
  and parameters in `snake_case`, constants in `UPPER_SNAKE_CASE`.
  [2026-10-13](DEVLOG/DEVLOG1.md#development-rules-compiled-from-three-books)
- **No `throw`.** Failure is reported through return values that every caller checks. A command returns a status
  and nothing else; an operation that can fail returns a small result struct, not output arguments.
  [2026-10-13](DEVLOG/DEVLOG1.md#development-rules-compiled-from-three-books)
- **Everything is made at startup.** After it: no `new`, no growing containers, no allocating string operations,
  no new threads, nothing made lazily. "Startup" is everything in `wWinMain` before the message loop.
  [2026-10-13](DEVLOG/DEVLOG1.md#development-rules-compiled-from-three-books)
- **Virtual functions are allowed, function pointers are not**, except those Win32 demands (window, dialog and
  subclass procedures, enumeration callbacks, thread entry points), which stay thin.
  [2026-10-13](DEVLOG/DEVLOG1.md#development-rules-compiled-from-three-books)
- **No recursion.** Trees are walked with an explicit, bounded stack. Re-entrancy that Windows causes is a platform
  boundary, but no `SendMessage` goes to a window from inside that window's handler.
  [2026-10-13](DEVLOG/DEVLOG1.md#development-rules-compiled-from-three-books)
- **Loops are bounded**, all but the message loop and the modal loops Windows runs.
  [2026-10-13](DEVLOG/DEVLOG1.md#development-rules-compiled-from-three-books)
- **60 lines is the ceiling of a function**, and assertions and parameter checks are kept even where they make a
  function longer. [2026-10-13](DEVLOG/DEVLOG1.md#development-rules-compiled-from-three-books)
- **One level of dereferencing** counts the dereferences written in the source; the implicit `this` does not count.
  [2026-10-13](DEVLOG/DEVLOG1.md#development-rules-compiled-from-three-books)
- **The most pedantic warning setting is `/Wall` with `/WX`**, the Windows SDK's headers set apart. An
  informational warning is never disabled silently.
  [2026-10-13](DEVLOG/DEVLOG1.md#development-rules-compiled-from-three-books)
- **The Goddard standard holds only where it contradicts nothing earlier.** Where the code used one form throughout,
  that form stays; where the code was mixed, the standard's rule was adopted. What it calls optional stays
  optional. [2026-10-16](DEVLOG/DEVLOG2.md#goddard-c-standard-adopted-and-the-code-brought-in-line)
- **Left out of the Goddard standard:** camelCase names, type encodings in names, exceptions, `T *p`, `NULL`,
  `.hpp`, braces always on their own line, alignment in columns, explicit comparison of booleans, lines of 80
  characters, prologs and mandated comments, Get and Set methods, include guard macros, the four special members
  in every class. [2026-10-16](DEVLOG/DEVLOG2.md#goddard-c-standard-adopted-and-the-code-brought-in-line)
- **Readings of the Goddard standard.** Classes stay at global scope. A single statement may share the line of its
  `if`. A Win32 `BOOL` is tested directly. Every switch ends with a default case. Structs may be returned by
  value. The `CountOf` helpers stay. System headers go lower levels first. Generated files keep their tool's
  format. Each main procedure has its own file. Includes name files only.
  [2026-10-16](DEVLOG/DEVLOG2.md#goddard-c-standard-adopted-and-the-code-brought-in-line)

## The interface

- **The program looks like the web version at frcp.net.** Desktop-only features (History, aoe2vils.net, Save and
  Load Preset, Quicksave, Quickload, Exit, the desktop's hotkeys) keep their place, with the parchment look.
  Web-only things are left out: the switch of hotkey mode, "When the page opens".
  [2026-10-13](DEVLOG/DEVLOG1.md#parchment-interface-as-the-web-versions)
- **The window has a fixed size**, 578 px wide, and takes its height from the parchment picture, which is never
  stretched or resized: 621 px. It can be moved and minimized, not resized or maximized. Replaced: 605 px, the
  web version's height; then 679 px with the first original picture.
  [2026-10-13](DEVLOG/DEVLOG1.md#parchment-interface-as-the-web-versions)
  [2026-10-15](DEVLOG/DEVLOG1.md#dlcs-added-by-datajson-alone-and-original-images)
  [2026-10-18](DEVLOG/DEVLOG2.md#shorter-parchment)
- **The Options button is on the Draw tab only**, as on the web; F1 and the menu open Options from every tab.
  [2026-10-13](DEVLOG/DEVLOG1.md#parchment-interface-as-the-web-versions)
- **Options are the desktop's, in the web's words** where an option is the same. Options that do not apply are
  disabled, not hidden. [2026-10-13](DEVLOG/DEVLOG1.md#parchment-interface-as-the-web-versions)
- **Dialogs.** About and Hotkeys keep the desktop's content in the web's layout; message boxes use the web's message
  dialog; a lobby code that is not valid shows the web's error line. A dialog opens with its first control
  focused, Tab follows the layout, and a focused control shows the dotted outline.
  [2026-10-13](DEVLOG/DEVLOG1.md#parchment-interface-as-the-web-versions)
- **Sounds are the desktop's.** The web's hover and tab sounds are not added.
  [2026-10-13](DEVLOG/DEVLOG1.md#parchment-interface-as-the-web-versions)
- **Tooltips use the web's words** where the hotkeys are the same, and DLC tooltips name civs as `data.json` does.
  [2026-10-13](DEVLOG/DEVLOG1.md#parchment-interface-as-the-web-versions)
  [2026-10-14](DEVLOG/DEVLOG1.md#editions-dlcs-and-civs-in-datajson-only)
- **The game's original images are used** where the program has them, and the converter never overwrites one.
  [2026-10-15](DEVLOG/DEVLOG1.md#dlcs-added-by-datajson-alone-and-original-images)
- **DLC toggles that do not fit scroll.** Only an edition's first nine have a hotkey.
  [2026-10-15](DEVLOG/DEVLOG1.md#dlcs-added-by-datajson-alone-and-original-images)
- **The program's icon stands before the title**, though the web version has none.
  [2026-10-15](DEVLOG/DEVLOG1.md#program-icon-before-the-title)
- **All of the content is the window's own**, on an opaque underlay; only the burnt edge outside it belongs to the
  skin. [2026-10-16](DEVLOG/DEVLOG2.md#content-never-under-the-skin)
- **The logs of the Log tab show civ icons**, and a drawn entry reads "<civ> (<k>/<n>)". The History dialog's log
  has none. [2026-10-16](DEVLOG/DEVLOG2.md#civ-icons-in-the-logs)
- **An open dropdown list has the menu's border colour**, #3d2c1c, not the theme's frame.
  [2026-10-16](DEVLOG/DEVLOG2.md#open-dropdown-list-framed-as-on-the-web-version)

## Hotkeys

- **P copies the share link and U opens one.** Ctrl+S and Ctrl+R stay Save Preset and Load Preset.
  [2026-10-14](DEVLOG/DEVLOG1.md#share-links-made-and-opened-as-on-the-web-version)
- **Every hotkey can be changed by the user**, in a dialog of its own under Options > Hotkeys > Customise; the
  Hotkeys dialog stays a read-only summary. Each hotkey has one key.
  [2026-10-17](DEVLOG/DEVLOG2.md#hotkeys-the-user-can-change)
- **A keystroke in use is taken over, not refused.** The status line names the hotkeys that lost it, and Cancel
  undoes everything. [2026-10-17](DEVLOG/DEVLOG2.md#hotkeys-the-user-can-change)
- **Keys that cannot be hotkeys.** Plain Backspace (it means "no key" while a key listens), punctuation keys
  (their characters depend on the keyboard's layout), F12 and the keystrokes Windows keeps. Esc, Tab and Enter
  can. [2026-10-17](DEVLOG/DEVLOG2.md#hotkeys-the-user-can-change)
- **Draw civ and Reset** of the Draw Civ tab and of the Log tab are separate hotkeys.
  [2026-10-17](DEVLOG/DEVLOG2.md#hotkeys-the-user-can-change)
- **F6 does not quicksave.** [2026-10-17](DEVLOG/DEVLOG2.md#hotkeys-the-user-can-change)

## Drawing, sharing and settings

- **At most 10 draws a second** by Space and the Draw button.
  [2026-10-13](DEVLOG/DEVLOG1.md#at-most-10-draws-a-second-and-images-found-beside-the-exe)
- **Images are loaded from the exe's folder**, not from the working directory.
  [2026-10-13](DEVLOG/DEVLOG1.md#at-most-10-draws-a-second-and-images-found-beside-the-exe)
- **Quirks of the old program are kept** where mending them would change the behaviour, until a fix is decided.
  [2026-10-13](DEVLOG/DEVLOG1.md#model-view-and-controller-in-place-of-one-file)
- **Share links are the web version's**, in format and in how they are written and read, so that a link made by
  either version opens in the other. [2026-10-14](DEVLOG/DEVLOG1.md#share-links-made-and-opened-as-on-the-web-version)
- **Continuous freshness is a number of civs**, at most half the pool, by default the most, as on the web. An old
  strength in `settings.ini` is turned into civs once. Replaced: a strength from 1 to 10.
  [2026-10-15](DEVLOG/DEVLOG1.md#continuous-freshness-counted-in-civs)
