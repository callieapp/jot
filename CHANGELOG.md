# Changelog

What changed in each version of Jot, for the people using it. Notes collect under Unreleased
as changes land, and `make release` dates them; see [RELEASING.md](RELEASING.md).

## Unreleased

### Fixed

- A long note opens at its title, not scrolled part way down.
- `jot upgrade` on a computer without Jot's package names Jot's installer, not Callie's.

## 0.0.4 - 2026-10-15

### New

- Lists carry on as you type: Return starts the next bullet, number or unticked box, Return on an
  empty item ends the list, and Tab and Shift+Tab nest and unnest items. Shift+Return breaks the
  line without a new item.
- Tables: the format bar's table button adds one, or lines up the one you are in. Tab and
  Shift+Tab move between cells, padding them so the columns align; Tab in the last cell and
  Return add a row, and Return on an empty last row ends the table. Tables show in a monospace
  font so the columns line up on screen too.
- Typing `[[` offers your notes as you go, by title, alias or folder. Arrows choose one, Return
  or Tab puts it in and closes the link, and Escape puts the list away. When two notes share a
  title, the link names its folder too.
- Ctrl+/ or the Source button shows a note's Markdown exactly as written, in the monospace font,
  with nothing folded away; again brings the styling back. Jot remembers which you prefer.
- Code blocks are colored by their language, named after the opening fence (` ```python `,
  ` ```rust `, ` ```sh ` and hundreds more), through KDE's syntax highlighting.

## 0.0.3 - 2026-10-15

### New

- Open a folder of Markdown notes as your vault. The sidebar lists its notebooks, which fold, and
  its tags; the middle column lists their notes, newest or by title, and searches them; Ctrl+P
  finds any note by a few letters of its title, an alias or its folder.
- Write notes in Markdown that styles itself as you type: headings grow, and the marks behind
  emphasis, links and code fold away except on the line you are on. Notes save as you type.
  Ctrl+N makes a note, its title renames it and every link to it, Ctrl+click follows a link (or
  makes the note a wikilink names), and a click ticks a task.
- Paste an image, or drop files on a note, to keep them beside it in its `.assets` folder. An image
  on a line of its own shows in place of its link.
- Under each note, the lines that link to it, and the notes that name it without a link. Resting the
  pointer on a link shows the start of the note it leads to.
- To-dos, in the sidebar: notes that are to-dos, soonest due first, and every unticked box in the
  vault. Ticking one changes its note; choosing one opens the note on that line.
- `jot status`, `ls`, `search`, `new`, `show`, `open` and `todo` work on your vault from a terminal,
  with `--json` for scripts; `jot new` takes a note's body from a pipe.
- A row of formatting over each note: headings, bold, italic, strikethrough, code, links, lists,
  checkboxes, quotes and code blocks, each also undone in one step, and most with a shortcut. The
  To-do button makes the note a to-do, with a due date and a Done switch under its title.
- Settings, under the title bar's slider button or Ctrl+,: change the notes folder, pick, import,
  export or recolor a theme, and turn off the daily check for new releases. The ? menu copies debug
  info, reports a bug, lists the keyboard shortcuts, says what is new, and tells you when an update
  is out.
- `jot upgrade` and `jot downgrade` install another release, `jot logs` and `jot doctor` help with
  bug reports, and the `jot` command completes in bash, zsh and fish.

## 0.0.2 - 2026-10-14

### New

- Add an install script

## 0.0.1 - 2026-10-14

### New

- A first window with Callie's look: the shared themes, a rounded frame and its own title bar.
- `.deb` and `.rpm` packages for Fedora, Debian and Ubuntu.
