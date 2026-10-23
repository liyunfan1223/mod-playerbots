---
name: pr-title
description: Propose PR or commit titles for the current branch changes in mod-playerbots' type(Scope/Subscope) format. Titles only; the PR body is human-authored.
metadata:
  version: "1.0"
---

# PR title

Print three candidate titles for the current changes. Output text only — never create or edit
the PR, commit, push, or run any state-changing git command.

## Gather the changes

"The changes" is the union of committed branch work (`git diff origin/test-staging...HEAD`, or
`upstream/test-staging` when the checkout is a fork with an `upstream` remote), staged and
unstaged changes, and the branch's commit messages (context only). If all three are empty, say so
and stop. When the user names an existing PR, use `gh pr diff <N>` instead.

## Format

`type(Scope/Subscope): short summary`, per `.agents/docs/pull-requests.md`:

- imperative, capitalised, no trailing period, max 72 characters;
- type from `feat`, `fix`, `refactor`, `style`, `docs`, `test`, `chore`, chosen from what the diff
  does, not from what the branch is called;
- scope and subscope copied from recent commits touching the same paths
  (`git log --oneline -20 -- <paths>`); never invent a new scope when an existing one fits;
- the summary names the observable change, not the mechanism ("Stop healers pulling with
  Flash Heal", not "Add multiplier to HealStrategy").

## Output

Print exactly three lines, best first, each a complete title, followed by one line naming the
scope precedent used (`as in <hash> <title>`). Nothing else.
