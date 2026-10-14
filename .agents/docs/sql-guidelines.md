# SQL guidelines (module)

This supersedes the core's `sql-guidelines.md` placement rules for everything under `data/sql/`
in this module: the module has no `pending_db_*` directories and no `create_sql.sh`.

## Where SQL goes

- `data/sql/playerbots/` — the module's own `acore_playerbots` database: `base/` (schema
  snapshot), `create/` (database creation), `archive/` (squashed updates), `updates/` (live),
  `custom/` (operator-local, not shipped).
- `data/sql/characters/` and `data/sql/world/` — updates the module applies to the core
  databases; `base/` and `updates/` each.
- New SQL is one dated file under the matching `updates/` directory, named
  `YYYY_MM_DD_NN_short_description.sql` (`NN` orders files from the same day). Copy the naming
  from the newest file in that directory.
- Never edit `base/`, `create/`, or `archive/`. Maintainers regenerate those snapshots, so edits
  there are lost and can break the updater.

## Conventions

- Every `INSERT` is preceded by a matching `DELETE` so the file is idempotent; no double
  semicolons, no multiple blank lines, InnoDB.
- One concern per file; a translation update ships in its own file next to the feature.
- There is no SQL linter in the module; review by eye against the rules above and the core's
  `codestyle-sql.py` conventions.

## Bot chat translations

Anything a bot says goes through `GetBotTextOrDefault` with an English fallback in code, plus an
SQL update under `data/sql/playerbots/updates/` covering the nine AzerothCore locales: English,
Korean, French, German, Chinese, Taiwanese, Spanish, Spanish Mexico, and Russian.

Example pair: `data/sql/playerbots/updates/2025_12_27_ai_playerbot_fishing_text.sql` and its
consumer `src/Ai/Base/Actions/FishingAction.cpp`.
