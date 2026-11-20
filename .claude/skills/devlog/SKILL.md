---
name: devlog
description: >-
  Write the development log in documentation/DEVLOG/ and keep documentation/DECISIONS.md in step with it: add a
  section for a change, record answers to earlier questions, list decisions, and start a new part before one passes
  1000 lines. Use this skill after every change to the project's files, however small, before reporting the work as
  done; when the chief developer answers a question from the log; when a decision is made or reversed; and when asked
  what was done last or which questions are still open.
---

# The development log

The log tells a reader who never saw the work what changed in the project, why, how it was checked and what is left.
It describes the project, never the people or agents who worked on it (Rule 17): the commits already say who did what.

## Where to write

1. Find the newest part: the `DEVLOG<n>.md` with the highest `n` in `documentation/DEVLOG/`.
2. Take the date and time from the shell, never from memory or the conversation:
   - Bash: `date '+%Y-%m-%d %H:%M:%S'`
   - PowerShell: `Get-Date -Format 'yyyy-MM-dd HH:mm:ss'`
3. Count the part's lines (`wc -l`). If the part with the new section would pass 1000 lines, follow
   [references/splitting.md](references/splitting.md) first (Rules 18 and 22).
4. If the part has no day for today yet, add `## YYYY-MM-DD` at its end, and `- [YYYY-MM-DD](#YYYY-MM-DD)` at the end of
   the list of days at the top of the part (Rules 19 and 20).
5. Add the section at the end of today's day.

## The time in the title

A section's title starts with the time it is written into the log, as `### hh:mm:ss: `, unless the section written
before it on the same day carries the same hour and minute; then its title has no time (Rule 21). The time makes the
title, and so its link, unique, which keeps `DECISIONS.md`'s links pointing at the right section.

## The form of a section

```markdown
### 14:07:50: settings saved by the model only

#### What changed

- **Why.** Two scripts saved the same setting, so a change made by one was lost when the other ran.
- **Saving.** `save()` in `settings.py` is now the only place that writes `settings.json`; `cli.py` calls it instead
  of writing the file itself. The program works as before.
- **Tests.** 31, up from 30: Test 18 checks that only `settings.py` writes `settings.json`.
- **Checked.** All 31 tests passed. With the old write put back into `cli.py`, Test 18 failed and named `cli.py`; the
  write was taken out again.
- **Documents.** `TESTS.md`: Test 18, and the numbers of Tests 19 to 31. `MAINTAINER.md`: the table of files.
  `DECISIONS.md`.

#### Not done

- The program was not tried by hand; only the tests ran.

#### Questions

1. **Old settings files.** Files written before this change have no `version`. Should `settings.py` add one when it
   reads such a file, or refuse it? Adding it would keep old files working.
```

- **The title** says what the project is like now, not what was done to it: "settings saved by the model only", not
  "refactoring the settings". After the time it starts in lower case.
- **What changed** is always there. Each point starts with a few words in bold that end in a full stop. Points that
  come up often, in this order where they fit:
  - **Why.** first, when the reason is not plain from the title.
  - one point for each thing changed, named as the code and documents name it;
  - **Tests.** how many there are now, and which were added, changed, merged, renumbered or removed;
  - **Checked.** how the change was shown to work, and the results. A test that is new or changed is best shown by
    breaking the code on purpose and seeing it fail; say so, and that the breakage was undone;
  - **Documents.** each document changed, and what in it;
  - **Answers.** answers to earlier questions, see below;
  - **Replaced.** earlier decisions this change replaces, each with a link to its section;
  - **Rules.** rules added, changed or lifted, by number.
- **Not done** lists what was left, what was not checked, and what is known to be wrong. Leave it out only when there
  is truly nothing to say.
- **Questions** comes last, only when there are questions.
- A large change may need more headings at the level of What changed. These have been used: "How the rules were read",
  "Decisions where rules met each other", "Obstacles met, and how they were solved".

## How to write

- Without people (Rule 17): "it was decided", "was found", "the tests were run". Never "I", "we", "you", "the chief
  developer asked", "Claude" or any other name. "The user" of the project and file names are fine.
- Plain words and short sentences, for a reader who has only the log. Use the terms of `TERMINOLOGY.md`.
- Concrete: names of files and functions in backticks, numbers, and what something was before and is now.
- No code copied into the log; name it and say where it is.
- Lines of at most 120 characters, a continued point indented by two spaces. Keep the spelling the log already uses.
- Never rewrite an earlier section. A correction is a new section that names the section it corrects. Only the list
  of days and the links that [splitting](references/splitting.md) moves are ever changed afterwards.

## Questions and answers

- Ask when a rule or requirement is unclear, when two of them conflict, and when a choice belongs to the chief
  developer. Meanwhile carry out the reading that breaks the least, and say which reading that is.
- Number the questions of each section from 1. Start each with its subject in bold, give the facts needed to decide,
  and a proposal.
- Ask them in the reply as well: the log is where they are kept, not where they are first read.
- When they are answered, the next section has an **Answers.** point, such as: "Of the questions of 14:07:50, in
  [settings saved by the model only](#140750-settings-saved-by-the-model-only): old files get a version (1)." The link
  goes to the section of the questions, in this part or another. Then record each answer that decides something in
  `DECISIONS.md`.
- A question is open until a later section answers it. To list the open questions, read the Questions of the latest
  sections and look for their answers after them.

## DECISIONS.md

Every section that decides something adds it to `documentation/DECISIONS.md` (Rule 23): a choice between two ways, a
reading of a rule or requirement, an answer to a question, a change of one of them. The form, the subjects and how to
write a link to a section are in [references/decision-list.md](references/decision-list.md).

## Before finishing

- The list of days at the top of the part lists every day in it, in order.
- Every new link leads to a heading that exists.
- `bash .github/scripts/check-rules.sh` passes its log checks.
