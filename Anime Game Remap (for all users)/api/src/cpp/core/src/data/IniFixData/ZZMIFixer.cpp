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

#include "AGRemapCore/data/IniFixData/ZZMIFixer.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "AGRemapCore/constants/FileExt.h"
#include "AGRemapCore/data/IniFixData/ModBranches.h"
#include "AGRemapCore/model/buffers/BufFloat.h"
#include "AGRemapCore/model/buffers/BufUnorm.h"
#include "AGRemapCore/model/files/IniScan.h"
#include "AGRemapCore/model/iftemplate/IfPredPart.h"
#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/model/IniNamingTools.h"
#include "AGRemapCore/model/VGRemap.h"
#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/model/files/TextureFile.h"
#include "AGRemapCore/model/iftemplate/IfTemplateRender.h"
#include "AGRemapCore/model/iniresources/RemapBlendResource.h"
#include "AGRemapCore/model/iniresources/RemapIniResource.h"
#include "AGRemapCore/model/iniresources/RemapTexResource.h"
#include "AGRemapCore/model/strategies/ModType.h"
#include "AGRemapCore/model/strategies/iniFixers/GIMIFixer.h"
#include "AGRemapCore/model/strategies/iniFixers/IniFileFixContext.h"
#include "AGRemapCore/model/strategies/iniFixers/graphEdits/GraphRename.h"
#include "AGRemapCore/model/strategies/iniFixers/graphEdits/RegBottomAdd.h"
#include "AGRemapCore/model/strategies/iniFixers/graphEdits/RegIfTemplateAdd.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphCopy.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphFilter.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphInherit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphCreate.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphGroupEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphGroupPartEdits.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphGroupRemap.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphGroupRemove.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/ResRegCollect.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/BlendEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/BufRestrideEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/FileAddEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/FileDownloadEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/TexEditorEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/VGRemapBlendEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegAssetRemap.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegNewVals.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegRemap.h"
#include "AGRemapCore/model/strategies/iniParsers/BaseIniParser.h"
#include "AGRemapCore/model/strategies/iniParsers/GIMIParser.h"
#include "AGRemapCore/model/strategies/iniParsers/IniFileParseContext.h"
#include "AGRemapCore/model/strategies/texEditors/TexEditor.h"
#include "AGRemapCore/model/strategies/texEditors/texFilters/MaterialBandRemapFilter.h"
#include "AGRemapCore/tools/DownloadTools.h"
#include "AGRemapCore/tools/StringTools.h"
#include "AGRemapCore/tools/TextTools.h"
#include "AGRemapCore/tools/z3/Z3Context.h"
#include "AGRemapCore/tools/files/FileDownload.h"
#include "AGRemapCore/tools/files/FileService.h"


namespace AGRemapCore {
    namespace {
        using Parser = GIMIParser<>;
        using Classifier = Parser::Classifier;
        using Fixer = GIMIFixer<>;
        using ModObj = Fixer::ModObj;
        using GraphId = BaseIniGraphGroupEdit<>::GraphId;
        using Collector = ResRegCollect<>;
        using ObjGroupEdit = GraphGroupEdit<>;

        // NOT 'GroupRemap': GIMIFixer inherits an alias of that name -- see GIMICharFixer
        using ObjGroupRemap = GraphGroupRemap<>;

        // The hash types that name a component's graphs, and the graph each sorts into
        const std::vector<std::pair<std::string, std::string>> HashKinds = {
            {"ib", "ib"}, {"blend_vb", "blend"}, {"texcoord_vb", "texcoord"}, {"draw_vb", "vertexlimit"},
            {"position_vb", "position"}};

        const std::string Vb2 = "vb2";
        const std::string ZZMIResource = "Resource\\ZZMI\\";
        const std::string LightMapRole = "lightmap";
        const std::string DiffuseRole = "diffuse";

        // ---- names ----

        // A component's mod object: the ZZZ component goes into the OBJECT name and the mod object's own
        // component stays "" -- a non-empty component means "the same object drawn more than once" to
        // the graph groups
        ModObj objOf(const std::string& comp, const std::string& name) {
            return {"", comp + "_" + name};
        }

        std::vector<std::string> compObjs(const ZZMIFixerConfig::Component& c) {
            std::vector<std::string> objs;
            for (const ZZMIFixerConfig::Slot& slot : c.slots) {
                objs.push_back(slot.name);
            }
            for (const auto& kind : HashKinds) {
                objs.push_back(kind.second);
            }
            for (const ZZMIFixerConfig::Texture& tex : c.textures) {
                if (std::find(objs.begin(), objs.end(), "tex_" + tex.role) == objs.end()) {
                    objs.push_back("tex_" + tex.role);
                }
            }
            return objs;
        }

        std::string capitalised(const std::string& role) {
            if (role == "lightmap") return "LightMap";
            if (role == "materialmap") return "MaterialMap";
            if (role == "normalmap") return "NormalMap";
            return TextTools::capitalize(role);
        }

        // `ref X` -> `X`: a register naming a resource may say `ref`
        std::string removeRef(const std::string& value) {
            return IniNamingTools::removeRefPrefix(value);
        }

        // The library's names append the tag (<name><tag>), which reorders sections of one hash and
        // first index -- and 3DMigoto runs those in NAME order. A mod may split one draw over two
        // (LegsA draws the legs, LegsA2 rebinds vb0 / vb1 to the Body and draws it): "LegsA<tag>"
        // sorts after "LegsA2<tag>" ('2' < 'M'), and LegsA then drew the legs' indices over the
        // Body's vertices. The tag goes after "TextureOverride" instead.
        std::string orderedName(const std::string& name, const std::string& fixed) {
            const std::string prefix = IniKeywords::TextureOverride;
            if (fixed.size() < name.size() || fixed.compare(0, name.size(), name) != 0
                    || name.compare(0, prefix.size(), prefix) != 0) {
                return fixed;
            }
            return prefix + fixed.substr(name.size()) + name.substr(prefix.size());
        }

        BaseResEdit<>::ResEditConfig resEditConfig() {
            return BaseResEdit<>::ResEditConfig{
                IniKeywords::Filename,
                [](const std::string& value) { return value; },
                [](const std::string& file) { return file; }};
        }

        // ---- the mod's own sections ----

        using Template = ModBranches::Template;

        // The mod's own sections, as the .ini file parsed to them: every one but those a previous fix of
        // this pair wrote (their names carry `<target><component>Remap`)
        std::vector<const Template*> ownSections(IniFile* ini, const ZZMIFixerConfig& config) {
            std::vector<const Template*> out;
            if (ini == nullptr) {
                return out;
            }
            std::vector<std::string> fixed = {config.target + IniKeywords::Remap};
            for (const ZZMIFixerConfig::Component& c : config.components) {
                fixed.push_back(config.target + c.target + IniKeywords::Remap);
            }
            for (const auto& entry : ini->getIfTemplates()) {
                const bool isFix = std::any_of(fixed.begin(), fixed.end(), [&entry](const std::string& f) {
                    return entry.first.find(f) != std::string::npos; });
                if (!isFix && entry.second != nullptr) {
                    out.push_back(entry.second.get());
                }
            }
            return out;
        }

        // A section's KVPs in order, through every branch
        std::vector<std::pair<std::string, std::string>> kvpsOf(const Template& tpl) {
            std::vector<std::pair<std::string, std::string>> out;
            for (const auto& part : tpl.parts()) {
                if (const auto* content = dynamic_cast<const IfContentPart<std::string, std::string>*>(part.get())) {
                    for (const auto& kv : content->entries()) {
                        out.emplace_back(std::string(StringTools::strip(kv.first)), std::string(StringTools::strip(kv.second)));
                    }
                }
            }
            return out;
        }

        std::optional<std::string> valOf(const Template& tpl, const std::string& key) {
            return ModBranches::firstValLoose(tpl, key);
        }

        std::string lowered(const std::string& s) {
            return StringTools::toLower(s);
        }

        // {resource section: (file name, declared stride)} -- a borrowed texcoord is told by its stride
        std::unordered_map<std::string, std::pair<std::string, int>> declaredStrides(const std::vector<const Template*>& own) {
            std::unordered_map<std::string, std::pair<std::string, int>> out;
            for (const Template* tpl : own) {
                if (tpl->name.compare(0, IniKeywords::Resource.size(), IniKeywords::Resource) != 0) {
                    continue;
                }
                const std::optional<std::string> stride = valOf(*tpl, IniKeywords::Stride);
                const std::optional<std::string> file = valOf(*tpl, IniKeywords::Filename);
                if (stride.has_value() && file.has_value()) {
                    std::string name = *file;
                    std::replace(name.begin(), name.end(), '\\', '/');
                    name = lowered(name.substr(name.find_last_of('/') == std::string::npos ? 0 : name.find_last_of('/') + 1));
                    out[tpl->name] = {name, std::atoi(stride->c_str())};
                }
            }
            return out;
        }

        // ---- the texcoord rewrite ----

        // ZZZ's texcoord layouts: 20 bytes is COLOR, TEXCOORD0 (half2), TEXCOORD1 (float2), TEXCOORD2
        // (half2); 24 appends a TEXCOORD3 (half2) that equals TEXCOORD0 on most vertices -- so narrowing
        // drops it and widening copies TEXCOORD0 into it (TexcoordFill)
        std::vector<std::unique_ptr<BufElementType>> texcoordLayout(int stride) {
            const auto half = []() -> std::unique_ptr<BufDataType> { return std::make_unique<BufFloat16>(); };
            std::vector<std::unique_ptr<BufElementType>> layout;
            layout.push_back(BufElementType::repeated("COLOR", 4, []() -> std::unique_ptr<BufDataType> {
                return std::make_unique<BufUnorm>("UNORM", 1); }, "R8G8B8A8_UNORM"));
            layout.push_back(BufElementType::repeated("TEXCOORD", 2, half, "R16G16_FLOAT"));
            layout.push_back(BufElementType::repeated("TEXCOORD1", 2, []() -> std::unique_ptr<BufDataType> {
                return std::make_unique<BufFloat>(); }, "R32G32_FLOAT"));
            layout.push_back(BufElementType::repeated("TEXCOORD2", 2, half, "R16G16_FLOAT"));
            if (stride >= 24) {
                layout.push_back(BufElementType::repeated("TEXCOORD3", 2, half, "R16G16_FLOAT"));
            }
            return layout;
        }

        const std::unordered_map<std::string, std::string> TexcoordFill = {{"TEXCOORD3", "TEXCOORD"}};

        // ---- the band edit ----

        // A light map's RED channel is a material band: each texel ON a band (within the tolerance) moves
        // per the component's band moves; a mod's in-between reds keep their value. The levels sit further
        // apart than twice the tolerance, so each band is one range of MaterialBandRemapFilter's, and its
        // every decision is made from the ORIGINAL red (the moves may be a permutation)
        TexEditor::Filter bandEdit(const ZZMIFixerConfig& config, const std::map<int, int>& moves) {
            std::vector<MaterialBandRemapFilter::Band> bands;
            for (int level : config.bandLevels) {
                const auto move = moves.find(level);
                if (move == moves.end() || move->second == level) {
                    continue;
                }
                bands.emplace_back(static_cast<std::uint8_t>(std::max(0, level - config.bandTolerance)),
                                   static_cast<std::uint8_t>(std::min(255, level + config.bandTolerance)),
                                   static_cast<std::uint8_t>(move->second));
            }
            return [bands](TextureFile& texFile) {
                MaterialBandRemapFilter(bands, "", 0).transform(texFile);
            };
        }

        // ---- the draw ranges two components' sections draw (a doubled mesh) ----

        struct DrawRange {
            bool all = true;
            long long start = 0;
            long long end = 0;
        };

        DrawRange drawRange(const std::string& value) {
            std::vector<std::string> parts;
            std::stringstream ss(value);
            std::string part;
            while (std::getline(ss, part, ',')) {
                parts.push_back(std::string(StringTools::strip(part)));
            }
            if (parts.size() < 2 || StringTools::equalsIgnoreCase(parts[0], "auto")) {
                return {};
            }
            const long long count = std::atoll(parts[0].c_str());
            const long long start = std::atoll(parts[1].c_str());
            return {false, start, start + count};
        }

        bool overlaps(const DrawRange& r, const std::vector<DrawRange>& ranges) {
            for (const DrawRange& o : ranges) {
                if (o.all || r.all || (r.start < o.end && o.start < r.end)) {
                    return true;
                }
            }
            return false;
        }

        // {ib resource: ranges} the mod's own sections on 'ibHash' draw, following each section's `ib =`
        // lines in order; a section with no `ib =` of its own draws whatever ib the hash's other sections
        // bound, credited to it when there is exactly one
        std::unordered_map<std::string, std::vector<DrawRange>> rangesDrawnBy(const std::vector<const Template*>& own,
                                                                             const std::string& ibHash) {
            std::vector<const Template*> sections;
            std::set<std::string> bound;
            for (const Template* tpl : own) {
                const std::optional<std::string> hash = valOf(*tpl, IniKeywords::Hash);
                if (!hash.has_value() || !StringTools::equalsIgnoreCase(*hash, ibHash)) {
                    continue;
                }
                sections.push_back(tpl);
                for (const auto& kv : kvpsOf(*tpl)) {
                    if (StringTools::equalsIgnoreCase(kv.first, IniKeywords::Ib) && !StringTools::equalsIgnoreCase(removeRef(kv.second), IniKeywords::Null)) {
                        bound.insert(removeRef(kv.second));
                    }
                }
            }
            const std::optional<std::string> inherited = bound.size() == 1 ? std::optional<std::string>(*bound.begin()) : std::nullopt;
            std::unordered_map<std::string, std::vector<DrawRange>> out;
            for (const Template* tpl : sections) {
                std::optional<std::string> cur = inherited;
                for (const auto& kv : kvpsOf(*tpl)) {
                    if (StringTools::equalsIgnoreCase(kv.first, IniKeywords::Ib)) {
                        cur = removeRef(kv.second);
                    } else if (StringTools::equalsIgnoreCase(kv.first, IniKeywords::DrawIndexed) && cur.has_value()
                               && !StringTools::equalsIgnoreCase(*cur, IniKeywords::Null)) {
                        out[*cur].push_back(drawRange(kv.second));
                    }
                }
            }
            return out;
        }

        // ---- the lines a cross-slot move carries ----

        bool isTextureLine(const std::string& key) {
            return (key.size() > ZZMIResource.size() && StringTools::equalsIgnoreCase(key.substr(0, ZZMIResource.size()), ZZMIResource))
                || (key.size() > 4 && StringTools::equalsIgnoreCase(key.substr(0, 4), "ps-t"));
        }

        bool isSetTextures(const std::pair<std::string, std::string>& kv) {
            return StringTools::equalsIgnoreCase(kv.first, IniKeywords::Run)
                && StringTools::equalsIgnoreCase(kv.second, "CommandList\\ZZMI\\SetTextures");
        }


        /**
         * The parser of a ZZMI character's mods: one classifier per component, filtered to that
         * component's own hash rows (every component carries the same hash TYPES, so a type alone
         * cannot say which one a section is), and a drawn object resolved from the component's slots
         */
        class ZZMIGIMIParser: public Parser {
            public:
                ZZMIGIMIParser(IniFile* iniFile, std::optional<int> modTypeId, std::vector<ModObj> modObjs,
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


        /**
         * The fixer of one source component onto its target component -- see ZZMIFixerConfig
         */
        class ZZMIFixerImpl: public Fixer {
            public:
                ZZMIFixerImpl(BaseIniParser<>* parser, const std::string& toModName, std::optional<int> modTypeId,
                               ZZMIFixerConfig config, const std::string& component):
                    Fixer(parser, nullptr, {},
                           toModName.empty() ? std::optional<std::vector<std::string>>(std::nullopt)
                                             : std::optional<std::vector<std::string>>({toModName}),
                           nullptr, makeConfig()),
                    ctx_(parser != nullptr ? parser->getIniFile() : nullptr, modTypeId), toModName_(toModName),
                    config_(std::move(config)) {
                    this->setCtx(&ctx_);

                    for (const ZZMIFixerConfig::Component& c : config_.components) {
                        if (c.name == component) {
                            comp_ = c;
                        }
                    }

                    own_ = ownSections(ctx_.getIniFile(), config_);
                    buildCrossSlotEdits();
                    buildRemap();
                    buildBlendCollect();
                    buildTexcoordCollects();
                    buildBandEdits();
                    buildObjEdits();
                    buildFallbacks();
                    buildExtras();

                    this->graphGroupEdits.clear();
                    for (auto& edit : preRemapEdits_) {
                        this->graphGroupEdits.push_back(edit.get());
                    }
                    this->graphGroupEdits.push_back(remap_.get());
                    for (auto& collect : collects_) {
                        this->graphGroupEdits.push_back(collect.get());
                    }
                    for (auto& edit : groupEdits_) {
                        this->graphGroupEdits.push_back(edit.get());
                    }
                    for (auto& edit : crossEdits_) {
                        this->graphGroupEdits.push_back(edit.get());
                    }
                    for (auto& edit : extrasEdits_) {
                        this->graphGroupEdits.push_back(edit.get());
                    }

                    this->copyPreamble = config_.copyPreamble;
                }

            protected:
                // GIMIFixer's own hands every group edit a nullptr .ini file, which stops a collect ever
                // building anything -- see GIMICharFixerImpl
                void applyGraphGroupEdits(const std::string& modName) override {
                    if (this->graphGroups() == nullptr) {
                        return;
                    }
                    for (Fixer::GroupEdit* edit : this->graphGroupEdits) {
                        if (edit != nullptr) {
                            edit->editFromIni(*this->graphGroups(), ctx_.getIniFile(), nullptr, modName);
                        }
                    }
                }

            private:
                static FixerConfig makeConfig() {
                    FixerConfig config{};
                    config.sectionToStr = &renderIfTemplate;
                    return config;
                }

                std::string srcName() const { return config_.source + comp_.name; }
                std::string targetName() const { return config_.target + comp_.target; }

                std::optional<Version> toVersion() const {
                    IniFile* ini = ctx_.getIniFile();
                    return ini == nullptr ? std::nullopt : ini->toVersion;
                }

                std::optional<std::string> hashOf(const std::string& name, const std::string& type) const {
                    Hashes* hashes = ctx_.modTypeHashes();
                    if (hashes == nullptr) {
                        return std::nullopt;
                    }
                    return hashes->get({name, type}, ctx_.version(), false);
                }

                // ---- 1. this pair's graphs only: every other component's are some other fixer's ----
                void buildRemap() {
                    ObjGroupRemap::RemapList remap;
                    for (const ZZMIFixerConfig::Component& other : config_.components) {
                        for (const std::string& obj : compObjs(other)) {
                            std::vector<ObjGroupRemap::RemapTarget> targets;
                            if (other.name == comp_.name) {
                                targets.emplace_back(GraphId(0, "", other.name + "_" + obj),
                                                     [](const std::string& name) { return name; });
                            }
                            remap.emplace_back(GraphId(0, "", other.name + "_" + obj), std::move(targets));
                        }
                    }
                    for (const GraphId& made : crossGraphs_) {
                        remap.emplace_back(made, std::vector<ObjGroupRemap::RemapTarget>{
                            ObjGroupRemap::RemapTarget(made, [](const std::string& name) { return name; })});
                    }
                    remap_ = std::make_unique<ObjGroupRemap>(std::move(remap));
                }

                // ---- 2. the blend, wherever the component's sections bind it ----
                //
                // A mod may run its pre-skin pass (`vb2 = ...` + `draw = <n>,0`) from the BLEND section or
                // from the POSITION section; a blend left unremapped there skins with the source's bone
                // indices on the target's palette, which shatters the model
                void buildBlendCollect() {
                    auto replace = std::make_unique<VGRemapBlendReplace<>>(
                        GraphId(0, "", comp_.name + "_RemapBlend"), resEditConfig(), ctx_.modType(), ctx_.version(),
                        toVersion(), "blend", std::nullopt, comp_.name, comp_.target);
                    replace->rowModName = config_.target;

                    auto collect = std::make_unique<Collector>();
                    collect->srcRegs = {{GraphId(0, "", comp_.name + "_blend"), Vb2},
                                        {GraphId(0, "", comp_.name + "_position"), Vb2}};
                    collect->resEdits = {{IniKeywords::Blend, replace.get()}};
                    resEdits_.push_back(std::move(replace));
                    collects_.push_back(std::move(collect));
                }

                // ---- 3. the texcoord: its own where the strides differ, and one an ib section BORROWS ----
                void buildTexcoordCollects() {
                    const int fromStride = comp_.srcTexcoordStride;
                    const int toStride = comp_.targetTexcoordStride;
                    std::unordered_map<std::string, int> strideOfFile;

                    if (fromStride != toStride) {
                        addTexcoordCollect({GraphId(0, "", comp_.name + "_texcoord")}, "RemapTexcoord", fromStride, toStride,
                                           strideOfFile, false);
                    }

                    // A texcoord an ib section borrows from another component (the Body's, drawn from the
                    // Legs section), or that the Blend / Position section binds for the draw passes: cut
                    // when its DECLARED stride is not the target's
                    std::vector<GraphId> holders;
                    for (const ZZMIFixerConfig::Slot& slot : comp_.slots) {
                        holders.emplace_back(0, "", comp_.name + "_" + slot.name);
                    }
                    holders.emplace_back(0, "", comp_.name + "_blend");
                    holders.emplace_back(0, "", comp_.name + "_position");
                    addTexcoordCollect(holders, "RemapBorrowedTexcoord", toStride == 20 ? 24 : 20, toStride, strideOfFile, true);
                }

                void addTexcoordCollect(const std::vector<GraphId>& graphs, const std::string& resName, int fromStride,
                                         int toStride, std::unordered_map<std::string, int> strideOfFile, bool onlyOtherStrides) {
                    (void)strideOfFile;
                    const GraphId resGraph(0, "", comp_.name + "_" + resName);
                    auto replace = std::make_unique<BufRestrideReplace<>>(resGraph, resEditConfig(), "texcoord",
                                                                          texcoordLayout(fromStride), texcoordLayout(toStride), TexcoordFill);
                    auto collect = std::make_unique<Collector>();
                    for (const GraphId& graph : graphs) {
                        collect->srcRegs[graph] = IniKeywords::Vb1;
                    }
                    collect->resEdits = {{"texcoord", replace.get()}};

                    // a file is rewritten from the stride it DECLARES: the component's own (or none) for its
                    // own texcoord, the other one for a texcoord it borrows
                    const auto declared = declaredStrides(own_);
                    for (const GraphId& graph : graphs) {
                        collect->resPredicates[graph] = [declared, fromStride, onlyOtherStrides](
                                const std::string&, const std::string& val, const Collector::IterData&) {
                            const auto found = declared.find(removeRef(val));
                            if (onlyOtherStrides) {
                                return found != declared.end() && found->second.second == fromStride;
                            }
                            return found == declared.end() || found->second.second == fromStride;
                        };
                    }

                    // ...and the section the collect writes still DECLARES the source's stride
                    auto stride = std::make_unique<RegNewVals<>>(
                        std::vector<std::pair<std::string, RegNewVals<>::NewValSpec>>{{IniKeywords::Stride, RegNewVals<>::NewVal(std::to_string(toStride))}});
                    auto adapter = std::make_unique<RegPartEdit<>>(stride.get());
                    ObjGroupEdit::IniEdits iniEdits;
                    iniEdits.edits[resGraph.modObj] = {adapter.get()};
                    auto strideEdit = std::make_unique<ObjGroupEdit>(std::vector<ObjGroupEdit::IniEdits>{iniEdits});

                    resEdits_.push_back(std::move(replace));
                    collects_.push_back(std::move(collect));
                    regEdits_.push_back(std::move(stride));
                    partEdits_.push_back(std::move(adapter));
                    groupEdits_.push_back(std::move(strideEdit));
                }

                // ---- 3b. THE BAND EDIT on every light map the component binds ----
                void buildBandEdits() {
                    if (comp_.bandMoves.empty()) {
                        return;
                    }

                    const std::string zzmiLightMap = ZZMIResource + "LightMap";
                    struct Ref { std::string graph; std::string reg; std::string res; };
                    std::vector<Ref> refs = {{comp_.name + "_tex_" + LightMapRole, IniKeywords::This, "RemapTexLightMap"}};
                    for (const ZZMIFixerConfig::Slot& slot : comp_.slots) {
                        refs.push_back({comp_.name + "_" + slot.name, zzmiLightMap, slot.name + "RemapTexLightMap"});
                        refs.push_back({comp_.name + "_" + slot.name, config_.roleRegs.at(LightMapRole), slot.name + "RemapTexLightMapT"});
                    }

                    // The `ref` comes off for the collect, which looks the value up as a section name, and goes
                    // back on after it (without it 3DMigoto COPIES the resource rather than referencing it)
                    auto stripRef = std::make_unique<RegNewVals<>>(std::vector<std::pair<std::string, RegNewVals<>::NewValSpec>>{
                        {zzmiLightMap, RegNewVals<>::NewVal(RegNewVals<>::OldValProducer(
                            [](const std::string& old, const ModType*) { return removeRef(old); }))}});
                    auto addRef = std::make_unique<RegNewVals<>>(std::vector<std::pair<std::string, RegNewVals<>::NewValSpec>>{
                        {zzmiLightMap, RegNewVals<>::NewVal(RegNewVals<>::OldValProducer(
                            [](const std::string& old, const ModType*) {
                                const std::string bare = removeRef(old);
                                return StringTools::equalsIgnoreCase(bare, "null") ? old : "ref " + bare;
                            }))}});
                    auto stripAdapter = std::make_unique<RegPartEdit<>>(stripRef.get());
                    auto addAdapter = std::make_unique<RegPartEdit<>>(addRef.get());

                    ObjGroupEdit::IniEdits strips, adds;
                    for (const ZZMIFixerConfig::Slot& slot : comp_.slots) {
                        strips.edits[objOf(comp_.name, slot.name)] = {stripAdapter.get()};
                        adds.edits[objOf(comp_.name, slot.name)] = {addAdapter.get()};
                    }
                    groupEdits_.push_back(std::make_unique<ObjGroupEdit>(std::vector<ObjGroupEdit::IniEdits>{strips}));

                    // a `ps-t5 = ref <resource VARIABLE>` (one command list every component shares) names no
                    // file: left unedited
                    const Collector::ResPredicate direct = [](const std::string&, const std::string& val, const Collector::IterData&) {
                        const std::string s = lowered(std::string(StringTools::strip(val)));
                        return s.compare(0, 4, "ref ") != 0;
                    };

                    for (const Ref& ref : refs) {
                        const GraphId srcGraph(0, "", ref.graph);
                        auto replace = std::make_unique<TexEditorReplace<>>(
                            GraphId(0, "", comp_.name + "_" + ref.res),
                            TexEditor({bandEdit(config_, comp_.bandMoves)}, false, true), resEditConfig(),
                            "resourceRemapTexEdit", comp_.target + "LightMap");
                        auto collect = std::make_unique<Collector>();
                        collect->srcRegs = {{srcGraph, ref.reg}};
                        collect->resEdits = {{"lightMap", replace.get()}};
                        collect->resPredicates = {{srcGraph, direct}};
                        resEdits_.push_back(std::move(replace));
                        groupCollectsAfterStrip_.push_back(std::move(collect));
                    }
                    for (auto& collect : groupCollectsAfterStrip_) {
                        collectsBox_.push_back(std::move(collect));
                    }
                    groupCollectsAfterStrip_.clear();

                    bandEdits_ = std::make_unique<ObjGroupEdit>(std::vector<ObjGroupEdit::IniEdits>{adds});
                    regEdits_.push_back(std::move(stripRef));
                    regEdits_.push_back(std::move(addRef));
                    partEdits_.push_back(std::move(stripAdapter));
                    partEdits_.push_back(std::move(addAdapter));
                }

                // ---- 4. names, hashes, slot indices, strides, and a doubled mesh ----
                void buildObjEdits() {
                    const std::string toModName = toModName_;
                    renames_.push_back(std::make_unique<GraphRename<>>([toModName](const std::string& n) {
                        return orderedName(n, IniNamingTools::getRemapFixName(n, toModName)); }));
                    renames_.push_back(std::make_unique<GraphRename<>>([toModName](const std::string& n) {
                        return orderedName(n, IniNamingTools::getRemapIbName(n, toModName)); }));
                    renames_.push_back(std::make_unique<GraphRename<>>([toModName](const std::string& n) {
                        return orderedName(n, IniNamingTools::getRemapBlendName(n, toModName)); }));
                    for (auto& rename : renames_) {
                        partEdits_.push_back(std::make_unique<GraphPartEdit<>>(rename.get()));
                    }
                    GroupPartEdit* rename = partEdits_[partEdits_.size() - 3].get();
                    GroupPartEdit* renameIb = partEdits_[partEdits_.size() - 2].get();
                    GroupPartEdit* renameBlend = partEdits_[partEdits_.size() - 1].get();

                    auto hashRemap = std::make_unique<RegAssetRemap<>>(
                        std::vector<std::pair<std::string, RegAssetRemap<>::AssetSpec>>{
                            {IniKeywords::Hash, RegAssetRemap<>::AssetSpec(ctx_.modTypeHashes(), IniKeywords::HashNotFound)}},
                        toModName_, srcName(), ctx_.version(), toVersion());
                    partEdits_.push_back(std::make_unique<RegPartEdit<>>(hashRemap.get()));
                    GroupPartEdit* hash = partEdits_.back().get();
                    regEdits_.push_back(std::move(hashRemap));

                    // a VertexLimitRaise's override_byte_stride is the SOURCE component's draw stride
                    // (Position 40 + Texcoord + Blend 32), and the target's is its own
                    auto drawStride = std::make_unique<RegNewVals<>>(std::vector<std::pair<std::string, RegNewVals<>::NewValSpec>>{
                        {"override_byte_stride", RegNewVals<>::NewVal(std::to_string(40 + comp_.targetTexcoordStride + 32))}});
                    partEdits_.push_back(std::make_unique<RegPartEdit<>>(drawStride.get()));
                    GroupPartEdit* stride = partEdits_.back().get();
                    regEdits_.push_back(std::move(drawStride));

                    ObjGroupEdit::IniEdits edits;
                    edits.edits[objOf(comp_.name, "ib")] = {renameIb, hash};
                    edits.edits[objOf(comp_.name, "blend")] = {renameBlend, hash};
                    edits.edits[objOf(comp_.name, "texcoord")] = {rename, hash};
                    edits.edits[objOf(comp_.name, "vertexlimit")] = {rename, hash, stride};
                    edits.edits[objOf(comp_.name, "position")] = {rename, hash};

                    // A mesh two components' sections BOTH draw shows whichever slot the game draws LATER,
                    // at equal depth -- and the two characters order their slots differently. So a draw
                    // whose index RANGE another component's sections also draw from the same ib is dropped
                    std::vector<DrawRange> shared;
                    for (const auto& pair : config_.crossSlotDraws) {
                        if (pair.first != comp_.name) {
                            continue;
                        }
                        const std::optional<std::string> mineHash = hashOf(srcName(), "ib");
                        const std::optional<std::string> laterHash = hashOf(config_.source + pair.second, "ib");
                        if (!mineHash.has_value() || !laterHash.has_value()) {
                            continue;
                        }
                        const auto mine = rangesDrawnBy(own_, *mineHash);
                        const auto later = rangesDrawnBy(own_, *laterHash);
                        for (const auto& entry : mine) {
                            const auto found = later.find(entry.first);
                            if (found != later.end()) {
                                shared.insert(shared.end(), found->second.begin(), found->second.end());
                            }
                        }
                    }
                    GroupPartEdit* noDouble = nullptr;
                    if (!shared.empty()) {
                        auto drop = std::make_unique<RegNewVals<>>(std::vector<std::pair<std::string, RegNewVals<>::NewValSpec>>{
                            {IniKeywords::DrawIndexed, RegNewVals<>::NewVal(RegNewVals<>::OldValProducer(
                                [shared](const std::string& old, const ModType*) {
                                    return overlaps(drawRange(old), shared) ? std::string("0, 0, 0") : old;
                                }))}});
                        partEdits_.push_back(std::make_unique<RegPartEdit<>>(drop.get()));
                        noDouble = partEdits_.back().get();
                        regEdits_.push_back(std::move(drop));
                        edits.edits[objOf(comp_.name, "ib")].push_back(noDouble);
                    }

                    for (const ZZMIFixerConfig::Slot& slot : comp_.slots) {
                        auto vals = std::make_unique<RegNewVals<>>(std::vector<std::pair<std::string, RegNewVals<>::NewValSpec>>{
                            {IniKeywords::MatchFirstIndex, RegNewVals<>::NewVal(slot.targetIndex)},
                            {"match_index_count", RegNewVals<>::NewVal(std::to_string(slot.targetIndexCount))}});
                        partEdits_.push_back(std::make_unique<RegPartEdit<>>(vals.get()));
                        std::vector<GroupPartEdit*> list = {rename, hash, partEdits_.back().get()};
                        if (noDouble != nullptr) {
                            list.push_back(noDouble);
                        }
                        edits.edits[objOf(comp_.name, slot.name)] = list;
                        regEdits_.push_back(std::move(vals));
                    }
                    for (const ZZMIFixerConfig::Texture& tex : comp_.textures) {
                        edits.edits[objOf(comp_.name, "tex_" + tex.role)] = {rename, hash};
                    }

                    // the band edit's collects, then the `ref` going back on, then the names
                    for (auto& collect : collectsBox_) {
                        groupEdits_.push_back(std::move(collect));
                    }
                    collectsBox_.clear();
                    if (bandEdits_ != nullptr) {
                        groupEdits_.push_back(std::move(bandEdits_));
                    }
                    groupEdits_.push_back(std::make_unique<ObjGroupEdit>(std::vector<ObjGroupEdit::IniEdits>{edits}));
                }

                // ---- the fallbacks: the source's game texture of a role the mod recolours by no hash ----
                std::string fallbackName(const std::string& role) const {
                    return dlName(capitalised(role));
                }

                // `<target component>RemapDL<thing><.ini stem>`: one name per .ini file, as two .ini files in one
                // folder writing the same file would have an undo of one delete the other's
                std::string dlName(const std::string& thing) const {
                    std::string stem;
                    const std::optional<std::string>& file = ctx_.getIniFile()->getFile();
                    if (file.has_value()) {
                        const std::filesystem::path path = FileService::strToPath(*file);
                        for (char ch : FileService::pathToStr(path.stem())) {
                            // a non-ASCII byte is kept: a stem in another script must not collapse to ""
                            const unsigned char byte = static_cast<unsigned char>(ch);
                            if (byte >= 0x80 || std::isalnum(byte) || ch == '_') {
                                stem += ch;
                            }
                        }
                    }
                    return toModName_ + IniKeywords::RemapDL + thing + stem;
                }

                bool drawsComponent() const {
                    return draws(comp_.name);
                }

                bool draws(const std::string& compName) const {
                    const std::optional<std::string> ib = hashOf(config_.source + compName, "ib");
                    if (!ib.has_value()) {
                        return false;
                    }
                    for (const Template* tpl : own_) {
                        const std::optional<std::string> hash = valOf(*tpl, IniKeywords::Hash);
                        if (hash.has_value() && StringTools::equalsIgnoreCase(*hash, *ib)) {
                            return true;
                        }
                    }
                    return false;
                }

                std::set<std::string> recolouredHashes() const {
                    std::set<std::string> found;
                    IniFile* ini = ctx_.getIniFile();
                    if (ini == nullptr) {
                        return found;
                    }
                    // the folder of the .ini and below, as readAllInis reads them
                    std::error_code ec;
                    for (const auto& entry : std::filesystem::recursive_directory_iterator(FileService::strToPath(ini->getFolder()), ec)) {
                        const std::string name = FileService::pathToStr(entry.path().filename());
                        if (!entry.is_regular_file() || !StringTools::endsWithIgnoreCase(name, FileExt::Ini)
                                || IniNamingTools::isDisabled(name)) {
                            continue;
                        }
                        for (const IniScanSection& s : IniScan::scan(FileService::pathToStr(entry.path()))) {
                            const std::optional<std::string> hash = IniScan::firstVal(s, IniKeywords::Hash);
                            if (hash.has_value() && s.name.find(IniKeywords::Remap) == std::string::npos
                                    && IniScan::firstVal(s, IniKeywords::This).has_value()) {
                                found.insert(lowered(*hash));
                            }
                        }
                    }
                    return found;
                }

                using FixSection = GraphCreate<>::Graph::Section;

                // ---- 6. THE EXTRAS: the source's extra objects, the frozen buffers and their pass filters ----

                std::unique_ptr<IfTemplatePart> pred(const std::string& src, IfPredPartType type) {
                    return std::make_unique<IfPredPart>(src, type, templateZ3_);
                }

                static std::unique_ptr<IfTemplatePart> kvps(std::vector<std::pair<std::string, std::string>> entries, int depth) {
                    return std::make_unique<IfContentPart<std::string, std::string>>(entries, depth);
                }

                FixSection* keepTemplate(std::vector<std::unique_ptr<IfTemplatePart>> parts) {
                    fallbackSections_.push_back(std::make_unique<FixSection>(std::move(parts), runConfig(), ""));
                    return fallbackSections_.back().get();
                }

                std::string downloadUrl(const std::string& kind, const std::string& ext) const {
                    return DownloadTools::downloadFolder() + "/" + DownloadTools::urlPath(
                        config_.downloadGameFolder, config_.downloadCharFolder, config_.downloadVersionFolder, config_.source, kind, ext);
                }

                // ResRegCollect + a resource edit over one register of some graphs, for the names 'names' only
                void collectNames(const std::vector<GraphId>& graphs, const std::string& reg, std::set<std::string> names, BaseResEdit<>* edit) {
                    auto collect = std::make_unique<Collector>();
                    const Collector::ResPredicate named = [names](const std::string&, const std::string& val, const Collector::IterData&) {
                        return names.count(removeRef(val)) > 0;
                    };
                    for (const GraphId& graph : graphs) {
                        collect->srcRegs[graph] = reg;
                        collect->resPredicates[graph] = named;
                    }
                    collect->resEdits = {{"extra", edit}};
                    extrasEdits_.push_back(std::move(collect));
                }

                void buildExtras() {
                    if (ctx_.getIniFile() == nullptr || !drawsComponent()) {
                        return;
                    }
                    const bool frozen = buildFrozenBuffers();
                    const bool extra = buildExtraObjects();

                    // the pass filters, once per .ini file: by the first component that needs them and that the mod draws
                    const ZZMIFixerConfig::Component* owner = nullptr;
                    for (const ZZMIFixerConfig::Component& c : config_.components) {
                        if ((!c.extraObjects.empty() || !c.frozenBuffers.empty()) && draws(c.name)) {
                            owner = &c;
                            break;
                        }
                    }
                    if (owner == nullptr || owner->name != comp_.name || config_.passFilters.empty() || !(frozen || extra)) {
                        return;
                    }
                    std::unordered_map<std::string, FixSection*> sections;
                    std::vector<std::string> names;
                    for (const ZZMIFixerConfig::PassFilter& filter : config_.passFilters) {
                        const std::string name = "ShaderOverride" + config_.target + IniKeywords::Remap + filter.label;
                        fallbackSections_.push_back(makeSection(name, {{IniKeywords::Hash, filter.shaderHash}, {"filter_index", filter.filterIndex}}));
                        sections[name] = fallbackSections_.back().get();
                        names.push_back(name);
                    }
                    extraGraphs_.push_back(std::make_unique<GraphCreate<>::Graph>(sections, names, runConfig()));
                    extrasEdits_.push_back(std::make_unique<GraphCreate<>>(GraphId(0, "", comp_.name + "_passFilters"), extraGraphs_.back().get()));
                }

                // Each frozen buffer bound per pass at the top of every drawn copy:
                //   if ps == <pass 1>  / <reg> = <its copy>  / else if ps == <pass 2> / ... / endif
                bool buildFrozenBuffers() {
                    if (comp_.frozenBuffers.empty()) {
                        return false;
                    }
                    std::vector<GraphId> slotGraphs;
                    for (const ZZMIFixerConfig::Slot& slot : comp_.slots) {
                        slotGraphs.emplace_back(0, "", comp_.name + "_" + slot.name);
                    }
                    ObjGroupEdit::IniEdits adds;
                    std::unordered_map<std::string, FileDownloadCreate<>::File> files;
                    for (const ZZMIFixerConfig::FrozenBuffer& buffer : comp_.frozenBuffers) {
                        std::vector<std::unique_ptr<IfTemplatePart>> parts;
                        std::set<std::string> names;
                        for (std::size_t i = 0; i < buffer.perPass.size(); ++i) {
                            const std::string name = dlName(buffer.perPass[i].second);
                            const std::string resource = IniKeywords::Resource + name;
                            parts.push_back(pred((i == 0 ? "if ps == " : "else if ps == ") + buffer.perPass[i].first,
                                                 i == 0 ? IfPredPartType::If : IfPredPartType::Elif));
                            parts.push_back(kvps({{buffer.reg, resource}}, 1));
                            FileDownloadCreate<>::File file;
                            file.keys = {{IniKeywords::Type, "Buffer"}};
                            file.file = name + ".buf";
                            file.url = downloadUrl(buffer.perPass[i].second, ".buf");
                            files[resource] = std::move(file);
                            names.insert(resource);
                        }
                        parts.push_back(pred("endif", IfPredPartType::EndIf));
                        templateAdds_.push_back(std::make_unique<RegIfTemplateAdd<>>(keepTemplate(std::move(parts)),
                            RegIfTemplateAdd<>::Position::AfterKey, IniKeywords::MatchFirstIndex));
                        partEdits_.push_back(std::make_unique<GraphPartEdit<>>(templateAdds_.back().get()));
                        for (const GraphId& graph : slotGraphs) {
                            adds.edits[graph.modObj].push_back(partEdits_.back().get());
                        }
                        frozenNames_.push_back(names);
                        frozenRegs_.push_back(buffer.reg);
                    }
                    extrasEdits_.push_back(std::make_unique<ObjGroupEdit>(std::vector<ObjGroupEdit::IniEdits>{adds}));
                    for (std::size_t i = 0; i < frozenRegs_.size(); ++i) {
                        std::unordered_map<std::string, FileDownloadCreate<>::File> subset;
                        for (const std::string& name : frozenNames_[i]) {
                            subset[name] = files[name];
                        }
                        resEdits_.push_back(std::make_unique<FileDownloadCreate<>>(
                            GraphId(0, "", comp_.name + "_frozenDL" + std::to_string(i)), subset, resEditConfig()));
                        collectNames(slotGraphs, frozenRegs_[i], frozenNames_[i], resEdits_.back().get());
                    }
                    return true;
                }

                // The mod's own sections on an extra object's ib, as parts that clear 'var' where they skip it.
                // None when a section skips it with no condition, or draws geometry of its own there
                std::optional<std::vector<std::unique_ptr<IfTemplatePart>>> gatesOf(const std::string& ibHash, const std::string& var) {
                    std::vector<std::unique_ptr<IfTemplatePart>> gates;
                    for (const Template* tpl : own_) {
                        const std::optional<std::string> hash = valOf(*tpl, IniKeywords::Hash);
                        if (!hash.has_value() || !StringTools::equalsIgnoreCase(*hash, ibHash)) {
                            continue;
                        }
                        if (valOf(*tpl, IniKeywords::Ib).has_value() || valOf(*tpl, IniKeywords::DrawIndexed).has_value()) {
                            ctx_.log("the mod draws its own " + ibHash + ": the game's object is not added");
                            return std::nullopt;
                        }
                        for (const auto& part : tpl->parts()) {
                            if (const auto* p = dynamic_cast<const IfPredPart*>(part.get())) {
                                gates.push_back(pred(p->src, p->type));
                                continue;
                            }
                            const auto* content = dynamic_cast<const IfContentPart<std::string, std::string>*>(part.get());
                            if (content == nullptr) {
                                continue;
                            }
                            for (const auto& kv : content->entries()) {
                                if (!StringTools::equalsIgnoreCase(std::string(StringTools::strip(kv.first)), IniKeywords::Handling)
                                        || !StringTools::equalsIgnoreCase(std::string(StringTools::strip(kv.second)), "skip")) {
                                    continue;
                                }
                                if (content->depth() == 0) {
                                    return std::nullopt;
                                }
                                gates.push_back(kvps({{var, "0"}}, content->depth()));
                            }
                        }
                    }
                    return gates;
                }

                // The source's extra objects: pre-skinned after the mod's mesh (stream-out APPENDS, so they land at
                // vertex N, the mod's vertex count), the vertex limit raised by their count, and drawn first in the
                // component's first object copy -- before the mod's own bindings, so they read the game's textures
                // (and any recolour of them) -- with base vertex N and a texcoord padded with N empty entries
                bool buildExtraObjects() {
                    if (comp_.extraObjects.empty() || comp_.slots.empty() || config_.passFilters.empty()) {
                        return false;
                    }
                    const std::optional<std::string> drawVb = hashOf(srcName(), "draw_vb");
                    const std::optional<std::string> ib = hashOf(srcName(), "ib");
                    std::optional<std::size_t> n;
                    std::vector<std::string> drawn;
                    for (const Template* tpl : own_) {
                        const std::optional<std::string> hash = valOf(*tpl, IniKeywords::Hash);
                        if (!hash.has_value()) {
                            continue;
                        }
                        if (drawVb.has_value() && StringTools::equalsIgnoreCase(*hash, *drawVb) && !n.has_value()) {
                            const std::optional<std::string> count = valOf(*tpl, "override_vertex_count");
                            if (count.has_value()) {
                                n = static_cast<std::size_t>(std::atoll(count->c_str()));
                            }
                        }
                        const std::optional<std::string> first = valOf(*tpl, IniKeywords::MatchFirstIndex);
                        if (ib.has_value() && StringTools::equalsIgnoreCase(*hash, *ib) && first.has_value()
                                && std::string(StringTools::strip(*first)) == comp_.slots.front().srcIndex) {
                            drawn.push_back(tpl->name);
                        }
                    }
                    if (!n.has_value() || drawn.empty()) {
                        ctx_.log("no " + comp_.name + " object / vertex limit section to add the extra objects to");
                        return false;
                    }
                    std::sort(drawn.begin(), drawn.end(), [](const std::string& a, const std::string& b) { return lowered(a) < lowered(b); });
                    const std::string into = orderedName(drawn.front(), IniNamingTools::getRemapFixName(drawn.front(), toModName_));

                    std::string passes;
                    for (const ZZMIFixerConfig::PassFilter& filter : config_.passFilters) {
                        passes += (passes.empty() ? "" : " || ") + std::string("ps == ") + filter.filterIndex;
                    }
                    const std::string bak = IniKeywords::Resource + toModName_ + IniKeywords::Remap + "Vb1Bak";
                    const std::string pad = IniKeywords::Resource + dlName("AccessoryTexcoord");

                    std::vector<std::pair<std::string, std::string>> preSkin, locals;
                    std::vector<std::unique_ptr<IfTemplatePart>> draws;
                    std::unordered_map<std::string, FileDownloadCreate<>::File> files;
                    std::set<std::string> positions, texcoords, blends, ibs;
                    std::vector<std::string> texcoordFiles;
                    std::size_t base = *n;
                    for (const ZZMIFixerConfig::ExtraObject& object : comp_.extraObjects) {
                        const std::string var = "$remap" + lowered(object.name);
                        std::optional<std::vector<std::unique_ptr<IfTemplatePart>>> gates = gatesOf(object.ibHash, var);
                        if (!gates.has_value()) {
                            continue;
                        }
                        const auto file = [this, &files, &object](const std::string& kind, const std::string& ext,
                                                                   std::vector<std::pair<std::string, std::string>> keys) {
                            const std::string name = dlName(object.name + kind);
                            FileDownloadCreate<>::File f;
                            f.keys = std::move(keys);
                            f.file = name + ext;
                            f.url = downloadUrl(object.name + (kind == "IB" ? "A" : kind), ext);
                            files[IniKeywords::Resource + name] = std::move(f);
                            return IniKeywords::Resource + name;
                        };
                        const std::string position = file("Position", ".buf", {{IniKeywords::Type, "Buffer"}, {IniKeywords::Stride, "40"}});
                        const std::string blend = file("Blend", ".buf", {{IniKeywords::Type, "Buffer"}, {IniKeywords::Stride, "32"}});
                        const std::string texcoord = file("Texcoord", ".buf", {{IniKeywords::Type, "Buffer"}, {IniKeywords::Stride, "20"}});
                        const std::string ibRes = file("IB", ".ib", {{IniKeywords::Type, "Buffer"}, {"format", "DXGI_FORMAT_R32_UINT"}});
                        positions.insert(position);
                        blends.insert(blend);
                        texcoords.insert(texcoord);
                        ibs.insert(ibRes);
                        texcoordFiles.push_back(files[texcoord].file);

                        preSkin.insert(preSkin.end(), {{IniKeywords::Vb0, position}, {IniKeywords::Vb1, texcoord}, {"vb2", blend},
                                                       {IniKeywords::Draw, std::to_string(object.vertexCount) + ", 0"}});
                        locals.emplace_back("local " + var, "");
                        draws.push_back(kvps({{var, "1"}}, 1));
                        for (auto& gate : *gates) {
                            if (auto* content = dynamic_cast<IfContentPart<std::string, std::string>*>(gate.get())) {
                                content->setDepth(content->depth() + 1);
                            }
                            draws.push_back(std::move(gate));
                        }
                        draws.push_back(pred("if " + var + " == 1", IfPredPartType::If));
                        draws.push_back(kvps({{IniKeywords::Ib, ibRes}, {IniKeywords::DrawIndexed, std::to_string(object.indexCount) + ", 0, " + std::to_string(base)}}, 2));
                        draws.push_back(pred("endif", IfPredPartType::EndIf));
                        base += object.vertexCount;
                    }
                    if (preSkin.empty()) {
                        return false;
                    }
                    // a mod runs its pre-skin pass from its Blend section or from its Position one (Miyabi11): the
                    // template goes after the `draw` of whichever has it
                    const GraphId blendGraph(0, "", comp_.name + "_blend");
                    const GraphId positionGraph(0, "", comp_.name + "_position");
                    const GraphId objGraph(0, "", comp_.name + "_" + comp_.slots.front().name);
                    const GraphId limitGraph(0, "", comp_.name + "_vertexlimit");

                    // the pre-skin: after the mod's mesh, inside its DRAW_TYPE test
                    std::vector<std::unique_ptr<IfTemplatePart>> preParts;
                    preParts.push_back(kvps(preSkin, 0));
                    templateAdds_.push_back(std::make_unique<RegIfTemplateAdd<>>(keepTemplate(std::move(preParts)),
                        RegIfTemplateAdd<>::Position::AfterKey, IniKeywords::Draw));
                    partEdits_.push_back(std::make_unique<GraphPartEdit<>>(templateAdds_.back().get()));
                    GroupPartEdit* addPreSkin = partEdits_.back().get();

                    // the vertex limit, raised by their count
                    const std::size_t was = *n;
                    const std::size_t raised = base;
                    regEdits_.push_back(std::make_unique<RegNewVals<>>(std::vector<std::pair<std::string, RegNewVals<>::NewValSpec>>{
                        {"override_vertex_count", RegNewVals<>::NewVal(RegNewVals<>::OldValProducer(
                            [was, raised](const std::string& old, const ModType*) {
                                return std::string(StringTools::strip(old)) == std::to_string(was) ? std::to_string(raised) : old;
                            }))}}));
                    partEdits_.push_back(std::make_unique<RegPartEdit<>>(regEdits_.back().get()));
                    GroupPartEdit* raise = partEdits_.back().get();

                    // the draws, first in the object copy, in the passes the game draws them in
                    std::vector<std::unique_ptr<IfTemplatePart>> drawParts;
                    drawParts.push_back(kvps(locals, 0));
                    drawParts.push_back(pred("if " + passes, IfPredPartType::If));
                    drawParts.push_back(kvps({{bak, "ref vb1"}, {IniKeywords::Vb1, pad}}, 1));
                    for (auto& part : draws) {
                        drawParts.push_back(std::move(part));
                    }
                    drawParts.push_back(kvps({{IniKeywords::Vb1, bak}}, 1));
                    drawParts.push_back(pred("endif", IfPredPartType::EndIf));
                    templateAdds_.push_back(std::make_unique<RegIfTemplateAdd<>>(keepTemplate(std::move(drawParts)),
                        RegIfTemplateAdd<>::Position::AfterKey, IniKeywords::MatchFirstIndex));
                    partEdits_.push_back(std::make_unique<GraphPartEdit<>>(templateAdds_.back().get()));

                    ObjGroupEdit::IniEdits edits;
                    edits.edits[blendGraph.modObj] = {addPreSkin};
                    edits.edits[positionGraph.modObj] = {addPreSkin};
                    edits.edits[limitGraph.modObj] = {raise};
                    edits.edits[objGraph.modObj] = {partEdits_.back().get()};
                    edits.keyFilters[objGraph.modObj] = {[into](const ObjGroupEdit::IterData& data, const ModType*, IniFile*) {
                        return data.sectionName == into ? ObjGroupEdit::OrderRanges::createFull() : ObjGroupEdit::OrderRanges::createEmpty();
                    }};
                    extrasEdits_.push_back(std::make_unique<ObjGroupEdit>(std::vector<ObjGroupEdit::IniEdits>{edits}));

                    // the files: the downloads, the texcoord padded with N empty entries (made from the downloaded
                    // texcoords when it is written), and the empty section the vb1 backup goes in
                    // one resource edit per collect: an edit a collect has built through does not serve another
                    const auto downloadsOf = [this, &files](const std::string& kind, const std::set<std::string>& names) {
                        std::unordered_map<std::string, FileDownloadCreate<>::File> subset;
                        for (const std::string& name : names) {
                            subset[name] = files[name];
                        }
                        resEdits_.push_back(std::make_unique<FileDownloadCreate<>>(GraphId(0, "", comp_.name + "_extraDL" + kind), subset, resEditConfig()));
                        return resEdits_.back().get();
                    };
                    collectNames({blendGraph, positionGraph}, IniKeywords::Vb0, positions, downloadsOf("Position", positions));
                    collectNames({blendGraph, positionGraph}, IniKeywords::Vb1, texcoords, downloadsOf("Texcoord", texcoords));
                    collectNames({blendGraph, positionGraph}, "vb2", blends, downloadsOf("Blend", blends));
                    collectNames({objGraph}, IniKeywords::Ib, ibs, downloadsOf("IB", ibs));

                    const std::string folder = ctx_.getIniFile()->getFolder();
                    FileAddCreate<>::File padFile;
                    padFile.keys = {{IniKeywords::Type, "Buffer"}, {IniKeywords::Stride, "20"}};
                    padFile.file = dlName("AccessoryTexcoord") + ".buf";
                    padFile.makeBytes = [folder, was, texcoordFiles]() {
                        std::vector<std::uint8_t> bytes(was * 20, 0);
                        for (const std::string& f : texcoordFiles) {
                            std::ifstream in(FileService::strToPath(FileService::absPathOfRelPath(f, folder)), std::ios::binary);
                            bytes.insert(bytes.end(), std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
                        }
                        return bytes;
                    };
                    resEdits_.push_back(std::make_unique<FileAddCreate<>>(GraphId(0, "", comp_.name + "_extraPad"),
                        std::unordered_map<std::string, FileAddCreate<>::File>{{pad, std::move(padFile)}}, resEditConfig()));
                    collectNames({objGraph}, IniKeywords::Vb1, {pad}, resEdits_.back().get());

                    std::unordered_map<std::string, FixSection*> bakSection;
                    fallbackSections_.push_back(makeSection(bak, {}));
                    bakSection[bak] = fallbackSections_.back().get();
                    extraGraphs_.push_back(std::make_unique<GraphCreate<>::Graph>(bakSection, std::vector<std::string>{bak}, runConfig()));
                    extrasEdits_.push_back(std::make_unique<GraphCreate<>>(GraphId(0, "", comp_.name + "_extraBak"), extraGraphs_.back().get()));
                    return true;
                }


                // One section the fix writes itself
                std::unique_ptr<FixSection> makeSection(const std::string& name, const std::vector<std::pair<std::string, std::string>>& kvps) {
                    std::vector<std::unique_ptr<IfTemplatePart>> parts;
                    parts.push_back(std::make_unique<IfContentPart<std::string, std::string>>(kvps, 0));
                    return std::make_unique<FixSection>(std::move(parts), runConfig(), name);
                }

                static IfTemplateRunConfig<std::string, std::string> runConfig() {
                    return IfTemplateRunConfig<std::string, std::string>{
                        IniKeywords::Run,
                        [](const std::string& val) { return val; },
                        [](const std::string& section) { return section; }
                    };
                }

                // A role the mod recolours by no hash gets the source's game texture on the target's hash (an
                // override section of the fix's own, added by GraphCreate). One the mod DOES recolour, maybe only
                // under a toggle, has each carried copy that recolours bind it FIRST (RegIfTemplateAdd), so a
                // toggle that is off does not leave the target's texture at the source's UVs. Every one of them
                // is a download (FileDownloadCreate, behind the `this` naming it); a light map with band moves is
                // downloaded raw and edited
                void buildFallbacks() {
                    IniFile* ini = ctx_.getIniFile();
                    if (ini == nullptr || comp_.textures.empty() || !drawsComponent()) {
                        return;
                    }

                    const std::set<std::string> recoloured = recolouredHashes();
                    std::unordered_map<std::string, FileDownloadCreate<>::File> downloads;
                    std::unordered_map<std::string, FixSection*> overrides;
                    std::vector<std::string> overrideNames;
                    std::unordered_map<std::string, FixSection*> raws;
                    std::vector<std::string> rawNames;
                    std::vector<GraphId> recolourCopies;
                    ObjGroupEdit::IniEdits defaults;

                    // only a copy that recolours: another section on the hash (a watcher, a `run =`) is left alone
                    const ObjGroupEdit::PartFilter recolours = [](const ObjGroupEdit::IterData& data, const ModType*, IniFile*) {
                        const bool hasThis = data.section != nullptr && ModBranches::firstVal(*data.section, IniKeywords::This).has_value();
                        return hasThis ? ObjGroupEdit::OrderRanges::createFull() : ObjGroupEdit::OrderRanges::createEmpty();
                    };

                    for (const ZZMIFixerConfig::Texture& tex : comp_.textures) {
                        if (!tex.fallback) {
                            continue;
                        }
                        const std::optional<std::string> srcHash = hashOf(srcName(), tex.hashType);
                        const std::optional<std::string> dstHash = hashOf(targetName(), tex.hashType);
                        if (!srcHash.has_value() || !dstHash.has_value()) {
                            continue;
                        }

                        const std::string name = fallbackName(tex.role);
                        const std::string resource = IniKeywords::Resource + name;
                        FileDownloadCreate<>::File file;
                        file.file = name + FileExt::DDS;
                        file.url = DownloadTools::downloadFolder() + "/"
                            + DownloadTools::urlPath(config_.downloadGameFolder, config_.downloadCharFolder,
                                                     config_.downloadVersionFolder, srcName(), "A" + capitalised(tex.role), FileExt::DDS);
                        if (tex.role == LightMapRole && !comp_.bandMoves.empty()) {
                            file.texEdit = TexEditor({bandEdit(config_, comp_.bandMoves)}, false, true);
                            file.rawFile = name + "Raw" + FileExt::DDS;
                            // the raw download is named by a section too: an undo deletes the files the fix's own
                            // sections name
                            const std::string rawName = resource + "Raw";
                            fallbackSections_.push_back(makeSection(rawName, {{IniKeywords::Filename, file.rawFile}}));
                            raws[rawName] = fallbackSections_.back().get();
                            rawNames.push_back(rawName);
                        }
                        downloads[resource] = std::move(file);

                        if (recoloured.count(lowered(*srcHash)) > 0) {
                            fallbackSections_.push_back(makeSection("", {{IniKeywords::This, resource}}));
                            defaultAdds_.push_back(std::make_unique<RegIfTemplateAdd<>>(
                                fallbackSections_.back().get(), RegIfTemplateAdd<>::Position::AfterKey, IniKeywords::Hash));
                            partEdits_.push_back(std::make_unique<GraphPartEdit<>>(defaultAdds_.back().get()));
                            const ModObj obj = objOf(comp_.name, "tex_" + tex.role);
                            defaults.edits[obj] = {partEdits_.back().get()};
                            defaults.keyFilters[obj] = {recolours};
                            recolourCopies.emplace_back(0, obj.first, obj.second);
                            continue;
                        }

                        const std::string sectionName = IniKeywords::TextureOverride + name;
                        fallbackSections_.push_back(makeSection(sectionName, {{IniKeywords::Hash, *dstHash}, {IniKeywords::This, resource}}));
                        overrides[sectionName] = fallbackSections_.back().get();
                        overrideNames.push_back(sectionName);
                    }
                    if (downloads.empty()) {
                        return;
                    }

                    const GraphId fallbackGraph(0, "", comp_.name + "_fallback");
                    if (!overrides.empty()) {
                        fallbackGraph_ = std::make_unique<GraphCreate<>::Graph>(overrides, overrideNames, runConfig());
                        groupEdits_.push_back(std::make_unique<GraphCreate<>>(fallbackGraph, fallbackGraph_.get()));
                    }
                    if (!raws.empty()) {
                        rawGraph_ = std::make_unique<GraphCreate<>::Graph>(raws, rawNames, runConfig());
                        groupEdits_.push_back(std::make_unique<GraphCreate<>>(GraphId(0, "", comp_.name + "_fallbackRaw"), rawGraph_.get()));
                    }
                    if (!defaults.edits.empty()) {
                        groupEdits_.push_back(std::make_unique<ObjGroupEdit>(std::vector<ObjGroupEdit::IniEdits>{defaults}));
                    }

                    auto download = std::make_unique<FileDownloadCreate<>>(GraphId(0, "", comp_.name + "_fallbackDL"), downloads, resEditConfig());
                    auto collect = std::make_unique<Collector>();
                    const Collector::ResPredicate isFallback = [downloads](const std::string&, const std::string& val, const Collector::IterData&) {
                        return downloads.count(removeRef(val)) > 0;
                    };
                    std::vector<GraphId> sources = recolourCopies;
                    if (!overrides.empty()) {
                        sources.push_back(fallbackGraph);
                    }
                    for (const GraphId& source : sources) {
                        collect->srcRegs[source] = IniKeywords::This;
                        collect->resPredicates[source] = isFallback;
                    }
                    collect->resEdits = {{"download", download.get()}};
                    resEdits_.push_back(std::move(download));
                    groupEdits_.push_back(std::move(collect));
                }

                // Whether the mod draws 'comp' from a section of its own (one with `match_first_index` on its ib):
                // the copy of that section is where a pair's cross draws move to, so without it they stay put
                bool drawsFromOwnSection(const std::string& comp) const {
                    const std::optional<std::string> ib = hashOf(config_.source + comp, "ib");
                    if (!ib.has_value()) {
                        return false;
                    }
                    for (const Template* tpl : own_) {
                        const std::optional<std::string> hash = valOf(*tpl, IniKeywords::Hash);
                        if (hash.has_value() && StringTools::equalsIgnoreCase(*hash, *ib)
                                && valOf(*tpl, IniKeywords::MatchFirstIndex).has_value()) {
                            return true;
                        }
                    }
                    return false;
                }

                // The buffers a section of 'comp' pre-skins into (`ResourceX = copy so0` / `= so0`), on whichever
                // of the component's hashes the mod captures it
                std::set<std::string> preSkinBuffers(const std::string& comp) const {
                    std::set<std::string> written;
                    std::set<std::string> hashes;
                    for (const char* type : {"blend_vb", "position_vb", "ib", "draw_vb", "texcoord_vb"}) {
                        const std::optional<std::string> h = hashOf(config_.source + comp, type);
                        if (h.has_value()) {
                            hashes.insert(lowered(*h));
                        }
                    }
                    for (const Template* tpl : own_) {
                        const std::optional<std::string> hash = valOf(*tpl, IniKeywords::Hash);
                        if (!hash.has_value() || hashes.count(lowered(*hash)) == 0) {
                            continue;
                        }
                        // the stream-out of the pre-skin pass, or (at the component's own draw) the pre-skinned vb0
                        // it draws with
                        const bool ownDraw = lowered(*hash) == lowered(hashOf(config_.source + comp, "ib").value_or(""));
                        for (const auto& kv : kvpsOf(*tpl)) {
                            const std::string v = lowered(kv.second);
                            if (v == "so0" || v == "copy so0" || (ownDraw && v == "copy vb0")) {
                                written.insert(kv.first);
                            }
                        }
                    }
                    return written;
                }

                // ---- 5. A MESH DRAWN FROM ANOTHER COMPONENT'S SLOT GOES BACK TO ITS OWN (crossSlotDraws) ----

                // What one pair's move is made of, read off the mod's own sections: the buffers the first
                // component pre-skins into, the second component's drawn sections, and the ib each cross
                // draw binds (one made while vb0 is such a buffer)
                struct Cross {
                    std::set<std::string> written;
                    std::set<std::string> sectionNames;
                    std::set<std::string> crossIbs;
                };

                Cross crossOf(const std::pair<std::string, std::string>& pair) const {
                    Cross cross;
                    const std::optional<std::string> secondIb = hashOf(config_.source + pair.second, "ib");
                    cross.written = preSkinBuffers(pair.first);
                    if (cross.written.empty() || !secondIb.has_value() || !drawsFromOwnSection(pair.first)) {
                        return {};
                    }
                    for (const Template* tpl : own_) {
                        const std::optional<std::string> hash = valOf(*tpl, IniKeywords::Hash);
                        if (!hash.has_value() || !StringTools::equalsIgnoreCase(*hash, *secondIb)
                                || !valOf(*tpl, IniKeywords::MatchFirstIndex).has_value()) {
                            continue;
                        }
                        cross.sectionNames.insert(tpl->name);
                        std::optional<std::string> ib, vb0;
                        for (const auto& kv : kvpsOf(*tpl)) {
                            if (StringTools::equalsIgnoreCase(kv.first, IniKeywords::Ib)) {
                                ib = removeRef(kv.second);
                            } else if (StringTools::equalsIgnoreCase(kv.first, IniKeywords::Vb0)) {
                                vb0 = removeRef(kv.second);
                            } else if (StringTools::equalsIgnoreCase(kv.first, IniKeywords::DrawIndexed)
                                       && vb0.has_value() && cross.written.count(*vb0) > 0 && ib.has_value()) {
                                cross.crossIbs.insert(*ib);
                            }
                        }
                    }
                    return cross;
                }

                // Whether a line of a second-component section moves with the cross draws: an ib, a vb0 / vb3
                // naming a pre-skin buffer, a texture line, the SetTextures call, and a cross draw itself
                static bool movesWithCross(const FixSection& section, const IfContentPart<std::string, std::string>& at,
                                           std::size_t atInd, const Cross& cross) {
                    if (cross.sectionNames.count(section.name) == 0) {
                        return false;
                    }
                    std::optional<std::string> ib, vb0;
                    for (const auto& part : section.parts()) {
                        const auto* content = dynamic_cast<const IfContentPart<std::string, std::string>*>(part.get());
                        if (content == nullptr) {
                            continue;
                        }
                        const std::size_t len = content->size();
                        for (std::size_t i = 0; i < len; ++i) {
                            const auto kv = content->getByInd(static_cast<long long>(i));
                            const std::string key = std::string(StringTools::strip(kv.first));
                            const std::string value = std::string(StringTools::strip(kv.second));
                            if (StringTools::equalsIgnoreCase(key, IniKeywords::Ib)) {
                                ib = removeRef(value);
                            } else if (StringTools::equalsIgnoreCase(key, IniKeywords::Vb0)) {
                                vb0 = removeRef(value);
                            }
                            if (content != &at || i != atInd) {
                                continue;
                            }
                            const bool preSkin = (StringTools::equalsIgnoreCase(key, IniKeywords::Vb0) || StringTools::equalsIgnoreCase(key, "vb3"))
                                && cross.written.count(removeRef(value)) > 0;
                            const bool crossDraw = StringTools::equalsIgnoreCase(key, IniKeywords::DrawIndexed)
                                && vb0.has_value() && cross.written.count(*vb0) > 0 && ib.has_value();
                            return StringTools::equalsIgnoreCase(key, IniKeywords::Ib) || preSkin || crossDraw
                                || isTextureLine(key) || isSetTextures({key, value});
                        }
                    }
                    return false;
                }

                void buildCrossSlotEdits() {
                    for (const auto& pair : config_.crossSlotDraws) {
                        const Cross cross = crossOf(pair);
                        if (cross.crossIbs.empty()) {
                            continue;
                        }

                        const ZZMIFixerConfig::Component* second = nullptr;
                        for (const ZZMIFixerConfig::Component& c : config_.components) {
                            if (c.name == pair.second) {
                                second = &c;
                            }
                        }
                        if (second == nullptr) {
                            continue;
                        }

                        // the second component's copies: the ib a cross draw binds is nulled, so neither the draw
                        // nor a later `drawindexed = auto` of the same slot draws the first's mesh there
                        if (pair.second == comp_.name) {
                            const std::set<std::string> crossIbs = cross.crossIbs;
                            auto nullIb = std::make_unique<RegNewVals<>>(std::vector<std::pair<std::string, RegNewVals<>::NewValSpec>>{
                                {IniKeywords::Ib, RegNewVals<>::NewVal(RegNewVals<>::OldValProducer(
                                    [crossIbs](const std::string& old, const ModType*) {
                                        return crossIbs.count(removeRef(old)) > 0 ? IniKeywords::Null : old;
                                    }))}});
                            partEdits_.push_back(std::make_unique<RegPartEdit<>>(nullIb.get()));
                            ObjGroupEdit::IniEdits edits;
                            for (const ZZMIFixerConfig::Slot& slot : second->slots) {
                                edits.edits[objOf(second->name, slot.name)] = {partEdits_.back().get()};
                            }
                            regEdits_.push_back(std::move(nullIb));
                            crossEdits_.push_back(std::make_unique<ObjGroupEdit>(std::vector<ObjGroupEdit::IniEdits>{edits}));
                            continue;
                        }
                        if (pair.first != comp_.name) {
                            continue;
                        }

                        // the first component's copy: the section the moved lines go to is its first drawn one
                        const std::optional<std::string> firstIb = hashOf(srcName(), "ib");
                        std::vector<std::string> drawn;
                        for (const Template* tpl : own_) {
                            const std::optional<std::string> hash = valOf(*tpl, IniKeywords::Hash);
                            if (firstIb.has_value() && hash.has_value() && StringTools::equalsIgnoreCase(*hash, *firstIb)
                                    && valOf(*tpl, IniKeywords::MatchFirstIndex).has_value()) {
                                drawn.push_back(tpl->name);
                            }
                        }
                        if (drawn.empty()) {
                            ctx_.log("the " + pair.first + " draws this mod makes from its " + pair.second
                                     + " slot have no " + pair.first + " section to move into");
                            continue;
                        }
                        std::sort(drawn.begin(), drawn.end(), [](const std::string& a, const std::string& b) { return lowered(a) < lowered(b); });
                        const std::string into = orderedName(drawn.front(), IniNamingTools::getRemapFixName(drawn.front(), toModName_));
                        std::string intoSlot;
                        for (const ZZMIFixerConfig::Slot& slot : comp_.slots) {
                            intoSlot = intoSlot.empty() ? slot.name : intoSlot;
                        }
                        const GraphId intoGraph(0, "", comp_.name + "_" + intoSlot);

                        const std::string toModName = toModName_;
                        const std::string listPrefix = "CommandList" + toModName + IniKeywords::Remap + "Cross";
                        renames_.push_back(std::make_unique<GraphRename<>>([listPrefix](const std::string& n) {
                            const std::string prefix = IniKeywords::TextureOverride;
                            return listPrefix + (n.compare(0, prefix.size(), prefix) == 0 ? n.substr(prefix.size()) : n);
                        }));
                        partEdits_.push_back(std::make_unique<GraphPartEdit<>>(renames_.back().get()));
                        GroupPartEdit* toList = partEdits_.back().get();
                        ObjGroupEdit::IniEdits renameLists;

                        // the moved lines run at the END of that section, at its own depth, and only there
                        const ObjGroupEdit::PartFilter onlyInto = [into](const ObjGroupEdit::IterData& data, const ModType*, IniFile*) {
                            return data.sectionName == into ? ObjGroupEdit::OrderRanges::createFull() : ObjGroupEdit::OrderRanges::createEmpty();
                        };
                        const GraphInherit<>::Adder atBottom = [](GraphCopy<>::Graph&, const GraphInherit<>::KVPs& kvps, IniFile*,
                                                                  const ModType*, const std::string&) -> GraphInherit<>::AddEdit {
                            return std::shared_ptr<GraphInherit<>::GraphEdit>(std::make_shared<RegBottomAdd<>>(kvps));
                        };
                        std::vector<GraphId> listGraphs;

                        // 5a. the mod's own recolours of the second component's textures, bound by register: the
                        // moved draws read them on a slot that binds other texture hashes
                        for (const auto& roleReg : config_.roleRegs) {
                            const std::optional<std::string> h = hashOf(config_.source + pair.second, "tex_a_" + roleReg.first);
                            if (!h.has_value()) {
                                continue;
                            }
                            std::unordered_map<std::string, FixSection*> recolours;
                            std::vector<std::string> names;
                            for (const Template* tpl : own_) {
                                const std::optional<std::string> hash = valOf(*tpl, IniKeywords::Hash);
                                if (hash.has_value() && StringTools::equalsIgnoreCase(*hash, *h) && valOf(*tpl, IniKeywords::This).has_value()) {
                                    recolours[tpl->name] = const_cast<FixSection*>(tpl);
                                    names.push_back(tpl->name);
                                }
                            }
                            if (recolours.empty()) {
                                continue;
                            }
                            const GraphId texGraph(0, "", comp_.name + "_crossTex_" + roleReg.first);
                            crossTexGraphs_.push_back(std::make_unique<GraphCreate<>::Graph>(recolours, names, runConfig(), true, true));
                            crossEdits_.push_back(std::make_unique<GraphCreate<>>(texGraph, crossTexGraphs_.back().get()));
                            crossEdits_.push_back(std::make_unique<GraphFilter<>>(std::vector<GraphId>{texGraph},
                                [](const FixSection&, const IfContentPart<std::string, std::string>& part, std::size_t i) {
                                    return StringTools::equalsIgnoreCase(std::string(StringTools::strip(part.getByInd(static_cast<long long>(i)).first)), IniKeywords::This);
                                }));
                            regEdits_.push_back(std::make_unique<RegRemap<>>(std::vector<std::pair<std::string, RegRemap<>::KeyRemapValue>>{
                                {IniKeywords::This, RegRemap<>::KeyRemapValue(RemapList<std::string, std::string>{roleReg.second})}}));
                            partEdits_.push_back(std::make_unique<RegPartEdit<>>(regEdits_.back().get()));
                            ObjGroupEdit::IniEdits toReg;
                            toReg.edits[texGraph.modObj] = {partEdits_.back().get()};
                            crossEdits_.push_back(std::make_unique<ObjGroupEdit>(std::vector<ObjGroupEdit::IniEdits>{toReg}));
                            renameLists.edits[texGraph.modObj] = {toList};
                            listGraphs.push_back(texGraph);
                        }

                        // 5b. the moved lines themselves, a copy of each second-component graph -- taken before the
                        // remap drops those graphs from this fixer's groups
                        for (const ZZMIFixerConfig::Slot& slot : second->slots) {
                            const GraphId copy(0, "", comp_.name + "_cross_" + slot.name);
                            preRemapEdits_.push_back(std::make_unique<GraphCopy<>>(GraphId(0, "", second->name + "_" + slot.name), copy,
                                [cross](const FixSection& section, const IfContentPart<std::string, std::string>& part, std::size_t i) {
                                    return movesWithCross(section, part, i, cross);
                                }));
                            crossGraphs_.push_back(copy);
                            renameLists.edits[copy.modObj] = {toList};
                            listGraphs.push_back(copy);
                        }

                        crossEdits_.push_back(std::make_unique<ObjGroupEdit>(std::vector<ObjGroupEdit::IniEdits>{renameLists}));
                        for (const GraphId& list : listGraphs) {
                            crossEdits_.push_back(std::make_unique<GraphInherit<>>(intoGraph, list, IniKeywords::Run, true, onlyInto, atBottom));
                        }
                    }
                }

                using GroupPartEdit = ObjGroupEdit::PartEdit;

                mutable IniFileFixContext ctx_;
                std::string toModName_;
                ZZMIFixerConfig config_;
                ZZMIFixerConfig::Component comp_;
                std::vector<const Template*> own_;

                std::unique_ptr<ObjGroupRemap> remap_;
                std::vector<std::unique_ptr<Fixer::GroupEdit>> preRemapEdits_;
                std::vector<std::unique_ptr<Fixer::GroupEdit>> crossEdits_;
                std::vector<GraphId> crossGraphs_;
                std::vector<std::unique_ptr<GraphCreate<>::Graph>> crossTexGraphs_;
                std::vector<std::unique_ptr<Collector>> collects_;
                std::vector<std::unique_ptr<Collector>> collectsBox_;
                std::vector<std::unique_ptr<Collector>> groupCollectsAfterStrip_;
                std::vector<std::unique_ptr<Fixer::GroupEdit>> groupEdits_;
                std::unique_ptr<ObjGroupEdit> bandEdits_;
                std::vector<std::unique_ptr<BaseResEdit<>>> resEdits_;
                std::vector<std::unique_ptr<BaseRegEdit<>>> regEdits_;
                std::vector<std::unique_ptr<GraphRename<>>> renames_;
                std::vector<std::unique_ptr<GroupPartEdit>> partEdits_;
                // before every section holding a predicate parsed under it: members are destroyed in reverse order
                Z3Context templateZ3_;
                std::vector<std::unique_ptr<FixSection>> fallbackSections_;
                std::unique_ptr<GraphCreate<>::Graph> fallbackGraph_;
                std::unique_ptr<GraphCreate<>::Graph> rawGraph_;
                std::vector<std::unique_ptr<Fixer::GroupEdit>> extrasEdits_;
                std::vector<std::unique_ptr<GraphCreate<>::Graph>> extraGraphs_;
                std::vector<std::unique_ptr<RegIfTemplateAdd<>>> templateAdds_;
                std::vector<std::set<std::string>> frozenNames_;
                std::vector<std::string> frozenRegs_;
                std::vector<std::unique_ptr<RegIfTemplateAdd<>>> defaultAdds_;
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
            return std::make_shared<ZZMIGIMIParser>(iniFile, modTypeId, std::move(modObjs), config);
        };
    }


    IniFixBuilder::Factory makeZZMIFixer(ZZMIFixerConfig config, std::string component) {
        return [config, component](BaseIniParser<>* parser, const std::string& toModName, std::optional<int> modTypeId) {
            return std::make_shared<ZZMIFixerImpl>(parser, toModName, modTypeId, config, component);
        };
    }
}
