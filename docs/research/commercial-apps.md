<!-- markdownlint-disable -->

# Commercial note-taking apps

Research snapshot from 2026-10-14, gathered to plan Jot. Prices and versions drift; check sources before relying on them.

Research: commercial note-taking apps, and lessons for Jot (as of 2026-10-14)

Caveat on sources: I confirmed Obsidian's prices on its official page. Most other prices come from aggregator sites that disagree with each other, so treat them as approximate and check vendor pages before quoting them.

== PER-APP ==

OBSIDIAN (local Markdown vault; Electron)

- The vault is a plain folder of .md files. CEO Steph Ango's "file over app" stance is that files outlast apps. https://stephango.com/file-over-app
- Bases (core plugin, v1.9, May 2025) gives database views (table, cards, list, map) over YAML frontmatter properties. Views are saved as .base files or embedded code blocks. Data stays in the Markdown files. https://obsidian.md/help/bases , https://obsidian.md/changelog/2025-05-21-desktop-v1.9.0/
- Canvas uses an open MIT-licensed format, JSON Canvas (nodes are text/file/link/group, plus edges). https://obsidian.md/blog/json-canvas/
- Also has: backlinks, unlinked mentions, graph view, wikilinks, and a huge plugin ecosystem (Dataview, Templater).
- Pricing: the app is free, including commercial use since Feb 2025. Sync is $4/mo yearly or $5 monthly. Publish is $8-10/mo. Optional commercial license $50/yr; Catalyst $25 one-time. https://obsidian.md/pricing
- Complaints:
  - Slow startup and indexing on large vaults, especially on Android (6 minutes reported for 45k files). https://forum.obsidian.md/t/android-large-vault-slow-to-load-on-with-all-plugins-disabled/90828
  - Electron memory use.
  - Users depend on plugins that break.
  - Confusing switches between Live Preview, Source and Reading modes. https://forum.obsidian.md/t/easily-switch-between-source-mode-live-preview-preview/27151
  - Live Preview "jumps" as syntax shows and hides.

NOTION

- Everything is a block. Databases have relations, rollups and many views.
- Templates and a template gallery. Collaborative, web-first.
- Offline mode only arrived in Aug 2025 (v2.53), it is partial, and auto-download needs a paid plan. https://www.notion.com/releases/2025-08-19
- Pricing: Free and Plus (~$10-12/user/mo). Full AI is now bundled mainly into Business (~$20-24). https://www.breeze.pm/articles/notion-ai-pricing
- Complaints:
  - Slow, and needs the cloud.
  - Proprietary block format; Markdown export is lossy.
  - AI upsell pressure.
  - "the app I love to use and hate to depend on."

BEAR (Apple-only)

- Known for typography and theming that make it pleasant to be in.
- Organizes with inline nested tags (#novel/chapter/1) instead of folders, with unlimited depth. https://blog.bear.app/2017/08/bear-tips-organize-notes-with-tags-and-infinite-nested-tags/
- Bear 2 (2023) moved to a CommonMark-based editor with tables, footnotes and nested styles. Markup is hidden until the cursor reaches it. https://blog.bear.app/2023/07/bear-2-is-here/
- Pricing: Pro ~$2.99/mo or $29.99/yr. Sync is paywalled.
- Complaints:
  - No Linux, Windows or web; no collaboration.
  - Tags only, no folders.
  - The Bear 2 migration converted Bear 1 bullets into asterisks.
  - Weak for long-form work: no goals and no way to combine notes into a manuscript. https://www.outlinersoftware.com/topics/viewt/10173

APPLE NOTES

- Free, instant, and good at quick capture: checklists, scanning, sharing, smart folders and tags.
- iOS 26 / macOS Tahoe added Markdown import and export. It still doesn't render Markdown while you type, and links between notes are lost on export. https://appleinsider.com/inside/ios-26/tips/how-to-import-and-export-markdown-with-apple-notes-in-ios-26
- Complaints: Apple-only; poor for structure and long-form writing.

EVERNOTE

- Bending Spoons bought it in 2023. It then raised prices (~$130/yr Personal), limited the free tier to 50 notes and 1 notebook, and laid off staff.
- Revenue rose while downloads fell; recent App Store reviews averaged 1.34/5. https://appfigures.com/resources/insights/the-company-squeezing-money-from-apps-you-used-to-love , https://ftm.eu/articles/wetransfer-owner-buys-up-apps-then-makes-them-more-expensive
- What Jot can learn from it:
  - Web clipping and OCR search were its killer features.
  - Its decline is a cautionary tale about lock-in plus a rent-seeking owner.

ONENOTE

- Freeform canvas pages inside notebooks, sections and pages; strong ink and handwriting.
- Free with a Microsoft account.
- Complaints:
  - Repeated churn of app versions (the Win10 UWP app became read-only Oct 2025). https://www.argentumit.com/blog/the-end-of-onenote-as-you-know-it-part-2
  - OneDrive-only notebooks; very poor export; no Markdown; no Linux.

CRAFT

- Beautiful blocks/cards that can be nested into pages; strong publishing and sharing to the web; native Apple apps.
- Pricing: free Starter (1,500 blocks); Plus ~$8-10/mo; Family and Team bundles. https://aiproductivity.ai/pricing/craft/
- Complaints: block limits on free; proprietary format; growing AI emphasis; Apple-first.

ULYSSES (writer's app, Apple-only)

- A library of "sheets" and groups. You can "glue" sheets into a manuscript.
- Word and character goals with deadlines; Markdown-XL; export styles (PDF, DOCX, ePub); publish to WordPress and Medium.
- Pricing: $5.99/mo, $40-50/yr. https://gappsy.com/tools/ulysses/
- Complaints: the 2017 switch to subscription caused lasting backlash. https://tidbits.com/2017/08/10/ulysses-writing-app-moves-to-subscription-pricing/

iA WRITER

- Focus mode dims everything except the current sentence or paragraph.
- Syntax highlighting colors parts of speech (adjectives, adverbs, nouns, and so on) for style editing.
- Style Check flags filler words, cliches and redundancies.
- Content blocks transclude files, images and tables into a document.
- Typography-first design; wikilinks since v6. https://ia.net/writer/support/basics/features
- Pricing: one-time per platform (~$49.99 Mac, ~$29.99 Windows; no Linux).
- Complaints: paying again per platform; minimal organization.

TYPORA

- The reference for seamless WYSIWYG Markdown: syntax disappears as you type, with no split pane.
- Outline sidebar, themes as plain CSS, Pandoc export, math, diagrams.
- Pricing: $14.99 one-time for 3 devices; runs on Linux. https://toolradar.com/tools/typora/pricing
- Complaints:
  - It edits single files, so it has no note management, sync or links.
  - Some find the hidden syntax disorienting when editing complex Markdown.

SIMPLENOTE (Automattic)

- Extremely fast, plain text with optional Markdown, tags, version history slider, free sync. Free.
- Complaints: barely changes; mostly bug-fix releases (v2.38, 2026); no attachments or structure. https://www.apkmirror.com/apk/automattic-inc/simplenote/

UPNOTE

- Evernote-like notebooks plus tags, a clean UI, fast sync, and Linux support.
- Pricing: $39.99 lifetime or $1.99/mo; free tier capped at 50 notes. https://makerstack.co/reviews/upnote-review/
- Users love the lifetime license.
- Complaints: no E2EE, no plugins, plain design, questions about long-term sustainability.

CAPACITIES (object-based notes; very relevant to Jot)

- Every note is an object with a type (Person, Book, Project, or custom types). Each type has its own property schema, and objects link to each other. https://docs.capacities.io/tutorials/getting-started
- Maps directly to Character, Location, Faction and Item notes for game design.
- Pricing: free tier; Pro ~$10/mo; Pro+ adds AI budget.
- Complaints:
  - Notes live in its own database, not Markdown files. https://docs.capacities.io/misc/offline-support
  - Offline is limited; lock-in.

TANA

- Outliner where each bullet is a node. Supertags turn a node into a typed object with fields. Every node with the same tag becomes a queryable collection with table views. https://tana.inc/articles/intro-to-nodes-fields-and-supertags
- Pricing: now centered on AI meetings. Pro $20-30/user/mo; Free has 50 AI queries. https://tana.inc/pricing
- Complaints:
  - Steep learning curve ("takes days to internalize; many quit in week one"). https://yespress.io/tana
  - Cloud-only; drifting toward an AI meeting product; expensive.

REFLECT

- Minimal daily notes plus backlinks, E2EE, AI built in.
- Pricing: ~$10/mo, no free tier.
- Complaints: paid-only; small feature set; AI conflicts with E2EE. https://costbench.com/software/note-taking/reflect/

MEM

- "AI organizes for you": no folders, automatic resurfacing, agentic chat. Rebuilt as Mem 2.0.
- Pricing: unclear and changing (~$8-20/mo).
- Complaints: the AI-first promise often underdelivers; users don't trust what ends up where; pricing churn. https://tft.digitalpress.blog/review-of-mem-ai/

HEPTABASE

- Cards live in a library and are placed on whiteboards. One card can appear on many boards, and boards nest like folders. Sections for large boards. Aimed at research and sensemaking. https://wiki.heptabase.com/fundamental-elements
- Pricing: ~$9-12/mo.
- Complaints: subscription; past export and lock-in concerns, which the company acknowledged. https://wiki.heptabase.com/newsletters/2023-10-05

DRAFTS (Apple)

- "Text first": opens straight to a blank draft for capture, and you decide where it goes later.
- Scriptable actions send text anywhere.
- Pricing: free, with Pro $1.99/mo or $19.99/yr. https://apps.apple.com/us/app/drafts/id1236254471

NOTESNOOK

- Open-source (GPLv3), E2EE (XChaCha20), self-hostable sync server, cross-platform including Linux.
- Pricing: generous free tier; Essential/Pro from ~$1.99/mo. https://makerstack.co/reviews/notesnook-review/
- Complaints: rich-text storage rather than Markdown files; fewer power features.

== SYNTHESIS ==

TABLE STAKES (2026)

- Instant launch and search, including full-text search with operators.
- Markdown with a good live-preview editor: tables, task lists, code blocks with highlighting, math, footnotes, images and attachments.
- Linking:
  - Wikilinks with autocomplete; rename-safe links.
  - Backlinks panel and unlinked mentions.
- Organization:
  - Folders or notebooks plus nested tags.
  - Pinned/favorite notes, quick switcher, command palette.
- Daily notes and templates (with variables).
- Outline/TOC sidebar; split panes and tabs.
- Data safety:
  - Version history and trash.
  - Import from Joplin, Evernote ENEX, Obsidian, Notion and Bear; export to MD, HTML, PDF and DOCX via Pandoc.
- Platform:
  - Light and dark themes; customizable fonts.
  - Offline-first.
  - Sync via your own folder (Syncthing, Nextcloud, Git, WebDAV), optionally E2EE.
- Mobile capture, or at least a share/quick-capture path.

TOP 15 DIFFERENTIATORS, ranked by value/effort (highest first)

1. Files-as-truth vault: plain .md plus YAML frontmatter, no proprietary database (any index is a rebuildable cache). Low effort, huge trust. This is Obsidian's moat and every cloud app's weakness.
2. Startup and typing speed as a feature. Native QML/C++ can beat Electron by a wide margin. Publish a cold-start number for a 10k-note vault.
3. Typora-quality live preview that does not jump. Reveal syntax only on the cursor line, keep line heights stable, and offer a toggle to source mode.
4. Nested tags in the Bear style, alongside folders.
5. Typed notes (Capacities objects / Tana supertags, file-based): a frontmatter `type: character` applies a template, a property schema and an icon. Good for characters, locations, items and quests.
6. Bases-style views over properties (table, cards, kanban by status), stored as plain files. Turns a folder of characters into a cast sheet.
7. iA-style focus mode and typewriter scrolling; a writing-goal word counter (per session, per document, per project, with deadline).
8. Manuscript assembly in the Ulysses "glue" / Scrivener compile style. An ordered project file lists notes, with drag reorder, combined word count and one-click Pandoc export to DOCX, ePub or PDF. Big draw for professional writers.
9. Transclusion / content blocks (`![[note#section]]`), in the iA and Obsidian style.
10. Hover and inline previews of linked notes. Cheap; essential when worldbuilding.
11. Canvas using the JSON Canvas format: corkboard for plots, quest flows and dialogue trees. Higher effort; using the open format means interop with Obsidian for free.
12. Style highlighting (parts of speech, filler words) as an opt-in plugin. Medium effort with a local NLP library; unique on Linux.
13. Quick-capture global hotkey in the Drafts style: tiny window, lands in the inbox.
14. Snapshot/version timeline per note (Simplenote's slider), backed by git or by internal snapshots.
15. A small, stable, sandboxed plugin/script API (JS or QML). Higher effort; do it after the core. Avoid Obsidian's breakage problem by versioning the API.

Also worth considering: a local graph view (for one note's neighborhood, rather than the whole-vault "hairball"). It is popular, but low on real use value.

EDITOR UX

- Split view (Joplin, old Markdown editors) is widely seen as dated: it wastes width and the eyes jump between panes.
- Full WYSIWYG (Notion, Craft, Apple Notes, Notesnook) is friendliest, but often stores non-Markdown data and fights power users.
- Hybrid live preview is what most users want now:
  - Typora hides syntax completely.
  - Bear 2 hides syntax until the cursor reaches it.
  - Obsidian Live Preview shows syntax on the active line.
  - Obsidian forum consensus: most users stay in Live Preview; a vocal minority prefers Source mode (lower CPU, no jumping). https://forum.obsidian.md/t/just-2-view-modes/51502
- Recommendation for Jot:
  - Default to cursor-line reveal with stable layout.
  - One keystroke toggles raw source.
  - Optional read-only reading view.
  - Never more than two modes visible in the UI.

ORGANIZATION MODELS

- Folders are familiar, but one note can live in only one place.
- Tags (Bear) are flexible but get messy without nesting.
- Links (Obsidian, Reflect) scale best for knowledge, but are weak for browsing.
- Typed objects with properties (Capacities, Tana, Notion databases, Obsidian Bases) are the 2025-26 trend and give the most power for structured creative work. They cost a learning curve, which is Tana's main complaint.
- What wins is layering:
  - Folders for coarse projects ("Game: Ashfall").
  - Links as the main connective tissue.
  - Optional properties and types for structured entities.
- Ango himself has moved to few folders, with category properties and Bases plus heavy linking. https://damoang.net/obsidang/1164
- Key rule: structure should be opt-in and should never be required before writing.

ANTI-PATTERNS TO AVOID

- Forced subscriptions and paywalled basics.
  - Ulysses (2017), Goodnotes and Evernote all drew lasting backlash.
  - Notability brought back a one-time purchase in July 2026, explicitly citing "subscription fatigue." https://www.businesswire.com/news/home/20260714768240/en/
  - Lifetime and one-time licenses (Typora, UpNote, iA Writer) are what users love most.
- AI pushing.
  - Recurring complaints: "drowning in AI features I never asked for" (HN thread). Hard-to-dismiss Gemini prompts. Copilot rebranded but not removable. https://decrypt.co/346170/ai-backlash-sophisticated-users-getting-sick-techs-latest-obsession
  - Notion moved AI up to a pricier tier, and Tana pivoted toward AI meetings. AI is increasingly used as an upsell.
  - This matches the user's reason for leaving Morgen. A credible market position: "no AI unless you install a plugin; no telemetry."
- Lock-in and proprietary storage (Capacities, Notion, Craft, OneNote, Evernote).
  - Owners change: Bending Spoons squeezed Evernote after buying it.
- Slow startup or heavy runtime (Obsidian on large vaults and Android; Notion generally).
- Mode confusion and editor "jumpiness."
- Mandatory setup before value (Tana's 10-15 hours).
- Breaking format migrations (Bear 2's asterisk conversion).
- Freemium caps on note counts (Evernote and UpNote's 50 notes, Craft's block limit). These read as hostile.
- Cloud-only sync with no self-host option. Notesnook's open sync server and Obsidian's "bring your own sync" are what privacy-minded users expect.

== SOURCES (beyond the inline links) ==
https://obsidian.md/pricing
https://help.obsidian.md/bases
https://9to5mac.com/2023/07/11/bear-2-features/
https://www.capacities.io/blog/why-offline-first
https://outliner.tana.inc/docs/supertags
https://tidbits.com/2017/08/10/ulysses-writing-app-moves-to-subscription-pricing/
https://www.plaud.ai/blogs/articles/evernote-alternatives
