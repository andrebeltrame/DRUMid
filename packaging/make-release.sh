#!/usr/bin/env bash
#
# Builds the release VST3 and assembles dist/.
#
#   ./packaging/make-release.sh [windows .vst3 or .dll]
#
# Produces:
#   dist/DRUMid <version>/DRUMid.vst3   + README.txt + LEIA-ME.txt
#   dist/DRUMid-<version>-macOS-VST3.zip
#
# Pass a Windows binary and it goes into the same bundle, so the one .vst3
# works on both systems. There is no way to produce that binary on a Mac, so it
# comes from the Windows CI job - see .github/workflows/build.yml, which builds
# and merges the whole thing without anyone running this script.

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

VERSION="$(sed -n 's/^project(DRUMid VERSION \([0-9.]*\).*/\1/p' CMakeLists.txt)"

if [[ -z "$VERSION" ]]; then
    echo "could not read the version out of CMakeLists.txt" >&2
    exit 1
fi

echo "DRUMid $VERSION"

# A CMake cache remembers the absolute path it was generated in, so it breaks
# the moment the project folder is renamed or moved. Drop it and reconfigure
# rather than making the person read the error.
if [[ -f build-release/CMakeCache.txt ]] \
   && ! grep -qx "CMAKE_HOME_DIRECTORY:INTERNAL=$ROOT" build-release/CMakeCache.txt; then
    echo "stale build cache from another path - reconfiguring"
    rm -rf build-release
fi

# Never copy a release build into the system plugin folder behind the user's
# back - the release is the thing in dist/, not something silently installed.
cmake -B build-release -DCMAKE_BUILD_TYPE=Release -DDRUMID_COPY_AFTER_BUILD=OFF >/dev/null
cmake --build build-release --config Release --target DRUMid_VST3 -j"$(sysctl -n hw.ncpu)"

VST3="build-release/DRUMid_artefacts/Release/VST3/DRUMid.vst3"

if [[ ! -d "$VST3" ]]; then
    echo "build produced no VST3 at $VST3" >&2
    exit 1
fi

WIN_BINARY="${1:-}"

OUT="dist/DRUMid $VERSION"
rm -rf "$OUT"
mkdir -p "$OUT"

cp -R "$VST3" "$OUT/"
sed "s/@VERSION@/$VERSION/g" packaging/README.txt   > "$OUT/README.txt"
sed "s/@VERSION@/$VERSION/g" packaging/LEIA-ME.txt  > "$OUT/LEIA-ME.txt"

# Ad-hoc signature, so Gatekeeper does not refuse a bundle with no signature at
# all. It is not notarised - the readme tells the user to clear the quarantine
# attribute after unzipping.
if [[ -n "$WIN_BINARY" ]]; then
    # merge-vst3.sh re-seals the bundle itself, which it has to: dropping a file
    # into Contents invalidates whatever signature was there.
    tools/merge-vst3.sh "$OUT/DRUMid.vst3" "$WIN_BINARY" "$OUT"
else
    codesign --force --deep --sign - "$OUT/DRUMid.vst3" 2>/dev/null || \
        echo "note: could not codesign; the bundle ships unsigned"
fi

if [[ -n "$WIN_BINARY" ]]; then
    ZIP="dist/DRUMid-$VERSION-VST3.zip"
else
    ZIP="dist/DRUMid-$VERSION-macOS-VST3.zip"
fi
rm -f "$ZIP"
( cd dist && zip -qr "../$ZIP" "DRUMid $VERSION" )

echo
echo "arch:  $(lipo -info "$OUT/DRUMid.vst3/Contents/MacOS/DRUMid" | sed 's/.*are: //')"
echo "built: $OUT"
echo "zip:   $ZIP  ($(du -h "$ZIP" | cut -f1))"
