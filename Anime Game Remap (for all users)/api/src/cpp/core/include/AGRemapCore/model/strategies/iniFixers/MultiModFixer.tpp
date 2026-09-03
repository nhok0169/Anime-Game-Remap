#ifndef AGRemapCore_MultiModFixer_TPP
#define AGRemapCore_MultiModFixer_TPP

#include <utility>

#include "AGRemapCore/model/files/IniFile.h"


namespace AGRemapCore {

    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename FixerBase>
    MultiModFixer<K, V, KeyHash, KeyEqual, FixerBase>::MultiModFixer(Children children,
                                                                      typename Child::Parser* parser):
        Base(), children_(std::move(children)) {
        // Through the setter rather than the base's constructor: a spliced-in FixerBase (the
        // pybind11 layer's own) may not forward a parser argument at all -- see GIMIFixer's
        // constructor for the same treatment.
        this->setParser(parser);
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename FixerBase>
    const typename MultiModFixer<K, V, KeyHash, KeyEqual, FixerBase>::Children&
    MultiModFixer<K, V, KeyHash, KeyEqual, FixerBase>::getChildren() const {
        return children_;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename FixerBase>
    void MultiModFixer<K, V, KeyHash, KeyEqual, FixerBase>::setChildren(Children children) {
        children_ = std::move(children);
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename FixerBase>
    void MultiModFixer<K, V, KeyHash, KeyEqual, FixerBase>::clear() {
        Base::clear();

        for (auto& entry : children_) {
            if (entry.second != nullptr) {
                entry.second->clear();
            }
        }
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename FixerBase>
    std::vector<int> MultiModFixer<K, V, KeyHash, KeyEqual, FixerBase>::selectedChildIds() const {
        // The filter lives on the .ini file rather than on this fixer, so that one filter governs
        // every fixer running over that file -- including the children of a nested MultiModFixer,
        // which never see this method.
        const IniFile* iniFile = this->getIniFile();
        const std::optional<std::unordered_set<int>>* filter =
            (iniFile != nullptr) ? &iniFile->filteredToModTypeIds : nullptr;

        std::vector<int> result;
        result.reserve(children_.size());

        for (const auto& entry : children_) {
            if (entry.second == nullptr) {
                continue;
            }

            // No .ini file to ask, or no filter set on it, means "run everything" -- the same
            // meaning std::nullopt carries everywhere else this filter is read. An EMPTY set is
            // deliberately different: it selects nothing.
            if (filter != nullptr && filter->has_value() && (*filter)->count(entry.first) == 0) {
                continue;
            }

            result.push_back(entry.first);
        }

        // Left in the order Children yielded them, which is insertion order -- Children is a
        // tsl::ordered_map precisely so the caller decides which child holds the file's first and
        // last word. Sorting here would take that decision away again.
        return result;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename FixerBase>
    typename MultiModFixer<K, V, KeyHash, KeyEqual, FixerBase>::FixResult
    MultiModFixer<K, V, KeyHash, KeyEqual, FixerBase>::fixImpl(ParseData& parseData, bool keepBackup, bool fixOnly,
                                                                bool hideOrig, bool withBoilerPlate, bool withSrc,
                                                                IniFixingContext fixingCtx) {
        // Not propagated -- see this method's own note. Named here so the unused-parameter warning
        // does not hide the fact that it is deliberate.
        (void)withBoilerPlate;
        (void)withSrc;

        FixResult result;

        const std::vector<int> ids = selectedChildIds();
        const std::size_t count = ids.size();

        for (std::size_t i = 0; i < count; ++i) {
            // NARROWED, not replaced: a child is the file's first only if it is this fixer's first
            // AND this fixer was itself told it holds the file's first word. That is what makes
            // nesting compose -- an inner MultiModFixer's children can never both claim the backup
            // and be told they are somewhere in the middle.
            IniFixingContext childCtx(fixingCtx.isFirstModType && i == 0,
                                       fixingCtx.isLastModType && i + 1 == count);

            FixResult childFix = children_.at(ids[i])->fix(parseData, keepBackup, fixOnly, hideOrig, childCtx);

            // A plain overwrite, matching IniFile::fix: two children writing the same path means
            // the later one wins.
            for (auto& entry : childFix) {
                result[entry.first] = std::move(entry.second);
            }
        }

        return result;
    }
}

#endif
