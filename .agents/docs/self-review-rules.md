# Self-review rules

Project-specific rules for
[/self-review](https://github.com/eai-org/agent-toolkit/blob/main/skills/self-review/SKILL.md),
run on a PR before it is submitted or updated. Generic improvements to the review process itself belong to the skill, not here.
The [code-review.md](code-review.md) rules apply on top, as do the core's
`../../.agents/docs/self-review-rules.md` rules on pinning the review to a commit.

## Regression risk

Automated test coverage has not been implemented, and every bot runs an instance of the ai
so one change reaches impacts every bot on the server.

- `src/Bot/`, `src/Ai/Base/`, and shared values: one change alters every class and every
  strategy — examine how the changed code is used elsewhere, not just the change itself.
- `src/Ai/Class/<Class>/`, `Dungeon/`, `Raid/`: impact is mostly contained to the class or
  encounter touched.
- SQL: watch for a `DELETE` or `UPDATE` whose `WHERE` catches rows it should not, and for a
  translation update that misses one of the nine locales.

## What the reviewer verifies itself

- Every new action, trigger, strategy, and value name resolves to a registered creator, and no
  registration is left orphaned (grep per `ai-engine.md`).
- New per-tick work is behind a gate or a cached value; a trigger that scans is a finding.
- No synchronous database query on a map-thread path.
- New `GetBotTextOrDefault` keys ship with their translation SQL.
- New config options are in `conf/playerbots.conf.dist` with a default and a comment.
- The PR body's Feature Evaluation and Impact Assessment match the diff: per-tick work added, a
  default strategy set or default config value changed, or a new decision branch is not ticked
  "No".
- Ported code carries its upstream notice and the PR's Code Provenance names the source; a
  missing credit is a finding.

## In-game testing

The author tests their change as reviewers cannot. Review for potential impact areas that
the author has not considered. Changes that can lead to increased per-tick work need to be
measured with `playerbot pmon stack` before and after.
