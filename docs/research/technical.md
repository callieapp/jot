<!-- markdownlint-disable -->

# Sync, editor, search and packaging technology

Research snapshot from 2026-10-14, gathered to plan Jot. Prices and versions drift; check sources before relying on them.

Jot technical research: recommendations for Lara's Qt 6/QML markdown notes app, as of 2026-10-14.

Stack read from Callie (read-only): Qt 6.8 minimum; Core/Quick/Network/NetworkAuth/Sql/DBus; Qt6Keychain; QOAuth2AuthorizationCodeFlow with setPkceMethod and QOAuthHttpServerReplyHandler (loopback); SQLite through Qt Sql. Packaging is native, in /home/lara/Code/personal/callie/packaging/debian and packaging/rpm/callie.spec. Jot should reuse all of this: the OAuth/token provider pattern, the keychain wrapper, the SQLite cache and the packaging layout.

Version checks were done by web search on the date above. Some API details below come from my own knowledge rather than a page I fetched, and I mark those "(unverified)".

1. SYNC ENGINE

Recommendation: write Jot's own engine in the Unison style. Keep per-remote "last synced base" state and make three-way decisions per path. Give the engine a small Transport interface (list, stat, get, put-conditional, delete, move, changes-since) with native transports for Local folder, Dropbox, Google Drive and S3. Optionally add an rclone transport later.

State: one SQLite DB per vault in <vault>/.jot/ (or in ~/.local/state so it never syncs). Table sync_state(remote_id, path, file_id, base_hash, base_size, base_mtime, remote_rev, remote_etag, remote_content_hash, last_sync), plus a local_index(path, size, mtime_ns, inode, sha256).

- Re-hash a file only when size, mtime or inode changes.
- Keep base content for text files in a small content-addressed blob store (.jot/base/<sha256>). It is the merge ancestor. Garbage-collect it after the file syncs cleanly on all remotes.
- This is Unison's "archive" model: the ancestor is needed to tell edit from delete and to do real three-way merges (https://www.cis.upenn.edu/~bcpierce/unison/download/releases/stable/unison-manual.html).

Decision table per path (L = local, R = remote, B = base):

- L == B and R != B: pull.
- L != B and R == B: push.
- Both changed: three-way merge for .md/.txt, otherwise make a conflict copy.
- Absent locally but present in B, R unchanged: delete remote. The mirror case deletes local.
- Delete on one side and modify on the other: keep the modified file. Never lose data.
- Deletes go to the system Trash locally (QFile::moveToTrash) and to a .jot/trash/ prefix remotely, or use the provider's own trash. Base state removes the need for explicit tombstones. For stores without cursors, the deleted set is the listing diff against base.

Text merge: use git_merge_file() from libgit2. It does a line-based diff3 merge with conflict markers, needs no repository, and is packaged everywhere (GPLv2 with linking exception).

- Merge the front matter key by key first, then the body.
- Prefer the line merge to diff-match-patch fuzzy patching. Obsidian Sync uses diff-match-patch for markdown and documents duplicated text and broken formatting as failure modes. Since 1.9.7 it offers a "create conflict file" option instead (https://help.obsidian.md/sync/troubleshoot, https://forum.obsidian.md/t/robust-sync-conflict-resolution/93544).
- If the merge has conflicts, write "Note (conflict <host> 2026-10-14 1530).md" and keep the local file as winner. Do not leave markers in the main note.
- Show a conflict banner with a side-by-side resolver.
- Binaries and attachments never merge: last writer wins plus a conflict copy, keyed by hash.

Renames and moves:

- Locally, a delete and a create with the same sha256 in one scan is a rename. Use the inode too if it is stable.
- Dropbox ("id:...") and Drive (fileId) have stable ids, so remote moves show up directly; issue server-side moves.
- S3 has no rename, so do copy plus delete.
- Store a stable note id in front matter only if the user opts in. Do not require it.

Atomic writes: use QSaveFile (temp file, fsync, rename; https://doc.qt.io/qt-6/qsavefile.html).

- Rename replaces the inode. Watch directories, not files, and record "expected hash after my write" so the app ignores its own events.
- QSaveFile also replaces symlinks and hard links. Detect them and fall back to in-place writes.

File watching:

- QFileSystemWatcher uses inotify on Linux. It is not recursive, so add each directory (https://doc.qt.io/qt-6/qfilesystemwatcher.html).
- The inotify max_user_watches default scales with RAM since kernel 5.11, so it is rarely a limit for a notes vault (https://man7.org/linux/man-pages/man7/inotify.7.html).
- Still handle failure: debounce for about 300 ms, rescan the changed directory, and run a periodic full stat scan (and one at startup), because events are lost on IN_Q_OVERFLOW, on network mounts (NFS/SMB give no inotify for remote changes) and under Flatpak document-portal FUSE.

External edits in the editor:

- Clean buffer: reload silently, keeping cursor and scroll.
- Dirty buffer: three-way merge of buffer vs disk using the buffer's load-time content as base, with the same merge code.

"Folder" target, two modes:

- (a) The vault lives inside a Syncthing or Nextcloud folder. Jot does no sync. It only handles external edits and surfaces Syncthing's "_.sync-conflict-YYYYMMDD-HHMMSS-DEVID._" files with the same resolver UI. Syncthing uses version vectors and keeps the older conflict copy (https://docs.syncthing.net/users/syncing.html).
- (b) A Local transport to another path such as a NAS mount, using the same engine.

Prior art:

- Joplin stores items as .md with id-based metadata and puts conflicts into a "Conflicts" notebook (https://joplinapp.org/help/dev/spec/sync). Its sync-target format is not a human vault, so do not copy the format. Do copy the lock and batch ideas.
- Git via libgit2 is a reasonable later "git remote" mode for power users. It is the wrong fit for dumb stores.

rclone:

- MIT licensed. Bisync left beta in v1.71 (Aug 2025) (https://rclone.org/bisync/). librclone exists as a C shared library with a JSON RPC call, but it is a full Go runtime of tens of MB.
- Pros: 70+ backends, mature, chunking and crypt.
- Cons:
  - Bisync conflicts are file-level only (rename or "newer" wins), with no content merge.
  - It needs --resync bootstrapping and keeps its own listing files.
  - It owns OAuth config (rclone's shared client IDs and their rate limits).
  - Large to bundle in .deb/.rpm, though both distros package rclone.
- Recommendation: do not use bisync as the engine. Later, add an optional "rclone remote" transport that drives `rclone rcd` (or `lsjson`/`copyto`/`deletefile` subprocesses) beneath Jot's own engine. That unlocks WebDAV, SFTP, OneDrive and others for free, and the package only Suggests: rclone.

E2EE (version 2):

- libsodium crypto_secretstream (XChaCha20-Poly1305) per file, with the key from Argon2id (crypto_pwhash) of a passphrase. Store the wrapped key in a remote /.jot/keys file. Optionally encrypt filenames deterministically (SIV-like, via HMAC-derived name plus an encrypted manifest).
- age has no maintained C implementation (only Go and rage in Rust), so prefer libsodium, which is packaged everywhere.
- With E2EE the remote hashes cover ciphertext. Keep plaintext sha256 in the local index and compare by rev/etag only. Server-side previews and Drive search stop working, which is the expected tradeoff.

2. BACKENDS

Shared: write a thin QNetworkAccessManager client per backend, as Callie does. No vendor SDKs, because there is no maintained C++ Dropbox or Drive SDK, and aws-sdk-cpp is huge and badly packaged.

QtNetworkAuth:

- Qt 6.8 added setPkceMethod with S256 as the default, and QOAuthUriSchemeReplyHandler (https://doc.qt.io/qt-6.8/qoauth2authorizationcodeflow.html).
- On Linux, keep using QOAuthHttpServerReplyHandler with a loopback redirect. The URI-scheme handler needs an x-scheme-handler .desktop registration plus single-instance forwarding, which is more moving parts.
- Qt 6.9 adds auto-refresh conveniences (unverified). Debian 13 ships 6.8, so keep Callie's own token provider.
- Tokens go in qtkeychain.

Dropbox API v2:

- Auth: PKCE with token_access_type=offline gives a short-lived access token (about 4 h) and a non-expiring refresh token. No client secret is needed to refresh (https://dropbox.tech/developers/using-oauth-2-0-with-offline-access). Dropbox redirect URIs must match exactly, so register a fixed loopback port (unverified).
- Use an "App folder" (Apps/Jot) app type. It has a smaller blast radius and makes approval simpler. New apps are limited to about 50 users until production approval (unverified).
- Delta: files/list_folder (recursive=true, include_deleted=true) once, then list_folder/continue with the cursor, and list_folder/longpoll for push-like waits.
- content_hash is Dropbox's block hash: SHA-256 over 4 MB block SHA-256s (https://www.dropbox.com/developers/reference/content-hash). Compute it locally to skip transfers.
- Conditional upload: files/upload with mode={".tag":"update","update":<rev>} and autorename=false returns a conflict error if someone else wrote first. Use mode "add" for new files. Use upload sessions for files over 150 MB.

Google Drive v3:

- Scope: drive.file only. drive.file and drive.appdata are "recommended, non-sensitive"; drive and drive.readonly are restricted, need restricted-scope verification, and need a third-party security assessment if data passes through servers (https://developers.google.com/workspace/drive/api/guides/api-specific-auth). That is untenable for an open-source desktop app, so avoid it.
- drive.file is enough because Jot creates the vault folder and every file in it. Caveat: files the user uploads into that folder through the web UI or other apps are invisible to Jot. Document this, and optionally offer the Google Picker to grant them.
- drive.appdata would hide notes from the user. Avoid it.
- Non-sensitive scopes still need brand verification only to show the name and logo. The open-source client secret is acceptable for the "Desktop" client type (Callie already ships this pattern).
- Delta: changes.getStartPageToken, then changes.list(pageToken, spaces=drive, includeRemoved=true) (https://developers.google.com/workspace/drive/api/guides/manage-changes).
- Files expose md5Checksum, plus sha1Checksum and sha256Checksum, and headRevisionId. Put jotPath and the plaintext sha256 in appProperties.
- Drive v3 has no If-Match on files.update. Mitigate by re-reading headRevisionId just before upload, then verifying afterwards. On a mismatch, fetch that revision and merge; Drive keeps revisions as a safety net.
- Drive allows duplicate names in one folder. Treat duplicates as a conflict.

S3-compatible storage:

- Hand-written SigV4 is very feasible: about 300 lines using QCryptographicHash::Sha256 and QMessageAuthenticationCode.
  - Build the canonical request and string-to-sign, derive the signing key, and send x-amz-content-sha256 (hash small bodies, or UNSIGNED-PAYLOAD over TLS).
  - Support path-style and virtual-host addressing, region "auto" for R2, and a configurable endpoint.
  - Test against MinIO in CI.
- Listing: ListObjectsV2 with prefix and continuation-token. Do a full listing each sync, which is cheap for notes, and compare ETags with stored ones.
- ETag is the MD5 only for single-part, non-SSE-KMS uploads. Treat it as opaque. Store the plaintext sha256 in x-amz-meta-jot-sha256, but note that listings do not return metadata.
- Conditional writes on AWS: If-None-Match: \* (Aug 2024) and If-Match: <etag> (Nov 2024) on PutObject, CompleteMultipartUpload and CopyObject (https://docs.aws.amazon.com/AmazonS3/latest/userguide/conditional-writes.html, https://aws.amazon.com/about-aws/whats-new/2024/11/amazon-s3-functionality-conditional-writes).
- R2 documents If-Match and If-None-Match on PutObject, but not on CompleteMultipartUpload (https://developers.cloudflare.com/r2/data-access/s3-api/api/). MinIO supports both.
- Backblaze B2 reportedly returns 501 for If-None-Match on PUT (Proxmox patches: https://lore.proxmox.com/all/20250728100154.556438-1-c.ebner@proxmox.com/T). Wasabi is unknown.
- So probe the capability at setup time. If unsupported, fall back to HEAD-then-PUT and accept the small race; turning on bucket versioning makes it recoverable.
- Avoid multipart for notes; single PUT up to 5 GB.
- C++ library alternatives: minio-cpp (Apache-2, weakly packaged), aws-sdk-cpp (heavy). Hand-written is better.

3. MARKDOWN EDITOR AND RENDERING

What exists:

- QTextDocument::setMarkdown/toMarkdown uses md4c (CommonMark plus GFM tables, task lists and strikethrough). The round trip is lossy: toMarkdown re-serializes and rewrites the user's syntax. It also has no math, no wikilinks and no footnotes. Qt 6.8 added MetaInformation::FrontMatter extraction but no YAML parsing (https://doc.qt.io/qt-6/qtextdocument.html).
- Never let QTextDocument be the source of truth for a plain-files app.
- Marknote (KDE, Kirigami) is a WYSIWYG editor on QTextDocument and added a raw "Source Mode" in 1.5 (https://apps.kde.org/marknote/). It inherits that lossiness.
- KleverNotes switched to md4qt (MIT, header-only, CommonMark 0.31.2 plus GFM, with source positions for every node) (https://apps.kde.org/klevernotes).
- QOwnNotes uses pbek/qmarkdowntextedit (QPlainTextEdit plus a hand-written highlighter) for editing, and md4c-to-HTML in a QTextBrowser for preview (https://github.com/pbek/qmarkdowntextedit).
- Ghostwriter has a hand-written highlighter, with cmark-gfm or other processors to HTML in QtWebEngine for the preview.

Parsers:

- md4c (MIT, packaged): fastest, has MD_FLAG_WIKILINKS and LATEXMATHSPANS, but reports no source offsets.
- cmark-gfm: has sourcepos for blocks, with weak inline positions.
- md4qt: full positions. Best for highlighting and live preview, but it must be vendored.
- Recommendation: md4c for rendering and indexing, and md4qt, or a hand-written block scanner plus inline tokenizer, for the editor highlighter.

Editor recommendation (tier 1, ship first):

- The plain text in a QML TextEdit/TextArea is the source of truth.
- Attach a QSyntaxHighlighter to TextEdit.textDocument (QQuickTextDocument::textDocument(); Qt 6.7 added source/status/save on QQuickTextDocument).
- "Live preview" styling:
  - Scale heading fonts and render bold, italic and code.
  - Dim the syntax markers, and shrink them to near-invisible outside the cursor's block. Re-run the highlighter on the cursor's block when the cursor moves.
  - Render wikilinks as links, with a hover or Ctrl-click handler via positionAt.
  - Highlight fenced code with KSyntaxHighlighting (KF6, MIT, packaged in Debian 13 and Fedora) run inside the same highlighter, using block state for fence tracking.
- Images, rendered math and mermaid: show them as QML overlay items anchored with positionToRectangle below the source line, either in the margin or in a "preview below the block" style.
- Highlighter formats are not in the undo stack, which is good. Do not edit the document text or block formats for display, because that pollutes undo and breaks IME.

Tier 2 (later): a Typora/Obsidian-style block editor. A ListView of blocks, where the focused block is a TextEdit and the rest are rendered.

What is hard:

- Cross-block selection, a unified undo stack, find/replace and IME across blocks.
- QML TextEdit lays out the whole document. Multi-MB notes get slow, so warn above about 1 MB.
- Rehighlight cascades: QSyntaxHighlighter re-runs following blocks until the state settles, and a fence opener re-highlights to the end.
- Skip reformatting while there is preedit (inputMethodComposing) to avoid IME glitches.

Rendering:

- Native reading view: md4c to a RichText subset in QML Text. It is limited (no real CSS), but light.
- KaTeX math: MicroTeX (MIT, C++, has a Qt backend; not distro-packaged, so vendor it) (https://github.com/NanoMichael/MicroTeX).
- Mermaid needs a JS DOM, which means QtWebEngine only.
- QtWebEngine costs about 150-250 MB installed (shared Chromium) and roughly 100 MB or more of RAM per renderer. It is packaged on all targets, though.
- Recommendation: native editor always. Put WebEngine in an optional "jot-preview" subpackage, loaded at runtime, for full-fidelity preview, mermaid and HTML/PDF export.

4. SEARCH

Use SQLite FTS5 in the same DB through Qt Sql. Distro sqlite builds enable FTS5; verify with "PRAGMA compile_options" at startup.

- An FTS5 table (path, title, aliases, tags, body) with tokenize='unicode61 remove_diacritics 2'.
- Use bm25() column weights (title above body), and snippet() and highlight() for results.
- Add a second trigram-tokenizer table (SQLite 3.34+) for substring search and code (https://www.sqlite.org/fts5.html).
- Index incrementally from the watcher with the hash gate. Strip the markdown with md4c text callbacks.

Rejected alternatives:

- Xapian: GPL v2+, which binds your license, and more ops work.
- Tantivy: Rust with no official C API, so a packaging burden.

Quick switcher: an in-memory fzy-style scorer over titles, paths and aliases. fzy's match.c is MIT and tiny, so vendor or port it (https://github.com/jhawthorn/fzy). It takes under 1 ms for 10k notes, with recent-files boosting.

5. FRONT MATTER AND LINKS

YAML parsing:

- yaml-cpp (MIT, packaged everywhere) is simple but does not round-trip comments or formatting.
- rapidyaml (MIT) is fast and keeps source locations. Packaging is uneven; Fedora allows bundling with Provides: bundled(rapidyaml).
- Recommendation: parse with yaml-cpp for reads. When Jot itself changes a key (tags, updated, aliases), patch only that key's lines textually so user formatting survives.
- Treat front matter as only "---" at byte 0 up to the next "---"/"..." line.

Links index, in the same DB:

- links(src, raw, target_path NULL, anchor, line, kind: wikilink/embed/md-link) and tags(path, tag).
- Resolve wikilinks Obsidian-style: exact path, then shortest unique basename, then aliases.
- Backlinks are a simple query, and so are unresolved links.
- On rename, offer to rewrite the inbound links.
- md4c's MD_FLAG_WIKILINKS handles [[a|b]].

6. PACKAGING

Distro Qt versions:

| Distro                  | Qt version | Notes                                                                            |
| ----------------------- | ---------- | -------------------------------------------------------------------------------- |
| Debian 13 (trixie)      | 6.8.2      | Debian testing is at 6.11.x (https://tracker.debian.org/pkg/qt6-base)            |
| Fedora 44               | 6.10.3     | https://linuxsecurity.com/advisories/fedora/qt6-qtbase-fedora-44-2026-70776c2dc3 |
| Ubuntu 26.04 (resolute) | 6.10.2     | https://packages.ubuntu.com/resolute/libqt6core6t64                              |
| Ubuntu 24.04 (noble)    | 6.4.2      | Too old: no PKCE/URI handler in NetworkAuth, no 6.7 QQuickTextDocument API       |

- Set the floor at Qt 6.8, matching Callie. Support Debian 13, Ubuntu 26.04 and Fedora 43/44, and point Ubuntu 24.04 users to Flatpak.
- Packaging method: native debian/ and spec files, as Callie does, rather than CPack. They give proper shlibs deps, clean-chroot builds and Launchpad PPA, COPR or OBS repos. CPack DEB/RPM is fine for throwaway CI artifacts only.
- Build in debian:13, ubuntu:26.04 and fedora:44 containers in CI.
- Dependencies: qt6-base, declarative, networkauth, qtkeychain-qt6, libgit2, libsodium (later), kf6-syntax-highlighting, md4c, yaml-cpp, sqlite. WebEngine goes in the subpackage.
- Flatpak later, on the org.kde.Platform 6.x runtime: WebEngine via io.qt.qtwebengine.BaseApp, the Secret portal for qtkeychain, and --filesystem=home or a chosen folder for the vault. Avoid the document portal for the vault, because inotify on external changes is unreliable through it.

Priority order:

1. Local vault, editor, FTS5 and links.
2. Sync engine with the Local/Syncthing modes.
3. S3 transport, which is the easiest to test with MinIO.
4. Dropbox.
5. Drive (drive.file).
6. rclone transport, E2EE and the WebEngine preview.
