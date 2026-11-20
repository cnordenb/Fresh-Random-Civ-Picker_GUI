---
name: terminology
description: >-
  Add, change and order terms in documentation/TERMINOLOGY.md, the project's glossary. Use this skill when a change or
  a discussion gives a name to something in the project (a part of the interface, a concept in the data, a kind of
  file or test); when a term comes to mean something else; when two words are being used for the same thing; and
  before writing questions, tests, log sections or names in the code that need a precise word.
---

# TERMINOLOGY.md

`documentation/TERMINOLOGY.md` gives each thing in the project one name, so that the requirements, the tests, the log,
the guide and the code all mean the same by it (Rule 9).

## Its form

```
term - what it is, in a phrase, with an example where it helps, e.g. "Expand all"
```

- One term a line, at most 120 characters, no heading and no blank lines.
- In lower case, except where it quotes the project's own text, such as a button's label, or names a file.
- Each line uses only terms defined above it. A new term goes below every term its line uses, and above every term
  that will use it. If two terms need each other, define the plainer one first without the other.
- Say what the thing is, not how it came about.

## When to change it

- **A new thing gets a name.** Add the term, then use exactly that word everywhere: in the code, the tests, the guide
  and the log.
- **Two words for one thing.** Choose one, ask under Questions if the choice is not plain, define it, and change the
  other word in the documents and the code, but not in old log sections.
- **A term changes meaning.** Change its line, and check every place that uses it.
- **The requirements.** `REQUIREMENTS.md` is the chief developer's text (Rule 2): the terms follow its words, never the
  other way round. When a requirement uses a word that is not defined, define it as the requirement uses it, and ask if
  that use is unclear.

## Check

Every line within 120 characters:

```
awk 'length($0) > 120 { print FILENAME ":" NR ": " length($0) }' documentation/TERMINOLOGY.md
```

Then read each line and check that every term in it is defined above. Name the terms added or changed in the log
section's **Documents.** point.
