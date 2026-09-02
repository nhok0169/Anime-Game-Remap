#ifndef AGRemapCore_RemapIniRemover_TPP
#define AGRemapCore_RemapIniRemover_TPP

#include <cctype>
#include <deque>
#include <filesystem>
#include <type_traits>
#include <utility>

#include "RemapIniRemover.h"
// Both of these need the whole IniFile, not the forward declaration RemapIniRemover.h carries:
// scanFile() cuts its section spans on IniFile::isSectionHeaderLine/getSectionNameFromLine (see
// those methods' own note on why they are reused rather than re-derived), and setIniFile()
// builds an IniFileRemoveContext. No cycle: IniFile.h reaches BaseIniRemover.h (through
// ModType.h -> IniRemoveBuilder.h) but never this header.
#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/model/strategies/iniRemovers/IniFileRemoveContext.h"
#include "AGRemapCore/tools/StringTools.h"


namespace AGRemapCore {
    namespace RemapIniRemoverDetail {
        // 'line' without its trailing newline (a context's readFileLines keeps them attached) or any
        // other trailing whitespace, and without leading whitespace. The heading lines this is used
        // on are written by this software itself and never indented, but a hand-edited .ini file is
        // free to have picked up either.
        inline std::string trimLine(const std::string& line) {
            return std::string(StringTools::strip(line));
        }

        // 'txt' repeated 'count' times.
        inline std::string repeat(const std::string& txt, std::size_t count) {
            std::string result;
            result.reserve(txt.size() * count);

            for (std::size_t i = 0; i < count; ++i) {
                result += txt;
            }

            return result;
        }
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename RemoverBase>
    typename RemapIniRemover<K, V, KeyHash, KeyEqual, RemoverBase>::RemoverConfig
    RemapIniRemover<K, V, KeyHash, KeyEqual, RemoverBase>::defaultConfig() {
        RemoverConfig result{};

        // Same constraint GIMISectionClassifier::defaultConfig has: a K/V that no .ini keyword can
        // be spelled as gets nothing here, and that instantiation supplies its own config.
        if constexpr (std::is_constructible_v<K, const std::string&> && std::is_constructible_v<V, const std::string&>) {
            result.hashKey = K(IniKeywords::Hash);
            result.filenameKey = K(IniKeywords::Filename);
            result.runConfig = IfTemplateRunConfig<K, V>{
                K(IniKeywords::Run),
                [](const V& value) { return std::string(value); },
                [](const std::string& name) { return V(name); }
            };
        }

        return result;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename RemoverBase>
    const std::vector<typename RemapIniRemover<K, V, KeyHash, KeyEqual, RemoverBase>::BoilerPlateHeading>&
    RemapIniRemover<K, V, KeyHash, KeyEqual, RemoverBase>::defaultHeadings() {
        // The two arms of the pure-Python original's _fixRemovalPattern, in the same order. Only the
        // ".*<suffix>" tails are kept: matching a title against a real regular expression is not
        // something the core has any machinery for (and deliberately so -- see AGRemapCore's own
        // zero-dependency rule), and "ends with" is what both of those two patterns actually ask.
        static const std::vector<BoilerPlateHeading> headings = {
            {"Boss Fix", IniBoilerPlate::DefaultHeadingSideLen, IniBoilerPlate::DefaultHeadingSideChar},
            {"Remap", IniBoilerPlate::DefaultHeadingSideLen, IniBoilerPlate::DefaultHeadingSideChar}
        };
        return headings;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename RemoverBase>
    std::function<std::shared_ptr<BaseIniRemover<>>(IniFile*)> RemapIniRemover<K, V, KeyHash, KeyEqual, RemoverBase>::factory() {
        return [](IniFile* iniFile) -> std::shared_ptr<BaseIniRemover<>> {
            // setIniFile is what builds the IniFileRemoveContext -- see its own doc.
            std::shared_ptr<RemapIniRemover<>> result = std::make_shared<RemapIniRemover<>>();
            result->setIniFile(iniFile);
            return result;
        };
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename RemoverBase>
    RemapIniRemover<K, V, KeyHash, KeyEqual, RemoverBase>::RemapIniRemover(Context* ctx, RemoverConfig config):
        RemoverBase(), config_(std::move(config)), ctx_(ctx) {}


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename RemoverBase>
    const typename RemapIniRemover<K, V, KeyHash, KeyEqual, RemoverBase>::RemoverConfig&
    RemapIniRemover<K, V, KeyHash, KeyEqual, RemoverBase>::config() const {
        return config_;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename RemoverBase>
    typename RemapIniRemover<K, V, KeyHash, KeyEqual, RemoverBase>::Context*
    RemapIniRemover<K, V, KeyHash, KeyEqual, RemoverBase>::getContext() const {
        return ctx_;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename RemoverBase>
    void RemapIniRemover<K, V, KeyHash, KeyEqual, RemoverBase>::setContext(Context* ctx) {
        // Whatever this remover built for itself is dropped: an explicitly-supplied context always
        // wins, and keeping the old one alive would leave a dangling IniFile* behind it.
        ownedCtx_.reset();
        ctx_ = ctx;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename RemoverBase>
    void RemapIniRemover<K, V, KeyHash, KeyEqual, RemoverBase>::setIniFile(IniFile* iniFile) {
        RemoverBase::setIniFile(iniFile);

        // An externally-supplied context is never replaced -- only one this remover built itself is
        // rebuilt. That is what lets IniRemoveBuilder::build bind the remover it just constructed,
        // while a caller driving a remover through setContext keeps control.
        if (ctx_ != nullptr && ownedCtx_ == nullptr) {
            return;
        }

        if constexpr (std::is_same_v<K, std::string> && std::is_same_v<V, std::string>) {
            if (iniFile == nullptr) {
                ownedCtx_.reset();
                ctx_ = nullptr;
                return;
            }

            ownedCtx_ = std::make_unique<IniFileRemoveContext>(iniFile);
            ctx_ = ownedCtx_.get();
        }
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename RemoverBase>
    const std::vector<std::string>& RemapIniRemover<K, V, KeyHash, KeyEqual, RemoverBase>::getTargetSectionNames() const {
        return targetSectionNames_;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename RemoverBase>
    const std::vector<std::string>& RemapIniRemover<K, V, KeyHash, KeyEqual, RemoverBase>::getRemovedSectionNames() const {
        return removedSectionNames_;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename RemoverBase>
    typename RemapIniRemover<K, V, KeyHash, KeyEqual, RemoverBase>::Graph*
    RemapIniRemover<K, V, KeyHash, KeyEqual, RemoverBase>::getRemovalGraph() const {
        return removalGraph_.get();
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename RemoverBase>
    const std::unordered_map<std::string, std::vector<std::unique_ptr<IniResource>>>&
    RemapIniRemover<K, V, KeyHash, KeyEqual, RemoverBase>::getRemovedResources() const {
        return removedResources_;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename RemoverBase>
    std::string RemapIniRemover<K, V, KeyHash, KeyEqual, RemoverBase>::removeAll(const std::string& txt, const std::string& target) {
        if (target.empty()) {
            return txt;
        }

        std::string result;
        result.reserve(txt.size());
        std::size_t searchInd = 0;

        while (true) {
            std::size_t foundInd = txt.find(target, searchInd);

            if (foundInd == std::string::npos) {
                result.append(txt, searchInd, std::string::npos);
                return result;
            }

            result.append(txt, searchInd, foundInd - searchInd);
            searchInd = foundInd + target.size();
        }
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename RemoverBase>
    std::string RemapIniRemover<K, V, KeyHash, KeyEqual, RemoverBase>::valToStr(const V& value) const {
        if (!config_.runConfig.sectionNameOf) {
            return "";
        }

        return config_.runConfig.sectionNameOf(value);
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename RemoverBase>
    std::optional<typename RemapIniRemover<K, V, KeyHash, KeyEqual, RemoverBase>::OpenMatch>
    RemapIniRemover<K, V, KeyHash, KeyEqual, RemoverBase>::matchBoilerPlateOpen(const std::string& line) const {
        std::string trimmed = RemapIniRemoverDetail::trimLine(line);

        for (const BoilerPlateHeading& heading : headings) {
            if (heading.sideChar.empty()) {
                continue;
            }

            std::string side = RemapIniRemoverDetail::repeat(heading.sideChar, heading.sideLen);
            std::string prefix = "; " + side + " ";
            std::string suffix = " " + side;

            // The two borders can't be allowed to overlap -- "; --- ---" would otherwise read as an
            // opening line whose title is the empty string.
            if (trimmed.size() < prefix.size() + suffix.size()) {
                continue;
            }

            if (trimmed.compare(0, prefix.size(), prefix) != 0) {
                continue;
            }

            // Deliberately a *prefix* match rather than a whole-line one, and deliberately greedy,
            // because that is what the pure-Python original's own pattern is. Its
            // "; <side> .*<suffix> <side>" is neither anchored at the end nor applied per line, so
            // a real heading like
            //
            //     ; --------------- Raiden Boss Fix -----------------
            //
            // (which the software really did write -- 15 sideChars on the left, 17 on the right)
            // still matches it: the regex stops after the 15th, and everything past that is just
            // the start of the region's body. Requiring the line to *end* on the border would
            // reject exactly the headings this has to find.
            std::string title;
            bool found = false;

            for (std::size_t titleEnd = trimmed.size() - suffix.size(); titleEnd + 1 > prefix.size(); --titleEnd) {
                if (trimmed.compare(titleEnd, suffix.size(), suffix) != 0) {
                    continue;
                }

                std::string candidate = trimmed.substr(prefix.size(), titleEnd - prefix.size());

                // What ".*<titleSuffix>" asks. An empty titleSuffix accepts any title, which is
                // what a bare ".*" would do too.
                if (candidate.size() < heading.titleSuffix.size() ||
                        candidate.compare(candidate.size() - heading.titleSuffix.size(),
                                          heading.titleSuffix.size(), heading.titleSuffix) != 0) {
                    continue;
                }

                // Greedy, so the first hit walking backwards from the end of the line wins.
                title = std::move(candidate);
                found = true;
                break;
            }

            if (!found) {
                continue;
            }

            // Heading::close()'s own width, in sideChars, less the 2 the original slices off it
            // before appending "(-)*" -- so a closing line drawn for a *longer* title than this one
            // still matches, exactly as that pattern intends.
            std::size_t closeLen = 2 * (heading.sideLen + 1) + title.size();
            std::size_t minSideChars = closeLen > 2 ? closeLen - 2 : 1;

            return OpenMatch{heading.sideChar, minSideChars};
        }

        return std::nullopt;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename RemoverBase>
    bool RemapIniRemover<K, V, KeyHash, KeyEqual, RemoverBase>::isBoilerPlateClose(const std::string& line, const OpenMatch& open) {
        const std::string& sideChar = open.sideChar;
        if (sideChar.empty()) {
            return false;
        }

        std::string trimmed = RemapIniRemoverDetail::trimLine(line);
        const std::string prefix = "; ";

        if (trimmed.compare(0, prefix.size(), prefix) != 0) {
            return false;
        }

        std::string border = trimmed.substr(prefix.size());
        if (border.empty() || border.size() % sideChar.size() != 0) {
            return false;
        }

        std::size_t sideCharCount = border.size() / sideChar.size();
        if (sideCharCount < open.minSideChars) {
            return false;
        }

        for (std::size_t i = 0; i < border.size(); i += sideChar.size()) {
            if (border.compare(i, sideChar.size(), sideChar) != 0) {
                return false;
            }
        }

        return true;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename RemoverBase>
    typename RemapIniRemover<K, V, KeyHash, KeyEqual, RemoverBase>::FileScan
    RemapIniRemover<K, V, KeyHash, KeyEqual, RemoverBase>::scanFile(const std::vector<std::string>& lines) const {
        FileScan result;

        std::size_t currentBoilerPlate = std::string::npos;
        OpenMatch currentOpen;

        bool hasSection = false;
        SectionSpan currentSection;

        // Ends the section occurrence currently being accumulated, if there is one, at 'endInd'.
        auto closeSection = [&result, &hasSection, &currentSection](std::size_t endInd) {
            if (!hasSection) {
                return;
            }

            currentSection.endInd = endInd;
            result.sections.push_back(currentSection);
            hasSection = false;
        };

        std::size_t linesLen = lines.size();
        for (std::size_t i = 0; i < linesLen; ++i) {
            const std::string& line = lines[i];

            // A boilerplate region can't nest, and an opening line while one is already open simply
            // starts the next -- the pure-Python original's own non-greedy "(.|\n)*?" behaves the
            // same way, since it can only ever match up to the first closing line.
            std::optional<OpenMatch> open = matchBoilerPlateOpen(line);
            if (open.has_value()) {
                closeSection(i);
                result.boilerPlates.push_back(BoilerPlateSpan{i, std::string::npos});  // closeInd filled in when the closing line is found
                currentBoilerPlate = result.boilerPlates.size() - 1;
                currentOpen = *open;
                continue;
            }

            if (currentBoilerPlate != std::string::npos && isBoilerPlateClose(line, currentOpen)) {
                closeSection(i);
                result.boilerPlates[currentBoilerPlate].closeInd = i;
                currentBoilerPlate = std::string::npos;
                continue;
            }

            // The same boundary IniFile::getIfTemplates cuts its own sections on -- see
            // IniFile::isSectionHeaderLine's note on why this reuses it rather than re-deriving it.
            if (IniFile::isSectionHeaderLine(line)) {
                closeSection(i);

                currentSection = SectionSpan{IniFile::getSectionNameFromLine(line), i, i, currentBoilerPlate};
                hasSection = true;
            }
        }

        closeSection(linesLen);

        // An opening line the file never closes is NOT a boilerplate region. The pure-Python
        // original's pattern needs both halves to match -- its closing alternative is a required
        // part of the regex, not an optional tail -- so a heading whose closing rule is too short
        // (or missing outright) leaves the whole block alone rather than eating the rest of the
        // file. Confirmed against test_IniFile.test_differentText_remapBlendSectionsAndScriptFixRemoved,
        // whose "; --------------- Raiden Boss Fix ---------------" block closes on only 37
        // sideChars where 45 are required, and is expected to survive untouched.
        //
        // The unterminated region is always the last one, so dropping it cannot renumber any other.
        if (currentBoilerPlate != std::string::npos) {
            result.boilerPlates.pop_back();

            // Its sections revert to being outside every region, and so are re-judged by name like
            // any other leftover.
            for (SectionSpan& span : result.sections) {
                if (span.boilerPlateInd == currentBoilerPlate) {
                    span.boilerPlateInd = std::string::npos;
                }
            }
        }

        return result;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename RemoverBase>
    bool RemapIniRemover<K, V, KeyHash, KeyEqual, RemoverBase>::hashBelongsToModType(const V& hashVal) const {
        if (ctx_ == nullptr) {
            return false;
        }

        // Every mod type the .ini file was classified as, not just one: unlike the pure-Python
        // original (whose 'availableType' is singular), an AGRemapCore::IniFile can carry several,
        // and IniFile::removeFix hands the same file to each one's remover in turn without telling
        // any of them which one it is acting for.
        std::optional<Version> version = ctx_->version();

        for (Assets* hashes : ctx_->modTypeHashes()) {
            if (hashes != nullptr && hashes->hasFrom(hashVal, version, hashNonVersionVals)) {
                return true;
            }
        }

        return false;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename RemoverBase>
    bool RemapIniRemover<K, V, KeyHash, KeyEqual, RemoverBase>::hasExt(const std::string& path, const std::string& ext) {
        if (ext.empty() || path.size() < ext.size()) {
            return false;
        }

        std::size_t offset = path.size() - ext.size();
        for (std::size_t i = 0; i < ext.size(); ++i) {
            unsigned char left = static_cast<unsigned char>(path[offset + i]);
            unsigned char right = static_cast<unsigned char>(ext[i]);

            if (std::tolower(left) != std::tolower(right)) {
                return false;
            }
        }

        return true;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename RemoverBase>
    std::string RemapIniRemover<K, V, KeyHash, KeyEqual, RemoverBase>::classifyResource(const std::string& sectionName,
                                                                                    const std::string& filePath) const {
        // First, and without looking at the path at all: a download is named for what fetched it,
        // and the file at the other end can be anything.
        if (!downloadKeyword.empty() && sectionName.find(downloadKeyword) != std::string::npos) {
            return ResourceType::Download;
        }

        if (hasExt(filePath, texExt)) {
            if (!texAddKeyword.empty() && sectionName.find(texAddKeyword) != std::string::npos) {
                return ResourceType::TexAdd;
            }

            return ResourceType::TexEdit;
        }

        if (hasExt(filePath, bufExt)) {
            // Checked in this order because the keywords are disjoint in practice -- no section name
            // this software writes carries two of them.
            if (!blendKeyword.empty() && sectionName.find(blendKeyword) != std::string::npos) {
                return ResourceType::Blend;
            }

            if (!positionKeyword.empty() && sectionName.find(positionKeyword) != std::string::npos) {
                return ResourceType::Position;
            }

            if (!texcoordKeyword.empty() && sectionName.find(texcoordKeyword) != std::string::npos) {
                return ResourceType::Texcoord;
            }

            return ResourceType::Buf;
        }

        return ResourceType::Other;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename RemoverBase>
    void RemapIniRemover<K, V, KeyHash, KeyEqual, RemoverBase>::collectRemovedResources(
            const std::unordered_map<std::string, Section*>& pool, const std::vector<std::string>& removedNames,
            const std::string& iniFolder) {
        for (const std::string& sectionName : removedNames) {
            auto found = pool.find(sectionName);
            if (found == pool.end() || found->second == nullptr) {
                continue;
            }

            for (const std::unique_ptr<IfTemplatePart>& partOwned : found->second->parts()) {
                // Only the KVP parts hold values at all; the if/elif/else/endif ones are structure.
                // Every branch is walked, not just one -- a section naming a different file per
                // branch has all of them removed, so all of them are collected.
                ContentPart* part = dynamic_cast<ContentPart*>(partOwned.get());
                if (part == nullptr || !part->contains(config_.filenameKey)) {
                    continue;
                }

                for (const V& rawPath : part->getVals(config_.filenameKey)) {
                    std::string filePath(StringTools::strip(valToStr(rawPath)));
                    if (filePath.empty()) {
                        continue;
                    }

                    std::string resType = classifyResource(sectionName, filePath);
                    removedResources_[resType].push_back(std::make_unique<IniResource>(resType, iniFolder, filePath));
                }
            }
        }
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename RemoverBase>
    std::vector<std::string> RemapIniRemover<K, V, KeyHash, KeyEqual, RemoverBase>::collectCandidates(const FileScan& scan) const {
        std::vector<std::string> result;
        std::unordered_set<std::string> seen;

        for (const SectionSpan& span : scan.sections) {
            bool inBoilerPlate = span.boilerPlateInd != std::string::npos;

            // Inside the boilerplate, everything counts, whatever it is called -- that is the whole
            // point of finding the fix by where it lives rather than by its name. Outside, only the
            // leftovers a previous fix named.
            if (!inBoilerPlate && span.name.find(remapKeyword) == std::string::npos) {
                continue;
            }

            if (seen.insert(span.name).second) {
                result.push_back(span.name);
            }
        }

        return result;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename RemoverBase>
    std::vector<std::string> RemapIniRemover<K, V, KeyHash, KeyEqual, RemoverBase>::findTargets(
            const Graph& graph, const std::vector<std::string>& candidates,
            const std::unordered_set<std::string>& boilerPlateSections, bool ignoreModType) const {
        // "Every candidate", spelled as "the candidate list verbatim" -- it is already in the .ini
        // file's declaration order and already de-duplicated, which is exactly what the rest of this
        // function spends its time producing. See IniRemovalContext::ignoreModType.
        if (ignoreModType) {
            return candidates;
        }

        // Everything the boilerplate surrounds is this software's own output by definition -- that
        // marker is exactly what a fix writes to say so, and it needs no corroboration from a
        // `hash`. This is what makes a GIMI fix matched purely by TextureOverride *name* (no `hash`
        // KVP anywhere in it, which is normal) removable at all.
        std::unordered_set<std::string> targets = boilerPlateSections;

        // colour = true is what makes this "some IniColouring state" rather than "some KVP of this
        // one section": a section reached through a run = call carries the KVP state of everything
        // that ran it, so a hash set by a caller counts for its callees too. Only the hash key is
        // tracked, since nothing else is asked about.
        //
        // Still run even when every candidate is already a target: the walk is what decides for the
        // leftovers OUTSIDE any boilerplate, which have no marker of their own.
        std::unordered_set<K, KeyHash, KeyEqual> colourKeys{config_.hashKey};
        Generator<typename Graph::IterData> walk = graph.iterByContentPart(1, true, colourKeys);

        while (walk.next()) {
            typename Graph::IterData& iterData = walk.value();
            if (iterData.colouring == nullptr || targets.count(iterData.sectionName)) {
                continue;
            }

            for (const V& hashVal : iterData.colouring->getVals(config_.hashKey)) {
                if (hashBelongsToModType(hashVal)) {
                    targets.insert(iterData.sectionName);
                    break;
                }
            }
        }

        // Back into the .ini file's own declaration order. The graph walk visits by DFS from its
        // roots, and IniSectionGraph::build's own results (roots, and the render order every graph
        // built from these targets inherits) depend on the order it is handed them.
        std::vector<std::string> result;
        for (const std::string& candidate : candidates) {
            if (targets.count(candidate)) {
                result.push_back(candidate);
            }
        }

        return result;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename RemoverBase>
    std::unordered_map<std::string, std::vector<std::string>> RemapIniRemover<K, V, KeyHash, KeyEqual, RemoverBase>::buildReferences(
            const std::unordered_map<std::string, Section*>& pool) const {
        std::unordered_map<std::string, std::vector<std::string>> result;

        for (const std::pair<const std::string, Section*>& entry : pool) {
            if (entry.second == nullptr) {
                continue;
            }

            std::unordered_set<std::string> seen;

            for (const std::unique_ptr<IfTemplatePart>& partOwned : entry.second->parts()) {
                ContentPart* part = dynamic_cast<ContentPart*>(partOwned.get());
                if (part == nullptr) {
                    continue;
                }

                // Every key, not a list of resource-pointing ones -- see buildReferences' own note
                // in the header for why. Every branch too: a section naming a different resource per
                // `if` arm references all of them.
                for (const K& key : part->getKeys()) {
                    for (const V& value : part->getVals(key)) {
                        std::string name(StringTools::strip(valToStr(value)));

                        // A self-reference is not an edge, and a value naming nothing in the pool is
                        // not a section reference at all (`type = Buffer`, `stride = 32`, a file
                        // path, a hash...).
                        if (name == entry.first || !pool.count(name)) {
                            continue;
                        }

                        if (seen.insert(name).second) {
                            result[entry.first].push_back(name);
                        }
                    }
                }
            }
        }

        return result;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename RemoverBase>
    std::unordered_map<std::string, std::vector<std::string>> RemapIniRemover<K, V, KeyHash, KeyEqual, RemoverBase>::reverse(
            const std::unordered_map<std::string, std::vector<std::string>>& edges) {
        std::unordered_map<std::string, std::vector<std::string>> result;

        for (const std::pair<const std::string, std::vector<std::string>>& entry : edges) {
            for (const std::string& to : entry.second) {
                result[to].push_back(entry.first);
            }
        }

        return result;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename RemoverBase>
    std::unordered_set<std::string> RemapIniRemover<K, V, KeyHash, KeyEqual, RemoverBase>::closeOver(
            const std::unordered_map<std::string, std::vector<std::string>>& edges,
            const std::unordered_set<std::string>& seeds) {
        std::unordered_set<std::string> result = seeds;
        std::deque<std::string> toVisit(seeds.begin(), seeds.end());

        while (!toVisit.empty()) {
            std::string current = std::move(toVisit.front());
            toVisit.pop_front();

            auto found = edges.find(current);
            if (found == edges.end()) {
                continue;
            }

            for (const std::string& next : found->second) {
                if (result.insert(next).second) {
                    toVisit.push_back(next);
                }
            }
        }

        return result;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename RemoverBase>
    std::string RemapIniRemover<K, V, KeyHash, KeyEqual, RemoverBase>::remove(bool parse, bool writeBack, IniRemovalContext context) {
        // Accepted and ignored -- see this method's own note.
        (void)parse;

        targetSectionNames_.clear();
        removedSectionNames_.clear();
        removedResources_.clear();
        removalGraph_.reset();

        if (ctx_ == nullptr || !ctx_->hasIni()) {
            return "";
        }

        // The equivalent of the pure-Python original's own "_readLines" decorator -- see
        // IniRemoveContext::readFileLines.
        std::vector<std::string> lines = ctx_->readFileLines();
        FileScan scan = scanFile(lines);

        std::vector<std::string> candidates = collectCandidates(scan);

        // Every candidate at least one of whose occurrences sits inside a boilerplate region. "At
        // least one" is the right test for a name that appears both inside and outside one (the
        // standard shape of a half-undone fix): the copy inside the boilerplate is this software's
        // own, and that is enough to make the name a target.
        std::unordered_set<std::string> boilerPlateSections;
        for (const SectionSpan& span : scan.sections) {
            if (span.boilerPlateInd != std::string::npos) {
                boilerPlateSections.insert(span.name);
            }
        }

        // The pool the whole rest of this works within. Keyed by name, so the duplicate occurrences
        // a half-fixed file has collapse here exactly the way sectionIfTemplates already collapsed
        // them (first one wins) -- the *spans* keep both, which is what the line removal below
        // needs.
        std::unordered_map<std::string, Section*> allSections = ctx_->sectionIfTemplates();
        std::unordered_map<std::string, Section*> pool;
        std::vector<std::string> poolNames;

        for (const std::string& candidate : candidates) {
            auto found = allSections.find(candidate);
            if (found == allSections.end() || found->second == nullptr) {
                continue;
            }

            pool.emplace(candidate, found->second);
            poolNames.push_back(candidate);
        }

        // Built twice on purpose. The first build makes every candidate a target, which is the only
        // way to get a graph that reaches all of them -- the hash-bearing ones can't be known before
        // walking it. The second re-roots it on the targets actually found.
        //
        // No Z3 context: nothing here calls iterByQuery, the one thing that needs one.
        removalGraph_ = std::make_unique<Graph>(pool, poolNames, config_.runConfig);
        // 'candidates', not 'poolNames': being inside the boilerplate makes a section a target on
        // its own, whether or not sectionIfTemplates managed to parse it. Only the hash half of the
        // rule needs a parsed section, and that half runs off the graph. IniSectionGraph::build
        // skips a target name it has no section for, and a name with no section simply has no
        // references either -- but its *spans* are still deleted, which is the point.
        targetSectionNames_ = findTargets(*removalGraph_, candidates, boilerPlateSections, context.ignoreModType);
        removalGraph_->build(std::nullopt, targetSectionNames_);

        // NOT removalGraph_->sections(): that is the targets plus what they reach by `run =` alone,
        // and a fix reaches most of its Resource sections by `vb1 =`/`ib =`/`ps-t0 =` instead. The
        // graph is still built (and still exposed) because its targets are the answer to "what did
        // this software write"; the removal set closes over the wider relation instead.
        std::unordered_map<std::string, std::vector<std::string>> references = buildReferences(pool);

        std::unordered_set<std::string> targets(targetSectionNames_.begin(), targetSectionNames_.end());

        // Forward first (the targets and everything they reach), then backward over the whole of
        // that (anything left pointing into it) -- so nothing survives holding a reference to
        // something deleted, in either direction.
        std::unordered_set<std::string> removed = closeOver(references, targets);
        removed = closeOver(reverse(references), removed);

        for (const std::string& candidate : candidates) {
            if (removed.count(candidate)) {
                removedSectionNames_.push_back(candidate);
            }
        }

        // Done before the lines are rewritten, and off the parsed sections rather than off the raw
        // text: a 'filename' KVP can sit inside an if/else branch, which only the IfTemplate knows
        // how to walk.
        //
        // The "." fallback is load-bearing, not cosmetic. An .ini file with no folder --  a
        // file-less one, or one whose path is a bare relative file name -- would otherwise hand an
        // empty folder to FileService::absPathOfRelPath, which passes it to
        // std::filesystem::absolute, which THROWS on MSVC rather than resolving to the working
        // directory. That escapes remove() as an uncaught filesystem_error and terminates the
        // process. "." is the same working directory, spelled in a way absolute() accepts.
        std::string iniFolder = ctx_->iniFolder();
        if (iniFolder.empty()) {
            iniFolder = ".";
        }

        collectRemovedResources(pool, removedSectionNames_, iniFolder);

        // ----- rewrite the file's lines -----

        std::size_t linesLen = lines.size();
        std::vector<bool> keep(linesLen, true);

        for (const SectionSpan& span : scan.sections) {
            if (!removed.count(span.name)) {
                continue;
            }

            for (std::size_t i = span.startInd; i < span.endInd && i < linesLen; ++i) {
                keep[i] = false;
            }
        }

        std::size_t boilerPlatesLen = scan.boilerPlates.size();
        for (std::size_t boilerPlateInd = 0; boilerPlateInd < boilerPlatesLen; ++boilerPlateInd) {
            bool survived = false;

            for (const SectionSpan& span : scan.sections) {
                if (span.boilerPlateInd == boilerPlateInd && !removed.count(span.name)) {
                    survived = true;
                    break;
                }
            }

            if (survived) {
                continue;
            }

            // Nothing the boilerplate was wrapped around is left, so the wrapper goes too -- its
            // heading lines and the credit comment between them, which belong to no section span at
            // all. Both ends are real line indices: scanFile drops any region it never found a
            // closing line for.
            const BoilerPlateSpan& boilerPlate = scan.boilerPlates[boilerPlateInd];

            for (std::size_t i = boilerPlate.openInd; i <= boilerPlate.closeInd && i < linesLen; ++i) {
                keep[i] = false;
            }
        }

        std::string newTxt;
        for (std::size_t i = 0; i < linesLen; ++i) {
            if (keep[i]) {
                newTxt += lines[i];
            }
        }

        // Un-hides the original mod. A fix applied with hideOrig comments its sections out with
        // this prefix, so a removal that only deleted the fix's own sections would leave the .ini
        // file with nothing switched on at all -- the original's own _removeFixComment step.
        newTxt = removeAll(newTxt, hideOriginalComment);

        // LEADING whitespace only, and that asymmetry is the pure-Python original's, not a
        // shortcut. That original removes a fix in two passes: _removeScriptFix strips the whole
        // text (both ends), and _removeFixSections then deletes further line ranges out of what is
        // left -- which can put trailing blank lines back. So the net contract is "never any leading
        // whitespace, whatever trailing the section removal happened to leave", which a single pass
        // reproduces by stripping one end. Pinned by
        // test_IniFile.test_differentText_remapBlendSectionsAndScriptFixRemoved, whose expected
        // output really does end on two newlines.
        std::size_t firstKept = 0;
        while (firstKept < newTxt.size() && std::isspace(static_cast<unsigned char>(newTxt[firstKept]))) {
            ++firstKept;
        }
        newTxt = newTxt.substr(firstKept);

        ctx_->setFileTxt(std::move(newTxt));

        // The .ini file no longer holds a fix -- see IniRemoveContext::setIsFixed on why a plain
        // C++ implementation of that can do nothing.
        ctx_->setIsFixed(false);

        if (!writeBack) {
            return ctx_->fileTxt();
        }

        std::string result = ctx_->write();
        ctx_->clearRead();
        return result;
    }
}

#endif
