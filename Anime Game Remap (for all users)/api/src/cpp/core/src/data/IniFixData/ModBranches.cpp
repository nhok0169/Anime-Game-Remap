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

#include "AGRemapCore/data/IniFixData/ModBranches.h"

#include <algorithm>
#include <unordered_map>
#include <utility>

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/model/IniSectionGraph.h"
#include "AGRemapCore/model/files/IbFile.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/ResGroupCollect.h"
#include "AGRemapCore/tools/StringTools.h"
#include "AGRemapCore/tools/files/FileService.h"


namespace AGRemapCore {
    namespace {
        const std::string FormatKey = "format";
    }


    Z3Context& ModBranches::context() {
        return ctx_;
    }


    std::vector<BranchVal> ModBranches::valsThroughRun(const Templates& templates, const std::string& rootSection,
                                                       const std::string& key) {
        auto rootIt = templates.find(rootSection);
        if (rootIt == templates.end() || rootIt->second == nullptr) {
            return {};
        }

        std::unordered_map<std::string, Template*> sections;
        sections.reserve(templates.size());
        for (const auto& entry : templates) {
            if (entry.second != nullptr) {
                sections.emplace(entry.first, entry.second.get());
            }
        }

        IfTemplateRunConfig<std::string, std::string> runConfig{
            IniKeywords::Run,
            [](const std::string& v) { return v; },
            [](const std::string& s) { return s; }
        };

        // IniSectionGraph is the library's call-graph walker -- it follows `run =` transitively and
        // copes with cycles. Built in OUR context: every query read here outlives this call.
        IniSectionGraph<std::string, std::string> graph(std::move(sections), {rootSection}, runConfig, true, false,
                                                        &ctx_);

        // iterByQuery rather than a walk over parts(): it is the same iteration, and it reports the
        // conditional predicate each part sits under, which is the half that is needed.
        tsl::ordered_map<std::string, std::vector<BranchVal>> bySection;

        auto parts = graph.iterByQuery();
        while (parts.next()) {
            auto& iterData = parts.value();
            if (iterData.part == nullptr) {
                continue;
            }

            for (const std::string& val : iterData.part->getVals(key)) {
                bySection[iterData.sectionName].push_back(BranchVal{std::string(StringTools::strip(val)), iterData.query});
            }
        }

        // Root first, then everything it reaches, in the graph's own order: a single-valued caller
        // takes the FIRST value, and the root's own binding should win.
        std::vector<BranchVal> out;
        const auto append = [&out, &bySection](const std::string& sectionName) {
            auto it = bySection.find(sectionName);
            if (it == bySection.end()) {
                return;
            }

            // Copied, not moved: tsl::ordered_map hands out CONST values through its iterator even
            // from a non-const begin().
            for (const BranchVal& val : it->second) {
                out.push_back(val);
            }
        };

        append(rootSection);
        for (const auto& entry : graph.sections()) {
            if (entry.first != rootSection && entry.second != nullptr) {
                append(entry.first);
            }
        }

        return out;
    }


    std::optional<std::string> ModBranches::firstValThroughRun(const Templates& templates, const std::string& rootSection,
                                                               const std::string& key) {
        std::vector<BranchVal> vals = valsThroughRun(templates, rootSection, key);
        if (vals.empty()) {
            return std::nullopt;
        }

        return vals.front().val;
    }


    std::optional<Z3Predicate> ModBranches::localQuery(const Z3Predicate* query) {
        if (query == nullptr) {
            return std::nullopt;
        }

        return ResGroupCollect<>::combineQueries(*query, Z3Predicate::trueValue(ctx_), &ctx_);
    }


    bool ModBranches::compatible(const Z3Predicate& a, const Z3Predicate& b) {
        return ResGroupCollect<>::combineQueries(a, b, &ctx_).isSatisfiable();
    }


    long long ModBranches::branchIndexOf(const std::vector<BranchVal>& branches, const std::optional<Z3Predicate>& query) {
        if (branches.size() <= 1) {
            return branches.empty() ? -1 : 0;
        }

        if (!query.has_value()) {
            return -1;
        }

        long long found = -1;
        for (std::size_t i = 0; i < branches.size(); ++i) {
            if (!branches[i].query.has_value()) {
                continue;
            }

            if (compatible(*query, *branches[i].query)) {
                if (found >= 0) {
                    return -1;
                }

                found = static_cast<long long>(i);
            }
        }

        return found;
    }


    std::string ModBranches::pick(const std::vector<BranchVal>& candidates, const std::string& fallback,
                                  const std::optional<Z3Predicate>& query) {
        if (candidates.empty()) {
            return fallback;
        }

        if (candidates.size() == 1 || !query.has_value()) {
            return candidates.front().val;
        }

        for (const BranchVal& candidate : candidates) {
            if (candidate.query.has_value() && compatible(*query, *candidate.query)) {
                return candidate.val;
            }
        }

        return candidates.front().val;
    }


    bool ModBranches::anyCompatible(const std::vector<BranchVal>& vals, const std::optional<Z3Predicate>& query) {
        for (const BranchVal& val : vals) {
            if (!val.query.has_value() || !query.has_value() || compatible(*query, *val.query)) {
                return true;
            }
        }

        return false;
    }


    std::vector<std::optional<Z3Predicate>> ModBranches::states(const std::vector<const std::vector<BranchVal>*>& lists) {
        std::vector<std::optional<Z3Predicate>> result{std::nullopt};

        for (const std::vector<BranchVal>* list : lists) {
            if (list == nullptr || list->size() <= 1) {
                continue;
            }

            std::vector<std::optional<Z3Predicate>> refined;
            std::vector<std::string> seen;
            const auto add = [&refined, &seen](std::optional<Z3Predicate> state) {
                std::string key = state.has_value() ? state->toString() : std::string();
                if (std::find(seen.begin(), seen.end(), key) == seen.end()) {
                    seen.push_back(std::move(key));
                    refined.push_back(std::move(state));
                }
            };

            for (const std::optional<Z3Predicate>& state : result) {
                bool split = false;
                for (const BranchVal& value : *list) {
                    if (!value.query.has_value()) {
                        continue;
                    }

                    if (!state.has_value()) {
                        add(value.query);
                        split = true;
                    } else if (state->toString() == value.query->toString()) {
                        add(state);
                        split = true;
                    } else if (compatible(*state, *value.query)) {
                        // combineQueries, not operator&: the two may come from different graphs'
                        // contexts, and it reparents into ours.
                        add(ResGroupCollect<>::combineQueries(*state, *value.query, &ctx_).simplify());
                        split = true;
                    }
                }

                if (!split) {
                    add(state);
                }
            }

            result = std::move(refined);
        }

        return result;
    }


    std::unique_ptr<RegBranchAdd<>> ModBranches::replacePerBranch(std::vector<BranchVal> branches, std::string keyPrefix,
                                                                  BranchReplacements replacementsOf) {
        return std::make_unique<RegBranchAdd<>>(
            [this, branches = std::move(branches), keyPrefix = std::move(keyPrefix),
             replacementsOf = std::move(replacementsOf)](const Z3Predicate& query, const RegBranchAdd<>::IterData&) {
                RegBranchAdd<>::Branch result;

                const std::optional<Z3Predicate> local = localQuery(&query);
                const long long branch = branchIndexOf(branches, local);
                if (branch < 0) {
                    return result;
                }

                RegBranchAdd<>::Additions replacements = replacementsOf(static_cast<std::size_t>(branch), local);
                if (replacements.empty()) {
                    return result;
                }

                result.key = keyPrefix + ";" + std::to_string(branch);
                result.replacements = std::move(replacements);
                return result;
            });
    }


    std::optional<std::string> ModBranches::firstVal(const Template& tpl, const std::string& key) {
        for (const auto& part : tpl.parts()) {
            const auto* content = dynamic_cast<const Template::ContentPart*>(part.get());
            if (content == nullptr) {
                continue;
            }

            std::vector<std::string> vals = content->getVals(key);
            if (!vals.empty()) {
                return std::string(StringTools::strip(vals.front()));
            }
        }

        return std::nullopt;
    }


    std::string ModBranches::resourceOf(const std::optional<std::string>& value) {
        if (!value.has_value() || value->empty() || StringTools::equalsIgnoreCase(*value, IniKeywords::Null)) {
            return "";
        }

        return *value;
    }


    std::string ModBranches::fileOf(const Templates& templates, const std::string& resource, const std::string& folder) {
        if (resource.empty()) {
            return "";
        }

        auto it = templates.find(resource);
        if (it == templates.end() || it->second == nullptr) {
            return "";
        }

        std::optional<std::string> file = firstVal(*it->second, IniKeywords::Filename);
        if (!file.has_value() || file->empty()) {
            return "";
        }

        return FileService::absPathOfRelPath(*file, folder);
    }


    std::size_t ModBranches::ibBytesPerIndexOf(const Templates& templates, const std::string& resource) {
        auto it = templates.find(resource);
        if (resource.empty() || it == templates.end() || it->second == nullptr) {
            return 4;
        }

        std::optional<std::string> format = firstVal(*it->second, FormatKey);
        return IbFile::bytesPerIndexOf(format.value_or(""));
    }
}
