#include "AGRemapCore/data/IniParseData/ZZMIParser.h"

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

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/data/ZZMIModObjs.h"
#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/model/strategies/iniParsers/GIMIParser.h"
#include "AGRemapCore/model/strategies/iniParsers/IniFileParseContext.h"
#include "AGRemapCore/tools/StringTools.h"


namespace AGRemapCore {
    namespace {
        using Parser = GIMIParser<>;
        using Classifier = Parser::Classifier;
        using ModObj = Parser::ModObj;
        using Section = Parser::Section;
        using ContentPart = Parser::ContentPart;
        using Colouring = Parser::Colouring;

        using ZZMIModObjs::HashKinds;
        using ZZMIModObjs::compObjs;
        using ZZMIModObjs::objOf;


        /**
         * The parser of a ZZMI character's mods: one classifier per component, filtered to that
         * component's own hash rows (every component carries the same hash TYPES, so a type alone
         * cannot say which one a section is), and a drawn object resolved from the component's slots
         */
        class ZZMIParserImpl: public Parser {
            public:
                ZZMIParserImpl(IniFile* iniFile, std::optional<int> modTypeId, std::vector<ModObj> modObjs,
                                const ZZMIFixerConfig& config):
                    Parser(nullptr, std::move(modObjs)), ctx_(iniFile, modTypeId), config_(config) {
                    this->setCtx(&ctx_);
                    this->setIniFile(iniFile);
                    this->disjointModObjs = false;
                    this->trackKeys = true;
                    this->keysToTrack = std::unordered_set<std::string>{IniKeywords::Hash, IniKeywords::MatchFirstIndex};

                    for (const ZZMIFixerConfig::Component& c : config_.components) {
                        std::unordered_map<std::string, ModObj> hashOnly;
                        for (const auto& kind : HashKinds) {
                            hashOnly[kind.first] = objOf(c.name, kind.second);
                        }
                        for (const ZZMIFixerConfig::Texture& tex : c.textures) {
                            hashOnly[tex.hashType] = objOf(c.name, "tex_" + tex.role);
                        }

                        auto classifier = std::make_unique<Classifier>(
                            std::move(hashOnly), ctx_.modTypeHashes(),
                            std::unordered_map<std::string, Classifier::IndexModObjs>{}, ctx_.modTypeIndices(),
                            ctx_.version());
                        classifier->setHashNonVersionVals({config_.source + c.name, std::nullopt});
                        classifiers_.push_back(std::move(classifier));
                    }

                    const ZZMIFixerConfig* cfg = &config_;
                    std::vector<Classifier*> classifiers;
                    for (const auto& classifier : classifiers_) {
                        classifiers.push_back(classifier.get());
                    }

                    this->objTargetFuncs.emplace_back(
                        [cfg, classifiers](Parser&, const std::string& sectionName, Section* section, bool,
                                            ContentPart*, const Colouring* kvps) {
                            std::vector<ModObj> result;
                            if (kvps == nullptr) {
                                return result;
                            }

                            for (std::size_t i = 0; i < classifiers.size() && i < cfg->components.size(); ++i) {
                                std::vector<ModObj> found = classifiers[i]->classify(sectionName, section, *kvps);
                                if (found.empty()) {
                                    continue;
                                }

                                // A drawn object: the component's ib hash plus a match_first_index it knows
                                const ZZMIFixerConfig::Component& c = cfg->components[i];
                                std::vector<std::string> indexVals = kvps->getVals(IniKeywords::MatchFirstIndex);
                                for (ModObj& modObj : found) {
                                    if (modObj != objOf(c.name, "ib") || indexVals.empty()) {
                                        continue;
                                    }
                                    for (const ZZMIFixerConfig::Slot& slot : c.slots) {
                                        if (slot.srcIndex == std::string(StringTools::strip(indexVals.front()))) {
                                            modObj = objOf(c.name, slot.name);
                                            break;
                                        }
                                    }
                                }
                                result.insert(result.end(), found.begin(), found.end());
                            }
                            return result;
                        });
                }

            private:
                IniFileParseContext ctx_;
                ZZMIFixerConfig config_;
                std::vector<std::unique_ptr<Classifier>> classifiers_;
        };
    }


    IniParseBuilder::Factory makeZZMIParser(ZZMIFixerConfig config) {
        return [config](IniFile* iniFile, std::optional<int> modTypeId) {
            std::vector<ModObj> modObjs;
            for (const ZZMIFixerConfig::Component& c : config.components) {
                for (const std::string& obj : compObjs(c)) {
                    modObjs.push_back(objOf(c.name, obj));
                }
            }
            return std::make_shared<ZZMIParserImpl>(iniFile, modTypeId, std::move(modObjs), config);
        };
    }
}
