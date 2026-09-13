#!/bin/bash
# Compile and run the standalone core/tests whose oracles the Yelan port changed, on Linux.
set -u
API="/mnt/e/Computer/Games/Genshin/Repos/Repos/Fix-Raiden-Boss/Anime Game Remap (for all users)/api"
CORE="$API/src/cpp/core"
B=~/cbuildlin-native
Z3INC=$(dirname "$(find "$API/cext" "$API/extern" -name "z3.h" 2>/dev/null | head -1)")
Z3LIB=$(find "$API/src/py/FixRaidenBoss2" "$B" "$API/cext" -name "libz3.so*" 2>/dev/null | head -1)
echo "z3 include: $Z3INC"; echo "z3 lib: $Z3LIB"
OUT=~/agremap-tests; mkdir -p "$OUT"
for t in "$@"; do
    echo "=== $t"
    g++-13 -std=c++20 -O1 -w \
        -I "$CORE/include" -I "$API/extern/utf8proc" -I "$API/extern/ordered-map/include" -I "$Z3INC" \
        "$CORE/tests/$t.cpp" -o "$OUT/$t" \
        "$B/src/cpp/core/libAGRemapCore.a" "$B/utf8proc/libutf8proc.a" \
        "$B/Compressonator/cmp_compressonatorlib/libCMP_Compressonator.a" "$B/Compressonator/cmp_framework/libCMP_Framework.a" \
        "$B/Compressonator/cmp_core/libCMP_Core.a" "$B/Compressonator/cmp_core/libCMP_Core_SSE.a" "$B/Compressonator/cmp_core/libCMP_Core_AVX.a" "$B/Compressonator/cmp_core/libCMP_Core_AVX512.a" \
        "$Z3LIB" "$B/curl/lib/libcurl.so" -lpthread -ldl 2>&1 | grep -E "error|undefined" | head -8
    if [ -x "$OUT/$t" ]; then
        LD_LIBRARY_PATH="$(dirname "$Z3LIB"):$B/curl/lib" "$OUT/$t" 2>&1 | tail -6
    else
        echo "  (did not link)"
    fi
done
