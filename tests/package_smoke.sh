#!/bin/sh
# Runs an installed Jot as a user would, to catch files and dependencies a
# package leaves out. Writes the app's window to the folder given.
set -eu

shots="${1:?usage: package_smoke.sh <folder for the screenshot>}"
home="$(mktemp -d)"
export HOME="$home" XDG_CONFIG_HOME="$home/config" XDG_DATA_HOME="$home/data" \
    XDG_CACHE_HOME="$home/cache" XDG_STATE_HOME="$home/state"

jot --version
jot doctor | grep -q "^Jot "
for file in /usr/share/bash-completion/completions/jot \
    /usr/share/zsh/site-functions/_jot \
    /usr/share/fish/vendor_completions.d/jot.fish \
    /usr/share/applications/org.callieapp.Jot.desktop \
    /usr/share/metainfo/org.callieapp.Jot.metainfo.xml \
    /usr/share/icons/hicolor/scalable/apps/org.callieapp.Jot.svg \
    /usr/share/icons/hicolor/16x16/apps/org.callieapp.Jot.png \
    /usr/share/icons/hicolor/32x32/apps/org.callieapp.Jot.png \
    /usr/share/jot/themes/callie.toml; do
    test -s "$file" || { echo "missing $file" >&2; exit 1; }
done

mkdir -p "$shots"
for theme in callie callie-light; do
    QT_QPA_PLATFORM=offscreen jot-gui --theme "$theme" --size 1280x840 \
        --screenshot "$shots/installed-$theme.png"
    test -s "$shots/installed-$theme.png"
done
# With a vault open too, from the source tree beside this script.
vault="$home/vault"
cp -r "$(dirname "$0")/data/sample-vault" "$vault"
QT_QPA_PLATFORM=offscreen jot-gui --vault "$vault" --note "Ashfall/Characters/Mara Vey.md" \
    --size 1280x840 --screenshot "$shots/installed-vault.png"
test -s "$shots/installed-vault.png"
# The command line reads the same vault.
jot status --vault "$vault" | grep -q "notes in"
jot search ferry --vault "$vault" | grep -q "Mara Vey"
echo "the installed app runs"
