#!/usr/bin/env bash
# Merges the macOS binary and the Windows binary into a single .vst3 bundle.
#
#   tools/merge-vst3.sh <mac.vst3> <windows.vst3 or .dll> <output dir>
#
# A VST3 bundle keeps one binary per platform in its own folder
# (Contents/MacOS, Contents/x86_64-win), so the same bundle works on both
# systems: one download, and nobody picks the wrong one.
#
# Runs on macOS, because the bundle has to be re-sealed at the end. Adding files
# inside Contents invalidates the signature, and a bundle with a broken
# signature is refused by the system and skipped by the host in silence - it
# simply never appears in the plugin list.

set -euo pipefail

MAC_BUNDLE="${1:?usage: merge-vst3.sh <mac.vst3> <windows.vst3|dll> <output dir>}"
WIN_BINARY="${2:?missing the Windows binary}"
OUT_DIR="${3:?missing the output directory}"

NAME="$(basename "$MAC_BUNDLE")"          # "DRUMid.vst3"
STEM="${NAME%.vst3}"                      # "DRUMid"
TARGET="$OUT_DIR/$NAME"

# The Windows path may be the whole bundle or just the DLL inside it.
if [[ -d "$WIN_BINARY" ]]; then
    WIN_DLL="$WIN_BINARY/Contents/x86_64-win/$STEM.vst3"
else
    WIN_DLL="$WIN_BINARY"
fi

[[ -f "$WIN_DLL" ]] || { echo "No Windows DLL at $WIN_DLL" >&2; exit 1; }

# A Windows executable starts with "MZ". Without this check, copying the wrong
# file would produce a bundle that only fails on someone else's machine.
if [[ "$(head -c 2 "$WIN_DLL")" != "MZ" ]]; then
    echo "$WIN_DLL does not look like a Windows binary (no MZ header)." >&2
    exit 1
fi

mkdir -p "$OUT_DIR"
rm -rf "$TARGET"
ditto "$MAC_BUNDLE" "$TARGET"

mkdir -p "$TARGET/Contents/x86_64-win"
cp "$WIN_DLL" "$TARGET/Contents/x86_64-win/$STEM.vst3"

# The old seal covers the tree as it was; re-sealing is required, not hygiene.
rm -rf "$TARGET/Contents/_CodeSignature"

if [[ -n "${MACOS_SIGN_IDENTITY:-}" ]]; then
    codesign --force --options runtime --timestamp \
             --sign "$MACOS_SIGN_IDENTITY" "$TARGET"
else
    codesign --force --sign - "$TARGET"
fi

codesign -v --strict "$TARGET"

echo "$NAME"
echo "  macOS   : $(lipo -archs "$TARGET/Contents/MacOS/$STEM")"
echo "  Windows : $(du -h "$TARGET/Contents/x86_64-win/$STEM.vst3" | cut -f1) x86_64"
