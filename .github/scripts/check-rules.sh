asset_folders="Fresh-Random-Civ-Picker_CPPGUI/images Fresh-Random-Civ-Picker_CPPGUI/sounds"
exempt_folders="documentation/DEVLOG $asset_folders"
code_pattern='\.(sh|ya?ml|cpp|h|py|bat)$'
not_code_pattern='(^|/)(Resource\.h|build\.bat|convert_web_assets\.py)$'

errors=0
template="${TEMPLATE:-false}"
awk_bin="$(command -v gawk || command -v awk)"

fail() {
  printf 'FAIL  %s\n' "$1"
  errors=$((errors + 1))
}

indent() {
  sed 's/^/      /'
}

joined() {
  printf '%s\n' "$1" | grep . | paste -sd ',' - | sed 's/,/, /g'
}

files() {
  git ls-files --cached --others --exclude-standard | sort -u | while IFS= read -r f; do
    if [ -e "$f" ]; then printf '%s\n' "$f"; fi
  done
}

check_placeholders() {
  local hits
  if [ "$template" = true ]; then
    printf 'skip  placeholders: this is the template itself\n'
    return 2
  fi
  hits="$(git grep --untracked -nE '\{\{[A-Z][A-Z0-9_]*\}\}|template:(start|end)' -- . \
    ':!.claude/skills/project-setup' ':!.github/scripts' ':!*/Resource.h' || true)"
  if printf '%s\n' "$hits" | grep -q 'template:start'; then
    fail "The project is not set up from the template yet; run /project-setup"
  elif [ -n "$hits" ]; then
    printf 'todo  placeholders, to fill in when known (.claude/skills/project-setup/SKILL.md, Later):\n'
    printf '%s\n' "$hits" | indent
    return 2
  elif [ -d .claude/skills/project-setup ]; then
    printf 'todo  no placeholder left: delete .claude/skills/project-setup/ and its row in AGENTS.md\n'
    return 2
  fi
}

check_folders() {
  local crowded
  crowded="$(printf '%s\n' "$all_files" | grep -vE "^(${exempt_folders// /|})/" | "$awk_bin" '
    { d = $0; if (sub(/\/[^\/]+$/, "", d) == 0) d = "."; n[d]++ }
    END { for (d in n) if (n[d] > (d == "." ? 8 : 7)) print (d == "." ? "the main folder" : d) ": " n[d] " files" }
  ' | sort)"
  [ -z "$crowded" ] && return 0
  fail "Folders with too many files, 7 at most and 8 in the main folder (Rule 14):"
  printf '%s\n' "$crowded" | indent
}

check_names() {
  local dupes
  dupes="$(printf '%s\n' "$all_files" | grep -vE "^(${asset_folders// /|})/" | sed 's#.*/##' \
    | grep -vx 'SKILL.md' | sort -f | uniq -di)"
  [ -z "$dupes" ] && return 0
  fail "File names used more than once, apart from SKILL.md and assets (Rule 15): $(joined "$dupes")"
}

check_devlog() {
  local dir=documentation/DEVLOG count n
  devlog_days=""
  count="$(printf '%s\n' "$all_files" | grep -cE "^$dir/DEVLOG[1-9][0-9]*\.md$" || true)"
  if [ "$count" -eq 0 ]; then
    fail "$dir/DEVLOG1.md is missing (Rule 16)"
    return 0
  fi
  for n in $(seq 1 "$count"); do
    if [ -f "$dir/DEVLOG$n.md" ]; then
      check_devlog_part "$dir/DEVLOG$n.md" "$n" "$count"
    else
      fail "$dir/DEVLOG$n.md is missing: the parts are numbered from 1, without gaps (Rule 18)"
    fi
  done
  check_devlog_days
}

check_devlog_part() {
  local part="$1" n="$2" last="$3" text lines days listed bad
  text="$(tr -d '\r' < "$part")"
  lines="$(wc -l < "$part" | tr -d ' ')"
  [ "$lines" -le 1000 ] || fail "$part has $lines lines, more than 1000 (Rule 18)"
  [ "$(printf '%s\n' "$text" | head -n 1)" = "# Development log, part $n" ] \
    || fail "$part does not start with \"# Development log, part $n\""
  days="$(printf '%s\n' "$text" | sed -n 's/^## //p')"
  bad="$(printf '%s\n' "$days" | grep -vxE '[0-9]{4}-[0-9]{2}-[0-9]{2}' | grep . || true)"
  [ -z "$bad" ] || fail "$part has entries not titled by their date (Rule 20): $(joined "$bad")"
  listed="$(printf '%s\n' "$text" | sed -n 's/^- \[\([0-9-]*\)\](#\1)$/\1/p')"
  [ "$days" = "$listed" ] || fail "$part: its contents do not list exactly its days, in order (Rule 19)"
  bad="$(printf '%s\n' "$text" | grep -E '^### [0-9]+:' \
    | grep -vE '^### ([01][0-9]|2[0-3]):[0-5][0-9]:[0-5][0-9]: ' || true)"
  [ -z "$bad" ] || fail "$part has times not written as hh:mm:ss: (Rule 21): $(joined "$bad")"
  if [ "$n" -gt 1 ] && ! printf '%s\n' "$text" | grep -qF "(DEVLOG$((n - 1)).md)"; then
    fail "$part does not link to the part before it, DEVLOG$((n - 1)).md"
  fi
  if [ "$n" -lt "$last" ] && ! printf '%s\n' "$text" | grep -qF "(DEVLOG$((n + 1)).md)"; then
    fail "$part does not link to the part after it, DEVLOG$((n + 1)).md"
  fi
  if [ -n "$days" ]; then devlog_days="$devlog_days$days"$'\n'; fi
}

check_devlog_days() {
  local days dupes
  days="$(printf '%s' "$devlog_days" | grep . || true)"
  [ -z "$days" ] && return 0
  dupes="$(printf '%s\n' "$days" | sort | uniq -d)"
  [ -z "$dupes" ] || fail "Days with more than one entry, or split between parts (Rules 20 and 22): $(joined "$dupes")"
  printf '%s\n' "$days" | sort -c 2> /dev/null || fail "The days of the development log are not in order"
}

check_tests_doc() {
  local doc=documentation/TESTS.md text headings listed expected="" count long
  if [ ! -f "$doc" ]; then
    fail "$doc is missing (Rule 6)"
    return 0
  fi
  text="$(tr -d '\r' < "$doc")"
  headings="$(printf '%s\n' "$text" | sed -n 's/^## Test \([0-9][0-9]*\)$/\1/p')"
  listed="$(printf '%s\n' "$text" | sed -n 's/^- \[Test \([0-9][0-9]*\)\](#test-\1): ..*$/\1/p')"
  count="$(printf '%s' "$headings" | grep -c . || true)"
  if [ "$count" -gt 0 ]; then expected="$(seq 1 "$count")"; fi
  [ "$headings" = "$expected" ] || fail "$doc: its tests are not numbered 1 to $count, in order"
  [ "$listed" = "$expected" ] || fail "$doc: its table of contents does not list Tests 1 to $count, in order"
  long="$(printf '%s\n' "$text" | "$awk_bin" '
    /^## / { field = 0 } /^### / { field = 1; next }
    field && length($0) > 120 { print "line " NR ": " length($0) " characters" }
  ')"
  [ -z "$long" ] && return 0
  fail "$doc has fields over 120 characters (Rule 6):"
  printf '%s\n' "$long" | indent
}

check_code() {
  local f long loc
  if [ -z "$code_pattern" ]; then
    printf 'skip  lengths of lines and files: code_pattern is not set in .github/scripts/check-rules.sh\n'
    return 2
  fi
  while IFS= read -r f; do
    long="$(tr -d '\r' < "$f" | "$awk_bin" 'length($0) > 120 { print NR }')"
    [ -z "$long" ] || fail "$f has lines over 120 characters (Rule 12): $(joined "$long")"
    loc="$(tr -d '\r' < "$f" | grep -cvE '^[[:space:]]*$' || true)"
    [ "$loc" -le 400 ] || fail "$f has $loc lines of code, more than 400 (Rule 13)"
  done < <(printf '%s\n' "$all_files" | grep -E "$code_pattern" | grep -vE "$not_code_pattern" || true)
}

run() {
  local before=$errors status
  "$2"
  status=$?
  if [ "$status" -ne 2 ] && [ "$errors" -eq "$before" ]; then printf 'ok    %s\n' "$1"; fi
}

cd "$(git rev-parse --show-toplevel)" || exit 1
all_files="$(files)"
run "placeholders filled in" check_placeholders
run "files per folder (Rule 14)" check_folders
run "unique file names (Rule 15)" check_names
run "development log (Rules 16 to 22)" check_devlog
run "TESTS.md (Rule 6)" check_tests_doc
run "lengths of lines and files (Rules 12 and 13)" check_code
if [ "$errors" -gt 0 ]; then
  printf '\n%s failed.\n' "$errors"
  exit 1
fi
printf '\nAll checks passed.\n'
