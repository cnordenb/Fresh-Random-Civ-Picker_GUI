# CLAUDE2.md - Binding development rules for this project

Every change to this project must follow these rules strictly. Compiled 2026-10-13 from three
documents in `Fresh-Random-Civ-Picker_CPPGUI/materials/` (not in git), with a fourth added on
2026-10-16:

- **P10**: `P10.pdf`, G. J. Holzmann, *The Power of Ten: Rules for Developing Safety Critical
  Code* (NASA/JPL). Read in full.
- **CC**: `cc.pdf`, R. C. Martin, *Clean Code*. Read: ch. 1-13 and 17 in full, plus the general
  parts of ch. 11, 14-16 and Appendix A. Skipped as Java-specific: EJB/AOP/proxies (ch. 11),
  heuristics J1/J2, the Java listings in the case studies and Appendix B.
- **HF**: `hfdp.pdf`, Freeman & Robson, *Head First Design Patterns* (2nd ed.). Read: every design
  principle (ch. 1-11), the MVC part of ch. 12, all of ch. 13.
- **GSFC**: `mab583.pdf`, *C++ Coding Standards and Style Guide*, NASA Goddard Space Flight
  Center, Mission Applications Branch (Code 583), modified for the GMAT project by L. Jun and
  W. Shoan, draft of 2005-05-24. A scan with an OCR text layer (the title page's text is lost),
  read in full. Adopted only where it conflicts with none of the rules above (section 14).

Source tags: `P10-n` = P10 rule n. `CCn` = CC chapter n. `CC-G5`, `CC-N3`, `CC-T1`, `CC-C5`,
`CC-F2`, `CC-E1` = CC ch. 17 heuristics. `HFn` = HF chapter n. `GSFC-n.n` = GSFC section n.n.

This file holds the rules. Their history is in `CLAUDE2_LOG.md` beside it: the state of the code, how each
change was built and verified, the decisions taken and the questions still open for the user (sections 8, 9,
10.2 to 10.4, 11 to 13, 14.5 and 16). The two files share one numbering, so a reference such as "I2" or
"question 13.1.6" means the same in either. They were one file until 2026-10-16, when the user's limit of
1000 lines per file (section 15) split them.

---------------------------------------------------------------------------------------------------

## 0. Precedence

1. **P10 overrides CC and HF** whenever they conflict (user instruction).
2. CC and HF agree on substance. Where they differ in emphasis, both say the same thing: pick
   the simplest solution that works (CC11, HF13).
3. Where the books leave a detail open (brace style, naming case, file layout), the project's
   existing code is the standard (CC5 "team rules", CC-G24). Observed convention: functions
   `PascalCase`, variables and parameters `snake_case`, constants `UPPER_SNAKE_CASE`.
4. A rule may be broken only with the user's explicit approval, and the exception is justified
   in a comment at the place where it happens.
5. The user's own rules for this project (sections 10 and 15) bind like P10's and come before the books.
6. GSFC (section 14) ranks below everything above: it was adopted only where it conflicts with
   none of it, and a conflict found later is settled in favour of the earlier rule.

## 1. Conflicts between the books and how they are resolved (P10 wins)

| # | Topic | CC / HF say | P10 says | Binding resolution |
|---|-------|-------------|----------|--------------------|
| 1 | Error reporting | Prefer exceptions to error codes; unchecked exceptions; write try/catch first (CC3, CC7) | Only simple control flow, no setjmp/longjmp (P10-1). Every caller checks every non-void return value and errors propagate up the call chain (P10-7). A failed assertion triggers an explicit recovery, e.g. returning an error (P10-5) | **No `throw` in project code.** Report failure through return values (status enum, bool, or a small result struct) that every caller checks. Code that can throw (std library, third party) is wrapped at the boundary (CC8) so nothing propagates; prefer non-throwing APIs. CC's intent still applies: keep error handling apart from the happy path (guard-clause early returns, error handling in its own function), say what failed and why, shape status values around what the caller has to tell apart (CC7), and keep status enums local to their module rather than one global error enum (the CC3 "dependency magnet") |
| 2 | Command-query separation | Commands should not return error codes (CC3) | Errors are returned and checked (P10-7) | A command may return a success/failure status **and nothing else**. Queries return data and have no side effects |
| 3 | Data from an operation that can fail | No output arguments (CC3, CC-F2). Don't return null (CC7) | Return values are checked (P10-7) | Return a small result struct (status + value) or a Special Case value. Output parameters only where a Win32 API signature forces them |
| 4 | Widening scope to shorten argument lists | Promote locals to member variables so extracted functions need no arguments (CC3, CC10) | Declare data at the smallest possible scope (P10-6) | Don't widen a variable's scope just to save arguments. Pass it (max 3 arguments), or extract a cohesive class that owns exactly that data |
| 5 | Polymorphism | Replace switch/if chains with polymorphism (CC-G23). Strategy, Observer, Command, State through interfaces (HF) | No function pointers (P10-9). The call graph must be acyclic and analyzable (P10-1) | Allowed: C++ virtual functions on small, closed hierarchies (see I1). Forbidden in our code: function pointers, pointers to member functions, `std::function`, stored lambdas, callback tables (Win32-mandated exceptions: I2). Dispatch must never create a call cycle |
| 6 | Creating objects at run time | Factories, decorators, command objects, observer registration at any time, lazy init (HF2-HF6). `new` everywhere in the examples | No dynamic memory allocation after initialization (P10-3) | All objects are created and wired at startup. CC11 "separation of main" says the same, so the two reinforce each other. After startup: no `new`/`malloc`/`make_unique`/`make_shared`, no growing std containers, no allocating `std::wstring`/`std::string` operations, no thread creation. Use fixed-capacity storage sized by named compile-time constants (`std::array`, fixed `wchar_t` buffers, pools allocated at startup). Factories run only at startup or hand out pre-built instances. Observer lists are fixed-capacity arrays filled at startup |
| 7 | Recursion | Recursive traversal (Composite, HF9/HF12; recursive helpers in CC listings) | No direct or indirect recursion (P10-1) | Traverse trees iteratively, with an explicit fixed-capacity stack and a bounded loop (P10-2) |
| 8 | Function size | "Small, then smaller": hardly ever 20 lines, ideally 2-5, blocks of one line, indentation depth <= 2 (CC3) | <= ~60 lines, one statement or declaration per line (P10-4) | 60 lines is the hard ceiling. CC's sizes are the target |
| 9 | Assertions vs brevity | Keep functions tiny (CC3). Assertions "don't solve" a null argument (CC7) | >= 2 assertions per function on average, each with recovery (P10-5). Every function validates every parameter (P10-7) | Keep P10's assertion density and parameter checks even when a function gets longer. Never drop a meaningful check to make code shorter |
| 10 | Lazy initialization | Discouraged in runtime logic (CC11). HF5's Singleton creates its instance lazily | No allocation after initialization (P10-3) | Construct everything eagerly at startup |
| 11 | Sharing data between threads | Give each thread a copy of the data (CC13) | No allocation after initialization (P10-3) | Copy into buffers allocated at startup |

## 2. Interpretations for C++/Win32 (adopted 2026-10-13; the user may overrule any of them)

- **I1** P10-9 bans *function pointers*. Virtual functions are not function pointers, and a closed
  program's set of overriders is known at compile time, so analyzers can resolve the call graph
  (class-hierarchy analysis). Virtual functions are therefore allowed, with constraints: shallow
  hierarchies, leaf classes `final`, and no virtual call that can re-enter its caller (P10-1).
- **I2** Win32 forces some function pointers on us: window, dialog and subclass procedures,
  enumeration callbacks, thread entry points. These are the only allowed function pointers. Each
  one carries a comment saying the API requires it, which is P10-9's "strong justification". Keep
  them thin: validate the parameters, then call an ordinary function.
- **I3** The Win32 message loop (and modal loops the OS runs) is the only loop without a fixed
  bound. It is the program's event loop, so it falls under the same exception P10-2 makes for a
  scheduler. Every other loop needs a provable fixed bound.
- **I4** P10-9's "one level of dereferencing" counts explicit dereferences written in the source
  (`*p`, `p->x`, indexing through a pointer). The implicit `this` does not count. So
  `observer->OnChanged()` is fine; `a->b->c` and `**p` are not.
- **I5** "Initialization" means everything in `wWinMain` before the message loop starts, including
  the main window's `WM_CREATE`. Dialogs opened later run after initialization.
- **I6** P10 governs production code. Test code (`UnitTesting_FRCP`) follows every CC cleanliness
  rule and must compile warning-free (P10-10). Under CC9's "dual standard" it may ignore efficiency
  limits, such as heap use inside test fixtures. Production code under test must comply in full.
- **I7** Re-entrancy the OS causes (`CreateWindow` sending `WM_CREATE`, `SetWindowText` sending
  `WM_SETTEXT`, ...) is a platform boundary, not recursion in our code. Our code must not cause it
  on purpose: no `SendMessage` to our own window from inside that window's handler. Call the
  handler function directly, or use `PostMessage`.
- **I8** P10-10 on MSVC: the "most pedantic setting" is `/Wall`, plus `/WX`. Windows SDK and other
  external headers are isolated with `/external:anglebrackets /external:W0` so the rule applies to
  our own code. If a purely informational `/Wall` diagnostic cannot be fixed in source (e.g.
  optimizer inlining notes), ask the user before disabling it. Never disable one silently.

## 3. P10: The Power of Ten (highest priority)

Philosophy: a few clear rules that can be checked mechanically. They are strict ("Draconian") on
purpose, like a seat belt: uncomfortable at first, then second nature. Rules 1-2 give clear,
analyzable control flow. Rule 3 removes a whole class of memory errors. Rules 4-7 are broadly
accepted good style and carry the benefits of design by contract.

1. **Simple control flow only.** No `goto`, no `setjmp`/`longjmp`, no direct or indirect
   recursion: the call graph must be acyclic. A single exit point is *not* required; an early
   error return is fine. (No exceptions either, see section 1 row 1.)
2. **Every loop has a fixed upper bound** that a tool could prove statically. If the bound can't be
   proven, the rule is violated. Loops with a variable iteration count (list walks, string scans,
   searches) get an explicit maximum; exceeding it is an assertion failure and the function returns
   an error. A loop that is meant never to end must provably never end (see I3).
3. **No dynamic memory allocation after initialization.** Why: allocators behave unpredictably, and
   leaks, use-after-free, over-allocation and overruns all come from them. The stack is the only
   memory claimed at run time, and without recursion its upper bound can be derived statically.
   It follows that there are no VLAs, no `alloca`, and no stack use whose size isn't known at
   compile time.
4. **No function longer than one printed page:** about 60 lines, with one statement and one
   declaration per line. Each function is one logical unit you can understand and verify on its
   own. Long functions are a sign of poorly structured code.
5. **Assertion density of at least 2 per function on average.** An assertion is a Boolean test with
   no side effects that checks for something that should never happen: a precondition,
   postcondition, parameter value, return value, or loop invariant. When one fails, take an
   explicit recovery action, e.g. return an error status to the caller. An assertion that a tool
   can prove always holds, or never holds, doesn't count (no `assert(true)`). Form: modelled on
   P10's `c_assert`, used as `if (!c_assert(expr)) { return <error>; }`. The macro logs file,
   line and `#expr` (in this GUI app via `OutputDebugStringW`) and evaluates to `false`. Do **not**
   use `assert()` from `<cassert>`: it aborts instead of recovering, and it disappears in Release.
6. **Declare every data object at the smallest possible scope.** Never reuse one variable for
   unrelated purposes. Preference order: local, then member, then file-local (`static` or anonymous
   namespace), then global. No `extern` globals.
7. **Check every non-void return value, and validate every parameter inside every function.** Where
   success and failure would get the same response (e.g. ignoring an informational result), cast the
   call to `(void)` to show the choice is deliberate; in doubtful cases add a comment saying why.
   Propagate errors up the call chain. "It will often be easier to comply with the rule than to
   explain why non-compliance might be acceptable."
8. **The preprocessor is only for `#include` and simple macros.** No token pasting (`##`), no
   variadic macros (`...`/`__VA_ARGS__`), no recursive macros. Every macro expands to a complete
   syntactic unit. Conditional compilation (`#if`/`#ifdef`) only rarely (one or two in the whole
   project, beyond include guards or `#pragma once`), and each use is flagged and justified in a
   comment: 10 such directives can mean up to 2^10 code versions to test. The resource editor's
   `#define`s in `Resource.h` are simple macro definitions and are fine.
9. **Restrict pointers.** At most one level of dereferencing (no `**p`, no `p->q->r`; see I4). No
   dereference hidden inside a macro or a `typedef`/`using` alias, so don't declare `LP...`/`P...`
   style aliases for data we dereference. Win32 handle types (`HWND`, `HINSTANCE`, ...) are opaque
   and never dereferenced, so they are fine. No function pointers (see I1, I2).
10. **Pedantic warnings and static analysis from day one.** All compiler warnings on at the most
    pedantic setting, and the code compiles with zero warnings (see I8). Check the code with at
    least one, preferably several, state-of-the-art static analyzers (MSVC Code Analysis
    `/analyze`, clang-tidy, cppcheck) with zero findings: at least daily, and here before any change
    is called done. Zero means zero even for a false positive: rewrite the code until it is
    trivially valid rather than suppressing the warning.

## 4. CC: Clean Code

### 4.1 Mindset (CC1, CC14-16, foreword)

- The only way to go fast is to keep the code clean at all times; a mess slows you down
  immediately. "Later equals never" (LeBlanc's law).
- It is not enough for code to work: working code is often badly broken. First make it work, then
  make it right. Clean code comes from successive refinement: write the rough draft, then clean it.
- **Boy Scout Rule**: leave the code a little cleaner than you found it. Keep it small (one better
  name, one split function, one removed duplication), in code you are already touching.
- **Incrementalism**: never make massive structural changes in one go. Make many tiny,
  behaviour-preserving steps, with the tests passing after each one; the system works at all times.
- We are authors. Code is read far more than it is written (well over 10:1), so optimize for the
  reader. Making code easy to read makes it easier to write.
- What clean code is:
  - It does one thing well. Its logic is straightforward, its dependencies minimal, its error
    handling complete according to an articulated strategy, and its performance close to optimal
    so nobody is tempted into messy optimizations (Stroustrup).
  - It reads like well-written prose, with crisp abstractions and straightforward lines of control
    (Booch).
  - Others can read and enhance it. It has tests and meaningful names, offers one way to do one
    thing, keeps its dependencies minimal and explicit, and exposes a clear, minimal API (Thomas).
  - It looks like it was written by someone who cares (Feathers).
  - Each routine turns out to be pretty much what you expected (Cunningham).
- Broken windows: bad code tempts people to add more bad code.
- 5S: sort (good names), systematize (everything where a reader expects it), shine (no litter, no
  commented-out code), standardize (one shared style), self-discipline (keep doing it and reflect).
- Defend the code with the same passion a manager defends the schedule.

### 4.2 Names (CC2, CC-N1..N7)

- Reveal intention: a name says why the thing exists, what it does and how it is used. If a name
  needs a comment, it has failed. Include units (`elapsed_time_in_days`).
- No disinformation: no false clues (don't say "list" unless it is one), no names that differ only
  slightly, no `l`/`O` that look like `1`/`0`.
- Make meaningful distinctions: no number series (`a1`, `a2`), no noise words (`Info`, `Data`,
  `Object`, `the...`, `a...`).
- Names are pronounceable and searchable. Single letters only as loop counters in tiny scopes
  (`i`, `j`, `k`, never `l`). The longer the scope, the longer the name (N5).
- No encodings: no Hungarian notation, no `m_`, no subsystem prefixes, no `I` prefix on
  interfaces (if one side must be encoded, encode the implementation). Win32's own parameter names
  (`hWnd`, `wParam`, `lParam`) are standard nomenclature in API-mandated signatures (N3): keep
  them there, but don't invent new encoded names.
- No mental mapping: clarity is king.
- Classes are nouns; avoid `Manager`, `Processor`, `Data`, `Info`. Functions are verbs. Accessors
  and predicates are named for their value (get/set/is). When constructors would be overloaded,
  prefer named creation functions that describe the arguments.
- Don't be cute: say what you mean, mean what you say.
- One word per concept (not fetch/retrieve/get side by side, not controller/manager/driver). No
  puns: don't use one word for two different ideas.
- Use solution-domain names (CS terms, pattern names) for technical things and problem-domain names
  (civs, pools, drafts, lobbies, histories) for domain things. Use the project's shared vocabulary
  consistently (N3).
- Add meaningful context through enclosing classes, functions and namespaces. Don't add gratuitous
  context, such as a project prefix on everything.
- Choose names at the right level of abstraction, not names that reveal the implementation (N2).
  Make them unambiguous (N4), and let them describe side effects (N7: `CreateOrGetX`, not `GetX`).
- Rename whenever you find a better name; names drift as meaning evolves (N1).

### 4.3 Functions (CC3, CC-F1..F4)

- Small, then smaller (hard ceiling P10-4). The body of an `if`/`else`/`while` is ideally one line,
  a function call. Indentation depth at most 2.
- **Do one thing.** Every statement is one level of abstraction below the function's name. If you
  can extract a function whose name isn't just a restatement of the code, the function does more
  than one thing. A function divided into sections does more than one thing.
- One level of abstraction per function. **Stepdown rule**: the code reads top-down, each function
  followed by the ones at the next level down.
- Switch statements: at most one switch per kind of selection (the "ONE SWITCH" rule, CC-G23). It
  is used to create or select polymorphic objects and is buried in a low-level place; everywhere
  else uses polymorphism (limited by section 1 row 5). For Win32: a procedure's message switch is
  that one dispatcher, and each case is a single call to a handler.
- Use descriptive names. A long descriptive name beats a short enigmatic one, and beats a
  descriptive comment. Phrase related functions consistently.
- Arguments: 0 is best, then 1, then 2. Avoid 3 where possible. More than 3 needs very special
  justification. Groups of arguments that travel together become a struct (argument object).
  API-mandated callback signatures are exempt.
- A function with one argument either asks a question about it, transforms it and returns the
  result, or handles an event (named clearly as one). Avoid other single-argument forms.
- No flag (boolean) arguments (F3) and no selector arguments of any type (CC-G15): split the
  function instead.
- Use two arguments only for ordered components of one value. Encode the argument order in the
  name (keyword form).
- **No side effects**: no hidden changes to members, parameters or globals. If a temporal coupling
  is unavoidable, make it visible in the name or by passing each step's result to the next
  (CC-G31).
- No output arguments (F2): change the state of the owning object instead (exception: section 1
  row 3).
- Command-query separation (as amended by section 1 row 2).
- Error handling is one thing: a function that handles errors does nothing else (section 1 row 1).
- **DRY**: duplication may be the root of all evil in software.
- Small functions may use early `return`, `break` and `continue`. Never `goto` (P10-1).
- Writing functions: get it working under tests first, then split, rename, remove duplication and
  reorder.
- Functions are the verbs and classes the nouns of the system's domain-specific language. The goal
  is to tell the story of the system.
- Delete functions that are never called (F4).

### 4.4 Comments (CC4, CC-C1..C5)

- Comments make up for failing to express yourself in code. Prefer code: extract a well-named
  function or variable. Don't comment bad code, rewrite it. An inaccurate comment is worse than
  none; the truth is only in the code.
- Acceptable: legal headers; informative comments where a name can't carry the information;
  explanation of intent; clarification of an obscure API result you can't change; warning of
  consequences; TODO (scan and clear regularly, never an excuse for bad code); amplification; doc
  comments on genuinely public APIs only.
- Required by P10: justification for ignored return values in doubtful cases (P10-7), for
  conditional compilation (P10-8), for each Win32-mandated function pointer (I2), and for any
  user-approved rule exception.
- Not acceptable: mumbling; redundant comments; misleading comments; mandated per-function or
  per-variable comments; journal/changelog comments; noise; position banners (very rarely, if
  ever); closing-brace comments; bylines (git knows); **commented-out code (delete it, C5)**; HTML
  in comments; nonlocal information; too much information or history; comments with no obvious
  connection to the code; header comments on short, well-named functions; obsolete comments (C2);
  inappropriate information that belongs in git or the tracker (C1).
- A comment worth writing is worth writing well: brief, correct grammar, no stating the obvious
  (C4).

### 4.5 Formatting (CC5)

- Formatting is communication. Use the project's existing style everywhere (team rules), and
  automate it where possible.
- Files small: typically ~200 lines, with 500 as a desirable upper limit.
- Newspaper metaphor: high-level concepts at the top, detail increasing downward.
- A blank line between concepts; lines that belong together are vertically dense.
- Vertical distance: closely related concepts stay close. Locals are declared near their first use;
  loop control variables inside the loop statement. Member variables live in one well-known place.
  Callers sit above callees (C++: forward-declare, then order definitions top-down). Code with
  conceptual affinity is grouped.
- Lines short: at most 120 characters.
- Use horizontal whitespace to show association and precedence. Don't align declarations or
  assignments into columns.
- Never collapse a scope onto one line to skip indentation. Avoid dummy loop bodies; if one is
  unavoidable, give it braces and put it on its own line.

### 4.6 Objects and data structures (CC6, CC-G36)

- Hide implementation behind abstractions. Don't blindly add getters and setters; express data in
  abstract terms.
- Objects hide their data and expose behaviour. Data structures expose data and have no meaningful
  behaviour. Choose deliberately: objects make it easy to add new types, procedural code makes it
  easy to add new functions.
- No hybrids that are half object and half data structure.
- **Law of Demeter**: a method `f` of class `C` calls methods only of `C`, of objects `f` creates,
  of objects passed to `f`, and of objects held in `C`'s members. No train wrecks
  (`a.GetB().GetC().Do()`): tell, don't ask. P10-9 reinforces this.
- Plain structs used to carry data around are fine; keep business rules out of them.

### 4.7 Error handling (CC7, as amended by section 1 row 1)

- Error handling must never obscure the logic. Keep it separate from the main algorithm.
- Give every failure context: what operation failed and how.
- Define failure values by how callers need to handle them. Wrap third-party APIs so they report
  failure in our own terms, with one status type per boundary.
- Define the normal flow with the Special Case pattern (an object or value that handles the special
  case), so callers don't have to handle exceptional paths.
- Don't return null; return a special-case or empty value. Don't pass null: a null argument is a
  programming error, caught by P10-7's parameter checks and recovered from as P10-5 requires.

### 4.8 Boundaries (CC8)

- Wrap third-party interfaces (Win32, std containers, vcpkg libraries) in our own classes. Don't
  pass boundary types around the system or through our public interfaces.
- Write learning tests and boundary tests for third-party behaviour we depend on.
- For an API that doesn't exist yet, write the interface you wish you had, and adapt to the real
  one later (Adapter).
- Depend on what you control. Only a few places in the code refer to third-party code.

### 4.9 Unit tests (CC9, CC-T1..T9, CC-E1/E2)

- The three laws of TDD: (1) no production code until there is a failing test; (2) no more test
  than is enough to fail, and not compiling counts as failing; (3) no more production code than is
  enough to pass.
- Test code is as important as production code and is kept just as clean: readability,
  readability, readability.
- Structure tests as build-operate-check (given-when-then), and grow a domain-specific testing API.
- Dual standard: test code may be less efficient than production code, never less clean (I6).
- One concept per test; minimize the asserts per concept.
- F.I.R.S.T.: Fast, Independent, Repeatable, Self-validating, Timely (written just before the code).
- T1 test everything that could break. T2 use a coverage tool. T3 don't skip trivial tests. T4 an
  ignored test is a question about an ambiguity. T5 test boundary conditions. T6 test exhaustively
  near bugs. T7/T8 patterns of failure and of coverage are revealing. T9 tests must be fast.
- E1: building the project is one step. E2: running all the tests is one step.

### 4.10 Classes (CC10)

- Lay out every class the same way: constants, static members, member variables, then public
  functions, each followed by the private helpers it uses (stepdown). Follow the existing layout in
  this project's headers.
- Keep variables and helpers private. Loosening encapsulation is a last resort (tests only).
- Classes are small, measured in responsibilities. Each has a concise name (no Manager, Processor,
  Super) and can be described in ~25 words without "if", "and", "or" or "but".
- **SRP**: one reason to change. Prefer many small classes to a few large ones.
- **High cohesion**: few member variables, and each method uses most of them. When a subset of
  methods shares a subset of variables, split the class.
- **OCP**: organize the code so new features are added as new code (e.g. a new class) instead of by
  modifying existing classes. Act on this when you find yourself having to open a class.
- **DIP**: depend on abstractions (interfaces), not concrete details. This is also what makes
  testing possible.

### 4.11 Systems (CC11)

- Separate building the system from using it. All objects are built and wired at startup
  (`wWinMain` plays the role of "main"); dependencies point away from main, and the rest of the
  application assumes everything is already built (this is also P10-3).
- When the application must control *when* something is created, use an Abstract Factory
  (P10: handing out pre-allocated objects).
- Dependency injection: classes receive their collaborators (constructor arguments) instead of
  constructing them.
- No lazy-initialization idioms scattered through runtime logic.
- Architecture can grow incrementally when concerns are separated. Implement today's needs; no Big
  Design Up Front, but don't go in rudderless either.
- Postpone decisions to the last responsible moment.
- Use standards wisely, only where they add demonstrable value.
- The code should read like the domain (a domain-specific language), with clear intent at every
  level.
- Use the simplest thing that can possibly work.

### 4.12 Simple design (CC12: Kent Beck's rules, in priority order)

1. Runs all the tests. A testable system pushes the design toward small, decoupled classes.
2. Contains no duplication, including duplicated implementation. Use Template Method or Strategy
   for duplicated algorithms.
3. Expresses the programmer's intent: names, small functions and classes, standard pattern names,
   tests as documentation.
4. Minimizes the number of classes and methods. This is the lowest priority; resist dogma such as
   an interface for every class.

### 4.13 Concurrency (CC13, CC Appendix A)

- Concurrency decouples *what* gets done from *when*. It doesn't always improve performance, it
  changes the design, and it adds overhead. Its bugs don't repeat reliably, so treat every
  "one-off" failure as a real defect.
- SRP: thread-management code lives apart from other code and does nothing else, in a few
  well-controlled places.
- Limit the scope of shared data and protect it. Prefer copies (into buffers allocated at startup,
  P10-3). Make threads as independent as possible.
- Know your library's thread-safe facilities and the basic execution models: producer-consumer,
  readers-writers, dining philosophers.
- Avoid calling more than one method on a shared object. If you must, prefer server-based locking
  (the shared object locks internally) over client-based locking.
- Keep critical sections small, and don't call one locked section from another.
- Deadlock needs all four of: mutual exclusion, lock-and-wait, no preemption, circular wait. Break
  one of them, usually with a global order for acquiring locks.
- Think about graceful shutdown early, and get it working early.
- Testing: get the non-threaded code working first. Make threaded code pluggable and tunable. Run
  with more threads than processors, on every target platform. Instrument the code ("jiggle") to
  force failures.

### 4.14 General heuristics (CC17)

- G1 One language per source file, as far as possible.
- G2 Implement the obvious behaviour (principle of least surprise).
- G3 Behave correctly at every boundary and corner case. Find each one and test it.
- G4 Never override safeties: don't turn warnings off or disable failing tests (= P10-10).
- G5 Eliminate duplication. Copy-paste becomes a function, repeated switches become polymorphism,
  similar algorithms become Template Method or Strategy.
- G6 Put code at the right level of abstraction, and separate the levels completely: base
  abstractions know nothing of details.
- G7 Base classes know nothing about their derivatives.
- G8 Keep interfaces small. Hide data, helpers, constants and temporaries; few methods, few
  members.
- G9 Delete dead code.
- G10 Define things close to where they are used: locals just above first use, private functions
  just below first use.
- G11 Be consistent: do similar things the same way.
- G12 No clutter: empty constructors, unused variables and functions, noise comments.
- G13 No artificial coupling: general enums and functions don't live inside specific classes.
- G14 No feature envy: a method works with its own class's data. The exception is when moving the
  method would couple a class to something like a report format.
- G15 No selector arguments.
- G16 Don't obscure intent with run-on expressions, Hungarian notation or magic numbers.
- G17 Put code where a reader expects to find it.
- G18 Prefer non-static member functions, unless there is no chance the function will need to be
  polymorphic.
- G19 Use explanatory variables for intermediate values.
- G20 Function names say what they do, including units and whether they mutate or return a new
  value.
- G21 Understand the algorithm: know the code is correct, not just that the tests pass.
- G22 Make logical dependencies physical: ask the module that owns a fact instead of assuming it.
- G23 Prefer polymorphism to if/else and switch, under the ONE SWITCH rule (limited by section 1
  row 5).
- G24 Follow standard conventions; this project's existing code is the convention.
- G25 Replace magic numbers and magic strings with named constants (see `GlobalConstants.h`).
- G26 Be precise: check every failure, don't assume the first match is the only one, don't use
  floating point for exact quantities, lock where concurrent update is possible.
- G27 Prefer structure over convention: enforce design decisions with types and abstract methods,
  not naming conventions.
- G28 Encapsulate conditionals in well-named functions.
- G29 Avoid negative conditionals.
- G30 Functions do one thing.
- G31 Don't hide temporal couplings.
- G32 Don't be arbitrary: the structure communicates the reasons behind it.
- G33 Encapsulate boundary conditions: compute `next_level = level + 1` once.
- G34 Functions descend only one level of abstraction.
- G35 Keep configurable data at high levels and pass it down.
- G36 Avoid transitive navigation.
- J3 (generalized) Use enums (`enum class` in C++) instead of integer constants.

## 5. HF: Design principles and patterns

### 5.1 OO design principles

OO basics (abstraction, encapsulation, polymorphism, inheritance) don't make a good designer on
their own. Good OO designs are reusable, extensible, maintainable, and cope with change. Most
principles and patterns address change.

1. **Encapsulate what varies**: identify the aspects of the application that vary and separate them
   from what stays the same (HF1). This is the basis of almost every pattern.
2. **Program to an interface, not an implementation**, meaning program to a supertype. Variables
   are declared as the supertype, and the concrete object is assigned from outside (HF1).
3. **Favor composition over inheritance**: HAS-A can be better than IS-A, and composed behaviour
   can be swapped at run time (HF1). Maintainability and extensibility matter more than reuse
   through inheritance.
4. **Strive for loosely coupled designs between objects that interact** (HF2).
5. **Classes should be open for extension but closed for modification** (HF3). Apply this only
   where change is likely: applying it everywhere is wasteful and makes code complex.
6. **Depend upon abstractions, not concrete classes** (DIP, HF4). High-level and low-level
   components both depend on abstractions. Guidelines to strive for, not laws: no variable holds a
   reference to a concrete class (use a factory); no class derives from a concrete class; no method
   overrides an implemented method of a base class. Break them knowingly, e.g. for very stable
   classes.
7. **Principle of Least Knowledge: talk only to your immediate friends** (HF7). From a method, call
   only methods of the object itself, objects passed in as parameters, objects the method creates,
   and components of the object. Trade-off: it adds wrapper methods.
8. **Hollywood Principle: don't call us, we'll call you** (HF8). High-level components decide
   when and how low-level ones are used; low-level components never call high-level ones directly.
   No circular dependencies, which supports P10-1.
9. **A class should have only one reason to change** (SRP, HF9). Keep cohesion high.

HF calls principles guidelines with trade-offs (abstraction vs speed, space vs time). In this
project that latitude never extends to P10 rules.

### 5.2 Using patterns (HF1, HF12, HF13)

- A pattern is a solution to a problem in a context: discovered, proven experience. Patterns give
  general solutions, not code.
- **KISS**: aim for simplicity, not "which pattern can I apply here?". Patterns are not a magic
  bullet.
- Introduce a pattern only when you're sure it solves a real problem in the design. Consider a
  simpler solution first, and check the pattern's consequences for the rest of the design.
- Use a pattern for expected variation only when the change is practical and likely, not
  hypothetical. **"If you don't need it now, don't do it now."**
- Refactoring time is patterns time: e.g. code full of conditionals may call for State, concrete
  dependencies for a Factory.
- Remove a pattern whose flexibility isn't needed.
- Let patterns emerge naturally. Think about the design, not about patterns. No "pattern for
  Hello World".
- Patterns add classes, layers and complexity, and can cost efficiency. Weigh that.
- Patterns are tools, not rules: adapt them, and document how an adaptation differs from the
  classic form.
- Shared vocabulary: name classes and methods after the patterns they implement (also CC-N3).
  Mention a pattern in a comment only when a name can't carry it.
- A compound pattern combines patterns to solve a general problem. Don't stack patterns for their
  own sake (HF calls its duck simulator "forced and artificial").
- GoF maxims: aim for practical extensibility, not hypothetical generality (Vlissides). Go for
  simplicity and don't get overexcited (Helm). If a simpler solution without a pattern exists, use
  it (Johnson). Patterns are tools, not rules (Gamma).
- Anti-patterns: learn to recognise attractive bad solutions, e.g. the Golden Hammer (forcing one
  familiar technology onto every problem).

### 5.3 Pattern notes, with P10 constraints

- **Strategy**: a family of interchangeable algorithms behind one interface. The client is composed
  with one of them and switches by re-pointing to another pre-built strategy.
- **Observer**: one-to-many. The subject knows its observers only through their interface, and
  observers come and go without the subject changing. Pulling state is preferred over pushing it.
  Never rely on notification order. P10: a fixed-capacity registration array filled at startup;
  notifications never re-enter the subject.
- **Decorator**: the same type as what it decorates, adding behaviour before or after it. Creates
  many small objects, so use it sparingly. Compose at startup.
- **Factory Method / Abstract Factory**: encapsulate creation and decouple clients from concrete
  types. P10: they run at startup or return pre-built instances.
- **Singleton**: at most one instance, plus global access. It breaks loose coupling and arguably
  SRP, so use it sparingly and prefer passing the object in. P10: a static instance created at
  startup, never lazily.
- **Command**: decouples the invoker from the receiver; commands can be queued, logged or undone.
  P10: commands pre-allocated, queues of fixed capacity.
- **Adapter**: converts an interface into the one clients expect (wraps a boundary). **Facade**:
  simplifies a subsystem's interface and decouples clients from the subsystem.
- **Template Method**: the algorithm's skeleton lives in a base class and its steps in subclasses;
  hooks are optional steps. The template method itself can't be overridden (C++: a non-virtual
  public function calling virtual steps).
- **Iterator**: traversal without exposing the representation. **Composite**: treat parts and
  wholes uniformly, trading transparency against safety. P10: iterate, never recurse.
- **State**: each state is a class and the context delegates to the current one. It replaces
  conditional-heavy state logic, at the cost of more classes. State and Strategy share a class
  diagram but differ in intent.
- **Proxy**: controls access to an object (remote, virtual, protection). Decorator adds behaviour;
  Proxy controls access.

## 6. MVC in this project (HF12 applied with the rules above)

Roles:

- **Model**: all data, state and application logic (civ pool, draw rules, histories, settings). It
  knows nothing about the View or the Controller. It offers an interface to change and read its
  state, and notifies registered observers when its state changes (Observer). It is plain C++, with
  no Win32 types and no `<windows.h>`, so it can be unit-tested in `UnitTesting_FRCP` (CC8, CC9).
- **View**: presentation only. When notified, it reads the state it displays directly from the
  model (pull). It hands every decision about UI behaviour to its controller (Strategy). The Win32
  window and control tree is the Composite. The view knows *how* to enable or disable controls,
  never *when*.
- **Controller**: receives user input from the view (`WM_COMMAND`, hotkeys, menus), interprets it,
  and calls the model. It may tell the view to change (enable, disable, show). It contains **no
  application logic**; that lives in the model. It is swappable, being the view's strategy. It may
  also observe the model when model state dictates UI state. One controller per view at run time.

Rules:

- Flow: the user acts on the View, the View tells the Controller, the Controller changes the Model
  (and may adjust the View), the Model notifies its observers, the View pulls state from the Model.
- The view never changes model state itself. Give it read-only access (a const/read-only interface,
  or an adapter exposing a subset).
- The model has no dependency on views or controllers, so one model can serve several views.
- Use an Adapter to fit a different model to an existing view and controller.
- Never put controller logic in view code: it gives the view two responsibilities and couples it
  to the model.

Win32/P10 application:

- Build the model, views and controllers and register the observers in `wWinMain` before the
  message loop (CC11, P10-3), and pass them in where needed. No globals: a window or dialog
  procedure reaches its object through window user data (`GWLP_USERDATA`/`DWLP_USER`, set during
  `WM_NCCREATE`/`WM_INITDIALOG`).
- Window and dialog procedures are thin view adapters. They validate their parameters, dispatch
  each message to exactly one handler (ONE SWITCH), and stay within 60 lines.
- The model has one observer slot (the view), filled by the controller at startup; a silent Special
  Case observer (CC7) stands in until then and in unit tests. A second observer would mean a
  fixed-capacity array filled at startup; it is not needed now (HF13). Notification is synchronous.
  An observer never calls model mutators during a notification (no recursion) and never depends on
  notification order.

## 7. Checklist before calling any change done

- [ ] No recursion, `goto`, `setjmp`/`longjmp` or exceptions (P10-1).
- [ ] Every loop has a provable fixed bound; the message loop is the only exception (P10-2, I3).
- [ ] No heap allocation and no thread creation outside startup (P10-3).
- [ ] Every function is at most 60 lines and meets CC's size and shape targets (P10-4, CC3).
- [ ] At least 2 meaningful, side-effect-free assertions per function on average, each with a
      recovery action (P10-5).
- [ ] Every variable is at its smallest possible scope (P10-6).
- [ ] Every return value is checked or `(void)`-cast, and every parameter is validated (P10-7).
- [ ] Preprocessor use is limited (P10-8); pointer rules hold (P10-9).
- [ ] Zero warnings at the pedantic level and zero static-analysis findings (P10-10). If I couldn't
      run the build or the analyzer, I say so explicitly.
- [ ] Names, functions, comments and formatting follow CC; no duplication; tests written first and
      passing (CC9).
- [ ] Design principles respected; patterns only where they solve a present problem (HF).
- [ ] Model/View/Controller separation respected (section 6).
- [ ] Editions, DLCs and civs come from data.json only; no name, ID, count, list or layout of
- [ ] No file is over 1000 lines, and no folder holds over 20 files besides the asset folders
      (section 15).
      theirs in code, resources, project files or tests (section 10).
- [ ] The GSFC rules of section 14.2 hold.
- [ ] Boy Scout: the code I touched is a little cleaner than before.

Sections 8 and 9 (the state of the code, the parchment UI) are in CLAUDE2_LOG.md.

## 10. Data and code (user rule, 2026-10-14)

The user's rule: **strict separation between data and code. `data.json` is the only place where
editions, DLCs and civs may reside.** It is the same file as the web version's
(`frcp_html/data.json`); the two are kept identical.

### 10.1 What the rule means here

- No edition, DLC or civ name, ID, key, count, list, per-civ or per-DLC constant, resource entry,
  control ID or layout in code, resources, project files or tests. Code reads them from `CivCatalog`
  (model, plain C++) or derives them from it: a civ's jingle and icon files are its name in lower
  case; an edition's and a DLC's icon files are named by their "icon" (and "banner"). The atlases are
  built from those files at startup (section 12): the civ icon atlas holds the civs in ID order and
  Random after them; the DLC atlas holds three looks of each edition's icon in data.json's order, then
  the icons of the DLCs that have one; the edition banners are stacked in data.json's order.
- Limits are capacities, not data (`CivDataLimits.h`): 3 editions, 128 DLCs, 128 DLC toggles per
  edition, 64 civs, 4 links per edition, set by the main window's room (Q/W/E pick the editions by
  their place; an edition's DLC toggles scroll once they don't fit, and A to L toggle its first nine
  by their place). A file beyond them, or with a broken reference, is refused at startup with a
  message that says what is wrong, and the program ends.
- Files named after civs (jingles, history texts) are data files next to the exe, not code.
- Unit tests use the real data.json; a file with a problem is data.json with one thing changed.

10.2 to 10.4 (how the separation was built and verified, its decisions) and sections 11 to 13 are in
CLAUDE2_LOG.md.

## 14. GSFC: NASA Goddard's C++ standard (adopted 2026-10-16)

The user asked for `mab583.pdf` to be added to these rules, leaving out anything that conflicts
with the existing ones, and for the whole project to comply.

### 14.1 How it was adopted

- A GSFC rule is left out when it contradicts P10, CC, HF, the interpretations of section 2 or
  the user's rules (section 10), or the project's own convention, which section 0.3 makes the
  standard for details the books leave open. A convention here is one form used throughout the
  code: `T* p` (246 to 0), unparenthesized conditions of `?:` (211 of 216). Where the code was
  mixed (switches with and without a default case: 10 to 16), GSFC's rule was adopted.
- What GSFC itself calls optional ("may", "suggested", "can") stays optional. Rules about GMAT,
  CVS, C code and Doxygen don't apply here. Everything left out is listed in 14.3.
- Rules that only restate one above are tagged with it and not repeated as new.

### 14.2 The rules

Names
1. Names are meaningful and readable (GSFC-2, = CC 4.2). An abbreviation is written one way
   throughout, all capitals or an initial capital, never both (`Dlc` everywhere, not `DLC` in
   some names); UPPER_SNAKE_CASE constants are all capitals anyway (GSFC-2).
2. Classes, structs, enum types, type aliases and named namespaces are PascalCase (GSFC-2.1,
   2.6, 2.8, 2.9). Functions are verbs in PascalCase (GSFC-2.4). A method's name doesn't repeat
   its class's name: `Vector::Normalize`, not `Vector::NormalizeVector` (GSFC-2.4). Is/Has/Can,
   Get/Set, Initialize and Compute prefixes are suggested.
3. Parameters are named like variables (GSFC-2.5; snake_case, section 0.3).
4. No global variables and no extern variables (GSFC-2.7.2, 10.1, = P10-6); file-local state
   lives in an anonymous namespace.
5. Constants are UPPER_SNAKE_CASE, declared `constexpr`/`const` or as enumerators, never as
   `#define` constants (GSFC-2.10). Exceptions: `Resource.h`, which the resource editor writes and
   the resource compiler reads, and configuration macros the Windows headers read
   (`WIN32_LEAN_AND_MEAN`; `NOMINMAX` is set in the project files).
6. A struct holds only public data; a type with private data is a class (GSFC-2.11).
7. Files a tool generates keep the names and the format the tool gives them: `Resource.h`, the
   .rc file, the project files (GSFC-2.14).

Formatting and statements
8. Blank lines group statements into paragraphs; a complex statement becomes several simple ones
   (GSFC-3, 3.6). One declaration per line (GSFC-3.1, = P10-4).
9. Spaces (GSFC-3.3, 3.5): between a control keyword and its parenthesis (`if (`, `for (`,
   `while (`, `switch (`, `catch (`); none between a function's name and its parenthesis, which
   includes `sizeof(` and casts; one after each comma and each semicolon of a `for` header; around
   assignment operators and around the `?` and `:` of the conditional operator; none between a
   unary operator and its operand; none around `->`, `.` and `[]`.
10. Indentation is four spaces (GSFC-3.4 allows three or four); no tabs (GSFC-3.5).
11. Arguments that don't fit on one line continue aligned with the first argument; when that
    still doesn't fit, under the function's name (GSFC-3.7).
12. An `if` inside another `if` without braces between them is not allowed: nested ifs always
    take braces (GSFC-3.8). Otherwise a single statement may go without braces.
13. A value that isn't a boolean is compared explicitly: `pointer != nullptr`, `handle != NULL`,
    `count != 0`, `(flags & BIT) == 0`, `std::iswspace(c) != 0` (C predicates return int), never
    tested through an implicit conversion (GSFC-3.8). Booleans, including Win32 `BOOL`, are tested
    directly, as P10-5's `if (!c_assert(...))` does.
14. Every switch has a default case, and it comes last (GSFC-3.9). A default that can't be reached
    fails a `c_assert` and takes the recovery, as GSFC's "fatal error exit". A case with statements
    that falls through into the next says so in a comment; variables declared in a case go in a
    block.
15. The conditional operator only for simple choices, never nested (GSFC-3.13).
16. Return an expression or a constructed value directly, not a local made only to be returned,
    unless it is an explanatory variable (CC G19) or checked before the return (GSFC-3.14, 9).
    `break` may leave a loop at a logical point; several returns are fine (GSFC-3.11, 3.14).
    No `goto` (GSFC-3.12, = P10-1).

Comments
17. Comments use `//` only; no pseudo-code (PDL); no rows of asterisks (GSFC-4.5, 7).
18. Code that implements a specification or a published algorithm names its source: here the web
    version's CSS rule or script function it reproduces (`// .log { border: 6px }`) (GSFC-4.5).

Classes
19. No public data members in classes (GSFC-5.1); structs are data structures (CC 4.6).
20. A class that can be derived from has a virtual destructor; every other class is `final` (I1);
    a method is virtual only where it is overridden (GSFC-5.1).
21. A declaration keeps its return type on the function name's line (GSFC-5.1).
22. Class sections come in the order public, protected, private (GSFC-5.1.2).
23. No function bodies inside class definitions; a header that must define a function does it
    outside the class (GSFC-5.1.4).
24. Constructors initialize the members and do nothing that can fail; work that can fail goes into
    a function that reports failure, which the caller checks (GSFC-5.2.1). Every member is
    initialized at construction (a default member initializer or the initializer list, in
    declaration order) (GSFC-5.2.1, 5.2.3).
25. Exceptions are caught by reference (GSFC-5.2.2), and only at boundaries (section 1 row 1).
26. Every variable is initialized where it is declared, structs with `= {}` (GSFC-5.2.3).
27. A definition's parameter names match its declaration's exactly (GSFC-5.2.3).

Includes and files
28. Every header guards against being included twice, with `#pragma once` (GSFC-5.1).
29. Includes come at the top of a file only, grouped and sorted (GSFC-5.1.3): the file's own
    header (or `pch.h`) alone; then a header that must be read before the others, alone
    (`targetver.h` before `<windows.h>`, `framework.h` before the SDK's headers); then system
    headers, lower levels first: the Windows SDK's, then the standard library's, each alphabetical;
    then the project's headers, alphabetical (section 0.3); in tests, the included sources last.
30. C++ headers instead of C headers: `<cstdlib>`, not `<stdlib.h>` (GSFC-5.1.3, 10.7). A file
    includes only what it uses (GSFC-5.1.3).
31. No directory names or absolute paths in include directives; the build sets the include
    paths (GSFC-5.1.3). Third-party headers keep the form their library documents
    (`"gtest/gtest.h"`).
32. The main procedure (`wWinMain`) lives in a file of its own (GSFC-7). A file that defines the
    members of one class is named after it (GSFC-2.13): `CivPickerView*.cpp`.
33. The project file lists every source file (GSFC-10.6; MSBuild's project file is the makefile),
    and the program links only the libraries it uses (GSFC-10.7).
34. The README says what the program does and how its code is organized (GSFC-10.5).

Portability, efficiency and libraries
35. ISO C++ wherever it offers what an extension does (GSFC-8): libraries are linked through the
    project file, not `#pragma comment`; a COM interface's ID comes from the SDK's `IID_` constant
    where the SDK declares one, else `__uuidof` (Direct2D and DirectWrite declare none).
36. Class and struct instances larger than 16 bytes, or with a non-trivial copy, are passed by
    const reference when the function doesn't change them (GSFC-8, 9). Smaller trivially copyable
    structs (`PixelRect`, `FloatRect`) may go by value, which costs less.
37. Embedded assignments (`while ((c = Next()) != END)`) only where proven faster (GSFC-9).
38. No using-directive at global scope in a header; in a source file, `using` for heavy use of a
    namespace, the scope operator for a few uses (GSFC-2.2, 10.8).
39. Standard library components instead of hand-written equivalents (GSFC-10.9), within P10-3
    (nothing that allocates after startup) and P10-2 (a loop over a run-time count keeps its
    written maximum, so hand-written bounded loops stay).

### 14.3 Left out, and why

| GSFC | Rule | Conflicts with |
|------|------|----------------|
| 2, 2.5, 2.7 | camelCase variables and parameters, no underscores | 0.3, naming case (snake_case) |
| 2.9 | enumerators in capitals (`DARK_BLUE`) | 0.3, naming case (`ImageId::civ_atlas`) |
| 2.1 | GUI classes named after their parent (`DisplayPanel : Panel`) | CC 4.2: no type encodings |
| 2.1, 5.2.2, 10.10 | exception class names, exception use, catching `bad_alloc` | Section 1 row 1, P10-3 |
| 2.6 | project name as a namespace prefix (suggested) | CC 4.2: no gratuitous context |
| 2.7, 4.5 | comments on unclear declarations; units in comments | CC 4.2: rename, units in names |
| 2.7 | variables used throughout declared at the top | CC 4.5, G10: just above first use |
| 2.7, 8 | project typedefs for built-in types (`Integer`) | CC 4.11: standards with value only |
| 2.7.1, 2.7.2 | `*` and `&` with the name (`T *p`) | 0.3 (`T* p`, `T& r` throughout) |
| 2.7.1 | `NULL` for a null pointer | 0.3, GSFC-8 (`nullptr`; `NULL` for handles) |
| 2.11 | struct names ending in `Type` | CC 4.2: no encodings |
| 2.12, 10.3 | C function names, mixing C and C++ | Not applicable: no C code |
| 2.13, 7 | `.hpp`; one class per file pair; constants in .cpp | 0.3, file layout |
| 3.2 | first brace on the next line for every block | 0.3, brace style (one-line bodies) |
| 3.4, 4.5 | variables and trailing comments lined up | CC 4.5: no alignment in columns |
| 3.8 | explicit comparisons of booleans (`!= true`) | P10-5's `if (!c_assert(...))` |
| 3.9 | a comment on stacked case labels | CC 4.4: noise |
| 3.13 | the condition of `?:` in parentheses | 0.3 (211 of 216 without) |
| 3.15 | 80 characters per line | CC 4.5: 120 |
| 4, 4.1-4.4, App. B | file and method prologs, Doxygen | CC 4.4: mandated comments, bylines |
| 4.5 | a block comment before each major section | CC 4.3: sections mean several things |
| 5.1 | Get and Set methods for data | CC 4.6: no blind accessors |
| 5.1 | include guard macros named after the class | P10-8: `#pragma once`, no `#ifndef` |
| 5.1.1 | default and copy constructors, destructor, assignment in every class | CC G12: clutter |
| 5.1.3 | a comment on each system include saying why | CC 4.4: mandated comments |
| 5.1.5, 5.2.4, 10.4 | file layouts with banners and CVS keywords | 0.3, CC 4.4 banners, git |
| 5.2.3 | definitions in declaration order | CC 4.3, 4.5: stepdown |
| 6 | `template<...>` alone on its line, upper-case parameters | 0.3 (every template) |
| 6 | template definitions behind conditional compilation | P10-8 |
| 8 | modifiable arguments by reference | CC 4.3 F2: no output arguments |
| 8 | optimizations considered from the start | CC 4.11: last responsible moment |
| 2.4, 2.5, 4.5 | names matching a formal specification | Not applicable |
| 10.2 | the `GMAT_API` define | Not applicable |

### 14.4 Interpretations

- GSFC-2.2 asks for namespaces against clashes between class libraries; the program is not a
  library, so its classes stay at global scope, as before.
- A single statement under an `if` or a loop is shown on its own line in GSFC's examples, but no
  rule says so; the project's same-line form (`if (!c_assert(x)) return false;`) stays.
- Win32 `BOOL` is a boolean by meaning, so rule 13 tests it directly (`if (!GetWindowRect(...))`).
  A Win32 function that returns other values in a `BOOL` (TrackPopupMenu with TPM_RETURNCMD) is
  compared explicitly.
- Rule 14's comment is for a case with statements; stacked labels (`case A: case B:`) share one
  body and don't fall through anything.
- Returning a struct by value is GSFC's own advice (construct on return, GSFC-3.14), so rule 36
  is about parameters only.
- GSFC-10.9's components are the STL's containers, iterators and algorithms. C++14 has no
  array-length function (`std::size` is C++17), so the `CountOf` helpers stay; hand-written
  minimums and maximums became `std::min` and `std::max`.
- The order of system headers was mixed (some files alphabetical, some SDK first), so GSFC's
  "lower levels first" decides it (rule 29); the project's headers were alphabetical throughout.
- Generated files (rule 7) keep their tool's format: `Resource.h` keeps its tabs and has no
  `#pragma once`.
- The test runner's `main` is a main procedure too (rule 32): `UnitTesting_FRCP/TestMain.cpp`,
  named apart from the program's `Main.cpp` so that their objects can't collide in a shared
  build folder.

14.5 (the compliance pass) is in CLAUDE2_LOG.md.

## 15. Files and folders (user rules, 2026-10-16)

The user's rules: **no file may be over 1000 lines, and no directory may contain over 20 files, unless it holds
assets such as the civ icons.** They bind like section 10's (0.5).

### 15.1 What the rules mean here

- The line limit holds for every file in the repository, code or not (the user's answer, 2026-10-16): sources,
  headers, project files, data, and these two rule files. Below it CC's sizes still apply: about 200 lines
  typical, 500 desirable (4.5).
- The folder limit counts the files directly in a folder, tracked or not; a subfolder doesn't count towards its
  parent. Exempt: asset folders (`images\...`, `sounds\...`: one file per civ, DLC, edition or picture, so they
  grow with data.json) and folders only builds and tools fill (`Debug`, `Release`, `x64`, `packages`, `.vs`).
- A file under 250 lines stays a file of its own (the user's decision, 2026-10-16). Merging small files would work
  against CC 4.5, and these rules keep particular ones apart in any case: GSFC 32 (`Main.cpp`, `TestMain.cpp`),
  GSFC 7 (`Resource.h`, the .rc file, and `targetver.h`, which the .rc file's generated TEXTINCLUDE includes),
  GSFC 30 with CC G13 (the constants headers serve different layers, and the model must not see the view's),
  section 6 (the model is plain C++ that the tests compile on their own) and GSFC 23 (a .cpp file can't fold into
  its header).
- A file that grows past the limit is split along its sections into files named after its class (GSFC 32:
  `CivPickerView*.cpp`, `CivPickerController*.cpp`), moving the code verbatim. A folder that grows past it is
  divided by what its files depend on, as `parchment\` is.

### 15.2 The program's folders

`Fresh-Random-Civ-Picker_CPPGUI\` keeps the project and resource files, `Main.cpp`, the headers the .rc file
includes (`Resource.h`, `targetver.h`) with `framework.h`, data.json, these rules and the asset folders. The code
is in folders by its role in MVC (section 6), the Parchment components by what they build on:

| Folder | What it holds | Includes from |
|--------|---------------|---------------|
| `common\` | `Assertions`, `CivDataLimits.h`, `GlobalConstants.h`: what every part uses | the root |
| `model\` | `CivCatalog`, `JsonReader`, `CivPickerModel`, `ShareLink`: plain C++, unit-tested | common |
| `controller\` | `CivPickerController*.cpp` | model, view, common |
| `view\` | `CivPickerView*.cpp`, `CivPickerLayout.h`, `DialogMetrics.h` | model, controller, parchment, common |
| `parchment\graphics\` | the renderer, images, fonts, layout arithmetic, theme, `PixelSurface` | common |
| `parchment\controls\` | the control base, every control, the logs and the scrollbar | graphics, common |
| `parchment\windows\` | window shapes, dialogs, the menu bar, tooltip and toast | controls, graphics, common |

- Includes name files only (GSFC 31). Each folder is an include directory of the program's project file in all
  four configurations; the test project lists the folders whose sources it compiles (`common`, `model`,
  `parchment\graphics`). A new folder goes into both project files, and into the `.vcxproj.filters` file, whose
  filters mirror the folders.
- Nothing in `model\` or `common\` includes from the view, the controller or Parchment, and no Parchment folder
  includes from one that builds on it.
