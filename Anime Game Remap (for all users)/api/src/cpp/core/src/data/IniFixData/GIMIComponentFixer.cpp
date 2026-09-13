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

#include "AGRemapCore/data/IniFixData/GIMIComponentFixer.h"

#include <algorithm>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/constants/RegFillMissingMode.h"
#include "AGRemapCore/model/IniNamingTools.h"
#include "AGRemapCore/model/VGRemap.h"
#include "AGRemapCore/model/buffers/VGComponentSplit.h"
#include "AGRemapCore/model/files/BlendFile.h"
#include "AGRemapCore/model/files/IbFile.h"
#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/model/iftemplate/IfTemplateRender.h"
#include "AGRemapCore/model/iniresources/VGSplitGroupResource.h"
#include "AGRemapCore/model/strategies/ModType.h"
#include "AGRemapCore/model/strategies/iniFixers/GIMIFixer.h"
#include "AGRemapCore/model/strategies/iniFixers/GIMIObjPartFilter.h"
#include "AGRemapCore/model/strategies/iniFixers/IniFileFixContext.h"
#include "AGRemapCore/model/strategies/iniFixers/graphEdits/GraphRename.h"
#include "AGRemapCore/model/strategies/iniFixers/graphEdits/RegDelimitedAdd.h"
#include "AGRemapCore/model/strategies/iniFixers/graphEdits/RegFillMissing.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphGroupEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphGroupPartEdits.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphGroupRemap.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/ResGroupCollect.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/ResRegCollect.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/VGSplitGroupResBuilder.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/BufEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/TexCreatorEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/TexEditorEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegAssetRemap.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegNewVals.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegRemap.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegRemove.h"
#include "AGRemapCore/model/strategies/iniParsers/BaseIniParser.h"
#include "AGRemapCore/model/strategies/texEditors/TexCreator.h"
#include "AGRemapCore/tools/StringTools.h"
#include "AGRemapCore/tools/files/FileService.h"


namespace AGRemapCore {
    namespace {
        using Fixer = GIMIFixer<>;
        using ModObj = Fixer::ModObj;
        using ObjGroupEdit = GraphGroupEdit<>;
        using GraphId = BaseIniGraphGroupEdit<>::GraphId;
        using Collector = ResRegCollect<>;
        using GroupCollector = ResGroupCollect<>;
        using ObjFilter = GIMIObjPartFilter<>;
        using Template = IfTemplate<std::string, std::string>;

        // NOT named 'GroupRemap' -- GIMIFixer inherits an alias of that name, which would win.
        using SlotRemap = GraphGroupRemap<>;

        const ModObj IbObj{"", "ib"};
        const ModObj BlendObj{"", "blend"};
        const ModObj PositionObj{"", "position"};
        const ModObj TexcoordObj{"", "texcoord"};
        const ModObj OtherObj{"", "other"};
        const ModObj FaceObj{"", "face"};

        const std::string IbHashKey = "ib";
        const std::string PositionHashKey = "position_vb";
        const std::string BlendHashKey = "blend_vb";
        const std::string TexcoordHashKey = "texcoord_vb";
        const std::string FaceDiffuseHashKey = "tex_face_diffuse";

        // The registers a normal-map slot reads: the mod's diffuse (ps-t0) moves to ps-t1 AND is
        // duplicated onto a scratch register the created normal map then replaces, the lightmap
        // moves to ps-t2, and the main edit renames the scratch register back to ps-t0.
        const std::string DiffuseReg = "ps-t0";
        const std::string LightMapReg = "ps-t1";
        const std::string ShiftedDiffuseReg = "ps-t1";
        const std::string ShiftedLightMapReg = "ps-t2";
        const std::string ScratchNormalReg = "ps-tNormal";

        const std::string OverrideByteStride = "override_byte_stride";
        const std::string OverrideVertexCount = "override_vertex_count";
        const std::string DrawIndexedAuto = "auto";

        // A rename rule: one register becoming the listed ones, unconditionally.
        std::pair<std::string, RegRemap<>::KeyRemapValue> renameRule(const std::string& from, std::vector<std::string> to) {
            RemapList<std::string, std::string> targets;
            for (std::string& reg : to) {
                targets.push_back(std::move(reg));
            }

            return {from, RegRemap<>::KeyRemapValue(std::move(targets))};
        }


        BaseResEdit<>::ResEditConfig makeResEditConfig() {
            return BaseResEdit<>::ResEditConfig{
                IniKeywords::Filename,
                [](const std::string& value) { return value; },
                [](const std::string& file) { return file; }
            };
        }


        // ---- what one mod's .ini names, read off the raw parsed sections ----
        //
        // The fixer is built BEFORE the parser parses (the builder's factory runs first), so the
        // files are found by hash over IniFile::getIfTemplates rather than through the parser's
        // graphs -- exactly as the prototype did.
        struct ModObjectFiles {
            std::string ib;
            std::string diffuse;
            std::string lightMap;
        };

        struct ModFiles {
            std::string position;
            std::string blend;
            std::string texcoord;
            std::string face;
            std::vector<std::pair<std::string, ModObjectFiles>> objects;    // in the config's draw order

            std::size_t vertexCount = 0;
            std::size_t positionStride = 0;
            std::size_t texcoordStride = 0;

            std::vector<std::string> ibNames;    // the objects that have an ib, in draw order
            std::vector<std::string> ibPaths;
        };


        std::optional<std::string> firstVal(const Template& tpl, const std::string& key) {
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


        std::size_t fileSize(const std::string& path) {
            std::error_code ec;
            const auto size = std::filesystem::file_size(FileService::strToPath(path), ec);
            return ec ? 0 : static_cast<std::size_t>(size);
        }


        /**
         * The multi-component fixer -- read GIMICharFixerImpl first, this is its shape with the
         * geometry split in the middle. One instance per (mod, target component).
         */
        class GIMIComponentFixerImpl: public Fixer {
            public:
                GIMIComponentFixerImpl(BaseIniParser<>* parser, const std::string& toModName,
                                        std::optional<int> modTypeId, GIMIComponentFixerConfig config,
                                        std::string componentName):
                    Fixer(parser, nullptr, {},
                           toModName.empty() ? std::optional<std::vector<std::string>>(std::nullopt)
                                             : std::optional<std::vector<std::string>>({toModName}),
                           nullptr, makeConfig()),
                    ctx_(parser != nullptr ? parser->getIniFile() : nullptr, modTypeId), toModName_(toModName),
                    config_(std::move(config)), componentName_(std::move(componentName)) {
                    this->setCtx(&ctx_);

                    for (const GIMIComponentFixerConfig::Component& c : config_.components) {
                        if (c.name == componentName_) {
                            component_ = c;
                        }
                    }

                    // Nothing to build without the mod's own files: every edit below is keyed by
                    // what the component draws, which only the split knows.
                    if (!readFiles() || !splitFiles()) {
                        return;
                    }

                    buildSlotRemap();
                    buildTexEdits();
                    buildBufferCollects();
                    buildIndexEdits();
                    buildEdits();

                    this->graphGroupEdits.clear();
                    this->graphGroupEdits.push_back(slotRemap_.get());

                    // Per drawn object, in the prototype's order: the diffuse edit, the lightmap
                    // edit, the register shift, the created normal map.
                    for (auto& edit : texGroupEdits_) {
                        this->graphGroupEdits.push_back(edit);
                    }

                    for (auto& collect : bufferCollects_) {
                        this->graphGroupEdits.push_back(collect.get());
                    }

                    if (!drawn_.empty()) {
                        this->graphGroupEdits.push_back(&indexEdits_);
                        this->graphGroupEdits.push_back(&mainEdits_);
                    }

                    // Nothing hidden: the source and the target are different models with
                    // different hashes, the face included, so no section of the original can fire
                    // on the target.
                    this->copyPreamble = config_.copyPreamble;
                }

            protected:
                // GIMIFixer's own hands every group edit a nullptr .ini file, which stops a
                // collect ever building anything -- see GIMICharFixerImpl.
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
                // ---- the mod's files ----
                bool readFiles() {
                    IniFile* iniFile = ctx_.getIniFile();
                    Hashes* hashes = ctx_.modTypeHashes();
                    Indices* indices = ctx_.modTypeIndices();
                    if (iniFile == nullptr || hashes == nullptr || indices == nullptr) {
                        return false;
                    }

                    const std::string srcName = ctx_.modTypeName().value_or("");
                    const std::optional<Version> version = ctx_.version();
                    const std::string folder = iniFile->getFolder();
                    const auto& templates = iniFile->getIfTemplates();

                    auto fileOf = [&](const std::optional<std::string>& resource) -> std::string {
                        if (!resource.has_value() || resource->empty() || StringTools::equalsIgnoreCase(*resource, IniKeywords::Null)) {
                            return "";
                        }

                        auto it = templates.find(*resource);
                        if (it == templates.end() || it->second == nullptr) {
                            return "";
                        }

                        std::optional<std::string> file = firstVal(*it->second, IniKeywords::Filename);
                        if (!file.has_value() || file->empty()) {
                            return "";
                        }

                        return FileService::absPathOfRelPath(*file, folder);
                    };

                    std::unordered_map<std::string, ModObjectFiles> objects;

                    for (const auto& entry : templates) {
                        if (entry.second == nullptr) {
                            continue;
                        }

                        const Template& tpl = *entry.second;
                        std::optional<std::string> hashVal = firstVal(tpl, IniKeywords::Hash);
                        if (!hashVal.has_value()) {
                            continue;
                        }

                        std::optional<std::vector<std::string>> hashKey =
                            hashes->getKey(StringTools::toLower(*hashVal), version,
                                            std::vector<std::optional<std::string>>{srcName, std::nullopt}, false);
                        if (!hashKey.has_value() || hashKey->empty()) {
                            continue;
                        }

                        const std::string& hashType = hashKey->back();
                        if (hashType == PositionHashKey) {
                            if (files_.position.empty()) {
                                files_.position = fileOf(firstVal(tpl, IniKeywords::Vb0));
                            }
                        } else if (hashType == BlendHashKey) {
                            if (files_.blend.empty()) {
                                files_.blend = fileOf(firstVal(tpl, IniKeywords::Vb1));
                            }
                        } else if (hashType == TexcoordHashKey) {
                            if (files_.texcoord.empty()) {
                                files_.texcoord = fileOf(firstVal(tpl, IniKeywords::Vb1));
                            }
                        } else if (hashType == FaceDiffuseHashKey) {
                            if (files_.face.empty()) {
                                files_.face = fileOf(firstVal(tpl, DiffuseReg));
                            }
                        } else if (hashType == IbHashKey) {
                            std::optional<std::string> index = firstVal(tpl, IniKeywords::MatchFirstIndex);
                            if (!index.has_value()) {
                                continue;
                            }

                            // UNFILTERED by name, on purpose, and checked against the drawn objects
                            // instead. A reverse lookup resolves through the newest version bucket
                            // holding the value, and "0" is every character's head: at the latest
                            // version that bucket holds a 5.3 character's row and nothing of a
                            // 4.0-era one, so a name filter finds nothing (the classifier is
                            // unfiltered for the same reason). The object NAME is what identifies
                            // the row's tail, and every character's head is "head".
                            std::optional<std::vector<std::string>> indexKey = indices->getKey(*index, version, {}, false);
                            if (!indexKey.has_value() || indexKey->empty()) {
                                continue;
                            }

                            const std::string obj = indexKey->back();
                            if (std::find(config_.drawnObjs.begin(), config_.drawnObjs.end(), obj) == config_.drawnObjs.end()
                                    || objects.find(obj) != objects.end()) {
                                continue;
                            }

                            objects[obj] = ModObjectFiles{fileOf(firstVal(tpl, IniKeywords::Ib)),
                                                          fileOf(firstVal(tpl, DiffuseReg)),
                                                          fileOf(firstVal(tpl, LightMapReg))};
                        }
                    }

                    for (const std::string& obj : config_.drawnObjs) {
                        auto it = objects.find(obj);
                        if (it != objects.end()) {
                            files_.objects.emplace_back(obj, it->second);
                            if (!it->second.ib.empty()) {
                                files_.ibNames.push_back(obj);
                                files_.ibPaths.push_back(it->second.ib);
                            }
                        }
                    }

                    return !files_.position.empty() && !files_.blend.empty() && !files_.texcoord.empty() && !files_.ibPaths.empty();
                }

                // ---- the split, once, to know what this component draws ----
                bool splitFiles() {
                    const ModType* modType = ctx_.modType();
                    IniFile* iniFile = ctx_.getIniFile();
                    if (modType == nullptr || modType->vgRemaps == nullptr || iniFile == nullptr) {
                        return false;
                    }

                    const std::string srcName = ctx_.modTypeName().value_or("");
                    const std::optional<Version> fromVersion = ctx_.version();
                    const std::optional<Version> toVersion = iniFile->toVersion;

                    for (const GIMIComponentFixerConfig::Component& c : config_.components) {
                        std::optional<VGRemap> forward = modType->getVGRemap(config_.targetSkin, fromVersion, toVersion, "", c.name);
                        if (!forward.has_value()) {
                            return false;
                        }

                        VGComponentSpec spec;
                        spec.name = c.name;
                        spec.remap = *forward;
                        spec.negativeIndex = c.negativeIndex;

                        // A negative-index component honours the REVERSE row turned around as its
                        // secondary bones: a hair vertex weighted head + bang keeps its head weight
                        // on the Bang's head bone.
                        if (c.negativeIndex) {
                            std::optional<VGRemap> reverse = modType->vgRemaps->get(
                                {config_.targetSkin, c.name, srcName, std::string("")}, {std::nullopt, std::nullopt}, false);
                            if (reverse.has_value()) {
                                const auto& forwardMap = forward->getRemap();
                                for (const auto& entry : reverse->getRemap()) {
                                    const long long bone = entry.first;
                                    const long long source = entry.second;
                                    if (forwardMap.find(source) == forwardMap.end() && spec.secondary.find(source) == spec.secondary.end()) {
                                        spec.secondary[source] = bone;
                                    }
                                }
                            }
                        }

                        specs_.push_back(std::move(spec));
                    }

                    BlendFile blend(files_.blend);
                    auto [weights, indices] = VGComponentSplit::readBlend(blend);
                    files_.vertexCount = weights.size();
                    if (files_.vertexCount == 0) {
                        return false;
                    }

                    files_.positionStride = fileSize(files_.position) / files_.vertexCount;
                    files_.texcoordStride = fileSize(files_.texcoord) / files_.vertexCount;

                    std::vector<VGComponentSplit::Triangles> ibs;
                    for (const std::string& path : files_.ibPaths) {
                        IbFile ib(path);
                        ibs.push_back(VGComponentSplit::readIb(ib));
                    }

                    VGComponentSplit split(std::move(weights), std::move(indices), std::move(ibs), specs_);
                    VGComponentBuffers result = split.split(componentName_);
                    keptVertices_ = result.stats.keptVertices;

                    for (std::size_t i = 0; i < files_.ibNames.size() && i < result.stats.trianglesKept.size(); ++i) {
                        if (result.stats.trianglesKept[i] > 0) {
                            drawn_.push_back(files_.ibNames[i]);
                        }
                    }

                    groupCount_ = std::max<std::size_t>(drawn_.size(), 1);
                    return true;
                }

                const ModObjectFiles* objectFiles(const std::string& name) const {
                    for (const auto& entry : files_.objects) {
                        if (entry.first == name) {
                            return &entry.second;
                        }
                    }

                    return nullptr;
                }

                GraphId slotGraph(std::size_t group) const {
                    return GraphId(group, "", component_.slot);
                }

                // ---- 1. every drawn object's graph onto the component's draw slot ----
                //
                // Every drawn object goes to the SAME slot; the second claimant lands in group 1,
                // which is a second .ini file (the merge). Every other graph is copied unrenamed
                // into every group, so each file is complete on its own -- see GIMICharFixerImpl's
                // buildObjMap for why unrenamed.
                void buildSlotRemap() {
                    SlotRemap::RemapList remap;
                    const SlotRemap::RenameFunc keepName = [](const std::string& name) { return name; };

                    for (const std::string& obj : config_.drawnObjs) {
                        std::vector<SlotRemap::RemapTarget> targets;
                        if (std::find(drawn_.begin(), drawn_.end(), obj) != drawn_.end()) {
                            targets.emplace_back(GraphId(0, "", component_.slot));
                        }

                        remap.emplace_back(GraphId(0, "", obj), std::move(targets));
                    }

                    for (const ModObj& modObj : {IbObj, BlendObj, PositionObj, TexcoordObj, OtherObj, FaceObj}) {
                        std::vector<SlotRemap::RemapTarget> targets;
                        const bool wanted = !drawn_.empty() && (modObj != FaceObj || component_.face);
                        if (wanted) {
                            for (std::size_t i = 0; i < groupCount_; ++i) {
                                targets.emplace_back(GraphId(0, modObj.first, modObj.second), keepName);
                            }
                        }

                        remap.emplace_back(GraphId(0, modObj.first, modObj.second), std::move(targets));
                    }

                    slotRemap_ = std::make_unique<SlotRemap>(std::move(remap));
                }

                // ---- 2. the textures, per drawn object ----
                void buildTexEdits() {
                    if (!component_.normalMap) {
                        return;
                    }

                    for (std::size_t group = 0; group < drawn_.size(); ++group) {
                        const std::string& name = drawn_[group];
                        const ModObjectFiles* files = objectFiles(name);
                        const GraphId slot = slotGraph(group);

                        // The diffuse edit, where the config has one for this object.
                        for (const auto& entry : config_.diffuseEdits) {
                            if (entry.first != name) {
                                continue;
                            }

                            auto replace = std::make_unique<TexEditorReplace<>>(
                                GraphId(group, "", component_.slot + "RemapTexDiffuse"),
                                TexEditor({entry.second}, true, config_.mipmaps), makeResEditConfig(),
                                "resourceRemapTexEdit", std::string("Diffuse"));

                            auto collect = std::make_unique<Collector>();
                            collect->srcRegs = {{slot, DiffuseReg}};
                            collect->resEdits = {{"diffuse", replace.get()}};

                            texGroupEdits_.push_back(collect.get());
                            texReplaces_.push_back(std::move(replace));
                            texCollects_.push_back(std::move(collect));
                        }

                        // The lightmap edit, built from this object's diffuse.
                        if (config_.lightMapEdit && files != nullptr) {
                            TexEditor::Filter filter = config_.lightMapEdit(files->diffuse);
                            if (filter) {
                                auto replace = std::make_unique<TexEditorReplace<>>(
                                    GraphId(group, "", component_.slot + "RemapTexLightMap"),
                                    TexEditor({filter}, true, config_.mipmaps), makeResEditConfig(),
                                    "resourceRemapTexEdit", std::string("LightMap"));

                                auto collect = std::make_unique<Collector>();
                                collect->srcRegs = {{slot, LightMapReg}};
                                collect->resEdits = {{"lightMap", replace.get()}};

                                texGroupEdits_.push_back(collect.get());
                                texReplaces_.push_back(std::move(replace));
                                texCollects_.push_back(std::move(collect));
                            }
                        }

                        // The register shift: ps-t0 -> ps-t1 AND the scratch register, ps-t1 -> ps-t2,
                        // in one pass, on this group's slot graph only.
                        auto shift = std::make_unique<RegRemap<>>(std::vector<std::pair<std::string, RegRemap<>::KeyRemapValue>>{
                            renameRule(DiffuseReg, {ShiftedDiffuseReg, ScratchNormalReg}),
                            renameRule(LightMapReg, {ShiftedLightMapReg})});
                        auto shiftAdapter = std::make_unique<RegPartEdit<>>(shift.get());

                        std::vector<ObjGroupEdit::IniEdits> shiftIniEdits(groupCount_);
                        shiftIniEdits[group].edits[ModObj("", component_.slot)] = {shiftAdapter.get()};
                        shiftIniEdits[group].trackKeys[ModObj("", component_.slot)] = false;
                        auto shiftEdit = std::make_unique<ObjGroupEdit>(std::move(shiftIniEdits), false);

                        texGroupEdits_.push_back(shiftEdit.get());
                        regRemaps_.push_back(std::move(shift));
                        regRemapAdapters_.push_back(std::move(shiftAdapter));
                        shiftEdits_.push_back(std::move(shiftEdit));

                        // The flat normal map, created on the scratch register the shift filled.
                        auto create = std::make_unique<TexCreatorCreate<>>(
                            GraphId(group, "", component_.slot + "RemapNormal"), "NormalMap",
                            TexCreator(config_.flatNormalSize, config_.flatNormalSize, config_.flatNormal, true, config_.mipmaps),
                            makeResEditConfig());

                        auto collect = std::make_unique<Collector>();
                        collect->srcRegs = {{slot, ScratchNormalReg}};
                        collect->resEdits = {{"normalMap", create.get()}};

                        texGroupEdits_.push_back(collect.get());
                        texCreates_.push_back(std::move(create));
                        texAddCollects_.push_back(std::move(collect));
                    }
                }

                // ---- 3. the buffers, as ONE resource group per .ini group ----
                //
                // The blend, the texcoord and this object's ib, plus the position for a cut
                // component (a negative-index component draws every vertex and keeps the mod's own
                // Position.buf). Every drawn object's ib is handed to the split whether or not this
                // group holds it: the vertex set is the union over all of them.
                void buildBufferCollects() {
                    VGSplitGroupConfig splitConfig;
                    splitConfig.component = componentName_;
                    splitConfig.specs = specs_;
                    splitConfig.ibPaths = files_.ibPaths;
                    splitConfig.texcoordLineEdit = makeTexcoordLineEdit();

                    const std::string srcName = ctx_.modTypeName().value_or("");
                    builder_ = std::make_unique<VGSplitGroupResBuilder>(srcName + toModName_ + "Buffers", splitConfig, ctx_.getIniFile());

                    for (std::size_t group = 0; group < drawn_.size(); ++group) {
                        const std::string& name = drawn_[group];

                        std::vector<std::pair<std::string, std::pair<GraphId, std::string>>> kinds = {
                            {"blend", {GraphId(group, BlendObj.first, BlendObj.second), IniKeywords::Vb1}},
                            {"texcoord", {GraphId(group, TexcoordObj.first, TexcoordObj.second), IniKeywords::Vb1}},
                            {"ib", {slotGraph(group), IniKeywords::Ib}}};
                        if (!component_.negativeIndex) {
                            kinds.emplace_back("position", std::make_pair(GraphId(group, PositionObj.first, PositionObj.second), IniKeywords::Vb0));
                        }

                        GroupCollector::ByGraph<GroupCollector::ByGraph<std::string>> srcRegs;
                        GroupCollector::ByGraph<tsl::ordered_map<std::string, GroupCollector::ResEdit*>> resEdits;

                        for (const auto& kind : kinds) {
                            std::string element = kind.first;
                            element[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(element[0])));
                            const GraphId resObj(group, "", component_.slot + "Remap" + element);

                            auto replace = std::make_unique<BufReplace<>>(
                                resObj, makeResEditConfig(), kind.first,
                                kind.first == "ib" ? std::optional<std::string>(name) : std::nullopt);

                            srcRegs[resObj] = {{kind.second.first, kind.second.second}};
                            resEdits[resObj] = {{componentName_, replace.get()}};
                            bufReplaces_.push_back(std::move(replace));
                        }

                        auto collect = std::make_unique<GroupCollector>(
                            std::vector<std::string>{componentName_}, std::move(srcRegs), std::move(resEdits),
                            tsl::ordered_map<std::string, GroupCollector::GroupedResBuilder*>{{componentName_, builder_.get()}},
                            [](const std::string& sectionName) { return sectionName; },
                            static_cast<long long>(group));

                        bufferCollects_.push_back(std::move(collect));
                    }
                }

                VGSplitGroupConfig::LineEdit makeTexcoordLineEdit() const {
                    const bool normalise = config_.normaliseVertexColour;
                    const bool zeroUV = config_.zeroSecondUV && files_.texcoordStride == 20;
                    if (!normalise && !zeroUV) {
                        return nullptr;
                    }

                    return [normalise, zeroUV](const ByteVec& line) {
                        ByteVec out = line;
                        if (normalise && out.size() >= 3) {
                            out[1] = 128;
                            out[2] = 128;
                        }

                        if (zeroUV && out.size() >= 20) {
                            std::fill(out.begin() + 12, out.begin() + 20, static_cast<std::uint8_t>(0));
                        }

                        return out;
                    };
                }

                // ---- 4. the index, windowed to the copied object's own KVPs ----
                void buildIndexEdits() {
                    IniFile* iniFile = ctx_.getIniFile();
                    Indices* indices = ctx_.modTypeIndices();
                    const std::optional<Version> toVersion = (iniFile == nullptr) ? std::nullopt : iniFile->toVersion;

                    objFilter_ = std::make_unique<ObjFilter>(ctx_.modTypeHashes(), ctx_.modTypeIndices(),
                                                              ObjFilter::KeySet{IbHashKey}, ctx_.version());

                    // The config's value first -- see GIMIComponentFixerConfig::Component::slotIndex
                    // for why the table is the fallback and not the source.
                    std::optional<std::string> slotIndex;
                    if (!component_.slotIndex.empty()) {
                        slotIndex = component_.slotIndex;
                    } else if (indices != nullptr) {
                        slotIndex = indices->get({toModName_, "", component_.slot}, toVersion, false);
                    }

                    std::vector<ObjGroupEdit::IniEdits> indexIniEdits(groupCount_);
                    const ModObj slotObj("", component_.slot);

                    for (std::size_t group = 0; group < drawn_.size(); ++group) {
                        if (!slotIndex.has_value()) {
                            break;
                        }

                        auto edit = std::make_unique<RegNewVals<>>(
                            std::vector<std::pair<std::string, RegNewVals<>::NewValSpec>>{
                                {IniKeywords::MatchFirstIndex, RegNewVals<>::NewValSpec(RegNewVals<>::NewVal(*slotIndex))}});
                        auto adapter = std::make_unique<RegPartEdit<>>(edit.get());

                        indexIniEdits[group].edits[slotObj] = {adapter.get()};
                        indexIniEdits[group].keyFilters[slotObj] = {objFilter_->filter(ModObj("", drawn_[group]))};
                        indexIniEdits[group].keysToTrack[slotObj] = objFilter_->keysToTrack();
                        indexIniEdits[group].trackKeys[slotObj] = true;

                        indexAdapters_.push_back(std::move(adapter));
                        indexRegEdits_.push_back(std::move(edit));
                    }

                    indexEdits_ = ObjGroupEdit(std::move(indexIniEdits), false);
                }

                // ---- 5. everything else, per group ----
                void buildEdits() {
                    IniFile* iniFile = ctx_.getIniFile();
                    const std::optional<Version> toVersion = (iniFile == nullptr) ? std::nullopt : iniFile->toVersion;
                    const std::string toModName = toModName_;

                    renameGraph_ = std::make_unique<GraphRename<>>(
                        [toModName](const std::string& n) { return IniNamingTools::getRemapFixName(n, toModName); });
                    renameIbGraph_ = std::make_unique<GraphRename<>>(
                        [toModName](const std::string& n) { return IniNamingTools::getRemapIbName(n, toModName); });
                    renameBlendGraph_ = std::make_unique<GraphRename<>>(
                        [toModName](const std::string& n) { return IniNamingTools::getRemapBlendName(n, toModName); });

                    assetRemap_ = std::make_unique<RegAssetRemap<>>(
                        std::vector<std::pair<std::string, RegAssetRemap<>::AssetSpec>>{
                            {IniKeywords::Hash, RegAssetRemap<>::AssetSpec(ctx_.modTypeHashes(), IniKeywords::HashNotFound)}},
                        toModName_, ctx_.modTypeName().value_or(""), ctx_.version(), toVersion);

                    // The face's own, lenient remap -- see GIMICharFixerImpl's faceAssetRemap_.
                    faceAssetRemap_ = std::make_unique<RegAssetRemap<>>(
                        std::vector<std::pair<std::string, RegAssetRemap<>::AssetSpec>>{
                            {IniKeywords::Hash, RegAssetRemap<>::AssetSpec(ctx_.modTypeHashes())}},
                        toModName_, ctx_.modTypeName().value_or(""), ctx_.version(), toVersion);

                    faceRegSwap_ = std::make_unique<RegRemap<>>(std::vector<std::pair<std::string, RegRemap<>::KeyRemapValue>>{
                        renameRule(DiffuseReg, {LightMapReg}),
                        renameRule(LightMapReg, {DiffuseReg})});

                    auto isFixCall = [](long long, const std::string& value) {
                        return value == IniKeywords::ORFixPath || value == IniKeywords::NNFixPath;
                    };
                    removeFixCalls_ = std::make_unique<RegRemove<>>(
                        std::vector<std::pair<std::string, std::optional<RegRemove<>::RemoveKeyCheck>>>{
                            {IniKeywords::Run, RegRemove<>::RemoveKeyCheck(isFixCall)}});

                    removeDrawIndexed_ = std::make_unique<RegRemove<>>(
                        std::vector<std::pair<std::string, std::optional<RegRemove<>::RemoveKeyCheck>>>{
                            {IniKeywords::DrawIndexed, std::nullopt}});

                    // BottomCover: the collects above spliced their registers into `if 1 ... endif`
                    // blocks, which split the section into parts, and the default FillMissing
                    // would put the draw in the FIRST part, ahead of the ib and the textures.
                    fillDrawIndexed_ = std::make_unique<RegFillMissing<>>(
                        IniKeywords::DrawIndexed,
                        RegFillMissing<>::makeFillMissing(IniKeywords::DrawIndexed, DrawIndexedAuto),
                        RegFillMissingMode::BottomCover);

                    const std::string fixPath = component_.normalMap ? IniKeywords::ORFixPath : IniKeywords::NNFixPath;
                    addFixCall_ = std::make_unique<RegDelimitedAdd<>>(
                        RegDelimitedAdd<>::Additions{{IniKeywords::Run, fixPath}},
                        RegDelimitedAdd<>::RegMap{{IniKeywords::DrawIndexed, {}}},
                        /*pathEndOnlyWhenUndelimited*/ true);

                    normalBack_ = std::make_unique<RegRemap<>>(std::vector<std::pair<std::string, RegRemap<>::KeyRemapValue>>{
                        renameRule(ScratchNormalReg, {DiffuseReg})});

                    // The vertex-limit raise: the target's own buffer is sized for its own vertex
                    // count, and the mod's kept vertices go through it.
                    overrides_ = std::make_unique<RegNewVals<>>(
                        std::vector<std::pair<std::string, RegNewVals<>::NewValSpec>>{
                            {OverrideByteStride, RegNewVals<>::NewValSpec(RegNewVals<>::NewVal(std::to_string(files_.positionStride)))},
                            {OverrideVertexCount, RegNewVals<>::NewValSpec(RegNewVals<>::NewVal(std::to_string(keptVertices_)))}},
                        /*addNewKVPs*/ true);

                    // A cut component draws only the vertices it kept.
                    blendDraw_ = std::make_unique<RegNewVals<>>(
                        std::vector<std::pair<std::string, RegNewVals<>::NewValSpec>>{
                            {IniKeywords::Draw, RegNewVals<>::NewValSpec(RegNewVals<>::NewVal(std::to_string(keptVertices_) + ",0"))}});

                    renameAdapter_ = std::make_unique<GraphPartEdit<>>(renameGraph_.get());
                    renameIbAdapter_ = std::make_unique<GraphPartEdit<>>(renameIbGraph_.get());
                    renameBlendAdapter_ = std::make_unique<GraphPartEdit<>>(renameBlendGraph_.get());
                    assetAdapter_ = std::make_unique<RegPartEdit<>>(assetRemap_.get());
                    faceAssetAdapter_ = std::make_unique<RegPartEdit<>>(faceAssetRemap_.get());
                    faceSwapAdapter_ = std::make_unique<RegPartEdit<>>(faceRegSwap_.get());
                    removeFixCallsAdapter_ = std::make_unique<RegPartEdit<>>(removeFixCalls_.get());
                    removeDrawIndexedAdapter_ = std::make_unique<RegPartEdit<>>(removeDrawIndexed_.get());
                    fillAdapter_ = std::make_unique<GraphPartEdit<>>(fillDrawIndexed_.get());
                    addFixCallAdapter_ = std::make_unique<GraphPartEdit<>>(addFixCall_.get());
                    normalBackAdapter_ = std::make_unique<RegPartEdit<>>(normalBack_.get());
                    overridesAdapter_ = std::make_unique<RegPartEdit<>>(overrides_.get());
                    blendDrawAdapter_ = std::make_unique<RegPartEdit<>>(blendDraw_.get());

                    const ModObj slotObj("", component_.slot);
                    std::vector<ObjGroupEdit::IniEdits> perGroup;

                    for (std::size_t group = 0; group < groupCount_; ++group) {
                        ObjGroupEdit::IniEdits iniEdits;

                        std::vector<ObjGroupEdit::PartEdit*> slotEdits = {removeFixCallsAdapter_.get()};
                        if (component_.normalMap) {
                            slotEdits.push_back(normalBackAdapter_.get());
                        }
                        slotEdits.push_back(fillAdapter_.get());
                        slotEdits.push_back(addFixCallAdapter_.get());
                        slotEdits.push_back(assetAdapter_.get());
                        iniEdits.edits[slotObj] = std::move(slotEdits);
                        iniEdits.trackKeys[slotObj] = false;

                        iniEdits.edits[IbObj] = {renameIbAdapter_.get(), assetAdapter_.get(), removeDrawIndexedAdapter_.get()};
                        iniEdits.trackKeys[IbObj] = false;

                        std::vector<ObjGroupEdit::PartEdit*> blendEdits = {renameBlendAdapter_.get(), assetAdapter_.get()};
                        if (!component_.negativeIndex) {
                            blendEdits.push_back(blendDrawAdapter_.get());
                        }
                        iniEdits.edits[BlendObj] = std::move(blendEdits);
                        iniEdits.trackKeys[BlendObj] = false;

                        iniEdits.edits[PositionObj] = {renameAdapter_.get(), assetAdapter_.get()};
                        iniEdits.trackKeys[PositionObj] = false;

                        iniEdits.edits[TexcoordObj] = {renameAdapter_.get(), assetAdapter_.get()};
                        iniEdits.trackKeys[TexcoordObj] = false;

                        iniEdits.edits[OtherObj] = {renameAdapter_.get(), assetAdapter_.get(), overridesAdapter_.get()};
                        iniEdits.trackKeys[OtherObj] = false;

                        if (component_.face) {
                            iniEdits.edits[FaceObj] = {renameAdapter_.get(), faceAssetAdapter_.get(), faceSwapAdapter_.get()};
                            iniEdits.trackKeys[FaceObj] = false;
                        }

                        perGroup.push_back(std::move(iniEdits));
                    }

                    mainEdits_ = ObjGroupEdit(std::move(perGroup), false);
                }

                static FixerConfig makeConfig() {
                    FixerConfig config{};
                    config.sectionToStr = &renderIfTemplate;
                    return config;
                }

                IniFileFixContext ctx_;
                std::string toModName_;
                GIMIComponentFixerConfig config_;
                std::string componentName_;
                GIMIComponentFixerConfig::Component component_;

                ModFiles files_;
                std::vector<VGComponentSpec> specs_;
                std::vector<std::string> drawn_;
                std::size_t keptVertices_ = 0;
                std::size_t groupCount_ = 1;

                std::unique_ptr<SlotRemap> slotRemap_;

                std::vector<Fixer::GroupEdit*> texGroupEdits_;
                std::vector<std::unique_ptr<TexEditorReplace<>>> texReplaces_;
                std::vector<std::unique_ptr<Collector>> texCollects_;
                std::vector<std::unique_ptr<TexCreatorCreate<>>> texCreates_;
                std::vector<std::unique_ptr<Collector>> texAddCollects_;
                std::vector<std::unique_ptr<RegRemap<>>> regRemaps_;
                std::vector<std::unique_ptr<RegPartEdit<>>> regRemapAdapters_;
                std::vector<std::unique_ptr<ObjGroupEdit>> shiftEdits_;

                std::unique_ptr<VGSplitGroupResBuilder> builder_;
                std::vector<std::unique_ptr<BufReplace<>>> bufReplaces_;
                std::vector<std::unique_ptr<GroupCollector>> bufferCollects_;

                std::unique_ptr<ObjFilter> objFilter_;
                std::vector<std::unique_ptr<RegNewVals<>>> indexRegEdits_;
                std::vector<std::unique_ptr<RegPartEdit<>>> indexAdapters_;
                ObjGroupEdit indexEdits_;

                std::unique_ptr<GraphRename<>> renameGraph_;
                std::unique_ptr<GraphRename<>> renameIbGraph_;
                std::unique_ptr<GraphRename<>> renameBlendGraph_;
                std::unique_ptr<RegAssetRemap<>> assetRemap_;
                std::unique_ptr<RegAssetRemap<>> faceAssetRemap_;
                std::unique_ptr<RegRemap<>> faceRegSwap_;
                std::unique_ptr<RegRemove<>> removeFixCalls_;
                std::unique_ptr<RegRemove<>> removeDrawIndexed_;
                std::unique_ptr<RegFillMissing<>> fillDrawIndexed_;
                std::unique_ptr<RegDelimitedAdd<>> addFixCall_;
                std::unique_ptr<RegRemap<>> normalBack_;
                std::unique_ptr<RegNewVals<>> overrides_;
                std::unique_ptr<RegNewVals<>> blendDraw_;

                std::unique_ptr<GraphPartEdit<>> renameAdapter_;
                std::unique_ptr<GraphPartEdit<>> renameIbAdapter_;
                std::unique_ptr<GraphPartEdit<>> renameBlendAdapter_;
                std::unique_ptr<RegPartEdit<>> assetAdapter_;
                std::unique_ptr<RegPartEdit<>> faceAssetAdapter_;
                std::unique_ptr<RegPartEdit<>> faceSwapAdapter_;
                std::unique_ptr<RegPartEdit<>> removeFixCallsAdapter_;
                std::unique_ptr<RegPartEdit<>> removeDrawIndexedAdapter_;
                std::unique_ptr<GraphPartEdit<>> fillAdapter_;
                std::unique_ptr<GraphPartEdit<>> addFixCallAdapter_;
                std::unique_ptr<RegPartEdit<>> normalBackAdapter_;
                std::unique_ptr<RegPartEdit<>> overridesAdapter_;
                std::unique_ptr<RegPartEdit<>> blendDrawAdapter_;

                ObjGroupEdit mainEdits_;
        };
    }

    IniFixBuilder::Factory makeGIMIComponentFixer(GIMIComponentFixerConfig config, std::string component) {
        return [config, component](BaseIniParser<>* parser, const std::string& toModName, std::optional<int> modTypeId) {
            return std::make_shared<GIMIComponentFixerImpl>(parser, toModName, modTypeId, config, component);
        };
    }
}
