# Rules

Rule 0: All commits must be made by humans. AI agents may not commit, per policy stated in CONTRIBUTING.md.

Rule 1: The intended UI and behaviour of Fresh Random Civ Picker is defined by REQUIREMENTS.md.

Rule 2: Only the chief developer (Carl Nordenbrink @cnordenb) may modify REQUIREMENTS.md. To others and to AI
agents, REQUIREMENTS.md is read-only.

Rule 3: Must be as easy as possible for humans to maintain with as few steps as possible.

Rule 4: Must have tests to ensure the project still works every push.

Rule 5: A test may test one thing and one thing only. A test must have one of the following purposes: requirement compliance, rule compliance, regression testing for previous bugs. A test may not test something that another test already tests.

Rule 6: Tests will be documented, without excess clutter, text or story-telling, in TESTS.md.

Rule 7: Must keep up-to-date MAINTAINER.md which explains pedagogically for humans how to maintain the project.

Rule 8: MAINTAINER.md should be beginner-friendly, stick to the essential stuff (don't clutter it with unnecessary
details), and have a table of contents. The steps of the most common maintenance task should always be the first
chapter. Never insert content to before that guide.

Rule 9: The project's terms are defined in TERMINOLOGY.md, one term a line of at most 120 characters, each line using
only terms defined above it.

Rule 10: No comments allowed in the code.

Rule 11: No function may exceed 60 LOC. This applies to code.

Rule 12: No LOC may exceed 120 characters. This applies to code.

Rule 13: No file may exceed 400 LOC. This applies to code.

Rule 14: No folder may exceed 7 files. Root may exceptionally contain up to 8 files. This rule does not apply to
documentation/DEVLOG/, nor to the asset folders `Fresh-Random-Civ-Picker_CPPGUI/images/` and
`Fresh-Random-Civ-Picker_CPPGUI/sounds/` and their subfolders.

Rule 15: No two files in this project may have identical file name. This rule does not apply to "SKILL.md", nor to
files in the asset folders `Fresh-Random-Civ-Picker_CPPGUI/images/` and `Fresh-Random-Civ-Picker_CPPGUI/sounds/` and
their subfolders.

Rule 16: Document development and changes in DEVLOG.

Rule 17: DEVLOG should never refer to any person(s), actor(s) or AI agent(s). Everything is described passively:
something was decided, something was discovered, a question was posed. Never "the chief developer asked" or the like.
Blame is already automatically logged with commits for designated responsibility, there is no need to mention anyone in
DEVLOG. Obvious exceptions that this rule does not apply to: conceptual general actors ("the user"), a file name and not
a person ("MAINTAINER.md").

Rule 18: DEVLOG may not exceed 1000 lines. DEVLOG parts will be DEVLOG1.md, DEVLOG2.md, etc.

Rule 19: Every DEVLOG file needs a table of contents at the start from which the reader can jump to any day.

Rule 20: A DEVLOG entry is always a day (titled date (YYYY-MM-DD)). The entry may have sub-sections timestamped with
hh:mm:ss.

Rule 21: Every new addition to DEVLOG entry must be timestamped with date if it is the first addition of the day and time
hh:mm:ss if first addition of current minute.

Rule 22: When splitting up the DEVLOG and starting a new DEVLOG file, a day entry may never be broken up. If 1000 lines
is reached, current day entry is moved in its entirety to a new file.

Rule 23: DECISIONS.md must list the decisions made in DEVLOG and refer to the date and time (if available) in the
relevant DEVLOG where each was made.

Rule 25: A test makes one check and tests one thing only. A test is written only to cover a requirement or to catch
again a bug that was found before (a regression test); no test is written for its own sake.

Rule 26: The following is not considered code (contingent on rules 10-13): *.md, *.json, *.txt, `FRCP_GUI.rc`,
`Resource.h`, *.sln, *.vcxproj, *.filters, `build.bat`, `convert_web_assets.py`

## Precedence

Rule 27: The rules from Rule 27 on are condensed from older sources, and each names its source by a tag. P10-n is rule n
of "The Power of Ten: Rules for Developing Safety Critical Code". CCn is chapter n of "Clean Code", and CC-G5, CC-N3,
CC-T1, CC-F2, CC-E1 and the like are the heuristics of its chapter 17. HFn is chapter n of "Head First Design Patterns".
GSFC-n.n is section n.n of NASA Goddard's "C++ Coding Standards and Style Guide". In is interpretation n of these for
C++ and Win32. CD is a rule of the chief developer's own. FN is the project's older naming goals.

Rule 28: Rules 0 to 26 come before every rule from Rule 27 on. Among the later rules the order is: the chief developer's
own (CD), then P10, then CC and HF, then GSFC. CC and HF agree in substance; where they differ in emphasis, the simplest
solution that works is taken (CC11, HF13). GSFC binds only where it conflicts with none of the others, and loses any
conflict found later.

Rule 29: Where the rules leave a detail open, the existing code is the standard (CC5, CC-G24): `T* p` and `T& r`,
`nullptr` for pointers and `NULL` for handles, no parentheses around the condition of `?:`, and the program's classes at
global scope, without a namespace of the project's name (GSFC-2.2, GSFC-2.6).

Rule 30: P10 governs production code. Test code follows every rule of cleanliness and compiles without warnings, but may
ignore limits of efficiency, such as the use of the heap in a test fixture. Production code under test complies in full
(I6, CC9, P10-10).

## Safety

Rule 31: Control flow is simple: no `goto`, no `setjmp` or `longjmp`, and no direct or indirect recursion, so that the
call graph is acyclic. An early return is fine; a single exit point is not required. A tree is traversed iteratively,
with an explicit stack of fixed capacity and a bounded loop (P10-1, P10-2, GSFC-3.12).

Rule 32: Project code never throws. Failure is reported through return values that every caller checks. Code that can
throw (the standard library, third parties) is wrapped at the boundary so that nothing propagates, its exceptions caught
there by reference; APIs that do not throw are preferred (P10-1, P10-7, CC8, GSFC-5.2.2).

Rule 33: Every loop has a fixed upper bound that a tool could prove. A loop with a variable number of iterations (a list
walk, a string scan, a search) gets an explicit maximum; exceeding it is a failed assertion and the function returns an
error. The only loops without a bound are the Win32 message loop and the modal loops the system runs (P10-2, I3).

Rule 34: Startup is everything in `wWinMain` before the message loop starts, including the main window's `WM_CREATE`. A
dialog opened later runs after startup (I5). Every object is built and wired at startup, eagerly and never lazily.
Dependencies point away from `wWinMain`, and the rest of the program assumes that everything is already built. A factory
runs only at startup or hands out instances built there (P10-3, CC11, HF4, HF5).

Rule 35: No memory is allocated dynamically after startup: no `new`, `malloc`, `make_unique` or `make_shared`, no
standard container that grows, no `std::wstring` or `std::string` operation that allocates, no thread creation, no
variable-length array and no `alloca`. Storage has a fixed capacity, sized by named compile-time constants:
`std::array`, fixed `wchar_t` buffers, pools allocated at startup (P10-3).

Rule 36: A line holds one statement or one declaration. A single statement under an `if` or a loop may share its line
and go without braces, as in `if (!c_assert(x)) return false;`; a scope of several statements is never put on one line.
An `if` inside another `if` always takes braces (P10-4, GSFC-3.1, GSFC-3.8, CC5).

Rule 37: Functions hold at least two assertions each on average. An assertion is a Boolean test without side effects of
something that should never happen: a precondition, a postcondition, a parameter value, a return value or a loop
invariant. An assertion that a tool can prove always true or always false does not count. The density is kept even when
it makes a function longer; no meaningful check is dropped to shorten code (P10-5, P10-7). An assertion is written
`if (!c_assert(expr)) { return <error>; }`: when it fails, the macro logs the file, the line and the expression through
`OutputDebugStringW` and evaluates to false, and the function recovers explicitly. `assert()` from `<cassert>` is not
used: it aborts instead of recovering, and it is gone in a Release build (P10-5).

Rule 38: Every data object is declared at the smallest possible scope, in the order of preference local, member,
file-local in an anonymous namespace. There are no global and no `extern` variables. A variable is never reused for an
unrelated purpose, and its scope is never widened to save arguments: it is passed, or a cohesive class is extracted that
owns exactly that data (P10-6, GSFC-2.7.2, GSFC-10.1).

Rule 39: Every function validates every one of its parameters, and every caller checks every return value that is not
void. Where success and failure would get the same response, the call is cast to `(void)` to show that the choice is
deliberate. Errors propagate up the call chain (P10-7).

Rule 40: The preprocessor is used only for `#include`, `#pragma once` and simple macros that expand to a complete
syntactic unit: no token pasting (`##`), no variadic macros and no recursive macros. Conditional compilation (`#if`,
`#ifdef`) is used once or twice in the whole project at most (P10-8).

Rule 41: A pointer is dereferenced at most one level in the source: `*p`, `p->x` and indexing through a pointer count,
the implicit `this` does not, so `observer->OnChanged()` is fine and `a->b->c` and `**p` are not. No dereference hides
inside a macro or an alias, so no `LP...` or `P...` aliases are declared for data that is dereferenced. Win32 handle
types are opaque, never dereferenced, and fine (P10-9, I4).

Rule 42: There are no function pointers, no pointers to member functions, no `std::function`, no stored lambdas and no
callback tables. The only exceptions are those Win32 forces: window, dialog and subclass procedures, enumeration
callbacks and thread entry points. Each of these is thin: it validates its parameters, then calls an ordinary function
(P10-9, I2).

Rule 43: Virtual functions are allowed, on small and closed hierarchies: the hierarchy is shallow, leaf classes are
`final`, and no virtual call can re-enter its caller. A class that can be derived from has a virtual destructor, every
other class is `final`, and a method is virtual only where it is overridden (I1, P10-1, GSFC-5.1).

Rule 44: Re-entrancy that the system causes (`CreateWindow` sending `WM_CREATE`, `SetWindowText` sending `WM_SETTEXT`)
is a platform boundary, not recursion. The code never causes it on purpose: a handler does not `SendMessage` to its own
window, it calls the handler function directly or uses `PostMessage` (I7, P10-1).

Rule 45: The code compiles without a single warning under `/Wall` and `/WX`, with the Windows SDK's and other external
headers set apart by `/external:anglebrackets /external:W0`. No warning is disabled and no failing test is switched off
(P10-10, I8, CC-G4). The code is checked with at least one static analyser (MSVC Code Analysis `/analyze`, clang-tidy,
cppcheck) and has no findings. A false positive is not suppressed: the code is rewritten until it is trivially valid
(P10-10).

## Names

Rule 46: Functions, classes, structs, enum types, type aliases and named namespaces are written in PascalCase;
variables, parameters and enumerators in snake_case; constants in UPPER_SNAKE_CASE. An abbreviation is written one way
throughout: `Dlc` in every name, never `DLC` in some (CC5, GSFC-2, GSFC-2.1, GSFC-2.4, GSFC-2.5, GSFC-2.10, FN).

Rule 47: A name reveals intention: why the thing exists, what it does and how it is used, with its unit where it has one
(`elapsed_time_in_days`). It gives no false clue (no "list" for what is not one), does not differ from another name only
slightly, and is no number series (`a1`, `a2`) or noise word (`Info`, `Data`, `Object`) (CC2, GSFC-2). A name can be
pronounced and searched for. Single letters are loop counters in tiny scopes only (`i`, `j`, `k`, never `l`); the longer
the scope, the longer the name (CC2, CC-N5).

Rule 48: Names carry no encodings: no Hungarian notation, no `m_`, no subsystem or project prefix, no `I` on interfaces.
Win32's own parameter names (`hWnd`, `wParam`, `lParam`) stay in the signatures the API dictates, and no new encoded
names are made (CC2, CC-N3, CC-N6).

Rule 49: Classes and variables are nouns, never `Manager`, `Processor`, `Data` or `Info`. Functions begin with a verb.
Accessors and predicates are named for their value (Get, Set, Is, Has, Can). A method's name does not repeat its
class's: `Vector::Normalize`, not `Vector::NormalizeVector`. Where constructors would be overloaded, named creation
functions that describe the arguments are preferred (CC2, GSFC-2.4, FN).

Rule 50: One word stands for one concept (not fetch, retrieve and get side by side), and one word never for two ideas.
Technical things take names from computer science and from patterns, things of the domain the terms of TERMINOLOGY.md
(CC2, CC-N3, HF13).

Rule 51: A name is at the level of abstraction of what it names and does not reveal the implementation, is unambiguous,
and tells of side effects (`CreateOrGetX`, not `GetX`). Context comes from the enclosing class, function or namespace. A
name is changed as soon as a better one is found (CC-N1, CC-N2, CC-N4, CC-N7).

## Functions

Rule 52: Within Rule 11 a function is as small as it can be made: the body of an `if`, an `else` or a loop is ideally
one line, a call, and indentation is at most two levels deep (CC3, P10-4).

Rule 53: A function does one thing, at one level of abstraction, one level below its name. A function that can be
divided into sections, or from which a function can be extracted whose name is not a restatement of the code, does more
than one thing (CC3, CC-G30, CC-G34).

Rule 54: Code reads from the top down: callers stand above their callees, each function followed by those at the next
level down, forward-declared where C++ needs it. Private functions stand just below their first use (CC3, CC5, CC-G10).

Rule 55: There is at most one switch for each kind of selection. It creates or selects objects in a low-level place;
everywhere else uses polymorphism, within Rules 42 and 43. In a Win32 procedure the message switch is that one
dispatcher, and each case is a single call to a handler (CC3, CC-G23). Every switch has a default case, and it comes
last. A default that cannot be reached fails a `c_assert` and takes the recovery. Variables declared in a case go in a
block (GSFC-3.9).

Rule 56: A function takes as few arguments as possible and never more than three; arguments that travel together become
a struct. Two arguments are the ordered components of one value, or the name gives their order. Signatures that an API
dictates are exempt (CC3, CC-F1). A function with one argument asks a question about it, transforms it and returns the
result, or handles an event and is named as that. There are no flag arguments and no selector arguments of any type: the
function is split instead (CC3, CC-F3, CC-G15).

Rule 57: A function has no side effects: no hidden change to members, parameters or globals. Where one step must come
before another, the name shows it or each step's result is passed to the next (CC3, CC-G31).

Rule 58: A command may return a status of success or failure and nothing else. A query returns data and has no side
effects. Data from an operation that can fail is returned as a small result struct (status and value) or as a Special
Case value. Output parameters are used only where a Win32 signature forces them (CC3, CC-F2, CC7, P10-7).

Rule 59: Nothing is duplicated: a copied passage becomes a function, repeated switches become polymorphism, similar
algorithms become Template Method or Strategy. Dead code, functions never called, unused variables and empty
constructors are deleted (CC3, CC-G5, CC-G9, CC-G12, CC-F4).

Rule 60: Conditionals are put in well-named functions and are phrased positively. Intermediate values get explanatory
variables, and a boundary value such as `level + 1` is computed in one place (CC-G19, CC-G28, CC-G29, CC-G33).

Rule 61: Magic numbers and magic strings become named constants. Constants are declared `constexpr` or `const`, or as
enumerators, never with `#define`; the exceptions are `Resource.h` and the configuration macros that the Windows headers
read, such as `WIN32_LEAN_AND_MEAN`. `enum class` stands in for integer constants (CC-G25, CC-J3, GSFC-2.10).

Rule 62: A value that is not a boolean is compared explicitly: `pointer != nullptr`, `handle != NULL`, `count != 0`,
`(flags & BIT) == 0`. Booleans, Win32 `BOOL` among them, are tested directly; a Win32 function that returns other values
in a `BOOL` is compared explicitly (GSFC-3.8).

Rule 63: A complex statement becomes several simple ones. The conditional operator is for simple choices and is never
nested. An assignment inside a condition is used only where it is proven faster. Floating point is not used for exact
quantities (GSFC-3.6, GSFC-3.13, GSFC-9, CC-G16, CC-G26).

Rule 64: A function returns an expression or a constructed value directly, not a local made only to be returned, unless
that local explains the value or is checked before the return. Several returns are fine, and `break` and `continue` may
leave a loop at a logical point (GSFC-3.11, GSFC-3.14, CC3).

Rule 65: An instance of a class or struct that is larger than 16 bytes, or not trivially copyable, is passed by const
reference when the function does not change it. Smaller trivially copyable structs may be passed by value. A struct may
be returned by value (GSFC-8, GSFC-9, GSFC-3.14).

Rule 66: Components of the standard library are used instead of hand-written equivalents, within Rule 35 (nothing that
allocates after startup) and Rule 33 (a loop over a run-time count keeps its written maximum). ISO C++ is used wherever
it offers what an extension does: libraries are linked through the project file, not `#pragma comment` (GSFC-8,
GSFC-10.9).

## Formatting

Rule 67: A file reads like a newspaper: the high-level concepts at the top, detail growing downward. A blank line parts
two concepts, and lines that belong together stand together, without more blank lines than that (CC5, GSFC-3, FN).

Rule 68: A local variable is declared just above its first use and initialised where it is declared, a struct with
`= {}`; a loop's control variable is declared in the loop statement; member variables stand in one place (CC5, CC-G10,
GSFC-5.2.3).

Rule 69: Indentation is four spaces, never tabs. Declarations and assignments are not aligned in columns. An empty loop
body is avoided; one that cannot be avoided takes braces on a line of its own (GSFC-3.4, GSFC-3.5, CC5).

Rule 70: There is a space between a control keyword and its parenthesis, after each comma and each semicolon of a `for`
header, and around assignment operators and the `?` and `:` of the conditional operator. There is none between a
function's name and its parenthesis (`sizeof(` and casts included), between a unary operator and its operand, or around
`->`, `.` and `[]` (GSFC-3.3, GSFC-3.5). Arguments that do not fit on one line continue aligned with the first argument
or, where that does not fit either, under the function's name. A declaration keeps its return type on the line of the
function's name (GSFC-3.7, GSFC-5.1).

Rule 71: Includes stand at the top of a file only, in this order: the file's own header (or `pch.h`) alone; a header
that must be read before the others, alone (`targetver.h` before `<windows.h>`, `framework.h` before the SDK's headers);
the Windows SDK's headers, then the standard library's, each alphabetical; the project's headers, alphabetical; in
tests, the included sources last (GSFC-5.1.3). Every header begins with `#pragma once`. A file includes only what it
uses, and C++ headers instead of C headers (`<cstdlib>`, not `<stdlib.h>`). A header has no using-directive at global
scope (GSFC-5.1, GSFC-5.1.3, GSFC-10.7, GSFC-10.8).

## Classes

Rule 72: An object hides its data and exposes behaviour; a data structure exposes its data and has no behaviour of note.
There are no hybrids. A struct holds public data only and no business rules; a type with private data is a class, and a
class has no public data members. Getters and setters are not added blindly (CC6, GSFC-2.11, GSFC-5.1).

Rule 73: A method calls only methods of its own class, of objects it creates, of objects passed to it and of objects
held in its class's members. There are no chains such as `a.GetB().GetC().Do()`: tell, don't ask (CC6, CC-G36, HF7).

Rule 74: A class is small, measured in responsibilities: it has one reason to change, and can be described in about 25
words without "if", "and", "or" or "but". It has few member variables and each method uses most of them; where some
methods share some of the variables, the class is split (CC10, HF9).

Rule 75: The sections of a class come in the order public, protected, private, laid out as the project's headers already
are. Variables and helpers are private. A class definition holds no function bodies; a header that must define a
function does so outside the class (CC10, GSFC-5.1, GSFC-5.1.2, GSFC-5.1.4).

Rule 76: A constructor initialises the members and does nothing that can fail; work that can fail goes into a function
that reports failure, which the caller checks. Every member is initialised at construction, by a default member
initialiser or in the initialiser list, in the order of declaration. A definition's parameter names match those of its
declaration exactly (GSFC-5.2.1, GSFC-5.2.3).

Rule 77: Interfaces are small and hide data, helpers, constants and temporaries. A base class knows nothing of its
derivatives. General enums and functions do not live inside specific classes. A method works with the data of its own
class. Member functions are non-static unless they can never need to be polymorphic (CC-G7, CC-G8, CC-G13, CC-G14,
CC-G18).

## Error handling and boundaries

Rule 78: Error handling never obscures the logic. It is kept apart from the main path by early returns in guard clauses,
and a function that handles errors does nothing else. Every failure says which operation failed and how (CC3, CC7).
Status values are shaped by what the caller has to tell apart. A status enum stays local to its module; there is no
single global error enum (CC3, CC7).

Rule 79: Null is neither returned nor passed. A Special Case value or an empty value is returned instead, so that
callers need no exceptional path. A null argument is a programming error, caught by the parameter checks of Rule 39
(CC7, P10-5, P10-7).

Rule 80: Third-party interfaces (Win32, the standard containers, libraries) are wrapped in the project's own classes,
which report failure in the project's terms with one status type for each boundary. Boundary types are not passed around
the program or through public interfaces, and only a few places refer to third-party code (CC7, CC8).

## Tests

Rule 81: Test code is kept as clean as production code. A test is built as build, operate, check, and is fast,
independent of every other test, repeatable and self-validating (CC9, CC-T9). Building the program is one step, and
running all the tests is one step (CC-E1, CC-E2).

Rule 82: Unit tests use the real `data.json`. A file with a problem is `data.json` with one thing changed (CD).

## Design

Rule 83: A design, in order of priority, passes all the tests, holds no duplication, expresses its intent, and has as
few classes and methods as that allows. No dogma, such as an interface for every class, overrides this (CC12).

Rule 84: What varies is separated from what stays the same and encapsulated. Code is written to an interface, not an
implementation. Composition is preferred to inheritance. Objects that interact are loosely coupled (HF1, HF2).

Rule 85: Code depends on abstractions, not on concrete classes, and classes receive their collaborators as constructor
arguments instead of building them. A class is opened to extension and closed to modification only where change is
likely (HF3, HF4, CC10, CC11).

Rule 86: High-level components decide when and how low-level ones are used, and low-level components never call
high-level ones directly. There are no circular dependencies. Levels of abstraction are kept apart, and configurable
data stays at a high level and is passed down (HF8, CC-G6, CC-G35).

Rule 87: Code behaves as a reader would expect, at every boundary and corner case, stands where a reader would look for
it, and does similar things in the same way. A fact is asked of the module that owns it, not assumed. A design decision
is enforced by types, not by naming conventions (CC-G2, CC-G3, CC-G11, CC-G17, CC-G22, CC-G27).

Rule 88: A pattern is brought in only when it solves a present problem in the design that nothing simpler solves, and is
removed when its flexibility is not needed: "if you don't need it now, don't do it now". Classes and methods are named
after the patterns they implement (HF13, CC-N3).

Rule 89: Patterns keep within the safety rules. An Observer's list is an array of fixed capacity filled at startup, and
a notification never re-enters the subject. Commands are allocated at startup and their queues have a fixed capacity. A
Singleton is a static instance built at startup, used sparingly. Decorators are composed at startup. A Composite is
iterated, never recursed (HF2, HF3, HF5, HF6, HF9, P10-1, P10-3). A Strategy is switched by pointing to another strategy
built at startup. A template method is a non-virtual public function that calls virtual steps. An Adapter wraps a
boundary, and a Facade simplifies a subsystem (HF1, HF7, HF8).

Rule 90: Code that manages threads stands apart from all other code, in a few places, and does nothing else. Shared data
is kept to a small scope and protected; a thread is given a copy in a buffer allocated at startup. Critical sections are
small and never call one another, and locks are taken in one global order (CC13, P10-3).

Rule 91: Structure is changed in many small steps that each keep the behaviour and the tests passing, never in one large
step. Code that is touched is left a little cleaner than it was found (CC1).

## Model, view and controller

Rule 92: The model holds all data, state and application logic: the civ pool, the rules of the draw, the histories, the
settings. It knows nothing of the view or the controller, and is plain C++ without Win32 types or `<windows.h>`, so that
the tests compile it on its own (HF12, CC8).

Rule 93: The view only presents. When notified, it reads the state it shows from the model, to which it has read-only
access; it never changes the model. It hands every decision about the interface's behaviour to its controller: it knows
how to enable or disable a control, never when (HF12).

Rule 94: The controller receives the user's input from the view (`WM_COMMAND`, hotkeys, menus), interprets it and calls
the model, and may tell the view to change. It holds no application logic. There is one controller for each view at run
time. Controller logic is never put in view code (HF12).

Rule 95: The model, the views and the controllers are built, and the observers registered, in `wWinMain` before the
message loop, and are passed in where they are needed. A window or dialog procedure reaches its object through the
window's user data (`GWLP_USERDATA` or `DWLP_USER`, set during `WM_NCCREATE` or `WM_INITDIALOG`) (HF12, CC11, P10-3).

Rule 96: The model has one observer slot, filled by the controller at startup; until then, and in unit tests, a silent
Special Case observer stands in. Notification is synchronous. An observer never calls a mutator of the model during a
notification and never depends on the order of notifications (HF2, HF12, CC7, P10-1).

## Data and code

Rule 97: Data and code are strictly apart. `data.json` is the only place where editions, DLCs and civs reside, and it is
kept identical to the web version's `data.json` (CD).

Rule 98: No name, ID, key, count, list, constant, resource entry, control ID or layout of an edition, a DLC or a civ
stands in code, resources, project files or tests. Code reads them from `CivCatalog` or derives them from it: a civ's
jingle and icon files are its name in lower case, and an edition's and a DLC's picture files are named by its "icon" and
"banner" (CD).

Rule 99: Limits are capacities, not data, and are named in `CivDataLimits.h`. A `data.json` beyond them, or with a
broken reference, is refused at startup with a message that says what is wrong, and the program ends (CD, P10-3). Files
named after civs, such as jingles and history texts, are data files beside the program, not code (CD).

## Files and folders

Rule 100: `Fresh-Random-Civ-Picker_CPPGUI/` holds `Main.cpp`, `FRCP_GUI.rc`, `Resource.h`, `targetver.h`, `data.json`
and the project file with its filters file; its folder `tools/` holds `build.bat`, `FRCP_GUI.manifest` and
`convert_web_assets.py`; its folders `images/` and `sounds/` hold the assets. `UnitTesting_FRCP/` holds the test
project, with the test files in `UnitTesting_FRCP/tests/` (CD).

Rule 101: The code stands in folders by its role: `common/` (what every part uses), `controller/`, `model/catalog/`,
`model/picker/`, `view/main/`, `view/dialogs/`, `view/layout/`, and for the Parchment components
`parchment/graphics/drawing/`, `parchment/graphics/layout/`, `parchment/controls/base/`, `parchment/controls/lists/`,
`parchment/windows/dialogs/` and `parchment/windows/popups/` (CD, HF12).

Rule 102: A folder includes only from the folders it builds on. `common/` includes from `targetver.h` alone;
`model/catalog/` from `common/`; `model/picker/` from `model/catalog/` and `common/`; `controller/` from the model, the
view and `common/`; the `view/` folders from the model, the controller, Parchment and `common/` (CD). Within Parchment,
the `parchment/graphics/` folders include from `common/`; the `parchment/controls/` folders from graphics and `common/`;
the `parchment/windows/` folders from controls, graphics and `common/`. Nothing in the model or in `common/` includes
from the view, the controller or Parchment, and no Parchment folder includes from one that builds on it (CD).

Rule 103: An include directive names a file only, without a folder or an absolute path; third-party headers keep the
form their library documents (`"gtest/gtest.h"`). Every folder of code is an include directory of the program's project
file in every configuration, and the test project lists the folders whose sources it compiles (GSFC-5.1.3, CD). The
project file lists every source file, its filters file mirrors the folders, and a new folder goes into both project
files and the filters file. The program links only the libraries it uses (GSFC-10.6, GSFC-10.7, CD).

Rule 104: `wWinMain` stands in a file of its own, `Main.cpp`, and the test runner's `main` in `TestMain.cpp`. A file
that defines the members of one class is named after that class, as in `CivPickerView*.cpp` (GSFC-2.13, GSFC-7). A file
that outgrows Rule 13 is split along its sections into files named after its class, the code moved unchanged. A folder
that outgrows Rule 14 is divided by what its files depend on (CD, GSFC-2.13).

Rule 105: A file that a tool generates keeps the name and the format the tool gives it: `Resource.h`, the .rc file, the
project files (GSFC-2.14).
