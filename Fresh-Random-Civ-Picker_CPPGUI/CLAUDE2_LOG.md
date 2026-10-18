# CLAUDE2_LOG.md - The work log of this project's rules

What was built when, how it was verified, the decisions taken and the questions still open for the user. The
rules themselves are in `CLAUDE2.md` beside this file; the two files share one numbering (CLAUDE2.md, top).
Sections are added in the order the work was done; an entry describes the code as it was then, and a later
section says what changed since.

---------------------------------------------------------------------------------------------------

## 8. Current state of the code (updated 2026-10-13, after the MVC split)

`FRCP_GUI.cpp`/`FRCP_GUI.h` were split into `CivPickerModel`, `CivPickerView` and
`CivPickerController` (.h/.cpp), following section 6. Verified: zero warnings at `/W4` and zero
findings under MSVC Code Analysis (`/analyze`); every function is under 60 lines; an automated
old-versus-new comparison (55 scripted steps, every control's text, visibility, check state and
position, plus the saved `log.txt` and `settings.ini`) found no differences; the unit test project
builds and passes.

Still open (next incremental steps):

- P10-10: both project files set `WarningLevel` `Level4` since 2026-10-16 (16.1), but no code analysis is
  configured, so /analyze is not enforced by the build, and /Wall waits for question 4 of 9.5.
  `LanguageStandard` is not set.
- P10-5: the `c_assert` macro exists since section 9 (`common\Assertions.h`), but the average of two
  assertions per function is not reached (about 1.1 in the view, 9.2; question 7 of 9.5).
- P10-7: Win32 return values in the view and the controller are mostly neither checked nor
  `(void)`-cast.
- P10-3: `std::wstring` is used for run-time state, log text and labels, so the program still
  allocates on the heap after startup. The model's name index (`std::unordered_map`) is built once
  at startup and only read afterwards.
- `GlobalConstants.h` used `#define` constants (CC J3) until 2026-10-16; they are `constexpr` now
  (section 14.5).
- Quirks kept on purpose because changing them would change behaviour; fix only when asked:
  - removing a civ before the pool was ever initialised underflows `iterator`
    (`CivPickerModel::RemoveCiv`);
  - the History dialog's search remembers the previous text length across openings;
  - changing the jingle type in Options plays the jingle with the previous setting;
  - `IsValidLobbyCode` does not check the first digit of a 20-character lobby link;
  - the B hotkey never plays its button sound when it turns UI sounds on;
  - `T` and `F5`/`F6` are registered twice under the same hotkey ids;
  - the manual-save dialog uses `OFN_FILEMUSTEXIST`.
- Fixed during the split (undefined behaviour or leaks, no visible change): the centre label's
  narrow-string conversion; file-dialog buffer sizes given in bytes instead of characters; the bold
  font being recreated (and leaked) on every Hotkeys dialog; the History search sending
  `WM_COMMAND` to its own dialog (I7); out-of-range civ indices now stop the operation instead of
  writing past the arrays; dead code removed (unused globals and functions, commented-out code).
- Correction to an earlier note: the old code never created a `std::thread`; that line was
  commented out.
- Draw rate cap (2026-10-13): Space and the Draw button draw at most 10 times per second
  (`CivPickerController::DrawCivWithinRateCap`). Each jingle is a new async `sndPlaySound`; Windows
  keeps about 1 MB per interrupted jingle and frees it slowly, so holding Space reached 1.5 GB.
  Measured flat at 10 per second, climbing at about 16 per second.
- Images (2026-10-13): loaded from `<exe folder>\images\` instead of the working directory, and the
  vcxproj target `CopyImagesToOutDir` copies `images\` next to the exe on every build. The aoe2vils.net
  button shows `images\aoe2vils.png` (12 x 12 pixel art), enlarged without smoothing. Since the
  parchment overhaul (section 9) every image is a PNG decoded through WIC; GDI+ is no longer used.

## 9. Parchment UI: the web version's overhaul (2026-10-13)

The view was rewritten so the program looks like the web version at frcp.net after its UI overhaul.
The model and the controller keep their roles; the controller gained Quicksave/Quickload menu
commands and the new links. No commit has been made.

### 9.1 How the view is built now

- `ParchmentRenderer` is the only place that talks to Direct2D, DirectWrite and WIC (CC8). Two
  software DC render targets share every resource: an opaque one (ClearType text, as Chrome draws
  on an opaque page) and a premultiplied one for layered windows. `ParchmentImages` decodes the PNGs
  and builds the nine-slices and cursors; `ParchmentFonts` holds the text formats and measures text.
- `ParchmentGeometry` is plain C++ (no Win32, no Direct2D) with all layout arithmetic: CSS
  border-image tiling, nine-slices, scrollbars, sliders, text wrapping, tooltip placement, box
  shadows, image filters, dotted outlines. It is unit-tested in `UnitTesting_FRCP`
  (`ParchmentGeometryTest.cpp`, 33 tests with the model's).
- `ParchmentShell` gives a window the web version's shape: the window region is the image's fully
  opaque pixels, and an owned click-through layered window ("skin") shows the burnt edges or the
  dialog's shadow. A second layer dims a window while a dialog is open over it (15 % per dialog,
  like the web's backdrop). The main window is a borderless popup moved by its title row.
- `ParchmentControl` is the base of every control (button, checkbox, tab strip, civ and DLC toggle,
  label, slider, link, picture); it behaves like a Win32 button towards the controller (`BM_*`,
  `WM_COMMAND`/`BN_CLICKED`, `TBM_*` and `WM_HSCROLL` for the slider), so the controller's control
  IDs and logic are unchanged. `ParchmentLog` is the scrolling text log, `ParchmentMenuBar` the menu
  bar with its popup lists, `ParchmentTooltip` the tooltip, `ParchmentDialogs` builds every dialog
  from a data layout (`DialogLayout`) over an empty in-memory template.
- `CivPickerView` is split over `CivPickerView.cpp` (window, observer notifications, sounds; 985
  lines), `CivPickerViewLayout.cpp` (the main window's controls and menus) and
  `CivPickerViewDialogs.cpp` (dialog layouts and procedures; 811 lines; `CivPickerDialogs.cpp` until
  14.5). These two, `ParchmentDialog.cpp` (626) and
  `ParchmentMenu.cpp` (512) are above CC's desirable 500 lines, as the controller and model already
  were; the next split would move the observer notifications out of `CivPickerView.cpp` (done on
  2026-10-16, 16.1).
- Painting: a control draws over a copy of its parent's backdrop into a shared back buffer and is
  shown in one step. Every sibling control waiting for its paint is drawn in the same Direct2D frame
  (a frame costs about as much as drawing a control), and a window paints its waiting controls
  before itself, so a tab switch appears at once. A window that has just shown fills itself with
  its backdrop before its controls paint, so it never shows black where controls are still painting.
- Assets: the game's original images where the program has them (section 12), otherwise
  `tools/convert_web_assets.py` converts the web version's WebP images in `..\frcp_html` to PNG
  (`images\ui`, `images\civ`, `images\dlc`). The CSS filters the web applies (grey icons, dimmed
  tabs, tooltip tint) are applied once when the images load.
- Memory: windows are shaped one at a time in one `ShellWorkspace` (canvas, shadow coverage, region
  rectangles) allocated at startup; a shell keeps only its window's backdrop, and the layered
  windows keep their own copies of their pictures. Images are decoded in strips of 48 K pixels
  straight into their Direct2D bitmaps, and the parchment background is released once the main
  window's shape and backdrop hold it. After 4 s without input the view hands its working set back
  to Windows (`SetProcessWorkingSetSize(-1, -1)`, what Windows does to a minimized window); the next
  paint pages back what it needs. Each key, click, hotkey or closed dialog restarts the 4 s.

### 9.2 Verification

- Pixel comparison against headless Chrome screenshots of frcp.net at 100 %: the Draw, Log and Civ
  Pool tabs and the Options, Hotkeys, About, Join Lobby and message dialogs line up with no offset;
  what differs is sub-pixel text rasterization (DirectWrite versus Skia, a mean of about 3 of 255
  per channel inside the window), content that is desktop-specific, and dialogs whose web height is
  fractional (Chrome rounds them up or down by one pixel depending on the browser window's height;
  the desktop matches the even-height case). Menus and tooltips were compared by eye.
- Behaviour: the 55 scripted steps of the old-versus-new harness compared by control ID. The
  `log.txt` and `settings.ini` the program writes are byte-identical to the previous build's; every
  remaining difference is intended (9.3). The lobby's error line and growth were driven separately.
  No assertion fired during any of these runs.
- Builds: x64 and Win32, Debug and Release, zero warnings at `/W4`, zero findings under `/analyze`.
  Unit tests: 34 pass.
- Found and fixed while verifying: the About dialog's author alias and copyright sign were garbled (a
  UTF-8 source without a byte order mark is read in the system code page; the text now uses `\u`
  escapes); dialogs focused their last control and tabbed backwards (controls were raised on
  creation); the dotted focus outline drew nothing (a DOT dash with flat caps); a dialog showed
  black where its controls had not painted yet; the Civ Pool tab took 250 ms to repaint; two
  expressions dereferenced twice (P10-9); a released control could still answer its dying window; a
  disabled dropdown blended over whatever the shared back buffer last held (the main window's menu
  bar showed through Options' "Jingle type"), now it copies the dialog's backdrop under itself first.
- Memory (Release x64, idle, after the 4 s trim): Task Manager's "Memory" column (the private
  working set) shows 0.1 MB; it was 2.8 MB before the overhaul and 41 to 48 MB after it. Committed
  memory is 44 MB; it was 4 MB before and 68 to 70 MB after. Most of what stays committed is
  Direct2D's software renderer, its surfaces and the decoded images (about 13 MB). Tabs and
  dialogs captured before and after the memory changes match (at most 1 of 255 in a few icon edge
  pixels, from rounding in the strip decoding).
- Rules: every new function is at most 60 lines, lines are at most 120 characters, and functions
  validate their parameters (P10-7). Assertion density in the new view code is 1.1 per function
  (1.25 counting only functions longer than one line), up from 0.5; P10-5's average of 2 is not
  reached, see question 7.

### 9.3 Decisions taken without asking (the user asked not to be interrupted; change any of them)

- The main window has the web version's fixed size (578 x 605 including the burnt edges); it can be
  moved by its title and minimized, not resized or maximized.
- Desktop-only features keep their place with the parchment look: History (book, top right of the
  Draw tab), aoe2vils.net (framed pixel-art button, bottom left), the desktop's menu items (Save and
  Load Preset, Quicksave F5, Quickload F9, Exit) and its hotkeys.
- The Options button is shown on the Draw tab only, as on the web; the old gear was on every tab.
  Options stays one key (F1) or one menu item away on every tab.
- Left out because they are web-only: the hotkey mode switch, "When the page opens" (the desktop
  says "When the program starts"). Share links were added later (section 11).
- Options: the desktop's options with the web's wording where an option is the same (Show civ name,
  Show round counter, Draw a civ, Show Hotkeys). Continuous freshness counts civs as on the web
  ("Exclude the last N civs", section 12.5; it was a strength from 1 to 10 until 2026-10-15).
  Options that don't apply are disabled instead of hidden, as on the web.
- About and Hotkeys keep the desktop's content in the web's layout; message boxes (e.g. the Age of
  Kings store note) use the web's message dialog; an invalid lobby code shows the web's error line.
- Keyboard: a dialog opens with its first control focused (the lobby code field, like the web's
  autofocus); Tab follows the layout; focused controls show the web's dotted outline.
- Sounds are the desktop's; the web's hover and tab sounds were not added.
- Tooltips use the web's wording where the hotkeys are the same; the tech tree's now reads "Hotkey: T
  (Draw tab only) / F4" on both versions (the desktop said "Hotkey: T").

### 9.4 Obstacles and notes

- The button font is a blackletter: "Lucida Blackletter", else "Old English Text MT" (installed
  with Office), else Times New Roman, the web's font stack. Without either font the buttons fall
  back like the web does on such a machine.
- The program is DPI-unaware, as before; on a scaled display Windows stretches the window. Note for
  screenshots: `PrintWindow` with `PW_RENDERFULLCONTENT` returns a resampled picture on a scaled
  display; copying from the window's device context gives the real pixels, but only while the
  window is on a screen (off-screen it returns black or stale pixels). `SetWindowPlacement` moves a
  window that would be off every monitor back onto one; use `SetWindowPos` to keep it off-screen.
  On a busy machine the first `PrintWindow` of a newly shown off-screen window can come back black
  however long one waits; a second capture is correct, so take a throwaway capture first.
- Speed: Direct2D draws in software here. A frame costs about 0.5 ms and a high-quality scaled image
  0.3 to 0.6 ms, so the Civ Pool tab (about 80 controls) repaints in about 50 ms (Release). Scaling
  the icons once at startup would cut that further if needed.
- The jingles used to be resources of the exe (86 MB); since the data separation (section 10) they
  are files next to it and the Release exe is 0.7 MB.
- Allocation after initialization (P10-3): dialogs create their child windows when they open,
  Direct2D/DirectWrite/GDI allocate internally while drawing, and Ctrl+C in a log hands the clipboard
  a `GlobalAlloc` block (the clipboard API requires it). The program's own code allocates nothing
  after startup; see question 3.
- Not used: `EM_SETCUEBANNER` (the lobby field's placeholder is drawn by a subclass to get the web's
  colour). The open dropdown list kept the system's 1 px list border until 12.9.

### 9.5 Questions for the user

1. Boolean value setters such as `SetChecked(bool)`, `ShowControl(HWND, bool)` and the existing
   view API (`ShowDrawTab(bool)`, `SetDlcChecked(..., bool)`) are read as values, not flag arguments
   (CC F3). Agreed, or should they become pairs (`Check`/`Uncheck`)?
2. A message switch writes each case as one line, `case X: Handler(); return 0;` (the handler call
   and the statement that ends the case), as the MVC split's dispatchers do. Agreed, or should every
   handler return the result so each case is a single `return Handler();`?
3. API-internal allocations after startup (9.4): treat them like the Win32 function pointers of I2,
   as a platform boundary (a new interpretation I9)?
4. `/Wall` (I8) leaves only informational warnings: C4820 (padding), C5045 (Spectre mitigation),
   C4625/C4626/C5026/C5027/C4623 (implicitly deleted copy, move or default members), C4514 (an
   unused inline function removed), C4355 (`this` in the controller's member initializers). Disable
   these, or keep them visible?
5. Now unused: `images\civ_icons`, `images\dlc_icons`, `images\edition_icons` (BMPs, still copied
   next to the exe with the other images). Delete them? (The `.rc` dialog templates and menu were
   deleted with the data separation: they named editions and DLCs, which section 10 forbids.)
6. Should the window be resizable like a browser window, or stay at the web version's size?
7. P10-5 asks for two assertions per function on average. The new view code has 1.1: every function
   checks its parameters and the state it relies on, but a second check in most small functions would
   test something that cannot fail, which P10 says does not count. Accept this for drawing code, or
   should the count be raised anyway?
8. Memory (9.2): Task Manager shows 0.1 MB at idle, but 44 MB stay committed (the old build: 4 MB),
   most of it Direct2D's. Storing the large images at the size they are drawn (Options button, tabs,
   grey civ icons, DLC icons) would commit about 8 MB less and paint faster, with a small risk of
   resampling differences against the web version. Do it? And is 4 s without input the right wait
   before the working set is handed back?

## 10. Data and code: how it was built (2026-10-14)

The rule itself and what it means here (10.1) are in CLAUDE2.md.

### 10.2 How it is built

- `JsonReader` tokenizes JSON in one loop with an explicit stack (no recursion, every loop bounded,
  at startup only; unit-tested). `CivCatalog::Load` reads and checks the file: every reference
  resolves, civ IDs run from 0 without gaps, each DLC has at most one toggle and a toggled DLC has an
  icon, civ names are unique words other than "Random" (the log file separates a name from its state
  with a space; "Random" means no civ drawn).
- `wWinMain` loads the catalog from the exe's folder before it makes the model. The model, view and
  controller take every edition, DLC and civ from it: edition rows ("short (year)"), DLC toggles
  (dlcToggles at dlcToggleLayout), the civ grid and the remaining log and History list (in the order
  of names, as the web version), tooltips (worded as the web version's), the Hotkeys dialog's Civ Pool
  list, the Game menu (`links`), the edition banner's store page or note (`url`, `storeNote`), the
  tech tree name (`techTree`), auto-toggle (`olderDlcs`), the edition saved in a log (`id`), History's
  "Added" line (`released`), the About notice (`series`, the first edition's `name`) and the jingle
  type option (the first edition's name).
- Fields the desktop added to data.json (in both copies; the web version ignores them): a DLC's
  `released` date and an edition's `links` (the Game menu's entries, text and URL).
- The jingles are files in `sounds/civ_jingles` (the legacy ones in `legacy`), mapped into memory at
  startup and played from there as the resources were. The build copies `data.json` and the jingles
  next to the exe (`CopyProgramDataToOutDir`); a release zip must hold them with `images`.
- Removed: the `Edition` and `Dlc` enums, the 56-civ table, the per-DLC control, tooltip and resource
  IDs, the jingle resources, the unused `.rc` menu and dialog templates, and the project files' stale
  per-civ items (`civ_icons` entries, two history texts in a Steam folder).

### 10.3 Verification (2026-10-14)

- No edition, DLC or civ name, ID or key in any source, resource, project or test file (a scan with
  every name in data.json finds 232 in the previous sources and none now).
- Behaviour: the 55 scripted steps compared with the build before the change, its control IDs mapped
  to the new ones: no differences, and `log.txt` and `settings.ini` byte-identical. The three tabs,
  the Game menu and the Options, Hotkeys, About, Join Lobby and History dialogs are pixel-identical.
  History picked by list position and by search shows the same civs and dates as before.
- Jingles drawn with both jingle types (with silent copies of the files): no error. Without
  data.json, with a broken one, and without the jingles (run on a hidden desktop): the expected
  message each time; the program ends without data.json and runs without jingles.
- Builds as in 9.2; the unit tests are 59 (25 new for the reader, the catalog and the model).
  Idle memory is unchanged (private working set 0.1 MB, commit within the run-to-run spread of about
  1 MB). Assertions in the new files: about 0.8 per function; the functions without one are total
  (every input is valid), see question 7.

### 10.4 Decisions and questions

- Intended changes: the DLC toggles' tooltips now use the web version's wording and list the civs as
  data.json names them ("Toggles The Last Chieftains", "Rise of the Rajas", "Inca", "Maya"); History
  says "African Kingdoms" (data.json's name) where the code said "The African Kingdoms".
- The DLC toggles, edition rows and Game menu links have new control and command IDs (7900 + DLC,
  7950 + edition, 32800 + link) in place of one constant each.
1. The Join Lobby dialog says "AoE2DE" ("Enter AoE2DE lobby code", "AoE2DE must be already running"),
   as the web version does: lobbies open through the `aoe2de://` links of that game's client, so this
   was read as the name of the program the feature launches, not as edition data. Should it come
   from data.json too?
2. data.json may list at most 3 editions and 64 civs because the main window has room for no more.
   A fourth edition or a 65th civ would need a layout change first; agreed?

## 11. Share links (2026-10-14)

The user asked for compatibility with the web version's links: the program both shares and opens
them. A link made by either version opens in the other with the same edition, civ pool and draws.

### 11.1 The format and where it lives

- `https://frcp.net/#p=` and a code: the format version "1", the edition by its place in data.json,
  the pool as a bitfield (six civs per base64url digit, bit b of digit c is the civ with ID 6c + b),
  and optionally "." and the round's draws, one digit each in draw order. Civs past the end of a
  link's bitfield count as enabled, and civs outside the edition as left out. The format allows 64
  civs, which is `MAX_CIVS`.
- `ShareLink` (model, plain C++) writes and opens links exactly as the web version's `shareCode`
  and `parseShare`: a link is read from any text that holds one (trimmed, from after its "#",
  "p=" optional, percent escapes decoded); anything else is refused.
- The model gives what a link carries (`GetSharedPool`: the edition, the pool, and this round's
  draws that are still drawn and in the pool), whether loading one would replace something
  (`HasCustomState`), and loads one (`LoadSharedPool`: the edition without auto-toggle, the pool,
  then a new round with the link's draws; the log of past rounds stays, as on the web).

### 11.2 In the program

- File menu: Copy Share Link (P), Share ..., Open Link ... (U), as the web version's browser key
  set; the user chose P and U, so Ctrl+S and Ctrl+R stay Save and Load Preset.
- Share buttons where the web version has them: on the Log tab (the pool and this round's draws)
  and on the Civ Pool tab (the pool alone), the wooden nine-slice button with `ui/share_icon.png`.
- Copying puts the link on the clipboard and shows the web version's status message below the
  window (`ParchmentToast`: "Link copied: civ pool + 3 drawn civs", 2.2 s, fading in and out). If
  the clipboard refuses, the Share dialog opens with the links to copy by hand.
- Dialogs, laid out from the web version's CSS: Share (both links in read-only fields with Copy
  buttons, and a note), Open Link (a field, the error line "That isn't a Fresh Random Civ Picker
  link." that grows the dialog, Open and Cancel), Open Shared Link (what the link holds, Load Link
  and Cancel). The preview shows only if loading would replace something: another edition than the
  first, a civ left out, a draw, or anything in the drawn log. A loaded link's draws come in
  without their jingles; then the button sound and "Loaded the shared link".
- New for this: `PartKind::readonly_field` in the dialogs, `DialogMetrics.h` (the dialog measures
  CivPickerViewDialogs.cpp and CivPickerViewShare.cpp share), and a smooth or pixelated icon in
  `FramedIconButton`.

### 11.3 Verification (2026-10-14)

- Against the web version's own code: a scratchpad harness runs `shareCode` and `parseShare` from
  frcp_html/script.js in Node beside the desktop's `ShareLink` (built into a small test program):
  3000 random pools gave identical links, the desktop opened all 3000 web links to the same pool,
  and 6010 mutated texts (3589 of them still links) were accepted or refused alike, with the same
  result. No differences.
- In the program: a round of three draws shared, read from the Share dialog and opened by the web
  version's parser gives the same civs in order; a link made by the web version's code opened
  through Open Link (after the error line for a text that isn't a link, and the preview) leaves
  the edition, every civ's check state and the round as the link says, and sharing again gives the
  same link. The status message shows centred 14 px below the window and goes after about 2.4 s.
  The share buttons and the File menu match the web version's screenshots; the Hotkeys dialog lists
  P and U at its old size. The 55 scripted steps against the build before: no differences; no
  assertion fired.
- Not tested automatically: the clipboard write itself. Copying would replace the user's clipboard,
  and a window station of its own (with its own clipboard) can't be made in this environment. It
  uses the same calls as the drawn log's Ctrl+C.
- Builds as in 9.2, the unit tests are 69 (10 new for links and sharing), idle memory unchanged.

## 12. Original images and DLCs added in data.json alone (2026-10-15)

The user asked: raise the DLC limit from 9 to 128, so that adding a DLC needs nothing but data.json,
its jingles and its icons; use the game's original images instead of the converted WebPs as the
user obtains them, starting with the cursors (32 and 48 px), the parchment, the slider (with hover
and active looks for the arrows and the handle) and the dialogs' panel; never stretch or resize the
parchment.

### 12.1 Adding a DLC

- data.json: the DLC in "dlcs" (with its "icon"), its key at the top of its edition's "dlcToggles",
  its civs in "civs" with the next IDs. Files: `images\dlc\<icon>.png`, `images\civ\<civ in lower
  case>.png`, `sounds\civ_jingles\<civ in lower case>.wav` (and in `legacy\` for an edition with
  legacy jingles), `Histories\<Civ>.txt`. Nothing in code, resources or project files.
- Icons are one file each, of any size: each is fitted into its atlas cell with its proportions kept
  (`ContainedSize`, CSS object-fit: contain; WIC's Fant scaler on premultiplied pixels) when the
  atlases are built at startup (`ParchmentImages::BuildAtlas`). A file that can't be loaded leaves
  its cell empty, and a startup message names the first such file.
- `MAX_DLCS` = `MAX_DLC_TOGGLES` = 128. An edition's toggles that don't fit above the civ grid
  scroll (`CivPickerViewDlcToggles.cpp`): how many show follows from its dlcToggleLayout (the rows
  between its top and 6 px above the civ grid, `CivPickerLayout.h`); a `ScrollBar` control (the
  logs' look, counting rows) sits at the client area's right edge, and the wheel scrolls the list
  under the mouse. A to L toggle an edition's first nine (`DLC_HOTKEY_COUNT`); later toggles have no
  hotkey, their tooltips name none, and the Hotkeys dialog lists the nine.
- Control IDs: DLC toggles moved to 8000 + DLC (7900 + 128 would have run into the edition rows at
  7950); the scrollbars are 7960 + edition. static_asserts keep the ranges apart.
- Still a limit: `MAX_CIVS` = 64 (56 used). The share-link format writes a draw as one base64url
  digit and the civ grid has 13 rows of five, so three more DLCs of three civs reach it.
- Fixed on the way: a 10th toggle crashed at startup (ISSUES.md, Fixed). A startup that fails
  without crashing now says so in a message box instead of ending silently.

### 12.2 The original images

- `ui/background.png` (1024 x 1167): drawn unstretched. Its visible part (x 26 to 995, y 13 to 1151:
  the paper and its burnt edge without the transparent margin, alpha above 3) fills the window's
  578 px width, so the window is 679 px tall instead of 605 (`ParchmentTheme.h`). The content keeps
  its place, so the 74 px it gained are all parchment below the client area (question 12.4.1).
- Cursors: `ui/cursor_x1.png` and `ui/cursor_pointing_x1.png` (32 px), the `_x2` files (48 px);
  hotspots (0, 0) for the arrow, (2, 0) and (3, 0) for the hand. They are created on a thread
  switched to per-monitor DPI awareness, so Windows shows them at their own size instead of
  enlarging them for the DPI-unaware window. The view asks for the real DPI of the window's monitor
  (GetDpiForMonitor, again on a per-monitor aware thread) when the window is made and whenever it
  moves to another monitor, and picks 48 px from 144 DPI (150 %) on. This needs Windows 10 1607
  (SetThreadDpiAwarenessContext is linked directly).
- Slider: `ui/slider/` (arrows, bar, progress, handle; the arrows and the handle in normal, hover and
  active looks). `LayOutSlider` (ParchmentGeometry, unit-tested) scales the images so the handle is
  as tall as the slider (24 px in Options): the arrows at the ends, the bar from arrow centre to
  arrow centre under them, the handle travelling between the arrows. Bar and progress are drawn as
  three-slices with unstretched 4 px ends; the progress is clipped at the handle's centre. An arrow
  steps the value, a press on the bar moves the handle there, the handle drags. The old track's
  groove percentages are gone.
- Panel: `ui/9slice/boxstyle3a_*.png`, one file per tile (the same art as the old 192 px
  `panel_9slice.png`, alpha identical), loaded by `ParchmentImages::CreatePanelSlices`; the inlay is
  the panel's middle as before.
- Icons: `images\civ\*.png` and `images\dlc\*.png` were cut losslessly from the old atlases, so they
  look exactly as before until the originals replace them. `tools/convert_web_assets.py` no longer
  lists the images that are originals now, and copies a web icon only where none of that name
  exists, so it never overwrites an original.
- `DrawScrollbar` (ParchmentScrollBar.cpp) draws the scrollbar for the logs and the DLC lists alike.

### 12.3 Verification

- Builds: x64 and Win32, Debug and Release, zero warnings at /W4; zero findings under /analyze
  (x64 Debug, 28 analysis logs). Both were given through the `CL` environment variable: msbuild's
  `/p:AdditionalOptions=` doesn't reach the compiler. A /Wall probe shows only the warnings of
  question 4 of 9.5 and one in a Windows SDK header. Unit tests: 76 pass (7 new: slider layout and
  hit test, contained size, icon file names, more than nine toggles, the 128 DLC limit, an edition's
  icon and banner required).
- On a hidden desktop (scratch harness, clicks and wheel posted to the windows): the Draw and Civ
  Pool tabs and the Options and Hotkeys dialogs; with data.json alone (plus icon and jingle files)
  12 DE toggles and 3 more civs, the list scrolled by wheel, buttons and track; 128 DLCs (123 DE
  toggles); a missing icon named at startup; the author's failing data.json from 2026-10-15 starts.
  The slider's arrows step the value and show their active look. No assertion fired.
- Not verified here: the hover looks (a hidden desktop has no pointer, so controls get
  WM_MOUSELEAVE at once), dragging the slider's handle and the scrollbar's knob, and the cursors on
  a high-DPI display.

### 12.4 Decisions and questions

Gathered with the rest of what waits for the user in section 13.

### 12.5 Continuous freshness counts civs (2026-10-15)

The user asked for the web version's slider, which steps in civs, in place of the strength from 1 to
10 (each step a tenth of half the pool).

- Setting `contfresh_civs`: how many of the civs drawn last sit out the next draws, at most half the
  pool (`FreshnessMaximum`, at least 1; `FreshnessLimit` is the setting within it). The slider's top
  stores `CONTFRESH_MOST` (= `MAX_CIVS`), "the most", which follows the pool's size, as the web
  version's FRESH_MOST does. Default: the most, as on the web. settings.ini key `ContFreshCivs`;
  `ContFreshStrengthValue` is read once more and turned into civs (1 is 1 civ, 10 the most, steps
  between a tenth of half the civs) and deleted on the next save.
- Options: the slider runs from 1 to half the pool, the label reads "Exclude the last N civs", with
  "(the most)" at the top, or "Needs at least 2 civs in the pool" (slider greyed), and the hint is
  the web's "When a round ends, these civs sit out the first draws of the next round."
- Model: the history keeps the last half-pool's worth of draws, oldest first; the setting picks how
  many of the newest sit out, so a new setting holds from the next draw on, in both directions. The
  special case for strength 1 (the previous civ only) is gone: 1 civ does the same. Loading a log
  replaces the history that the replayed draws built with the saved one, and the log may now hold
  the whole history (`LOG_LINE_LIMIT` = civ states + draws + edition + half the civs; it was cut
  short before, see ISSUES.md).
- Unchanged: when every candidate sits out (only after the pool changes mid-round), the draw falls
  back as before rather than letting the oldest ones back in as the web version does; undo doesn't
  take a civ out of the history (questions 13.1.6 and 13.1.7).
- Verified: zero warnings at /W4 and zero /analyze findings; 78 unit tests (2 new: the last N civs
  of a round sit out the next round's first draws, repeated 500 times; the most follows the pool).
  On a hidden desktop: "Exclude the last 28 civs (the most)" with the full pool, 27 after a click on
  the left arrow, an old strength of 4 read as 11, and settings.ini saved with `ContFreshCivs=11`
  and without the old key.

### 12.6 The program's icon in the title row (2026-10-15)

The user asked for the icon before the window's title, "[icon] Fresh Random Civ Picker", as the
window had before the parchment UI (the web version has none). `CivPickerView::DrawTitleIcon`
draws the 16 px frame of the `IDI_SMALL` icon by GDI (DrawIconEx, which blends its alpha) onto the
main window's backdrop, centred in the 30 px title row where the title used to start; the title
follows 6 px after it (`TITLE_ICON_SIZE`, `TITLE_ICON_GAP`). The icon is an opaque black square with
the white question mark, as on the taskbar. A failed load or draw is reported and leaves the title
alone. Verified: zero warnings at /W4, zero /analyze findings, captured on a hidden desktop.

### 12.7 The content never lies under the skin (2026-10-16)

The user reported a piece of parchment over the drawn log that lagged behind the window as it
moved. The original parchment's burnt edge reaches into the content: a notch over the drawn log
(x 32 to 44, y 292 to 323, alpha 228 to 254), a smaller one at the remaining log's right edge, the
client frame's sides and the close button's top right corner. Those pixels weren't fully opaque,
so they fell outside the window region and the skin showed them, and the skin follows the window
a moment late (it moves in WM_WINDOWPOSCHANGED). `BuildMainShell` now lays the parchment on an
opaque underlay (`CONTENT_UNDERLAY_COLOUR`, black) under the title row, menu bar and client area,
so all of the content is the window's own; where the edge reaches in it shows a little darker,
like the backdrop the controls already copied. The burnt edge outside the content still belongs
to the skin and still trails a moving window.
- Verified: on the Log tab, content pixels outside the window region: 885 before (278 in the drawn
  log, 38 in the remaining log), 0 after. Zero warnings at /W4 (Debug and Release x64), zero
  /analyze findings, 78 unit tests pass.

### 12.8 Civ icons in the logs (2026-10-16)

The user asked for the civs' icons in the Log tab's logs, a drawn entry reading "[icon] Celts
(1/56)", as the web version's logs show them (`drawnRows`, `fillLog`, `.log-row`).
- Drawn entries are now "<civ> (<k>/<n>)" instead of "<k>/<n> - <civ>"; the remaining log is
  unchanged. Only the display changed: log.txt stores names, and the drawn log is rebuilt from it.
- `ParchmentLog::ShowIcons` turns a log's icons on: a line of the text that starts with one of the
  labels as whole words is led by that label's atlas cell, 18 px, centred in a 20 px line, the text
  4 px after it (.log-icon, gap: 4px, the parchment logs' line-height: 20px). The rest of a wrapped
  line has none; the text keeps one column, so it lines up. Scrolling steps a line of 20 px.
- The labels are the civs' names from the catalog (so from data.json), each with its own cell
  (`CivPickerView::ShowCivIconsInLogs`); `IconLabels` (ParchmentGeometry) keeps copies and finds
  the longest label a line starts with. Copying text from a log gives the text alone.
- The History dialog's log keeps its 16.2 px lines and no icons (the web version has no such log).
- Verified: on a 578 px wide window the rows sit where the web version's do (drawn log row 1 at
  x 46, y 144, rows 20 px apart; remaining log at x 363), measured in headless Chrome against a
  copy of frcp_html with the probe; icon and text baseline match in a side-by-side capture.
  Zero warnings at /W4 (Debug x64 and Win32, Release x64), zero /analyze findings, 83 unit tests
  (5 new: a label leads a line, only as whole words at its start, the longest wins, bad labels
  are refused, a fixed capacity). ParchmentLog.cpp is now 528 lines, past CC's 500.

### 12.9 The open dropdown list's frame (2026-10-16)

The user saw a blue outline around an open dropdown in Options. The list (the combobox's
`ComboLBox` popup, `WS_BORDER`) had the theme's list frame: #7a7a7a, #171717 while focused and
#0078d7 while hot, which it is under the mouse whenever the user picks an item. `StyleDropdown` now
also subclasses the list (`DropdownListProc`): after the system paints the non-client area (a long
list's scrollbar included), the 1 px frame is painted over in `MENU_BORDER_COLOUR` (#3d2c1c), as
the web version's open list has (`::picker(select) { border: 1px solid #3d2c1c }`), on WM_NCPAINT
and in WM_PRINT.
- Verified on a hidden desktop (Options' jingle dropdown and the History dialog's long list,
  opened by a posted click): the frame reads #3d2c1c from the list's window DC and in PrintWindow,
  #7a7a7a before; the long list keeps its scrollbar. The hot (blue) look itself can't be produced
  there, since a hidden desktop has no pointer; the theme's colours per state were read with
  DrawThemeBackground (Listbox, LBCP_BORDER_NOSCROLL). Zero warnings at /W4 (Debug and Release
  x64), zero /analyze findings, 83 unit tests pass.

## 13. Waiting for the user (gathered 2026-10-15)

Everything from the sessions of 2026-10-15 that needs a decision, a look or a hand. Older open
questions: 9.5 (1 to 8) and 10.4 (1 and 2).

### 13.1 Decisions

1. The parchment (12.2) made the window 74 px taller, and all of it is bare parchment below the
   content. Keep it, or let the client area use it (taller logs, more civ grid rows, more DLC rows
   before the list scrolls)?
2. 64 civs is the next limit a DLC can hit (56 used; three more DLCs of three civs reach it). It is
   set by the share-link format (a draw is one base64url digit) and the civ grid (13 rows of five).
   Raise it with a new share-link format version, shared with the web version?
3. An edition's DLC toggles after the ninth have no hotkey (A to L take the first nine, newest
   first, so older DLCs lose theirs as new ones arrive). Another key set for them, or none?
4. The 48 px cursors are used from 150 % display scaling on. At 125 % the 32 px ones show at their
   own size, smaller than the system's 40 px cursors. Move the threshold, or let Windows enlarge the
   32 px ones there?
5. Windows 10 1607 or later is now required (SetThreadDpiAwarenessContext for the cursors). Fine,
   or should the program still start on older Windows?
6. Continuous freshness: when every civ left sits out (only after the pool changes mid-round), the
   desktop draws as before; the web version lets the oldest ones back in first. Match the web?
7. Continuous freshness: an undone draw stays in the desktop's history of recent civs, while the web
   version's history is its log, which loses the undone draw. Match the web?
8. The title icon is now drawn from `images\ui\frcp.png` (500 px, scaled down; 12.6 first used the
   .ico's 16 px frame). It is cleaner at 100 %, but the program is DPI-unaware, so on a 175 % display
   Windows enlarges the whole window, icon included, and it stays soft. Truly sharp needs either a
   per-monitor DPI aware program (a large change) or a small per-monitor aware layered window for
   the icon over the title row. Which, if any? (`frcp.png` is untracked: add it to git.)

### 13.2 To check by hand

A hidden desktop has no mouse pointer, so these could not be seen there:

- The hover looks of the slider's arrows and handle (the active looks were checked).
- Dragging the slider's handle and the DLC list's scrollbar knob.
- The cursors on the 175 % 4K display: 48 px and sharp, not enlarged to about 84 px. If they are
  too big, Windows scales cursors made on a per-monitor aware thread after all (12.2).
- The window on a 1080p display at 150 %: 679 px tall, it only just fits the work area.

### 13.3 Files to delete

Unused since 12.2, but still copied next to the exe by every build. Deleting tracked files was
refused to Claude, so they are left for the user:

- `images\ui\New folder\` (the originals' staging folder; its files are copied into place)
- `images\ui\cursor.png`, `cursor_pointing.png`, `slider.png`, `slider_knob.png`,
  `panel_9slice.png`
- `images\civ\civicon_atlas.png`, `images\dlc\dlc_atlas.png`, `images\dlc\edition_banners_atlas.png`

Nothing of these sessions is committed.

## 14. GSFC: the compliance pass

The adopted rules (14.1 to 14.4) are in CLAUDE2.md.

### 14.5 The compliance pass (2026-10-16)

Every C++ file of the program and the tests was checked against 14.2: by a scratch checker for the
mechanical rules (spacing, switches, includes, classes, comments, conditionals, nesting), by
clang-tidy 22 (implicit conversions in conditions, uninitialized variables, parameter names,
virtual destructors, large by-value parameters, unused includes), and by reading. Changes:

- Switches: a default case in the 16 without one (rule 14).
- Conditions: 7 implicit pointer, handle and integer tests made explicit (rule 13).
- Initialization: 5 buffers, `MSG`, `SYSTEMTIME`, `OptionsEvent` and a local array initialized
  where declared; `Civ`'s fields and `CivPickerModel::available_civs` given member initializers
  (rules 24, 26). Four declarations renamed their parameters to match the definitions (rule 27).
- Classes: `final` on CivPickerController, CivPickerModel, WrappedLines, IconLabels, OpaqueRuns,
  ShadowBlur and BoxShadow; virtual destructors on the two base classes, ParchmentControl and
  CivPickerObserver (rule 20). Every virtual method of ParchmentControl is overridden somewhere.
- Constants: of `GlobalConstants.h`'s 52 `#define`s, 45 are `constexpr` now and 7 unused ones were
  deleted (`DT_UNDERLINE`, `MAX_LENGTH`, `MIN_WIDTH`, `MIN_HEIGHT`, `BUTTON_WIDTH`, `BUTTON_HEIGHT`,
  `HOTKEY_ID_F6`) (rule 5).
- Standard library: 36 hand-written minimums and maximums, and the `Larger` helper with its 4 uses,
  became `std::min` and `std::max`; `NOMINMAX` is defined in both project files so that
  `<windows.h>` doesn't hide them (rule 39).
- Includes: `framework.h` lost the template's four C runtime headers (`<stdlib.h>`, `<malloc.h>`,
  `<memory.h>`, `<tchar.h>`; one `memcpy` became `std::memcpy` from `<cstring>`); unused includes
  removed (`<cmath>`, `<cwctype>`, `CivPickerController.h`, `<d2d1_1helper.h>` where nothing used
  them); system headers ordered by rule 29 in 8 files; the tests include the program's sources by
  name, with the program's folder as an include directory of the test project (rules 29-31).
- Files: `wWinMain` moved to `Main.cpp`, `LoadCatalog` became a static member of
  CivPickerController; `CivPickerShare.cpp`, `CivPickerDialogs.cpp` and `CivPickerDlcToggles.cpp`
  were renamed `CivPickerViewShare.cpp`, `CivPickerViewDialogs.cpp`, `CivPickerViewDlcToggles.cpp`
  (rule 32). The test runner's `main` moved to `TestMain.cpp`; `test.cpp` lost its tabs and took
  the project's brace style (rule 10).
- Linking: the ten `#pragma comment` lines became the project file's `AdditionalDependencies` and
  `AdditionalManifestDependencies`; the unused `gdiplus.lib` was removed; the WIC factory is created
  with `IID_IWICImagingFactory` (rules 33, 35). The embedded manifest and the imported DLLs are
  the same as before.
- Formatting: 8 argument lines aligned with their first argument (rule 11).
- README: a section on how the code is organized; the compile steps named a file that no longer
  exists (rule 34).

Found compliant without changes: names and abbreviations (`Dlc`, `Id` throughout; only the SDK's COM
interfaces use capitals), class section order, no function bodies in class definitions, no public
data in classes, no block comments, no embedded assignments, no `goto`, no global variables (the one
mutable file-scope variable, `control_class_atom`, is file-local), no large by-value parameters, no
real nesting of conditional operators, one declaration per line.

Verified: zero warnings at /W4 in Debug and Release, x64 and Win32; zero /analyze findings (x64 and
Win32); the /Wall probe shows no warning beyond the informational ones; 83 unit tests pass on x64
and Win32; no data names in code; the 55 scripted steps against the build before the pass: no
differences, no assertion fired; the three tabs on a fresh start are pixel-identical to it.

## 16. Files and folders within the user's limits (2026-10-16)

The user asked for the project to comply with CLAUDE2.md and with two new rules (CLAUDE2.md section 15): no file
over 1000 lines, no folder over 20 files besides asset folders. Asked before anything changed, the user decided:
the line limit covers every file, so CLAUDE2.md splits too; files under 250 lines stay separate, as the rules
that keep them apart stand (15.1); CLAUDE.md is rewritten from these files; the P10 backlog of section 8 is
recorded rather than worked off in this pass, and the project files get Level4.

### 16.1 What changed

- Split along their section banners, the code moved verbatim: `CivPickerView.cpp` (1100 lines) gave
  `CivPickerViewObserver.cpp` its observer notifications and the labels, logs and tooltips they update (808 and
  299 lines, the split planned in 9.1); `CivPickerController.cpp` (1021) gave `CivPickerControllerInput.cpp` its
  hotkeys and the commands of the menus, buttons and toggles, with the tables only they use (656 and 379).
  `OpenEditionLink` and `OpenEditionPage` stayed with the other actions that open pages, so `OpenUrl` stays
  file-local. `CivPickerView.h` labels its declaration groups with their files, as before.
- The 60 sources moved from one folder of 71 files into the folders of CLAUDE2.md 15.2, with `git mv`, so git
  records them as renames. Both project files list the folders as include directories; the filters mirror them.
- CLAUDE2.md (1387 lines) became the rules (CLAUDE2.md, 856 lines) and this log.
- GSFC 23, which the pass of 14.5 had checked in headers only: three classes in .cpp files had function bodies in
  their definitions: `DataReader` (CivCatalog.cpp, 13 bodies), `SilentObserver` (CivPickerModel.cpp, 16) and
  `LinkOpener` (ShareLink.cpp, its constructor). The bodies now follow the class definitions.
- Both project files set `WarningLevel` `Level4` instead of `Level3` (P10-10).

### 16.2 Verification

- Builds: x64 and Win32, Debug and Release, zero warnings at /W4, set by the project files and confirmed in the
  compiler's command logs. Zero /analyze findings (x64 and Win32 Debug; 31 analysis logs each, none with a
  defect). Unit tests: 83 pass in all four configurations.
- Verbatim moves: a scratch tool compared every definition, with its comment, before and after: 761 of 764 are
  identical, and the 3 that differ are the GSFC 23 classes above. clang-tidy 22 with the checks of 14.5 finds
  nothing in the touched files that it didn't find before them (the same 13 tests of a Win32 BOOL, which rule 13
  allows, and the base classes' protected virtual destructors of rule 20).
- Mechanical rules, by a scratch checker shown to catch a planted violation of each: no file over 1000 lines, no
  folder over 20 files besides the asset folders, no function over 60 lines, no line over 120 characters, no
  tab, every switch with a default case last, no #define constant, no conditional compilation, no goto, setjmp,
  throw, new or malloc, no function pointer, no block comment, no directory in an include, no #pragma comment,
  every source in its project file, no data.json name in code.
- Behaviour, on a hidden desktop with silent jingle files and the UI sounds off, the build before against the
  build after: a scenario through the three tabs, DLC, civ and edition toggles, Enable and Disable All,
  auto-toggle and auto-reset, undo, redo, reset, quicksave and quickload, and the Options, Hotkeys, About,
  History, Join Lobby and Open Link dialogs gave 56 pictures and 44 snapshots of every control (class, ID,
  visibility, check state, place, text). The snapshots and the log.txt, settings.ini and quicksave written were
  identical in every run. The pictures of the build before differ from one run to the next, in up to 425 pixels
  of some (its first run against its second); the build after matched its second run pixel for pixel and never
  differed from either run beyond that. 70 draws through an exhausted pool: the same controls, round (26 of 56)
  and freshness history (28). No assertion fired, and both exited normally.

### 16.3 Found on the way

- The /W4 and /analyze results of 9.2, 12.3 and 14.5 were taken with the `CL` environment variable, which goes
  before the project's own options, so the project's /W3 won (cl said so: D9025, overriding '/W4' with '/W3').
  The builds of 16.2 are /W4 for real and were clean; options for a single build go in `_CL_`, which goes after
  the project's.
- The solution builds as it stands (Debug, x64 and x86: zero warnings, 83 tests pass): the program's project
  uses v145, the test project v143 (MSVC 14.44) with Windows SDK 10.0.22621.0, both installed here. ISSUES.md
  still said the tests didn't compile; that went to Fixed. Building the tests with v145 instead, as 16.2 did to
  test both projects with one compiler, needs
  /p:Force-Enable-Microsoft-googletest-v140-windesktop-msvcstl-static-rt-dyn=true, since the googletest package
  switches itself off for toolsets after v143. From a copy of the repository in the temporary folder the gtest
  include path passes 260 characters and cl.exe can't open it.
- A Rebuild clears what the last build in its intermediate folder wrote. One Rebuild here ran in the default
  intermediate folders with the output folder pointed elsewhere, and so deleted the exes and test binaries in
  `x64\Debug` and `Debug` (data, logs, settings, saves and histories stayed). A Build into the default folders
  put them back, built from the code of 16.1. Scratch builds now point both folders elsewhere.
- Hotkeys can't be tried on a hidden desktop: the controller ignores a hotkey unless its window is the
  foreground window, and GetForegroundWindow answers for the input desktop only. The scenario clicks the controls
  and sends the menu commands instead; the hotkey code was shown to be moved verbatim. FRCP shouldn't be tried on
  the user's desktop either: while it runs, Space, Tab, Enter and the letter keys are its system-wide hotkeys.

### 16.4 Still open

- The P10 backlog of section 8 and the questions of 9.5, 10.4 and 13.
- To check by hand: the solution in Visual Studio after the move (Solution Explorer shows the folders as filters,
  F5 builds and runs as before), and a few hotkeys.

## 17. Customisable hotkeys (2026-10-17)

The user asked: "new feature: customisable hotkeys. the user should be able to set their own custom hotkeys in
options." The web version has nothing to match (it has two fixed key sets), so the design is the desktop's own; the
decisions it took are in 17.3.

### 17.1 What changed

- Model: `HotkeyBindings` (`model\HotkeyBindings.h/.cpp`, plain C++, unit-tested). Every line of the Hotkeys dialog
  is a hotkey (`HotkeyAction`, 50 of them) with a scope (global, or the Draw Civ, Log or Civ Pool tab), a default key
  (the key the program always had), a name in settings.ini, and the key the user gave it (`Keystroke`: a virtual-key
  code and the MOD_ALT, MOD_CONTROL and MOD_SHIFT modifiers; the controller static_asserts that the numbers are
  Windows'). Two hotkeys that can work at the same time (on the same tab, or one of them global) never share a
  keystroke: giving a hotkey a keystroke takes it from the hotkeys it clashes with (`ClashesWith`, at most one on
  each tab). The editions' and DLC slots' hotkeys that data.json gives nothing to do (past its editions, past the most
  DLC toggles an edition has) are unused: not registered, not shown, never clashing. Keystrokes have names ("Ctrl+S",
  "Num 5", "None") that `ParseKeystroke` reads back; one can be a hotkey when its key has a name (letters, digits, F1
  to F11, the editing and arrow keys, the number pad) and Windows doesn't keep it (F12; Alt with Tab, F4 or Space; Esc
  with Alt or Ctrl; Ctrl+Alt+Delete). `CivPickerModel` holds the bindings (`GetHotkeys`, `EditHotkeys`) as it holds
  the settings.
- Controller: `CivPickerControllerHotkeys.cpp` (new) registers each keystroke of the hotkeys in use once (the ID is
  the action's number + 1), turns a WM_HOTKEY's keystroke into the hotkey for the tab shown (`ActionFor`) and runs it,
  one switch per scope. The table of IDs and its handlers left `CivPickerControllerInput.cpp`; `GlobalConstants.h`
  lost `HOTKEY_AMOUNT`, the `HOTKEY_ID_*` constants and the editions' and DLC slots' ID and key tables. settings.ini
  has a `[Hotkeys]` section with a line for each hotkey whose key isn't its default (`DrawTabDrawCiv=D`,
  `Exit=None`), written anew on exit, so with default keys there is no section; a line that names no keystroke the
  hotkey can have is left out at startup.
- Options > Hotkeys has a Customise button beside Show Hotkeys. It opens the Customise Hotkeys dialog (desktop only,
  `view\CivPickerViewHotkeys.cpp`): a hint, a radio button for each section, the section's hotkeys in rows of a key
  (`KeyButton`) and what it does (one column, or two even ones past 12 rows), two status lines, and Defaults, Cancel
  and OK. A click sets a key listening (gold, an ellipsis): it takes every key, Tab, Enter and Esc too, and reports
  the first that isn't a modifier, with the modifiers held; Backspace leaves the hotkey without a key (a dash).
  Another click, the focus leaving or another section stops it. The status lines say what became of the key: the
  hotkey's new key, the hotkeys it was taken from ("Q was taken from Open Options (Draw Civ tab), Clear drawn log (Log
  tab) and Definitive Edition (Civ Pool tab)."), a keystroke Windows keeps or one without a name, and that another
  program has the keystroke as a hotkey (the controller tries to register it; the program's own hotkeys are
  unregistered while the dialog is open). Keys change at once; Cancel or Esc gives back the keys the dialog opened
  with, and Defaults gives every hotkey its default.
- The menus' shortcuts, the tooltips' "Hotkey:" lines and the Hotkeys dialog show the keys the hotkeys have
  (`RefreshHotkeyTexts` once the dialog closes); a hotkey without a key has no shortcut or hotkey line, and a dash in
  the dialogs. The Hotkeys dialog's layout is made from the bindings (it moved from `CivPickerViewDialogs.cpp`); the
  Draw and Reset buttons, shown on the Draw Civ and Log tabs, say the key of the tab shown.
- With a menu open, a hotkey's keystroke goes to the menu first as its own key (Esc, Enter, the arrows) and
  otherwise closes it, as Esc and Enter did before.
- Parchment: `KeyButton` and `PaintKeyCap` (the Hotkeys dialog's keys are drawn with it too); `PartKind::key_button`
  with 24 key buttons per dialog level, and 32 labels instead of 16; `ParchmentControl::OnFocusLost` and
  `NotifyParentLater` (posted, so a key's parent changes the key after the key's message, I7);
  `ParchmentMenuBar::Remeasure`.

### 17.2 Verification

- Unit tests: 113 pass on x64 and Win32 (30 new: every default is the key the program had and none clash, what a
  keystroke means on each tab, clashes and taking a key, unused slots, names read back for all 596 usable keystrokes,
  F12 and the keystrokes Windows keeps refused, the model's hotkeys follow data.json). Breaking the clash rule (global
  keystrokes not clashing with the tabs') fails 3 of them.
- Builds: x64 and Win32, Debug and Release, zero warnings at /W4; zero /analyze findings (x64 Debug, 34 analysis
  logs). clang-tidy 22 with the checks of 14.5 finds nothing new in the touched files but one test of a Win32 BOOL,
  which rule 13 allows. A scratch checker of the touched files: none over 1000 lines (the largest, CivPickerModel.cpp,
  has 872), no folder over 20 files, no function over 60 lines, no line over 120 characters, every switch with a
  default case last, ASCII and CRLF throughout.
- On a hidden desktop (a new scratch harness: posted clicks and keys, PrintWindow, the debug output read for failed
  assertions), Debug x64, Release x64 and Release Win32: the sections, listening and stopping, a global keystroke
  taken from three tabs' hotkeys, Backspace, F12, a key without a name, F10 (a system key), Esc, Tab and Enter taken as
  keys while listening and Esc cancelling otherwise, Ctrl+Shift+Y with the modifiers held (the harness shared the
  program's input state), Defaults, Cancel, OK, and another program's hotkey (the harness's own) warned of.
  settings.ini kept the changed lines only, and at the next start the menus, the Hotkeys dialog and the Customise
  Hotkeys dialog showed them (a refused F12 and broken lines were left out). No assertion fired, and the program
  exited normally each time.
- With default keys, against the build before (each build run twice; the first run of the build before differed from
  its second in up to 49 pixels, as in 16.2): the three tabs, all six menus and the Hotkeys and About dialogs
  pixel-identical, Options different only inside its new button, settings.ini byte-identical.
- Not verified here: pressing the hotkeys themselves (a hidden desktop isn't the input desktop, 16.3), the keys'
  hover look, and keys typed on a real keyboard into the dialog (the tests posted them to the key, which the dialog
  manager handles the same way).
- While this ran, `images\ui\background.png` was moved to `images\ui\obsolete\` and `images\ui\new_background.png`
  appeared (not by Claude; left as they are). The scratch test folders used the moved file.

### 17.3 Decisions taken without asking (change any of them)

- Where: Options > Hotkeys > Customise, a dialog of its own; the Hotkeys dialog stays the read-only summary. Every
  hotkey it lists can change, the editions' and DLC slots' too, and each has one key.
- A keystroke in use where the hotkey would also work is taken from the hotkeys that had it, and the status line
  names them, rather than refused: swapping two keys takes two presses, and Cancel undoes everything.
- While a key listens, Backspace means "no key", so plain Backspace can't be a hotkey (Ctrl+Backspace can). Esc, Tab
  and Enter can.
- Punctuation keys (; = , - . / ` [ ] \ ') can't be hotkeys: their characters depend on the keyboard's layout.
- The Draw Civ and Log tabs' Draw civ and Reset are separate hotkeys, as the Hotkeys dialog lists them.
- F6 no longer quicksaves (ISSUES.md, Fixed). The aoe2vils.net tooltip says "Ctrl+T" where it said "Ctrl + T", as
  the program names keys everywhere else.
- Found and not changed: Options' two radio buttons don't untick each other (ISSUES.md, Tracked).

### 17.4 Questions

1. Should the web version get customisable hotkeys too?
2. Taking a keystroke over (17.3): keep it, refuse a keystroke in use, or swap the two hotkeys' keys?
3. Should punctuation keys be allowed, named after the keyboard layout's characters?

## 18. The shorter parchment (2026-10-18)

The user asked for the new `ui/background.png` (1024 x 1070, the author's), which "should fit neater and doesn't
have the excess vertical space".

- The new picture is the old one of 12.2 with its rows 0 to 886 unchanged and a new burnt bottom edge 97 px higher.
  Its visible part (alpha above 3) is x 26 to 995, y 13 to 1054, so `BACKGROUND_VISIBLE_BOTTOM` is 1054 (it was
  1151) and the window, whose height follows from it, is 621 px tall instead of 679: 38 px of parchment below the
  client area instead of 96 (the web version has 22). This settles question 13.1.1 (the bare parchment below the
  content) and leaves more room on the 1080p display at 150 % of 13.2.
- Verified: zero warnings at /W4 in Debug and Release, x64 and Win32; 113 unit tests pass. On a hidden desktop the
  window is 578 x 621; no pixel of the content lies outside the window region (12.7), which reaches row 609 in the
  middle; the paper is fully opaque under the content's columns down to row 596, and the content ends at row 583. The
  menus and the Options, Hotkeys and About dialogs are pixel-identical to the build before (the Hotkeys dialog once
  showed its OK button's focus outline, which Windows shows or hides by the keys of the run), and no assertion fired.
- The pictures' git state is the author's: `images\ui\new_background.png` and `images\ui\obsolete\background.png`
  (committed in 5b78ec3) are deleted in the working tree and `images\ui\background.png` is untracked; nothing was
  staged.
