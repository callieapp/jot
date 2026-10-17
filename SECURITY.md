# Security policy

## Supported versions

Fixes go into the latest release only. Please check that a problem still happens on the
[latest release](https://github.com/greylocklabs/jot/releases/latest) before reporting it.

## Reporting a vulnerability

Please do not open a public issue. Report it privately through GitHub instead:
[report a vulnerability](https://github.com/greylocklabs/jot/security/advisories/new).

Include what you can of:

- the Jot version and how it was installed (rpm, deb, installer script, source)
- what an attacker could do, and what they need to do it
- steps or a file that reproduces it

Jot is maintained by one person, so replies can take a few days. You will hear back
within a week, and once a fix is released the advisory is published with credit to you,
unless you would rather not be named.

## Scope

In scope: the app, the `jot` command, the packages and the installer script at
jotwriter.org. That includes how Jot reads and writes notes, stores sync credentials,
talks to sync services, imports notes from other apps, and opens links in notes.

Out of scope: problems in Dropbox, Google Drive, S3-compatible services or sync tools
themselves, and in Qt or other libraries, unless Jot uses them unsafely. Please report those
upstream.
