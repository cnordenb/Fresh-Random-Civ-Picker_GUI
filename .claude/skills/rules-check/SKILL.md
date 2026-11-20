---
name: rules-check
description: >-
  Check the whole project against documentation/RULES.md and documentation/REQUIREMENTS.md, mend what breaks them,
  and report every breach with its evidence. Use this skill when asked whether the project follows its rules or
  requirements, to check compliance or audit the project; after a large change, a refactor or a change of many files;
  after RULES.md or REQUIREMENTS.md has changed; and when a rule seems to contradict another rule or a requirement.
---

# Checking the rules and the requirements

## 1. What a script can check

```
bash .github/scripts/check-rules.sh
```

It checks Rule 14 (files per folder), Rule 15 (unique names), Rules 12 and 13 (lengths of lines and files, in the
files that `code_pattern` at its top matches), Rules 16 to 22 (the log's form) and Rule 6 (`TESTS.md`). Every
`FAIL` line names the file and the rule. A `skip` line means it could not check, and says why. A `todo` line lists
placeholders left from the template: not a breach; fill in those whose answers are known, as `project-setup` says.

## 2. What the tests check

Run the tests. For each rule and requirement that a test covers, note the test by its number in `TESTS.md`.

## 3. The rest, by reading

Go through `RULES.md` line by line, then `REQUIREMENTS.md` line by line. For each, decide how it can be shown to hold:
by the script, by a test, or by reading the code and the documents. Read what is needed, such as every function for
Rule 11 (60 lines at most) or every file of code for Rule 10 (no comments). Record each as holding, broken (where,
exactly), or not checkable, and why.

A requirement with no test is not a breach, but list it: it may need a test.

## 4. Mend, or ask

- Mend every breach that can be mended within the rules and the requirements. A mend is a change like any other: it
  has tests, documents and a log section.
- Never change `REQUIREMENTS.md` (Rule 2), and change `RULES.md` only when the chief developer asks. If a requirement
  cannot be met, or conflicts with a rule, ask under Questions with a proposal.
- When two rules, or a rule and a requirement, can be read in more than one way, take the reading that breaks the
  fewest, write it down under "How the rules were read", and record it in `DECISIONS.md` under "Rules, and how they are
  read" or "Requirements, and how they are read".

## 5. Report

In the log, a section titled for what it found, such as "compliance with RULES.md and REQUIREMENTS.md", with these
headings as they apply:

- **What changed: the code**, **What changed: requirements** and **What changed: documents**, each breach mended with
  the rule or requirement it broke;
- **Tests**, with the count;
- **How the rules were read**;
- **Not done**: breaches left, and what could not be checked;
- **Questions**.

In the reply, a table of every rule and requirement that does not plainly hold, with its state and the evidence: the
`FAIL` line, the test, or the file and line.
