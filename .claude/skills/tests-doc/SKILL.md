---
name: tests-doc
description: >-
  Keep documentation/TESTS.md in step with the tests: add, remove, merge, renumber and describe tests, and keep each
  test's file and lines right. Use this skill whenever a test is added, changed, moved, merged or removed; whenever
  code in a test file moves so that the lines of its tests change; when deciding whether a new test is worth writing;
  and when a test fails and its entry is needed to judge whether the failure is real or a false positive.
---

# TESTS.md

`documentation/TESTS.md` lists every test, why it exists and where it is, so that no test is written twice and a
failure can be judged quickly (Rule 6). It is a list, not a story: no history, no reasons beyond the purpose.

## An entry

```markdown
## Test 18

### File residence:
`tests/settings_test.py`, lines 40-52

### Purpose:
Enforce that only the model saves settings, so no change is lost (Req. 7).

### Assertion expectation:
No file but `settings.py` writes `settings.json`; the failure names the file that does.
```

- **File residence:** the test file, from the project's folder, and the lines of the test itself: from its first
  line to its last, as an editor numbers them.
- **Purpose:** why the test exists, starting "Verify that", "Prevent" or "Enforce", with the requirements or rules it
  serves, as "(Req. 3, 4)" or "Rule 13".
- **Assertion expectation:** what has to hold for the test to pass, concretely: values, counts, names.
- Each field is one line of at most 120 characters (Rule 6). `check-rules.sh` measures them.

## Numbers and order

- A test's number is its place in the order the tests run. Entries, and the table of contents, are in that order.
- The table of contents lists each test once, under its category, as `- [Test 18](#test-18): <its purpose in a few
  words>`. The categories are: functional completeness & correctness, accessibility, maintainability. Run the
  categories in that order too, so the numbers run through them. Remove the accessibility category from both the tests
  and `TESTS.md` if the project has no user interface.
- So that the report can say "Test 18 failed", give every test its number in its name or description, such as
  `test_18_only_the_model_saves_settings`, or have the test runner number the tests as they run.
- When a test is added, removed or moved, renumber every test after it: the headings, the table of contents, the
  numbers in the tests' names, and every mention of them in `MAINTAINER.md`, `DECISIONS.md` and the code. Leave the
  log alone; its sections describe the numbers as they were then.
- After a test file changes, check the lines of every test in it, not only of the test that changed.

## Which tests to write

- Tests should _sufficiently_ ensure that the project is complete, correct, maintainable and accessible; coverage is
  not a goal. Do not write a test that another test already makes (Rule 5).
- A test that fails without a real fault, a false positive, is mended or removed, not left to be ignored.
- Rules that `check-rules.sh` cannot check, such as Rule 10 (no comments) and Rule 11 (no function over 60 lines),
  belong in a maintainability test written in the project's own language.

## When a test is new or changed

1. Write the test, with its number.
2. Run it and see it pass. Then break the code it protects, on purpose, and see it fail with a message that names
   what is wrong. Undo the breakage. The log's **Checked.** point says that this was done.
3. Write or mend its entry, and renumber what follows.
4. Run all the tests, then `bash .github/scripts/check-rules.sh`, which checks that the tests are numbered 1, 2, 3 in
   order in both the headings and the table of contents, and the length of the fields.
5. Record it in the log: the count in **Tests.**, and `TESTS.md` in **Documents.**
