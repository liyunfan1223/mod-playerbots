#!/usr/bin/env python3
# This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
# information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
# or (at your option) any later version.

"""Integration-test run wrapper, sitting in an integration-tests/ folder beside
the worldserver binary.

    python run_test.py "self_test bot_smoke ready_check"   (or set SCENARIOS)

Machine-specific settings (test DB credentials) live in a `test_env` file next to
this script - copy `test_env.example` and adjust. Test-semantic bot config is
enforced by IntegrationTestMgr::OnWorldStartup.
"""

import os
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
BIN_DIR = HERE.parent
WORLDSERVER = "worldserver.exe" if os.name == "nt" else "worldserver"


def load_env(path):
    """Parse a KEY=VALUE file (blank lines and #-comments ignored)."""
    env = {}
    for line in path.read_text().splitlines():
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        key, _, value = line.partition("=")
        env[key.strip()] = value.strip()
    return env


def worldserver_running():
    if os.name == "nt":
        out = subprocess.run(["tasklist", "/FI", f"IMAGENAME eq {WORLDSERVER}"],
                             capture_output=True, text=True).stdout
        return WORLDSERVER.lower() in out.lower()
    return subprocess.run(["pgrep", "-x", WORLDSERVER], capture_output=True).returncode == 0


def main():
    env_file = HERE / "test_env"
    if not env_file.exists():
        sys.exit(f"ERROR: {env_file} not found - copy test_env.example and adjust it.")

    env = os.environ.copy()
    env.update(load_env(env_file))
    if not env.get("AC_LOGIN_DATABASE_INFO"):
        sys.exit("ERROR: test_env did not set AC_LOGIN_DATABASE_INFO.")

    binary = BIN_DIR / WORLDSERVER
    if not binary.exists():
        sys.exit(f"ERROR: {WORLDSERVER} not found in {BIN_DIR}.")

    # Two servers would share the test DBs and ports, contaminating both result sets.
    if worldserver_running():
        sys.exit("ERROR: a worldserver is already running - refusing to start a second one.")

    # Scenarios from the command line (or SCENARIOS), space- or comma-separated,
    # normalised to the CSV the config expects.
    scenarios = " ".join(sys.argv[1:]) or env.get("SCENARIOS", "")
    tokens = scenarios.replace(",", " ").split()
    if not tokens:
        sys.exit("No scenarios given (arg or SCENARIOS env).")
    env["AC_PLAYERBOTS_INTEGRATION_TEST_RUN"] = ",".join(tokens)

    # Disable the CLI console so an EOF on stdin can't shut the server down at boot.
    env["AC_CONSOLE_ENABLE"] = "0"

    # Run from the binary directory so the server finds its config and data.
    result = subprocess.run([str(binary)], cwd=str(BIN_DIR), env=env, stdin=subprocess.DEVNULL)
    print(f"WORLDSERVER_EXIT_CODE={result.returncode}")
    sys.exit(result.returncode)


if __name__ == "__main__":
    main()
