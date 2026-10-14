# Jot roadmap

Jot is a Linux-first notes app built with Qt 6 and QML. Its first job is to replace Joplin for
everyday Markdown notes. Its second job is to be the best Linux home for long-form writers and
for people building fictional worlds and games, with first-class support for characters, places
and other story entities.

Jot is the second app in a small suite with Callie (and later a to-do app). The code lives at
`github.com/callieapp/jot`, the app id is `org.callieapp.Jot`, and the website is a section of
Callie's site at <https://callieapp.org/jot>. The first packages are .deb and .rpm for Fedora,
Debian and Ubuntu.

Background research lives in [docs/research/](research/):

- [callie-reuse.md](research/callie-reuse.md): what to take from Callie
- [joplin.md](research/joplin.md): Joplin and open-source peers (QOwnNotes, Marknote, Ghostwriter, Logseq)
- [commercial-apps.md](research/commercial-apps.md): Obsidian, Notion, Bear, iA Writer, Typora, Capacities, Tana
- [writers-and-worldbuilding.md](research/writers-and-worldbuilding.md): Scrivener, novelWriter, Fountain, articy, Ink
- [technical.md](research/technical.md): sync engine, backend APIs, Markdown editing in QML, search, packaging

## Positioning

The market gap is clear: a fast, native, good-looking app that keeps notes as plain files and
treats sync as optional. Joplin is open and syncs everywhere, but it runs on Electron, keeps notes
in SQLite under hex ids, and its UI is dated. Obsidian has the right model (a folder of Markdown
files) but is proprietary Electron and charges for sync. QOwnNotes is native and file-based, but
dense, keeps tags outside the files, and leaves sync to others. Writers on Linux have no
Scrivener, and the worldbuilding tools (Campfire, World Anvil, LegendKeeper) are cloud services.

Jot's promises:

1. **Your notes are files.** A vault is a folder of `.md` files with YAML front matter. Folders
   are notebooks. Tags, to-dos and properties live in the files. Every database Jot keeps is a
   cache that can be deleted and rebuilt.
2. **Native and fast.** Cold start and search should be quick enough to measure and publish,
   for example a 10,000-note vault.
3. **Sync is yours to choose.** No sync, a folder that Syncthing or Nextcloud already syncs, S3,
   Dropbox or Google Drive. No Jot account and no Jot server.
4. **No AI, no telemetry, no subscription.** This carries over Callie's non-goals and is a
   selling point given what Notion, Evernote, Tana and others have done.
5. **Structure is opt-in.** You can write without first setting up types, templates or projects.
   The writer and worldbuilding tools grow out of the same plain files.

## Settled decisions

These follow Callie's GUIDELINES.md and DESIGN.md unless noted.

- **Stack:** C++20, Qt 6.8 minimum, CMake with presets, Ninja, QML with Basic controls, no
  Kirigami. Dependencies come from distro packages via `find_package`.
- **Layers:** the same layering as Callie:
  - `jotcore`: a static library with no QML.
  - `Jot.Ui`: the QML module plus controllers.
  - `jot-gui`: the desktop app.
  - `jot`: the CLI, built from a library so tests can drive it.
  - `jot-gallery`: a component and theme viewer.
- **Design language:** Callie's theme system, palette, fonts (Nunito and Fraunces), radii,
  motion, sticker buttons, frameless window and design rules. Dark first, light equal. Pink is
  reserved for a fixed set of uses. The editor needs one added font role, see Milestone 1.
- **License:** MIT, matching Callie. Never copy Joplin source (AGPL); write Joplin import from its
  documented formats.
- **Storage:** plain files are the source of truth. SQLite holds the index, search and sync state.
- **Editor:** the Markdown text in a QML TextEdit is the source of truth. Live preview comes from
  a QSyntaxHighlighter, never from round-tripping through `QTextDocument::setMarkdown`. Marknote
  is rewriting its editor because that round trip was lossy.
- **Sync engine:** Jot's own, Unison-style three-way engine over thin transports. Not rclone
  bisync and not the Joplin sync format.
- **Names:** repo `callieapp/jot`, app id `org.callieapp.Jot`, binaries `jot-gui` and `jot`.
  Debian's `athena-jot` package may also install `/usr/bin/jot`. If it does, the .deb declares
  `Conflicts: athena-jot`.
- **Platforms:** Fedora 44+, Debian 13+ and Ubuntu 26.04+ on x86_64. All three ship Qt 6.8 or
  newer. Ubuntu 24.04 (Qt 6.4) waits for Flatpak.
- **Packaging:** hand-written rpm spec and debian/ directory, built and smoke-tested in containers
  in CI, plus an installer at `callieapp.org/jot/install.sh`.
- **Website:** a `www/jot/` section of Callie's site, in the Callie repo, with its own landing
  page, privacy and terms pages adapted from Callie's, and installer. It shares Callie's
  stylesheet and theme script. Links stay relative so the section can move to its own domain
  later.

## Reuse from Callie

Code is copied from Callie, not shared, for now. Copying is set up so that a shared "kit" can be
extracted once a third app (to-dos) arrives. [SUITE.md](SUITE.md) has the rules: shared files stay
identical to Callie's, and app-specific values sit in one marked place per file.

| Copy nearly as-is                                                                                                                                                                                                | Adapt                                          | Leave behind                             |
| ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------- | ---------------------------------------- |
| CMake skeleton, presets, `Identity` devel mode, LiveQml, gallery                                                                                                                                                 | Settings (notes options)                       | Event, ICS, recurrence, CalDAV, Graph    |
| Makefile, `format.sh`, `check-version.sh`, `release-notes.sh`                                                                                                                                                    | commitlint scopes                              | Calendar views and models                |
| `.clang-format`, `.qmlformat.ini`, `.qmllint.ini`, prettier, markdownlint, `.editorconfig`, `.gitattributes`, git hook                                                                                           | Theme TOMLs: drop `[calendar]`, add `[editor]` | `[calendar]` theme section, grid metrics |
| ThemeLoader, ThemeLibrary, Base16, Color (OKLCH, contrast), ThemeController, ThemeEditor                                                                                                                         | `QueuedSource` pattern applied to sync         | QuickAdd, Places, Contacts               |
| Theme files (Callie, Callie Light, Paper, Minimal, Tomorrow)                                                                                                                                                     | CLI commands                                   |                                          |
| TokenStore and Keychain, GoogleAuth, PKCE loopback, SignInPage, credential injection CMake                                                                                                                       | `doctor`, `status`                             |                                          |
| AccountStore and AccountManager ordering, Outcome, Logging, Diagnostics                                                                                                                                          |                                                |                                          |
| UpdateCheck, Releases, `upgrade`/`downgrade`, SingleInstance, TrayItem, Autostart, KeyRouter (vi keys, WhichKey)                                                                                                 |                                                |                                          |
| QML: StickerButton, PillButton, DangerButton, Toggle, Dropdown, Field, FormRow, MenuCard, Tip, Scrim, LazyPopup, SuggestionPopup, UndoToast, Glyph, WindowControls, ResizeFrame, About, WhatsNew, Update dialogs |                                                |                                          |
| CLI Paint and Palette, embedded completions, `--json` conventions                                                                                                                                                |                                                |                                          |
| CI workflows (ci, release, pages, review-bot), `package_smoke.sh`, `install.sh` with tests, rpm and deb skeletons                                                                                                |                                                |                                          |
| `www/` style.css, theme.js, carousel, privacy and terms pages                                                                                                                                                    |                                                |                                          |

## Vault format (draft)

Settle this before writing code, because everything else depends on it.

```text
My Notes/
  .jot/
    vault.yaml        # vault settings, explicit ordering (optional)
    types/            # note types: character.yaml, location.yaml, scene.yaml
    views/            # saved table, card and board views
    templates/
  Inbox/
  Recipes/
    Pancakes.md
    Pancakes.assets/  # attachments next to the note (or a shared assets/ folder)
  Ashfall (game)/
    Characters/Mara.md
```

```markdown
---
title: Mara Vey # optional; filename is the default title
tags: [ashfall, cast/lead]
type: character # optional
aliases: [Mara, the Ferrywoman]
created: 2026-10-14T15:30:00Z
todo: false # to-dos are notes with todo fields, as in Joplin
---

Body in CommonMark + GFM, with [[wikilinks]], ![[embeds]], $math$, and footnotes.
```

Notes on the format:

- Front matter is optional. Unknown keys are preserved. When Jot changes a key, it edits only
  that key's lines so the user's formatting survives.
- No ids are required in front matter. Links resolve as in Obsidian: exact path, then shortest
  unique name, then aliases. That keeps vaults compatible with Obsidian.
- Machine-local state (the index, sync bases, caches) lives under `$XDG_STATE_HOME` or
  `$XDG_CACHE_HOME`, never inside the vault, so syncing the vault never syncs the cache.

## Milestones

Each milestone ends in a release that is useful on its own. Versions follow Callie (0.x until
the format and sync are trusted).

### Milestone 0: Foundation

- Done:
  - Tooling copied from Callie: Makefile, CMake presets, clang-format, qmlformat, qmllint,
    prettier, markdownlint, commitlint, git-cliff and the commit hook.
  - A build with `jotcore`, `Jot.Ui`, `jot-gui`, `jot` and a first test.
  - CI on fedora:44, debian:trixie and ubuntu:26.04.
  - Draft logo: a spiral-bound pink notebook with Callie's face.
- Bring over from Callie:
  - The theme system and themes, the window chrome, the gallery, LiveQml with `make watch`,
    `Identity` devel builds, logging, and `make screenshots`.
  - `check-version.sh`, `release-notes.sh`, RELEASING.md, the release workflow, the rpm and deb
    packaging, `package_smoke.sh`, and the review-bot workflow.
- Logo: finish the notebook. Hand-tune the 16 and 32 px icons, which Callie still lacks.
- Website: add `www/jot/` to the Callie repo with the landing page, privacy, terms and
  `install.sh`, and link it from Callie's footer.
- Write the vault format spec (above) as `docs/FORMAT.md`.

### Milestone 1: A notes app I can live in (0.1)

Goal: Lara can move off Joplin for local use.

- Open or create a vault. A notebook tree based on folders, with drag to move, rename and new
  folder. A note list that sorts by updated, created or title, and can be pinned. Trash goes to
  the system trash.
- Editor:
  - Live-preview highlighting: headings scale, markers dim and appear only on the cursor's
    block. Lists, task checkboxes, tables, links and wikilinks.
  - Fenced code highlighting through KSyntaxHighlighting.
  - Images shown inline under their line.
  - Autosave with QSaveFile.
  - One key toggles source mode.
  - Paste or drop to attach files.
  - Smart list continuation and table formatting.
  - Add an `[editor]` theme section and a body font role. Writers will want a serif or mono
    option.
- Reading view rendered natively with md4c. Math with MicroTeX can come later in this milestone
  or the next.
- Links:
  - `[[wikilink]]` autocomplete.
  - Backlinks and unlinked-mentions panel.
  - Rename that rewrites inbound links.
  - Hover preview.
- Tags: nested tags (`#a/b`) in front matter and inline, plus a tag browser.
- To-dos: notes with checkbox state and a due date, and an "open to-dos" view. Reminders reuse
  Callie's notification and tray code, but can wait if needed.
- Search:
  - SQLite FTS5 with filters (`tag:`, `notebook:`, `type:`, `todo:`), snippets and highlighting.
  - A quick switcher (fzy-style) and command palette.
  - Vi keys through KeyRouter.
- Watching for outside edits: reload clean buffers, three-way merge dirty ones, rescan on startup.
- **Joplin import:** from a JEX export, rewriting `:/id` links into relative paths and
  attachments. Notebooks become folders; tags, to-dos and created/updated times go into front
  matter; HTML notes are converted or kept as `.html`. Import from the SQLite database comes
  next, to keep revisions.
- CLI: `jot new`, `jot search`, `jot open`, `jot ls`, `jot tags`, `jot import joplin`,
  `jot doctor`, with `--json` throughout.
- Packaging: .deb, .rpm, `install.sh`, and the site with real screenshots.

### Milestone 2: Sync (0.2 to 0.4)

Goal: the same vault on a laptop and a desktop through a cloud the user picks.

- Sync engine in `jotcore`, behind a `Transport` interface (list, stat, get, conditional put,
  delete, move, changes since):
  - Per-remote base state (hash, rev or etag) plus a content-addressed store of base text
    versions.
  - Three-way decisions per path.
  - Text merge with libgit2's `git_merge_file`, merging front matter key by key.
  - When a merge fails, write a conflict copy and show a banner with a side-by-side resolver.
  - Binaries keep the last write and also get a conflict copy.
  - Never lose data: when one side deletes and the other edits, keep the edit.
  - Rename detection by hash.
  - Callie's `QueuedSource` and `Outcome` model for retry and offline.
- Sync status in the title bar, as in Callie: last synced, failures, sync now.
- Transports, in order:
  1. **Folder:** a vault already inside Syncthing, Nextcloud or Dropbox desktop. Jot only
     watches it and shows `*.sync-conflict-*` and similar copies in the resolver. Also another
     path, such as a NAS mount.
  2. **S3-compatible:** hand-written SigV4, endpoint and region settings (AWS, R2, B2, MinIO,
     Wasabi), and checks for conditional-write support. Test in CI against MinIO.
  3. **Dropbox:** PKCE with offline tokens, an App folder, `list_folder/continue` cursors,
     `content_hash`, and rev-conditional uploads.
  4. **Google Drive:** the `drive.file` scope only, which is not sensitive and needs no
     security assessment. The `changes.list` delta. Files added through the Drive web UI stay
     invisible until picked, so document that.
- Note history: snapshots on save, kept locally and pruned, with a diff view.

### Milestone 3: Writer basics (0.5)

Goal: pleasant for daily writing, not just notes.

- Focus mode (dim everything but the current sentence or paragraph), typewriter scrolling,
  distraction-free fullscreen.
- Word and character counts. Targets per note, session and project, with deadlines. Session
  history and streaks in a stats file.
- Outline panel, split panes, tabs.
- Templates with variables. Daily notes.
- Comments with CriticMarkup, shown as margin notes.
- Export through system Pandoc to DOCX, ODT, PDF, ePub and HTML, with bundled presets including
  standard manuscript format (Shunn).
- Spellcheck via Sonnet or Hunspell.

### Milestone 4: Types, properties and views (0.6)

This is the foundation for the writer and worldbuilding tools.

- Note types in `.jot/types/*.yaml`: name, icon, color, a property schema (text, number, date,
  enum, list, link to a type, image) and a body template. Ship starter types (Character,
  Location, Faction, Item, Scene, Chapter, Event).
- Properties panel above the editor, with typed widgets.
- Saved views over properties (table, cards, board grouped by a field), like Obsidian Bases.
  They make "all characters" a cast sheet and "all scenes by status" a kanban.

### Milestone 5: Writing projects (0.7)

- A project is a folder with an ordered binder, stored in `.jot/` rather than in filename
  prefixes.
- Corkboard (synopsis cards) and outliner (property columns), both reorderable by drag.
- Scrivenings: edit many scenes as one document.
- Include and exclude from compile. Compile through Pandoc.
- Named snapshots with word-level diffs.
- Fountain as its own document mode: highlighting, autocomplete for scene headings and
  characters, a scene navigator, and screenplay PDF through afterwriting at first.
- Import from Scrivener (`.scrivx` plus RTF through Pandoc, one way only) and from novelWriter.

### Milestone 6: Worldbuilding and characters (0.8)

- **Codex:** entity types get `aliases`. Mentions in the text are detected (case-insensitive,
  with plurals) and underlined, without rewriting files. One action makes a mention an explicit
  link.
- Mentions panel and appearance heatmap per chapter. Explicit appearances (`pov`, `characters`
  on scenes) are shown apart from detected mentions.
- Typed relationships in front matter (`relationships: [{to: "[[Mara]]", kind: sister}]`),
  drawn as a relationship graph and family tree.
- Character template extras: interview question sets, voice samples, a mood board image grid.
- Timeline: events are notes with a story date. Keep narrative order and story-time order as
  two separate orderings.
- Progressions: changes to an entity anchored to a scene ("scar from chapter 12"), plus simple
  consistency checks against them.
- Image maps with pins that link to notes. No tile servers.

### Milestone 7: Game narrative (0.9)

- `.ink`, `.yarn` and `.twee` files: highlighting, knot, node and passage outlines, and a link
  graph. Codex mentions work inside them.
- Compile and validate by shelling out to inklecate, ysc and tweego, with errors shown inline.
- A game design document project template: pillars, core loop, systems, item, quest and faction
  types, and a decision log.
- Export entities and properties as JSON for game engines.

### Later

- End-to-end encryption for cloud targets: libsodium secretstream per file, Argon2id key,
  versioned format.
- rclone transport (`rclone rcd`) for WebDAV, SFTP, OneDrive and more. The package would only
  suggest rclone.
- An optional `jot-preview` subpackage using QtWebEngine for Mermaid and full-fidelity HTML.
- Canvas in JSON Canvas format, compatible with Obsidian.
- Global quick-capture hotkey into an inbox.
- Flatpak, then Arch and Nix.
- A small, versioned plugin or script API.
- Web clipper. Joplin users value it, but it is a large project.
- Mobile. Jot will have no app for a long time. Plain files in Dropbox, Drive or Syncthing mean
  any mobile Markdown editor (Markor, Obsidian) can read and edit the vault meanwhile. That
  fallback should be called out on the site.

## Risks

- **Editor quality decides everything.** A live-preview editor that jumps, lags or breaks IME
  will sink the app. Build a long-document test note and an IME smoke test early. Keep source
  mode as the escape hatch. Defer a block-based Typora-style editor until the highlighter
  approach has clearly run out.
- **Sync correctness.** Write the engine against fake transports with randomized two-device
  scenarios before any real backend. Never ship a sync path that can lose a byte.
- **Google verification.** `drive.file` avoids the security assessment, but the consent screen
  still needs brand verification for name and logo. Start it early, as for Callie.
- **Dropbox app review.** Development apps are capped at a small number of users until
  production approval.
- **Scope.** Milestones 4 to 7 are the differentiator but also the scope trap. Each must reuse
  the one index and the plain-file format. Avoid real-time collaboration, a home-grown
  typesetter, a flow-graph editor and map tiles.
- **Distro Qt floor.** Ubuntu 24.04 ships Qt 6.4 and cannot be supported natively. Point those
  users at Flatpak later.
- **Shared site.** Jot's pages deploy with Callie's, so a broken Jot page blocks Callie's site
  deploy and the other way round. Keep Jot's pages self-contained under `www/jot/`.

## Open questions

1. Attachments: next to each note (`Note.assets/`) or one `assets/` folder per vault or
   notebook? Joplin imports and Obsidian compatibility both favor a configurable folder.
2. Ubuntu support in Callie's installer, which refuses Ubuntu today. Jot's installer will accept
   26.04, so the shared installer logic should too.
