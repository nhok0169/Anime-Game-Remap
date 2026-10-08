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
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <algorithm>
#include <filesystem>
#include <functional>

#include "AGRemapCore/constants/IniGraphModObjKeywords.h"
#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniFixData/ModBranches.h"
#include "AGRemapCore/data/IniFixData/RegValChecks.h"
#include "AGRemapCore/data/IniParseData/TextureOverrides.h"
#include "AGRemapCore/model/IniNamingTools.h"
#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/model/strategies/iniParsers/GIMIParser.h"
#include "AGRemapCore/model/strategies/iniParsers/IniFileParseContext.h"
#include "AGRemapCore/tools/DownloadTools.h"
#include "AGRemapCore/tools/StringTools.h"
#include "AGRemapCore/tools/files/FileService.h"


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
        const std::string TexKeyPrefix = "tex_";

        const ModObj FaceObj{"", "face"};


        /**
         * A mod of a skin of several components, classified one component at a time.
         *
         * One classifier per component, each filtered to that component's own mod type name, because
         * a multi-component skin files each component's hashes under the COMPONENT's name and a
         * classifier's hash filter can only name one. A hash value is unique to one character, so a
         * section is answered by at most one of them.
         */
        class GIMIComponentGIMIParser: public Parser, public GIMIComponentParseFacts {
            public:
                GIMIComponentGIMIParser(IniFile* iniFile, std::optional<int> modTypeId, std::vector<ModObj> modObjs,
                                         const GIMIComponentParserConfig& config):
                    Parser(nullptr, std::move(modObjs)), ctx_(iniFile, modTypeId), config_(config) {
                    this->setCtx(&ctx_);
                    this->setIniFile(iniFile);

                    // One section may name several mod objects -- see RaidenParser.
                    this->disjointModObjs = false;

                    // A slot that DRAWS before it binds its textures needs the download for that
                    // draw -- see GIMIParser::ParserConfig::drawKey.
                    this->setDrawKey(IniKeywords::DrawIndexed);

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

            protected:
                // A MOD WITH NO COMPONENT-LEVEL ib OR VertexLimitRaise SECTION STILL NEEDS THEM
                // (2026-09-23). GIMIMergeFixer makes the TARGET's `handling = skip` and its vertex
                // limit out of the mod's own component-level sections (see its donorFor), and the
                // parser invents a section only for a download -- which the component-level ib and
                // `other` objects never have. A texture-only CharlotteHurlock mod (two texture hash
                // overrides, every buffer downloaded) therefore came out with no skip at all:
                // Charlotte's own head drew through her own index buffer over the merged vertices,
                // torn triangles over the legs and a red strip down one side.
                //
                // Only when NO component carries one, and then for the first component the mod
                // (or its downloads) gives a slot: a mod that has one keeps exactly what it had.
                void editCommands() override {
                    bindTextureOverrides();
                    declareSiblingRefs();
                    if (hasContent_) {
                        for (const std::string kind : {"ib", "other"}) {
                            inventComponentSection(kind);
                        }
                    }
                    Parser::editCommands();
                }

                // A SKIN RECOLOURED BY TEXTURE HASH ALONE (2026-09-24). A mod may replace a slot's
                // texture with nothing but `hash = <the game's texture> / this = Resource...`, which
                // applies wherever the GAME binds that texture -- and never to a download, which is
                // the mod's own resource with its own hash. Remapped, every slot of such a mod comes
                // from downloads, so CharlotteHurlock3's recolour was dropped and base Charlotte
                // wore the plain skin. The override is found before anything else is parsed, so the
                // slot's download for that role is never registered and the mod's own resource is
                // bound in its place (bindTextureOverrides).
                //
                // AND A FILE THAT ONLY WATCHES THE SKIN IS TOLD APART HERE, before any download is
                // registered: once setupDownloads has run, a download for a register the mod's own
                // section lacks has been ADDED to that section, and ToggleMenu.ini's position watcher
                // looks like it binds vb0. Such a file gets no downloads at all, so nothing is
                // invented and the fix carries only its own sections -- see hasRemappableContent.
                void getSectionTargets() override {
                    readTextureOverrides();
                    if (config_.downloadsByName) {
                        placeDownloadsByName();
                    }
                    Parser::getSectionTargets();

                    const bool bindsSomething = targetsBindSomething();
                    hasContent_ = !textureBindings_.empty() || bindsSomething;

                    // A RECOLOUR BESIDE ITS OWN MESH (2026-09-25). A file with nothing but texture
                    // overrides is remapped by drawing the whole skin from downloads in the recolour's
                    // textures -- right for a mod that IS only a recolour, and a second whole model over
                    // the real one when a sibling .ini draws the mesh (NeuvilletteMelusent1's tex.ini).
                    // That sibling takes the recolour over (readTextureOverrides), so this file keeps only
                    // its own sections, as a file that watches the skin does.
                    if (!bindsSomething && ownTextureOverrides_ && siblingDrawsMesh()) {
                        textureBindings_.clear();
                        hasContent_ = false;
                    }
                    if (!hasContent_) {
                        this->downloads.clear();
                    }

                    findUndrawnSlots();
                }

            public:
                bool hasRemappableContent() const override {
                    return hasContent_;
                }

                bool isSlotUndrawn(const std::string& component, const std::string& slot) const override {
                    return undrawnSlots_.count(ModObj(component, slot)) != 0;
                }

            private:
                static std::string lowered(std::string_view text) {
                    return StringTools::toLower(text);
                }

                // (component;slot;role) -> the mod's resource, for every `this =` override whose
                // hash is one of the skin's tex_<slot>_<role> rows. Then, per slot that DRAWS with
                // that texture (its own, or its donor's -- the game draws a borrowing slot with the
                // donor's texture, so an override of that hash recolours it too), the register it is
                // bound at, with that slot's download for it dropped.
                // The key (the hash's row, last value) a hash is filed under for one of this skin's
                // components, or empty.
                std::string hashKeyOf(const std::string& hash, const GIMIComponentParserConfig::Component& component) {
                    auto* hashes = ctx_.modTypeHashes();
                    if (hashes == nullptr || component.modTypeName.empty()) {
                        return "";
                    }

                    std::optional<std::vector<std::string>> key = hashes->getKey(
                        lowered(StringTools::strip(hash)), ctx_.version(),
                        std::vector<std::optional<std::string>>{component.modTypeName, std::nullopt}, false);
                    return (!key.has_value() || key->empty()) ? std::string() : key->back();
                }

                // Every `this =` override in 'templates' whose hash is one of the skin's tex_<slot>_<role>
                // rows, as (component;slot;role) -> the resource to bind. 'bindingOf' turns the override's
                // own resource name into that value; an empty answer drops the override.
                void collectTextureOverrides(const ModBranches::Templates& templates,
                                             const std::function<std::string(const std::string&)>& bindingOf,
                                             std::unordered_map<std::string, std::string>& overrides) {
                    for (const TextureOverrides::Override& override_ : TextureOverrides::collect(templates)) {
                        for (const GIMIComponentParserConfig::Component& component : config_.components) {
                            // Every slot the texture is filed under: two slots may draw one texture
                            // (YelanTranquil's Body B and C), and the override recolours both.
                            for (const std::string& key : TextureOverrides::keysOf(component.modTypeName, override_.hash)) {
                                // tex_<slot>_<role>
                                const std::string rest = key.substr(TexKeyPrefix.size());
                                const std::size_t sep = rest.rfind('_');
                                const std::string binding = bindingOf(override_.resource);
                                if (sep == std::string::npos || binding.empty()) {
                                    continue;
                                }
                                overrides.emplace(component.name + ";" + rest.substr(0, sep) + ";" + rest.substr(sep + 1), binding);
                            }
                        }
                    }
                }

                // Whether 'templates' draws this skin's mesh: a section matching one of its components'
                // buffer or index hashes that binds a buffer.
                bool drawsMesh(const ModBranches::Templates& templates) {
                    for (const auto& entry : templates) {
                        if (entry.second == nullptr) {
                            continue;
                        }

                        const std::optional<std::string> hash = ModBranches::firstVal(*entry.second, IniKeywords::Hash);
                        if (!hash.has_value()) {
                            continue;
                        }

                        for (const GIMIComponentParserConfig::Component& component : config_.components) {
                            const std::string key = hashKeyOf(*hash, component);
                            if (key.empty() || StringTools::startsWith(key, TexKeyPrefix)) {
                                continue;
                            }
                            for (const std::string& reg : {IniKeywords::Ib, IniKeywords::Vb0, IniKeywords::Vb1}) {
                                const std::optional<std::string> value = ModBranches::firstVal(*entry.second, reg);
                                if (value.has_value() && !ModBranches::resourceOf(value).empty()) {
                                    return true;
                                }
                            }
                        }
                    }
                    return false;
                }

                // The other .ini files of the mod's folder the game loads: not this one, not a DISABLED
                // one, and not a copy a fix wrote (<name>RemapFix<N>.ini).
                std::vector<std::string> siblingInis() const {
                    return TextureOverrides::siblingInis(const_cast<GIMIComponentGIMIParser*>(this)->getIniFile());
                }

                // Whether this file's own sections draw the skin's mesh.
                bool targetsDrawMesh() {
                    IniFile* iniFile = this->getIniFile();
                    return iniFile != nullptr && drawsMesh(iniFile->getIfTemplates());
                }

                bool siblingDrawsMesh() {
                    for (const std::string& path : siblingInis()) {
                        IniFile sibling(path);
                        if (drawsMesh(sibling.getIfTemplates())) {
                            return true;
                        }
                    }
                    return false;
                }

                // A SLOT WRITTEN IN THE GAME'S REGISTER ORDER -- see GIMIComponentParserConfig::downloadsByName.
                // Runs before the downloads are applied, on the slot's own entry section: its hash is the
                // component's ib and its match_first_index the slot's.
                void placeDownloadsByName() {
                    IniFile* iniFile = this->getIniFile();
                    if (iniFile == nullptr || ctx_.modTypeHashes() == nullptr) {
                        return;
                    }

                    const ModBranches::Templates& templates = iniFile->getIfTemplates();
                    for (const GIMIComponentParserConfig::Component& component : config_.components) {
                        for (const GIMIComponentParserConfig::Slot& slot : component.slots) {
                            if (slot.noTextures) {
                                continue;
                            }

                            auto objDownloads = this->downloads.find(ModObj(component.name, slot.name));
                            if (objDownloads == this->downloads.end()) {
                                continue;
                            }

                            // role -> the register the slot expects it on
                            std::vector<std::pair<std::string, std::string>> roleRegs{{"diffuse", slot.diffuseReg}, {"lightmap", slot.lightMapReg}};
                            if (!slot.normalMapReg.empty()) {
                                roleRegs.emplace_back("normalmap", slot.normalMapReg);
                            }

                            for (const auto& entry : templates) {
                                if (entry.second == nullptr) {
                                    continue;
                                }

                                const std::optional<std::string> hash = ModBranches::firstVal(*entry.second, IniKeywords::Hash);
                                const std::optional<std::string> index = ModBranches::firstVal(*entry.second, IniKeywords::MatchFirstIndex);
                                if (!hash.has_value() || !index.has_value() || hashKeyOf(*hash, component) != IbHashKey
                                        || StringTools::strip(*index) != slot.index) {
                                    continue;
                                }

                                // register -> role, of every texture the section binds at one of the slot's registers
                                std::unordered_map<std::string, std::string> boundRoles;
                                std::set<std::string> roles;
                                bool believed = true;
                                for (const auto& [role, reg] : roleRegs) {
                                    const std::string res(ModBranches::resourceOf(ModBranches::firstVal(*entry.second, reg)));
                                    if (res.empty()) {
                                        continue;
                                    }

                                    const char* named = RegValChecks::isNormalMap(res) ? "normalmap"
                                                      : RegValChecks::isLightMap(res) ? "lightmap"
                                                      : RegValChecks::isDiffuse(res) ? "diffuse" : nullptr;
                                    believed = believed && named != nullptr && roles.insert(named).second;
                                    if (named != nullptr) {
                                        boundRoles.emplace(reg, named);
                                    }
                                }

                                // nothing bound, a name that says nothing, or two alike: the register reading stands
                                if (boundRoles.empty() || !believed) {
                                    break;
                                }

                                bool gameOrder = false;
                                for (const auto& [reg, role] : boundRoles) {
                                    for (const auto& [expectedRole, expectedReg] : roleRegs) {
                                        gameOrder = gameOrder || (expectedRole == role && expectedReg != reg);
                                    }
                                }
                                if (!gameOrder) {
                                    break;
                                }

                                // Decided first, applied after, so a moved download is never read again
                                // under the role of the register it moved to.
                                auto& regDownloads = objDownloads.value();
                                std::vector<std::pair<std::string, DownloadData*>> kept;
                                std::vector<DownloadData*> homeless;
                                for (const auto& [role, reg] : roleRegs) {
                                    auto download = regDownloads.find(reg);
                                    if (download == regDownloads.end() || roles.count(role) != 0) {
                                        continue;   // none registered, or the mod brings this role itself under another register
                                    }
                                    if (boundRoles.count(reg) != 0) {
                                        homeless.push_back(download->second);   // its register holds another role's texture
                                    } else {
                                        kept.emplace_back(reg, download->second);
                                    }
                                }

                                for (const auto& [role, reg] : roleRegs) {
                                    regDownloads.erase(reg);
                                }
                                for (const auto& [reg, data] : kept) {
                                    regDownloads[reg] = data;
                                }
                                auto home = homeless.begin();
                                for (const auto& [role, reg] : roleRegs) {
                                    if (home == homeless.end()) {
                                        break;
                                    }
                                    if (boundRoles.count(reg) == 0 && regDownloads.count(reg) == 0) {
                                        regDownloads[reg] = *home;
                                        ++home;
                                    }
                                }
                                break;
                            }
                        }
                    }
                }

                void readTextureOverrides() {
                    textureBindings_.clear();
                    siblingRefs_.clear();
                    ownTextureOverrides_ = false;
                    IniFile* iniFile = this->getIniFile();
                    auto* hashes = ctx_.modTypeHashes();
                    if (iniFile == nullptr || hashes == nullptr) {
                        return;
                    }

                    std::unordered_map<std::string, std::string> overrides;
                    collectTextureOverrides(iniFile->getIfTemplates(), [](const std::string& resource) { return resource; },
                                            overrides);
                    ownTextureOverrides_ = !overrides.empty();

                    // A RECOLOUR IN A SIBLING FILE (2026-09-25): NeuvilletteMelusent1 is its mesh in one
                    // .ini and `this =` overrides of the skin's textures in another (tex.ini), which on the
                    // skin reach the mesh because the game binds those textures. Remapped, the mesh's slots
                    // take the skin's textures from downloads and nothing would reach them, so the
                    // sibling's overrides are read here too -- only by a file that draws the mesh itself,
                    // and after this file's own, which win. A section name cannot be read across .ini
                    // files, so each file is declared again under a resource of this file's own, named
                    // with RemapRef: an undo removes the section and leaves the mod's texture.
                    if (targetsDrawMesh()) {
                        for (const std::string& path : siblingInis()) {
                            IniFile sibling(path);
                            const ModBranches::Templates& templates = sibling.getIfTemplates();
                            collectTextureOverrides(templates, [&](const std::string& resource) {
                                auto found = templates.find(resource);
                                if (found == templates.end() || found->second == nullptr) {
                                    return std::string();
                                }
                                const std::optional<std::string> file = ModBranches::firstVal(*found->second, IniKeywords::Filename);
                                if (!file.has_value() || StringTools::strip(*file).empty()) {
                                    return std::string();
                                }

                                const std::string name = resource + IniKeywords::RemapRef;
                                siblingRefs_.emplace(name, std::string(StringTools::strip(*file)));
                                return name;
                            }, overrides);
                        }
                    }

                    if (overrides.empty()) {
                        return;
                    }

                    for (const GIMIComponentParserConfig::Component& component : config_.components) {
                        for (const GIMIComponentParserConfig::Slot& slot : component.slots) {
                            std::string source = component.name + ";" + lowered(slot.name);
                            if (slot.noTextures) {
                                const std::size_t sep = slot.textureDonor.find(';');
                                if (sep == std::string::npos) {
                                    continue;
                                }
                                source = slot.textureDonor.substr(0, sep) + ";" + lowered(slot.textureDonor.substr(sep + 1));
                            }

                            const ModObj modObj(component.name, slot.name);
                            for (const auto& [role, reg] : std::vector<std::pair<std::string, std::string>>{
                                     {"diffuse", slot.diffuseReg}, {"lightmap", slot.lightMapReg}, {"normalmap", slot.normalMapReg}}) {
                                auto found = overrides.find(source + ";" + role);
                                if (reg.empty() || found == overrides.end()) {
                                    continue;
                                }

                                auto objDownloads = this->downloads.find(modObj);
                                if (objDownloads != this->downloads.end()) {
                                    objDownloads.value().erase(reg);
                                }
                                textureBindings_[modObj].emplace_back(reg, found->second);
                            }
                        }
                    }
                }

                // Each sibling texture a binding names, as a resource of this file's own -- see
                // readTextureOverrides. Only those some slot binds.
                void declareSiblingRefs() {
                    refGraphs_.clear();
                    std::set<std::string> bound;
                    for (const auto& [modObj, bindings] : textureBindings_) {
                        for (const auto& binding : bindings) {
                            bound.insert(binding.second);
                        }
                    }

                    for (const auto& [name, file] : siblingRefs_) {
                        if (bound.count(name) == 0) {
                            continue;
                        }

                        // An existing one is the PREVIOUS fix's, read before its undo, and is the right
                        // section to render again -- skipping it declared nothing on every later run.
                        Section* section = ctx_.getSection(name);
                        if (section == nullptr) {
                            section = ctx_.addSection(name,
                                std::make_unique<Section>(std::vector<std::unique_ptr<IfTemplatePart>>{}, this->config().runConfig, name));
                            section->addKVPsToFront(std::vector<std::pair<std::string, std::string>>{{IniKeywords::Filename, file}});
                        }

                        Graph* graph = ctx_.graphGroups().createGraph({}, {}, false, nullptr);
                        graph->build(std::unordered_map<std::string, Section*>{{name, section}}, std::vector<std::string>{name});
                        refGraphs_.emplace_back(name, graph);
                    }
                }

                // The sibling textures' resources ride with the downloads' -- declared once per .ini
                // file, and by every copy that binds one (GIMIFixer::groupToStr).
                std::vector<Parser::GraphGroup> collectParseResult() const override {
                    std::vector<Parser::GraphGroup> result = Parser::collectParseResult();
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

                // At the TOP of the slot's entry section, the place a download is bound too: a GIMI
                // TextureOverride binds for its whole draw, and one the mod wrote itself that binds
                // the register as well keeps the last word.
                void bindTextureOverrides() {
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

                // Whether any of the mod's OWN sections this parser targets -- or a command list one of
                // them runs, if the file defines it -- binds something: a buffer, an index buffer, a
                // texture register or a draw. Asked before the downloads; see getSectionTargets.
                //
                // NOT a `this` (2026-10-07): the one targeted section that can carry one is the face's
                // texture override, and a face is a draw of its own on both characters. Counted, a file
                // that only recolours the skin's face (Bennett2's BennettSkin.ini) drew the whole skin
                // from downloads over the base character; it now keeps its own sections, the face
                // override retargeted onto the target's face hash. Slot texture overrides are weighed
                // apart, in readTextureOverrides.
                bool targetsBindSomething() {
                    IniFile* iniFile = this->getIniFile();
                    if (iniFile == nullptr) {
                        return true;
                    }
                    const auto& templates = iniFile->getIfTemplates();

                    static const std::vector<std::string> BindingKeys = {
                        IniKeywords::Vb0, IniKeywords::Vb1, "vb2", IniKeywords::Ib, IniKeywords::DrawIndexed,
                        "draw", "ps-t0", "ps-t1", "ps-t2", "ps-t3"};

                    std::unordered_set<std::string> visited;
                    std::vector<std::string> toVisit;
                    for (const auto& entry : this->sectionTargets()) {
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

                // A SLOT THE MOD LEAVES UNDRAWN GETS NO DOWNLOAD (2026-09-24). A mod that carries a
                // component's buffers and skips its index buffer draws exactly the slots it writes a
                // section for; the rest are never drawn on its own character. Downloading the game's
                // index buffer for one of those and drawing it over the MOD's vertices -- in the
                // game's vertex order, which the mod's buffers do not keep -- is stretched shards:
                // CharlotteHurlock1 moved its slot C geometry into its slot B section and declared no
                // C or D, and the downloaded C and D pulled its skirt up to its chest. A component the
                // mod does not carry, or does not skip, keeps its downloads: there the game draws the
                // slot too. Asked by GIMIMergeFixer through GIMIComponentParseFacts::isSlotUndrawn.
                void findUndrawnSlots() {
                    undrawnSlots_.clear();
                    IniFile* iniFile = this->getIniFile();
                    if (!hasContent_ || iniFile == nullptr) {
                        return;
                    }

                    const auto& templates = iniFile->getIfTemplates();
                    const auto& targets = this->sectionTargets();
                    auto sectionsOf = [&](const ModObj& modObj) -> const std::vector<std::string>* {
                        auto found = targets.find(modObj);
                        return (found == targets.end() || found->second.empty()) ? nullptr : &found->second;
                    };

                    for (const GIMIComponentParserConfig::Component& component : config_.components) {
                        const std::vector<std::string>* ibSections = sectionsOf(ModObj(component.name, "ib"));
                        if (sectionsOf(ModObj(component.name, "position")) == nullptr || ibSections == nullptr) {
                            continue;
                        }

                        bool skips = false;
                        for (const std::string& name : *ibSections) {
                            auto found = templates.find(name);
                            if (found == templates.end() || found->second == nullptr) {
                                continue;
                            }
                            const std::optional<std::string> handling = ModBranches::firstVal(*found->second, IniKeywords::Handling);
                            if (handling.has_value() && StringTools::equalsIgnoreCase(StringTools::strip(*handling), "skip")) {
                                skips = true;
                                break;
                            }
                        }
                        if (!skips) {
                            continue;
                        }

                        for (const GIMIComponentParserConfig::Slot& slot : component.slots) {
                            const ModObj modObj(component.name, slot.name);
                            if (sectionsOf(modObj) != nullptr) {
                                continue;
                            }
                            this->downloads.erase(modObj);
                            undrawnSlots_.insert(modObj);
                        }
                    }
                }

                void inventComponentSection(const std::string& kind) {
                    const GIMIComponentParserConfig::Component* owner = nullptr;
                    for (const GIMIComponentParserConfig::Component& component : config_.components) {
                        Graph* graph = this->getCommandGraph(ModObj(component.name, kind));
                        if (graph != nullptr && !graph->isEmpty()) {
                            return;
                        }

                        if (owner != nullptr) {
                            continue;
                        }
                        for (const GIMIComponentParserConfig::Slot& slot : component.slots) {
                            Graph* slotGraph = this->getCommandGraph(ModObj(component.name, slot.name));
                            if (slotGraph != nullptr && !slotGraph->isEmpty()) {
                                owner = &component;
                                break;
                            }
                        }
                    }

                    if (owner == nullptr || !this->objIdentityKVPs) {
                        return;
                    }

                    const ModObj modObj(owner->name, kind);
                    Graph* graph = this->getCommandGraph(modObj);
                    std::vector<std::pair<std::string, std::string>> kvps = this->objIdentityKVPs(modObj);
                    if (graph == nullptr || kvps.empty()) {
                        return;
                    }
                    if (kind == "ib") {
                        kvps.emplace_back(IniKeywords::Handling, "skip");
                    }

                    // Named the way a mod names these (and identityMod.py writes them), so the fix's
                    // copy comes out as `...<Component><Target>RemapIB`, the same as for a mod
                    // that carries its own.
                    const std::string name = "TextureOverride" + ctx_.modTypeName() + owner->name
                                             + (kind == "ib" ? "IB" : "VertexLimitRaise");
                    if (ctx_.getSection(name) != nullptr) {
                        return;
                    }
                    Section* section = ctx_.addSection(name,
                        std::make_unique<Section>(std::vector<std::unique_ptr<IfTemplatePart>>{}, this->config().runConfig, name));
                    section->addKVPsToFront(kvps);
                    graph->build(std::unordered_map<std::string, Section*>{{name, section}}, std::vector<std::string>{name});
                }

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

                                    // See Slot::donorNormalMap.
                                    if (slot.donorNormalMap && !slot.normalMapReg.empty()) {
                                        add(config, modObj, slot.normalMapReg, donor + "NormalMap", donor + "NormalMap", ".dds",
                                             {}, {}, true);
                                    }
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
                std::map<ModObj, std::vector<std::pair<std::string, std::string>>> textureBindings_;
                bool hasContent_ = true;

                // A sibling .ini's recolour -- see readTextureOverrides: the RemapRef resource name ->
                // the file it names, and the graphs that declare the ones a slot binds.
                std::map<std::string, std::string> siblingRefs_;
                std::vector<std::pair<std::string, Graph*>> refGraphs_;
                bool ownTextureOverrides_ = false;
                std::set<ModObj> undrawnSlots_;
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
