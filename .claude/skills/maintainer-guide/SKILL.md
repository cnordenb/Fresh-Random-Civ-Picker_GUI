---
name: maintainer-guide
description: >-
  Keep documentation/MAINTAINER.md, the guide that teaches a human how to maintain the project, true, short and
  beginner-friendly. Use this skill whenever a change adds, removes, renames or moves a file or folder; changes how the
  code is organised, built, run, tested or released; changes the steps of the most common maintenance task; or would
  otherwise have needed a comment in the code to be understood. Also use it when asked to explain the code to a
  maintainer. Not for README.md, nor for the log.
---

# MAINTAINER.md

`documentation/MAINTAINER.md` teaches someone who did not write the project how to keep it working (Rule 7), with as
few steps as possible (Rule 3). Rule 10 forbids comments in the code, so what a comment would have said goes here.

## Its form (Rule 8)

1. The title, then straight under it a numbered table of contents with no heading of its own, linking every `##` and
   `###` heading.
2. **The first chapter is always the most common maintenance task**, step by step. Nothing ever goes before it, not
   even an introduction.
3. Then "How the code works", with these sections, in this order where they apply:
   - **Files and folders:** a table with every file and folder in the main folder, and the folders of code, each with
     one line on what it holds. Compare it with `git ls-files` after every change of files.
   - **Diagram:** once the code has more than a few parts, a mermaid diagram of them and how they call each other,
     drawn so that no arrow crosses another. Keep it in step with the code.
   - One section for each idea a maintainer must grasp, such as how the parts work together, or how the data is laid
     out, each with a small example.
   - **Tests:** how to run them, what each test file holds, and that `TESTS.md` lists them.
   - **Rules for changing the code:** the rules a maintainer needs while changing code, in short, pointing to
     `RULES.md` for the rest. Not a copy of `RULES.md`.

## How to write

- For a beginner: say what they need (programs, accounts, time) before the first step.
- Steps are numbered, each starting with what it does in bold: "1. **Run the updater.**" Give the exact command, the
  exact file and what they will see, and for each message they may see, what to do about it.
- Only what is needed to do the work. History belongs in the log, the reasons behind choices in `DECISIONS.md`.
- Use the terms of `TERMINOLOGY.md`, and the names the code uses, in backticks.
- Lines of at most 120 characters, except tables.

## After a change

1. Read the chapters the change touches, and make them true again. A step that no longer works is worse than none.
2. Keep the table of contents in step with the headings, and check that its links lead to them.
3. Name the guide and what changed in it in the log section's **Documents.** point.
