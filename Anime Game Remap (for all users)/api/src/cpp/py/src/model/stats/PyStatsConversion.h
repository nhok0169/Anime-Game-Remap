#ifndef AGRemapPyBind_PyStatsConversion_H
#define AGRemapPyBind_PyStatsConversion_H

#include "AGRemapCore/model/stats/CachedFileStats.h"
#include "AGRemapCore/model/stats/FileStats.h"
#include "AGRemapCore/model/stats/RemapStats.h"
#include "PyCachedFileStats.h"
#include "PyFileStats.h"
#include "PyRemapStats.h"

// Shared conversion helpers between the Python-facing PyFileStats/PyCachedFileStats/PyRemapStats
// (py::object-based exception storage) and the plain-C++ AGRemapCore::FileStats/CachedFileStats/
// RemapStats (std::exception_ptr-based) -- needed because every RemapIniResourceMixin-derived
// class's status-query methods (srcEncounteredError/srcIsFixed/fixEncounteredError/fixIsFixed/
// fixExists) and RemapIniDownload's fix()/remapFix() are declared against the plain-C++ types in
// AGRemapCore itself, but a Python caller only ever holds a Py*Stats instance -- the two families
// are unrelated C++ types (Py*Stats intentionally does NOT inherit from AGRemapCore::RemapStats;
// see PyFileStats.h's own note), so pybind11 can't convert between them automatically.
//
// toCppXxxStats() builds a throwaway AGRemapCore::XxxStats snapshot for a read-only status check --
// 'fixed'/'removed'/'undoed'/'visitedAtRemoval'/'hit' copy over directly (same real data, no
// mismatch), while 'skipped'/'skippedByMods' only copy over the KEYS with a placeholder (null)
// std::exception_ptr, since every RemapIniResourceMixin status-query method only ever checks key
// membership (eg. "stats.blend.skipped.contains(srcPath)"), never reads the exception value itself
// -- see PyFileStats.h's own note for why the real exception data can't round-trip through
// std::exception_ptr at all, and why this doesn't matter here.
//
// copyBackXxxStats() copies the OTHER direction, after a mutating call (RemapIniDownload::fix/
// remapFix) -- only 'fixed'/'removed'/'undoed'/'visitedAtRemoval'/'hit' are copied back (the only
// fields any real C++-side fix logic in this codebase ever mutates internally); 'skipped'/
// 'skippedByMods' are deliberately never copied back (nothing in AGRemapCore's own fix logic calls
// addSkipped -- only Python-side callers populate that, directly through PyFileStats's own
// addSkipped, which already writes real exception objects there).

inline AGRemapCore::FileStats toCppFileStats(const PyFileStats &py) {
    AGRemapCore::FileStats result;
    result.fixed = py.fixed;
    result.removed = py.removed;
    result.undoed = py.undoed;
    result.visitedAtRemoval = py.visitedAtRemoval;

    for (const auto &entry : py.skipped) {
        result.skipped[entry.first] = std::exception_ptr();
    }
    for (const auto &modEntry : py.skippedByMods) {
        for (const auto &entry : modEntry.second) {
            result.skippedByMods[modEntry.first][entry.first] = std::exception_ptr();
        }
    }

    return result;
}

inline AGRemapCore::CachedFileStats toCppCachedFileStats(const PyCachedFileStats &py) {
    AGRemapCore::CachedFileStats result;
    static_cast<AGRemapCore::FileStats &>(result) = toCppFileStats(py);
    result.hit = py.hit;
    return result;
}

// The two classes carry exactly the same members again: the eight file kinds
// RemapIniRemover::classifyResource sorts a removed resource into, plus 'ini'. The 'mod' member both
// used to have (a mod-FOLDER fixed/skipped tally, never a file kind) is gone from both sides.

inline AGRemapCore::RemapStats toCppRemapStats(const PyRemapStats &py) {
    AGRemapCore::RemapStats result;
    result.blend = toCppFileStats(py.blend);
    result.position = toCppFileStats(py.position);
    result.texcoord = toCppFileStats(py.texcoord);
    result.buf = toCppFileStats(py.buf);
    result.other = toCppFileStats(py.other);
    result.ini = toCppFileStats(py.ini);
    result.texEdit = toCppFileStats(py.texEdit);
    result.texAdd = toCppFileStats(py.texAdd);
    result.download = toCppCachedFileStats(py.download);
    return result;
}

inline void copyBackFileStats(const AGRemapCore::FileStats &src, PyFileStats &dst) {
    dst.fixed = src.fixed;
    dst.removed = src.removed;
    dst.undoed = src.undoed;
    dst.visitedAtRemoval = src.visitedAtRemoval;
}

inline void copyBackCachedFileStats(const AGRemapCore::CachedFileStats &src, PyCachedFileStats &dst) {
    copyBackFileStats(src, dst);
    dst.hit = src.hit;
}

inline void copyBackRemapStats(const AGRemapCore::RemapStats &src, PyRemapStats &dst) {
    copyBackFileStats(src.blend, dst.blend);
    copyBackFileStats(src.position, dst.position);
    copyBackFileStats(src.texcoord, dst.texcoord);
    copyBackFileStats(src.buf, dst.buf);
    copyBackFileStats(src.other, dst.other);
    copyBackFileStats(src.ini, dst.ini);
    copyBackFileStats(src.texEdit, dst.texEdit);
    copyBackFileStats(src.texAdd, dst.texAdd);
    copyBackCachedFileStats(src.download, dst.download);
}

#endif
