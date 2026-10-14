# Code review (module)

The core's `../../.agents/docs/code-review.md` applies as written: findings only, verify every
claim, walk every discussion item, review codestyle on every changed line. The module docs
(`cpp-guidelines.md`, `cpp-playerbots.md`, `ai-engine.md`, `sql-guidelines.md`,
`pull-requests.md`) apply as a checklist to every changed line. The sections below are what the
module adds.

## Feature analysis

Features are welcome, and making bots behave more like players in a cheap and scalable way is desirable.
Any new feature needs maintainer approval and a clear understanding of runtime cost.

- The PR names the player-visible problem: what a bot does today, or fails to do, that a player
  or an admin notices.
- It fits the engine's shape: strategy, trigger, action, value or multiplier, with no
  special-casing of shared code.
- The minimal implementation for the feature implemented.
- Run cost is known: what now runs per bot per tick, and what gates it.
- No existing mechanism already covers it, orphaned ones included. Grep `creators["..."]` for
  names in the same area, then grep for their consumers; zero consumers is an orphan the PR must
  wire up, remove, or explain. Check the base classes the new code inherits, too.
- Built at the right layer: a per-class rotation fix belongs in that class's directory, an
  encounter fix in `Dungeon/` or `Raid/`, shared behaviour in `Base/`.
- It does not defeat an exclusive mechanism (sibling strategy groups, `ApplyInstanceStrategies`).
- Opt-in is not validation: a config knob does not excuse a design that is wrong when enabled.
- One PR, one feature.

## Design quality

- Shared code does not branch on strategy names.
- One action, one job; no fallback cascades inside `Execute()`.
- Expensive work sits two gates. `isUseful()` for the first and cheapest check, followed by
  `isPossible()`, which can be more expensive. Cached values can be used to minimize per tick costs. Code found in isUseful and in isPossible should not also be found in the action.
- Relevance values are a named band plus a small offset, not bare floats.

## Wiring and lifetime

- Every new name is registered and every consumer resolves (`ai-engine.md` checklist).
- `InitTriggers()` chains the parent unless the override is deliberate.
- Multipliers return 0 only for the actions that should not be taken.
- No raw `new` without an owner; pointers should not be held across ticks.
- No synchronous database queries on the map thread; multi-statement writes use a transaction.
- Shared state reachable from several map threads is protected.

## Logic and priority

- State transitions only in `DoNextAction()`; default-action lists end in a terminal fallback; no
  trigger pair that oscillates every tick (aspect swaps, movement re-issues).
- Default actions stay in the default band, interrupts in `ACTION_INTERRUPT`, boss mechanics in
  `ACTION_RAID`, survival in `ACTION_EMERGENCY`.
- Value `checkInterval` matches how often the answer can change.

## Dead code and impact

- New code introduces no dead code and removes the orphaned registrations it makes obsolete.
- The impact assessment is checked, not accepted: a `pmon` before/after for anything that adds
  per-tick work, and a review of shared-code changes as if the gated feature were off.
- In-game testing is the author's; ask what was tested, record it, and name the branches the test
  did not take.
