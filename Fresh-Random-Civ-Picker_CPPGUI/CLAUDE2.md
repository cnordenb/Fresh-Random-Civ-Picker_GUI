# CLAUDE2.md - Binding development rules for this project

Every change to this project must follow these rules strictly. Compiled 2026-10-13 from three
documents in `Fresh-Random-Civ-Picker_CPPGUI/`:

- **P10**: `P10.pdf`, G. J. Holzmann, *The Power of Ten: Rules for Developing Safety Critical
  Code* (NASA/JPL). Read in full.
- **CC**: `cc.pdf`, R. C. Martin, *Clean Code*. Read: ch. 1-13 and 17 in full, plus the general
  parts of ch. 11, 14-16 and Appendix A. Skipped as Java-specific: EJB/AOP/proxies (ch. 11),
  heuristics J1/J2, the Java listings in the case studies and Appendix B.
- **HF**: `hfdp.pdf`, Freeman & Robson, *Head First Design Patterns* (2nd ed.). Read: every design
  principle (ch. 1-11), the MVC part of ch. 12, all of ch. 13.

Source tags: `P10-n` = P10 rule n. `CCn` = CC chapter n. `CC-G5`, `CC-N3`, `CC-T1`, `CC-C5`,
`CC-F2`, `CC-E1` = CC ch. 17 heuristics. `HFn` = HF chapter n.

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
5. The user's own rules for this project (section 10) bind like P10's and come before the books.

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
      theirs in code, resources, project files or tests (section 10).
- [ ] Boy Scout: the code I touched is a little cleaner than before.

## 8. Current state of the code (updated 2026-10-13, after the MVC split)

`FRCP_GUI.cpp`/`FRCP_GUI.h` were split into `CivPickerModel`, `CivPickerView` and
`CivPickerController` (.h/.cpp), following section 6. Verified: zero warnings at `/W4` and zero
findings under MSVC Code Analysis (`/analyze`); every function is under 60 lines; an automated
old-versus-new comparison (55 scripted steps, every control's text, visibility, check state and
position, plus the saved `log.txt` and `settings.ini`) found no differences; the unit test project
builds and passes.

Still open (next incremental steps):

- `Fresh-Random-Civ-Picker_CPPGUI.vcxproj` still has `WarningLevel` `Level3` and no code analysis
  configured, so P10-10 is met but not yet enforced by the build. `LanguageStandard` is not set.
- P10-5: no assertion macro yet. The model checks civ indices before using them, but the average
  of two assertions per function is not reached.
- P10-7: Win32 return values in the view and the controller are mostly neither checked nor
  `(void)`-cast.
- P10-3: `std::wstring` is used for run-time state, log text and labels, so the program still
  allocates on the heap after startup. The model's name index (`std::unordered_map`) is built once
  at startup and only read afterwards.
- `GlobalConstants.h` still uses `#define` constants (CC J3); it has an include guard since the
  data separation (section 10).
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
  lines), `CivPickerViewLayout.cpp` (the main window's controls and menus) and `CivPickerDialogs.cpp`
  (dialog layouts and procedures; 811 lines). These two, `ParchmentDialog.cpp` (626) and
  `ParchmentMenu.cpp` (512) are above CC's desirable 500 lines, as the controller and model already
  were; the next split would move the observer notifications out of `CivPickerView.cpp`.
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
  colour); the open dropdown list keeps the system's 1 px list border.

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
  CivPickerDialogs.cpp and CivPickerShare.cpp share), and a smooth or pixelated icon in
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
  scroll (`CivPickerDlcToggles.cpp`): how many show follows from its dlcToggleLayout (the rows
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
