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
#include "AGRemapCore/data/IniFixData/ModBranches.h"

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
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/BaseIniGraphGroupEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphGroupEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphGroupPartEdits.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphGroupRemap.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphGroupRemove.h"
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
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegRestrict.h"
#include "AGRemapCore/model/strategies/iniParsers/BaseIniParser.h"
#include "AGRemapCore/model/strategies/texEditors/TexCreator.h"
#include "AGRemapCore/tools/StringTools.h"
#include "AGRemapCore/tools/files/FileService.h"


namespace AGRemapCore {
    namespace {
        // Whether a key is a texture register, `ps-t<number>` -- the keys the slot restriction governs.
        // The fix's own scratch register (ps-tNormal) is not one.
        //
        // Nor are TexFx's two dedicated registers, ps-t69 and ps-t70 (2026-09-22): they are not a
        // shader slot of the TARGET but the input of the TexFx library the mod calls, and no target
        // slot lists them. Restricted, a mod's `ps-t69 = ResourceTransparency` was deleted while its
        // `run = CommandList\TexFx\TN.0` stayed -- a Citlali mod's lace skirt frill, alpha-cut through
        // TexFx, vanished from the remap.
        bool isTextureRegister(const std::string& key) {
            if (key == IniKeywords::PsT69 || key == IniKeywords::PsT70) {
                return false;
            }

            const std::string prefix = "ps-t";
            if (key.size() <= prefix.size() || key.compare(0, prefix.size(), prefix) != 0) {
                return false;
            }

            return std::all_of(key.begin() + prefix.size(), key.end(), [](char c) { return c >= '0' && c <= '9'; });
        }

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


        const std::string FormatKey = "format";
        const std::string R32Format = "DXGI_FORMAT_R32_UINT";
        const std::string StrideKey = "stride";


        // 'extras' are forced onto the generated resource section -- see ResEditConfig::extraKVPs.
        // Empty for every buffer but the Texcoord, whose width the target decides.
        BaseResEdit<>::ResEditConfig makeResEditConfig(std::vector<std::pair<std::string, std::string>> extras = {}) {
            BaseResEdit<>::ResEditConfig config{
                IniKeywords::Filename,
                [](const std::string& value) { return value; },
                [](const std::string& file) { return file; }
            };
            config.extraKVPs = std::move(extras);
            return config;
        }


        // A file the PARSER registered as a download rather than one the mod ships, recognised by the
        // RemapDL its name must carry, as the no-mesh check below recognises one.
        bool isDownloadPath(const std::string& path) {
            return FileService::pathToStr(FileService::strToPath(path).filename()).find(IniKeywords::RemapDL) != std::string::npos;
        }

        // ...and one not on disk (yet): downloads are fetched in fixResources, AFTER the fix reads them.
        bool isUnfetchedDownload(const std::string& path) {
            if (!isDownloadPath(path)) {
                return false;
            }
            std::error_code ec;
            return !std::filesystem::exists(FileService::strToPath(path), ec);
        }


        // ---- a mod's own draw ranges, through the split (2026-09-21) ----
        //
        // A mod that draws for itself carries `drawindexed = <count>, <start>, <base>` lines, and a
        // mod with toggles carries SEVERAL, each a range of its object's index buffer that one toggle
        // shows or hides. The split removes triangles from that buffer -- the ones another component
        // draws, and not only at its end (Citlali's eyes sit in the middle of her body) -- so the
        // mod's own numbers overrun the written buffer and every range after a removal draws the
        // wrong triangles. Measured on a real Citlali mod: a head draw of 23439 indices over a split
        // buffer of 20451, and nine body ranges summing past the body's end by exactly the eyes' 864.
        //
        // Each range is remapped through the SOURCE index of every kept triangle (ascending): its new
        // start is the number of kept triangles before it, its new count the number inside it. An
        // `auto` draw is left alone. File-local: it needs the split, which only this template has.
        class DrawRangeRemap : public BaseRegEdit<> {
            public:
                explicit DrawRangeRemap(std::vector<std::size_t> keptIds): keptIds_(std::move(keptIds)) {}

                ContentPart& edit(ContentPart& part, const std::string& sectionName, const ModType* modType = nullptr,
                                  const std::string& modName = "", const OrderRanges* partRanges = nullptr) override {
                    (void)sectionName;
                    (void)modType;
                    (void)modName;

                    const auto ranges = toRangeSpec(partRanges);
                    const std::vector<std::pair<long long, std::string>> vals = part.getValsWithInds(IniKeywords::DrawIndexed, true, ranges);
                    if (vals.empty()) {
                        return part;
                    }

                    std::vector<std::string> remapped;
                    bool changed = false;
                    for (const auto& entry : vals) {
                        std::optional<std::string> value = remap(entry.second);
                        changed = changed || value.has_value();
                        remapped.push_back(value.value_or(entry.second));
                    }

                    if (changed) {
                        part.replaceVals({{IniKeywords::DrawIndexed, ContentPart::ReplaceSpec(remapped)}}, false, ranges);
                    }
                    return part;
                }

            private:
                std::vector<std::size_t> keptIds_;

                // "count, start, base" -> the remapped line, or nullopt for anything else (`auto`)
                std::optional<std::string> remap(const std::string& value) const {
                    std::vector<long long> numbers;
                    std::size_t pos = 0;
                    while (pos <= value.size()) {
                        std::size_t comma = value.find(',', pos);
                        std::string field(StringTools::strip(value.substr(pos, comma == std::string::npos ? std::string::npos : comma - pos)));
                        if (field.empty() || field.find_first_not_of("-0123456789") != std::string::npos) {
                            return std::nullopt;
                        }
                        numbers.push_back(std::stoll(field));
                        if (comma == std::string::npos) {
                            break;
                        }
                        pos = comma + 1;
                    }

                    if (numbers.size() < 2 || numbers[0] < 0 || numbers[1] < 0) {
                        return std::nullopt;
                    }

                    const std::size_t first = static_cast<std::size_t>(numbers[1]) / 3;
                    const std::size_t last = (static_cast<std::size_t>(numbers[1]) + static_cast<std::size_t>(numbers[0])) / 3;
                    const auto begin = std::lower_bound(keptIds_.begin(), keptIds_.end(), first);
                    const auto end = std::lower_bound(keptIds_.begin(), keptIds_.end(), last);
                    const std::size_t start = static_cast<std::size_t>(begin - keptIds_.begin());
                    const std::size_t count = static_cast<std::size_t>(end - begin);

                    std::string out = std::to_string(count * 3) + ", " + std::to_string(start * 3);
                    for (std::size_t i = 2; i < numbers.size(); ++i) {
                        out += ", " + std::to_string(numbers[i]);
                    }
                    return out;
                }
        };


        // ---- what one mod's .ini names, read off the raw parsed sections ----
        //
        // The fixer is built BEFORE the parser parses (the builder's factory runs first), so the
        // files are found by hash over IniFile::getIfTemplates rather than through the parser's
        // graphs -- exactly as the prototype did.
        struct ModObjectFiles {
            // The first branch's ib that names a file, for everything that needs one path.
            std::string ib;

            // EVERY ib the object's section binds through `run =`, each with its condition: a merged
            // master names one variant's per $swapvar branch. A branch that nulls the object is kept,
            // as an entry with no file, so it can answer for itself -- see GIMIMergeFixer's
            // SlotFiles::ibs for what dropping it cost.
            std::vector<BranchVal> ibs;

            std::string diffuse;
            std::string lightMap;

            // Whether the object's section binds the normal-map layout (normal map, diffuse, light
            // map), so a normal-map slot needs no shift -- see GIMIComponentFixerConfig::sourceLayout.
            bool normalMapLayout = false;

            // Whether the object's section (through `run =`) already calls ORFix itself -- see
            // buildEdits for why such a mod's own calls are kept.
            bool ownORFix = false;
        };

        struct ModFiles {
            // The first branch's, as ModObjectFiles::ib.
            std::string position;
            std::string blend;
            std::string texcoord;
            std::string face;

            // Whether the face diffuse was found at ps-t0 -- see
            // GIMIComponentFixerConfig::faceSwapOnlyFromDiffuseReg.
            bool faceOnDiffuseReg = false;

            // Every branch's -- see ModObjectFiles::ibs.
            std::vector<BranchVal> positions;
            std::vector<BranchVal> blends;
            std::vector<BranchVal> texcoords;

            std::vector<std::pair<std::string, ModObjectFiles>> objects;    // in the config's draw order

            std::size_t vertexCount = 0;
            std::size_t positionStride = 0;
            std::size_t texcoordStride = 0;

            // What each ib's resource section DECLARES one index to take -- 2 for R16_UINT, keyed by
            // path over every branch. Read, not inferred: a 16-bit buffer whose size divides by 12
            // reads as 32-bit without complaint.
            std::unordered_map<std::string, std::size_t> ibBytesPerIndex;
        };


        // How a merged master's per-branch values are read and paired is shared with the merge --
        // see ModBranches.
        const auto firstVal = &ModBranches::firstVal;


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
                        if (!authorsNoMesh_) {
                            ctx_.log("the " + componentName_ + " fix found nothing it can remap -- no blend, position,"
                                     " texcoord or drawn index buffer it could read, or no vertex-group row --"
                                     " so it writes nothing for this .ini");
                        }

                        giveUp();
                        return;
                    }

                    buildHiddenComponents();
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
                void giveUp() {
                    gaveUp_ = true;
                    this->graphGroupEdits = {&removeEveryGroup_};
                }

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

                    auto fileOf = [&](const std::string& resource) {
                        return ModBranches::fileOf(templates, resource, folder);
                    };

                    // THROUGH `run =`, every branch: a merged master's TextureOverride is nothing but
                    // hash, match_first_index and run, and read off the matched section alone every
                    // buffer of it came out empty and the whole .ini was skipped.
                    const auto filesThroughRun = [&](const std::string& section, const std::string& reg) {
                        std::vector<BranchVal> out;
                        for (const BranchVal& value : branches_.valsThroughRun(templates, section, reg)) {
                            std::string file = fileOf(ModBranches::resourceOf(value.val));
                            if (!file.empty()) {
                                out.push_back(BranchVal{std::move(file), value.query});
                            }
                        }

                        return out;
                    };

                    const auto firstFile = [&](const std::string& section, const std::string& reg) {
                        return fileOf(ModBranches::resourceOf(branches_.firstValThroughRun(templates, section, reg)));
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

                        const std::string& sectionName = entry.first;
                        const std::string& hashType = hashKey->back();
                        const auto readBuffer = [&](std::vector<BranchVal>& branches, std::string& first, const std::string& reg) {
                            if (!branches.empty()) {
                                return;
                            }

                            branches = filesThroughRun(sectionName, reg);
                            if (!branches.empty()) {
                                first = branches.front().val;
                            }
                        };

                        if (hashType == PositionHashKey) {
                            readBuffer(files_.positions, files_.position, IniKeywords::Vb0);
                        } else if (hashType == BlendHashKey) {
                            readBuffer(files_.blends, files_.blend, IniKeywords::Vb1);
                        } else if (hashType == TexcoordHashKey) {
                            readBuffer(files_.texcoords, files_.texcoord, IniKeywords::Vb1);
                        } else if (hashType == FaceDiffuseHashKey) {
                            if (files_.face.empty()) {
                                files_.face = firstFile(sectionName, DiffuseReg);
                                files_.faceOnDiffuseReg = !files_.face.empty();

                                // A 6.x-shaped mod binds its face diffuse at ps-t1. Looked for only
                                // when the guard is on, so no older config's output can move.
                                if (files_.face.empty() && config_.faceSwapOnlyFromDiffuseReg) {
                                    files_.face = firstFile(sectionName, LightMapReg);
                                }
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

                            ModObjectFiles objFiles;
                            for (const BranchVal& rawIb : branches_.valsThroughRun(templates, sectionName, IniKeywords::Ib)) {
                                // `ib = null` hides the object in THIS branch -- see ModObjectFiles::ibs.
                                if (StringTools::equalsIgnoreCase(rawIb.val, IniKeywords::Null)) {
                                    objFiles.ibs.push_back(BranchVal{std::string(), rawIb.query});
                                    continue;
                                }

                                const std::string resource = ModBranches::resourceOf(rawIb.val);
                                std::string file = fileOf(resource);
                                if (file.empty()) {
                                    continue;
                                }

                                files_.ibBytesPerIndex[file] = ModBranches::ibBytesPerIndexOf(templates, resource);
                                if (objFiles.ib.empty()) {
                                    objFiles.ib = file;
                                }

                                objFiles.ibs.push_back(BranchVal{std::move(file), rawIb.query});
                            }

                            // The first branch's textures: a band legend's diffuse gate is one
                            // filter per object, not per branch.
                            using SourceLayout = GIMIComponentFixerConfig::SourceLayout;
                            objFiles.normalMapLayout = (config_.sourceLayout == SourceLayout::NormalMap)
                                || (config_.sourceLayout == SourceLayout::Detect && !firstFile(sectionName, ShiftedLightMapReg).empty());
                            for (const BranchVal& call : branches_.valsThroughRun(templates, sectionName, IniKeywords::Run)) {
                                if (StringTools::equalsIgnoreCase(StringTools::strip(call.val), IniKeywords::ORFixPath)) {
                                    objFiles.ownORFix = true;
                                    break;
                                }
                            }

                            if (objFiles.normalMapLayout) {
                                objFiles.diffuse = firstFile(sectionName, ShiftedDiffuseReg);
                                objFiles.lightMap = firstFile(sectionName, ShiftedLightMapReg);
                            } else {
                                objFiles.diffuse = firstFile(sectionName, DiffuseReg);
                                objFiles.lightMap = firstFile(sectionName, LightMapReg);
                            }
                            objects[obj] = std::move(objFiles);
                        }
                    }

                    // A DOWNLOAD BESIDE THE MOD'S OWN FILE SERVES ONLY THE FALL-THROUGH PATH (2026-09-22).
                    // A merged master binds a buffer in an `if $swapvar == 0 / else if == 1` chain with
                    // no `else`; the parser covers the path where neither holds by referencing the game's
                    // buffer as a download UNCONDITIONALLY at the top of the root section, where the
                    // chain's own binding, run after it, overrides it on every real path. But a branch
                    // list reads root first, and ModBranches::pick takes the first compatible value, so
                    // the download answered for EVERY state: the split read it (a crash on a first run,
                    // before fixResources fetches it -- a Citlali merged master) or, once fetched,
                    // remapped the game's own buffer in place of the mod's. When a list holds any file
                    // of the mod's own, its downloads are dropped from it; a list of nothing but
                    // downloads (a mod missing that part outright) keeps them.
                    const auto preferAuthored = [](std::vector<BranchVal>& list, std::string& first) {
                        const bool anyAuthored = std::any_of(list.begin(), list.end(), [](const BranchVal& v) {
                            return !v.val.empty() && !isDownloadPath(v.val);
                        });
                        if (!anyAuthored) {
                            return;
                        }
                        list.erase(std::remove_if(list.begin(), list.end(), [](const BranchVal& v) {
                            return !v.val.empty() && isDownloadPath(v.val);
                        }), list.end());
                        first.clear();
                        for (const BranchVal& v : list) {
                            if (!v.val.empty()) {
                                first = v.val;
                                break;
                            }
                        }
                    };
                    preferAuthored(files_.blends, files_.blend);
                    preferAuthored(files_.positions, files_.position);
                    preferAuthored(files_.texcoords, files_.texcoord);
                    for (auto& entry : objects) {
                        preferAuthored(entry.second.ibs, entry.second.ib);
                    }

                    bool anyIb = false;
                    for (const std::string& obj : config_.drawnObjs) {
                        auto it = objects.find(obj);
                        if (it != objects.end()) {
                            anyIb = anyIb || !it->second.ib.empty();
                            files_.objects.emplace_back(obj, it->second);
                        }
                    }

                    if (files_.position.empty() || files_.blend.empty() || files_.texcoord.empty() || !anyIb) {
                        return false;
                    }

                    // A MOD THAT AUTHORS NO MESH IS NOT SOMETHING TO SPLIT.
                    //
                    // The parser fills every object whose command graph is empty with the GAME's
                    // buffers from the downloads, so an .ini that only replaces a texture -- one
                    // mod ships a 124-byte face override beside its model -- arrives here carrying a
                    // complete vanilla model with every path a download. Splitting that fabricates a
                    // mesh the author never provided, and it fails before it can: the downloads land
                    // in fixResources, after this reads the blend.
                    //
                    // A download is recognised the way the remover recognises one, by the RemapDL
                    // its name must carry (see DownloadTools::DownloadPart). The vertex buffers and
                    // EVERY index buffer have to be downloads for this to skip: a mod that ships only
                    // index buffers over the game's own vertices is a real kind of mod -- it hides
                    // parts of the model -- and still has something of its own to split.
                    const auto isDownload = [](const std::string& path) {
                        return FileService::pathToStr(FileService::strToPath(path).filename()).find(IniKeywords::RemapDL)
                               != std::string::npos;
                    };

                    bool authored = false;
                    for (const std::vector<BranchVal>* branches : {&files_.blends, &files_.positions, &files_.texcoords}) {
                        for (const BranchVal& branch : *branches) {
                            authored = authored || !isDownload(branch.val);
                        }
                    }

                    for (const auto& object : files_.objects) {
                        for (const BranchVal& ib : object.second.ibs) {
                            authored = authored || (!ib.val.empty() && !isDownload(ib.val));
                        }
                    }

                    if (!authored) {
                        ctx_.log("this .ini authors no mesh of its own -- every buffer it would split is a download -- so"
                                  " there is no geometry to remap");
                        authorsNoMesh_ = true;
                    }

                    return authored;
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

                    // ONE SPLIT PER STATE. A merged master is several mods behind one .ini, each with its
                    // own blend and index buffers, so what this component draws and how many vertices
                    // it keeps are both per state: an object is drawn if ANY state keeps triangles of
                    // it, and each blend branch's `draw` is its own count (see buildEdits). A mod that
                    // does not branch is one state, and this is the single split it has always been.
                    //
                    // The states come from the blend AND every object's index buffers, not the blend
                    // alone: an animated master binds one blend for its whole frame range and a
                    // different ib per frame. Identical states -- the same blend and index buffers
                    // under different conditions -- are split once.
                    std::vector<const std::vector<BranchVal>*> lists{&files_.blends};
                    for (const auto& object : files_.objects) {
                        lists.push_back(&object.second.ibs);
                    }
                    states_ = branches_.states(lists);

                    std::unordered_map<std::string, std::pair<std::size_t, std::vector<std::string>>> splitCache;
                    std::vector<std::string> drawnAny;

                    // Which states run through a parser download, and whether any does not -- see below.
                    std::vector<bool> throughDownload(states_.size(), false);
                    std::vector<bool> throughUnfetched(states_.size(), false);
                    bool anyAuthoredState = false;
                    for (std::size_t state = 0; state < states_.size(); ++state) {
                        std::vector<std::string> statePaths = ibsFor(states_[state]).second;
                        statePaths.push_back(branches_.pick(files_.blends, files_.blend, states_[state]));
                        for (const std::string& path : statePaths) {
                            throughDownload[state] = throughDownload[state] || isDownloadPath(path);
                            throughUnfetched[state] = throughUnfetched[state] || isUnfetchedDownload(path);
                        }
                        anyAuthoredState = anyAuthoredState || !throughDownload[state];
                    }

                    for (std::size_t state = 0; state < states_.size(); ++state) {
                        const std::string blendPath = branches_.pick(files_.blends, files_.blend, states_[state]);
                        const auto [names, paths] = ibsFor(states_[state]);

                        // A STATE THROUGH A PARSER DOWNLOAD IS NOT SPLIT (2026-09-22). A merged master
                        // binds an object's ib in an `if $swapvar == 0 / else if == 1` chain with no
                        // `else`, so the parser sees a fall-through path with no ib and registers the
                        // game's own buffer as a DOWNLOAD for it. That path is never taken ($swapvar
                        // only cycles its listed values) -- and downloads are fetched in fixResources,
                        // after this reads them, so on a first run in the default Normal mode the file
                        // is not there: reading it threw and skipped the whole .ini (a Citlali merged
                        // master; Bennett's too, on any first run).
                        //
                        // When some OTHER state is fully the mod's own, such a state is skipped whether
                        // the download is on disk or not, so a second run (download now fetched) cannot
                        // split the game's own buffer in and move keptVertices_ -- the output must not
                        // depend on what an earlier run left behind. When EVERY state runs through a
                        // download (a mod that ships only index buffers over the game's own vertices,
                        // or one that does not branch), the downloads are the mod's geometry and are
                        // split as before -- except one not on disk, which leaves nothing to read: that
                        // state is skipped, and with none left the fixer gives up instead of throwing.
                        if ((anyAuthoredState && throughDownload[state]) || throughUnfetched[state]) {
                            stateKept_.push_back(0);
                            continue;
                        }

                        std::string cacheKey = blendPath;
                        for (const std::string& path : paths) {
                            cacheKey += "\n" + path;
                        }

                        auto cached = splitCache.find(cacheKey);
                        if (cached == splitCache.end()) {
                            std::pair<std::size_t, std::vector<std::string>> result{0, {}};

                            BlendFile blend(blendPath);
                            auto [weights, indices] = VGComponentSplit::readBlend(blend);
                            if (files_.vertexCount == 0) {
                                files_.vertexCount = weights.size();
                                if (files_.vertexCount == 0) {
                                    return false;
                                }

                                files_.positionStride = fileSize(files_.position) / files_.vertexCount;
                                files_.texcoordStride = fileSize(files_.texcoord) / files_.vertexCount;
                            }

                            if (!weights.empty() && !paths.empty()) {
                                std::vector<VGComponentSplit::Triangles> ibs;
                                for (const std::string& path : paths) {
                                    auto widthIt = files_.ibBytesPerIndex.find(path);
                                    IbFile ib(path, widthIt == files_.ibBytesPerIndex.end() ? 4 : widthIt->second);
                                    ibs.push_back(VGComponentSplit::readIb(ib));
                                }

                                VGComponentSplit split(std::move(weights), std::move(indices), std::move(ibs), specs_);
                                VGComponentBuffers buffers = split.split(componentName_);
                                result.first = buffers.stats.keptVertices;

                                // The kept triangles per object, for the mod's own draw ranges -- only
                                // for a mod that does not branch: a merged master's ranges belong to
                                // whichever variant's index buffer each branch binds, and are left as
                                // they are (see DrawRangeRemap).
                                if (states_.size() == 1) {
                                    for (std::size_t i = 0; i < names.size() && i < buffers.keptTriangleIds.size(); ++i) {
                                        keptTriangleIds_[names[i]] = buffers.keptTriangleIds[i];
                                    }
                                }

                                for (std::size_t i = 0; i < names.size() && i < buffers.stats.trianglesKept.size(); ++i) {
                                    if (buffers.stats.trianglesKept[i] > 0) {
                                        result.second.push_back(names[i]);
                                    }
                                }
                            }

                            cached = splitCache.emplace(std::move(cacheKey), std::move(result)).first;
                        }

                        stateKept_.push_back(cached->second.first);
                        keptVertices_ = std::max(keptVertices_, cached->second.first);
                        for (const std::string& name : cached->second.second) {
                            if (std::find(drawnAny.begin(), drawnAny.end(), name) == drawnAny.end()) {
                                drawnAny.push_back(name);
                            }
                        }
                    }

                    // every state ran through an unfetched download: nothing of the mod's own to split
                    if (files_.vertexCount == 0) {
                        return false;
                    }

                    for (const std::string& obj : config_.drawnObjs) {
                        if (std::find(drawnAny.begin(), drawnAny.end(), obj) != drawnAny.end()) {
                            drawn_.push_back(obj);
                        }
                    }

                    groupCount_ = std::max<std::size_t>(drawn_.size(), 1);
                    return true;
                }

                // The index buffer of every drawn object that has one under 'query', in draw order,
                // as (names, paths). std::nullopt takes each object's first branch.
                std::pair<std::vector<std::string>, std::vector<std::string>> ibsFor(const std::optional<Z3Predicate>& query) {
                    std::pair<std::vector<std::string>, std::vector<std::string>> out;
                    for (const auto& object : files_.objects) {
                        const std::string path = branches_.pick(object.second.ibs, object.second.ib, query);
                        if (!path.empty()) {
                            out.first.push_back(object.first);
                            out.second.push_back(path);
                        }
                    }

                    return out;
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

                        // An object already on the normal-map layout keeps its own three textures:
                        // no shift, no created normal map, and its diffuse / light map are read
                        // where they already are.
                        const bool alreadyNormalMap = (files != nullptr && files->normalMapLayout);
                        const std::string diffuseReg = alreadyNormalMap ? ShiftedDiffuseReg : DiffuseReg;
                        const std::string lightMapReg = alreadyNormalMap ? ShiftedLightMapReg : LightMapReg;

                        // The diffuse edit, where the config has one for this object.
                        for (const auto& entry : config_.diffuseEdits) {
                            if (entry.first != name) {
                                continue;
                            }

                            auto replace = std::make_unique<TexEditorReplace<>>(
                                GraphId(group, "", component_.slot + "RemapTexDiffuse"),
                                TexEditor({entry.second}, config_.compressTextures, config_.mipmaps), makeResEditConfig(),
                                "resourceRemapTexEdit", std::string("Diffuse"));

                            auto collect = std::make_unique<Collector>();
                            collect->srcRegs = {{slot, diffuseReg}};
                            collect->resEdits = {{"diffuse", replace.get()}};

                            texGroupEdits_.push_back(collect.get());
                            texReplaces_.push_back(std::move(replace));
                            texCollects_.push_back(std::move(collect));
                        }

                        // The lightmap edit, built from this object's diffuse. Restricted to the
                        // objects config_.lightMapObjs names, when it names any: a band legend is
                        // per OBJECT, and Bennett's band 0 is hair on his head and cloth on his body.
                        const bool editThisObj = config_.lightMapObjs.empty() ||
                            std::find(config_.lightMapObjs.begin(), config_.lightMapObjs.end(), name)
                                != config_.lightMapObjs.end();

                        if (config_.lightMapEdit && files != nullptr && editThisObj) {
                            TexEditor::Filter filter = config_.lightMapEdit(files->diffuse);
                            if (filter) {
                                auto replace = std::make_unique<TexEditorReplace<>>(
                                    GraphId(group, "", component_.slot + "RemapTexLightMap"),
                                    TexEditor({filter}, config_.compressTextures, config_.mipmaps), makeResEditConfig(),
                                    "resourceRemapTexEdit", std::string("LightMap"));

                                auto collect = std::make_unique<Collector>();
                                collect->srcRegs = {{slot, lightMapReg}};
                                collect->resEdits = {{"lightMap", replace.get()}};

                                texGroupEdits_.push_back(collect.get());
                                texReplaces_.push_back(std::move(replace));
                                texCollects_.push_back(std::move(collect));
                            }
                        }

                        if (alreadyNormalMap) {
                            continue;
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
                bool objectIbWasNarrow(const std::string& obj) const {
                    for (const auto& entry : files_.objects) {
                        if (entry.first != obj) {
                            continue;
                        }

                        for (const BranchVal& ib : entry.second.ibs) {
                            auto it = files_.ibBytesPerIndex.find(ib.val);
                            if (it != files_.ibBytesPerIndex.end() && it->second == 2) {
                                return true;
                            }
                        }
                    }

                    return false;
                }

                void buildBufferCollects() {
                    VGSplitGroupConfig splitConfig;
                    splitConfig.component = componentName_;
                    splitConfig.specs = specs_;
                    splitConfig.ibBytesPerIndex = files_.ibBytesPerIndex;
                    splitConfig.texcoordLineEdit = makeTexcoordLineEdit();

                    // The index buffers PER GROUP: a group is one satisfiable state of the mod, and
                    // the split needs every drawn object's ib of THAT state -- see
                    // VGSplitGroupResBuilder's resolver.
                    const std::string srcName = ctx_.modTypeName().value_or("");
                    builder_ = std::make_unique<VGSplitGroupResBuilder>(
                        srcName + toModName_ + "Buffers",
                        [this, splitConfig](const Z3Predicate* query) {
                            VGSplitGroupConfig config = splitConfig;
                            config.ibPaths = ibsFor(branches_.localQuery(query)).second;
                            return config;
                        },
                        ctx_.getIniFile());

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

                            // The Texcoord is written at the TARGET's width, so the copied
                            // section's `stride` has to say so -- it came from the MOD, and a
                            // widened buffer declared at the mod's width is read short per vertex.
                            std::vector<std::pair<std::string, std::string>> extras;
                            if (kind.first == "texcoord" && component_.texcoordStride != 0) {
                                extras.emplace_back(StrideKey, std::to_string(component_.texcoordStride));
                            }

                            // Same rule for an index buffer read at 16 bits: the split writes 32, so
                            // the copied section's R16_UINT would describe a file that no longer
                            // exists. Only where the source was 16-bit, so nothing else moves.
                            if (kind.first == "ib" && objectIbWasNarrow(name)) {
                                extras.emplace_back(FormatKey, R32Format);
                            }

                            auto replace = std::make_unique<BufReplace<>>(
                                resObj, makeResEditConfig(std::move(extras)), kind.first,
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

                // ---- the components nothing is remapped onto ----
                //
                // Written by the fixer for the LAST configured component and by no other. Two
                // constraints meet here and only one arrangement satisfies both:
                //
                //   * ONE owner, or several fixers over one .ini emit the same section name twice
                //     and GIMI warns that two sections claim one hash;
                //   * the LAST one, because each fixer's output REPLACES the .ini rather than
                //     adding to what the previous fixer wrote. Owned by the first, the text is
                //     built (measured: 76 bytes on the Body fixer) and then written over by the Eye
                //     fixer, whose own copy is empty -- the section never reaches the file and
                //     nothing says so. This is the hazard Jean's two-target row found: fixers
                //     overwriting each other's .ini text.
                //
                // So config.components must be in the same order as the character's
                // IniFixBuilderData rows, which is the order the fixers run in.
                //
                // The hash is read out of the hash table under the component's own mod type name,
                // which is where a target component's rows are filed.
                void buildHiddenComponents() {
                    if ((config_.hiddenComponents.empty() && config_.unremappedSlots.empty())
                            || config_.components.empty() || config_.components.back().name != componentName_) {
                        return;
                    }

                    IniFile* iniFile = ctx_.getIniFile();
                    Hashes* hashes = ctx_.modTypeHashes();
                    if (hashes == nullptr) {
                        return;
                    }

                    // The TARGET's version, not the source's. These names are target components and
                    // their rows are filed under the target's game version -- BennettAdventure's at
                    // 5.7, where the mod being fixed is a 4.0 Bennett. Asking the source's bucket
                    // finds nothing, and with errorOnNotFound false that is silent: the section is
                    // simply never written, which looks exactly like not having asked for one.
                    const std::optional<Version> toVersion = (iniFile == nullptr) ? std::nullopt : iniFile->toVersion;
                    std::string text;

                    for (const std::string& modTypeName : config_.hiddenComponents) {
                        // Hashes are keyed {name, type} -- TWO non-version values, where Indices are keyed
                        // {name, obj, objName} and take three. Passing the index shape here made
                        // every .ini skip with "expected 2 non-version values, got 3".
                        std::optional<std::string> hash = hashes->get({modTypeName, IbHashKey}, toVersion, false);
                        if (!hash.has_value() || hash->empty()) {
                            continue;
                        }

                        if (!text.empty()) {
                            text += "\n";
                        }

                        // Named with the Remap keyword like every other section a fix writes.
                        text += "[TextureOverride" + modTypeName + "IB" + IniKeywords::Remap + "Hide]\n"
                                "hash = " + *hash + "\n"
                                "handling = skip\n";
                    }

                    if (!text.empty()) {
                        text = "; The skin's own draws for components nothing was remapped onto. Left drawing,\n"
                               "; they sit on top of the mod -- her bangs over his hair, as two different whites.\n\n"
                               + text;
                    }

                    // The skin's slots nothing is remapped onto withdraw a TexFx request -- see
                    // GIMIComponentFixerConfig::unremappedSlots. Only for a mod that calls TexFx.
                    const bool callsTexFx = iniFile != nullptr
                        && StringTools::containsIgnoreCase(iniFile->getFileTxt(), IniKeywords::TexFxFolder + "\\");
                    std::string guards;
                    for (const auto& [modTypeName, slotIndices] : (callsTexFx ? config_.unremappedSlots
                                                                              : decltype(config_.unremappedSlots){})) {
                        std::optional<std::string> hash = hashes->get({modTypeName, IbHashKey}, toVersion, false);
                        if (!hash.has_value() || hash->empty()) {
                            continue;
                        }

                        for (const std::string& slotIndex : slotIndices) {
                            guards += "\n[TextureOverride" + modTypeName + slotIndex + IniKeywords::Remap + "TexFxGuard]\n"
                                      "hash = " + *hash + "\n"
                                      "match_first_index = " + slotIndex + "\n"
                                      "$\\TexFx\\use_default_shader = -1\n";
                        }
                    }

                    if (!guards.empty()) {
                        if (!text.empty()) {
                            text += "\n";
                        }
                        text += "; The skin's own slots nothing was remapped onto. They are skipped already; these\n"
                                "; withdraw a TexFx request the mod's draw made, which the skin's next outline draw\n"
                                "; -- one of these -- would otherwise serve by drawing its whole ib over the mod's\n"
                                "; vertex buffers.\n"
                                + guards;
                    }

                    if (text.empty()) {
                        return;
                    }

                    this->appendedSections = text;
                }

                VGSplitGroupConfig::LineEdit makeTexcoordLineEdit() const {
                    const bool normalise = config_.normaliseVertexColour;
                    const bool zeroUV = config_.zeroSecondUV;
                    const std::size_t want = component_.texcoordStride;

                    // Nothing to do only when the width is already right AND neither edit applies.
                    if (!normalise && (!zeroUV || files_.texcoordStride < 20)
                            && (want == 0 || want == files_.texcoordStride)) {
                        return nullptr;
                    }

                    return [normalise, zeroUV, want](const ByteVec& line) {
                        ByteVec out = line;

                        // The target's width FIRST, so the edits below see the final line. Widening
                        // zero-pads at the end, which is where TEXCOORD1 sits; narrowing drops it.
                        if (want != 0 && out.size() != want) {
                            out.resize(want, static_cast<std::uint8_t>(0));
                        }

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
                    // ONE CALL PER PATH, as the classic and merge templates already do: ORFix /
                    // NNFix re-slot the bound ps-t registers rather than setting them, so a second
                    // call over one set of bindings undoes the first and the model renders flat
                    // green. The default (PerSegment) puts one before EVERY drawindexed, which a
                    // mod that draws an object as several toggled ranges turns into several calls
                    // on one path -- 9 on a real Citlali mod (Tools/Misc/Diagnostics/fixCallPaths.py).
                    // Every mod this template had been checked on drew through one draw per path.
                    addFixCall_ = std::make_unique<RegDelimitedAdd<>>(
                        RegDelimitedAdd<>::Additions{{IniKeywords::Run, fixPath}},
                        RegDelimitedAdd<>::RegMap{{IniKeywords::DrawIndexed, {}}},
                        /*pathEndOnlyWhenUndelimited*/ true,
                        RegDelimitedAddMode::PerPath);

                    normalBack_ = std::make_unique<RegRemap<>>(std::vector<std::pair<std::string, RegRemap<>::KeyRemapValue>>{
                        renameRule(ScratchNormalReg, {DiffuseReg})});

                    // The vertex-limit raise: the target's own buffer is sized for its own vertex
                    // count, and the mod's kept vertices go through it. The LARGEST branch's, since
                    // this one number has to cover whichever variant is selected.
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

                    if (!component_.slotRegisters.empty()) {
                        // Per part -- see RegRestrict and Component::slotRegisters.
                        trimRegisters_ = std::make_unique<RegRestrict<>>(component_.slotRegisters, &isTextureRegister);
                        trimRegistersAdapter_ = std::make_unique<RegPartEdit<>>(trimRegisters_.get());
                    }

                    // A merged master's blend carries a `draw` per branch, and each is that variant's
                    // own count: one number for all of them stops a bigger variant part way through
                    // its model. REPLACED, not added -- a second `draw` draws the model twice.
                    ObjGroupEdit::PartEdit* drawEdit = blendDrawAdapter_.get();
                    if (files_.blends.size() > 1) {
                        blendBranchDraw_ = branches_.replacePerBranch(
                            files_.blends, "blend",
                            [this](std::size_t, const std::optional<Z3Predicate>& local) -> RegBranchAdd<>::Additions {
                                // The largest of the states this blend branch is drawn in: several
                                // index buffers may go with one blend, and `draw` has to cover
                                // whichever of them is selected.
                                std::size_t kept = 0;
                                for (std::size_t state = 0; state < states_.size() && state < stateKept_.size(); ++state) {
                                    if (!states_[state].has_value() || !local.has_value()
                                            || branches_.compatible(*states_[state], *local)) {
                                        kept = std::max(kept, stateKept_[state]);
                                    }
                                }

                                if (kept == 0) {
                                    return {};
                                }

                                return {{IniKeywords::Draw, std::to_string(kept) + ",0"}};
                            });
                        blendBranchDrawAdapter_ = std::make_unique<GraphPartEdit<>>(blendBranchDraw_.get());
                        drawEdit = blendBranchDrawAdapter_.get();
                    }

                    const ModObj slotObj("", component_.slot);
                    std::vector<ObjGroupEdit::IniEdits> perGroup;

                    for (std::size_t group = 0; group < groupCount_; ++group) {
                        ObjGroupEdit::IniEdits iniEdits;

                        // KEEP THE MOD'S OWN FIX CALLS when they are already the right ones: an object
                        // on the normal-map layout that calls ORFix itself, drawn through a slot that
                        // reads ORFix. Then wherever the author put the calls is right for the target
                        // too -- which is what the pure-Python original always did, renaming the
                        // modder's calls out of the way and back rather than inserting any.
                        //
                        // Re-issuing instead (drop them, add one per path) assumes a section never
                        // rebinds its ps-t registers after drawing -- RegDelimitedAddMode::PerPath says
                        // so, measured over 33030 sections. A real Citlali mod does: it binds, calls
                        // ORFix, draws, binds again, calls ORFix again and draws again. With one call
                        // per path, the second set of bindings drew un-reslotted -- the light map as the
                        // albedo, flat green over every cloth part (2026-09-22). fixCallPaths.py cannot
                        // see that shape: it counts calls made twice, not a call missing after a rebind.
                        bool keepOwnFixCalls = false;
                        if (component_.normalMap && group < drawn_.size()) {
                            const ModObjectFiles* objFiles = objectFiles(drawn_[group]);
                            keepOwnFixCalls = objFiles != nullptr && objFiles->normalMapLayout && objFiles->ownORFix;
                        }

                        std::vector<ObjGroupEdit::PartEdit*> slotEdits;
                        if (!keepOwnFixCalls) {
                            slotEdits.push_back(removeFixCallsAdapter_.get());
                        }
                        if (component_.normalMap) {
                            slotEdits.push_back(normalBackAdapter_.get());
                        }

                        // The mod's own draw ranges, through the split -- see DrawRangeRemap. Group
                        // `group` holds drawn_[group]'s graph.
                        if (group < drawn_.size()) {
                            auto kept = keptTriangleIds_.find(drawn_[group]);
                            if (kept != keptTriangleIds_.end()) {
                                drawRangeRemaps_.push_back(std::make_unique<DrawRangeRemap>(kept->second));
                                drawRangeAdapters_.push_back(std::make_unique<RegPartEdit<>>(drawRangeRemaps_.back().get()));
                                slotEdits.push_back(drawRangeAdapters_.back().get());
                            }
                        }

                        // AFTER the shift and the normal map's rename back: the double binding is
                        // made by the shift, and ps-t0 is only a real register again after the rename.
                        if (trimRegistersAdapter_ != nullptr) {
                            slotEdits.push_back(trimRegistersAdapter_.get());
                        }
                        slotEdits.push_back(fillAdapter_.get());
                        if (!keepOwnFixCalls) {
                            slotEdits.push_back(addFixCallAdapter_.get());
                        }
                        slotEdits.push_back(assetAdapter_.get());
                        iniEdits.edits[slotObj] = std::move(slotEdits);
                        iniEdits.trackKeys[slotObj] = false;

                        iniEdits.edits[IbObj] = {renameIbAdapter_.get(), assetAdapter_.get(), removeDrawIndexedAdapter_.get()};
                        iniEdits.trackKeys[IbObj] = false;

                        std::vector<ObjGroupEdit::PartEdit*> blendEdits = {renameBlendAdapter_.get(), assetAdapter_.get()};
                        if (!component_.negativeIndex) {
                            blendEdits.push_back(drawEdit);
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
                            // The swap only for a mod on the pre-6.x register, when the config asks
                            // -- see GIMIComponentFixerConfig::faceSwapOnlyFromDiffuseReg.
                            if (!config_.faceSwapOnlyFromDiffuseReg || files_.faceOnDiffuseReg) {
                                iniEdits.edits[FaceObj] = {renameAdapter_.get(), faceAssetAdapter_.get(), faceSwapAdapter_.get()};
                            } else {
                                iniEdits.edits[FaceObj] = {renameAdapter_.get(), faceAssetAdapter_.get()};
                            }
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

                // FIRST, so it is destroyed LAST: every Z3Predicate read out of the mod's sections
                // belongs to its context -- see ModBranches.
                ModBranches branches_;

                IniFileFixContext ctx_;
                std::string toModName_;
                GIMIComponentFixerConfig config_;
                std::string componentName_;
                GIMIComponentFixerConfig::Component component_;

                ModFiles files_;
                std::vector<VGComponentSpec> specs_;
                std::vector<std::string> drawn_;
                std::size_t keptVertices_ = 0;                  // the largest state's
                std::vector<std::optional<Z3Predicate>> states_;  // see ModBranches::states
                std::vector<std::size_t> stateKept_;            // per states_ entry
                std::size_t groupCount_ = 1;

                bool authorsNoMesh_ = false;
                bool gaveUp_ = false;
                GraphGroupRemove<> removeEveryGroup_;
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
                std::unordered_map<std::string, std::vector<std::size_t>> keptTriangleIds_;
                std::vector<std::unique_ptr<DrawRangeRemap>> drawRangeRemaps_;
                std::vector<std::unique_ptr<RegPartEdit<>>> drawRangeAdapters_;
                std::unique_ptr<RegDelimitedAdd<>> addFixCall_;
                std::unique_ptr<RegRemap<>> normalBack_;
                std::unique_ptr<RegNewVals<>> overrides_;
                std::unique_ptr<RegNewVals<>> blendDraw_;
                std::unique_ptr<RegRestrict<>> trimRegisters_;
                std::unique_ptr<RegPartEdit<>> trimRegistersAdapter_;
                std::unique_ptr<RegBranchAdd<>> blendBranchDraw_;
                std::unique_ptr<GraphPartEdit<>> blendBranchDrawAdapter_;

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
