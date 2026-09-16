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

#include "AGRemapCore/data/IniFixData/GIMIMergeFixer.h"

#include <algorithm>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/constants/RegDelimitedAddMode.h"
#include "AGRemapCore/constants/RegFillMissingMode.h"
#include "AGRemapCore/model/IniNamingTools.h"
#include "AGRemapCore/tools/TextTools.h"
#include "AGRemapCore/model/VGRemap.h"
#include "AGRemapCore/model/buffers/VGComponentMerge.h"
#include "AGRemapCore/model/IniSectionGraph.h"
#include "AGRemapCore/model/iftemplate/IfTemplate.h"
#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/model/iftemplate/IfTemplateRender.h"
#include "AGRemapCore/model/iniresources/VGMergeGroupResource.h"
#include "AGRemapCore/model/strategies/ModType.h"
#include "AGRemapCore/model/strategies/iniFixers/GIMIFixer.h"
#include "AGRemapCore/model/strategies/iniFixers/GIMIObjPartFilter.h"
#include "AGRemapCore/model/strategies/iniFixers/IniFileFixContext.h"
#include "AGRemapCore/model/strategies/iniFixers/graphEdits/GraphRename.h"
#include "AGRemapCore/model/strategies/iniFixers/graphEdits/RegBottomAdd.h"
#include "AGRemapCore/model/strategies/iniFixers/graphEdits/RegBranchAdd.h"
#include "AGRemapCore/model/strategies/iniFixers/graphEdits/RegDelimitedAdd.h"
#include "AGRemapCore/model/strategies/iniFixers/graphEdits/RegFillMissing.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphGroupEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphGroupPartEdits.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphGroupRemap.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/ResGroupCollect.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/ResRegCollect.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/VGMergeGroupResBuilder.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/BufEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/TexEditorEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegAssetRemap.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegNewVals.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegRemap.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegRemove.h"
#include "AGRemapCore/model/strategies/iniParsers/BaseIniParser.h"
#include "AGRemapCore/tools/StringTools.h"
#include "AGRemapCore/tools/DownloadTools.h"
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

        const ModObj FaceObj{"", "face"};

        const std::string IbHashKey = "ib";
        const std::string PositionHashKey = "position_vb";
        const std::string BlendHashKey = "blend_vb";
        const std::string TexcoordHashKey = "texcoord_vb";
        const std::string FaceDiffuseHashKey = "tex_face_diffuse";

        const std::string DiffuseReg = "ps-t0";
        const std::string LightMapReg = "ps-t1";
        const std::string NormalShiftedDiffuseReg = "ps-t1";
        const std::string NormalShiftedLightMapReg = "ps-t2";

        const std::string OverrideByteStride = "override_byte_stride";
        const std::string OverrideVertexCount = "override_vertex_count";
        const std::string DrawIndexedAuto = "auto";

        const std::size_t BlendStride = 32;

        // Every ib this fix reads and writes is DXGI_FORMAT_R32_UINT -- four bytes an index. The
        // merge's own output declares that format, and so does every source mod measured.
        const std::size_t IbIndexStride = 4;



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


        // Every value of one key in a section AND in everything that section `run =`s, in order.
        //
        // A merged mod's master binds nothing directly: its TextureOverride carries
        // `run = CommandListX` and the real `vb1 =` sits inside a $swapvar branch, one per variant.
        // Reading only the section the hash matched finds nothing at all, which is why every buffer
        // path of such a mod came out empty and the merge then failed on all of them.
        //
        // IniSectionGraph is the library's call-graph walker -- it follows `run =` transitively and
        // copes with cycles -- so this asks it rather than hand-rolling a second walker. The graph is
        // built over the RAW parsed sections because a fixer is constructed before the parser runs,
        // so the parser's own graphs do not exist yet.
        // One value a register takes, with the condition it is taken under.
        //
        // The condition is the whole reason this is not a plain string: which branch of the Bang's
        // CommandList goes with which branch of the Body's is decided by whether the two can hold at
        // the same time, so a value that arrives without its condition cannot be paired at all --
        // see configForGroup.
        struct BranchVal {
            std::string val;
            std::optional<Z3Predicate> query;
        };


        std::vector<BranchVal> valsThroughRun(const tsl::ordered_map<std::string, std::unique_ptr<Template>>& templates,
                                               const std::string& rootSection, const std::string& key,
                                               Z3Context* z3Ctx) {
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

            // The context is the CALLER's, not one made here: a Z3Predicate is only usable while
            // the context it was generated in is alive, and these outlive this call by the whole
            // life of the fixer.
            IniSectionGraph<std::string, std::string> graph(std::move(sections), {rootSection}, runConfig, true, false,
                                                            z3Ctx);

            // iterByQuery rather than a walk over parts(): it is the same iteration, and it reports
            // the conditional predicate each part sits under, which is the half that is needed.
            tsl::ordered_map<std::string, std::vector<BranchVal>> bySection;

            auto parts = graph.iterByQuery();
            while (parts.next()) {
                auto& iterData = parts.value();
                if (iterData.part == nullptr) {
                    continue;
                }

                for (const std::string& val : iterData.part->getVals(key)) {
                    bySection[iterData.sectionName].push_back(BranchVal{std::string(StringTools::strip(val)),
                                                                        iterData.query});
                }
            }

            // Root first, then everything it reaches, in the graph's own order. Order matters only
            // in that the FIRST value is the one a single-valued caller takes, and the root's own
            // binding should win -- which is why this is regrouped by section rather than emitted in
            // iteration order.
            std::vector<BranchVal> out;
            const auto append = [&out, &bySection](const std::string& sectionName) {
                auto it = bySection.find(sectionName);
                if (it == bySection.end()) {
                    return;
                }

                // Copied, not moved: tsl::ordered_map hands out CONST values through its
                // iterator even from a non-const begin().
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


        std::optional<std::string> firstValThroughRun(const tsl::ordered_map<std::string, std::unique_ptr<Template>>& templates,
                                                       const std::string& rootSection, const std::string& key,
                                                       Z3Context* z3Ctx) {
            std::vector<BranchVal> vals = valsThroughRun(templates, rootSection, key, z3Ctx);
            if (vals.empty()) {
                return std::nullopt;
            }

            return vals.front().val;
        }


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
            if (path.empty()) {
                return 0;
            }

            std::error_code ec;
            auto size = std::filesystem::file_size(FileService::strToPath(path), ec);
            return ec ? 0 : static_cast<std::size_t>(size);
        }


        // ---- what one source SLOT's section names ----
        struct SlotFiles {
            std::string ib;

            // As ComponentFiles' lists: one ib per $swapvar branch of a merged master.
            std::vector<BranchVal> ibs;

            // Some branch's ib is explicitly `null` -- the author hid this object there, which
            // three of one NSFW mod's twelve variants do to take the gloves off. That is NOT the
            // same as a component the mod does not have: there is nothing to download, so the
            // download fallback must not invent one. resourceOf flattens both cases to an empty
            // string, which is why the distinction is kept here.
            //
            // The nulled branch is still IN #ibs, as an entry with no file. It is a real state of
            // the mod and has to be able to answer for itself -- dropped, it left nine entries
            // describing twelve states and every entry after the first null answered for the wrong
            // one (measured: branch 7's merged body came out as branch 7's head plus branch EIGHT's
            // body). #ib is the first branch that does have a file, for everything that needs one
            // path rather than the branch's own.
            bool nullIb = false;
            std::string diffuse;
            std::string lightMap;
            std::string diffuseRes;      // the .ini resource NAME, for a borrowing slot to reference
            std::string lightMapRes;
            bool found = false;

            // How many indices this slot's ib holds -- measured, or the config's game-model
            // fallback when the file is a download that is not on disk yet.
            long long indexCount = 0;

            // Whether the mod's own section for this slot issues ANY drawindexed. A mod that draws
            // for itself has expressed what it wants drawn, and `drawindexed = auto` on top of that
            // always draws something twice -- see where fillAdapter_ is applied.
            bool draws = false;

            // Every draw the slot issues, with the condition it issues it under. A merged master
            // draws in SOME of its branches and not others -- this mod issues none in four of its
            // twelve -- so "does the mod draw here" is a per-branch question wherever the branches
            // are different models.
            std::vector<BranchVal> drawVals;
        };

        // ---- what one source COMPONENT's sections name ----
        struct ComponentFiles {
            std::string position;
            std::string blend;
            std::string texcoord;

            // EVERY value the register takes through `run =`, not just the first, each with the
            // condition it is taken under. A merged mod's master names one variant's buffer per
            // $swapvar branch, and each branch is a different mod that has to be merged on its own
            // -- see configForGroup. One entry for an ordinary mod, which is the same thing said
            // once.
            std::vector<BranchVal> positions;
            std::vector<BranchVal> blends;
            std::vector<BranchVal> texcoords;
            std::size_t vertexCount = 0;
            std::size_t positionStride = 0;
            std::unordered_map<std::string, SlotFiles> slots;     // by slot name
        };


        /**
         * A mod of a skin of SEVERAL components, fixed onto a target of one.
         *
         * The inverse of GIMIComponentFixerImpl, and simpler in one structural way: there is always
         * exactly ONE .ini group. The target draws through one set of buffer hashes, so the
         * components' buffers become one merged set, and two source slots landing on one target
         * object are concatenated into a single draw rather than spilling into a second file.
         */
        class GIMIMergeFixerImpl: public Fixer {
            public:
                GIMIMergeFixerImpl(BaseIniParser<>* parser, const std::string& toModName,
                                    std::optional<int> modTypeId, GIMIMergeFixerConfig config):
                    Fixer(parser, nullptr, {},
                           toModName.empty() ? std::optional<std::vector<std::string>>(std::nullopt)
                                             : std::optional<std::vector<std::string>>({toModName}),
                           nullptr, makeConfig()),
                    ctx_(parser != nullptr ? parser->getIniFile() : nullptr, modTypeId), toModName_(toModName),
                    config_(std::move(config)) {
                    this->setCtx(&ctx_);

                    if (!readFiles()) {
                        return;
                    }

                    resolveTargets();
                    if (drawn_.empty()) {
                        return;
                    }

                    buildSlotRemap();
                    buildBorrowEdits();
                    buildTexEdits();
                    buildBufferCollect();
                    buildIndexEdits();
                    buildEdits();

                    this->graphGroupEdits.clear();

                    // BEFORE the slot remap, which is the point: these read a MEMBER's own graph,
                    // and the remap folds those into the target's.
                    for (auto& edit : preRemapTexGroupEdits_) {
                        this->graphGroupEdits.push_back(edit);
                    }

                    this->graphGroupEdits.push_back(slotRemap_.get());

                    if (borrowEdit_ != nullptr) {
                        this->graphGroupEdits.push_back(borrowEdit_.get());
                    }

                    for (auto& edit : texGroupEdits_) {
                        this->graphGroupEdits.push_back(edit);
                    }

                    if (bufferCollect_ != nullptr) {
                        this->graphGroupEdits.push_back(bufferCollect_.get());
                    }

                    this->graphGroupEdits.push_back(&indexEdits_);
                    this->graphGroupEdits.push_back(&mainEdits_);

                    // Nothing hidden: source and target are different models with different hashes.
                    this->copyPreamble = config_.copyPreamble;
                }

            protected:
                // GIMIFixer's own hands every group edit a nullptr .ini file, which stops a collect
                // ever building anything -- see GIMIComponentFixerImpl.
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
                // ---- the mod's files, per SOURCE component ----
                //
                // Each component's hashes are filed under the COMPONENT's mod type name, so the
                // reverse lookup is filtered to that rather than to the skin's own name -- the same
                // asymmetry the parser has, and for the same reason.
                bool readFiles() {
                    IniFile* iniFile = ctx_.getIniFile();
                    Hashes* hashes = ctx_.modTypeHashes();
                    if (iniFile == nullptr || hashes == nullptr) {
                        return false;
                    }

                    const std::optional<Version> version = ctx_.version();
                    const std::string folder = iniFile->getFolder();
                    const auto& templates = iniFile->getIfTemplates();

                    auto resourceOf = [&](const std::optional<std::string>& resource) -> std::string {
                        if (!resource.has_value() || resource->empty()
                                || StringTools::equalsIgnoreCase(*resource, IniKeywords::Null)) {
                            return "";
                        }
                        return *resource;
                    };

                    auto fileOf = [&](const std::string& resource) -> std::string {
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
                    };

                    for (const GIMIMergeFixerConfig::Component& component : config_.components) {
                        ComponentFiles files;

                        for (const auto& entry : templates) {
                            if (entry.second == nullptr) {
                                continue;
                            }

                            const Template& tpl = *entry.second;
                            std::optional<std::string> hashVal = firstVal(tpl, IniKeywords::Hash);
                            if (!hashVal.has_value()) {
                                continue;
                            }

                            std::optional<std::vector<std::string>> hashKey = hashes->getKey(
                                StringTools::toLower(*hashVal), version,
                                std::vector<std::optional<std::string>>{componentModTypeName(component.name), std::nullopt}, false);
                            if (!hashKey.has_value() || hashKey->empty()) {
                                continue;
                            }

                            // THROUGH `run =`, not just off the matched section: a merged mod's
                            // master binds its buffers inside a CommandList. See valsThroughRun.
                            const std::string& sectionName = entry.first;
                            const std::string& hashType = hashKey->back();

                            // Every branch, not just the first -- see ComponentFiles::blends.
                            const auto filesThroughRun = [&](const std::string& reg) {
                                std::vector<BranchVal> out;
                                for (const BranchVal& resource : valsThroughRun(templates, sectionName, reg, &z3Ctx_)) {
                                    std::string file = fileOf(resource.val);
                                    if (!file.empty()) {
                                        out.push_back(BranchVal{std::move(file), resource.query});
                                    }
                                }

                                return out;
                            };

                            if (hashType == PositionHashKey) {
                                if (files.positions.empty()) {
                                    files.positions = filesThroughRun(IniKeywords::Vb0);
                                    if (!files.positions.empty()) {
                                        files.position = files.positions.front().val;
                                    }
                                }
                            } else if (hashType == BlendHashKey) {
                                if (files.blends.empty()) {
                                    files.blends = filesThroughRun(IniKeywords::Vb1);
                                    if (!files.blends.empty()) {
                                        files.blend = files.blends.front().val;
                                    }
                                }
                            } else if (hashType == TexcoordHashKey) {
                                if (files.texcoords.empty()) {
                                    files.texcoords = filesThroughRun(IniKeywords::Vb1);
                                    if (!files.texcoords.empty()) {
                                        files.texcoord = files.texcoords.front().val;
                                    }
                                }
                            } else if (hashType == FaceDiffuseHashKey) {
                                if (faceFile_.empty()) {
                                    faceFile_ = fileOf(resourceOf(firstValThroughRun(templates, sectionName, DiffuseReg,
                                                                                     &z3Ctx_)));
                                }
                            } else if (hashType == IbHashKey) {
                                std::optional<std::string> index = firstVal(tpl, IniKeywords::MatchFirstIndex);
                                if (!index.has_value()) {
                                    continue;
                                }

                                for (const GIMIMergeFixerConfig::Slot& slot : component.slots) {
                                    if (slot.index != *index || files.slots.count(slot.name) != 0) {
                                        continue;
                                    }

                                    // The layout is read off the MOD, not assumed from the
                                    // config: a mod may bind its objects differently, and a slot
                                    // with a ps-t2 is the three-register normal-map layout.
                                    //
                                    // THROUGH `run =`, like the buffers: a merged master's
                                    // TextureOverride is nothing but hash, match_first_index and
                                    // run, so reading the registers off the matched section alone
                                    // found none and every such mod was taken for the two-register
                                    // layout. The normal map then stayed at ps-t0, where the
                                    // target's shader reads the diffuse, and the whole model
                                    // rendered pale and flat.
                                    const bool normalMap =
                                        !resourceOf(firstValThroughRun(templates, sectionName, "ps-t2", &z3Ctx_)).empty();
                                    SlotFiles slotFiles;
                                    slotFiles.found = true;
                                    for (const BranchVal& rawIb : valsThroughRun(templates, sectionName, IniKeywords::Ib,
                                                                                 &z3Ctx_)) {
                                        // `ib = null` hides the object in THIS branch, and the
                                        // branch is kept so it can say so -- see SlotFiles::nullIb.
                                        if (StringTools::equalsIgnoreCase(rawIb.val, IniKeywords::Null)) {
                                            slotFiles.nullIb = true;
                                            slotFiles.ibs.push_back(BranchVal{std::string(), rawIb.query});
                                            continue;
                                        }

                                        std::string file = fileOf(resourceOf(rawIb.val));
                                        if (!file.empty()) {
                                            slotFiles.ibs.push_back(BranchVal{std::move(file), rawIb.query});
                                        }
                                    }

                                    for (const BranchVal& candidate : slotFiles.ibs) {
                                        if (!candidate.val.empty()) {
                                            slotFiles.ib = candidate.val;
                                            break;
                                        }
                                    }
                                    // Also through `run =`: a merged master issues its draws
                                    // inside the CommandList's branches, and read off the section
                                    // alone it looks like a mod that draws nothing of its own --
                                    // which adds `drawindexed = auto` on top of the draws it does
                                    // issue, and tells the appended-draw pass it has nothing to do.
                                    slotFiles.drawVals =
                                        valsThroughRun(templates, sectionName, IniKeywords::DrawIndexed, &z3Ctx_);
                                    slotFiles.draws = !slotFiles.drawVals.empty();

                                    // Measured first, config second -- see Slot::indexCount. Only
                                    // a target object several slots land on ever reads this.
                                    slotFiles.indexCount =
                                        static_cast<long long>(fileSize(slotFiles.ib) / IbIndexStride);
                                    if (slotFiles.indexCount == 0) {
                                        slotFiles.indexCount = slot.indexCount;
                                    }
                                    slotFiles.diffuseRes = resourceOf(firstValThroughRun(
                                        templates, sectionName, normalMap ? NormalShiftedDiffuseReg : DiffuseReg, &z3Ctx_));
                                    slotFiles.lightMapRes = resourceOf(firstValThroughRun(
                                        templates, sectionName, normalMap ? NormalShiftedLightMapReg : LightMapReg, &z3Ctx_));
                                    slotFiles.diffuse = fileOf(slotFiles.diffuseRes);
                                    slotFiles.lightMap = fileOf(slotFiles.lightMapRes);
                                    normalMap_[key(component.name, slot.name)] = normalMap;
                                    files.slots[slot.name] = std::move(slotFiles);
                                }
                            }
                        }

                        // A component the mod does not carry at all: its buffers are DOWNLOADS the
                        // service has not fetched yet, so nothing is on disk and every path above is
                        // still empty. Point them at where the downloads will land -- named with the
                        // same helper the parser registers them under, so the two cannot drift.
                        //
                        // Without this the empty paths go into VGMergeGroupConfig unchanged and the
                        // merge throws "Unable to open file: " with nothing after the colon, losing
                        // every buffer of that .ini. Measured on an NSFW edit with no Eye sections.
                        if (!config_.downloadPrefix.empty()) {
                            const std::string folder = iniFile->getFolder();
                            const auto downloadPath = [&](const std::string& kind, const std::string& ext) {
                                return FileService::absPathOfRelPath(
                                    DownloadTools::fixedFileName(config_.downloadPrefix, component.name + kind, ext), folder);
                            };

                            if (files.blend.empty()) {
                                files.blend = downloadPath("Blend", ".buf");
                                files.blends = {BranchVal{files.blend, std::nullopt}};
                            }
                            if (files.position.empty()) {
                                files.position = downloadPath("Position", ".buf");
                                files.positions = {BranchVal{files.position, std::nullopt}};
                            }
                            if (files.texcoord.empty()) {
                                files.texcoord = downloadPath("Texcoord", ".buf");
                                files.texcoords = {BranchVal{files.texcoord, std::nullopt}};
                            }

                            for (const GIMIMergeFixerConfig::Slot& slot : component.slots) {
                                SlotFiles& slotFiles = files.slots[slot.name];
                                // A NULLED slot is not a missing one: nothing was ever meant to be
                                // there, so there is no download to point at -- see SlotFiles::nullIb.
                                if (slotFiles.ib.empty() && !slotFiles.nullIb) {
                                    slotFiles.ib = FileService::absPathOfRelPath(
                                        DownloadTools::fixedFileName(config_.downloadPrefix,
                                                                      component.name + slot.name, ".ib"), folder);
                                    slotFiles.ibs = {BranchVal{slotFiles.ib, std::nullopt}};
                                }
                                if (slotFiles.indexCount == 0) {
                                    slotFiles.indexCount = slot.indexCount;
                                }
                            }
                        }

                        files.vertexCount = fileSize(files.blend) / BlendStride;

                        // Nothing on disk yet -- this component is one the mod does not have, and
                        // its buffers are downloads the service has not fetched. Fall back to the
                        // game model's own count; see Component::vertexCount. Without this the
                        // component is dropped by resolveTargets and the merge comes out short by
                        // exactly its vertices, with its downloads written, referenced by nothing
                        // and paid for.
                        if (files.vertexCount == 0 && component.vertexCount > 0) {
                            files.vertexCount = static_cast<std::size_t>(component.vertexCount);
                        }

                        if (files.vertexCount != 0) {
                            const std::size_t positionSize = fileSize(files.position);
                            if (positionSize != 0) {
                                files.positionStride = positionSize / files.vertexCount;
                            }
                        }

                        files_[component.name] = std::move(files);
                    }

                    // A slot with no textures of its own borrows another's -- see Slot::borrowFrom.
                    for (const GIMIMergeFixerConfig::Component& component : config_.components) {
                        for (const GIMIMergeFixerConfig::Slot& slot : component.slots) {
                            SlotFiles* files = slotFiles(component.name, slot.name);
                            if (files == nullptr || !files->diffuseRes.empty() || !files->lightMapRes.empty()
                                    || slot.borrowFrom.empty()) {
                                continue;
                            }

                            const std::size_t sep = slot.borrowFrom.find(';');
                            if (sep == std::string::npos) {
                                continue;
                            }

                            SlotFiles* donor = slotFiles(slot.borrowFrom.substr(0, sep), slot.borrowFrom.substr(sep + 1));
                            if (donor == nullptr) {
                                continue;
                            }

                            files->diffuse = donor->diffuse;
                            files->diffuseRes = donor->diffuseRes;
                            files->lightMap = donor->lightMap;
                            files->lightMapRes = donor->lightMapRes;
                            // Added in the target's own layout, so nothing to shift afterwards.
                            normalMap_[key(component.name, slot.name)] = false;
                            borrowed_.push_back({component.name, slot.name});
                        }
                    }

                    return !files_.empty();
                }

                // ---- which target object each source slot lands on, and who represents it ----
                void resolveTargets() {
                    for (const GIMIMergeFixerConfig::Component& component : config_.components) {
                        const ComponentFiles* files = componentFiles(component.name);
                        if (files == nullptr || files->vertexCount == 0) {
                            continue;
                        }

                        mergeOrder_.push_back(component.name);
                        offsets_[component.name] = totalVertices_;
                        totalVertices_ += files->vertexCount;
                        if (positionStride_ == 0 && files->positionStride != 0) {
                            positionStride_ = files->positionStride;
                        }

                        for (const GIMIMergeFixerConfig::Slot& slot : component.slots) {
                            auto slotIt = files->slots.find(slot.name);
                            if (slotIt == files->slots.end()) {
                                continue;
                            }

                            // `ib = null` on every branch -- no branch left a file behind, so the
                            // author hid this object outright and it brings no geometry to the merge
                            // at all. A slot nulled in only SOME branches is a different thing and
                            // stays: see SlotFiles::nullIb, and pick.
                            if (slotIt->second.nullIb && slotIt->second.ib.empty()) {
                                continue;
                            }

                            if (std::find(drawn_.begin(), drawn_.end(), slot.to) == drawn_.end()) {
                                drawn_.push_back(slot.to);
                                representative_[slot.to] = {component.name, slot.name};
                            }

                            members_[slot.to].push_back({component.name, slot.name});
                        }
                    }

                    // The target's draw order, not the source's -- the .ini's sections come out in
                    // the order the objects are listed.
                    std::vector<std::string> ordered;
                    for (const std::string& obj : config_.targetObjs) {
                        if (std::find(drawn_.begin(), drawn_.end(), obj) != drawn_.end()) {
                            ordered.push_back(obj);
                        }
                    }
                    drawn_ = std::move(ordered);
                }

                // Do this object's members disagree about which textures they read? One section
                // binds one set, so a disagreement is what forces a draw per member.
                bool membersDiffer(const std::string& obj) {
                    auto membersIt = members_.find(obj);
                    auto repIt = representative_.find(obj);
                    if (membersIt == members_.end() || repIt == representative_.end()) {
                        return false;
                    }

                    const SlotFiles* repFiles = slotFiles(repIt->second.first, repIt->second.second);
                    if (repFiles == nullptr) {
                        return false;
                    }

                    for (const auto& member : membersIt->second) {
                        const SlotFiles* files = slotFiles(member.first, member.second);
                        if (files == nullptr) {
                            continue;
                        }

                        if (files->diffuseRes != repFiles->diffuseRes
                                || files->lightMapRes != repFiles->lightMapRes) {
                            return true;
                        }
                    }

                    return false;
                }


                static std::string key(const std::string& component, const std::string& slot) {
                    return component + ";" + slot;
                }

                std::string componentModTypeName(const std::string& component) const {
                    // The skin's own name plus the component, which is how ModTypeId names them.
                    return ctx_.modTypeName().value_or("") + component;
                }

                ComponentFiles* componentFiles(const std::string& component) {
                    auto it = files_.find(component);
                    return it == files_.end() ? nullptr : &it->second;
                }

                const ComponentFiles* componentFiles(const std::string& component) const {
                    auto it = files_.find(component);
                    return it == files_.end() ? nullptr : &it->second;
                }

                SlotFiles* slotFiles(const std::string& component, const std::string& slot) {
                    ComponentFiles* files = componentFiles(component);
                    if (files == nullptr) {
                        return nullptr;
                    }

                    auto it = files->slots.find(slot);
                    return it == files->slots.end() ? nullptr : &it->second;
                }

                const SlotFiles* slotFiles(const std::string& component, const std::string& slot) const {
                    const ComponentFiles* files = componentFiles(component);
                    if (files == nullptr) {
                        return nullptr;
                    }

                    auto it = files->slots.find(slot);
                    return it == files->slots.end() ? nullptr : &it->second;
                }

                // ---- 1. the graphs onto the target's, all in ONE .ini file ----
                void buildSlotRemap() {
                    SlotRemap::RemapList remap;
                    const SlotRemap::RenameFunc keepName = [](const std::string& name) { return name; };
                    const std::string& skeleton = mergeOrder_.front();

                    for (const GIMIMergeFixerConfig::Component& component : config_.components) {
                        const bool isSkeleton = (component.name == skeleton);

                        for (const char* kind : {"ib", "blend", "position", "texcoord", "other"}) {
                            std::vector<SlotRemap::RemapTarget> targets;
                            if (isSkeleton) {
                                targets.emplace_back(GraphId(0, "", kind), keepName);
                            }
                            remap.emplace_back(GraphId(0, component.name, kind), std::move(targets));
                        }

                        for (const GIMIMergeFixerConfig::Slot& slot : component.slots) {
                            std::vector<SlotRemap::RemapTarget> targets;
                            auto it = representative_.find(slot.to);
                            if (it != representative_.end() && it->second.first == component.name
                                    && it->second.second == slot.name) {
                                targets.emplace_back(GraphId(0, "", slot.to));
                            }
                            remap.emplace_back(GraphId(0, component.name, slot.name), std::move(targets));
                        }
                    }

                    std::vector<SlotRemap::RemapTarget> faceTargets;
                    if (!config_.faceReg.empty()) {
                        faceTargets.emplace_back(GraphId(0, FaceObj.first, FaceObj.second), keepName);
                    }
                    remap.emplace_back(GraphId(0, FaceObj.first, FaceObj.second), std::move(faceTargets));

                    slotRemap_ = std::make_unique<SlotRemap>(std::move(remap));
                }

                // ---- 2. a borrowing slot gets its donor's registers ADDED, before any collect ----
                void buildBorrowEdits() {
                    if (borrowed_.empty()) {
                        return;
                    }

                    std::vector<ObjGroupEdit::IniEdits> iniEdits(1);
                    bool any = false;

                    for (const auto& entry : borrowed_) {
                        auto it = representative_.end();
                        for (auto candidate = representative_.begin(); candidate != representative_.end(); ++candidate) {
                            if (candidate->second.first == entry.first && candidate->second.second == entry.second) {
                                it = candidate;
                                break;
                            }
                        }
                        if (it == representative_.end()) {
                            continue;
                        }

                        const SlotFiles* files = slotFiles(entry.first, entry.second);
                        if (files == nullptr || files->diffuseRes.empty() || files->lightMapRes.empty()) {
                            continue;
                        }

                        // BEFORE THE FIRST DRAW ON EVERY PATH, which a plain append is not.
                        //
                        // This used to be a RegNewVals with addNewKVPs, and that puts the bindings at
                        // the END of the part -- fine while a borrowing slot's section only declares
                        // an ib and lets the fix supply `drawindexed = auto` at the bottom, which is
                        // every mod this met until one turned up whose Bang declares an ib, no
                        // textures, and its own literal draw. The bindings then landed AFTER that
                        // draw and the hair rendered with whatever the previous draw had left bound
                        // (2026-09-14).
                        //
                        // A texture binding has the same placement rule as the fix call that follows
                        // it -- once per path, ahead of every draw on that path -- so it uses the
                        // same machinery. Ordered before addFixCall_ so that NNFix, placed by the
                        // same rule, ends up between these registers and the draw.
                        auto edit = std::make_unique<RegDelimitedAdd<>>(
                            RegDelimitedAdd<>::Additions{{DiffuseReg, files->diffuseRes},
                                                          {LightMapReg, files->lightMapRes}},
                            RegDelimitedAdd<>::RegMap{{IniKeywords::DrawIndexed, {}}},
                            /*pathEndOnlyWhenUndelimited*/ true,
                            RegDelimitedAddMode::PerPath);
                        auto adapter = std::make_unique<GraphPartEdit<>>(edit.get());

                        iniEdits[0].edits[ModObj("", it->first)] = {adapter.get()};
                        iniEdits[0].trackKeys[ModObj("", it->first)] = false;

                        borrowAdapters_.push_back(std::move(adapter));
                        borrowRegEdits_.push_back(std::move(edit));
                        any = true;
                    }

                    if (any) {
                        borrowEdit_ = std::make_unique<ObjGroupEdit>(std::move(iniEdits), false);
                    }
                }

                // ---- 3. the light map bands, at the register the SOURCE holds them in ----
                void buildTexEdits() {
                    if (!config_.lightMapEdit) {
                        return;
                    }

                    for (const std::string& obj : drawn_) {
                        auto it = representative_.find(obj);
                        if (it == representative_.end()) {
                            continue;
                        }

                        const SlotFiles* files = slotFiles(it->second.first, it->second.second);
                        if (files == nullptr || files->lightMapRes.empty()) {
                            continue;
                        }

                        TexEditor::Filter filter = config_.lightMapEdit(files->diffuse);
                        if (!filter) {
                            continue;
                        }

                        const bool normalMap = hasNormalMap(it->second.first, it->second.second);
                        const std::string reg = normalMap ? NormalShiftedLightMapReg : LightMapReg;

                        auto replace = std::make_unique<TexEditorReplace<>>(
                            GraphId(0, "", obj + "RemapTexLightMap"),
                            TexEditor({filter}, config_.compressTextures, config_.mipmaps), makeResEditConfig(),
                            "resourceRemapTexEdit", std::string("LightMap"));

                        auto collect = std::make_unique<Collector>();
                        collect->srcRegs = {{GraphId(0, "", obj), reg}};
                        collect->resEdits = {{"lightMap", replace.get()}};

                        texGroupEdits_.push_back(collect.get());
                        texReplaces_.push_back(std::move(replace));
                        texCollects_.push_back(std::move(collect));
                    }

                    buildMemberTexEdits();
                }

                // ---- 3b. and the band edit for a member that brought its OWN light map ----
                //
                // The loop above edits one light map per target object, found through that object's
                // graph and its register. A merged object can carry a second: a component the mod
                // does not have is downloaded whole and uses the GAME's textures, which are still in
                // the SOURCE skin's band space and need the same remap as everything else. Without
                // it the eye whites keep Tranquil's band 0, which on Yelan is her hair.
                //
                // Collected from the MEMBER's own graph rather than the target's, because a target
                // graph holds one register per name and the merged section now binds `ps-t1` twice.
                // That graph only exists before slotRemap_ folds the members into the target, which
                // is why these edits are applied ahead of it -- see the graphGroupEdits order.
                void buildMemberTexEdits() {
                    for (const std::string& obj : drawn_) {
                        auto membersIt = members_.find(obj);
                        auto repIt = representative_.find(obj);
                        if (membersIt == members_.end() || repIt == representative_.end()) {
                            continue;
                        }

                        const SlotFiles* repFiles = slotFiles(repIt->second.first, repIt->second.second);

                        for (const auto& member : membersIt->second) {
                            if (member == repIt->second) {
                                continue;
                            }

                            const SlotFiles* files = slotFiles(member.first, member.second);
                            if (files == nullptr || files->lightMapRes.empty() || repFiles == nullptr
                                    || files->lightMapRes == repFiles->lightMapRes) {
                                continue;
                            }

                            TexEditor::Filter filter = config_.lightMapEdit(files->diffuse);
                            if (!filter) {
                                continue;
                            }

                            const bool normalMap = hasNormalMap(member.first, member.second);
                            const std::string reg = normalMap ? NormalShiftedLightMapReg : LightMapReg;

                            auto replace = std::make_unique<TexEditorReplace<>>(
                                GraphId(0, member.first, member.second + "RemapTexLightMap"),
                                TexEditor({filter}, config_.compressTextures, config_.mipmaps), makeResEditConfig(),
                                "resourceRemapTexEdit", std::string("LightMap"));

                            auto collect = std::make_unique<Collector>();
                            collect->srcRegs = {{GraphId(0, member.first, member.second), reg}};
                            collect->resEdits = {{"lightMap", replace.get()}};

                            preRemapTexGroupEdits_.push_back(collect.get());
                            texReplaces_.push_back(std::move(replace));
                            texCollects_.push_back(std::move(collect));
                        }
                    }
                }

                RegPartEdit<>* assetAdapterOf(const std::string& component) {
                    auto it = assetAdapters_.find(component);
                    return it == assetAdapters_.end() ? nullptr : it->second.get();
                }

                bool hasNormalMap(const std::string& component, const std::string& slot) const {
                    auto it = normalMap_.find(key(component, slot));
                    return it != normalMap_.end() && it->second;
                }

                // The group's or the part's query, in OUR context.
                //
                // combineQueries does a full render / re-parse round trip whenever its two sides
                // belong to different Z3Contexts, and this pair always does -- the query comes from
                // a graph the library built, while every candidate was read through valsThroughRun
                // into z3Ctx_. Reparented once here rather than once per candidate.
                std::optional<Z3Predicate> localQuery(const Z3Predicate* query) {
                    if (query == nullptr) {
                        return std::nullopt;
                    }

                    return GroupCollector::combineQueries(*query, Z3Predicate::trueValue(z3Ctx_), &z3Ctx_);
                }

                // WHICH branch a query belongs to, or -1 for "cannot say".
                //
                // Exactly one candidate satisfiable with it is the whole test: a part inside
                // `$swapvar == 3` rules out every other branch, while a section's unconditional
                // preamble is satisfiable with all of them and is correctly declined. A source that
                // does not branch has one candidate and every part belongs to it.
                long long branchIndexOf(const std::vector<BranchVal>& branches,
                                         const std::optional<Z3Predicate>& query) {
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

                        if (GroupCollector::combineQueries(*query, *branches[i].query, &z3Ctx_).isSatisfiable()) {
                            if (found >= 0) {
                                return -1;
                            }

                            found = static_cast<long long>(i);
                        }
                    }

                    return found;
                }

                // Which of a register's values a group takes.
                //
                // SATISFIABILITY, not position: a candidate belongs to this group when its own
                // condition can hold at the same time as the group's. That is indifferent to how the
                // conditions are shaped -- an if / else if chain, independent toggles, nested
                // predicates, a component that does not branch at all -- and pairing by index is not.
                //
                // The first satisfiable candidate wins. Several can be satisfiable at once, when the
                // group's own state simply does not constrain this register: a component with a
                // toggle of its own that the target's buffers say nothing about. Any of them is then
                // a correct reading of that state, and taking the first keeps a non-branching
                // component on its only value.
                //
                // Falling back to 'fallback' covers the one case with no candidate to test: a
                // component the mod does not carry, whose buffers are downloads that are not on disk
                // yet and appear in no section.
                std::string pick(const std::vector<BranchVal>& candidates, const std::string& fallback,
                                  const std::optional<Z3Predicate>& query) {
                    if (candidates.empty()) {
                        return fallback;
                    }

                    if (candidates.size() == 1 || !query.has_value()) {
                        return candidates.front().val;
                    }

                    for (const BranchVal& candidate : candidates) {
                        if (candidate.query.has_value()
                                && GroupCollector::combineQueries(*query, *candidate.query, &z3Ctx_).isSatisfiable()) {
                            return candidate.val;
                        }
                    }

                    return candidates.front().val;
                }

                // The merge for ONE of the collect's groups.
                //
                // A group is one satisfiable state of the mod -- one variant of a merged master, one
                // combination of a hand-made mod's toggles -- and 'query' is the condition its own
                // resources co-occur under (see GroupedResBuilder::beginGroup). Everything structural
                // is the same in every group; only which buffer each component contributes differs,
                // and that is what pick answers.
                //
                // 'query' is nullptr for a group with no condition at all, which is every ordinary
                // unmerged mod: then every component keeps its only value and this is the same
                // single config the merge has always built.
                VGMergeGroupConfig configForGroup(const Z3Predicate* query) {
                    // Reparented ONCE per group -- see localQuery.
                    const std::optional<Z3Predicate> local = localQuery(query);

                    VGMergeGroupConfig config;

                    for (const std::string& component : mergeOrder_) {
                        const ComponentFiles* files = componentFiles(component);
                        auto remapIt = remaps_.find(component);
                        if (files == nullptr || remapIt == remaps_.end()) {
                            return VGMergeGroupConfig{};
                        }

                        VGMergeComponentFiles entry;
                        entry.spec.name = component;
                        entry.spec.remap = remapIt->second;
                        entry.blendPath = pick(files->blends, files->blend, local);
                        entry.positionPath = pick(files->positions, files->position, local);
                        entry.texcoordPath = pick(files->texcoords, files->texcoord, local);
                        config.components.push_back(std::move(entry));
                    }

                    for (const std::string& obj : drawn_) {
                        auto membersIt = members_.find(obj);
                        auto repIt = representative_.find(obj);
                        if (membersIt == members_.end() || repIt == representative_.end()) {
                            continue;
                        }

                        VGMergeObject object;
                        const SlotFiles* repFiles = slotFiles(repIt->second.first, repIt->second.second);
                        object.srcPath = (repFiles == nullptr) ? "" : pick(repFiles->ibs, repFiles->ib, local);

                        // Nothing to merge INTO: the object this group draws through is nulled here,
                        // so the collect found no reference to it either and there is no member
                        // waiting to be matched against this object.
                        if (object.srcPath.empty()) {
                            continue;
                        }

                        for (const auto& member : membersIt->second) {
                            const SlotFiles* files = slotFiles(member.first, member.second);
                            if (files == nullptr) {
                                continue;
                            }

                            // An empty pick is a nulled branch: this slot draws nothing in this
                            // group, so it contributes no geometry to the merge here. The vertex
                            // OFFSETS are unaffected -- they come from the components' vertex
                            // counts, which do not change with the variant.
                            std::string path = pick(files->ibs, files->ib, local);
                            if (path.empty()) {
                                continue;
                            }

                            object.members.emplace_back(member.first, std::move(path));
                        }

                        config.objects.push_back(std::move(object));
                    }

                    return config;
                }

                // ---- 4. the buffers, as ONE merged resource group ----
                void buildBufferCollect() {
                    const ModType* modType = ctx_.modType();
                    IniFile* iniFile = ctx_.getIniFile();
                    if (modType == nullptr || modType->vgRemaps == nullptr) {
                        return;
                    }

                    const std::string srcName = ctx_.modTypeName().value_or("");
                    const std::optional<Version> fromVersion = ctx_.version();
                    const std::optional<Version> toVersion = (iniFile == nullptr) ? std::nullopt : iniFile->toVersion;

                    // Each component has a row of its OWN -- that is the whole reason they cannot
                    // be remapped together. Looked up once: the row does not vary with the variant,
                    // and configForGroup is called per group.
                    remaps_.clear();
                    for (const std::string& component : mergeOrder_) {
                        if (componentFiles(component) == nullptr) {
                            return;
                        }

                        std::optional<VGRemap> remap = modType->vgRemaps->get(
                            {srcName, component, toModName_, std::string("")}, {fromVersion, toVersion}, false);
                        if (!remap.has_value()) {
                            return;
                        }

                        remaps_.emplace(component, *remap);
                    }

                    // Only an index buffer the merge actually MOVES needs replacing, and whether it
                    // moves is a property of the merge's shape rather than of any one variant: a
                    // target object drawn by one slot of the component at offset 0 already addresses
                    // the right vertices in every variant.
                    std::vector<std::string> changedIbs;
                    for (const std::string& obj : drawn_) {
                        auto membersIt = members_.find(obj);
                        auto repIt = representative_.find(obj);
                        if (membersIt == members_.end() || repIt == representative_.end()) {
                            continue;
                        }

                        bool changed = membersIt->second.size() > 1;
                        for (const auto& member : membersIt->second) {
                            if (slotFiles(member.first, member.second) != nullptr && offsets_[member.first] != 0) {
                                changed = true;
                            }
                        }

                        if (changed) {
                            changedIbs.push_back(obj);
                        }
                    }

                    builder_ = std::make_unique<VGMergeGroupResBuilder>(
                        srcName + toModName_ + "Buffers",
                        [this](const Z3Predicate* query) { return configForGroup(query); }, ctx_.getIniFile());

                    GroupCollector::ByGraph<GroupCollector::ByGraph<std::string>> srcRegs;
                    GroupCollector::ByGraph<tsl::ordered_map<std::string, GroupCollector::ResEdit*>> resEdits;

                    const std::vector<std::pair<std::string, std::pair<GraphId, std::string>>> kinds = {
                        {"blend", {GraphId(0, "", "blend"), IniKeywords::Vb1}},
                        {"position", {GraphId(0, "", "position"), IniKeywords::Vb0}},
                        {"texcoord", {GraphId(0, "", "texcoord"), IniKeywords::Vb1}}};

                    for (const auto& kind : kinds) {
                        std::string element = kind.first;
                        element[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(element[0])));
                        const GraphId resObj(0, "", "Merged" + element);

                        auto replace = std::make_unique<BufReplace<>>(resObj, makeResEditConfig(), kind.first, std::nullopt);
                        srcRegs[resObj] = {{kind.second.first, kind.second.second}};
                        resEdits[resObj] = {{MergeGroupType, replace.get()}};
                        bufReplaces_.push_back(std::move(replace));
                    }

                    // See where changedIbs is filled.
                    for (const std::string& obj : changedIbs) {
                        const GraphId resObj(0, "", obj + "MergedIb");
                        auto replace = std::make_unique<BufReplace<>>(resObj, makeResEditConfig(), "ib",
                                                                       std::optional<std::string>(obj));
                        srcRegs[resObj] = {{GraphId(0, "", obj), IniKeywords::Ib}};
                        resEdits[resObj] = {{MergeGroupType, replace.get()}};
                        bufReplaces_.push_back(std::move(replace));
                    }

                    bufferCollect_ = std::make_unique<GroupCollector>(
                        std::vector<std::string>{MergeGroupType}, std::move(srcRegs), std::move(resEdits),
                        tsl::ordered_map<std::string, GroupCollector::GroupedResBuilder*>{{MergeGroupType, builder_.get()}},
                        [](const std::string& sectionName) { return sectionName; }, 0);
                }

                // ---- 5. the target's index, windowed to the copied object's own KVPs ----
                void buildIndexEdits() {
                    IniFile* iniFile = ctx_.getIniFile();
                    Indices* indices = ctx_.modTypeIndices();
                    const std::optional<Version> toVersion = (iniFile == nullptr) ? std::nullopt : iniFile->toVersion;

                    objFilter_ = std::make_unique<ObjFilter>(ctx_.modTypeHashes(), ctx_.modTypeIndices(),
                                                              ObjFilter::KeySet{IbHashKey}, ctx_.version());

                    std::vector<ObjGroupEdit::IniEdits> iniEdits(1);

                    for (const std::string& obj : drawn_) {
                        auto repIt = representative_.find(obj);
                        if (repIt == representative_.end() || indices == nullptr) {
                            continue;
                        }

                        // The TARGET's own rows, which a classic character does have -- the
                        // IndexData note that keeps a skin's slots out of the table is about the
                        // source side, not this one.
                        std::optional<std::string> index = indices->get({toModName_, "", obj}, toVersion, false);
                        if (!index.has_value()) {
                            continue;
                        }

                        auto edit = std::make_unique<RegNewVals<>>(
                            std::vector<std::pair<std::string, RegNewVals<>::NewValSpec>>{
                                {IniKeywords::MatchFirstIndex, RegNewVals<>::NewValSpec(RegNewVals<>::NewVal(*index))}});
                        auto adapter = std::make_unique<RegPartEdit<>>(edit.get());

                        // NOT windowed by GIMIObjPartFilter, unlike the split's. That filter exists
                        // because several of the split's objects share ONE slot graph and an
                        // unwindowed write would set them all to one index. Here every target object
                        // has a graph of its own, so there is nothing to tell apart -- and windowing
                        // would in fact write nothing, since the filter identifies a mod object by
                        // its Indices row and the source's slots deliberately have none.
                        const ModObj objKey("", obj);
                        iniEdits[0].edits[objKey] = {adapter.get()};
                        iniEdits[0].trackKeys[objKey] = false;

                        indexAdapters_.push_back(std::move(adapter));
                        indexRegEdits_.push_back(std::move(edit));
                    }

                    indexEdits_ = ObjGroupEdit(std::move(iniEdits), false);
                }

                // ---- the appended member draws, one block per BRANCH ----
                //
                // The same shape as the unbranched case below, asked once per branch: how many
                // indices each member contributes there, and therefore where the next one starts.
                // Every number comes from the branch's own files, which is the whole point.
                bool buildBranchDraws(const std::string& obj) {
                    auto membersIt = members_.find(obj);
                    auto repIt = representative_.find(obj);
                    if (membersIt == members_.end() || repIt == representative_.end()) {
                        return false;
                    }

                    const SlotFiles* repFiles = slotFiles(repIt->second.first, repIt->second.second);
                    if (repFiles == nullptr) {
                        return false;
                    }

                    const std::vector<std::pair<std::string, std::string>> members = membersIt->second;
                    const std::vector<BranchVal> branches = repFiles->ibs;

                    auto branchAdd = std::make_unique<RegBranchAdd<>>(
                        [this, obj, members, branches, repFiles](const Z3Predicate& query,
                                                                  const RegBranchAdd<>::IterData&) {
                            RegBranchAdd<>::Branch result;

                            const std::optional<Z3Predicate> local = localQuery(&query);
                            const long long branch = branchIndexOf(branches, local);
                            if (branch < 0) {
                                return result;
                            }

                            // Does the mod draw this object itself in THIS branch? Four of one
                            // master's twelve leave it to the whole-ib override, which the remap
                            // takes away -- so the first member needs a draw of its own there, and
                            // needs none where the mod already issues its ranges.
                            bool branchDraws = false;
                            for (const BranchVal& drawVal : repFiles->drawVals) {
                                if (!drawVal.query.has_value() || !local.has_value()) {
                                    branchDraws = true;
                                    break;
                                }

                                if (GroupCollector::combineQueries(*local, *drawVal.query, &z3Ctx_).isSatisfiable()) {
                                    branchDraws = true;
                                    break;
                                }
                            }

                            RegBranchAdd<>::Additions additions;
                            long long offset = 0;

                            for (std::size_t i = 0; i < members.size(); ++i) {
                                const SlotFiles* files = slotFiles(members[i].first, members[i].second);
                                if (files == nullptr) {
                                    continue;
                                }

                                const std::string path = pick(files->ibs, files->ib, local);
                                if (path.empty()) {
                                    // `ib = null`: this member draws nothing in this branch, and
                                    // contributes nothing to the offsets either.
                                    continue;
                                }

                                const long long count = static_cast<long long>(fileSize(path) / IbIndexStride);
                                if (count <= 0) {
                                    // Guessing a count would address whatever happens to sit at that
                                    // offset -- and every member after it too, so the whole branch
                                    // is left to the mod rather than half drawn.
                                    return RegBranchAdd<>::Branch{};
                                }

                                if (i > 0) {
                                    appendMemberBindings(additions, members[i], repFiles);
                                }

                                if (i > 0 || !branchDraws) {
                                    additions.emplace_back(IniKeywords::DrawIndexed,
                                                            std::to_string(count) + ", " + std::to_string(offset) + ", 0");
                                }

                                offset += count;
                            }

                            if (additions.empty()) {
                                return result;
                            }

                            result.key = obj + ";" + std::to_string(branch);
                            result.additions = std::move(additions);
                            return result;
                        });

                    extraDrawAdapters_[obj] = std::make_unique<GraphPartEdit<>>(branchAdd.get());
                    branchDraws_.push_back(std::move(branchAdd));
                    return true;
                }

                // A member that reads its own textures binds them ahead of its draw, and then needs
                // its own fix call -- rebinding ps-t0/ps-t1 starts a new binding generation and
                // NNFix re-slots whatever is bound when it runs. Shared with the unbranched path so
                // the two cannot drift.
                void appendMemberBindings(std::vector<std::pair<std::string, std::string>>& additions,
                                           const std::pair<std::string, std::string>& member, const SlotFiles* repFiles) {
                    const SlotFiles* files = slotFiles(member.first, member.second);
                    const bool ownTextures = (files != nullptr) && (repFiles != nullptr)
                                              && (!files->diffuseRes.empty() || !files->lightMapRes.empty())
                                              && (files->diffuseRes != repFiles->diffuseRes
                                                   || files->lightMapRes != repFiles->lightMapRes);
                    if (!ownTextures) {
                        return;
                    }

                    if (!files->diffuseRes.empty()) {
                        additions.emplace_back(DiffuseReg, files->diffuseRes);
                    }

                    if (!files->lightMapRes.empty()) {
                        // The name buildMemberTexEdits' own edit will produce, worked out with the
                        // very function that produces it rather than by copying the convention.
                        const std::string edited =
                            config_.lightMapEdit
                                ? IniNamingTools::getRemapTexResourceName(
                                      files->lightMapRes, TextTools::capitalize(toModName_) + "LightMap")
                                : files->lightMapRes;

                        additions.emplace_back(LightMapReg, edited);
                    }

                    additions.emplace_back(IniKeywords::Run, IniKeywords::NNFixPath);
                }

                // How many vertices the merged buffer holds in ONE branch.
                //
                // Every component contributes its own, and a merged master's components do not all
                // branch -- the Bang and the Eye have one buffer for every variant while the Body
                // has twelve. pick answers each of them for this branch, so the sum is this branch's.
                std::size_t mergedVertexCount(const std::optional<Z3Predicate>& query) {
                    std::size_t total = 0;
                    for (const std::string& component : mergeOrder_) {
                        const ComponentFiles* files = componentFiles(component);
                        if (files == nullptr) {
                            continue;
                        }

                        const std::string path = pick(files->blends, files->blend, query);
                        const std::size_t count = fileSize(path) / BlendStride;
                        total += (count > 0) ? count : files->vertexCount;
                    }

                    return total;
                }

                // ---- the drawn vertex count, per BRANCH ----
                //
                // The blend override re-issues the vertex pass, and `draw` says how many vertices it
                // covers. One number for a branching source draws the first variant's worth of every
                // variant: correct for that one, and for a bigger one it stops part way through the
                // model -- the legs, the back of the hair and the eyes were simply past the end.
                bool buildBranchVertexCounts() {
                    const ComponentFiles* skeleton = componentFiles(mergeOrder_.front());
                    if (skeleton == nullptr || skeleton->blends.size() <= 1) {
                        return false;
                    }

                    const std::vector<BranchVal> branches = skeleton->blends;

                    auto branchAdd = std::make_unique<RegBranchAdd<>>(
                        [this, branches](const Z3Predicate& query, const RegBranchAdd<>::IterData&) {
                            RegBranchAdd<>::Branch result;

                            const std::optional<Z3Predicate> local = localQuery(&query);
                            const long long branch = branchIndexOf(branches, local);
                            if (branch < 0) {
                                return result;
                            }

                            const std::size_t vertices = mergedVertexCount(local);
                            if (vertices == 0) {
                                return result;
                            }

                            // REPLACED, not added: the branch already carries a `draw` of its own,
                            // and a second one draws the model twice rather than correcting the first.
                            result.key = "blend;" + std::to_string(branch);
                            result.replacements = {{IniKeywords::Draw, std::to_string(vertices) + ",0"}};
                            return result;
                        });

                    blendBranchDraw_ = std::move(branchAdd);
                    blendBranchDrawAdapter_ = std::make_unique<GraphPartEdit<>>(blendBranchDraw_.get());
                    return true;
                }

                // ---- 6. everything else ----
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

                    // ONE hash remap PER COMPONENT, because RegAssetRemap is reverse-then-forward
                    // and the reverse half is filtered by the source's NAME -- and a multi-component
                    // skin files each component's hashes under that COMPONENT's mod type name, not
                    // under its own. Filtered to the skin's name, every lookup misses and every
                    // section comes out `hash = HashNotFound`, which is what the first compiled run
                    // did while its BUFFERS were already byte-identical to the prototype's.
                    for (const std::string& component : mergeOrder_) {
                        const std::string fromName = componentModTypeName(component);
                        assetRemaps_[component] = std::make_unique<RegAssetRemap<>>(
                            std::vector<std::pair<std::string, RegAssetRemap<>::AssetSpec>>{
                                {IniKeywords::Hash, RegAssetRemap<>::AssetSpec(ctx_.modTypeHashes(), IniKeywords::HashNotFound)}},
                            toModName_, fromName, ctx_.version(), toVersion);
                        assetAdapters_[component] = std::make_unique<RegPartEdit<>>(assetRemaps_[component].get());
                    }

                    // The face's own, lenient remap: every component files the skin's face diffuse
                    // under its own name with the same value, so the skeleton's serves.
                    faceAssetRemap_ = std::make_unique<RegAssetRemap<>>(
                        std::vector<std::pair<std::string, RegAssetRemap<>::AssetSpec>>{
                            {IniKeywords::Hash, RegAssetRemap<>::AssetSpec(ctx_.modTypeHashes())}},
                        toModName_, componentModTypeName(mergeOrder_.front()), ctx_.version(), toVersion);

                    // The face diffuse onto the register the TARGET binds it to. A no-op when the
                    // mod already agrees; the mods that do not are the pre-6.x ones still writing
                    // ps-t0, which on a 6.x target replaces the face LIGHT MAP.
                    if (config_.faceReg == LightMapReg) {
                        faceRegFix_ = std::make_unique<RegRemap<>>(std::vector<std::pair<std::string, RegRemap<>::KeyRemapValue>>{
                            renameRule(DiffuseReg, {LightMapReg})});
                    } else if (config_.faceReg == DiffuseReg) {
                        faceRegFix_ = std::make_unique<RegRemap<>>(std::vector<std::pair<std::string, RegRemap<>::KeyRemapValue>>{
                            renameRule(LightMapReg, {DiffuseReg})});
                    }

                    auto isFixCall = [](long long, const std::string& value) {
                        return value == IniKeywords::ORFixPath || value == IniKeywords::NNFixPath;
                    };
                    removeFixCalls_ = std::make_unique<RegRemove<>>(
                        std::vector<std::pair<std::string, std::optional<RegRemove<>::RemoveKeyCheck>>>{
                            {IniKeywords::Run, RegRemove<>::RemoveKeyCheck(isFixCall)}});

                    // The target has no normal-map slot: drop it and shift the rest down.
                    dropNormalMap_ = std::make_unique<RegRemove<>>(
                        std::vector<std::pair<std::string, std::optional<RegRemove<>::RemoveKeyCheck>>>{
                            {DiffuseReg, std::nullopt}});
                    shiftDown_ = std::make_unique<RegRemap<>>(std::vector<std::pair<std::string, RegRemap<>::KeyRemapValue>>{
                        renameRule(NormalShiftedDiffuseReg, {DiffuseReg}),
                        renameRule(NormalShiftedLightMapReg, {LightMapReg})});

                    removeDrawIndexed_ = std::make_unique<RegRemove<>>(
                        std::vector<std::pair<std::string, std::optional<RegRemove<>::RemoveKeyCheck>>>{
                            {IniKeywords::DrawIndexed, std::nullopt}});

                    // BottomCover: the collects spliced their registers into `if 1 ... endif` blocks,
                    // which split the section into parts, and the default fill would put the draw in
                    // the FIRST part, ahead of the ib and the textures.
                    fillDrawIndexed_ = std::make_unique<RegFillMissing<>>(
                        IniKeywords::DrawIndexed,
                        RegFillMissing<>::makeFillMissing(IniKeywords::DrawIndexed, DrawIndexedAuto),
                        RegFillMissingMode::BottomCover);

                    // NNFix and ORFix are mandatory and keyed on the draw call instead.
                    //
                    // PerPath, not the per-segment default: these command lists READ the bound
                    // ps-t registers and write them back re-slotted, so a second call over the same
                    // bindings undoes the first (NNFix reads the diffuse from ps-t0 and the light
                    // map from ps-t1, then writes the light map to ps-t0 and the diffuse to ps-t1).
                    // A section whose draws sit in independent `if` blocks issues several in one
                    // pass, and every second one then rendered with the light map as its albedo --
                    // flat green. See RegDelimitedAddMode::PerPath, and note that this mode makes
                    // pathEndOnlyWhenUndelimited redundant: "once at the end of a path that never
                    // draws" is what it already does.
                    // ---- a target object SEVERAL source slots land on needs the later ones DRAWN ----
                    //
                    // The merge concatenates its members' index buffers, so the first member keeps
                    // its own index range and every later one is pushed past it. A mod's
                    // `drawindexed` lines address ITS OWN buffer, which is the first member's -- so
                    // they cover the first member exactly and never reach the rest. Yelan's head is
                    // her Bang followed by her Eye, and a mod that issues its own draws rendered the
                    // fringe and no eyes at all (2026-09-14).
                    //
                    // TWO THINGS HAVE TO BE TRUE AND THE SECOND IS THE ONE THAT BIT.
                    //
                    // (1) It must happen only when the mod DREW FOR ITSELF. A section the fix leaves
                    //     with `drawindexed = auto` already draws the whole merged buffer, members
                    //     and all, and a second draw of the later members would be a duplicate. The
                    //     gate is read off the source section rather than expressed as an edit --
                    //     SlotFiles::draws -- because it is a fact about the mod, known here, and
                    //     every way of asking the graph instead has to run either before the fill
                    //     (and so cannot see it) or after (and so cannot tell `auto` from a real
                    //     draw).
                    //
                    // (2) It must land where EVERY path reaches it. The obvious edit, a
                    //     RegSurroundedAdd keyed on `drawindexed` with latest = true, puts it at the
                    //     latest valid position -- which in a section full of toggles is inside the
                    //     LAST `if` block. Yelan then had eyes only while `$pubic == 1`, which is
                    //     the same bug one layer down and is invisible to any check that strips
                    //     leading whitespace before looking. RegFillMissingMode::BottomCover is the
                    //     placement that is right: `addBottomContentPart` appends a fresh part at
                    //     the section's own depth, outside every block.
                    //
                    //     RegBottomAdd is that placement without RegFillMissing's missing-register
                    //     gate, which is a different question and answers "do nothing" for a
                    //     register the mod already has.
                    //
                    // The appended draw needs no fix call of its own: RegDelimitedAddMode::PerPath
                    // issues exactly one for the whole path, ahead of every draw on it.
                    for (const std::string& obj : drawn_) {
                        auto membersIt = members_.find(obj);
                        if (membersIt == members_.end() || membersIt->second.size() < 2) {
                            continue;
                        }

                        auto repIt = representative_.find(obj);
                        const SlotFiles* repFiles = (repIt == representative_.end())
                                                     ? nullptr : slotFiles(repIt->second.first, repIt->second.second);
                        if (repFiles == nullptr) {
                            continue;
                        }

                        // Appended draws are needed when the mod draws for itself (its ranges reach
                        // only the first member), and ALSO when the members end up on different
                        // textures -- one draw binds one set, so members that disagree have to be
                        // drawn separately whatever the mod did. Only when neither holds can
                        // `drawindexed = auto` cover the whole merged buffer on its own.
                        if (!repFiles->draws && !membersDiffer(obj)) {
                            continue;
                        }

                        // A SOURCE WHOSE BRANCHES ARE DIFFERENT MODELS NEEDS A BLOCK PER BRANCH.
                        //
                        // The counts and offsets come from the index buffers being drawn, and a
                        // merged master's branches have a set each. One block at the section's own
                        // depth carries one set: right for one branch, and in the others it draws a
                        // slice of the FIRST member's geometry again under the SECOND member's
                        // textures. So when the representative's ib branches, the draws go inside
                        // the branches instead -- see RegBranchAdd.
                        //
                        // It also keeps the fix call landing correctly. A draw added to the section
                        // makes a part that both `run =`s and draws, and RegDelimitedAdd treats a
                        // part as atomic: it takes its call before that part's own draw and counts
                        // every path covered, leaving the draws inside the callee without one.
                        if (repFiles->ibs.size() > 1 && buildBranchDraws(obj)) {
                            continue;
                        }

                        std::vector<std::string> extras;
                        long long offset = 0;
                        bool measured = true;

                        for (std::size_t i = 0; i < membersIt->second.size(); ++i) {
                            const auto& member = membersIt->second[i];
                            const SlotFiles* files = slotFiles(member.first, member.second);
                            const long long count = (files == nullptr) ? 0 : files->indexCount;

                            if (count <= 0) {
                                // Nothing to go on -- neither the file nor the config. Emitting a
                                // draw from a guessed count would address whatever happens to sit
                                // at that offset, so the member is left undrawn and said so.
                                ctx_.log("could not size the '" + member.first + " " + member.second
                                          + "' index buffer, so the '" + obj
                                          + "' object will draw without it");
                                measured = false;
                                break;
                            }

                            if (i > 0) {
                                extras.push_back(std::to_string(count) + ", " + std::to_string(offset) + ", 0");
                            }

                            offset += count;
                        }

                        if (!measured || extras.empty()) {
                            continue;
                        }

                        // A MEMBER MAY NEED ITS OWN TEXTURES, AND THEN ITS OWN FIX CALL.
                        //
                        // One section binds one set of registers, and that set is the FIRST
                        // member's -- right while every member reads the same textures. A component
                        // A member that binds no textures of its own breaks it: the game draws such
                        // a slot with the game's own atlas, so the parser downloads that donor
                        // (GIMIComponentParserConfig::Slot::textureDonor) and the member ends up on
                        // a different texture from the representative. Binding the representative's
                        // for both aimed one mod's eye UVs at a repainted 4096x2048 atlas and
                        // another mod's hair at a bunny costume (2026-09-14).
                        //
                        // So a member whose textures differ from the representative's gets its own
                        // bindings ahead of its draw -- and its own fix call after them, because
                        // rebinding ps-t0/ps-t1 starts a new binding epoch and NNFix re-slots
                        // whatever is bound when it runs. This is the only place anything rebinds
                        // mid-section, which is exactly why RegDelimitedAddMode::PerPath must not be
                        // what places that second call: this block carries its own.
                        RegBottomAdd<>::Additions block;

                        for (std::size_t i = 0; i < extras.size(); ++i) {
                            appendMemberBindings(block, membersIt->second[i + 1], repFiles);
                            block.emplace_back(IniKeywords::DrawIndexed, extras[i]);
                        }

                        auto bottomAdd = std::make_unique<RegBottomAdd<>>(std::move(block));
                        extraDrawAdapters_[obj] = std::make_unique<GraphPartEdit<>>(bottomAdd.get());
                        extraDraws_.push_back(std::move(bottomAdd));
                    }

                    // An object whose members cannot share a draw needs the FIRST member's range
                    // rather than `auto`, for the section that draws nothing of its own.
                    for (const std::string& obj : drawn_) {
                        auto membersIt = members_.find(obj);
                        auto repIt = representative_.find(obj);
                        if (membersIt == members_.end() || repIt == representative_.end()
                                || membersIt->second.size() < 2 || !membersDiffer(obj)) {
                            continue;
                        }

                        // A branching object supplies its own first-member draw, per branch,
                        // inside the branch -- one range at the section's depth would be right for
                        // one branch and wrong for the rest. See buildBranchDraws.
                        const SlotFiles* repSlot = slotFiles(repIt->second.first, repIt->second.second);
                        if (repSlot != nullptr && repSlot->ibs.size() > 1) {
                            continue;
                        }

                        const SlotFiles* first = slotFiles(membersIt->second.front().first,
                                                            membersIt->second.front().second);
                        if (first == nullptr || first->indexCount <= 0) {
                            continue;
                        }

                        const std::string range = std::to_string(first->indexCount) + ", 0, 0";
                        auto fill = std::make_unique<RegFillMissing<>>(
                            IniKeywords::DrawIndexed,
                            RegFillMissing<>::makeFillMissing(IniKeywords::DrawIndexed, range),
                            RegFillMissingMode::BottomCover);

                        objFillAdapters_[obj] = std::make_unique<GraphPartEdit<>>(fill.get());
                        objFills_.push_back(std::move(fill));
                    }

                    addFixCall_ = std::make_unique<RegDelimitedAdd<>>(
                        RegDelimitedAdd<>::Additions{{IniKeywords::Run, IniKeywords::NNFixPath}},
                        RegDelimitedAdd<>::RegMap{{IniKeywords::DrawIndexed, {}}},
                        /*pathEndOnlyWhenUndelimited*/ true,
                        RegDelimitedAddMode::PerPath);

                    // THE LARGEST BRANCH, not the first. This one number covers the whole
                    // `.ini` -- the section it goes in carries no conditions to vary it by -- and it
                    // has to be big enough for whichever variant the player picks. Equal to
                    // totalVertices_ for a source that does not branch.
                    std::size_t maxVertices = totalVertices_;
                    const ComponentFiles* skeletonFiles = componentFiles(mergeOrder_.front());
                    if (skeletonFiles != nullptr) {
                        for (const BranchVal& branch : skeletonFiles->blends) {
                            maxVertices = std::max(maxVertices, mergedVertexCount(branch.query));
                        }
                    }

                    overrides_ = std::make_unique<RegNewVals<>>(
                        std::vector<std::pair<std::string, RegNewVals<>::NewValSpec>>{
                            {OverrideByteStride, RegNewVals<>::NewValSpec(RegNewVals<>::NewVal(std::to_string(positionStride_)))},
                            {OverrideVertexCount, RegNewVals<>::NewValSpec(RegNewVals<>::NewVal(std::to_string(maxVertices)))}},
                        /*addNewKVPs*/ true);

                    blendDraw_ = std::make_unique<RegNewVals<>>(
                        std::vector<std::pair<std::string, RegNewVals<>::NewValSpec>>{
                            {IniKeywords::Draw, RegNewVals<>::NewValSpec(RegNewVals<>::NewVal(std::to_string(totalVertices_) + ",0"))}});

                    renameAdapter_ = std::make_unique<GraphPartEdit<>>(renameGraph_.get());
                    renameIbAdapter_ = std::make_unique<GraphPartEdit<>>(renameIbGraph_.get());
                    renameBlendAdapter_ = std::make_unique<GraphPartEdit<>>(renameBlendGraph_.get());
                    faceAssetAdapter_ = std::make_unique<RegPartEdit<>>(faceAssetRemap_.get());
                    if (faceRegFix_ != nullptr) {
                        faceRegAdapter_ = std::make_unique<RegPartEdit<>>(faceRegFix_.get());
                    }
                    removeFixCallsAdapter_ = std::make_unique<RegPartEdit<>>(removeFixCalls_.get());
                    dropNormalMapAdapter_ = std::make_unique<RegPartEdit<>>(dropNormalMap_.get());
                    shiftDownAdapter_ = std::make_unique<RegPartEdit<>>(shiftDown_.get());
                    removeDrawIndexedAdapter_ = std::make_unique<RegPartEdit<>>(removeDrawIndexed_.get());
                    fillAdapter_ = std::make_unique<GraphPartEdit<>>(fillDrawIndexed_.get());
                    addFixCallAdapter_ = std::make_unique<GraphPartEdit<>>(addFixCall_.get());
                    overridesAdapter_ = std::make_unique<RegPartEdit<>>(overrides_.get());
                    blendDrawAdapter_ = std::make_unique<RegPartEdit<>>(blendDraw_.get());

                    std::vector<ObjGroupEdit::IniEdits> perGroup(1);
                    ObjGroupEdit::IniEdits& iniEdits = perGroup[0];

                    for (const std::string& obj : drawn_) {
                        auto repIt = representative_.find(obj);
                        const bool normalMap = (repIt != representative_.end())
                                               && hasNormalMap(repIt->second.first, repIt->second.second);
                        const SlotFiles* files = (repIt == representative_.end())
                                                 ? nullptr : slotFiles(repIt->second.first, repIt->second.second);
                        const bool hasTextures = (files != nullptr) && (!files->diffuseRes.empty() || !files->lightMapRes.empty());

                        std::vector<ObjGroupEdit::PartEdit*> edits = {removeFixCallsAdapter_.get()};
                        if (normalMap) {
                            edits.push_back(dropNormalMapAdapter_.get());
                            edits.push_back(shiftDownAdapter_.get());
                        }
                        // THE FILL IS FOR A SECTION THAT DRAWS NOTHING OF ITS OWN.
                        //
                        // `drawindexed = auto` draws the whole merged buffer, so adding it to a
                        // section that already draws always draws something twice -- and one mod
                        // whose Bang picks a hair variant out of an `if` chain had BOTH variants
                        // rendered because RegFillMissing cannot prove such a chain exhaustive and
                        // supplied `auto` anyway (2026-09-14). The mod drawing at all is the signal
                        // that it has said what it wants drawn.
                        //
                        // And when the members disagree about textures, `auto` is not available
                        // even then: it is one draw and they need one each. Such an object gets the
                        // FIRST member's explicit range here, and the rest as appended blocks.
                        auto repDrawIt = representative_.find(obj);
                        const SlotFiles* repDrawFiles = (repDrawIt == representative_.end())
                                                         ? nullptr
                                                         : slotFiles(repDrawIt->second.first, repDrawIt->second.second);

                        if (repDrawFiles != nullptr && !repDrawFiles->draws) {
                            auto objFillIt = objFillAdapters_.find(obj);
                            edits.push_back(objFillIt != objFillAdapters_.end() ? objFillIt->second.get()
                                                                                : fillAdapter_.get());
                        }

                        // AFTER the fill: when the fill supplies the first member's draw, this
                        // block has to follow it, and RegBottomAdd appends where the fill did.
                        auto extraIt = extraDrawAdapters_.find(obj);
                        if (extraIt != extraDrawAdapters_.end()) {
                            edits.push_back(extraIt->second.get());
                        }

                        // No textures and no donor: no fix call either, or NNFix re-slots registers
                        // this section never bound and scrambles what the game had set.
                        if (hasTextures) {
                            edits.push_back(addFixCallAdapter_.get());
                        }
                        // this object's OWN component's hash remap -- the head comes from the Bang,
                        // whose hashes are filed under a different name than the Body's
                        RegPartEdit<>* objAsset = assetAdapterOf(repIt == representative_.end() ? mergeOrder_.front()
                                                                                                : repIt->second.first);
                        if (objAsset != nullptr) {
                            edits.push_back(objAsset);
                        }

                        const ModObj objKey("", obj);
                        iniEdits.edits[objKey] = std::move(edits);
                        iniEdits.trackKeys[objKey] = false;
                    }

                    RegPartEdit<>* skeletonAsset = assetAdapterOf(mergeOrder_.front());

                    const ModObj ibObj("", "ib");
                    iniEdits.edits[ibObj] = {renameIbAdapter_.get(), skeletonAsset, removeDrawIndexedAdapter_.get()};
                    iniEdits.trackKeys[ibObj] = false;

                    const ModObj blendObj("", "blend");
                    ObjGroupEdit::PartEdit* drawEdit = blendDrawAdapter_.get();
                    if (buildBranchVertexCounts()) {
                        drawEdit = blendBranchDrawAdapter_.get();
                    }

                    iniEdits.edits[blendObj] = {renameBlendAdapter_.get(), skeletonAsset, drawEdit};
                    iniEdits.trackKeys[blendObj] = false;

                    for (const char* kind : {"position", "texcoord"}) {
                        const ModObj objKey("", kind);
                        iniEdits.edits[objKey] = {renameAdapter_.get(), skeletonAsset};
                        iniEdits.trackKeys[objKey] = false;
                    }

                    const ModObj otherObj("", "other");
                    iniEdits.edits[otherObj] = {renameAdapter_.get(), skeletonAsset, overridesAdapter_.get()};
                    iniEdits.trackKeys[otherObj] = false;

                    if (!config_.faceReg.empty()) {
                        std::vector<ObjGroupEdit::PartEdit*> faceEdits = {renameAdapter_.get(), faceAssetAdapter_.get()};
                        if (faceRegAdapter_ != nullptr) {
                            faceEdits.push_back(faceRegAdapter_.get());
                        }
                        iniEdits.edits[FaceObj] = std::move(faceEdits);
                        iniEdits.trackKeys[FaceObj] = false;
                    }

                    mainEdits_ = ObjGroupEdit(std::move(perGroup), false);
                }

                static FixerConfig makeConfig() {
                    FixerConfig config{};
                    config.sectionToStr = &renderIfTemplate;
                    return config;
                }

                static const std::string MergeGroupType;

                // FIRST, so it is destroyed LAST: every Z3Predicate read out of a mod's sections
                // belongs to this context and is unusable once it goes. Z3 member ORDER has bitten
                // this repo before -- see Z3Predicate::Impl.
                Z3Context z3Ctx_;

                IniFileFixContext ctx_;
                std::string toModName_;
                GIMIMergeFixerConfig config_;

                std::unordered_map<std::string, ComponentFiles> files_;
                std::unordered_map<std::string, bool> normalMap_;
                std::vector<std::pair<std::string, std::string>> borrowed_;
                std::string faceFile_;

                std::vector<std::string> mergeOrder_;
                std::unordered_map<std::string, VGRemap> remaps_;
                std::unordered_map<std::string, std::size_t> offsets_;
                std::size_t totalVertices_ = 0;
                std::size_t positionStride_ = 40;

                std::vector<std::string> drawn_;
                std::unordered_map<std::string, std::pair<std::string, std::string>> representative_;
                std::unordered_map<std::string, std::vector<std::pair<std::string, std::string>>> members_;

                std::unique_ptr<SlotRemap> slotRemap_;
                std::unique_ptr<ObjGroupEdit> borrowEdit_;
                std::vector<std::unique_ptr<RegDelimitedAdd<>>> borrowRegEdits_;
                std::vector<std::unique_ptr<GraphPartEdit<>>> borrowAdapters_;

                std::vector<Fixer::GroupEdit*> texGroupEdits_;
                std::vector<std::unique_ptr<TexEditorReplace<>>> texReplaces_;
                std::vector<std::unique_ptr<Collector>> texCollects_;

                std::unique_ptr<VGMergeGroupResBuilder> builder_;
                std::vector<std::unique_ptr<BufReplace<>>> bufReplaces_;
                std::unique_ptr<GroupCollector> bufferCollect_;

                std::unique_ptr<ObjFilter> objFilter_;
                std::vector<std::unique_ptr<RegNewVals<>>> indexRegEdits_;
                std::vector<std::unique_ptr<RegPartEdit<>>> indexAdapters_;
                ObjGroupEdit indexEdits_;

                std::unique_ptr<GraphRename<>> renameGraph_;
                std::unique_ptr<GraphRename<>> renameIbGraph_;
                std::unique_ptr<GraphRename<>> renameBlendGraph_;
                std::unordered_map<std::string, std::unique_ptr<RegAssetRemap<>>> assetRemaps_;
                std::unordered_map<std::string, std::unique_ptr<RegPartEdit<>>> assetAdapters_;
                std::unique_ptr<RegAssetRemap<>> faceAssetRemap_;
                std::unique_ptr<RegRemap<>> faceRegFix_;
                std::unique_ptr<RegRemove<>> removeFixCalls_;
                std::unique_ptr<RegRemove<>> dropNormalMap_;
                std::unique_ptr<RegRemap<>> shiftDown_;
                std::unique_ptr<RegRemove<>> removeDrawIndexed_;
                std::unique_ptr<RegFillMissing<>> fillDrawIndexed_;
                std::vector<std::unique_ptr<RegFillMissing<>>> objFills_;
                std::unordered_map<std::string, std::unique_ptr<GraphPartEdit<>>> objFillAdapters_;
                std::vector<Fixer::GroupEdit*> preRemapTexGroupEdits_;
                std::vector<std::unique_ptr<RegBottomAdd<>>> extraDraws_;
                std::vector<std::unique_ptr<RegBranchAdd<>>> branchDraws_;
                std::unique_ptr<RegBranchAdd<>> blendBranchDraw_;
                std::unique_ptr<GraphPartEdit<>> blendBranchDrawAdapter_;
                std::unordered_map<std::string, std::unique_ptr<GraphPartEdit<>>> extraDrawAdapters_;
                std::unique_ptr<RegDelimitedAdd<>> addFixCall_;
                std::unique_ptr<RegNewVals<>> overrides_;
                std::unique_ptr<RegNewVals<>> blendDraw_;

                std::unique_ptr<GraphPartEdit<>> renameAdapter_;
                std::unique_ptr<GraphPartEdit<>> renameIbAdapter_;
                std::unique_ptr<GraphPartEdit<>> renameBlendAdapter_;
                std::unique_ptr<RegPartEdit<>> faceAssetAdapter_;
                std::unique_ptr<RegPartEdit<>> faceRegAdapter_;
                std::unique_ptr<RegPartEdit<>> removeFixCallsAdapter_;
                std::unique_ptr<RegPartEdit<>> dropNormalMapAdapter_;
                std::unique_ptr<RegPartEdit<>> shiftDownAdapter_;
                std::unique_ptr<RegPartEdit<>> removeDrawIndexedAdapter_;
                std::unique_ptr<GraphPartEdit<>> fillAdapter_;
                std::unique_ptr<GraphPartEdit<>> addFixCallAdapter_;
                std::unique_ptr<RegPartEdit<>> overridesAdapter_;
                std::unique_ptr<RegPartEdit<>> blendDrawAdapter_;

                ObjGroupEdit mainEdits_;
        };

        const std::string GIMIMergeFixerImpl::MergeGroupType = "merge";
    }


    IniFixBuilder::Factory makeGIMIMergeFixer(GIMIMergeFixerConfig config) {
        return [config](BaseIniParser<>* parser, const std::string& toModName, std::optional<int> modTypeId) {
            return std::make_shared<GIMIMergeFixerImpl>(parser, toModName, modTypeId, config);
        };
    }
}
