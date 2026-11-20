---
name: project-setup
description: >-
  Set a fresh copy of this repository template up as a new project in three steps: one question, fill in what is
  known, record it. Every other placeholder is left to be filled in later, when the work makes its answer known. Set
  up only when asked, as with /project-setup. Use it later whenever check-rules.sh lists placeholders under todo and
  the answer to one has become known, such as the stack and the test command when the first code is written.
---

# Setting up a project from the template

Three steps and one question. What cannot be known yet stays a placeholder, to be filled in down the line: see
[Later](#later). If `README.md` has no `<!-- template:start -->` line, the setup is done: go to Later. In the template
repository itself, stop: the template keeps its placeholders.

## 1. Ask one question

Read what can be known: the chief developer from `git config user.name`, the owner and the repository's name from
`git remote get-url origin`, the year from the shell, and the model you run on. Then ask once, showing what was found
so that it can be corrected: the project's name (propose the repository's), and in a sentence or two what it does and
for whom. Ask nothing else, and wait for the answer.

## 2. Fill in what is known

1. Replace `{{PROJECT_NAME}}`, `{{PROJECT_DESCRIPTION}}`, `{{GITHUB_OWNER}}`, `{{REPO_NAME}}`, `{{CHIEF_DEVELOPER}}`
   and `{{AI_MODEL}}` everywhere but in this skill, and any other placeholder the answer happened to settle.
2. In `LICENSE`, put this year and the chief developer's name.
3. Delete the README's note from `<!-- template:start -->` to `<!-- template:end -->`, both lines included.

Leave every other placeholder as it is. `REQUIREMENTS.md` has none and is not touched (Rule 2).

## 3. Record and hand over

1. Following the `devlog` skill, write the first section of the log, titled like "the project set up from the
   template": what the project is for, and under Not done, `REQUIREMENTS.md`, still to be written by the chief
   developer, and the placeholders left, by name.
2. Run `bash .github/scripts/check-rules.sh`. It passes, and lists the placeholders left under `todo`.
3. Tell the chief developer what is left for a human: writing `REQUIREMENTS.md`, and reading and committing the
   changes.

## Later

Fill in each placeholder, everywhere, as soon as the work makes its answer known, and name it in that change's log
section. When none is left, delete this skill's folder and its row in `AGENTS.md`; `check-rules.sh` says so.

- **When the first code is written:** `{{STACK}}` (language, version, main libraries, test tool), `{{TEST_COMMAND}}`
  and `{{LANGUAGE}}`, the README's badge, where `-` becomes `--`, `_` `__`, a space `_` and `#` `%23`, such as `C%23`.
  With them, set `TEST_COMMAND` in `.github/workflows/checks.yml`, adding a step before "Run the tests" that installs
  the toolchain if the stack needs one; add the code's file endings to `code_pattern` in
  `.github/scripts/check-rules.sh`, such as `'\.(sh|ya?ml|py)$'`; and add what the stack leaves behind to `.gitignore`.
- **When the code has taken shape:** `{{MAINTENANCE_TASK}}`, `{{MAINTENANCE_STEPS}}` and `{{CODE_OVERVIEW}}` in
  `MAINTAINER.md`, following the `maintainer-guide` skill. `{{MAINTENANCE_TASK_ANCHOR}}` is the link GitHub makes from
  the task's title: lower case, every character but letters, digits, spaces, hyphens and underscores left out, spaces
  as hyphens.
- **When there is something to use:** `{{USAGE}}` in the README.
- **Before the first release:** `{{SECURITY_CONTACT}}`, asked of the chief developer.

These are not placeholders, and are settled when they come up:

- **No user interface:** delete `.github/ACCESSIBILITY.md`, the accessibility category of `TESTS.md`, and the terms
  success criterion and human verification in `TERMINOLOGY.md`.
- **The chief developer's choices:** rules of the project's own, folders exempt from Rule 14 (then also in
  `exempt_folders` in `check-rules.sh`), funding in `.github/FUNDING.yml`, another licence, WCAG 2.2 AAA instead of
  AA. Propose them under Questions in the log when the work needs them.
- **On GitHub, by a human:** turning on Discussions, for the issue form's "Question" link.
