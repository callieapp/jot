<!-- markdownlint-disable -->

# What Jot can reuse from Callie

Research snapshot from 2026-10-14, gathered to plan Jot. Prices and versions drift; check sources before relying on them.

## Callie survey for planning Jot (repo: /home/lara/Code/personal/callie)

There is no `.codegraph/` directory, so I used grep and read the files directly. The repo has 538 commits, origin `github.com/callieapp/callie`, version 0.4.1, MIT license.

### 1. Stack

- **Language and toolchain:** C++20, Qt **6.8 minimum** (Debian trixie sets that floor), CMake 3.28, Ninja. Warnings are `-Wall -Wextra -Wpedantic`, with `-Werror` under the CI preset.
- **Qt modules** (`/home/lara/Code/personal/callie/CMakeLists.txt`): Core, Gui, Qml, Quick, QuickControls2 (Basic style), Network, NetworkAuth, Sql, DBus, plus Test.
- **Dependencies** all come from distro packages through `find_package`. There is no FetchContent and no vcpkg. `make deps` installs them with dnf or apt.
  - Qt6Keychain
  - tomlplusplus (theme files)
  - KF6CalendarCore (iCalendar)
  - FTXUI 5 or later, optional, for the TUI (`CALLIE_TUI=ON|OFF|AUTO`)
- **Layers:**
  - `src/core/` builds the static `calliecore` library, headers under `callie/`, namespace `callie`. It deliberately has no QML, so the CLI starts fast.
  - `src/ui/` is the static QML module `Callie.Ui`: about 60 QML files plus C++ controllers. Core types are exposed with `QML_FOREIGN`.
  - `src/gui/main.cpp` builds `callie-gui`.
  - `src/cli/` builds a `calliecli` static library (so tests can call the commands) plus the `callie` executable.
  - `src/gallery/` builds `callie-gallery`, a theme and component viewer that is not installed.
  - Options: `CALLIE_DEVEL` gives a dev build its own app id, folders and keyring entries (`core/callie/Identity.h`). `CALLIE_LIVE_QML` adds `--live-qml`, which serves QML from `src/` and reloads on save.
- **Presets** (`CMakePresets.json`): `dev` (Debug, qmlls ini, live QML, devel identity), `release` (RelWithDebInfo), and `ci` (Werror, TUI ON). There is also a `ci-qmllint` build of `all_qmllint`, and a `ci` workflow preset that runs configure, build, qmllint and test.
- **Storage:**
  - Settings are INI at `$XDG_CONFIG_HOME/callie/settings.ini` (`Settings.h`, a QObject full of Q_PROPERTYs).
  - Accounts are `accounts.json` and hold no secrets.
  - The Google cache is SQLite through QtSql (`$XDG_CACHE_HOME/callie/google.sqlite`). It is treated as disposable: a schema change drops the tables.
  - Local calendars are vdir folders of `.ics` files under `$XDG_DATA_HOME/callie/calendars`.
  - The offline write queue is `pending.json`.
  - Logs go to `$XDG_STATE_HOME/callie/logs`.
- **Sync engine:**
  - `CalendarSource.h` is an abstract QObject. Reads are answered from local state ("Must not block on the network"), with an async `load()` returning a `QFuture<SourceSnapshot>`. `refresh()` emits `changed`, and `statusChanged` carries `syncReport`.
  - Sources are wrapped as decorators: `QueuedSource` (optimistic writes held 8s for undo, persisted, retried when back online, refused changes dropped), `CombinedSource` and `LookedSource`.
  - `Outcome.h` separates retryable failures from refused ones.
  - Threading is minimal: a `QThreadPool m_readers` in GoogleSource and VdirSource for disk reads, and `QFutureWatcher` on the consumers. Everything else is async signals and callbacks on the main thread.
- **Secrets:** `TokenStore` is an async interface; `KeychainTokenStore` uses QtKeychain (Secret Service) and "never falls back to plain text". `AccountManager` owns the ordering: the token goes into the keyring before the account is listed, and is removed before it is unlisted.
- **OAuth:** `GoogleAuth` uses QtNetworkAuth `QOAuth2AuthorizationCodeFlow` with PKCE and a loopback redirect bound to `LocalHost` (a test checks this). It has a sign-in timeout and a themed browser callback page (`SignInPage`). Microsoft is a public client with no secret.
- **How credentials ship:** the client id and secret come from CMake cache vars, then the env vars `CALLIE_GOOGLE_CLIENT_ID`/`SECRET`, then the gitignored `google-oauth-credentials.json`.
  - They are validated against `^[A-Za-z0-9._-]*$` and written into a generated header (`GoogleBuildConfig.h.in`), never into compile definitions.
  - They can be overridden by env at runtime.
  - Release CI injects them from Actions secrets, and `CALLIE_REQUIRE_GOOGLE_CLIENT=1` fails a tag build that has none.
- **Network:** plain `QNetworkAccessManager` REST clients (`GoogleCalendarApi`, `GraphApi`, `CalDavClient`) with token providers that refresh and retry once. Tests use fakes: `FakeHttpServer.h`, `FakeGraphServer.h`, `FakeCalDavServer.h`, `FakeTokenStore.h`.

### 2. Theme and design

- **Theme files:** `themes/*.toml` are Callie, Callie Light, Paper, Minimal, Minimal Dark, Tomorrow and Tomorrow Night. `callie.toml` is the default and sets every key; other themes inherit what they omit. They are compiled into qrc and also installed. Sections are `[theme] name/variant`, `[colors]`, `[calendar]` (OKLCH), `[shape]`, `[shadow]`, `[type]` and `[motion]`.
- **Dark palette (default):**
  - bg `#1c1220`, surface `#261a2a`, surface-alt `#33243a`
  - hairline `#2c2031`, border `#3d2c42`
  - text `#f6eef5`, text-muted `#c9b7c6`, text-faint `#9a8797`
  - **accent `#f08fb8`**, accent-text `#2e0f20`, danger `#e66e6d`
  - edge `#140c17`, accent-edge `#a64b7a`
- **Light palette:**
  - bg `#fbf5f8`, surface `#fffafc`, surface-alt `#f3e7ee`
  - hairline `#efe2e9`, border `#e0ccd7`
  - text `#2b1a26`, text-muted `#6b5562`, text-faint `#8f7a87`
  - **accent `#e45f97`**, danger `#c93c45`
  - edge `#dcc8d3`, accent-edge `#a83a6d`
  - shadow `#5c2a47` at 0.18
- **Shape and motion:** radii 6/11/14/16, sticker-edge 3; durations 140/200/280 ms, bounce 1.2.
- **Fonts:** Nunito Variable for UI text and Fraunces Variable for display, both bundled under OFL in `data/fonts/` and copied into `www/fonts/`. Weights are 600, 700 and 800. Sizes run 11 to 28 and are constants, not themeable.
- **`Theme` singleton:** `src/ui/ThemeController.h` (`QML_NAMED_ELEMENT(Theme)`, `QML_SINGLETON`). It holds theme-driven colors, radii, shadow, fonts and motion. Spacing `space1..7`, text sizes and grid metrics are CONSTANT. It also exposes contrast `warnings` and swatches. `Color.h` does OKLCH and WCAG contrast; `Base16.h` imports base16 schemes; `ThemeLibrary` and `ThemeEditor.qml` handle user themes, which are hot-reloaded.
- **Dark/light:** the default is dark. Light is a separate theme with `variant = "light"`, and `Theme.dark` is exposed. The app does not follow the system scheme (I found no colorScheme or styleHints code). The website does follow `prefers-color-scheme` and has a toggle.
- **Window chrome:** `Qt.FramelessWindowHint` with a custom title bar that doubles as the toolbar and uses `startSystemMove`. `ResizeFrame.qml` handles `startSystemResize`. `WindowControls.qml` plus `WindowButtons.cpp` follow the desktop's left-or-right button layout. `SingleInstance` (D-Bus), `TrayItem` (StatusNotifierItem/dbusmenu), `NotificationServer` and `StartAtLogin`/`Autostart` (XDG) round out desktop integration.
- **Accessibility:** about 31 QML files set `Accessible.name`/`role`/`checked`. Themes need 4.5:1 contrast, and the app warns rather than refusing a theme that falls short.
- **Generic reusable components** in `src/ui/`:
  - Buttons and controls: StickerButton, PillButton, DangerButton (click twice to confirm), Toggle, Dropdown, PickerButton, Field, FormRow, KeyCap
  - Menus, popups and overlays: MenuCard, MenuEntry, Tip, SoftShadow, Scrim, LazyPopup, SuggestionPopup, UndoToast
  - Icons and identity: Glyph (line icons drawn from rounded bars, so no image files), Logo (the face hops and wiggles)
  - Keyboard: WhichKey and KeyHelp, backed by the `KeyRouter` vi-mode and leader-key engine
  - Window chrome: WindowControls, ResizeFrame
  - App-level dialogs: AboutDialog, WhatsNewDialog (`ReleaseNotes` parses CHANGELOG.md), UpdateDialog, ThemeEditor
  - Also: SettingsDialog patterns, and `LiveQml`

### 3. Tooling

- **Makefile** targets: help, deps, setup (`npm ci`, hooksPath, configure), configure, build, run (with `DEV_ENV` logging rules), logs, cli, gallery, watch and watch-gallery (entr loop, QML live reload), screenshots, test, lint, format, check, clean.
- **scripts/:**
  - `format.sh fix|check` runs clang-format and qmlformat; qmlformat is checked by diffing.
  - `check-version.sh` checks that CMake, spec, debian/changelog, metainfo and CHANGELOG agree, including the release date.
  - `release-notes.sh VER` extracts that version's CHANGELOG section.
- **Node** is dev tooling only (`package.json` "callie-devtools"): commitlint, prettier 3.8.1 (md/json/yml/js at 120 columns), markdownlint-cli2. `.githooks/commit-msg` skips silently when node_modules is missing.
- **Formatters and linters:**
  - `.clang-format`: LLVM base, 100 columns, 4-space indent, Qt-style braces.
  - `.qmlformat.ini`: 100 columns, sorted imports.
  - `.qmllint.ini`: `MaxWarnings=0`.
  - `.editorconfig`, `.clangd` (points at `build/dev`), shellcheck on `install.sh` in CI.
- **commitlint:** a fixed type-enum and scope-enum, lower-case subject, 72-character header, body lines at most 100 (Dependabot exempt).
- **cliff.toml:** git-cliff groups only feat, fix and perf into New, Fixed and Faster. The draft is meant to be rewritten for users.
- **Tests:**
  - Qt Test, one executable per `tests/tst_*.cpp`, registered by `callie_add_test(name [BUS])`. `BUS` runs the test under `dbus-run-session`. Tests pin `TZ=UTC` and run with `QT_QPA_PLATFORM=offscreen`.
  - `tst_radicale` runs against a real CalDAV server.
  - Shell tests: `install_test.sh` (installer against a fake os-release and release JSON), `completion_test.sh`, `package_smoke.sh`.
- **Screenshot testing:** there is no pixel diffing. The app has `--sample --now --size --theme --screenshot`, `make screenshots` renders every theme, and CI uploads the images as an artifact.
- **CI** (`.github/workflows/`):
  - `ci.yml`: commitlint; docs (markdownlint, prettier, version check, shellcheck); format in a fedora:44 container; build matrix on fedora:44 and debian:trixie containers running `cmake --workflow --preset ci`; screenshots.
  - `release.yml`: on a `v*` tag, builds `git archive` into a `.tar.xz`, then `rpmbuild` on Fedora and `dpkg-buildpackage` on Debian, then installs each in a clean container and runs `package_smoke`, then `gh release create`.
  - `pages.yml`: deploys the site.
  - `code-review.yml`: calls the **shared reusable workflow** `lararosekelley/review-bot/.github/workflows/review.yml@v1` with `.github/review-bot/REPO_NOTES.md`.
- **Releasing** (`RELEASING.md`): bump the version in CMakeLists, spec `%changelog`, `debian/changelog` and the metainfo `<release>`; `git cliff --unreleased --tag vX.Y.Z`; hand-write the CHANGELOG section; run `check-version.sh`; merge; tag on main and push.
- **Packaging:** hand-written `packaging/rpm/callie.spec` (BuildRequires on `cmake(...)`, `%ctest`, desktop-file-validate, appstream-util) and `packaging/debian/` (debhelper 13, `dh --buildsystem=cmake+ninja`). **Packages are not GPG-signed.** `install.sh` verifies the SHA-256 digest GitHub records for each release asset.
- **Release tarballs:** `.gitattributes` uses `export-ignore` to keep `www/`, dev tooling and the agent docs out of them.

### 4. Website

- `www/` is static, hand-written HTML/CSS/JS with no build step: `index.html`, `style.css` (CSS variables copied from the two Callie themes), `theme.js` (no-flash light/dark toggle, `localStorage` plus `prefers-color-scheme`), `carousel.js`, `privacy.html`, `terms.html`, fonts, and screenshots with dark and light variants.
- `pages.yml` uploads `www/` as-is on pushes to main that touch `www/**`.
- `CNAME` is `callieapp.org`.
- `install.sh` lives in `www/`, so it is served at `https://callieapp.org/install.sh` (`curl ... | sh`). It supports `--dry-run`, `--force` and `--version`; only Fedora 44+ and Debian 13+ on x86_64 are supported, and Ubuntu is refused.

### 5. Logo and assets

- `assets/logo.svg` (128 viewBox) is a cute pink calendar face. The binding rings are worn as ears in `#a23f72`. The body is a rounded rect with a gradient from `#f7aed0` to `#d8639c`, cheeks are `#e0508e` at 0.55, and the closed eyes and smile are `#3a1630`.
- `assets/logo.png` is 512x512, 16-bit. It is used for the window icon, tray, notifications and `Logo.qml`. I found no generation script, so it appears to be exported by hand.
- **No rasterized icon sizes are shipped.** Only the SVG is installed, to `hicolor/scalable/apps/<appid>.svg`. Hand-tuned 16 and 32 px icons are a roadmap item.

### 6. CLI design

- Built on QCommandLineParser with subcommands: agenda (the default with no arguments), search, invites, contacts, add, edit, respond, delete, duplicate, accounts, calendars, import, export, subscribe, settings, sync, logs, status, doctor, upgrade, downgrade and completions.
- `--json` on every command that prints data. Data goes to stdout, everything else to stderr. Exit codes are 0, 1 and 2. Quiet on success. No prompts when not a TTY.
- Colors come from the active theme (`cli/Palette`, `Paint`), only on a TTY, and never with `NO_COLOR`. `CALLIE_THEME` picks a theme for one run.
- Bash, zsh and fish completions are embedded in qrc and also installed, and they complete dynamic ids.
- `--verbose`. `doctor --report` masks emails and opens an issue. `upgrade` and `downgrade` reuse the release digest logic. There is a daily update check, with `CALLIE_NO_UPDATE_CHECK` to disable it.
- An FTXUI full-screen TUI uses the same vi keys as the app.
- Commands live in a library that takes `QTextStream` out and err, so tests can drive them.

### 7. Generic and extractable vs Callie-specific

**Generic** (would need renaming from `callie`/`CALLIE_`):

- **Build:** CMake skeleton and presets, devel identity (`Identity`), LiveQml, gallery app.
- **Repo tooling:** Makefile, `format.sh`, `check-version.sh`, `release-notes.sh`, commitlint config (scopes differ), cliff.toml, prettier and markdownlint configs, `.clang-format`, `.qmlformat.ini`, `.qmllint.ini`, `.editorconfig`, `.gitattributes`, git hook.
- **CI and release:** all four workflows, `package_smoke.sh`, `install.sh` plus `install_test.sh`, the rpm and deb skeletons.
- **Theming:** `ThemeLoader`/`ThemeSpec`/`ThemeLibrary`/`Base16`/`Color` (OKLCH, contrast), `ThemeController`, the theme TOMLs minus `[calendar]`.
- **Accounts and network:** `TokenStore`, `GoogleAuth`/`GoogleClientConfig`/`GoogleTokenProvider`, the build-time credential injection CMake, `SignInPage`, `AccountStore`/`AccountManager` ordering, `Outcome`, the `QueuedSource` pattern (optimistic, undoable, persisted queue).
- **App services:** `Settings` pattern, `Logging`/`LogFile`, `Diagnostics` (email masking), `UpdateCheck`/`Releases`, `Autostart`/`StartAtLogin`, `SingleInstance`, `TrayItem`, `NotificationServer`, `KeyRouter`, `WindowButtons`.
- **QML and CLI:** all chrome and widget QML listed above; CLI `Paint`/`Palette`; the completion-in-qrc pattern.
- **Website:** `www/` style.css, theme.js, carousel.

**Callie-specific:** Event, Ics, recurrence, CalDAV, Graph, the Google Calendar API, the sources, the models (Month/Today/Search/Invites), QuickAdd, Places, Contacts, the views, the `[calendar]` theme section, grid metrics.

**Existing shared-code strategy:** only `lararosekelley/review-bot` (reusable workflow, local checkout at `/home/lara/Code/personal/review-bot`). There is no shared C++/QML library or template repo. `/home/lara/Code/personal/jot` already exists.

### 8. Normative docs (key rules)

- **AGENTS.md and CLAUDE.md** are identical stubs: "Read [GUIDELINES.md] before writing code, comments, or commit messages."
- **GUIDELINES.md:**
  - Layout table and conventions: "Every visual value in QML reads from the `Theme` singleton"; "Date and layout arithmetic belongs in C++ models, not QML."
  - "Qt 6.8 (Debian trixie) cannot add a bare `u"..."` literal to a QString; use `QStringLiteral` or `_s`."
  - Settled: "Qt6 + QML, not Electron, Tauri, or GTK4/libadwaita"; "No Kirigami"; scope is a calendar only; Google via the v3 REST API; distribution is an own repo first.
  - "Log through `lcAuth`... never `qDebug()`, and never log tokens."
- **CONTRIBUTING.md:**
  - Conventional Commits, "Subject is lower case, imperative mood, no trailing period", 72-character header, body wrapped at 100.
  - Comments: `///` on public API and on the root of every QML component; "One or two sentences", "explain why, not what", "No em dashes", "Never commit commented-out code", `TODO(scope):`.
  - "New logic in `src/core/` needs a test"; follow the fake-source pattern; pin TZ.
- **DESIGN.md:**
  - "Every line should be specific enough that a diff could violate it."
  - "Do the plainest thing." North star: "friendly, cute and personal... the way Things is"; the craft bar is Morgen; "for Linux enthusiasts".
  - Non-goals: no AI, no accounts, servers or telemetry, no theme-supplied code ("Themes are data").
  - Rules: "A literal color, radius or duration in a `.qml` file is a bug." "Pink appears in a fixed set of places: today, the now line, selection, focus, and the primary action." "Hairlines separate, borders enclose." Pressables are "stickers" raised on a 3px edge. "Dark first." "Light and dark are equal citizens." "Three text weights... Nothing gets a fourth." Font sizes "Never below 11." 8px spacing rhythm.
  - CLI rules follow clig.dev, as in section 6.
- **REPO_NOTES.md** (review bot, `.github/review-bot/`) repeats the binding seams: credentials never committed or logged; PKCE plus loopback on LocalHost; core has no QML; new dependencies need matching rpm and deb entries.
