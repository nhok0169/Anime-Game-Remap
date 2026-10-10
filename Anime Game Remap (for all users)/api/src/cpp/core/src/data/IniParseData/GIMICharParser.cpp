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

#include "AGRemapCore/data/IniParseData/GIMICharParser.h"

#include <algorithm>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "AGRemapCore/constants/IniGraphModObjKeywords.h"
#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/data/IniFixData/ModBranches.h"
#include "AGRemapCore/data/IniParseData/TextureOverrides.h"
#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/model/strategies/iniParsers/GIMIParser.h"
#include "AGRemapCore/model/strategies/iniParsers/GIMISectionClassifier.h"
#include "AGRemapCore/model/strategies/iniParsers/IniFileParseContext.h"
#include "AGRemapCore/tools/DownloadTools.h"
#include "AGRemapCore/tools/StringTools.h"
#include "AGRemapCore/tools/TextTools.h"


namespace AGRemapCore {
    namespace {
        using Parser = GIMIParser<>;
        using Classifier = Parser::Classifier;
        using ModObj = Parser::ModObj;

        // Hash-data keys, not .ini register names -- spelled literally for the same reason
        // HashToModObjData.cpp spells them that way.
        const std::string IbHashKey = "ib";
        const std::string BlendHashKey = "blend_vb";
        const std::string PositionHashKey = "position_vb";
        const std::string TexcoordHashKey = "texcoord_vb";

        // The VertexLimitRaise section: a hash type belonging to no drawn object of its own, and a
        // plain hash swap with no geometry behind it. That is what ("", "other") is for.
        const std::string DrawHashKey = "draw_vb";

        // The face's own diffuse. It gets a mod object of its own -- rather than sharing
        // ("", "other") with the above, which is where it used to live -- because the fix reaches
        // into this graph to swap the diffuse and lightmap registers, which GI 6.x swapped under
        // everyone's feet. See GIMICharFixer.
        const std::string FaceDiffuseHashKey = "tex_face_diffuse";
        const std::string TexKeyPrefix = "tex_";
        const std::string FaceObjName = "face";

        // The version VertexCountData is keyed under. Every row in that table is 4.0; it is the
        // count of the model itself, which no game version since has changed.
        const std::string VertexCountVersion = "4.0";

        const std::string ConstantsSection = "constants";
        const std::string KeySectionPrefix = "key";
        const std::string ConditionKey = "condition";

        bool isIdentChar(char c) {
            return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
        }

        // Each `$name` in 'text' through 'visit(start, end, name)', where [start, end) spans the
        // `$name`. A namespaced reference (`$\ns\name`) belongs to another file and is skipped.
        template <typename Visit>
        void forEachVar(const std::string& text, Visit&& visit) {
            for (std::size_t i = 0; i < text.size(); ++i) {
                if (text[i] != '$' || i + 1 >= text.size() || !isIdentChar(text[i + 1])) {
                    continue;
                }

                std::size_t end = i + 1;
                while (end < text.size() && isIdentChar(text[end])) {
                    ++end;
                }
                visit(i, end, text.substr(i + 1, end - i - 1));
                i = end - 1;
            }
        }

        // The variables 'text' reads, lowercased (3DMigoto's names are case-insensitive)
        std::vector<std::string> varsIn(const std::string& text) {
            std::vector<std::string> result;
            forEachVar(text, [&result](std::size_t, std::size_t, const std::string& name) {
                result.push_back(StringTools::toLower(name));
            });
            return result;
        }

        // 'text' with each variable 'renames' names (by its lowercased name) renamed
        std::string renameVars(const std::string& text, const std::map<std::string, std::string>& renames) {
            std::string result;
            std::size_t copied = 0;
            forEachVar(text, [&](std::size_t start, std::size_t end, const std::string& name) {
                auto found = renames.find(StringTools::toLower(name));
                if (found == renames.end()) {
                    return;
                }
                result += text.substr(copied, start - copied) + "$" + found->second;
                copied = end;
            });
            return result + text.substr(copied);
        }

        // The variable a `[Constants]` line declares (`global persist $color = 0` -> "color"), lowercased
        std::string declaredVar(const std::string& key) {
            const std::vector<std::string> vars = varsIn(key);
            return vars.empty() ? std::string() : vars.back();
        }

        // Every variable the `[Constants]` sections of 'templates' declare, lowercased
        std::set<std::string> declaredVars(const ModBranches::Templates& templates) {
            std::set<std::string> result;
            for (const auto& entry : templates) {
                if (entry.second == nullptr || StringTools::toLower(entry.first) != ConstantsSection) {
                    continue;
                }
                for (const auto& part : entry.second->parts()) {
                    if (const auto* content = dynamic_cast<const ModBranches::Template::ContentPart*>(part.get())) {
                        for (const auto& kvp : content->entries()) {
                            const std::string var = declaredVar(kvp.first);
                            if (!var.empty()) {
                                result.insert(var);
                            }
                        }
                    }
                }
            }
            return result;
        }

        // A toggled texture override (see GIMICharGIMIParser::readTextureOverrides): the register it
        // binds, the resource, and the `if` blocks it is bound in, outermost first
        struct CondBinding {
            std::string reg;
            std::string resource;
            std::vector<std::vector<std::string>> branches;
        };

        // `key = val`, or the bare key for a line that had no `=`
        std::string iniLine(const std::string& key, const std::string& val) {
            const std::string_view strippedVal = StringTools::strip(val);
            return strippedVal.empty() ? key : key + " = " + std::string(strippedVal);
        }


        /**
         * A GIMIParser that owns its context, its classifier AND every download it was given.
         *
         * GIMIParser's 'downloads' map holds BORROWED pointers, so the DownloadData objects have to
         * outlive the parser -- the same ownership story as the context and the classifier, and the
         * same reason a Factory subclass exists at all rather than a bare lambda.
         */
        class GIMICharGIMIParser: public Parser, public TextureOverrideFacts {
            public:
                GIMICharGIMIParser(IniFile* iniFile, std::optional<int> modTypeId, std::vector<ModObj> modObjs,
                                    std::unordered_map<std::string, ModObj> hashKeyOnlyToModObj,
                                    std::unordered_map<std::string, Classifier::IndexModObjs> indexKeyToModObj,
                                    const GIMICharParserConfig& config, long long vertexCount):
                    Parser(nullptr, std::move(modObjs)), ctx_(iniFile, modTypeId), config_(config), vertexCount_(vertexCount) {
                    this->setCtx(&ctx_);
                    this->setIniFile(iniFile);

                    // One section may name several mod objects -- see RaidenParser for the 3dmigoto
                    // grammar bug this works around.
                    this->disjointModObjs = false;

                    // Copied before the classifier takes them: the parser needs the same two maps
                    // to answer objIdentityKVPs below, and the classifier's constructor moves from
                    // these.
                    const auto hashOnlyMap = hashKeyOnlyToModObj;
                    const auto indexMap = indexKeyToModObj;

                    classifier_ = std::make_unique<Classifier>(
                        std::move(hashKeyOnlyToModObj), ctx_.modTypeHashes(),
                        std::move(indexKeyToModObj), ctx_.modTypeIndices(), ctx_.version());

                    // HASH lookups filtered to this character's own rows. A hash value is unique
                    // to one character, so for a well-formed mod this changes nothing -- what it
                    // stops is a section carrying ANOTHER character's hash of the same TYPE being
                    // read as one of this character's objects: a hand-made YelanTranquil section
                    // (her ib hash, index 0) in a Yelan mod's folder classified as Yelan's HEAD,
                    // followed its `ib = null`, and failed the whole file (2026-09-13). Every name
                    // in HashData is a ModTypeId name, so the filter can never miss a real row.
                    //
                    // The INDEX lookup is deliberately left unfiltered: an index value is shared by
                    // every character (0 is every head), and ModMappedAssets::getKey resolves it
                    // through the newest version bucket holding the value -- a bucket that need
                    // not hold this character's row at all. The object NAME is what identifies the
                    // row's tail, and that is the same for every character.
                    const std::string ownName = ctx_.modTypeName();
                    if (!ownName.empty()) {
                        classifier_->setHashNonVersionVals({ownName, std::nullopt});
                    }

                    // WHAT A SECTION FOR THIS OBJECT WOULD HAVE CARRIED, for the one case where the
                    // parser has to invent one -- see GIMIParser::objIdentityKVPs. Built from the
                    // very maps the classifier uses to recognise a real section, so the two cannot
                    // drift: whatever identifies an object on the way in is what an invented section
                    // says about itself on the way out.
                    this->objIdentityKVPs =
                        [this, hashOnlyMap, indexMap](const ModObj& modObj) {
                            std::vector<std::pair<std::string, std::string>> kvps;

                            // Asked of the context rather than captured: it is the same name the
                            // classifier's own lookups are filtered by, and asking keeps the two
                            // from drifting if the .ini is reclassified.
                            const std::string srcModName = ctx_.modTypeName();

                            auto* hashes = ctx_.modTypeHashes();
                            auto* indices = ctx_.modTypeIndices();
                            const std::optional<Version> version = ctx_.version();

                            // A DRAWN object first: identified by the shared 'ib' hash AND its own
                            // match_first_index. Checked before the hash-only map because ('', 'ib')
                            // lives in both -- see makeGIMICharParser's note on that overlap.
                            for (const auto& indexEntry : indexMap) {
                                for (const auto& objEntry : indexEntry.second) {
                                    if (objEntry.second != modObj) {
                                        continue;
                                    }

                                    if (hashes != nullptr) {
                                        std::optional<std::string> hash =
                                            hashes->get({srcModName, indexEntry.first}, version, false);
                                        if (hash.has_value()) {
                                            kvps.emplace_back(IniKeywords::Hash, *hash);
                                        }
                                    }

                                    if (indices != nullptr) {
                                        std::optional<std::string> index =
                                            indices->get({srcModName, objEntry.first.first, objEntry.first.second},
                                                          version, false);
                                        if (index.has_value()) {
                                            kvps.emplace_back(IniKeywords::MatchFirstIndex, *index);
                                        }
                                    }

                                    return kvps;
                                }
                            }

                            // Everything else -- the face, the buffers -- is named by a hash of its
                            // own and has no index at all.
                            for (const auto& hashEntry : hashOnlyMap) {
                                if (hashEntry.second != modObj || hashes == nullptr) {
                                    continue;
                                }

                                std::optional<std::string> hash =
                                    hashes->get({srcModName, hashEntry.first}, version, false);
                                if (hash.has_value()) {
                                    kvps.emplace_back(IniKeywords::Hash, *hash);
                                }

                                return kvps;
                            }

                            return kvps;
                        };

                    Classifier* classifier = classifier_.get();
                    this->objTargetFuncs.emplace_back(
                        [classifier](Parser&, const std::string& sectionName, Section* section, bool,
                                      ContentPart*, const Colouring* kvps) {
                            if (kvps == nullptr) {
                                return std::vector<ModObj>();
                            }

                            return classifier->classify(sectionName, section, *kvps);
                        });

                    buildDownloads(config, vertexCount);
                    allDownloads_ = this->downloads;
                }

                // A MOD THAT ONLY REPLACES TEXTURES (2026-10-07). A recolour may be nothing but
                // `hash = <the character's texture> / this = Resource...`, which applies wherever the
                // GAME binds that texture -- and never to a download, which is a resource of the mod's
                // own with no hash. Every object of such a mod is filled from downloads, so the
                // remap drew the vanilla model on the target and the recolour was lost, for every
                // character on this template. The override is read before anything is parsed: the
                // object's download for that texture is never registered, and the mod's own resource
                // is bound in its place (bindTextureOverrides). The multi-component skins' parser
                // does the same; see GIMIComponentParser's readTextureOverrides.
                //
                // AND A FILE WITH NOTHING OF THE CHARACTER TO REMAP GETS NO DOWNLOADS. Filled from
                // downloads, a file that only overrides UI icons, only watches the position hash
                // (a toggle or help menu), or only recolours a texture the target draws too (a face
                // two skins share), drew a second whole vanilla model over the real mod. It now keeps
                // only its own sections. So does a recolour whose mesh is drawn by a SIBLING .ini of
                // the folder: the sibling binds the recolour instead (readTextureOverrides).
                void getSectionTargets() override {
                    this->downloads = allDownloads_;
                    recolourOnly_ = false;
                    readTextureOverrides();
                    Parser::getSectionTargets();

                    if (targetsBindSomething()) {
                        return;
                    }

                    if (!needsSourceModel_ || (ownTextureOverrides_ && siblingDrawsMesh())) {
                        textureBindings_.clear();
                        condBindings_.clear();
                        carriedSections_.clear();
                        this->downloads.clear();
                        return;
                    }
                    recolourOnly_ = true;
                }

                bool isRecolourOnly() const override {
                    return recolourOnly_;
                }

                std::string carriedSections() const override {
                    return std::string(StringTools::rstrip(carriedSections_));
                }

            protected:
                void editCommands() override {
                    bindTextureOverrides();
                    declareSiblingRefs();
                    if (recolourOnly_) {
                        inventSection(ModObj("", "ib"), "IB", {{IniKeywords::Handling, "skip"}});
                        if (vertexCount_ > 0) {
                            inventSection(ModObj("", "other"), "VertexLimitRaise",
                                          {{"override_byte_stride", std::to_string(config_.positionStride)},
                                           {"override_vertex_count", std::to_string(vertexCount_)}});
                        }
                    }
                    Parser::editCommands();
                }

                // A RECOLOUR HAS NONE OF THE SECTIONS THAT TURN THE GAME'S DRAW OFF (2026-10-07). A mod
                // that draws its own mesh skips the character's draw call (`handling = skip` on its
                // ib) and raises the vertex limit; the fix copies both onto the target's hashes, so
                // the target's own model stops drawing under the remapped one. A recolour carries
                // neither, so without these the target's model drew through the downloaded one, and
                // a fixer that draws its objects through the skip section's copy (LisaStudent and
                // XianglingCheer, onto their base characters) drew nothing at all. Invented the way a
                // mod writes them, so the fix comes out in a mesh mod's shape. The component parser
                // does the same for a skin; see its inventComponentSection.
                void inventSection(const ModObj& modObj, const std::string& suffix,
                                   const std::vector<std::pair<std::string, std::string>>& extra) {
                    Graph* graph = this->getCommandGraph(modObj);
                    if (graph == nullptr || !graph->isEmpty() || !this->objIdentityKVPs) {
                        return;
                    }

                    std::vector<std::pair<std::string, std::string>> kvps = this->objIdentityKVPs(modObj);
                    if (kvps.empty()) {
                        return;
                    }
                    kvps.insert(kvps.end(), extra.begin(), extra.end());

                    const std::string name = "TextureOverride" + ctx_.modTypeName() + suffix;
                    if (ctx_.getSection(name) != nullptr) {
                        return;
                    }
                    Section* section = ctx_.addSection(name,
                        std::make_unique<Section>(std::vector<std::unique_ptr<IfTemplatePart>>{}, this->config().runConfig, name));
                    section->addKVPsToFront(kvps);
                    graph->build(std::unordered_map<std::string, Section*>{{name, section}}, std::vector<std::string>{name});
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

            private:
                // The key HashData files 'hash' under for this character (eg. tex_body_diffuse), or empty.
                std::string hashKeyOf(const std::string& hash) {
                    auto* hashes = ctx_.modTypeHashes();
                    const std::string ownName = ctx_.modTypeName();
                    if (hashes == nullptr || ownName.empty()) {
                        return "";
                    }

                    std::optional<std::vector<std::string>> key = hashes->getKey(
                        StringTools::toLower(StringTools::strip(hash)), ctx_.version(),
                        std::vector<std::optional<std::string>>{ownName, std::nullopt}, false);
                    return (!key.has_value() || key->empty()) ? std::string() : key->back();
                }

                // The (object, register) a tex_<obj>_<role> key binds at -- the register the object's
                // download for that role is registered on -- or an empty register for a texture no
                // download of this parser fills (a shadow ramp, a metal map, the face's light map).
                std::pair<ModObj, std::string> bindingOf(const std::string& key) const {
                    const std::pair<ModObj, std::string> none{ModObj("", ""), ""};
                    if (!StringTools::startsWith(key, TexKeyPrefix)) {
                        return none;
                    }

                    const std::string rest = key.substr(TexKeyPrefix.size());
                    const std::size_t sep = rest.rfind('_');
                    if (sep == std::string::npos) {
                        return none;
                    }

                    const std::string obj = rest.substr(0, sep);
                    const std::string role = rest.substr(sep + 1);
                    if (obj == FaceObjName) {
                        return (role == "diffuse" && config_.faceDownload) ? std::make_pair(ModObj("", obj), std::string("ps-t0")) : none;
                    }

                    if (std::find(config_.drawnObjs.begin(), config_.drawnObjs.end(), obj) == config_.drawnObjs.end()) {
                        return none;
                    }

                    GIMICharParserConfig::ObjDownloadRegs regs;
                    for (const auto& override_ : config_.objDownloadRegs) {
                        if (override_.obj == obj) {
                            regs = override_;
                            break;
                        }
                    }

                    const std::string reg = (role == "diffuse") ? regs.diffuseReg
                                          : (role == "lightmap") ? regs.lightMapReg
                                          : (role == "normalmap") ? regs.normalMapReg : std::string();
                    return reg.empty() ? none : std::make_pair(ModObj("", obj), reg);
                }

                // Reads the `this =` overrides of the character's own textures: this file's, and --
                // for a file that draws the mesh itself -- its siblings' (NeuvilletteMelusent1's
                // tex.ini shape: the mesh in one .ini, its recolour in another). A sibling's resource
                // cannot be named across .ini files, so it is declared again in this one under a
                // RemapRef name, which an undo removes while leaving the mod's texture alone.
                //
                // A RECOLOUR BEHIND A TOGGLE KEEPS ITS TOGGLE (2026-10-08). An override inside
                // `if $color == 0` is the mod's only while the toggle says so, and the game's texture
                // otherwise; bound unconditionally, the toggle did nothing on the target. Such an
                // override keeps the object's download and is bound after it under the same `if`
                // (bindTextureOverrides). A SIBLING's variable cannot be read from this file -- a file
                // with no `namespace =` is named after its path -- so its condition is carried under a
                // copy of the variable, which this file declares and drives with a copy of the
                // sibling's key sections (carriedSections). CherryHutao6's `left` key, on Hu Tao.
                void readTextureOverrides() {
                    textureBindings_.clear();
                    condBindings_.clear();
                    carriedSections_.clear();
                    siblingRefs_.clear();
                    ownTextureOverrides_ = false;
                    needsSourceModel_ = false;
                    IniFile* iniFile = this->getIniFile();
                    const std::string ownName = ctx_.modTypeName();
                    if (iniFile == nullptr || ctx_.modTypeHashes() == nullptr || ownName.empty()) {
                        return;
                    }

                    const ModBranches::Templates& own = iniFile->getIfTemplates();
                    const std::set<std::string> ownVars = declaredVars(own);

                    // (object, register) -> the resource to bind and the `if` blocks it is bound in;
                    // this file's first, which win
                    std::map<std::pair<ModObj, std::string>, CondBinding> bindings;
                    std::set<std::string> carriedNames;
                    auto addOverrides = [&](const ModBranches::Templates& templates,
                                            const std::function<std::string(const std::string&)>& resourceOf,
                                            bool own) {
                        std::map<std::string, std::string> carried;
                        for (const TextureOverrides::Override& override_ : TextureOverrides::collect(templates)) {
                            // Every object the texture is drawn on: one texture may serve several
                            // (Yelan's body and dress share one diffuse)
                            std::vector<std::pair<ModObj, std::string>> targets;
                            for (const std::string& key : TextureOverrides::keysOf(ownName, override_.hash)) {
                                std::pair<ModObj, std::string> target = bindingOf(key);
                                if (!target.second.empty()) {
                                    targets.push_back(std::move(target));
                                }
                            }
                            if (targets.empty()) {
                                continue;
                            }

                            const std::string resource = resourceOf(override_.resource);
                            if (resource.empty()) {
                                continue;
                            }

                            if (own) {
                                ownTextureOverrides_ = true;

                                // A texture the target draws too is recoloured there by the override
                                // itself; only one the target does not share needs the source's model.
                                // And never the FACE's: the face is a draw of its own on both
                                // characters, and its override is carried onto the target's face hash
                                // with the face section, so a face recolour (Bennett2's "adventurer
                                // face" mod) must not bring the source's whole outfit with it.
                                const bool drawnObj = std::any_of(targets.begin(), targets.end(),
                                    [](const std::pair<ModObj, std::string>& target) { return target.first.second != FaceObjName; });
                                needsSourceModel_ = needsSourceModel_ || (drawnObj && !TextureOverrides::isShared(override_.hash));
                            }
                            std::vector<std::vector<std::string>> branches = override_.branches;
                            if (!own && !branches.empty()) {
                                std::optional<std::vector<std::vector<std::string>>> renamed =
                                    carryBranches(templates, branches, ownVars, carried);
                                branches = renamed.has_value() ? std::move(*renamed) : std::vector<std::vector<std::string>>();
                            }
                            for (const auto& target : targets) {
                                bindings.emplace(target, CondBinding{target.second, resource, branches});
                            }
                        }

                        if (!own) {
                            carrySections(templates, carried, ownVars, carriedNames);
                        }
                    };

                    addOverrides(own, [](const std::string& resource) { return resource; }, true);

                    if (drawsMesh(own)) {
                        for (const std::string& path : TextureOverrides::siblingInis(iniFile)) {
                            IniFile sibling(path);
                            const ModBranches::Templates& templates = sibling.getIfTemplates();
                            addOverrides(templates, [&](const std::string& resource) {
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
                            }, false);
                        }
                    }

                    for (const auto& [target, binding] : bindings) {
                        if (!binding.branches.empty()) {
                            condBindings_[target.first].push_back(binding);
                            continue;
                        }

                        auto objDownloads = this->downloads.find(target.first);
                        if (objDownloads != this->downloads.end()) {
                            objDownloads.value().erase(target.second);
                        }
                        textureBindings_[target.first].emplace_back(target.second, binding.resource);
                    }
                }

                // Whether 'templates' draws this character's mesh: a section matching one of its
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

                        const std::string key = hashKeyOf(*hash);
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
                    return false;
                }

                bool siblingDrawsMesh() {
                    for (const std::string& path : TextureOverrides::siblingInis(this->getIniFile())) {
                        IniFile sibling(path);
                        if (drawsMesh(sibling.getIfTemplates())) {
                            return true;
                        }
                    }
                    return false;
                }

                // Whether any of the mod's OWN sections this parser targets -- or a command list one of
                // them runs, if the file defines it -- binds a buffer, an index buffer, a texture
                // register or a draw. A `this =` is not counted: an override is weighed on its own
                // (readTextureOverrides). Asked before the downloads; see getSectionTargets.
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

                // Each sibling texture a binding names, as a resource of this file's own -- see
                // readTextureOverrides. Only those some object binds.
                void declareSiblingRefs() {
                    refGraphs_.clear();
                    std::set<std::string> bound;
                    for (const auto& [modObj, bindings] : textureBindings_) {
                        for (const auto& binding : bindings) {
                            bound.insert(binding.second);
                        }
                    }
                    bound.insert(condBound_.begin(), condBound_.end());

                    for (const auto& [name, file] : siblingRefs_) {
                        if (bound.count(name) == 0) {
                            continue;
                        }

                        // An existing one is the PREVIOUS fix's, read before its undo, and is the right
                        // section to render again.
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

                // At the TOP of the object's entry section, where a download is bound too: a GIMI
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

                    condBound_.clear();
                    for (const auto& [modObj, bindings] : condBindings_) {
                        bindConditionally(modObj, bindings);
                    }
                }

                // A toggled recolour (see readTextureOverrides), at the top of the object's entry
                // section: the download for the register first, then the recolour under the
                // override's own `if` blocks --
                //
                //     ps-t1 = Resource<Mod>HeadDiffuseRemapDL
                //     if $colorRemapRef == 0
                //         ps-t1 = ResourceHuTaoCherryHeadDiffuseRemapRef
                //     endif
                //
                // -- so whichever branch the toggle is in, the register is bound before the section
                // draws, and every later register edit sees both. A register with no download there
                // is one the mod binds itself, which a `this =` never reaches on the source either.
                void bindConditionally(const ModObj& modObj, const std::vector<CondBinding>& bindings) {
                    Graph* graph = this->getCommandGraph(modObj);
                    if (graph == nullptr || graph->isEmpty() || graph->roots().empty()) {
                        return;
                    }
                    Section* section = ctx_.getSection(graph->roots().front());
                    if (section == nullptr || section->parts().empty()) {
                        return;
                    }
                    auto* first = dynamic_cast<ContentPart*>(section->parts().front().get());
                    if (first == nullptr) {
                        return;
                    }

                    using Kvps = std::vector<std::pair<std::string, std::string>>;
                    Kvps downloaded;
                    std::vector<std::pair<std::vector<std::vector<std::string>>, Kvps>> groups;
                    for (const CondBinding& binding : bindings) {
                        std::optional<std::string> download;
                        for (const std::string& val : first->getVals(binding.reg)) {
                            const std::string stripped(StringTools::strip(val));
                            if (StringTools::endsWith(stripped, IniKeywords::RemapDL)) {
                                download = stripped;
                                break;
                            }
                        }
                        if (!download.has_value()) {
                            continue;
                        }

                        const std::string downloadName = *download;
                        first->removeKey(binding.reg, std::nullopt, [&downloadName](long long, const std::string& val) {
                            return std::string(StringTools::strip(val)) == downloadName;
                        });
                        downloaded.emplace_back(binding.reg, downloadName);

                        auto group = std::find_if(groups.begin(), groups.end(),
                            [&binding](const auto& g) { return g.first == binding.branches; });
                        if (group == groups.end()) {
                            groups.emplace_back(binding.branches, Kvps{});
                            group = std::prev(groups.end());
                        }
                        group->second.emplace_back(binding.reg, binding.resource);
                        condBound_.insert(binding.resource);
                    }
                    if (groups.empty()) {
                        return;
                    }

                    std::optional<Z3Context> fallbackZ3Ctx;
                    Z3Context* z3Ctx = graph->z3Ctx();
                    if (z3Ctx == nullptr) {
                        fallbackZ3Ctx.emplace();
                        z3Ctx = &(*fallbackZ3Ctx);
                    }

                    std::vector<std::unique_ptr<IfTemplatePart>> front;
                    front.push_back(std::make_unique<ContentPart>(downloaded, 0));
                    for (const auto& [branches, binds] : groups) {
                        for (const std::vector<std::string>& level : branches) {
                            for (const std::string& header : level) {
                                const std::optional<IfPredPartType> type = IfPredPartTypeTools::getType(header);
                                front.push_back(std::make_unique<IfPredPart>(header, type.value_or(IfPredPartType::If), *z3Ctx));
                            }
                        }
                        front.push_back(std::make_unique<ContentPart>(binds, static_cast<int>(branches.size())));
                        for (std::size_t i = 0; i < branches.size(); ++i) {
                            front.push_back(std::make_unique<IfPredPart>(
                                IfPredPartTypeTools::getName(IfPredPartType::EndIf), IfPredPartType::EndIf, *z3Ctx));
                        }
                    }

                    auto& parts = section->parts();
                    if (first->size() == 0) {
                        parts.erase(parts.begin());
                    }
                    parts.insert(parts.begin(), std::make_move_iterator(front.begin()), std::make_move_iterator(front.end()));
                    section->rebuild();
                }

                // Whether a `[Key...]` section of 'templates' sets the variable 'var' (lowercased)
                static bool keySets(const ModBranches::Templates& templates, const std::string& var) {
                    for (const auto& entry : templates) {
                        if (entry.second == nullptr || !StringTools::startsWith(StringTools::toLower(entry.first), KeySectionPrefix)) {
                            continue;
                        }
                        for (const auto& part : entry.second->parts()) {
                            const auto* content = dynamic_cast<const ModBranches::Template::ContentPart*>(part.get());
                            if (content == nullptr) {
                                continue;
                            }
                            for (const auto& kvp : content->entries()) {
                                if (StringTools::toLower(StringTools::strip(kvp.first)) == "$" + var) {
                                    return true;
                                }
                            }
                        }
                    }
                    return false;
                }

                // A SIBLING override's `if` blocks, reading this file's copies of the sibling's
                // variables -- or nothing when one cannot be read here. A variable the sibling
                // declares and one of its key sections sets (a toggle) is carried, as
                // `$<name>RemapRef`; one this file declares itself is read as it is (`$active`).
                static std::optional<std::vector<std::vector<std::string>>> carryBranches(
                        const ModBranches::Templates& sibling, const std::vector<std::vector<std::string>>& branches,
                        const std::set<std::string>& ownVars, std::map<std::string, std::string>& carried) {
                    const std::set<std::string> siblingVars = declaredVars(sibling);
                    std::map<std::string, std::string> renames;
                    for (const std::vector<std::string>& level : branches) {
                        for (const std::string& header : level) {
                            for (const std::string& var : varsIn(header)) {
                                if (renames.count(var) != 0) {
                                    continue;
                                }
                                if (siblingVars.count(var) != 0 && keySets(sibling, var)) {
                                    renames.emplace(var, var + IniKeywords::RemapRef);
                                } else if (ownVars.count(var) == 0) {
                                    return std::nullopt;
                                }
                            }
                        }
                    }

                    std::vector<std::vector<std::string>> result;
                    for (const std::vector<std::string>& level : branches) {
                        std::vector<std::string>& out = result.emplace_back();
                        for (const std::string& header : level) {
                            out.push_back(renameVars(header, renames));
                        }
                    }
                    carried.insert(renames.begin(), renames.end());
                    return result;
                }

                // The copies 'carried' names (lowercased sibling variable -> copy): a `[Constants]`
                // declaring each the way the sibling does, and each sibling key section that sets one,
                // as `[<its name>RemapRef]`. A key's other variables are dropped, and so is a
                // `condition` reading one this file cannot see -- the key then works everywhere.
                void carrySections(const ModBranches::Templates& sibling, const std::map<std::string, std::string>& carried,
                                   const std::set<std::string>& ownVars, std::set<std::string>& carriedNames) {
                    if (carried.empty()) {
                        return;
                    }

                    std::set<std::string> visible = ownVars;
                    for (const auto& entry : carried) {
                        visible.insert(StringTools::toLower(entry.second));
                    }

                    std::string constants;
                    for (const auto& entry : sibling) {
                        if (entry.second == nullptr || StringTools::toLower(entry.first) != ConstantsSection) {
                            continue;
                        }
                        for (const auto& part : entry.second->parts()) {
                            const auto* content = dynamic_cast<const ModBranches::Template::ContentPart*>(part.get());
                            if (content == nullptr) {
                                continue;
                            }
                            for (const auto& kvp : content->entries()) {
                                const std::string var = declaredVar(kvp.first);
                                if (carried.count(var) != 0 && carriedNames.insert("$" + var).second) {
                                    constants += iniLine(renameVars(std::string(StringTools::strip(kvp.first)), carried), kvp.second) + "\n";
                                }
                            }
                        }
                    }
                    if (!constants.empty()) {
                        carriedSections_ += "[Constants]\n" + constants + "\n";
                    }

                    for (const auto& entry : sibling) {
                        if (entry.second == nullptr || !StringTools::startsWith(StringTools::toLower(entry.first), KeySectionPrefix)) {
                            continue;
                        }

                        std::string body;
                        bool setsCarried = false;
                        for (const auto& part : entry.second->parts()) {
                            const auto* content = dynamic_cast<const ModBranches::Template::ContentPart*>(part.get());
                            if (content == nullptr) {
                                continue;
                            }
                            for (const auto& kvp : content->entries()) {
                                const std::string key(StringTools::strip(kvp.first));
                                if (StringTools::startsWith(key, "$")) {
                                    auto found = carried.find(StringTools::toLower(key.substr(1)));
                                    if (found != carried.end()) {
                                        body += iniLine("$" + found->second, kvp.second) + "\n";
                                        setsCarried = true;
                                    }
                                    continue;
                                }

                                if (StringTools::toLower(key) == ConditionKey) {
                                    const std::string condition = renameVars(kvp.second, carried);
                                    const std::vector<std::string> vars = varsIn(condition);
                                    if (!std::all_of(vars.begin(), vars.end(), [&visible](const std::string& var) { return visible.count(var) != 0; })) {
                                        continue;
                                    }
                                    body += iniLine(key, condition) + "\n";
                                    continue;
                                }
                                body += iniLine(key, kvp.second) + "\n";
                            }
                        }

                        const std::string name = entry.first + IniKeywords::RemapRef;
                        if (setsCarried && carriedNames.insert(StringTools::toLower(name)).second) {
                            carriedSections_ += "[" + name + "]\n" + body + "\n";
                        }
                    }
                }

                /**
                 * The default parts a modder may have left out, so a fix can still reference them.
                 *
                 * Per-object file downloads and per-.buf-kind downloads share one 'downloads' map,
                 * keyed by mod object throughout: the .buf kinds ARE mod objects here (the parser
                 * classifies blend/position/texcoord in their own right), so no information is lost.
                 *
                 * The shape is the same for every character this builds: two textures and an index
                 * buffer per drawn object, the three buffers, and the face. Only the names differ.
                 */
                void buildDownloads(const GIMICharParserConfig& config, long long vertexCount) {
                    for (const std::string& obj : config.drawnObjs) {
                        // "head" is the mod object; "Head" is the middle of the file name AND of the
                        // .ini resource name.
                        //
                        // THE OBJECT HAS TO BE IN THE .INI NAME, not just the file name.
                        // createDownloadResource builds the resource section as
                        // "<modType><download name>" and dedupes on it, so calling these "Diffuse"
                        // gave every drawn object the same section: head's and body's diffuse
                        // downloads collapsed into one [Resource<Mod>DiffuseRemapDL], one of them
                        // silently winning. Naming them "HeadDiffuse"/"BodyDiffuse" separates them
                        // and matches the old script, which emits [Resource<Mod>BodyDiffuseRemapDL].
                        const std::string file = TextTools::capitalize(obj);

                        // refToSection: referenced ONCE from the top of each root section the
                        // register does not fully cover, which is what the pure-Python original
                        // does -- its output carries all three on
                        // [TextureOverride<Mod><Obj><Target>RemapFix].
                        //
                        // Per-part placement (the default) only agrees with that while every
                        // branch of a CommandList binds every register, as Ningguang's single
                        // if-block does. GanyuTwilight's body is two sequential if-blocks -- six
                        // branches binding ib, then two binding ps-t0/ps-t1 -- so per-part gave
                        // the $Tight branches an ib they should not have and the ib branches a
                        // ps-t0 they should not have. The draw call is derived from ib, so the two
                        // stray ib lines became two stray drawindexed: 8 draws instead of 6, and
                        // ORFix/NNFix run 8 times instead of 2. ORFix swaps the diffuse and
                        // lightmap registers per call, so the surplus swaps turned the model green
                        // and yellow.
                        // ps-t0/ps-t1 unless this object says otherwise -- see
                        // GIMICharParserConfig::objDownloadRegs for why a 4.0-era character does.
                        GIMICharParserConfig::ObjDownloadRegs regs;
                        for (const auto& override_ : config.objDownloadRegs) {
                            if (override_.obj == obj) {
                                regs = override_;
                                break;
                            }
                        }

                        // Before the diffuse, matching the order the pure-Python table lists them
                        // in -- the .ini file's sections come out in the order they were added.
                        if (!regs.normalMapReg.empty()) {
                            add(config, {"", obj}, regs.normalMapReg, file + "NormalMap", file + "NormalMap", ".dds",
                                 {}, {}, true);
                        }

                        add(config, {"", obj}, regs.diffuseReg, file + "Diffuse", file + "Diffuse", ".dds",
                             {}, {}, true);
                        // ...unless there is no lightmap upstream to fetch -- see
                        // GIMICharParserConfig::objsWithoutLightMap.
                        if (std::find(config.objsWithoutLightMap.begin(), config.objsWithoutLightMap.end(), obj)
                                == config.objsWithoutLightMap.end()) {
                            add(config, {"", obj}, regs.lightMapReg, file + "LightMap", file + "LightMap", ".dds",
                                 {}, {}, true);
                        }
                        add(config, {"", obj}, IniKeywords::Ib, file + "Ib", file, ".ib",
                             DownloadTools::ibResourceKVPs(), {}, true);

                        // A texture download any of the object's cover registers satisfies -- see
                        // GIMICharParserConfig::ObjDownloadRegs::coverRegs.
                        if (!regs.coverRegs.empty()) {
                            for (const std::string& reg : {regs.normalMapReg, regs.diffuseReg, regs.lightMapReg}) {
                                if (reg.empty()) {
                                    continue;
                                }
                                std::vector<std::string> alts;
                                for (const std::string& cover : regs.coverRegs) {
                                    if (cover != reg) {
                                        alts.push_back(cover);
                                    }
                                }
                                this->setDownloadAltRegs({"", obj}, reg, std::move(alts));
                            }
                        }
                    }

                    // The face diffuse. Named for the character rather than for an object, so
                    // 'FaceDiffuse' serves as both the .ini name and the middle of the file name.
                    //
                    // Filled on the register the mod WOULD have used, ps-t0, and left for the
                    // fixer's swap to move to ps-t1 along with everything else in the graph. The
                    // parser adds a download's KVP straight into the part it is missing from
                    // (GIMIParser::addDownloads), so by the time the fix runs a downloaded diffuse
                    // is indistinguishable from one the mod shipped -- which is exactly what makes
                    // the ordering work.
                    // ...from wherever this character's face diffuse actually lives, which for three
                    // of them is not where the rest of their assets do -- see
                    // GIMICharParserConfig::faceDownloadVersionFolder.
                    GIMICharParserConfig faceConfig = config;
                    if (!config.faceDownloadVersionFolder.empty()) {
                        faceConfig.downloadVersionFolder = config.faceDownloadVersionFolder;
                    }

                    if (!config.faceDownloadPrefix.empty()) {
                        faceConfig.downloadPrefix = config.faceDownloadPrefix;
                    }

                    if (config.faceDownload) {
                        add(faceConfig, {"", "face"}, "ps-t0", "FaceDiffuse", "FaceDiffuse", ".dds");
                    }

                    // Per buffer kind. The blend is the one that needs downloadRefKVPs -- see
                    // DownloadTools::blendRefKVPs for why a downloaded Blend.buf has to be drawn
                    // by hand.
                    add(config, {"", "blend"}, IniKeywords::Vb1, IniKeywords::Blend, "Blend", ".buf",
                         DownloadTools::bufResourceKVPs(config.blendStride),
                         DownloadTools::blendRefKVPs(vertexCount));
                    add(config, {"", "position"}, IniKeywords::Vb0, IniKeywords::Position, "Position", ".buf",
                         DownloadTools::bufResourceKVPs(config.positionStride));
                    add(config, {"", "texcoord"}, IniKeywords::Vb1, IniKeywords::Texcoord, "Texcoord", ".buf",
                         DownloadTools::bufResourceKVPs(config.texcoordStride));
                }

                /**
                 * One download, named the same way on both sides.
                 *
                 * 'kind' is what the download is called in the .ini file it creates; 'file' is the
                 * middle of the file name on GitHub and on disk. They differ where the file name
                 * carries the object ("HeadDiffuse") but the .ini name does not ("Diffuse").
                 */
                void add(const GIMICharParserConfig& config, const ModObj& modObj, const std::string& reg,
                          const std::string& kind, const std::string& file, const std::string& ext,
                          DownloadTools::KVPs resourceKVPs = {}, DownloadTools::KVPs downloadRefKVPs = {},
                          bool refToSection = false) {
                    downloadStore_.add(
                        this->downloads, modObj, reg,
                        DownloadTools::make(kind,
                                             DownloadTools::urlPath(config.downloadCharFolder,
                                                                     config.downloadVersionFolder,
                                                                     config.downloadPrefix, file, ext),
                                             DownloadTools::fixedFileName(config.downloadPrefix, file, ext),
                                             std::move(resourceKVPs), std::move(downloadRefKVPs),
                                             refToSection));
                }

                IniFileParseContext ctx_;
                GIMICharParserConfig config_;

                // The game model's vertex count, for a recolour's invented VertexLimitRaise
                long long vertexCount_;
                std::unique_ptr<Classifier> classifier_;
                DownloadStore downloadStore_;

                // Every download the constructor registered: a parse drops the ones a texture override
                // replaces, and the next parse of the same file starts from all of them again.
                Parser::Downloads allDownloads_;

                // object -> (register, the mod's resource) for each texture override it binds
                std::map<ModObj, std::vector<std::pair<std::string, std::string>>> textureBindings_;

                // A sibling .ini's recolour -- see readTextureOverrides: the RemapRef resource name ->
                // the file it names, and the graphs that declare the ones an object binds.
                std::map<std::string, std::string> siblingRefs_;
                std::vector<std::pair<std::string, Graph*>> refGraphs_;

                // Whether this file overrides any of the character's textures, and any the target
                // does not draw itself.
                // object -> its toggled overrides, and the resources bindConditionally bound
                std::map<ModObj, std::vector<CondBinding>> condBindings_;
                std::set<std::string> condBound_;

                // What carriedSections() hands the fixer
                std::string carriedSections_;

                bool ownTextureOverrides_ = false;
                bool needsSourceModel_ = false;
                bool recolourOnly_ = false;
        };
    }


    IniParseBuilder::Factory makeGIMICharParser(GIMICharParserConfig config) {
        // FOUR kinds of mod object, and only the first needs anything the classifier does not
        // already do on its own:
        //
        //  * the drawn objects all share one 'ib' and are told apart by the match_first_index that
        //    follows it -- each keyed by the last two index columns of its own Indices row,
        //    (component, object)
        //  * blend/position/texcoord are each named outright by their own hash
        //  * ("", "ib") is the section carrying the 'ib' hash and NO match_first_index at all --
        //    the shared draw call the objects run into. GIMISectionClassifier already does this:
        //    with "ib" in BOTH maps, a part whose index matches resolves to a drawn object, and one
        //    with no index falls through to the hash-only entry. See its classify(), where
        //    'inHashOnly' is what catches the no-index case
        //  * ("", "other") and ("", "face") are each named by a hash of their own, like the buffers
        std::vector<ModObj> ibModObjs;
        for (const std::string& obj : config.drawnObjs) {
            ibModObjs.emplace_back("", obj);
        }

        const ModObj otherObj{"", "other"};
        const ModObj faceObj{"", "face"};
        const std::vector<std::pair<std::string, ModObj>> hashOnlyObjs = {
            {IbHashKey, {"", "ib"}},
            {BlendHashKey, {"", "blend"}},
            {PositionHashKey, {"", "position"}},
            {TexcoordHashKey, {"", "texcoord"}},
            {DrawHashKey, otherObj},
            {FaceDiffuseHashKey, faceObj}};

        // Deduplicated, because the map above may be many-hash-types-to-one-object: an object
        // appearing twice there must appear once here, or the parser is handed a duplicate graph.
        std::vector<ModObj> modObjs = ibModObjs;
        for (const auto& entry : hashOnlyObjs) {
            if (std::find(modObjs.begin(), modObjs.end(), entry.second) == modObjs.end()) {
                modObjs.push_back(entry.second);
            }
        }

        Classifier::IndexModObjs indexModObjs;
        for (const ModObj& modObj : ibModObjs) {
            indexModObjs.emplace(Classifier::IndexKey(modObj.first, modObj.second), modObj);
        }

        const std::unordered_map<std::string, ModObj> hashKeyOnlyToModObj(hashOnlyObjs.begin(), hashOnlyObjs.end());
        const std::unordered_map<std::string, Classifier::IndexModObjs> indexKeyToModObj = {
            {IbHashKey, std::move(indexModObjs)}};

        // Baked in once, exactly as the pure-Python original bakes 'VertexCountData[4.0][<char>]'
        // into its own table.
        const long long vertexCount = DownloadTools::vertexCountOf(
            VertexCountVersion, ModTypeIdTools::getName(config.modTypeId), "");

        return [modObjs, hashKeyOnlyToModObj, indexKeyToModObj, config, vertexCount](
                   IniFile* iniFile, std::optional<int> modTypeId) {
            return std::make_shared<GIMICharGIMIParser>(iniFile, modTypeId, modObjs, hashKeyOnlyToModObj,
                                                         indexKeyToModObj, config, vertexCount);
        };
    }
}
