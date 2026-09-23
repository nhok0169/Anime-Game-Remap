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

#include "AGRemapCore/data/IniFixData/RegValChecks.h"
#include "AGRemapCore/data/IniFixData/TexRegLayout.h"
#include "AGRemapCore/data/IniFixData/ModBranches.h"

#include <algorithm>
#include <map>
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
#include "AGRemapCore/model/files/IbFile.h"
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
#include "AGRemapCore/model/strategies/iniFixers/graphEdits/RegSurroundedAdd.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphGroupEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphGroupPartEdits.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphGroupRemap.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphGroupRemove.h"
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
        const std::string DrawHashKey = "draw_vb";          // the VertexLimitRaise section's hash
        const std::string FaceDiffuseHashKey = "tex_face_diffuse";

        const std::string DiffuseReg = "ps-t0";
        const std::string LightMapReg = "ps-t1";
        const std::string NormalShiftedDiffuseReg = "ps-t1";
        const std::string NormalShiftedLightMapReg = "ps-t2";

        // True everywhere but the outline pass: ORFix tags every outline vertex shader with this
        // filter_index (BufferValues/ORFix.ini, [ShaderOverrideOutlineVS...]).
        const std::string NotOutlinePass = "vs != 037730.0";

        const std::string OverrideByteStride = "override_byte_stride";
        const std::string OverrideVertexCount = "override_vertex_count";
        const std::string DrawIndexedAuto = "auto";
        const std::string SkipHandling = "skip";

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


        // ps-t0 again: the register a normal-map layout reads the NORMAL MAP out of, which the
        // plain layout reads the diffuse out of -- named for both so a rule says which it means.
        const std::string& NormalMapReg = DiffuseReg;


        // ---- a carried member's draws, moved to where it sits in the merged ib ----
        //
        // A merged object's ib is member after member, so a member carried as a command list of its
        // own (see buildSlotRemap) draws its OWN ranges, each shifted by its start: `count, start,
        // base` -> `count, start + offset, base`, and `auto` -> the member's whole range. File-local,
        // like the component template's DrawRangeRemap: it needs the merge's offsets.
        class DrawOffset : public BaseRegEdit<> {
            public:
                DrawOffset(long long offset, long long count): offset_(offset), count_(count) {}

                ContentPart& edit(ContentPart& part, const std::string& sectionName, const ModType* modType = nullptr,
                                  const std::string& modName = "", const OrderRanges* partRanges = nullptr) override {
                    (void)sectionName;
                    (void)modType;
                    (void)modName;

                    const auto ranges = toRangeSpec(partRanges);
                    const std::vector<std::pair<long long, std::string>> vals = part.getValsWithInds(IniKeywords::DrawIndexed, true, ranges);
                    if (!vals.empty()) {
                        std::vector<std::string> shifted;
                        shifted.reserve(vals.size());
                        for (const auto& entry : vals) {
                            shifted.push_back(shift(entry.second));
                        }
                        part.replaceVals({{IniKeywords::DrawIndexed, ContentPart::ReplaceSpec(shifted)}}, false, ranges);
                    }

                    // AND TEXFX'S OWN DRAW: its component command lists draw
                    // `drawindexed = $_1, $_2, 0` from variables the mod sets, so `$\texfx\_2` is a
                    // start index too (a CitlaliWhisperofStars mod's toggled piece, 2026-09-22).
                    std::vector<std::string> startKeys;
                    for (const auto& kvp : part.entries()) {
                        if (StringTools::equalsIgnoreCase(StringTools::strip(kvp.first), TexFxDrawStart)
                                && std::find(startKeys.begin(), startKeys.end(), kvp.first) == startKeys.end()) {
                            startKeys.push_back(kvp.first);
                        }
                    }
                    for (const std::string& key : startKeys) {
                        std::vector<std::string> shifted;
                        for (const auto& entry : part.getValsWithInds(key, true, ranges)) {
                            const std::string value(StringTools::strip(entry.second));
                            shifted.push_back(!value.empty() && value.find_first_not_of("0123456789") == std::string::npos
                                              ? std::to_string(std::stoll(value) + offset_) : entry.second);
                        }
                        part.replaceVals({{key, ContentPart::ReplaceSpec(shifted)}}, false, ranges);
                    }

                    return part;
                }

            private:
                static inline const std::string TexFxDrawStart = "$\\texfx\\_2";

                long long offset_;
                long long count_;

                std::string shift(const std::string& value) const {
                    if (StringTools::equalsIgnoreCase(StringTools::strip(value), "auto")) {
                        return std::to_string(count_) + ", " + std::to_string(offset_) + ", 0";
                    }

                    std::vector<std::string> fields;
                    std::size_t pos = 0;
                    while (true) {
                        std::size_t comma = value.find(',', pos);
                        fields.emplace_back(StringTools::strip(value.substr(pos, comma == std::string::npos ? std::string::npos : comma - pos)));
                        if (comma == std::string::npos) {
                            break;
                        }
                        pos = comma + 1;
                    }

                    if (fields.size() < 2 || fields[1].empty() || fields[1].find_first_not_of("0123456789") != std::string::npos) {
                        return value;
                    }

                    fields[1] = std::to_string(std::stoll(fields[1]) + offset_);
                    std::string out = fields[0];
                    for (std::size_t i = 1; i < fields.size(); ++i) {
                        out += ", " + fields[i];
                    }
                    return out;
                }
        };


        const std::string FormatKey = "format";
        const std::string R32Format = "DXGI_FORMAT_R32_UINT";

        // 'extras' are forced onto the generated resource section -- see ResEditConfig::extraKVPs.
        BaseResEdit<>::ResEditConfig makeResEditConfig(std::vector<std::pair<std::string, std::string>> extras = {}) {
            BaseResEdit<>::ResEditConfig config{
                IniKeywords::Filename,
                [](const std::string& value) { return value; },
                [](const std::string& file) { return file; }
            };
            config.extraKVPs = std::move(extras);
            return config;
        }


        // How a merged master's per-branch values are read and paired is shared with the split --
        // see ModBranches.
        const auto firstVal = &ModBranches::firstVal;


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
            std::string normalMapRes;    // only on the normal-map layout, for a normal-map TARGET's re-issued bindings
            std::string section;         // the slot's own TextureOverride, for carrying it -- see buildSlotRemap
            bool ownFix = false;         // whether that section (through `run =`) calls ORFix / NNFix itself
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
            bool hasIbSection = false;     // the component's own ib section (hash, no match_first_index)
            bool hasOtherSection = false;  // its VertexLimitRaise (the draw_vb hash), which carries the overrides
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
                        giveUp("could not read the mod's sections against " + toModName_ + "'s hashes");
                        return;
                    }

                    resolveTargets();
                    if (drawn_.empty()) {
                        giveUp("found no object of " + toModName_ + " that any of the mod's slots draws");
                        return;
                    }

                    buildSlotRemap();
                    buildBorrowEdits();
                    buildTexEdits();
                    buildBufferCollect();
                    buildIndexEdits();
                    buildEdits();

                    // AFTER buildEdits: the adapter it runs is one buildEdits creates.
                    buildTexRegNormalize();

                    this->graphGroupEdits.clear();

                    // FIRST OF ALL: every carried binding onto the register its name says, on each
                    // slot's OWN graph. Before the collects (which look a register up to find the
                    // light map) and before the remap (which folds these graphs into the target's),
                    // so that everything after this reads one layout -- see texRegsByName.
                    if (texRegNormalize_.has_value()) {
                        this->graphGroupEdits.push_back(&texRegNormalize_.value());
                    }

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

                    // A fixer that gave up withdraws the downloads too. The parser registers one per
                    // object it filled, at parse time, and fixResources fetches every one it finds
                    // whether or not anything references it -- so writing nothing would still drop the
                    // game's files into the mod's folder. They are the .ini file's rather than this
                    // fixer's: this is right while every fixer over one file succeeds or fails
                    // together, which holds here because each component's fixer reads the same files.
                    if (gaveUp_ && ctx_.getIniFile() != nullptr) {
                        ctx_.getIniFile()->getFileDownloads().clear();
                    }
                }

            private:
                // Gives up: the fixer writes NOTHING. See GraphGroupRemove -- returning without this
                // renders the mod's own sections again under their source names, which the remover
                // cannot strip and every later run appends to, and the run counts the .ini as fixed.
                void giveUp(const std::string& why) {
                    ctx_.log("the merge " + why + ", so it writes nothing for this .ini");
                    gaveUp_ = true;
                    this->graphGroupEdits = {&removeEveryGroup_};
                }

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

                    const auto resourceOf = &ModBranches::resourceOf;
                    auto fileOf = [&](const std::string& resource) {
                        return ModBranches::fileOf(templates, resource, folder);
                    };

                    // How many bytes an index takes in the buffer this resource names -- see
                    // IbFile::bytesPerIndexOf.
                    auto bytesPerIndexOf = [&](const std::string& resource) {
                        return ModBranches::ibBytesPerIndexOf(templates, resource);
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
                                for (const BranchVal& resource : branches_.valsThroughRun(templates, sectionName, reg)) {
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
                            } else if (hashType == DrawHashKey) {
                                // The VertexLimitRaise -- see ComponentFiles::hasOtherSection.
                                files.hasOtherSection = true;
                            } else if (hashType == FaceDiffuseHashKey) {
                                if (faceFile_.empty()) {
                                    faceFile_ = fileOf(resourceOf(branches_.firstValThroughRun(templates, sectionName, DiffuseReg)));
                                }
                            } else if (hashType == IbHashKey) {
                                std::optional<std::string> index = firstVal(tpl, IniKeywords::MatchFirstIndex);
                                if (!index.has_value()) {
                                    // The component's own ib section -- the one that carries
                                    // `handling = skip`, and the one the target's skip is made from.
                                    files.hasIbSection = true;
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
                                        !resourceOf(branches_.firstValThroughRun(templates, sectionName, "ps-t2")).empty();
                                    SlotFiles slotFiles;
                                    slotFiles.found = true;
                                    slotFiles.section = sectionName;

                                    for (const BranchVal& call : branches_.valsThroughRun(templates, sectionName, IniKeywords::Run)) {
                                        const std::string value(StringTools::strip(call.val));
                                        if (StringTools::equalsIgnoreCase(value, IniKeywords::ORFixPath)
                                                || StringTools::equalsIgnoreCase(value, IniKeywords::NNFixPath)) {
                                            slotFiles.ownFix = true;
                                        }
                                    }
                                    for (const BranchVal& rawIb : branches_.valsThroughRun(templates, sectionName, IniKeywords::Ib)) {
                                        // `ib = null` hides the object in THIS branch, and the
                                        // branch is kept so it can say so -- see SlotFiles::nullIb.
                                        if (StringTools::equalsIgnoreCase(rawIb.val, IniKeywords::Null)) {
                                            slotFiles.nullIb = true;
                                            slotFiles.ibs.push_back(BranchVal{std::string(), rawIb.query});
                                            continue;
                                        }

                                        std::string file = fileOf(resourceOf(rawIb.val));
                                        if (!file.empty()) {
                                            ibBytesPerIndex_[file] = bytesPerIndexOf(resourceOf(rawIb.val));
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
                                        branches_.valsThroughRun(templates, sectionName, IniKeywords::DrawIndexed);
                                    slotFiles.draws = !slotFiles.drawVals.empty();

                                    // Measured first, config second -- see Slot::indexCount. Only
                                    // a target object several slots land on ever reads this.
                                    slotFiles.indexCount =
                                        static_cast<long long>(fileSize(slotFiles.ib) / IbIndexStride);
                                    if (slotFiles.indexCount == 0) {
                                        slotFiles.indexCount = slot.indexCount;
                                    }
                                    slotFiles.diffuseRes = resourceOf(branches_.firstValThroughRun(
                                        templates, sectionName, normalMap ? NormalShiftedDiffuseReg : DiffuseReg));
                                    slotFiles.lightMapRes = resourceOf(branches_.firstValThroughRun(
                                        templates, sectionName, normalMap ? NormalShiftedLightMapReg : LightMapReg));
                                    if (normalMap) {
                                        slotFiles.normalMapRes = resourceOf(branches_.firstValThroughRun(
                                            templates, sectionName, DiffuseReg));
                                    }

                                    // WHICH REGISTER HOLDS WHICH ROLE, BY NAME.
                                    //
                                    // The reading above is positional, which is only right for a mod
                                    // written in the fix's own layout -- and the three roles it hands
                                    // out decide far more than the bindings do: which file the light
                                    // map band edit reads, which register that edit COLLECTS, and
                                    // which texture a missing role downloads. A mod dumped from the
                                    // game has them in the game's order, and the band edit then
                                    // rewrote its DIFFUSE and named the result `...LightMapRemapTex`.
                                    // See GIMIMergeFixerConfig::texRegsByName. A role the mod does
                                    // not name keeps whatever the positional reading gave it.
                                    if (config_.texRegsByName) {
                                        for (const std::string& reg : {NormalMapReg, NormalShiftedDiffuseReg,
                                                                       NormalShiftedLightMapReg}) {
                                            const std::string res(resourceOf(branches_.firstValThroughRun(
                                                templates, sectionName, reg)));
                                            if (res.empty()) {
                                                continue;
                                            }

                                            if (RegValChecks::isNormalMap(res)) {
                                                slotFiles.normalMapRes = res;
                                            } else if (RegValChecks::isLightMap(res)) {
                                                slotFiles.lightMapRes = res;
                                            } else if (RegValChecks::isDiffuse(res)) {
                                                slotFiles.diffuseRes = res;
                                            }
                                        }
                                    }
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

                // ---- a merged member CARRIED as a command list of its own (2026-09-22) ----
                //
                // Every member but an object's first used to be DROPPED and replaced by one draw of
                // its whole range, so everything its own section said -- its toggles, its draw ranges,
                // where its fix calls sit -- was lost. A CitlaliWhisperofStars mod toggles inside every
                // slot: its hair drew all three of its variants at once, its outfit every piece of
                // every outfit. Now each such member is copied under a command-list name, its draws
                // shifted to where it sits in the merged ib, and called from the object's section.
                //
                // Not carried, and drawn as before: a member whose section the mod does not have (a
                // downloaded component) and one of an object whose first member's ib branches (a
                // merged master's variants have offsets per branch -- see buildBranchDraws).
                bool isCarried(const std::string& obj, const std::pair<std::string, std::string>& member) const {
                    auto repIt = representative_.find(obj);
                    if (repIt == representative_.end() || repIt->second == member) {
                        return false;
                    }

                    const SlotFiles* repFiles = slotFiles(repIt->second.first, repIt->second.second);
                    const SlotFiles* files = slotFiles(member.first, member.second);
                    return repFiles != nullptr && repFiles->ibs.size() <= 1 && files != nullptr && files->found
                           && !files->section.empty() && files->ibs.size() <= 1 && files->indexCount > 0;
                }

                bool anyCarried(const std::string& obj) const {
                    auto membersIt = members_.find(obj);
                    if (membersIt == members_.end()) {
                        return false;
                    }
                    for (const auto& member : membersIt->second) {
                        if (isCarried(obj, member)) {
                            return true;
                        }
                    }
                    return false;
                }

                static std::string memberKey(const std::string& obj, const std::pair<std::string, std::string>& member) {
                    return obj + ";" + member.first + ";" + member.second;
                }

                // The copy's names: the section's own, suffixed per member so two members' copies of
                // one shared command list cannot collide, and a TextureOverride made a command list --
                // the copy is only ever called, and has no hash left to match with.
                SlotRemap::RenameFunc memberRename(const std::pair<std::string, std::string>& member) const {
                    const std::string suffix = toModName_ + member.first + member.second;
                    return [suffix](const std::string& name) {
                        std::string renamed = IniNamingTools::getRemapFixName(name, suffix);
                        const std::string textureOverride = "TextureOverride";
                        if (renamed.size() >= textureOverride.size()
                                && StringTools::equalsIgnoreCase(std::string_view(renamed).substr(0, textureOverride.size()), textureOverride)) {
                            renamed = "CommandList" + renamed.substr(textureOverride.size());
                        }
                        return renamed;
                    };
                }

                // ---- 1. the graphs onto the target's, all in ONE .ini file ----
                void buildSlotRemap() {
                    SlotRemap::RemapList remap;
                    const SlotRemap::RenameFunc keepName = [](const std::string& name) { return name; };
                    const std::string& skeleton = mergeOrder_.front();

                    // THE TARGET'S `handling = skip` COMES FROM WHICHEVER COMPONENT HAS AN ib SECTION.
                    //
                    // It hides the target's own draws, and without it they run against the MERGED
                    // buffers -- the position / blend / texcoord overrides are by hash and apply to
                    // every draw -- through the target's own index buffer: a spray of stretched
                    // triangles (a CitlaliWhisperofStars mod of the Bangs alone, 2026-09-22; see
                    // Images/CitlaliWhisper/6_7/CitlaliBrokenModel.jpg). It used to be taken from the
                    // skeleton's, and a mod that does not carry the skeleton component at all has
                    // none: the parser invents that component's position, blend and texcoord, but its
                    // index buffers are invented per SLOT, so no component-level ib section exists.
                    // The same for the VertexLimitRaise (the "other" kind), which carries
                    // `override_vertex_count` -- the merged model has every component's vertices and
                    // the target's own limit is its own model's, so without the override everything
                    // past it reads whatever follows in memory: the same mod drew a black sheet from
                    // the head down (Images/CitlaliWhisper/6_7/CitlaliBrokenModel2.jpg).
                    const auto donorFor = [this, &skeleton](bool ComponentFiles::*has) {
                        const ComponentFiles* skeletonFiles = componentFiles(skeleton);
                        if (skeletonFiles != nullptr && skeletonFiles->*has) {
                            return skeleton;
                        }

                        for (const std::string& component : mergeOrder_) {
                            const ComponentFiles* files = componentFiles(component);
                            if (files != nullptr && files->*has) {
                                return component;
                            }
                        }

                        return skeleton;
                    };

                    ibDonor_ = donorFor(&ComponentFiles::hasIbSection);
                    otherDonor_ = donorFor(&ComponentFiles::hasOtherSection);
                    const std::string& otherDonor = otherDonor_;
                    const std::string& ibDonor = ibDonor_;

                    for (const GIMIMergeFixerConfig::Component& component : config_.components) {
                        const bool isSkeleton = (component.name == skeleton);

                        for (const char* kind : {"ib", "blend", "position", "texcoord", "other"}) {
                            const std::string kindName(kind);
                            const bool isDonor = (kindName == "ib") ? (component.name == ibDonor)
                                               : (kindName == "other") ? (component.name == otherDonor)
                                               : isSkeleton;

                            std::vector<SlotRemap::RemapTarget> targets;
                            if (isDonor) {
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
                            } else if (isCarried(slot.to, {component.name, slot.name})) {
                                const std::pair<std::string, std::string> member{component.name, slot.name};
                                targets.emplace_back(GraphId(0, "", memberKey(slot.to, member)), memberRename(member));
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
                        RegDelimitedAdd<>::Additions bindings;
                        if (normalTarget() && !files->normalMapRes.empty()) {
                            bindings.emplace_back(DiffuseReg, files->normalMapRes);
                        }
                        bindings.emplace_back(targetDiffuseReg(), files->diffuseRes);
                        bindings.emplace_back(targetLightMapReg(), files->lightMapRes);

                        auto edit = std::make_unique<RegDelimitedAdd<>>(
                            std::move(bindings),
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
                // ---- every slot's bindings onto the registers their NAMES say ----
                //
                // See GIMIMergeFixerConfig::texRegsByName. One edit over every slot graph rather
                // than a per-object one inside mainEdits_, because the ROLE a register holds is
                // asked long before that: the light map band edit COLLECTS a register, and on a mod
                // binding two different layouts in one section (its own slot, then a second draw of
                // the target's own body) no single register answers for both -- it edited Citlali's
                // normal map. Normalising up front leaves exactly one layout for everything after.
                void buildTexRegNormalize() {
                    if (texRegsByNameAdapter_ == nullptr) {
                        return;
                    }

                    std::vector<ObjGroupEdit::IniEdits> iniEdits(1);
                    for (const auto& component : files_) {
                        for (const auto& slot : component.second.slots) {
                            const ModObj objKey(component.first, slot.first);

                            std::vector<ObjGroupEdit::PartEdit*> edits;
                            if (dropNormalMapByNameAdapter_ != nullptr) {
                                edits.push_back(dropNormalMapByNameAdapter_.get());
                            }
                            edits.push_back(texRegsByNameAdapter_.get());

                            iniEdits[0].edits[objKey] = std::move(edits);
                            iniEdits[0].trackKeys[objKey] = false;
                        }
                    }

                    texRegNormalize_ = ObjGroupEdit(std::move(iniEdits), false);
                }

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
                        const std::string reg = config_.texRegsByName
                            ? targetLightMapReg() : (normalMap ? NormalShiftedLightMapReg : LightMapReg);

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
                            const std::string reg = config_.texRegsByName
                                ? targetLightMapReg() : (normalMap ? NormalShiftedLightMapReg : LightMapReg);

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
                    const std::optional<Z3Predicate> local = branches_.localQuery(query);

                    VGMergeGroupConfig config;
                    config.ibBytesPerIndex = ibBytesPerIndex_;

                    for (const std::string& component : mergeOrder_) {
                        const ComponentFiles* files = componentFiles(component);
                        auto remapIt = remaps_.find(component);
                        if (files == nullptr || remapIt == remaps_.end()) {
                            return VGMergeGroupConfig{};
                        }

                        VGMergeComponentFiles entry;
                        entry.spec.name = component;
                        entry.spec.remap = remapIt->second;
                        entry.blendPath = branches_.pick(files->blends, files->blend, local);
                        entry.positionPath = branches_.pick(files->positions, files->position, local);
                        entry.texcoordPath = branches_.pick(files->texcoords, files->texcoord, local);
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
                        object.srcPath = (repFiles == nullptr) ? "" : branches_.pick(repFiles->ibs, repFiles->ib, local);

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
                            std::string path = branches_.pick(files->ibs, files->ib, local);
                            if (path.empty()) {
                                continue;
                            }

                            object.members.emplace_back(member.first, std::move(path));
                        }

                        config.objects.push_back(std::move(object));
                    }

                    return config;
                }

                // Whether any branch of any member this target object draws was declared 16-bit.
                bool objectHasNarrowIb(const std::string& obj) {
                    auto membersIt = members_.find(obj);
                    if (membersIt == members_.end()) {
                        return false;
                    }

                    for (const auto& member : membersIt->second) {
                        const SlotFiles* files = slotFiles(member.first, member.second);
                        if (files == nullptr) {
                            continue;
                        }

                        for (const BranchVal& branch : files->ibs) {
                            auto it = ibBytesPerIndex_.find(branch.val);
                            if (it != ibBytesPerIndex_.end() && it->second == 2) {
                                return true;
                            }
                        }
                    }

                    return false;
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

                        // The merge writes 32-bit indices whatever it read, so where any member was
                        // 16-bit the copied section's R16_UINT describes a file that is not there.
                        std::vector<std::pair<std::string, std::string>> extras;
                        if (objectHasNarrowIb(obj)) {
                            extras.emplace_back(FormatKey, R32Format);
                        }

                        auto replace = std::make_unique<BufReplace<>>(resObj, makeResEditConfig(std::move(extras)), "ib",
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

                            const std::optional<Z3Predicate> local = branches_.localQuery(&query);
                            const long long branch = branches_.branchIndexOf(branches, local);
                            if (branch < 0) {
                                return result;
                            }

                            // Does the mod draw this object itself in THIS branch? Four of one
                            // master's twelve leave it to the whole-ib override, which the remap
                            // takes away -- so the first member needs a draw of its own there, and
                            // needs none where the mod already issues its ranges.
                            const bool branchDraws = branches_.anyCompatible(repFiles->drawVals, local);

                            RegBranchAdd<>::Additions additions;
                            long long offset = 0;
                            const SlotFiles* bound = repFiles;

                            for (std::size_t i = 0; i < members.size(); ++i) {
                                const SlotFiles* files = slotFiles(members[i].first, members[i].second);
                                if (files == nullptr) {
                                    continue;
                                }

                                const std::string path = branches_.pick(files->ibs, files->ib, local);
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
                                    if (!slotOutlined(members[i])) {
                                        ctx_.log("the '" + members[i].first + " " + members[i].second
                                                  + "' slot is drawn per branch, so it stays in the outline pass");
                                    }
                                    appendMemberBindings(additions, members[i], bound);
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

                    extraDrawAdapters_[obj].push_back(std::make_unique<GraphPartEdit<>>(branchAdd.get()));
                    branchDraws_.push_back(std::move(branchAdd));
                    return true;
                }

                // A member that reads its own textures binds them ahead of its draw, and then needs
                // its own fix call -- rebinding ps-t0/ps-t1 starts a new binding generation and
                // NNFix re-slots whatever is bound when it runs. Shared with the unbranched path so
                // the two cannot drift.
                //
                // Compared with what is BOUND at that point, not with the representative's: a member
                // on the representative's textures that follows one on its own has to put them
                // back. CitlaliWhisperofStars' Eyes borrow Body A's set and are merged after Body D,
                // which binds its own, and the eyes drew with Body D's atlas (2026-09-22). 'bound'
                // starts as the representative's and follows every member that rebinds.
                void appendMemberBindings(std::vector<std::pair<std::string, std::string>>& additions,
                                           const std::pair<std::string, std::string>& member, const SlotFiles*& bound) {
                    const SlotFiles* files = slotFiles(member.first, member.second);
                    const bool ownTextures = (files != nullptr) && (bound != nullptr)
                                              && (!files->diffuseRes.empty() || !files->lightMapRes.empty())
                                              && (files->diffuseRes != bound->diffuseRes
                                                   || files->lightMapRes != bound->lightMapRes);
                    if (!ownTextures) {
                        return;
                    }
                    bound = files;

                    // A normal-map target reads the member's normal map too; without one of its own
                    // (a plain-layout slot) the draw keeps whatever ps-t0 already holds.
                    if (normalTarget() && !files->normalMapRes.empty()) {
                        additions.emplace_back(DiffuseReg, files->normalMapRes);
                    }

                    if (!files->diffuseRes.empty()) {
                        additions.emplace_back(targetDiffuseReg(), files->diffuseRes);
                    }

                    if (!files->lightMapRes.empty()) {
                        // The name buildMemberTexEdits' own edit will produce, worked out with the
                        // very function that produces it rather than by copying the convention.
                        const std::string edited =
                            config_.lightMapEdit
                                ? IniNamingTools::getRemapTexResourceName(
                                      files->lightMapRes, TextTools::capitalize(toModName_) + "LightMap")
                                : files->lightMapRes;

                        additions.emplace_back(targetLightMapReg(), edited);
                    }

                    additions.emplace_back(IniKeywords::Run, fixPath());
                }

                // Whether a slot is drawn in the target's outline pass -- see Slot::outline.
                bool slotOutlined(const std::pair<std::string, std::string>& member) const {
                    for (const GIMIMergeFixerConfig::Component& component : config_.components) {
                        if (component.name != member.first) {
                            continue;
                        }
                        for (const GIMIMergeFixerConfig::Slot& slot : component.slots) {
                            if (slot.name == member.second) {
                                return slot.outline;
                            }
                        }
                    }
                    return true;
                }

                // ---- the TARGET's register layout -- see GIMIMergeFixerConfig::TargetLayout ----
                bool normalTarget() const {
                    return config_.targetLayout == GIMIMergeFixerConfig::TargetLayout::NormalMap;
                }

                const std::string& targetDiffuseReg() const {
                    return normalTarget() ? NormalShiftedDiffuseReg : DiffuseReg;
                }

                const std::string& targetLightMapReg() const {
                    return normalTarget() ? NormalShiftedLightMapReg : LightMapReg;
                }

                const std::string& fixPath() const {
                    return normalTarget() ? IniKeywords::ORFixPath : IniKeywords::NNFixPath;
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

                        const std::string path = branches_.pick(files->blends, files->blend, query);
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

                    // REPLACED, not added: the branch already carries a `draw` of its own, and a second
                    // one draws the model twice rather than correcting the first.
                    auto branchAdd = branches_.replacePerBranch(
                        skeleton->blends, "blend",
                        [this](std::size_t, const std::optional<Z3Predicate>& local) -> RegBranchAdd<>::Additions {
                            const std::size_t vertices = mergedVertexCount(local);
                            if (vertices == 0) {
                                return {};
                            }

                            return {{IniKeywords::Draw, std::to_string(vertices) + ",0"}};
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

                    // And the other way, for a plain-layout slot onto a normal-map target.
                    shiftUp_ = std::make_unique<RegRemap<>>(std::vector<std::pair<std::string, RegRemap<>::KeyRemapValue>>{
                        renameRule(DiffuseReg, {NormalShiftedDiffuseReg}),
                        renameRule(LightMapReg, {NormalShiftedLightMapReg})});

                    // Both of those by NAME instead -- see GIMIMergeFixerConfig::texRegsByName,
                    // and TexRegLayout, which is where the rule itself lives so the other two
                    // templates can reach it.
                    if (config_.texRegsByName) {
                        const std::vector<std::string> texRegs{NormalMapReg, NormalShiftedDiffuseReg,
                                                               NormalShiftedLightMapReg};
                        texRegsByName_ = std::make_unique<RegRemap<>>(
                            TexRegLayout::byName(TexRegLayout::fixLibraryRoles(normalTarget()), texRegs));

                        // The plain target has no normal-map slot to keep one on.
                        if (!normalTarget()) {
                            dropNormalMapByName_ = std::make_unique<RegRemove<>>(
                                TexRegLayout::removeNormalMap(texRegs));
                        }
                    }

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
                        if (!repFiles->draws && !membersDiffer(obj) && !anyCarried(obj)) {
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
                        std::vector<std::pair<long long, long long>> ranges;     // (start, count) per member after the first
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
                                ranges.emplace_back(offset, count);
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
                        //
                        // ONE BLOCK PER RUN of members that agree about the outline pass: a member
                        // kept out of it (Slot::outline) goes in a block of its own under
                        // `if vs != 037730.0`, in order with the rest. After such a block what is
                        // bound depends on the pass, so the next member rebinds whatever it reads --
                        // 'unknown' has no resources, so every member with textures differs from it.
                        static const SlotFiles unknown{};

                        if (!slotOutlined(membersIt->second.front())) {
                            ctx_.log("the '" + membersIt->second.front().first + " " + membersIt->second.front().second
                                      + "' slot draws '" + obj + "' first, so it stays in the outline pass");
                        }

                        //
                        // A CARRIED member is one `run =` of its own command list, in order with the
                        // rest, under the same outline condition. What it leaves bound is its own
                        // business, so the member after it rebinds.
                        const SlotFiles* bound = repFiles;
                        RegBottomAdd<>::Additions block;
                        bool blockOpen = false;
                        bool blockOutlined = true;

                        const auto addBottom = [&](RegBottomAdd<>::Additions additions, bool outlined) {
                            auto bottomAdd = std::make_unique<RegBottomAdd<>>(std::move(additions), outlined ? "" : NotOutlinePass);
                            extraDrawAdapters_[obj].push_back(std::make_unique<GraphPartEdit<>>(bottomAdd.get()));
                            extraDraws_.push_back(std::move(bottomAdd));
                        };
                        const auto flush = [&]() {
                            if (blockOpen && !block.empty()) {
                                addBottom(std::move(block), blockOutlined);
                                if (!blockOutlined) {
                                    bound = &unknown;
                                }
                            }
                            block.clear();
                            blockOpen = false;
                        };

                        for (std::size_t i = 0; i < extras.size(); ++i) {
                            const auto& member = membersIt->second[i + 1];
                            const bool outlined = slotOutlined(member);

                            if (isCarried(obj, member)) {
                                flush();
                                const SlotFiles* files = slotFiles(member.first, member.second);
                                addBottom({{IniKeywords::Run, memberRename(member)(files->section)}}, outlined);
                                carriedDraws_[memberKey(obj, member)] = {member, ranges[i]};
                                bound = &unknown;
                                continue;
                            }

                            if (blockOpen && outlined != blockOutlined) {
                                flush();
                            }
                            if (!blockOpen) {
                                blockOpen = true;
                                blockOutlined = outlined;
                            }

                            appendMemberBindings(block, member, bound);
                            block.emplace_back(IniKeywords::DrawIndexed, extras[i]);
                        }
                        flush();
                    }

                    // An object whose members cannot share a draw needs the FIRST member's range
                    // rather than `auto`, for the section that draws nothing of its own.
                    for (const std::string& obj : drawn_) {
                        auto membersIt = members_.find(obj);
                        auto repIt = representative_.find(obj);
                        if (membersIt == members_.end() || repIt == representative_.end()
                                || membersIt->second.size() < 2 || (!membersDiffer(obj) && !anyCarried(obj))) {
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
                        RegDelimitedAdd<>::Additions{{IniKeywords::Run, fixPath()}},
                        RegDelimitedAdd<>::RegMap{{IniKeywords::DrawIndexed, {}}},
                        /*pathEndOnlyWhenUndelimited*/ true,
                        RegDelimitedAddMode::PerPath);

                    // ONE CALL PER BINDING GENERATION, for a section whose own bindings are KEPT.
                    //
                    // The section above has had the mod's calls stripped and its bindings replaced,
                    // so one per path is right for it. A carried section is the other case: it may
                    // bind twice (its own slot's textures and a draw, then the TARGET character's
                    // and another draw -- a real CitlaliWhisperofStars mod does) and it may already
                    // call the library over the second of those, which is the author's placement
                    // and stays. invalidatorRegs opens a generation, coveredRegs is the author's
                    // own call serving one; what is left over gets a call of its own.
                    RegDelimitedAdd<>::RegMap bindings;
                    for (const std::string& reg : {NormalMapReg, NormalShiftedDiffuseReg, NormalShiftedLightMapReg}) {
                        bindings.emplace(reg, RegDelimitedAdd<>::Predicate{});
                    }

                    keepOwnFixCall_ = std::make_unique<RegDelimitedAdd<>>(
                        RegDelimitedAdd<>::Additions{{IniKeywords::Run, fixPath()}},
                        RegDelimitedAdd<>::RegMap{{IniKeywords::DrawIndexed, {}}},
                        /*pathEndOnlyWhenUndelimited*/ false,
                        RegDelimitedAddMode::PerBindingGeneration,
                        std::move(bindings),
                        RegDelimitedAdd<>::RegMap{{IniKeywords::Run, [](const std::string& val) {
                            const std::string call(StringTools::strip(val));
                            return StringTools::equalsIgnoreCase(call, IniKeywords::ORFixPath)
                                    || StringTools::equalsIgnoreCase(call, IniKeywords::NNFixPath);
                        }}});

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

                    // AND WRITTEN WHEN THE SECTION HAS THEM NOWHERE.
                    //
                    // `handling = skip` and `draw` re-issue the vertex pass over the MERGED buffer:
                    // without them the game runs its own with the TARGET's vertex count, so every
                    // vertex past it is never skinned and the model hangs off the rig in stretched
                    // sheets. Both are normally copied from the mod's own blend section -- and a mod
                    // that does not carry the skeleton component has only the one the parser invented
                    // for its downloads, which has neither (2026-09-22; the third section this shape
                    // was missing, after the ib skip and the VertexLimitRaise).
                    //
                    // A COVER, not RegNewVals' addNewKVPs: that adds the pair to every part lacking
                    // it, which writes a second copy inside the `if` block the buffer collect spliced
                    // in. The cover asks whether the ROOT has it at all -- so a mod that carries its
                    // own blend section, or a master carrying one `draw` per branch, is untouched.
                    blendSkipFill_ = std::make_unique<RegFillMissing<>>(
                        IniKeywords::Handling,
                        RegFillMissing<>::makeFillMissing(IniKeywords::Handling, SkipHandling),
                        RegFillMissingMode::BottomCover);

                    blendDrawFill_ = std::make_unique<RegFillMissing<>>(
                        IniKeywords::Draw,
                        RegFillMissing<>::makeFillMissing(IniKeywords::Draw, std::to_string(totalVertices_) + ",0"),
                        RegFillMissingMode::BottomCover);

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
                    shiftUpAdapter_ = std::make_unique<RegPartEdit<>>(shiftUp_.get());
                    if (texRegsByName_ != nullptr) {
                        texRegsByNameAdapter_ = std::make_unique<RegPartEdit<>>(texRegsByName_.get());
                    }
                    if (dropNormalMapByName_ != nullptr) {
                        dropNormalMapByNameAdapter_ = std::make_unique<RegPartEdit<>>(dropNormalMapByName_.get());
                    }
                    removeDrawIndexedAdapter_ = std::make_unique<RegPartEdit<>>(removeDrawIndexed_.get());
                    fillAdapter_ = std::make_unique<GraphPartEdit<>>(fillDrawIndexed_.get());
                    addFixCallAdapter_ = std::make_unique<GraphPartEdit<>>(addFixCall_.get());
                    keepOwnFixCallAdapter_ = std::make_unique<GraphPartEdit<>>(keepOwnFixCall_.get());
                    overridesAdapter_ = std::make_unique<RegPartEdit<>>(overrides_.get());
                    blendDrawAdapter_ = std::make_unique<RegPartEdit<>>(blendDraw_.get());
                    blendSkipFillAdapter_ = std::make_unique<GraphPartEdit<>>(blendSkipFill_.get());
                    blendDrawFillAdapter_ = std::make_unique<GraphPartEdit<>>(blendDrawFill_.get());

                    std::vector<ObjGroupEdit::IniEdits> perGroup(1);
                    ObjGroupEdit::IniEdits& iniEdits = perGroup[0];

                    for (const std::string& obj : drawn_) {
                        auto repIt = representative_.find(obj);
                        const bool normalMap = (repIt != representative_.end())
                                               && hasNormalMap(repIt->second.first, repIt->second.second);
                        const SlotFiles* files = (repIt == representative_.end())
                                                 ? nullptr : slotFiles(repIt->second.first, repIt->second.second);
                        const bool hasTextures = (files != nullptr) && (!files->diffuseRes.empty() || !files->lightMapRes.empty());

                        // KEEP THE MOD'S OWN FIX CALLS on a normal-map target: they are already the
                        // library the target reads, and where the author put them is right. Dropping
                        // them and adding one per path lost the second of a section that binds, calls
                        // ORFix, draws, binds again, calls ORFix and draws again -- the forward
                        // template's Citlali3 finding, met again on a CitlaliWhisperofStars mod.
                        // Only on the layout the target reads: a plain slot is shifted up and its
                        // own NNFix would then read the wrong registers.
                        const bool keepOwnFix = normalTarget() && normalMap && files != nullptr && files->ownFix;

                        std::vector<ObjGroupEdit::PartEdit*> edits;
                        if (!keepOwnFix) {
                            edits.push_back(removeFixCallsAdapter_.get());
                        }
                        // texRegsByName did both of these before the remap -- see
                        // buildTexRegNormalize -- and a positional shift on top would undo it.
                        if (config_.texRegsByName) {
                            // nothing: already in the target's layout
                        } else if (normalMap && !normalTarget()) {
                            edits.push_back(dropNormalMapAdapter_.get());
                            edits.push_back(shiftDownAdapter_.get());
                        } else if (!normalMap && normalTarget() && hasTextures) {
                            edits.push_back(shiftUpAdapter_.get());
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
                            for (const auto& adapter : extraIt->second) {
                                edits.push_back(adapter.get());
                            }
                        }

                        // No textures and no donor: no fix call either, or NNFix re-slots registers
                        // this section never bound and scrambles what the game had set.
                        if (hasTextures && !keepOwnFix) {
                            edits.push_back(addFixCallAdapter_.get());
                        } else if (keepOwnFix) {
                            edits.push_back(keepOwnFixCallAdapter_.get());
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

                    // ---- the carried members' copies ----
                    stripCarried_ = std::make_unique<RegRemove<>>(
                        std::vector<std::pair<std::string, std::optional<RegRemove<>::RemoveKeyCheck>>>{
                            {IniKeywords::Hash, std::nullopt}, {IniKeywords::MatchFirstIndex, std::nullopt},
                            {IniKeywords::Handling, std::nullopt}, {IniKeywords::Ib, std::nullopt}});
                    stripCarriedAdapter_ = std::make_unique<RegPartEdit<>>(stripCarried_.get());

                    for (const auto& entry : carriedDraws_) {
                        const auto& member = entry.second.first;
                        const long long start = entry.second.second.first;
                        const long long count = entry.second.second.second;
                        const SlotFiles* files = slotFiles(member.first, member.second);
                        const bool normalMap = hasNormalMap(member.first, member.second);
                        const bool hasTextures = (files != nullptr) && (!files->diffuseRes.empty() || !files->lightMapRes.empty());
                        const bool keepOwnFix = normalTarget() && normalMap && files != nullptr && files->ownFix;

                        std::vector<ObjGroupEdit::PartEdit*> edits = {stripCarriedAdapter_.get()};
                        if (!keepOwnFix) {
                            edits.push_back(removeFixCallsAdapter_.get());
                        }
                        // texRegsByName did both of these before the remap -- see
                        // buildTexRegNormalize -- and a positional shift on top would undo it.
                        if (config_.texRegsByName) {
                            // nothing: already in the target's layout
                        } else if (normalMap && !normalTarget()) {
                            edits.push_back(dropNormalMapAdapter_.get());
                            edits.push_back(shiftDownAdapter_.get());
                        } else if (!normalMap && normalTarget() && hasTextures) {
                            edits.push_back(shiftUpAdapter_.get());
                        }

                        auto offsetEdit = std::make_unique<DrawOffset>(start, count);
                        auto offsetAdapter = std::make_unique<RegPartEdit<>>(offsetEdit.get());
                        edits.push_back(offsetAdapter.get());

                        // A member that draws nothing of its own draws its whole range.
                        if (files != nullptr && !files->draws) {
                            auto fill = std::make_unique<RegFillMissing<>>(
                                IniKeywords::DrawIndexed,
                                RegFillMissing<>::makeFillMissing(IniKeywords::DrawIndexed,
                                                                  std::to_string(count) + ", " + std::to_string(start) + ", 0"),
                                RegFillMissingMode::BottomCover);
                            auto fillAdapter = std::make_unique<GraphPartEdit<>>(fill.get());
                            edits.push_back(fillAdapter.get());
                            objFills_.push_back(std::move(fill));
                            carriedGraphAdapters_.push_back(std::move(fillAdapter));
                        }

                        // AFTER the draw is in place, so the fill lands in front of it, not behind it.
                        //
                        // A COPY THAT BINDS NOTHING OF ITS OWN gets the member's textures -- its own,
                        // a download or its donor's -- at the top, in the target's layout. Otherwise
                        // its fix call would run over the PREVIOUS member's already-fixed bindings,
                        // and ORFix / NNFix are involutions: a YelanTranquil mod's Eye sections bind
                        // nothing, and its eyes would have drawn flat green. TopdownCover adds at the
                        // top of the copy only where some part lacks the register, and a member that
                        // binds its own (or carries the parser's downloads) rebinds over it before
                        // its fix call, so for it the fill changes nothing.
                        if (hasTextures) {
                            std::vector<std::pair<std::string, std::string>> bindings;
                            if (normalTarget() && !files->normalMapRes.empty()) {
                                bindings.emplace_back(DiffuseReg, files->normalMapRes);
                            }
                            if (!files->diffuseRes.empty()) {
                                bindings.emplace_back(targetDiffuseReg(), files->diffuseRes);
                            }
                            if (!files->lightMapRes.empty()) {
                                bindings.emplace_back(targetLightMapReg(),
                                    config_.lightMapEdit
                                        ? IniNamingTools::getRemapTexResourceName(
                                              files->lightMapRes, TextTools::capitalize(toModName_) + "LightMap")
                                        : files->lightMapRes);
                            }

                            // To the FRONT of the root's first part, in reverse so the lines keep their
                            // order: TopdownCover hands the fill that part, and a copy whose own lines
                            // were all stripped has one part only -- the one its draw is already in.
                            for (auto it = bindings.rbegin(); it != bindings.rend(); ++it) {
                                const auto& binding = *it;
                                auto fill = std::make_unique<RegFillMissing<>>(
                                    binding.first, RegFillMissing<>::makeFillMissing(binding.first, binding.second, /*toFront*/ true),
                                    RegFillMissingMode::TopdownCover);
                                auto fillAdapter = std::make_unique<GraphPartEdit<>>(fill.get());
                                edits.push_back(fillAdapter.get());
                                objFills_.push_back(std::move(fill));
                                carriedGraphAdapters_.push_back(std::move(fillAdapter));
                            }
                        }

                        if (hasTextures && !keepOwnFix) {
                            edits.push_back(addFixCallAdapter_.get());
                        } else if (keepOwnFix) {
                            edits.push_back(keepOwnFixCallAdapter_.get());
                        }

                        RegPartEdit<>* memberAsset = assetAdapterOf(member.first);
                        if (memberAsset != nullptr) {
                            edits.push_back(memberAsset);
                        }

                        const ModObj objKey("", entry.first);
                        iniEdits.edits[objKey] = std::move(edits);
                        iniEdits.trackKeys[objKey] = false;

                        carriedOffsets_.push_back(std::move(offsetEdit));
                        carriedAdapters_.push_back(std::move(offsetAdapter));
                    }

                    RegPartEdit<>* skeletonAsset = assetAdapterOf(mergeOrder_.front());

                    // The DONOR's own hash remap: its section is the component's, and each
                    // component's hashes are filed under that component's name -- the skeleton's
                    // filter would miss and write HashNotFound. See buildSlotRemap's ibDonor.
                    RegPartEdit<>* ibAsset = assetAdapterOf(ibDonor_.empty() ? mergeOrder_.front() : ibDonor_);
                    if (ibAsset == nullptr) {
                        ibAsset = skeletonAsset;
                    }

                    const ModObj ibObj("", "ib");
                    iniEdits.edits[ibObj] = {renameIbAdapter_.get(), ibAsset, removeDrawIndexedAdapter_.get()};
                    iniEdits.trackKeys[ibObj] = false;

                    const ModObj blendObj("", "blend");
                    ObjGroupEdit::PartEdit* drawEdit = blendDrawAdapter_.get();
                    if (buildBranchVertexCounts()) {
                        drawEdit = blendBranchDrawAdapter_.get();
                    }

                    iniEdits.edits[blendObj] = {renameBlendAdapter_.get(), skeletonAsset, drawEdit,
                                                 blendSkipFillAdapter_.get(), blendDrawFillAdapter_.get()};
                    iniEdits.trackKeys[blendObj] = false;

                    for (const char* kind : {"position", "texcoord"}) {
                        const ModObj objKey("", kind);
                        iniEdits.edits[objKey] = {renameAdapter_.get(), skeletonAsset};
                        iniEdits.trackKeys[objKey] = false;
                    }

                    // The donor's own hash remap again -- see buildSlotRemap's donorFor.
                    RegPartEdit<>* otherAsset = assetAdapterOf(otherDonor_.empty() ? mergeOrder_.front() : otherDonor_);
                    if (otherAsset == nullptr) {
                        otherAsset = skeletonAsset;
                    }

                    const ModObj otherObj("", "other");
                    iniEdits.edits[otherObj] = {renameAdapter_.get(), otherAsset, overridesAdapter_.get()};
                    iniEdits.trackKeys[otherObj] = false;

                    if (!config_.faceReg.empty()) {
                        // A FACE DRAW CARRIES NO FIX LIBRARY CALL.
                        //
                        // Every one of Citlali's own mods binds her face diffuse and calls nothing:
                        // the identity's face section is `hash` + `ps-t1 = <diffuse>`, and the
                        // compiled FORWARD fix writes a face as `hash` + `this = <resource>`, with
                        // no register and no call at all. The face is the one object whose register
                        // the fix names outright (config.faceReg), so there is nothing for ORFix to
                        // re-slot -- and `NNFix` over a diffuse sitting at `ps-t1` reads it as the
                        // LIGHT MAP, which is a wrong-textured face.
                        //
                        // A mod only ever has such a call here by carrying one: this one's face
                        // section is written in GIMI's newer API, whose `run = SetTextures`
                        // GIMIApiNormalizer faithfully turns into the traditional call.
                        std::vector<ObjGroupEdit::PartEdit*> faceEdits = {renameAdapter_.get(),
                                                                         removeFixCallsAdapter_.get(),
                                                                         faceAssetAdapter_.get()};
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
                // belongs to its context and is unusable once it goes. Z3 member ORDER has bitten
                // this repo before -- see Z3Predicate::Impl.
                ModBranches branches_;

                bool gaveUp_ = false;
                GraphGroupRemove<> removeEveryGroup_;

                IniFileFixContext ctx_;
                std::string toModName_;
                GIMIMergeFixerConfig config_;

                std::unordered_map<std::string, ComponentFiles> files_;
                std::unordered_map<std::string, bool> normalMap_;
                std::vector<std::pair<std::string, std::string>> borrowed_;
                std::string faceFile_;

                std::vector<std::string> mergeOrder_;
                std::string ibDonor_;                // the component whose ib section becomes the target's skip
                std::string otherDonor_;             // and whose VertexLimitRaise carries the overrides
                std::unordered_map<std::string, VGRemap> remaps_;
                std::unordered_map<std::string, std::size_t> ibBytesPerIndex_;    // declared width, by ib path
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
                std::optional<ObjGroupEdit> texRegNormalize_;

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
                std::unique_ptr<RegRemap<>> shiftUp_;
                std::unique_ptr<RegRemap<>> texRegsByName_;
                std::unique_ptr<RegRemove<>> dropNormalMapByName_;
                std::unique_ptr<RegRemove<>> removeDrawIndexed_;
                std::unique_ptr<RegFillMissing<>> fillDrawIndexed_;
                std::vector<std::unique_ptr<RegFillMissing<>>> objFills_;

                // The carried members -- see isCarried. Keyed by memberKey: (member, (start, count)).
                std::map<std::string, std::pair<std::pair<std::string, std::string>, std::pair<long long, long long>>> carriedDraws_;
                std::unique_ptr<RegRemove<>> stripCarried_;
                std::unique_ptr<RegPartEdit<>> stripCarriedAdapter_;
                std::vector<std::unique_ptr<DrawOffset>> carriedOffsets_;
                std::vector<std::unique_ptr<RegPartEdit<>>> carriedAdapters_;
                std::vector<std::unique_ptr<GraphPartEdit<>>> carriedGraphAdapters_;
                std::unordered_map<std::string, std::unique_ptr<GraphPartEdit<>>> objFillAdapters_;
                std::vector<Fixer::GroupEdit*> preRemapTexGroupEdits_;
                std::vector<std::unique_ptr<RegBottomAdd<>>> extraDraws_;
                std::vector<std::unique_ptr<RegBranchAdd<>>> branchDraws_;
                std::unique_ptr<RegBranchAdd<>> blendBranchDraw_;
                std::unique_ptr<GraphPartEdit<>> blendBranchDrawAdapter_;
                std::unordered_map<std::string, std::vector<std::unique_ptr<GraphPartEdit<>>>> extraDrawAdapters_;
                std::unique_ptr<RegDelimitedAdd<>> addFixCall_;
                std::unique_ptr<RegDelimitedAdd<>> keepOwnFixCall_;
                std::unique_ptr<RegNewVals<>> overrides_;
                std::unique_ptr<RegNewVals<>> blendDraw_;
                std::unique_ptr<RegFillMissing<>> blendSkipFill_;
                std::unique_ptr<RegFillMissing<>> blendDrawFill_;

                std::unique_ptr<GraphPartEdit<>> renameAdapter_;
                std::unique_ptr<GraphPartEdit<>> renameIbAdapter_;
                std::unique_ptr<GraphPartEdit<>> renameBlendAdapter_;
                std::unique_ptr<RegPartEdit<>> faceAssetAdapter_;
                std::unique_ptr<RegPartEdit<>> faceRegAdapter_;
                std::unique_ptr<RegPartEdit<>> removeFixCallsAdapter_;
                std::unique_ptr<RegPartEdit<>> dropNormalMapAdapter_;
                std::unique_ptr<RegPartEdit<>> shiftDownAdapter_;
                std::unique_ptr<RegPartEdit<>> shiftUpAdapter_;
                std::unique_ptr<RegPartEdit<>> texRegsByNameAdapter_;
                std::unique_ptr<RegPartEdit<>> dropNormalMapByNameAdapter_;
                std::unique_ptr<RegPartEdit<>> removeDrawIndexedAdapter_;
                std::unique_ptr<GraphPartEdit<>> fillAdapter_;
                std::unique_ptr<GraphPartEdit<>> addFixCallAdapter_;
                std::unique_ptr<GraphPartEdit<>> keepOwnFixCallAdapter_;
                std::unique_ptr<RegPartEdit<>> overridesAdapter_;
                std::unique_ptr<RegPartEdit<>> blendDrawAdapter_;
                std::unique_ptr<GraphPartEdit<>> blendSkipFillAdapter_;
                std::unique_ptr<GraphPartEdit<>> blendDrawFillAdapter_;

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
