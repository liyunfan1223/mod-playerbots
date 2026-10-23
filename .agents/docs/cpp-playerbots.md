# C++ in mod-playerbots

Module specialization of `cpp-guidelines.md`, which applies in full. Read both for any C++ change.

The linter runs from the module root: `python apps/codestyle/codestyle-cpp.py`. Format touched files
with `clang-format` (`.clang-format` in the module root); `code_format.sh` runs it over the tree.

## Naming and layout

- Methods and public/protected members `UpperCamelCase`; private members `_lowerCamelCase`;
  locals and parameters `lowerCamelCase`. No Hungarian notation, no abbreviated names.
- Float literals always suffixed: `1.0f`. One pointer per declaration: `Player* player`.
  `static` first in a declaration.
- No magic numbers: `constexpr` or `enum` / `enum class`, never `#define`. Standard prefixes
  `SPELL_`, `NPC_`, `ITEM_`, `GO_`, `QUEST_`, `SAY_`, `EVENT_`, `DATA_`, `ACHIEV_`.
- Never redefine `me`; the core reserves it for the scripted object pointer.
- New source files carry the mod-playerbots GPLv2 header (copy it from any file in `src/`). Ported
  files keep their upstream notice and add an in-file "Ported/adapted from <project>" note.

## Module APIs

- Logging category is `playerbots`: `LOG_INFO("playerbots", "msg {}", arg)` and siblings.
- Config: every option is declared in `conf/playerbots.conf.dist` with a default and a comment,
  read once at load into `sPlayerbotAIConfig`, and read from there. Never call `sConfigMgr` in bot
  code.
- Bot chat goes through `GetBotTextOrDefault` with an English fallback; the translations ship as
  a SQL update (see `sql-guidelines.md`).
- Overload signatures are checked before copying a nearby call: `PlayerbotAI::CanCastSpell` has
  string, spell-id, GameObject, and coordinate forms, and they do not behave alike.

## Map-thread safety

Bot AI runs on map threads, in parallel per map. A crash there takes the whole map down.

- No synchronous database queries in actions, triggers, values, strategies, or `PlayerbotAI`:
  no `*Database.Query` / `PQuery`; they block every bot on that map. Factory and login paths on
  the world thread are the exception.
- Anything not thread-safe (group invites, guild operations, login finalisation) is queued through
  `PlayerbotWorldThreadProcessor` and runs on the world thread.
- The core's long-lived reference rule is per tick here: never hold a raw `Player*` / `Unit*` /
  `Creature*` across ticks; store the `ObjectGuid` and resolve it at use time.
- Every lookup by GUID, target, pet, or map is null-checked before use, with an early exit.
- An override that replaces a base `Execute()` / `IsActive()` either calls the base or keeps its
  guards and side effects.
- Shared state touched from more than one map thread is protected or made thread-local.

## Cost discipline

Every action, trigger, and value runs per bot, per tick, across thousands of bots.

- Triggers are O(1): no scans of the quest log, inventory, nearby units, or group members inside a
  trigger without a cached value behind it.
- Actions gate expensive work behind `isUseful()` / `isPossible()`; `Execute()` never runs a full
  search just to return `false`.
- Loop nesting follows the question being asked; iterating the wrong dimension outermost is the
  usual cause of O(n³) scans. Three levels deep is a restructure signal.
- Expensive or behaviour-changing features are opt-in via `playerbots.conf.dist`, off by default.
