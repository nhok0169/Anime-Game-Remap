/*
 * Force-included (-include) ahead of every source of the vendored Compressonator, curl and the other
 * third-party code core/CMakeLists.txt adds after it, on GCC and Clang. Not part of AGRemapCore.
 *
 * It repairs two things in code this repo cannot patch (the submodules' changes cannot be committed
 * here), without touching it:
 *
 *   1. Compressonator's applications/_plugins/common headers use uintmax_t / uint16_t without including
 *      <cstdint>. MSVC pulls those in transitively, GCC 13+ does not. <stdint.h> rather than <cstdint>
 *      so this stays valid for the C sources too.
 *
 *   2. cmp_core/shaders/common_def.h does '#define __local const' (an OpenCL keyword, for its shaders
 *      compiled as C++). libc++ has a function of that very name -- __segmented_iterator_traits::__local,
 *      used by <algorithm> and specialized by <deque> and <ranges> -- so any libc++ header included
 *      AFTER common_def.h fails with "expected unqualified-id" at '_Traits::__local(...)'. Seen first
 *      with the Xcode 26 SDK on the macOS wheel runners (2026-09-18); libstdc++ has no such name, which
 *      is why Linux builds were fine.
 *      The cure is order: a header's text is macro-expanded when it is first included, and its include
 *      guard makes every later '#include' of it a no-op -- so if libc++'s headers are read here, before
 *      the macro exists, the macro can never reach them. The four headers below are every place libc++
 *      names __local (checked against libc++'s own sources); if a newer libc++ adds a fifth, the error
 *      will name it, and it goes here.
 */

#ifndef AGREMAP_COMPRESSONATOR_PRELUDE_H
#define AGREMAP_COMPRESSONATOR_PRELUDE_H

#include <stdint.h>

#ifdef __cplusplus
// any standard header tells us which library this is; <cstddef> is about the cheapest (<ciso646>,
//   the traditional choice, is deprecated in C++20 and warns under newer libc++)
#include <cstddef>
#ifdef _LIBCPP_VERSION
#include <algorithm>
#include <deque>
#include <iterator>
#if __has_include(<ranges>)
#include <ranges>
#endif
#endif
#endif

#endif
