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

#include "AGRemapCore/data/IniParseData/GIMIComponentParser.h"

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/model/strategies/iniParsers/GIMIParser.h"
#include "AGRemapCore/model/strategies/iniParsers/IniFileParseContext.h"
#include "AGRemapCore/tools/DownloadTools.h"


namespace AGRemapCore {
    namespace {
        using Parser = GIMIParser<>;
        using Classifier = Parser::Classifier;
        using ModObj = Parser::ModObj;

        const std::string IbHashKey = "ib";
        const std::string BlendHashKey = "blend_vb";
        const std::string PositionHashKey = "position_vb";
        const std::string TexcoordHashKey = "texcoord_vb";
        const std::string DrawHashKey = "draw_vb";
        const std::string FaceDiffuseHashKey = "tex_face_diffuse";

        const ModObj FaceObj{"", "face"};


        /**
         * A mod of a skin of several components, classified one component at a time.
         *
         * One classifier per component, each filtered to that component's own mod type name, because
         * a multi-component skin files each component's hashes under the COMPONENT's name and a
         * classifier's hash filter can only name one. A hash value is unique to one character, so a
         * section is answered by at most one of them.
         */
        class GIMIComponentGIMIParser: public Parser {
            public:
                GIMIComponentGIMIParser(IniFile* iniFile, std::optional<int> modTypeId, std::vector<ModObj> modObjs,
                                         const GIMIComponentParserConfig& config):
                    Parser(nullptr, std::move(modObjs)), ctx_(iniFile, modTypeId), config_(config) {
                    this->setCtx(&ctx_);
                    this->setIniFile(iniFile);

                    // One section may name several mod objects -- see RaidenParser.
                    this->disjointModObjs = false;

                    for (const GIMIComponentParserConfig::Component& component : config_.components) {
                        std::unordered_map<std::string, ModObj> hashOnly = {
                            {PositionHashKey, ModObj(component.name, "position")},
                            {BlendHashKey, ModObj(component.name, "blend")},
                            {TexcoordHashKey, ModObj(component.name, "texcoord")},
                            {DrawHashKey, ModObj(component.name, "other")},
                            {IbHashKey, ModObj(component.name, "ib")},
                            {FaceDiffuseHashKey, FaceObj}};

                        // No index map at all: the slot is resolved from the config below, because
                        // the components' slot indices are deliberately absent from IndexData.
                        auto classifier = std::make_unique<Classifier>(
                            std::move(hashOnly), ctx_.modTypeHashes(),
                            std::unordered_map<std::string, Classifier::IndexModObjs>{}, ctx_.modTypeIndices(),
                            ctx_.version());

                        if (!component.modTypeName.empty()) {
                            classifier->setHashNonVersionVals({component.modTypeName, std::nullopt});
                        }

                        classifiers_.push_back(std::move(classifier));
                    }

                    const GIMIComponentParserConfig* cfg = &config_;
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
                                result = classifiers[i]->classify(sectionName, section, *kvps);
                                if (result.empty()) {
                                    continue;
                                }

                                // A drawn object: the component's shared ib hash plus a
                                // match_first_index this component knows. Resolved here rather than
                                // by the classifier, which would have to reverse-look it up in a
                                // table these rows are deliberately not in.
                                const std::vector<std::string> indexVals = kvps->getVals(IniKeywords::MatchFirstIndex);
                                if (indexVals.empty()) {
                                    return result;
                                }

                                const GIMIComponentParserConfig::Component& component = cfg->components[i];
                                for (ModObj& modObj : result) {
                                    if (modObj.first != component.name || modObj.second != "ib") {
                                        continue;
                                    }

                                    for (const GIMIComponentParserConfig::Slot& slot : component.slots) {
                                        if (slot.index == indexVals.front()) {
                                            modObj.second = slot.name;
                                            break;
                                        }
                                    }
                                }

                                return result;
                            }

                            return result;
                        });

                    // WHAT AN INVENTED SECTION HAS TO SAY ABOUT ITSELF -- see
                    // GIMIParser::objIdentityKVPs. A skin of several components hits this far more
                    // often than the classic shape does: a mod may simply not have one of the
                    // components (the NSFW edit has no Eye at all), and the merge then reads that
                    // component's buffers out of the sections the parser invents for its downloads.
                    //
                    // Without this the download is fetched, written and referenced by a
                    // TextureOverride with no `hash`, which matches no draw call -- and, here,
                    // which GIMIMergeFixer's own file discovery cannot find either, since that walks
                    // the sections looking up each one's `hash`. The measured symptom was a merged
                    // buffer 120 vertices short and four orphaned RemapDL resources.
                    this->objIdentityKVPs =
                        [this, cfg](const ModObj& modObj) {
                            std::vector<std::pair<std::string, std::string>> kvps;
                            auto* hashes = ctx_.modTypeHashes();
                            if (hashes == nullptr) {
                                return kvps;
                            }

                            const std::optional<Version> version = ctx_.version();
                            auto addHash = [&](const std::string& modName, const std::string& hashKey) {
                                std::optional<std::string> hash = hashes->get({modName, hashKey}, version, false);
                                if (hash.has_value()) {
                                    kvps.emplace_back(IniKeywords::Hash, *hash);
                                }
                            };

                            if (modObj == FaceObj) {
                                // Filed under every component, identically -- the first will do.
                                if (!cfg->components.empty()) {
                                    addHash(cfg->components.front().modTypeName, FaceDiffuseHashKey);
                                }
                                return kvps;
                            }

                            for (const GIMIComponentParserConfig::Component& component : cfg->components) {
                                if (modObj.first != component.name || component.modTypeName.empty()) {
                                    continue;
                                }

                                // A DRAWN object first: the component's shared ib hash plus the
                                // match_first_index that tells its slots apart. The index comes from
                                // the config, for the same reason the classifier's does -- these
                                // rows are deliberately absent from IndexData.
                                for (const GIMIComponentParserConfig::Slot& slot : component.slots) {
                                    if (modObj.second != slot.name) {
                                        continue;
                                    }

                                    addHash(component.modTypeName, IbHashKey);
                                    kvps.emplace_back(IniKeywords::MatchFirstIndex, slot.index);
                                    return kvps;
                                }

                                static const std::unordered_map<std::string, std::string> KindHashKeys = {
                                    {"position", PositionHashKey}, {"blend", BlendHashKey},
                                    {"texcoord", TexcoordHashKey}, {"other", DrawHashKey}, {"ib", IbHashKey}};

                                auto kind = KindHashKeys.find(modObj.second);
                                if (kind != KindHashKeys.end()) {
                                    addHash(component.modTypeName, kind->second);
                                }
                                return kvps;
                            }

                            return kvps;
                        };

                    buildDownloads(config_);
                }

            private:
                /**
                 * The defaults a modder may have left out, per component and per slot. The file
                 * naming carries the component: <Prefix><Component><Slot><Kind>, which is how
                 * Data/Mod Downloads/GI/<Char>/<X_Y>/ is laid out for a multi-component skin.
                 */
                void buildDownloads(const GIMIComponentParserConfig& config) {
                    for (const GIMIComponentParserConfig::Component& component : config.components) {
                        for (const GIMIComponentParserConfig::Slot& slot : component.slots) {
                            const ModObj modObj(component.name, slot.name);
                            const std::string file = component.name + slot.name;

                            // A SLOT THAT BINDS NOTHING RENDERS WITH THE GAME'S TEXTURES.
                            //
                            // A GIMI TextureOverride binds registers for the draw call its hash
                            // matches and no other, so a slot whose own section declares no ps-t is
                            // drawn with whatever the game had bound -- the game's own atlas -- no
                            // matter what the mod did to the donor's textures. Reading the donor out
                            // of the MOD instead put a repainted atlas under the game's UVs: one
                            // YelanTranquil mod's hair and another's eyes both came out sampling
                            // somebody else's islands (2026-09-14).
                            //
                            // Gated by nothing beyond the register being uncovered, which is what
                            // the download machinery already tests. Restricting it to a component
                            // the mod lacks ENTIRELY was the first attempt and covered only one of
                            // the three mods that need it.
                            if (slot.noTextures && !slot.textureDonor.empty()) {
                                const std::size_t sep = slot.textureDonor.find(';');
                                if (sep != std::string::npos) {
                                    const std::string donor = slot.textureDonor.substr(0, sep)
                                                               + slot.textureDonor.substr(sep + 1);

                                    // Named after the DONOR, so the file fetched is the one the
                                    // game draws this slot with, and two slots borrowing the same
                                    // donor share one resource section rather than fetching twice.
                                    add(config, modObj, slot.diffuseReg, donor + "Diffuse", donor + "Diffuse", ".dds",
                                         {}, {}, true);
                                    add(config, modObj, slot.lightMapReg, donor + "LightMap", donor + "LightMap", ".dds",
                                         {}, {}, true);
                                }
                            }

                            if (!slot.noTextures) {
                                if (!slot.normalMapReg.empty()) {
                                    add(config, modObj, slot.normalMapReg, file + "NormalMap", file + "NormalMap", ".dds",
                                         {}, {}, true);
                                }

                                add(config, modObj, slot.diffuseReg, file + "Diffuse", file + "Diffuse", ".dds", {}, {}, true);
                                add(config, modObj, slot.lightMapReg, file + "LightMap", file + "LightMap", ".dds", {}, {}, true);
                            }

                            add(config, modObj, IniKeywords::Ib, file + "Ib", file, ".ib",
                                 DownloadTools::ibResourceKVPs(), {}, true);
                        }

                        const std::string prefix = component.name;
                        DownloadTools::KVPs blendRef;
                        if (component.vertexCount > 0) {
                            blendRef = DownloadTools::blendRefKVPs(component.vertexCount);
                        }

                        add(config, ModObj(component.name, "blend"), IniKeywords::Vb1, prefix + "Blend", prefix + "Blend", ".buf",
                             DownloadTools::bufResourceKVPs(config.blendStride), blendRef);
                        add(config, ModObj(component.name, "position"), IniKeywords::Vb0, prefix + "Position", prefix + "Position", ".buf",
                             DownloadTools::bufResourceKVPs(config.positionStride));
                        add(config, ModObj(component.name, "texcoord"), IniKeywords::Vb1, prefix + "Texcoord", prefix + "Texcoord", ".buf",
                             DownloadTools::bufResourceKVPs(component.texcoordStride));
                    }

                    // NO face-diffuse download, deliberately -- and this is the one place this
                    // parser differs from makeGIMICharParser rather than generalising it.
                    //
                    // A download is keyed by (mod object, REGISTER) and fires when that one
                    // register is uncovered, which works for the classic shape because every mod of
                    // a pre-6.x character binds its face diffuse to ps-t0. A 6.x skin's mods split:
                    // measured over the three test mods, the identity mod and the NSFW edit bind
                    // ps-t1 (the 6.x convention) and YelanOutfitRecolor binds ps-t0 (a port that
                    // kept the old one). Whichever register this registered, the other half of the
                    // mods would look like they were MISSING a face -- and since the fixer then
                    // renames the face diffuse onto GIMIMergeFixerConfig::faceReg, the download
                    // and the mod's own texture end up as two bindings of one register, where the
                    // download wins and replaces the face the modder drew.
                    //
                    // There is no "either register" form of a download need
                    // (GIMIParser::getDownloads checks one at a time), and a face diffuse is the
                    // one texture a character mod always ships, so the safety net is not worth a
                    // silently overwritten face. The prototype registers none either.
                }

                void add(const GIMIComponentParserConfig& config, const ModObj& modObj, const std::string& reg,
                          const std::string& kind, const std::string& file, const std::string& ext,
                          DownloadTools::KVPs resourceKVPs = {}, DownloadTools::KVPs downloadRefKVPs = {},
                          bool refToSection = false) {
                    downloadStore_.add(
                        this->downloads, modObj, reg,
                        DownloadTools::make(kind,
                                             DownloadTools::urlPath(config.downloadCharFolder, config.downloadVersionFolder,
                                                                     config.downloadPrefix, file, ext),
                                             DownloadTools::fixedFileName(config.downloadPrefix, file, ext),
                                             std::move(resourceKVPs), std::move(downloadRefKVPs), refToSection));
                }

                IniFileParseContext ctx_;
                GIMIComponentParserConfig config_;
                std::vector<std::unique_ptr<Classifier>> classifiers_;
                DownloadStore downloadStore_;
        };
    }


    IniParseBuilder::Factory makeGIMIComponentParser(GIMIComponentParserConfig config) {
        return [config](IniFile* iniFile, std::optional<int> modTypeId) {
            std::vector<ModObj> modObjs;
            for (const GIMIComponentParserConfig::Component& component : config.components) {
                for (const GIMIComponentParserConfig::Slot& slot : component.slots) {
                    modObjs.emplace_back(component.name, slot.name);
                }

                for (const char* kind : {"ib", "blend", "position", "texcoord", "other"}) {
                    modObjs.emplace_back(component.name, kind);
                }
            }

            modObjs.push_back(FaceObj);
            return std::make_shared<GIMIComponentGIMIParser>(iniFile, modTypeId, std::move(modObjs), config);
        };
    }
}
