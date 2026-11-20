# DECISIONS.md

`documentation/DECISIONS.md` lists every decision made in the log, by subject, so that nobody has to read the whole
log to learn what holds now (Rule 23).

## Its form

- A list of its subjects at the top, each a link to its heading, then one paragraph saying what the file is.
- One `##` heading per subject. The template starts with: Rules, and how they are read; Requirements, and how they are
  read; Code and files; Tests; Documents. Add a subject when a group of decisions fits none of them, and add it to the
  list at the top; leave a subject empty rather than force a decision into it.
- One point per decision, starting with a few words in bold that end in a full stop, then what was decided in one or
  two sentences, and last the link to the section that decided it:

  ```markdown
  - **Settings are saved by the model only.** `settings.py` writes `settings.json`; no other script does.
    [2026-11-02 14:07:50](DEVLOG/DEVLOG1.md#140750-settings-saved-by-the-model-only)
  ```

  The link's text is the day, and the time when the section has one. A decision taken up again later has one link
  per section, oldest first.
- It says what holds now, not the story of how it came about; that is in the log.

## When a decision replaces another

Rewrite the old decision's point into the new decision, and name the old one after it with "Replaced:", keeping both
links:

```markdown
- **English by default** (Requirement 10), unless the address or an earlier choice says otherwise. Replaced: the
  browser's language on a first visit. [2026-10-18](DEVLOG/DEVLOG1.md#translations)
  [2026-10-22](DEVLOG/DEVLOG3.md#requirements-read-again)
```

The section that made the new decision names what it replaced in its **Replaced.** point.

## Links to a section

GitHub turns a heading into its link like this: everything in lower case; every character dropped but letters,
digits, spaces, hyphens and underscores; every space a hyphen. A heading that occurs more than once in a file gets
`-1`, `-2` and so on after its second and later copies, which is why sections carry their time.

| Heading | Link |
| --- | --- |
| `## 2026-10-31` | `#2026-10-31` |
| `### 18:06:02: the tests on GitHub run in real time` | `#180602-the-tests-on-github-run-in-real-time` |
| ``### `code_docu.pdf` made again`` | `#code_docupdf-made-again` |
| `### Two style sheets in css/, and the game's scroll bars` | `#two-style-sheets-in-css-and-the-games-scroll-bars` |

From `DECISIONS.md` a link goes to `DEVLOG/DEVLOG<n>.md#<anchor>`. Never link to a `#### What changed` or another
heading inside a section: they repeat in every section.
