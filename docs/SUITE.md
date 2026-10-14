# Sharing with Callie

Jot copies its tooling and much of its code from [Callie](https://github.com/callieapp/callie)
rather than sharing it. When a third app arrives, the shared parts will move into one place that
all the apps use. These rules keep that move mechanical.

## Rules

- A shared file stays identical to Callie's copy. When one changes, make the same change in the
  other, in the same pull request series, or note why the two differ in the table below.
- App-specific values live in one marked place per file: the block above the "Shared by every app
  in the suite" line in the Makefile, and the `scopes` list at the top of
  `commitlint.config.mjs`.
- Shared code names nothing app-specific in comments or messages. Say "the app", not "Jot".
- Copied C++ keeps its structure and changes only the namespace (`callie` to `jot`), the
  include prefix (`callie/` to `jot/`) and the CMake prefix (`CALLIE_` to `JOT_`), so a diff
  between the two copies shows only real changes.

## Shared files

| File                                                                          | State                                                                                   |
| ----------------------------------------------------------------------------- | --------------------------------------------------------------------------------------- |
| `.clang-format`, `.qmlformat.ini`, `.qmllint.ini`, `.editorconfig`, `.clangd` | Identical                                                                               |
| `.prettierrc`, `.prettierignore`, `.markdownlint-cli2.yaml`                   | Identical                                                                               |
| `.githooks/commit-msg`, `scripts/format.sh`                                   | Identical                                                                               |
| `cliff.toml`                                                                  | Identical apart from "the app" in one comment                                           |
| `package.json`, `package-lock.json`                                           | Identical apart from the name and description                                           |
| `commitlint.config.mjs`                                                       | Shared rules, app scopes at the top                                                     |
| `Makefile`                                                                    | App block at the top; gallery, watch, screenshots and logs targets come with their code |
| `CMakePresets.json`                                                           | Same presets, `JOT_` cache variables                                                    |
| `.github/workflows/ci.yml`                                                    | Same jobs, Jot's dependencies, Ubuntu added                                             |
| `.github/FUNDING.yml`, `.github/ISSUE_TEMPLATE/config.yml`                    | Identical                                                                               |
| `.github/ISSUE_TEMPLATE/bug_report.yml`, `SECURITY.md`                        | Same text, Jot's names and scope                                                        |
| `.github/workflows/code-review.yml`                                           | Identical                                                                               |
| `.github/review-bot/REPO_NOTES.md`                                            | Same structure, Jot's stack and risks                                                   |

Callie's `commitlint.config.mjs`, `Makefile` and `cliff.toml` still use the older layout. Moving
them to this one is the first step of extraction.

Repository settings match Callie's too: rebase merges only, auto-merge, branches deleted on
merge, the "Protect main branch" ruleset (with `build (ubuntu)` added to its required checks),
labels, Dependabot security updates and private vulnerability reporting.
