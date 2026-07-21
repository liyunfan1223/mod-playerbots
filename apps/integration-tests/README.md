# mod-playerbots integration tests

Compiled only with `-DPLAYERBOTS_INTEGRATION_TESTS=ON` (CMake). Stock worldserver is the
test runner; environment variables override any config key (module confs would override a
`-c` conf file, env vars beat everything).

## One-time setup

Create isolated test databases (auth/characters/playerbots; the world DB is shared with
dev and never written):

    CREATE DATABASE acore_test_auth;
    CREATE DATABASE acore_test_characters;
    CREATE DATABASE acore_test_playerbots;
    GRANT ALL PRIVILEGES ON acore_test_auth.* TO 'acore'@'localhost';
    GRANT ALL PRIVILEGES ON acore_test_characters.* TO 'acore'@'localhost';
    GRANT ALL PRIVILEGES ON acore_test_playerbots.* TO 'acore'@'localhost';

Fastest way to get a working bot pool: clone your dev databases into them once —

    mysqldump --single-transaction acore_auth       | mysql acore_test_auth
    mysqldump --single-transaction acore_characters | mysql acore_test_characters
    mysqldump --single-transaction acore_playerbots | mysql acore_test_playerbots

(Alternative: leave them empty — the DB updater creates the schema and the first boot
creates the bot account pool, which takes several minutes once.)

## Run

The build drops `run_test.py` and `test_env.example` into an `integration-tests/`
folder beside the built worldserver binary. There, once — copy `test_env.example` to
`test_env` and edit in the test DB credentials:

    cd bin/<config>/integration-tests
    cp test_env.example test_env        # Windows: copy test_env.example test_env

Then run one or more scenarios (space- or comma-separated):

    python run_test.py "self_test bot_smoke ready_check"

The wrapper loads `test_env`, points the server at the test databases, disables the
console, runs the scenarios from the binary directory and exits with the server's code
(`WORLDSERVER_EXIT_CODE=0` means all passed). Everything else (random bots off, full
activity, react delay, autogear, RPG status) is enforced in code by
`IntegrationTestMgr::OnWorldStartup`, so results never depend on local config.

Results: `test_results.json` next to the binary, plus a console summary.
Optional debug detail: set `AC_LOGGER_PLAYERBOTS=5,Playerbots` in `test_env` — level 5
is DEBUG (3 is WARN and shows nothing from `LOG_DEBUG`), and the `Playerbots` appender
writes `Playerbots.log` next to the binary.

## Scheduling

Scenarios in one run execute **in parallel**, each with its own raid. Repeating a name runs
it multiple times concurrently (results are labeled `name#1`, `name#2`, ...); batch
wall-clock is the slowest scenario. Mind the bot pool: concurrent raids draw disjoint
characters from the AddClass pool.

Instancing is per *map*, not per scenario, so **two copies of a continent scenario share the
world** and contend for one set of spawns — a contention test, not a repeat. The runner
refuses those: a duplicate name whose scenario reports `SharesWorldWithCopies()` (its map is
not instanceable) fails the run, and a `name*N` bench of one runs serially. **To repeat a
continent case, boot once per repetition.** Instanced scenarios are unaffected: each run
gets a real instance, so they still run `BENCH_CONCURRENCY` at a time.

Interactive use against a normal dev server: `.pbtest list`, `.pbtest run <name>`
(in-game GM chat or the worldserver console). No shutdown, results logged + JSON written.
