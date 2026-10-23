# Commits and pull requests

## Branches

PRs target `test-staging`, never `master`. `master` only receives promotion PRs from
`test-staging`, opened by maintainers; `check_pr_source.yml` rejects anything else.

## Commit messages

`.git_commit_template.txt` is the format:

- Subject: `type(Scope/Subscope): short summary`, imperative, capitalised, no trailing period, max 72 characters. Types: `feat`, `fix`, `refactor`, `style`, `docs`, `test`, `chore`. Copy
  scope and subscope naming from recent commits touching the same files
  (`git log --oneline -- <paths>`).
- Blank line, then the why (what was wrong, what changes), wrapped at 72 columns.
- Links to issues, upstream commits, or references last.

## Upstream credit

mod-playerbots is GPLv2 and derived from the CMaNGOS playerbots (ike3). Code ported or adapted
from CMaNGOS, MaNGOS, or another GPLv2 project:

- keeps its upstream copyright notice; never replace it with the project header;
- gets a `Co-authored-by: Name <email>` trailer per original author on the commit (this module
  uses the trailer, not `--author` as the core does);
- carries an in-file "Ported/adapted from <project>" note on substantially ported files, or an
  inline `// Adapted from <project>` comment for small snippets;
- is declared in the PR's Code Provenance section with the source project, URL, and authors.

## PR body

Fill in `PULL_REQUEST_TEMPLATE.md` completely; do not delete sections. Strip only the
instructional `<!-- -->` comments.

- **Description**: the concrete, observable failure before and after. "Adds support for X" or
  "more realistic" is not a problem statement.
- **Feature Evaluation**: the minimum logic needed, and the processing cost when it runs across
  thousands of bots. Skippable only for trivial changes (typos, wrong IDs).
- **How to Test**: steps, required setup (bot count, config), expected behaviour.
- **Impact Assessment**: the three checkboxes, each explained when not "No". The standard
  measurement is `playerbot pmon` before and after: set `BotActiveAlone = 100` and
  `botActiveAloneSmartScale = 0`, boot, run `playerbot pmon toggle` once the bots have logged
  in, then `playerbot pmon stack` five minutes later and compare the last "Total" line.
- **AI Assistance**: tick honestly; name the tool, what it produced, and that it was reviewed.
- **Code Provenance**: per the upstream credit section above.
- **Final Checklist**: stability and performance tested, new bot lines translated, GPLv2 header on
  new files, docs and `playerbots.conf.dist` comments updated, no new compiler warnings.

## Scope

One PR, one feature. Global changes to shared code are split into their own PR and reviewed as if
any gated feature they support were off, because for most servers it will be. A change that needs
core edits gets a companion PR to `mod-playerbots/azerothcore-wotlk`, and the module PR links it.
