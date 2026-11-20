@AGENTS.md

@documentation/RULES.md

## Claude Code

- The procedures named in `AGENTS.md` are skills; use the skill rather than working from memory.
- `.claude/settings.json` denies `git commit`, `git push` and the like, and every edit of `REQUIREMENTS.md`. A denial is
  the rule working: say what a human needs to do, and do not look for another way round it.
- Take the date and time for the log from the shell, never from memory: `date '+%Y-%m-%d %H:%M:%S'` in Bash or
  `Get-Date -Format 'yyyy-MM-dd HH:mm:ss'` in PowerShell.
- Credit for the work goes in the Acknowledgements of `README.md`, as `CONTRIBUTING.md` says, not in commits.
