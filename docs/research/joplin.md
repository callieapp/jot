<!-- markdownlint-disable -->

# Joplin and open-source alternatives

Research snapshot from 2026-10-14, gathered to plan Jot. Prices and versions drift; check sources before relying on them.

Joplin research for Jot (as of 2026-10-14)

Facts about Joplin's code and sync format come from reading the laurent22/joplin `dev` branch. Facts about Marknote, Ghostwriter and QOwnNotes come from their source repositories. The rest comes from web sources linked inline.

## 1. Feature inventory

Joplin is at stable v3.7.21 (2026-09-25), has 56.7k GitHub stars, and runs on Electron 42. The apps are AGPL-3.0-or-later. Joplin Server uses a separate non-free "Personal Use License" (https://github.com/laurent22/joplin/blob/dev/LICENSE, https://lwn.net/Articles/1016400).

**Organization**

- Nested notebooks (folders with a parent_id).
- Tags, linked to notes through note_tag join items.
- To-dos: a note with `is_todo`, `todo_due` and `todo_completed`, plus alarms.
- Trash, since 3.0.
- Conflicts notebook.
- Multiple profiles.
- Multi-window, since 3.2.

**Attachments ("resources")**

- Each attachment is a separate item with a 32-hex id.
- Links use `:/<id>`, both for notes (`[x](:/id)`) and for resources (`![](:/id)`). HTML `<img src=":/id">` also appears.
- OCR of images and PDFs is on by default. As of 3.7 OCR text is searchable and can be layered over PDFs.

**Editors**

- The Markdown editor is CodeMirror 6. It was the "beta editor" from 2.x and became the default in 3.1.1 (Aug 2024). The CM5 editor survives as "legacy".
- In-editor rendering (live-preview style) became the default in 3.6.3 (Mar 2026). It includes a table editor, inline HTML and front matter blocks.
- The Rich Text editor is TinyMCE and round-trips to Markdown.
- A separate viewer pane uses markdown-it plus plugins.
- 3.7 added whiteboards and an AI chat panel with an MCP server and note embeddings (https://joplinapp.org/help/about/changelog/desktop).

**Rendering**: KaTeX math, Mermaid, abc notation, footnotes, TOC, checkboxes, `==mark==`, sub/sup, and HTML notes (`markup_language=2`, mostly from the web clipper).

**Plugins**: a JS/TS API with webviews, content scripts, CM6 extensions and renderer extensions. Plugins also run on mobile.

**Web Clipper**: a browser extension that talks to the desktop's local REST server.

**E2EE**: a master password protects the master keys, and items are encrypted in the sync target. The local SQLite database is not encrypted (LWN).

**Sync targets** (https://joplinapp.org/help/apps/sync/):

- Supported: Joplin Cloud, Joplin Server, Nextcloud, WebDAV, Dropbox, OneDrive, S3 and the local filesystem.
- Google Drive is not supported. The maintainer distrusts Google API stability and has said he'd accept a PR (https://discourse.joplinapp.org/t/feature-request-google-gdrive-support-option-for-sync-backup/1142).

**Search**: SQLite FTS4 (`notes_fts`, `items_fts`) with BM25 ranking, filters (`tag:`, `notebook:`, `created:`, etc.) and wildcards.

**History**: revisions are stored as diff-match-patch diffs (`title_diff`, `body_diff`, `metadata_diff`), kept 90 days by default, and they sync.

**Import/export**:

- JEX and RAW (both lossless).
- ENEX import.
- Markdown, and Markdown + front matter.
- HTML, PDF, JSON.
- OneNote import (improved in 3.6.11).

**Other**:

- Templates come from a plugin.
- Backlinks and graph views come from plugins (Backlinks, Link Graph). Core has no backlinks.
- Mobile apps: Android and iOS are React Native and use the same CM6 editor.
- Also: a CLI app, note publishing and notebook sharing (Cloud/Server only).

## 2. Sync architecture

**Layout of a file-like target** (Dirnames in `packages/lib/services/synchronizer/utils/types.ts`):

- `<id>.md` at the root, one file per item: notes, folders, tags, note_tags, resource metadata, revisions.
- `.resource/<id>`: raw attachment blobs.
- `locks/`: older targets used `.lock/`.
- `temp/`: also used for a clock-skew check (`temp/timeCheck*`).
- `info.json`: target version (currently 3), `e2ee`, `activeMasterKeyId`, `masterKeys`, `ppk` and `appMinVersion`. Per-property `updatedTime` values are merged heuristically (https://joplinapp.org/help/dev/spec/sync).

**Item serialization** (`BaseItem.serialize`/`unserialize`):

- The file starts with the title, a blank line, then the body.
- After a blank line comes a footer of `key: value` lines ending with `type_: N`.
- Type numbers: 1 note, 2 folder, 4 resource, 5 tag, 6 note_tag, 9 master key, 13 revision.
- The parser reads lines from the end up to the first blank line.
- Timestamps are ISO8601 UTC with milliseconds.
- `\n` is escaped in every property except body.
- Ids must match `^[a-f0-9]{32}$`.

**Delta sync**:

- File-like drivers (filesystem, WebDAV, S3) use `basicDelta`. It stats the whole root directory, sorts by mtime, and compares against a context `{timestamp, filesAtTimestamp}`.
- Deletions are found by diffing the remote id list against local ids.
- So every sync is O(total items) in directory listing. That is the root cause of slow WebDAV/Nextcloud syncs with tens of thousands of files, made worse by one HTTP request per item.
- Dropbox, OneDrive and Joplin Server use native cursor-based delta APIs.

**Locks** (https://joplinapp.org/help/dev/spec/sync_lock):

- Files are named `<sync|exclusive>_<desktop|mobile|cli>_<clientId>.json`.
- Many SYNC locks may coexist. One EXCLUSIVE lock is used only for target upgrades.
- TTL is 3 minutes, refreshed every 60 s (`LockHandler.ts`).
- Acquisition is "write, then re-check" because object stores have no atomic create.

**Conflicts**:

- Non-note items: the remote wins.
- Notes: the local copy is saved as a new note with `is_conflict=1` (the "Conflicts" notebook) and the remote overwrites.
- Conflicts on unimportant fields such as `todo_completed` resolve silently.
- Deleting a folder that still has notes marks those notes as conflicts.
- New in dev (PR #16023, merged 2026-09-06, not in stable 3.7.21): a 3-way automerge of title/body against a stored "sync base" copy (`autoMergeNote`, `handleConflictAction.ts`). It falls back to a conflict note when the merge fails or the item is encrypted or locked.

**E2EE** (https://joplinapp.org/help/dev/spec/e2ee, `EncryptionService.ts`):

- Random master keys are encrypted with the master password using KeyV1: AES-256-GCM, PBKDF2-SHA512, 220k iterations.
- Items use StringV1 and resources use FileV1: AES-256-GCM with a per-item salt-derived key. Legacy data uses SJCL AES-CCM.
- Ciphertext uses the "JED" header format: version, then a 32-hex master key id, then length-prefixed chunks.
- An encrypted item file keeps only `id`, `parent_id`, `updated_time`, `type_`, `share_id`, `deleted_time` and `encryption_cipher_text` in clear. Note counts and hierarchy leak.
- A PPK in `info.json` handles sharing.

**Good**: the format is simple, text-based and storage-agnostic (anything that can list/get/put/delete works), and decent for offline multi-device use.

**Bad**:

- Thousands of opaque `<hex>.md` files are not browsable by humans.
- Full listing on every sync.
- Per-file request overhead.
- Mtime and clock-skew sensitivity.
- Item-level last-writer-wins with conflict copies (until automerge ships).
- Lock files on eventually consistent stores.
- Tags and notebooks are separate join files, so one logical edit becomes several writes.

**Can Jot interoperate?**

- Read-only import from a filesystem, Dropbox or S3 target is very feasible: the parser is about 50 lines, and E2EE needs AES-GCM plus PBKDF2 and, for old data, SJCL CCM.
- Two-way live interop is risky. Jot would have to honor the locks protocol, `appMinVersion` and target-version upgrades, trash `deleted_time`, revisions, share_id, and Joplin's conflict semantics. Joplin can bump the target version under an EXCLUSIVE lock at any time.
- Don't copy Joplin code unless Jot is AGPL-compatible. Reimplementing from the spec is fine.
- Recommendation: one-way import first. Treat "act as a Joplin sync client" as an optional later bridge, not Jot's native format.

## 3. Complaints and loved features

**Complaints**:

- **Electron weight.** Over 500 MB RAM at startup (https://lwn.net/Articles/1016400). Reports range from 600 MB to over 1 GB, and from 8 to 13 GB after long sessions (https://discourse.joplinapp.org/t/memory-use-on-windows/33832, https://discourse.joplinapp.org/t/excessive-ram-usage/27333). Startup can take about 5 minutes with 27k resources (https://discourse.joplinapp.org/t/extreme-slow-starting/23499).
- **Sync speed.** One user reports Nextcloud taking 1.5 h for 910 items versus 17 min on plain WebDAV. There is one connection per file, and first syncs can take hours (https://discourse.joplinapp.org/t/sync-with-nextcloud-fast-with-mobile-extremely-slow-on-linux/4017, https://discourse.joplinapp.org/t/generating-too-many-connections-to-nextcloud-server/13673). LWN cites WebDAV latency and weak conflict resolution.
- **Opaque storage.** Data lives in SQLite. Editing outside Joplin breaks sync, and exports need cleanup (https://lwn.net/Articles/1016400). Tags and metadata aren't in the file, attachments use hex filenames, and links use the non-standard `:/id` syntax (https://discourse.joplinapp.org/t/reflecting-on-joplin/31821). A user even forked Joplin to mirror notes to .md files (https://discourse.joplinapp.org/t/created-a-new-version-of-joplin-that-keeps-local-md-files/33339).
- **Editors.** Switching between TinyMCE and Markdown "loses" data. Formatting gets stripped when pasting, and TinyMCE has spacing limits (https://discourse.joplinapp.org/t/markdown-editor/33246, https://discourse.joplinapp.org/t/joplin-has-again-removed-all-formatting/50230). Table editing is weak (LWN).
- **Plugin breakage.** The CM5 to CM6 switch shipped with a list of "plugins incompatible with the new editor" (changelog 3.1 to 3.3). Plugins are also the top cause of slowness on the forum.
- **Linux.**
  - Blurry on HiDPI Wayland without Ozone flags (https://discourse.joplinapp.org/t/enable-native-support-for-wayland-in-linux/15823).
  - IME needed `--enable-wayland-ime` (changelog 3.0).
  - AppImage needs libfuse2 and hits the Ubuntu 24.04 AppArmor chrome-sandbox problem, worked around with `--no-sandbox` (https://www.kubuntuforums.net/forum/currently-supported-releases/kubuntu-24-04-nitpick-noble-lts/post-installation-az/678671-appimages-libfuse2).
  - AppImage restart and StartupWMClass bugs were still being fixed in 2026.
- **UI.** "Less polished or intuitive" than Obsidian (LWN). People who left for Obsidian cite its polish, built-in backlinks and plain files (https://discourse.joplinapp.org/t/back-to-joplin-after-a-few-weeks-with-obsidian/15390).

**Loved features a replacement must keep**:

- Reliable "never lost a note" sync across many backends, including self-hosted ones.
- E2EE.
- Plain Markdown with good math and Mermaid rendering.
- Nested notebooks plus tags plus to-dos.
- Web clipper.
- Attachments stored inside the app.
- Note history.
- Fast full-text search with filters.
- Being fully open source, with a lean core and extras in plugins.
- Mobile apps.

## 4. Data migration

**Option 1: JEX export.** This is the best default because it is lossless and needs no running app.

- A JEX file is a tar of the RAW export (`InteropService_Exporter_Jex.ts` wraps `InteropService_Exporter_Raw.ts`).
- Root holds one `<id>.md` per item in the sync serialization above.
- `resources/<id>.<ext>` holds the blobs.
- Parse it with the same footer rules, then rebuild the tree from `parent_id`, tags from type 6 items, and to-do fields.
- Rewrite `:/id` links (Markdown and `<img src=":/id">`) to relative paths.
- Handle `markup_language=2` HTML notes.
- Revisions are probably not included; this is unverified.

**Option 2: Read the SQLite database directly.** Use this when you want history.

- Database: `~/.config/joplin-desktop/database.sqlite`. Blobs: `~/.config/joplin-desktop/resources/<id>.<ext>`. Flatpak installs use `~/.var/app/net.cozic.joplin_desktop/config/joplin-desktop`.
- Tables: `notes` (id, parent*id, title, body, created_time/updated_time/user*_ in ms epoch, is*todo, todo_due, todo_completed, is_conflict, markup_language, source_url, author, deleted_time, encryption*_), `folders`, `tags`, `note_tags`, `resources` (mime, filename, file_extension, size, ocr_text), `revisions`, `notes_fts` (FTS4), `sync_items`, `deleted_items`, `settings`, `key_values`, plus new 3.7 AI tables (from `packages/lib/services/database/types.ts`).
- Open it read-only, and only while Joplin is closed.
- The schema migrates between versions, so code defensively.

**Option 3: Data API.** Use this for live or incremental migration or a bridge (https://joplinapp.org/help/api/references/rest_api).

- Port 41184; scan 41184 to 41194. `GET /ping` returns "JoplinClipperServer".
- `?token=` comes from Web Clipper options or a programmatic auth flow.
- Endpoints: `/notes`, `/folders`, `/tags`, `/resources/:id/file`, `/search` and `/notes/:id/revisions`.
- `/events?cursor=` returns a change feed retained 90 days.
- Pagination uses `page`/`limit` (max 100) and `has_more`. Select columns with `fields=`.
- The desktop app must be running.

**Option 4: Read a filesystem sync target** with the parser from section 2. This needs decryption if E2EE is on.

**Option 5: Markdown + front matter export.** Human-readable, with links rewritten, but it loses ids and history.

## 5. Competitors: takeaways for Jot

**QOwnNotes** (Qt Widgets, C++, https://www.qownnotes.org/getting-started/concept.html). This is the closest prior art.

- Notes are plain .md files in a folder with subfolders.
- QOwnNotes does not sync itself. It relies on the Nextcloud client, Syncthing, Dropbox or git. The QOwnNotesAPI Nextcloud app adds server-side version history and trash.
- Editor: QMarkdownTextEdit (pbek's library), a QPlainTextEdit with a hand-written QSyntaxHighlighter, plus FakeVim and Sonnet spellcheck.
- Preview: bundled md4c to HTML, shown in a QTextBrowser. Qt6 builds can use a qlitehtml (litehtml) preview, an on-by-default CMake option.
- Also: Botan AES note encryption, QML/JS scripting, a browser web companion, Nextcloud Deck integration, and newer LSP, LanguageTool/Harper, OpenAI and MCP services.
- **Weakness: portability.** Tags live in `QOwnNotes.sqlite` inside the note folder, not in the files.
- **Weakness: preview fidelity.** QTextBrowser supports only a limited HTML/CSS subset (https://doc.qt.io/qt-6/richtext-html-subset.html) and runs no JS, so math and Mermaid need scripts.
- **Weakness: UI and code size.** A dense, dock-heavy UI, and `mainwindow.cpp` is about 8.7k lines.
- **Lesson:** copy the plain-files model, but put tags in front matter, build sync in rather than delegating it, and choose a better renderer.

**Marknote** (KDE, Kirigami/QML, https://apps.kde.org/marknote/). This is the closest to Jot's stack.

- Notebooks are folders with a `.directory` desktop-entry file for icon and color. Notes are .md files in a configurable folder.
- Parsing uses md4qt (KDE's AST parser), with md4c optional for HTML.
- The rich editor in released versions sits on QQuickTextDocument/QTextDocument (`notesmodel.cpp` still calls `QTextDocument::setMarkdown`).
- 1.5 (Mar 2026) added a source mode with highlighting, cross-notebook wiki links and a KRunner plugin. 1.6 (May 2026) added subfolders.
- Master (2026) is replacing the editor with a block editor: `MDTreeModel` over the md4qt AST, per-block QML `TextEdit`s, and `MDSerializer` back to Markdown. That rewrite tells you QTextDocument's Markdown round-trip was not faithful enough.
- No sync, tags or search beyond a command bar.
- **Lesson:** don't make QTextDocument's markdown import/export the source of truth. Edit the Markdown text, or use an AST-backed block model.

**Ghostwriter** (KDE, Qt Widgets). A single-document writing app.

- It bundles cmark-gfm, and its parsed AST drives a custom QSyntaxHighlighter.
- Preview is a sandboxed QtWebEngine page with bundled MathJax. Export goes through cmark-gfm or a command-line Pandoc/MultiMarkdown.
- Sonnet spellcheck. Strong focus and Hemingway modes.
- No library, tags or sync, and WebEngine is a heavy dependency.
- Neither Ghostwriter nor Marknote uses KSyntaxHighlighting. Jot could use it for fenced code blocks: QML-friendly, with 300+ languages.

**Obsidian**: the benchmark for UX. A plain-folder vault, wiki links, backlinks and a graph in core, properties (YAML), and a big plugin ecosystem. Proprietary Electron, with paid E2EE sync. Takeaway: make a plain folder the database, treat backlinks and front matter as first-class, and keep the app's index rebuildable.

**Logseq**: an outliner. It split into a file-based "OG" app (maintenance only) and a DB version (2.0 beta, mid-2026) with a read-only Markdown mirror; two-way sync is still in development (https://discuss.logseq.com/t/logseq-db-changelog/30013/37). Takeaway: a SQLite source of truth fights plain files, so pick one canonical store.

**Standard Notes** (now owned by Proton): an audited E2EE design and a minimal editor. Takeaway: if Jot adds E2EE, encrypt per item with a documented, versioned format.

**Trilium/TriliumNext**: a powerful hierarchical, scriptable, cloneable-notes app stored in SQLite with its own sync server. Sync-version bumps broke compatibility with the old server (https://docs.triliumnotes.org). Takeaway: version your sync protocol carefully.

**Zettlr**: Electron, academic. Plain files, Pandoc and Zotero citations. Takeaway: front matter plus Pandoc export covers serious writing needs.

**Notesnook**: open-source E2EE Evernote-style app with a polished UI and its own sync. Takeaway: users want E2EE plus a good mobile experience, and Joplin's UI is the weak point to beat.

**SilverBullet v2**: Markdown pages in a plain folder, with Space Lua for queries, templates and commands (https://community.silverbullet.md/t/silverbullet-v2-released/3100). Takeaway: an embedded scripting and query layer over plain files is powerful without a proprietary store.

**AppFlowy / Anytype**: block-based, Notion-like, with CRDT sync (Anytype's any-sync) and data in their own stores, not Markdown. Takeaway: CRDTs remove sync conflicts but cost file transparency, so they don't fit Jot's goals.

## Recommendations for Jot

- Store notes as plain .md files with YAML front matter (id, tags, todo fields, created/updated), with folders as notebooks and attachments beside the notes or in a per-notebook folder.
- Keep SQLite FTS5 only as a rebuildable cache.
- Sync with a manifest or index file so a sync isn't a full directory listing: diff hashes, use ETag/version checks for optimistic concurrency on S3 and Dropbox, and do a 3-way merge against a stored base. Joplin is only now adding this.
- Ship a JEX importer first, with a SQLite reader as the option that keeps history.
- For the editor, use a QML TextArea over raw Markdown with a QSyntaxHighlighter (md4c or cmark-gfm positions, plus KSyntaxHighlighting for code), and a separate preview.

**Not verified**: whether JEX includes revisions, the exact Markdown + front matter field list, and the current QOwnNotes Android situation (its docs mention an Android app; historically it used Nextcloud Notes).
