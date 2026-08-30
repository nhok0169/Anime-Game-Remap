#ifndef AGRemapCore_GIMIFixer_TPP
#define AGRemapCore_GIMIFixer_TPP

#include <utility>

#include "GIMIFixer.h"


namespace AGRemapCore {
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
    typename GIMIFixer<K, V, KeyHash, KeyEqual, FixerBase>::FixResult
    GIMIFixer<K, V, KeyHash, KeyEqual, FixerBase>::fixImpl(ParseData& parseData, bool keepBackup, bool fixOnly, bool hideOrig,
                                                            bool withBoilerPlate, bool withSrc) {
        FixResult result;
        if (ctx_ == nullptr) {
            return result;
        }

        // The .ini file's own text, saved before hiding the original sections rewrites it, and put
        // back at the end. The pure-Python original saves 'self._fileTxt' here -- an attribute no
        // fixer ever sets -- see this class's own note.
        std::string uncommentedTxt;
        if (hideOrig) {
            uncommentedTxt = ctx_->fileTxt();
            ctx_->hideOriginalSections();
        }

        if (keepBackup && fixOnly && ctx_->fixedFileExists()) {
            ctx_->log("Cleaning up and disabling the OLD STINKY ini");
            ctx_->disableIni();
        }

        fixTargets_ = getFix(parseData, false);
        fixedContents_.clear();

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

            const std::optional<std::string>& target = fixTargets_[i];
            if (!target.has_value()) {
                // No path to key it by, and nothing to write to -- reachable only for an .ini file
                // that was constructed from raw text. #fixedContents still holds the content.
                continue;
            }

            ctx_->writeFixedFile(*target, content);
            result[*target] = content;
        }

        if (hideOrig) {
            ctx_->setFileTxt(std::move(uncommentedTxt));
        }

        ctx_->setIsFixed(true);
        return result;
    }
}

#endif
