#!/bin/bash
# Rebuild the core module on the Linux side (native build tree on ext4, per Building's re-measure)
# and drop the .so into the shared package folder. The Windows .pyd is not touched.
set -u
source ~/agremap-venv/bin/activate
PKG='/mnt/e/Computer/Games/Genshin/Repos/Repos/Fix-Raiden-Boss/Anime Game Remap (for all users)/api/src/py/FixRaidenBoss2'
BUILD=~/cbuildlin-native
echo "== before: $(ls -la --time-style=long-iso "$PKG"/core.cpython-*.so 2>/dev/null)"
echo "== compiler pinned in the tree: $(grep CMAKE_CXX_COMPILER:FILEPATH $BUILD/CMakeCache.txt)"
cd "$BUILD" || exit 1
ninja core 2>&1 | tail -15
STATUS=${PIPESTATUS[0]}
echo "NINJA_EXIT=$STATUS"
[ "$STATUS" -ne 0 ] && exit 1
SO=$(find "$BUILD" -name 'core.cpython-*.so' -newer "$BUILD/CMakeCache.txt" | head -1)
[ -z "$SO" ] && SO=$(find "$BUILD" -name 'core.cpython-*.so' | head -1)
echo "== built: $(ls -la --time-style=long-iso "$SO")"
cp "$SO" "$PKG/" || exit 1
echo "COPY_EXIT=$?"
echo "== after: $(ls -la --time-style=long-iso "$PKG"/core.cpython-*.so)"
