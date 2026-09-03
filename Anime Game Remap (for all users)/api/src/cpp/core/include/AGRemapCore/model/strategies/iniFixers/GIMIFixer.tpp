#ifndef AGRemapCore_GIMIFixer_TPP
#define AGRemapCore_GIMIFixer_TPP

#include <string_view>
#include <utility>

#include "GIMIFixer.h"
#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/tools/StringTools.h"


namespace AGRemapCore {
    namespace GIMIFixerDetail {
        // Whether 'txt' names something this software already wrote. Matched anywhere in the name
        // rather than as a suffix, and case-insensitively (by grapheme, through StringTools, so a
        // non-ASCII mod object name lowers correctly too) -- the same convention
        // GIMIParser::classifyByTextureOverrideName uses to refuse to classify its own output.
        inline bool hasRemapKeyword(const std::string& txt) {
            return StringTools::toLower(txt).find(StringTools::toLower(IniKeywords::Remap)) != std::string::npos;
        }
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename FixerBase>
    GIMIFixer<K, V, KeyHash, KeyEqual, FixerBase>::GIMIFixer(typename Core::Parser* parser, Context* ctx,
                                                              std::vector<GroupEdit*> graphGroupEdits,
                                                              std::optional<std::vector<std::string>> modsToFix,
                                                              GIMIFixer<K, V, KeyHash, KeyEqual, FixerBase>* prevFixer,
                                                              FixerConfig config):
        Base(), graphGroupEdits(std::move(graphGroupEdits)), modsToFix(std::move(modsToFix)),
        prevFixer(prevFixer), ctx_(ctx), config_(std::move(config)) {
        // Not forwarded through Base's own constructor: a FixerBase supplied by the pybind11 layer
        // takes a py::object there, and 'nullptr' would quietly become a null py::object rather
        // than an unbound parser. setParser does the same work either way.
        this->setParser(parser);
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename FixerBase>
    typename GIMIFixer<K, V, KeyHash, KeyEqual, FixerBase>::Context* GIMIFixer<K, V, KeyHash, KeyEqual, FixerBase>::ctx() const {
        return ctx_;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename FixerBase>
    void GIMIFixer<K, V, KeyHash, KeyEqual, FixerBase>::setCtx(Context* ctx) {
        ctx_ = ctx;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename FixerBase>
    typename GIMIFixer<K, V, KeyHash, KeyEqual, FixerBase>::GraphGroups*
    GIMIFixer<K, V, KeyHash, KeyEqual, FixerBase>::graphGroups() const {
        return graphGroups_.get();
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename FixerBase>
    const typename GIMIFixer<K, V, KeyHash, KeyEqual, FixerBase>::FixerConfig&
    GIMIFixer<K, V, KeyHash, KeyEqual, FixerBase>::config() const {
        return config_;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename FixerBase>
    const typename GIMIFixer<K, V, KeyHash, KeyEqual, FixerBase>::FixTargets&
    GIMIFixer<K, V, KeyHash, KeyEqual, FixerBase>::fixTargets() const {
        return fixTargets_;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename FixerBase>
    const std::vector<std::string>& GIMIFixer<K, V, KeyHash, KeyEqual, FixerBase>::fixedContents() const {
        return fixedContents_;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename FixerBase>
    std::vector<std::string> GIMIFixer<K, V, KeyHash, KeyEqual, FixerBase>::getModsToFix() const {
        if (modsToFix.has_value()) {
            return *modsToFix;
        }

        if (ctx_ == nullptr) {
            return {};
        }

        return ctx_->modsToFix();
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename FixerBase>
    void GIMIFixer<K, V, KeyHash, KeyEqual, FixerBase>::clear() {
        Base::clear();
        graphGroups_.reset();
        fixTargets_.clear();
        fixedContents_.clear();
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename FixerBase>
    void GIMIFixer<K, V, KeyHash, KeyEqual, FixerBase>::applyGraphGroupEdits(const std::string& modName) {
        if (graphGroups_ == nullptr) {
            return;
        }

        for (GroupEdit* edit : graphGroupEdits) {
            if (edit != nullptr) {
                // nullptr for both collaborators -- see this method's own note.
                edit->editFromIni(*graphGroups_, nullptr, nullptr, modName);
            }
        }
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename FixerBase>
    typename GIMIFixer<K, V, KeyHash, KeyEqual, FixerBase>::FixTargets
    GIMIFixer<K, V, KeyHash, KeyEqual, FixerBase>::getFix(ParseData& parseData, bool onlyEditObjGraphs) {
        if (ctx_ == nullptr) {
            return {};
        }

        if (prevFixer != nullptr) {
            // The previous fixer has already done its own edit pass; take its groups over outright
            // rather than rebuilding from the parse data, then leave it empty. Same handover the
            // pure-Python original does, just as a pointer move instead of a shared list.
            prevFixer->getFix(parseData, true);
            graphGroups_ = std::move(prevFixer->graphGroups_);
            prevFixer->clear();
        } else {
            graphGroups_ = ctx_->makeGraphGroups();
            graphGroups_->insertGroup(0);

            for (GraphGroup& srcGroup : parseData) {
                for (const ModObj& modObj : srcGroup.modObjs()) {
                    Graph* srcGraph = srcGroup.getGraph(modObj);
                    if (srcGraph == nullptr) {
                        continue;
                    }

                    // Every graph is copied, not just the command ones -- see this class's own note.
                    graphGroups_->addGraph(0, modObj, graphGroups_->deepcopyGraph(*srcGraph));
                }
            }
        }

        for (const std::string& modName : getModsToFix()) {
            applyGraphGroupEdits(modName);
        }

        if (onlyEditObjGraphs) {
            return {};
        }

        FixTargets result;
        std::size_t groupCount = (graphGroups_ == nullptr) ? 0 : graphGroups_->size();
        for (std::size_t i = 0; i < groupCount; ++i) {
            result.push_back(ctx_->fixedFilePath(i));
        }

        return result;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename FixerBase>
    std::string GIMIFixer<K, V, KeyHash, KeyEqual, FixerBase>::fixKey(
            std::size_t groupInd, const std::optional<std::string>& fixedFilePath) const {
        if (fixedFilePath.has_value()) {
            return *fixedFilePath;
        }

        // No file to write to -- an .ini file built from raw text. The fix still exists and still
        // needs a key, so fall back to the group index, which is unique per group.
        return std::to_string(groupInd);
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename FixerBase>
    std::string GIMIFixer<K, V, KeyHash, KeyEqual, FixerBase>::groupToStr(std::size_t groupInd) const {
        std::string result;
        if (graphGroups_ == nullptr || !config_.sectionToStr) {
            return result;
        }

        bool first = true;
        for (const ModObj& modObj : graphGroups_->modObjs(groupInd)) {
            Graph* graph = graphGroups_->getGraph(groupInd, modObj);
            if (graph == nullptr) {
                continue;
            }

            std::string current = graph->toStr(config_.sectionToStr, true);
            if (current.empty()) {
                continue;
            }

            if (!first) {
                result += "\n\n";
            }

            result += current;
            first = false;
        }

        return result;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename FixerBase>
    std::unordered_set<std::string> GIMIFixer<K, V, KeyHash, KeyEqual, FixerBase>::touchedSectionNames() const {
        std::unordered_set<std::string> result;
        if (graphGroups_ == nullptr) {
            return result;
        }

        std::size_t groupCount = graphGroups_->size();

        for (std::size_t groupInd = 0; groupInd < groupCount; ++groupInd) {
            for (const ModObj& modObj : graphGroups_->modObjs(groupInd)) {
                if (modObj.first == IniGraphModObjKeywords::Download) {
                    continue;
                }

                // A mod object either half of whose name carries the 'remap' keyword is one this
                // software wrote rather than one the original mod shipped -- see this method's
                // own note.
                if (GIMIFixerDetail::hasRemapKeyword(modObj.first) || GIMIFixerDetail::hasRemapKeyword(modObj.second)) {
                    continue;
                }

                Graph* graph = graphGroups_->getGraph(groupInd, modObj);
                if (graph == nullptr) {
                    continue;
                }

                for (const auto& section : graph->sections()) {
                    result.insert(section.first);
                }
            }
        }

        return result;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename FixerBase>
    typename GIMIFixer<K, V, KeyHash, KeyEqual, FixerBase>::FixResult
    GIMIFixer<K, V, KeyHash, KeyEqual, FixerBase>::fixImpl(ParseData& parseData, bool keepBackup, bool fixOnly, bool hideOrig,
                                                            bool withBoilerPlate, bool withSrc, IniFixingContext fixingCtx) {
        FixResult result;
        if (ctx_ == nullptr) {
            return result;
        }

        // Only the first mod type's fixers take the backup, for the mirror image of the reason only
        // the last one hides: disabling the existing .ini file moves it aside on disk, and a later
        // pass doing it again would be backing up a file the first pass already moved. The
        // pure-Python original never had to say so -- one .ini file, one fixer, one pass -- which is
        // why IniFixingContext::isFirstModType defaults to true.
        bool backingUp = keepBackup && fixingCtx.isFirstModType;

        if (backingUp && fixOnly && ctx_->fixedFileExists()) {
            ctx_->log("Cleaning up and disabling the OLD STINKY ini");
            ctx_->disableIni();
        }

        fixTargets_ = getFix(parseData, false);
        fixedContents_.clear();

        // Hiding comes *after* the fix is built, not before: which sections to comment out is
        // #touchedSectionNames, and there is nothing to read that off until the groups exist. The
        // pure-Python original orders it the same way for the same reason -- its own fixer fills
        // the .ini file's '_remappedSectionNames' while rendering, and only then does
        // 'ini.hideOriginalSections()' run over whatever landed in there.
        //
        // The .ini file's own text is saved first and put back at the end, so hiding only ever
        // affects the copy that goes into the fix.
        //
        // Only the last mod type's fixers do it, too. Several fixers chain over one .ini file -- one
        // per mod type it was classified as, and one per target mod each of those fixes to -- and
        // hiding rewrites the *file's* text rather than only adding to this fixer's own output. Doing
        // it on every pass would have each one hide the original for a fix that a later pass then
        // overwrites; see IniFixingContext::isLastModType.
        bool hiding = hideOrig && fixingCtx.isLastModType;

        std::string uncommentedTxt;
        if (hiding) {
            uncommentedTxt = ctx_->fileTxt();
            ctx_->hideOriginalSections(touchedSectionNames());
        }

        std::string srcTxt = ctx_->fileTxt();

        for (std::size_t i = 0; i < fixTargets_.size(); ++i) {
            std::string content = groupToStr(i);

            if (withBoilerPlate) {
                content = ctx_->addFixBoilerPlate(content);
            }

            if (withSrc) {
                content = srcTxt + "\n\n" + content;
            }

            fixedContents_.push_back(content);

            // Two separate questions, and only the first can be unanswerable: an .ini file built
            // from raw text has nowhere to write, but its fix still belongs in the result.
            const std::optional<std::string>& target = fixTargets_[i];
            if (target.has_value()) {
                ctx_->writeFixedFile(*target, content);
            }

            result[fixKey(i, target)] = content;
        }

        if (hiding) {
            ctx_->setFileTxt(std::move(uncommentedTxt));
        }

        ctx_->setIsFixed(true);
        return result;
    }
}

#endif
