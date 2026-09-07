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

        // Snapshotted here, between building the groups and running any edit over them -- see
        // preEditSectionNames_ for why the post-edit names are the wrong ones to hide by.
        preEditSectionNames_.clear();
        if (graphGroups_ != nullptr) {
            std::size_t groupCount = graphGroups_->size();
            preEditSectionNames_.resize(groupCount);

            for (std::size_t groupInd = 0; groupInd < groupCount; ++groupInd) {
                for (const ModObj& modObj : graphGroups_->modObjs(groupInd)) {
                    Graph* graph = graphGroups_->getGraph(groupInd, modObj);
                    if (graph == nullptr) {
                        continue;
                    }

                    std::vector<std::string>& names = preEditSectionNames_[groupInd][modObj];
                    for (const auto& section : graph->sections()) {
                        names.push_back(section.first);
                    }
                }
            }
        }

        // The mod type being fixed FROM. modTypeName() is RemapIniFixContext's rather than
        // IniFixContext's -- which is what this class's Context typedef is -- so it is asked for
        // rather than assumed. Both real implementations are RemapIniFixContexts; one that is not
        // simply goes unnamed.
        const auto* remapCtx = dynamic_cast<const RemapIniFixContext<K, V, KeyHash, KeyEqual>*>(ctx_);
        const std::optional<std::string> fromModType =
            (remapCtx == nullptr) ? std::nullopt : remapCtx->modTypeName();

        const std::vector<std::string> modsBeingFixedTo = getModsToFix();
        const bool canName = (ctx_ != nullptr) && fromModType.has_value() && !fromModType->empty();

        // No targets at all is a normal state, not an empty run: IniFixBuilder fans one fixer out
        // per target only on its table-driven path, while a fixed-factory builder "has no targets
        // to fan out over" and hands back a single fixer under the empty name -- which leaves
        // 'modsToFix' unset and this list empty. The fix still happens; there is simply no second
        // mod to name, so the line says what it can rather than nothing at all.
        if (canName && modsBeingFixedTo.empty()) {
            ctx_->log("Fixing the .ini file for " + *fromModType);
        }

        for (const std::string& modName : modsBeingFixedTo) {
            // One line per target: a single .ini file is fixed onto several mods, and which one is
            // running is exactly what a reader needs when only one of them goes wrong.
            if (canName) {
                ctx_->log("Fixing the .ini file from " + *fromModType + " to " + modName);
            }

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
    std::string GIMIFixer<K, V, KeyHash, KeyEqual, FixerBase>::labelTargetBlock(const std::string& content) const {
        const std::vector<std::string> targets = getModsToFix();

        // Nothing to name, or nothing to name it over. Both are normal: a fixed-factory builder
        // leaves 'modsToFix' unset (see getFix's own note), and a group can render empty.
        if (targets.empty() || content.empty()) {
            return content;
        }

        // Only the FIRST target names the block. A fixer reaches this with more than one only if
        // something built it that way by hand -- IniFixBuilder::buildAll fans one fixer out per
        // target precisely so each has exactly one, which is what makes this label meaningful.
        const Heading heading(targets.front(), IniBoilerPlate::DefaultModHeadingSideLen,
                               IniBoilerPlate::DefaultModHeadingSideChar);

        return "; " + heading.open() + "\n" + content + "\n\n; " + heading.close();
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

        for (const auto& group : preEditSectionNames_) {
            for (const auto& entry : group) {
                const ModObj& modObj = entry.first;

                if (modObj.first == IniGraphModObjKeywords::Download) {
                    continue;
                }

                // A mod object either half of whose name carries the 'remap' keyword is one this
                // software wrote rather than one the original mod shipped -- see this method's
                // own note.
                if (GIMIFixerDetail::hasRemapKeyword(modObj.first) || GIMIFixerDetail::hasRemapKeyword(modObj.second)) {
                    continue;
                }

                result.insert(entry.second.begin(), entry.second.end());
            }
        }

        return result;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename FixerBase>
    std::unordered_set<std::string> GIMIFixer<K, V, KeyHash, KeyEqual, FixerBase>::hiddenSectionNames() const {
        std::unordered_set<std::string> result;
        if (hiddenModObjs.empty()) {
            return result;
        }

        for (const auto& group : preEditSectionNames_) {
            // Driven by the group's own mod objects rather than by hiddenModObjs, so an object
            // named here that this .ini file does not have is simply never reached -- see this
            // method's own note.
            for (const auto& entry : group) {
                if (hiddenModObjs.find(entry.first) == hiddenModObjs.end()) {
                    continue;
                }

                result.insert(entry.second.begin(), entry.second.end());
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

        // NOT gated on 'fixOnly'. A backup is what --deleteBackup deletes and what the tips
        // tell a user to look for, so every run that rewrites an .ini file owes them one -- the
        // pure-Python script writes a RemapBKUP on a plain fix too. Gating it behind fixOnly meant
        // a normal run silently produced none, while still happily deleting them on request.
        //
        // 'fixOnly' only decides the WORDING: that mode is the one that leaves an existing fixed
        // file in place, so it is the only one with an "old stinky ini" to talk about.
        if (backingUp && ctx_->fixedFileExists()) {
            if (fixOnly) {
                ctx_->log("Cleaning up and disabling the OLD STINKY ini");
            }

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
        // Two independent reasons to hide, unioned into one pass: the user's own hideOrig (which
        // hides everything this fix touched) and this fixer's own hiddenModObjs (which hides the
        // objects its fix replaces rather than adds to). Both are gated on isLastModType for the
        // same reason -- see hiddenModObjs' own note for how the two differ.
        std::unordered_set<std::string> toHide;
        if (fixingCtx.isLastModType) {
            if (hideOrig) {
                toHide = touchedSectionNames();
            }

            for (const std::string& sectionName : hiddenSectionNames()) {
                toHide.insert(sectionName);
            }
        }

        bool hiding = !toHide.empty();

        std::string uncommentedTxt;
        if (hiding) {
            uncommentedTxt = ctx_->fileTxt();
            ctx_->hideOriginalSections(toHide);
        }

        std::string srcTxt = ctx_->fileTxt();

        for (std::size_t i = 0; i < fixTargets_.size(); ++i) {
            std::string content = groupToStr(i);

            // ---- one .ini file, several fixers ----
            //
            // This fixer renders the file's WHOLE new content, and IniFile::fix keeps the last
            // content it is handed for a given path. With one fixer per file that is right; with
            // two it would silently throw the first away. So each fixer contributes its own block
            // to a shared accumulator and renders everything in it, which makes the last render the
            // complete one -- see IniFixingContext::priorFixBlocks for the whole story.
            //
            // Deliberately BEFORE the boilerplate and the source text: those wrap the accumulated
            // blocks once, not once per fixer, so the file gets one credit header rather than one
            // per target.
            const std::string blockKey = fixKey(i, fixTargets_[i]);

            if (fixingCtx.labelTargets) {
                content = labelTargetBlock(content);
            }

            if (fixingCtx.priorFixBlocks != nullptr) {
                std::string& accumulated = (*fixingCtx.priorFixBlocks)[blockKey];

                if (content.empty()) {
                    content = accumulated;
                } else if (!accumulated.empty()) {
                    content = accumulated + "\n\n" + content;
                }

                accumulated = content;
            }

            if (withBoilerPlate) {
                content = ctx_->addFixBoilerPlate(content);
            }

            if (withSrc) {
                // rstrip'd first: the separator below is added on EVERY run, while the removal that
                // precedes it does not take the previous run's trailing whitespace back out. Left
                // alone, each fix appends another blank line to the same .ini file -- small, but it
                // grows without bound and makes two runs of the same fix differ byte for byte.
                content = std::string(StringTools::rstrip(srcTxt)) + "\n\n" + content;
            }

            // Copies only. Index 0 is the mod's own .ini file -- it was already there and needs no
            // note explaining its existence; every later index is a file this fix invented.
            //
            // Outermost, after the boilerplate and the source text, so it is the first thing in the
            // file rather than buried under a section the reader has to scroll past.
            if (i > 0 && !copyPreamble.empty()) {
                content = copyPreamble + "\n\n" + content;
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
