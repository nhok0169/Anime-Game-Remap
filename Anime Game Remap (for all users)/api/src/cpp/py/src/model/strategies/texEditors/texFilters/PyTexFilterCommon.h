#ifndef AGRemapPyBind_PyTexFilterCommon_H
#define AGRemapPyBind_PyTexFilterCommon_H

// ##### Credits

// ===== Anime Game Remap (AG Remap) =====
// Authors: Albert Gold#2696, NK#1321
//
// if you used it to remap your mods pls give credit for "Albert Gold#2696" and "Nhok0169"
// Special Thanks:
//   nguen#2011 (for support)
//   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
//   HazrateGolabi#1364 (for being awesome, and improving the code)

// ##### EndCredits

#include <pybind11/pybind11.h>

#include "AGRemapCore/model/files/TextureFile.h"

/**
 * @brief
 @rst
 Every ``texFilters`` C++ filter operates on :cpp:class:`AGRemapCore::TextureFile`'s native RGBA8
 buffer, not `Pillow`_ -- but ``texFile.img`` (a real Pillow ``Image``) is the single source of
 truth shared across a whole :class:`TexEditor` filter chain (since the other, still-unported
 filters only ever touch ``.img``). These two helpers are the shared pull/push seam every ported
 filter's binding uses to stay interoperable with that chain: pull the current ``.img`` pixels into
 the core buffer immediately before running the real C++ transform, then push the (possibly edited)
 buffer back out to a fresh ``.img`` immediately after
 @endrst
 */

/**
 * @brief Pulls 'texFileObj.img's pixels into the core TextureFile's buffer
 *
 * @param texFileObj A Python ``TextureFile`` instance (or subclass) with a non-``None`` ``img``
 *
 * @return A reference to the underlying AGRemapCore::TextureFile, now holding 'img's pixels
 */
AGRemapCore::TextureFile& syncTextureFileFromImg(pybind11::object texFileObj);

/**
 * @brief Rebuilds 'texFileObj.img' from the core TextureFile's current buffer
 *
 * @param texFileObj A Python ``TextureFile`` instance (or subclass)
 */
void syncTextureFileToImg(pybind11::object texFileObj);

#endif
