# Starting a new part of the log

A part holds at most 1000 lines (Rule 18), and a day is never split between two parts (Rule 22). So when a section
would take the newest part, `DEVLOG<n>.md`, past 1000 lines, part `n + 1` is started, and today's day goes into it
whole.

## Steps

1. **Measure.** Count the lines of `DEVLOG<n>.md` (`wc -l`) and of the section about to be written.
2. **Decide what moves.**
   - If `DEVLOG<n>.md` has no day for today yet, nothing moves: today's day starts `DEVLOG<n+1>.md`.
   - If it has, today's whole day moves, from its `## YYYY-MM-DD` line to the end of the file, with every section
     already in it.
   - If today's day alone, with the new section, would be over 1000 lines, Rules 18 and 22 cannot both be kept. Do not
     split the day. Write the section in the new part anyway, and ask under Questions how the chief developer wants it
     handled.
3. **Write the new part**, `documentation/DEVLOG/DEVLOG<n+1>.md`, starting:

   ```markdown
   # Development log, part <n+1>

   - [YYYY-MM-DD](#YYYY-MM-DD)

   Part <n+1> of the development log. It follows [part <n>](DEVLOG<n>.md). The decisions made in the log are listed in
   [DECISIONS.md](../DECISIONS.md).

   ## YYYY-MM-DD
   ```

   then the day that moves, unchanged, and the new section.
4. **Close the old part.**
   - Take the day that moved out of its list of days.
   - Its line under the list ends "It follows [part <n-1>](DEVLOG<n-1>.md) and goes on in [part <n+1>](DEVLOG<n+1>.md).",
     or, in part 1, "It goes on in [part 2](DEVLOG2.md)." Keep lines within 120 characters.
5. **Mend the links** to and from the day that moved:
   - In `DECISIONS.md` and the other documents, `DEVLOG<n>.md#<anchor>` of a moved section becomes
     `DEVLOG<n+1>.md#<anchor>`.
   - In the moved day, a link `(#<anchor>)` to a section that stayed in part `n` becomes `(DEVLOG<n>.md#<anchor>)`.
   - In part `n`, a link `(#<anchor>)` to a moved section becomes `(DEVLOG<n+1>.md#<anchor>)`.
   - Search for them with `grep -rn 'DEVLOG<n>.md#' documentation` and `grep -n '](#' documentation/DEVLOG/DEVLOG<n>.md`.
6. **Check.** `bash .github/scripts/check-rules.sh` checks the numbering of the parts, their first lines, their lists
   of days, the links between neighbouring parts and that no day is in two parts. Then record the new part in the
   section's **Documents.** point.

`documentation/DEVLOG/` is exempt from Rule 14, so it may hold as many parts as the log needs.
