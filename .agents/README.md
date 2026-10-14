# .agents

Source of truth for this module's agent skills and docs, laid out like the AzerothCore core's
`.agents/` so one convention covers both trees:

- `skills/<name>/SKILL.md`
- `docs/` — task-scoped agent docs; taxonomy and placement policy in `docs/README.md`
- `plans/` — per-task planning docs, gitignored

Conventions live in `AGENTS.md` and `docs/`. Agent-specific files point there instead of restating
them, so there is one copy to keep current.

## Relationship to the core's `.agents/`

The module is always checked out at `modules/mod-playerbots/` inside an AzerothCore fork, so the
core's docs are reachable at `../../.agents/docs/`. The module tree is self-contained anyway:
`docs/cpp-guidelines.md` is a verbatim copy of the core's, and the other docs hold what is specific
to the module. Where module and core docs differ, the module docs win for paths under the module.

## Hooking up your agent

- **Claude Code** — reads `CLAUDE.md`, which imports `AGENTS.md`. It discovers skills under
  `.claude/skills/`, not `.agents/skills/`, so each skill ships a `.claude/skills/<name>/SKILL.md`
  stub carrying the frontmatter and one line pointing at `.agents/skills/<name>/SKILL.md`. The core
  uses a symlink for this; the module does not, because a Windows clone without
  `core.symlinks true` and Developer Mode checks the link out as a plain text file and the skill
  silently never loads.
- **GitHub Copilot** — reads `AGENTS.md` natively.
- **Any other agent** — point it at `AGENTS.md` through its own entry file, or tell it to read
  `AGENTS.md` first.

A new skill is therefore two files: the real one under `.agents/skills/<name>/`, and the stub.
