# Changelog

What changed in each version of Jot, for the people using it. Notes collect under Unreleased
as changes land, and `make release` dates them; see [RELEASING.md](RELEASING.md).

## Unreleased

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
- `jot upgrade` and `jot downgrade` install another release, `jot logs` and `jot doctor` help with
  bug reports, and the `jot` command completes in bash, zsh and fish.

## 0.0.2 - 2026-10-14

### New

- Add an install script

## 0.0.1 - 2026-10-14

### New

- A first window with Callie's look: the shared themes, a rounded frame and its own title bar.
- `.deb` and `.rpm` packages for Fedora, Debian and Ubuntu.
