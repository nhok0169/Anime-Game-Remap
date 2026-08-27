#ifndef AGRemapPyBind_PyFileStats_H
#define AGRemapPyBind_PyFileStats_H

#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include <pybind11/pybind11.h>

#include "AGRemapCore/model/stats/FileStats.h"

// A Python-facing counterpart to AGRemapCore::FileStats -- inherits #fixed/#removed/#undoed/
// #visitedAtRemoval (and their add*/update* methods) directly from the real C++ class, since those
// translate to Python cleanly as-is. SHADOWS 'skipped'/'skippedByMods' (and addSkipped/
// updateSkipped/clear) with new members storing raw Python exception objects (py::object) instead
// of the inherited std::exception_ptr-typed ones.
//
// Why: every real Python call site (e.g. Mod.py's own
// "resourceStats.addSkipped(filePath, exception, modFolder = ...)") populates this from a live,
// already-caught Python exception object -- there is no meaningful way to wrap an arbitrary
// already-caught Python exception as a std::exception_ptr (that type wraps a C++ exception
// in-flight, not an arbitrary foreign-language object handed to you as a value). The inherited
// AGRemapCore::FileStats::skipped/skippedByMods fields stay exception_ptr-typed for a hypothetical
// pure-C++ caller, but the Python-facing surface never reads or writes them -- don't "fix" this by
// deleting these shadow members and rebinding the inherited ones instead.
class PyFileStats: public AGRemapCore::FileStats {
    public:
        std::unordered_map<std::string, pybind11::object> skipped;
        std::unordered_map<std::string, std::unordered_map<std::string, pybind11::object>> skippedByMods;

        virtual ~PyFileStats() = default;

        virtual void clear();

        void addSkipped(const std::string& filePath, pybind11::object error, std::optional<std::string> modFolder = std::nullopt);
        void updateSkipped(const std::unordered_map<std::string, pybind11::object>& newSkipped, std::optional<std::string> modFolder = std::nullopt);

        void update(std::optional<std::string> modFolder = std::nullopt,
                    std::optional<std::unordered_set<std::string>> newFixed = std::nullopt,
                    std::optional<std::unordered_map<std::string, pybind11::object>> newSkipped = std::nullopt,
                    std::optional<std::unordered_set<std::string>> newRemoved = std::nullopt,
                    std::optional<std::unordered_set<std::string>> newUndoed = std::nullopt,
                    std::optional<std::unordered_set<std::string>> newVisitedAtRemoval = std::nullopt);
};

void initCppFileStats(pybind11::module_ &m);

#endif
