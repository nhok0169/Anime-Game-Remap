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

#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphCollect.h"
#include "AGRemapCore/model/IniNamingTools.h"
#include "AGRemapCore/data/IniParseData/GIMITemplateParser.h"

#include <unordered_set>

#include "AGRemapCore/constants/IniGraphModObjKeywords.h"
#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/data/IniParseData/TextureOverrides.h"
#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/tools/StringTools.h"


namespace AGRemapCore {
    void GIMITemplateParser::addBound(std::set<std::string>& bound) const {
        (void)bound;
    }

    bool GIMITemplateParser::skipsBindCheck(const ModObj& modObj) const {
        (void)modObj;
        return false;
    }

    std::vector<std::string> GIMITemplateParser::siblingInis() const {
        IniFile* iniFile = const_cast<GIMITemplateParser*>(this)->getIniFile();
        return iniFile == nullptr ? std::vector<std::string>{} : iniFile->loadedSiblings();
    }

    bool GIMITemplateParser::drawsMesh(const IfTemplates& templates) {
        for (const auto& entry : templates) {
            if (entry.second == nullptr) {
                continue;
            }

            const std::optional<std::string> hash = entry.second->firstVal(IniKeywords::Hash);
            if (!hash.has_value()) {
                continue;
            }

            for (const std::string& key : hashKeysOf(*hash)) {
                if (key.empty() || StringTools::startsWith(key, TexKeyPrefix)) {
                    continue;
                }
                for (const std::string& reg : {IniKeywords::Ib, IniKeywords::Vb0, IniKeywords::Vb1}) {
                    const std::optional<std::string> value = entry.second->firstVal(reg);
                    if (value.has_value() && !IniNamingTools::boundResource(value).empty()) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    bool GIMITemplateParser::siblingDrawsMesh() {
        for (const std::string& path : siblingInis()) {
            IniFile sibling(path);
            if (drawsMesh(sibling.getIfTemplates())) {
                return true;
            }
        }
        return false;
    }

    bool GIMITemplateParser::bindsPosition(IniFile& ini) {
        for (const auto& entry : ini.getIfTemplates()) {
            if (entry.second == nullptr) {
                continue;
            }
            const std::optional<std::string> hash = entry.second->firstVal(IniKeywords::Hash);
            if (!hash.has_value()) {
                continue;
            }
            for (const std::string& key : hashKeysOf(*hash)) {
                if (key == PositionHashKey) {
                    auto graph = ini.sectionGraph(entry.first);
                    if (GraphCollect<>::rowsOf(graph, IniKeywords::Vb0).empty()) {
                        continue;
                    }
                    return true;
                }
            }
        }
        return false;
    }

    bool GIMITemplateParser::siblingBindsPosition() {
        for (const std::string& path : siblingInis()) {
            IniFile sibling(path);
            if (bindsPosition(sibling)) {
                return true;
            }
        }
        return false;
    }

    void GIMITemplateParser::declareSiblingRefs() {
            refGraphs_.clear();
            std::set<std::string> bound;
            for (const auto& [modObj, bindings] : textureBindings_) {
                for (const auto& binding : bindings) {
                    bound.insert(binding.second);
                }
            }
            addBound(bound);

            for (const auto& [name, file] : siblingRefs_) {
                auto adopted = adoptedRefs_.find(name);
                if (bound.count(name) == 0 && adopted == adoptedRefs_.end()) {
                    continue;
                }

                // An existing one is the PREVIOUS fix's, read before its undo, and is the right
                // section to render again -- skipping it declared nothing on every later run.
                Section* section = ctx_.getSection(name);
                if (section == nullptr) {
                    section = ctx_.addSection(name,
                        std::make_unique<Section>(std::vector<std::unique_ptr<IfTemplatePart>>{}, this->config().runConfig, name));
                    section->addKVPsToFront(adopted != adoptedRefs_.end() ? adopted->second
                        : std::vector<std::pair<std::string, std::string>>{{IniKeywords::Filename, file}});
                }

                Graph* graph = ctx_.graphGroups().createGraph({}, {}, false, nullptr);
                graph->build(std::unordered_map<std::string, Section*>{{name, section}}, std::vector<std::string>{name});
                refGraphs_.emplace_back(name, graph);
            }
        }

    void GIMITemplateParser::bindTextureOverrides() {
            for (const auto& [modObj, bindings] : textureBindings_) {
                Graph* graph = this->getCommandGraph(modObj);
                if (graph == nullptr || graph->isEmpty() || graph->roots().empty()) {
                    continue;
                }

                Section* section = ctx_.getSection(graph->roots().front());
                if (section != nullptr) {
                    section->addKVPsToFront(bindings);
                }
            }
        }

    bool GIMITemplateParser::targetsBindSomething() {
            IniFile* iniFile = this->getIniFile();
            if (iniFile == nullptr) {
                return true;
            }
            const auto& templates = iniFile->getIfTemplates();

            static const std::vector<std::string> BindingKeys = {
                IniKeywords::Vb0, IniKeywords::Vb1, "vb2", IniKeywords::Ib, IniKeywords::DrawIndexed,
                "draw", "ps-t0", "ps-t1", "ps-t2", "ps-t3"};

            // NOR THE FACE'S OWN SECTIONS AT ALL (2026-10-20), for the same reason: a face
            // recolour written with `ps-t` lines rather than `this` (Lumine12's
            // Face Texture/Lumine.ini) drew the whole skin from downloads, outside the merged
            // master's outfit switch, over every outfit at once.
            std::unordered_set<std::string> visited;
            std::vector<std::string> toVisit;
            for (const auto& entry : this->sectionTargets()) {
                if (skipsBindCheck(entry.first)) {
                    continue;
                }
                toVisit.insert(toVisit.end(), entry.second.begin(), entry.second.end());
            }

            while (!toVisit.empty()) {
                const std::string name = toVisit.back();
                toVisit.pop_back();
                if (!visited.insert(name).second) {
                    continue;
                }

                auto found = templates.find(name);
                if (found == templates.end() || found->second == nullptr) {
                    continue;     // an external command list (ORFix, ...) binds nothing of the mod's
                }

                for (const auto& part : found->second->parts()) {
                    const auto* content = dynamic_cast<const ContentPart*>(part.get());
                    if (content == nullptr) {
                        continue;
                    }
                    for (const std::string& key : BindingKeys) {
                        if (content->containsKey(key)) {
                            return true;
                        }
                    }
                    for (const std::string& call : content->getVals(IniKeywords::Run)) {
                        toVisit.emplace_back(StringTools::strip(call));
                    }
                }
            }

            return false;
        }

    std::vector<GIMITemplateParser::GraphGroup> GIMITemplateParser::collectParseResult() const {
            std::vector<GraphGroup> result = TemplateParser::collectParseResult();
            if (result.empty()) {
                return result;
            }

            for (const auto& [name, graph] : refGraphs_) {
                const ModObj modObj(IniGraphModObjKeywords::Download, name);
                if (graph != nullptr && result.front().getGraph(modObj) == nullptr) {
                    result.front().addGraph(modObj, graph->deepcopy());
                }
            }
            return result;
        }
}
