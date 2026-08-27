#ifndef AGRemapPyBind_PyCachedFileStats_H
#define AGRemapPyBind_PyCachedFileStats_H

#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include <pybind11/pybind11.h>

#include "PyFileStats.h"

// Python-facing counterpart to AGRemapCore::CachedFileStats -- see PyFileStats.h's own note for why
// this doesn't just bind AGRemapCore::CachedFileStats directly (its 'skipped'/'skippedByMods' have
// the same std::exception_ptr-vs-Python-exception-object mismatch PyFileStats already works around).
// Inherits from PyFileStats (not AGRemapCore::CachedFileStats) so that fix stays in effect here too.
class PyCachedFileStats: public PyFileStats {
    public:
        std::unordered_set<std::string> hit;

        void clear() override;

        void addHit(const std::string& filePath);
        void updateHit(const std::unordered_set<std::string>& newHit);

        void update(std::optional<std::string> modFolder = std::nullopt,
                    std::optional<std::unordered_set<std::string>> newFixed = std::nullopt,
                    std::optional<std::unordered_map<std::string, pybind11::object>> newSkipped = std::nullopt,
                    std::optional<std::unordered_set<std::string>> newRemoved = std::nullopt,
                    std::optional<std::unordered_set<std::string>> newUndoed = std::nullopt,
                    std::optional<std::unordered_set<std::string>> newVisitedAtRemoval = std::nullopt,
                    std::optional<std::unordered_set<std::string>> newHit = std::nullopt);
};

void initCppCachedFileStats(pybind11::module_ &m);

#endif
