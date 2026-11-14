#include "AGRemapCore/data/IniFixData/WWMIFixer.h"
#include "AGRemapCore/constants/GameType.h"
#include "AGRemapCore/model/strategies/iniFixers/graphEdits/GraphVarRename.h"
#include "AGRemapCore/data/IniFixData/WWMISections.h"

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

#include <tuple>
#include <algorithm>
#include <cstdint>
#include <cctype>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <system_error>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "AGRemapCore/constants/FileExt.h"
#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/constants/WWMINames.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/RabbitFX.h"
#include "AGRemapCore/data/WWMITextureFacts.h"
#include "AGRemapCore/model/strategies/iniParsers/IniParseData.h"
#include "AGRemapCore/model/strategies/iniParsers/TextureRoles.h"
#include "AGRemapCore/data/IniParseData/WWMIParser.h"
#include "AGRemapCore/model/strategies/iniParsers/IniParseRows.h"
#include "AGRemapCore/model/iftemplate/IfContentPart.h"
#include "AGRemapCore/model/iftemplate/IfPredPart.h"
#include "AGRemapCore/model/iftemplate/IfTemplate.h"
#include "AGRemapCore/tools/z3/Z3Context.h"
#include "AGRemapCore/model/IniNamingTools.h"
#include "AGRemapCore/model/IniSectionText.h"
#include "AGRemapCore/model/assets/Hashes.h"
#include "AGRemapCore/model/Version.h"
#include "AGRemapCore/model/buffers/BufInt.h"
#include "AGRemapCore/model/buffers/WWMIBlend.h"
#include "AGRemapCore/model/buffers/WWMIShapeKeys.h"
#include "AGRemapCore/model/files/IbFile.h"
#include "AGRemapCore/model/files/TexcoordFile.h"
#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/model/files/IniSharedState.h"
#include "AGRemapCore/model/iniresources/RemapBlendResource.h"
#include "AGRemapCore/model/strategies/ModType.h"
#include "AGRemapCore/model/strategies/iniFixers/GIMIFixer.h"
#include "AGRemapCore/data/IniFixData/TemplateFixer.h"
#include "AGRemapCore/model/strategies/iniFixers/ShaderFilterTags.h"
#include "AGRemapCore/model/strategies/iniFixers/CustomShaderBackups.h"
#include "AGRemapCore/model/strategies/iniFixers/graphEdits/GraphRename.h"
#include "AGRemapCore/model/strategies/iniFixers/graphEdits/RegFillMissing.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphCopy.h"
#include "AGRemapCore/constants/IniGraphModObjKeywords.h"
#include "AGRemapCore/model/strategies/iniFixers/graphEdits/RegIfTemplateAdd.h"
#include "AGRemapCore/model/strategies/iniFixers/graphEdits/RegSurroundedAdd.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphGroupEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphGroupPartEdits.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/IterGraphGroupEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphGroupRemap.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphRemove.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/ResRegCollect.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/FileAddEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/FileDownloadEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/TexEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/BlendEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/VGRemapBlendEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegAdd.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegAnchorAdd.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegAssetRemap.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegNewVals.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegRemap.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegRemove.h"

#include <stdexcept>
#include "AGRemapCore/model/strategies/iniParsers/GIMIParser.h"
#include "AGRemapCore/model/strategies/texEditors/TexCreator.h"
#include "AGRemapCore/tools/StringTools.h"
#include "AGRemapCore/tools/TextTools.h"
#include "AGRemapCore/tools/files/FileService.h"


namespace AGRemapCore {
    namespace {

        using Fixer = GIMIFixer<>;
        using ObjGroupEdit = GraphGroupEdit<>;
        using ModObj = ObjGroupEdit::ModObj;
        using GraphId = GraphRemove<>::GraphId;
        using Collector = ResRegCollect<>;
        using PartEdit = ObjGroupEdit::PartEdit;
        using IterEdit = IterGraphGroupEdit<>;

        // THE KINDS OF PER-OBJECT EDIT A WWMI FIX MAKES, each a named step of its list, in the order an object's edits run:
        // the call of its texture list, the shared-list bypass, the mod's own texture lines carried, the other additions,
        // the reroute's draw filters, the lines the section drops, RabbitFX's pins, the target slot's
        // numbers, the hashes, the shape-key dispatch size and lists, the shaders after the draw, the marked lines, the
        // overlays, the rerouted shaders and the passes a component is off in
        const std::vector<std::string> ObjEditKinds{
            "textureListCalls", "sharedBypass", "modTexLines", "additions", "rerouteFilters", "removedLines",
            "rabbitFXPins", "slotValues", "hashes", "dispatchSize", "shapeKeyLists", "afterShaders", "markLines", "overlays",
            "rerouteShaders", "offPasses"};

        // Hash-data keys and .ini registers spelled literally, as the rest of data/ spells them.
        const std::string Vb0HashKey = IniKeywords::Vb0;
        const std::string MeshVertexCountKey = IniKeywords::Global + " " + IniKeywords::WWMIMeshVertexCount;

        // What a ResGroupCollect files the blend's edit under -- this library's vocabulary, not an
        // `.ini` key, so it does not belong in IniKeywords
        const std::string BlendResGroupMember = "blend";

        // A WWMI Tools export's shape-key buffers -- see WWMIFixerConfig::shapeKeyOrder
        const std::vector<std::string> ShapeKeyResources = {
            IniKeywords::WWMIShapeKeyOffsetBuffer, IniKeywords::WWMIShapeKeyVertexIdBuffer, IniKeywords::WWMIShapeKeyVertexOffsetBuffer};



        const std::string Cb4HashKey = WWMINames::BoneDataHash;

        const std::string ShapeKeyZero = WWMINames::ShapeKeyZero;

        // Marks everything belonging to the SOURCE's own game texture for a role the mod also
        // serves -- the download, its edits and the list that binds them. See
        // the fix's "gameDefaults" step.
        const std::string GameSuffix = WWMINames::GameSuffix;
        const std::string ChecksumNotFound = WWMINames::ChecksumNotFound;

        // What a Texcoord.buf's bytes per vertex are taken to be when the resource declares
        // none of its own.
        constexpr std::size_t DefaultTexcoordStride = 16;

        // A BLEND LIFT THAT GIVES UP HAS TO SAY WHY (2026-09-29).
        //
        // `RemapService::_fixResource` returning false is recorded as neither fixed nor skipped and
        // logs nothing -- only a thrown exception carries a reason into the summary. So a bare
        // `return false` here is a `.buf` the fix decided it could not write, with no line anywhere
        // saying so, and the mod renders with a part missing.
        //
        // Every one of these is a shape check on the mod's own buffers, so the reason IS the
        // diagnosis. Returns false exactly as before; this only adds the line.
        bool bail(const RemapBlendResource& resource, const std::string& why) {
            // THROWN, not returned. `RemapService::_fixResource` returning false is a bare
            // `continue` -- the file is counted as neither fixed nor skipped and nothing is printed
            // -- where a thrown exception is recorded against the resource and its message appears
            // in the summary's per-file list, which is where "see log above" sends the user.
            // Measured by truncating a mod's Blend.buf: `fixed 0 ... and skipped 0`, for a file the
            // fix had just refused to write (2026-09-29).
            throw std::runtime_error(
                "cannot fix " + FileService::pathToStr(FileService::strToPath(resource.srcPath).filename())
                + ": " + why);
        }

        // A WWMIBlend step under bail's message: the library says why, this says which file
        template <typename Fn>
        bool orBail(const RemapBlendResource& resource, Fn&& step) {
            try {
                step();
            } catch (const std::exception& exception) {
                bail(resource, exception.what());
            }

            return true;
        }


        using AnchorAdd = RegAnchorAdd<>;
        using Kvp = std::pair<std::string, std::string>;
        using Kvps = std::vector<std::pair<std::string, std::string>>;
        using DrawRange = std::pair<long long, long long>;



        // A `run =` of the section 'sectionOf' names for each draw it knows, in place of the draw
        // (a rerouted draw, drawn by the reroute's own pixel shader -- see
        // WWMIFixerConfig::DrawReroute::shader -- or a draw suppressed on some passes, see
        // SourceComponent::offPasses) or right after it (SourceComponent::markLines). A draw it does
        // not name, or one with a base vertex, is left as it is.
        std::unique_ptr<AnchorAdd> drawRunAdd(std::map<DrawRange, std::string> sectionOf, AnchorAdd::Placement placement) {
            return std::make_unique<AnchorAdd>(
                std::vector<std::string>{IniKeywords::DrawIndexed},
                [sectionOf = std::move(sectionOf)](const std::string&, const std::string& value) -> std::optional<Kvps> {
                    const std::optional<DrawRange> range = IniParseData::drawRangeOf(value, true);
                    if (!range.has_value()) {
                        return std::nullopt;
                    }

                    const auto section = sectionOf.find(*range);
                    if (section == sectionOf.end()) {
                        return std::nullopt;
                    }

                    return Kvps{{IniKeywords::Run, section->second}};
                },
                placement);
        }


        // WWMIFixerConfig::Shader::after: a `run =` of 'list' right before each `run =` of one of the
        // anchors -- the cleanup list the section runs once after its draws, so the list runs once per
        // game draw however many ranges the mod draws the component as. The latch makes the list run
        // once per GAME draw however many of the mod's sections match it; `post` clears it once the
        // draw is made.
        std::unique_ptr<AnchorAdd> runBeforeRuns(std::vector<std::string> anchors, std::string list, std::string latch) {
            return std::make_unique<AnchorAdd>(
                std::vector<std::string>{IniKeywords::Run},
                [anchors = std::move(anchors), list = std::move(list), latch = std::move(latch)]
                (const std::string&, const std::string& value) -> std::optional<Kvps> {
                    const std::string run(StringTools::strip(value));
                    for (const std::string& anchor : anchors) {
                        if (StringTools::equalsIgnoreCase(run, anchor)) {
                            Kvps lines{{IniKeywords::Run, list}};
                            if (!latch.empty()) {
                                lines.emplace_back(IniKeywords::Post + " " + latch, "0");
                            }
                            return lines;
                        }
                    }

                    return std::nullopt;
                },
                AnchorAdd::Placement::Before);
        }


        /** @brief register -> the resource its previous binding is parked in -- see WWMISections::bypasses */
        using Bypasses = WWMISections::Bypasses;

        /**
         * @brief Whether 'name' is one of 'names', ignoring case -- a section filter of bypassRestore's
         */
        bool namesSection(const std::vector<std::string>& names, const std::string& name) {
            return std::any_of(names.begin(), names.end(), [&name](const std::string& one) {
                return StringTools::equalsIgnoreCase(one, name);
            });
        }

        // MAKING THE FIX'S CLEANUP LIST THE INVERSE OF ITS OVERRIDE LIST -- see
        // WWMIFixerConfig::cleanupResourcesList
        //
        // In the OVERRIDE list, each register is captured into a bypass resource immediately before
        // the line that rebinds it; in the CLEANUP list, each is bound back from that resource. The
        // pattern is WWMI's own for `vb0` (`ResourceBypassVB0 = ref vb0`, then
        // `vb0 = ref ResourceBypassVB0`), applied to the rest of what the list binds.
        //
        // It matters only on a remap. On the mod's own character every draw of that character is
        // matched by one of the mod's sections, so a buffer left bound is rebound before anything
        // reads it; a remapped mod is drawn on a target with slots it does not cover, and the first
        // such draw inherits the MOD's buffers at the TARGET's index range.

        /**
         * @brief
         * The edits that make the cleanup list the inverse of the override list, each with the names of
         * the sections it applies to: a capture of every bypassed register in the override list, and a
         * `run =` of the restore list at the end of the cleanup list
         *
         * The capture is `<bypass> = ref <reg>` just before the FIRST line that rebinds <reg>. The
         * restore is NOT the `<reg> = ref <bypass>` lines themselves: written into the cleanup list
         * they are part of the remapped graph, and the blend collect rewrites every line whose key is
         * its register -- which turned the blend's restore into a second binding of the fix's own
         * buffer. The capture half is safe where it is, because its key is a RESOURCE name and no
         * collect looks at those. Both skip a line the part already holds, so a list of several
         * branches gets one of each per branch and a repeated visit adds nothing.
         */
        std::vector<std::pair<std::unique_ptr<BaseRegEdit<>>, std::vector<std::string>>> bypassRestore(
                const std::vector<std::string>& overrideNames, const std::vector<std::string>& cleanupNames,
                const Bypasses& bypasses, const std::string& restoreList) {
            std::vector<std::pair<std::unique_ptr<BaseRegEdit<>>, std::vector<std::string>>> edits;
            for (const auto& [reg, bypass] : bypasses) {
                edits.emplace_back(
                    std::make_unique<AnchorAdd>(
                        std::vector<std::string>{reg},
                        [bypass = bypass, reg = reg](const std::string&, const std::string&) -> std::optional<Kvps> {
                            return Kvps{{bypass, IniKeywords::Ref + " " + reg}};
                        },
                        AnchorAdd::Placement::Before, nullptr, true, true),
                    overrideNames);
            }

            edits.emplace_back(std::make_unique<RegAdd<>>(Kvps{{IniKeywords::Run, restoreList}}, true, true), cleanupNames);
            return edits;
        }


        /**
         * @brief
         @rst
         What becomes of each of the mod's own texture lines in one component's remapped sections, as
         ``(register lowered, value stripped) -> the line in its place``: no line drops it, and a line the
         table does not name is left where it is :raw-html:`<br />` :raw-html:`<br />`

         A mod may bind its textures per DRAW rather than once per section, to give different parts
         different art. Those lines are copied into the remapped section and land after the fix's own
         ``run = CommandList<Char>Component<N>Textures``, so they win -- in the source's order, which is
         a role rotation on the target's shader. Re-keyed in place, the author's own ordering still
         decides who wins where two of them land on one register
         @endrst
         */
        using CarriedLines = std::map<std::pair<std::string, std::string>, std::optional<Kvp>>;

        /**
         * @brief
         @rst
         The edit that puts 'lines' in place: one :cpp:class:`RegAnchorAdd` over every line whose key
         starts with 'regPrefix'
         @endrst
         */
        std::unique_ptr<AnchorAdd> carriedTexRegs(const std::string& regPrefix, CarriedLines lines) {
            const std::string prefix = StringTools::toLower(regPrefix);
            return std::make_unique<AnchorAdd>(
                std::vector<std::string>{},
                [lines = std::move(lines)](const std::string& key, const std::string& value) -> std::optional<Kvps> {
                    const auto found = lines.find({StringTools::toLower(std::string(StringTools::strip(key))),
                                                   std::string(StringTools::strip(value))});
                    if (found == lines.end()) {
                        return std::nullopt;
                    }
                    return found->second.has_value() ? Kvps{*found->second} : Kvps{};
                },
                AnchorAdd::Placement::Replace,
                [prefix](const std::string& key) {
                    return StringTools::startsWith(StringTools::toLower(key), prefix);
                });
        }


        /**
         * @brief The pass names of one ``{pass -> bindings}`` table, in its own order
         *
         * @tparam ByPass The table type
         * @param byPass The table
         *
         * @return Its keys
         */
        template <typename ByPass>
        std::vector<std::string> passNamesOf(const ByPass& byPass) {
            std::vector<std::string> names;
            names.reserve(byPass.size());
            for (const auto& [pass, binds] : byPass) {
                (void)binds;
                names.push_back(pass);
            }

            return names;
        }



        // ---- one WWMI character, out of the library's tables ----

        struct Slot {
            std::string indexOffset;
            std::string indexCount;
            std::string vgOffset;
            std::string vgCount;
        };

        struct Character {
            std::string name;
            std::string vb0Hash;
            std::string cb4Hash;         // the game's bone-data constant buffer, which a legacy mod's merge is gated on
            std::vector<Slot> slots;
        };

        // Everything the fix needs to know about one character, read out of its ModType's tables
        // (the same shape WWMI-Assets' Metadata.json has). A missing row is an error, since a zero
        // here would look like a real value.
        std::optional<Character> readCharacter(const ModType& modType, const std::optional<Version>& version,
                                               const std::string& slotPrefix, std::string& error) {
            Character out;
            out.name = modType.name;
            if (modType.hashes == nullptr || modType.indices == nullptr) {
                error = "the library has no hash or index rows for " + modType.name;
                return std::nullopt;
            }

            std::optional<std::string> hash = modType.hashes->get({modType.name, Vb0HashKey}, version, false);
            if (!hash.has_value()) {
                error = "the library has no '" + Vb0HashKey + "' hash for " + modType.name;
                return std::nullopt;
            }

            out.vb0Hash = StringTools::toLower(*hash);
            out.cb4Hash = StringTools::toLower(modType.hashes->get({modType.name, Cb4HashKey}, version, false).value_or(""));
            for (std::size_t i = 0, slots = modType.getComponentCount(slotPrefix, version); i < slots; ++i) {
                const std::string slot = slotPrefix + std::to_string(i);
                std::optional<std::string> first = modType.indices->get({modType.name, "", slot}, version, false);
                std::optional<std::string> count = modType.getIndexCount(slot, "", version);
                std::optional<std::string> vgOffset = modType.getVGOffset(slot, "", version);
                std::optional<std::string> vgCount = modType.getVGCount(slot, "", version);
                if (!first.has_value() || !count.has_value() || !vgOffset.has_value() || !vgCount.has_value()) {
                    error = "the library has " + modType.name + "'s " + slot + " vg_offset but not all of its"
                            " match_first_index / match_index_count / vg_count";
                    return std::nullopt;
                }

                out.slots.push_back(Slot{*first, *count, *vgOffset, *vgCount});
            }

            if (out.slots.empty()) {
                error = "the library has no draw slots (VGOffsets rows typed " + slotPrefix + "N) for " + modType.name;
                return std::nullopt;
            }

            return out;
        }


        // ---- the mod's textures, by role ----
        //
        // Sorting a mod's files into roles is classification -- it reads only the source's own
        // textures -- so it lives where a parser reaches it too: `TextureRoles::Role`
        // (model/strategies/iniParsers/TextureRoles.h) and data/WWMITextureFacts.h.


        // ---- the fixer ----

        class WWMIFixerImpl: public TemplateFixer {
            // DECLARED FIRST, and it has to be: everything Z3 hands out belongs to this
            // context and must not outlive it, which member destruction order decides.
            Z3Context z3_;

            public:
                WWMIFixerImpl(BaseIniParser<>* parser, const std::string& toModName, std::optional<int> modTypeId,
                              WWMIFixerConfig config):
                    TemplateFixer(parser, toModName, modTypeId), config_(std::move(config)) {
                    this->copyPreamble = config_.copyPreamble;

                    std::string error;
                    const bool characters = readCharacters(error) && readGameData(error);
                    if (!characters || !readMod(error)) {
                        giveUp();
                        if (characters && buildWatcher()) {
                            return;
                        }

                        if (!error.empty()) {
                            ctx_.log(error + " -- so this .ini is left alone");
                        }
                        return;
                    }

                    // readMod has read what the mod draws (WWMIFixerConfig::drawnOf); readTextures gives up
                    // when the source has no texture roles
                    readTextures();
                    if (hasGivenUp()) {
                        return;
                    }

                    // THE FIX IS ITS LIST: the decisions, then the stages that write them (buildList). The fix's own sections are
                    // one step per kind, in this order
                    this->declareFixSectionKinds({"textureLists", "glowLists", "linkedState", "declaredFiles", "skeletonSections",
                                                  "blendRemapSections", "bypassSections", "hideSections", "stashResources", "passTags",
                                                  "sharedMeshes", "hiddenMeshes", "shaderDraws", "rabbitFXPinLists"});
                    buildList();

                    // The copies are the prototype's shape: each carries the fix's own sections (the
                    // texture command lists, the shader tags, the resources) and none of the mod's
                    // own draw sections live -- the mod's own file keeps serving the mod on its
                    // source character, and a copy exists only for one more claimant of a draw.
                    this->fixSectionsInCopies = true;
                    this->fixSectionsZ3 = &z3_;
                    // (The skeleton block re-declares the bone-data marker, which a mod that has its own
                    // already carries as a remapped section of the same name and text: a group renders a
                    // section of one name and text once -- see TemplateFixer::addFixSection.)
                    for (const std::string& section : ownSections()) {
                        if (StringTools::startsWith(section, IniKeywords::TextureOverride)) {
                            this->copyHiddenSectionNames.insert(section);
                        }
                    }
                }

            protected:
                // ---- THE FIX'S LIST ----
                //
                // What the fix decides is DATA, read off the mod and the config by WWMIFixerConfig's functions and held
                // in this fixer's members (decide). The list is then the edits that write it, each built with its data
                // and named (BaseIniGraphGroupEdit::name), in the order they run: the copies, the toggle lists, the slot
                // remap, the per-object edits by kind (ObjEditKinds), the blend and its rebind. A user reads and
                // rewrites the list whole (TemplateFixer::withList).
                void buildList() {
                    decide();
                    buildEdits();
                    buildAppended();
                    buildToggleLists();

                    this->graphGroupEdits.clear();
                    const auto add = [this](Fixer::GroupEdit* edit, const std::string& name) {
                        if (edit != nullptr) {
                            edit->name = name;
                            this->graphGroupEdits.push_back(edit);
                        }
                    };
                    for (Fixer::GroupEdit* edit : copyEdits_) {
                        add(edit, "copies");
                    }
                    for (Fixer::GroupEdit* edit : toggleEdits_) {
                        add(edit, "toggles");
                    }
                    add(slotRemap_.get(), "slotRemap");
                    for (const std::string& kind : ObjEditKinds) {
                        if (const auto found = kindEdits_.find(kind); found != kindEdits_.end()) {
                            add(found->second.get(), kind);
                        }
                    }
                    for (auto& collect : blendCollects_) {
                        add(collect.get(), "blend");
                    }
                    add(rebindEdits_.get(), "rebind");

                    // ...and the edits on the fix's own sections, which run after them (TemplateFixer::fixSectionEdits)
                    buildFixSectionEdits();
                }

                // THE FIX'S DECISIONS, from what the mod draws (readMod: WWMIFixerConfig::drawnOf) and the config:
                // which file each component goes in, what each role resolves to (reroutes reading their source's
                // files), the game's own textures, the texture edits' targets, what a linked .ini carries, RabbitFX's
                // flags, the texture lists, the mod's own texture lines, the blend width and the lines each section drops
                void decide() {
                    const IniParseData& data = parseData();
                    const std::string iniPath = ctx_.getIniFile()->getFile().value_or("");
                    const std::string iniFolder = ctx_.getIniFile()->getFolder();
                    const auto log = [this](const std::vector<std::string>& notes) {
                        for (const std::string& note : notes) {
                            ctx_.log(note);
                        }
                    };

                    const std::set<int> present = keysOf(drawnInfo_.present);
                    const std::set<int>& drawn = drawnInfo_.drawn;
                    const std::set<int>& rabbitFX = drawnInfo_.rabbitFX;
                    const std::set<int>& ownShader = drawnInfo_.ownShader;
                    const std::set<int>& overlaid = drawnInfo_.overlaid;

                    const WWMIFixerConfig::FileGroups groups = config_.fileGroups(present, source_.name);
                    log(groups.notes);
                    absorbGroups(groups);

                    WWMIFixerConfig::RoleResources roles = config_.roleResources(data, drawn, rabbitFX, source_.name, toModName_,
                                                                                 iniFolder);
                    log(roles.notes);
                    roles = WWMIFixerConfig::withReroutes(std::move(roles), drawnInfo_.rerouteFrom);
                    absorbRoles(roles);

                    const WWMIFixerConfig::GameDefaults defaults = config_.gameDefaults(data, roles, present, drawnInfo_.rerouteFrom, ownShader,
                                                                                        source_.name);
                    log(defaults.notes);
                    const WWMIFixerConfig::TexEditTargets edits = config_.texEditTargets(data, roles, present, ownShader, iniPath);
                    log(edits.notes);
                    absorbTexEditTargets(edits);
                    absorbGameDefaults(defaults);

                    const WWMIFixerConfig::LinkedLines linked = config_.linkedLines(data, roles, present, ctx_.getIniFile()->linkedInis(),
                                                                                    source_.name, toModName_, iniFolder);
                    log(linked.notes);
                    absorbLinkedLines(linked);

                    const std::map<int, std::array<int, 4>> flags = config_.rabbitFXFlags(data, roles, defaults.toggled, drawnInfo_.present,
                                                                                          drawnInfo_.rerouteFrom, iniPath, iniFolder);
                    absorbRabbitFX(flags);

                    const WWMIFixerConfig::TextureLists lists = config_.toggleLists(
                        config_.textureLists(data, roles, defaults, present, drawnInfo_.rerouteFrom, ownShader, overlaid), data, defaults,
                        source_.name, toModName_, iniPath);
                    absorbTextureLists(lists, config_.modTexLines(data, roles, lists, present, drawnInfo_.rerouteFrom, ownShader));

                    std::optional<VGRemap> row;
                    if (const ModType* source = ctx_.modType(); source != nullptr) {
                        row = source->getVGRemap(toModName_, fromVersion(), toVersion());
                    }
                    const WWMIFixerConfig::BlendWidth width = config_.blendWidth(row);
                    log(width.notes);
                    absorbBlendWidth(width);

                    std::set<int> components;
                    for (const std::vector<int>& group : groups.groups) {
                        components.insert(group.begin(), group.end());
                    }
                    absorbRemovals(config_.removedLines(width.targetPast256, components, keysOf(flags)));
                }

                template <typename V>
                static std::set<int> keysOf(const std::map<int, V>& map) {
                    std::set<int> keys;
                    for (const auto& entry : map) {
                        keys.insert(entry.first);
                    }
                    return keys;
                }

                // THE EDITS ON THE FIX'S OWN SECTIONS, run on every group once the slot remap has made
                // the copies and the fix's sections are in them (TemplateFixer::fixSectionEdits), in order:
                //
                //   1. the CustomShader files, collected off the shader graph   (shaderFileEdits)
                //   2. the fix's buffers, downloads and created textures         (fixFileEdits)
                //   3. the frame's skeleton remap, latched on $object_detected   (latchEdits)
                //   4. the texture edits, one collect per edit                   (addTexEditCollects)
                //
                // Steps 1-3 are IterGraphGroupEdits: a graph is named by its group's index, and the
                // number of groups is decided at fix time. Step 4 builds each edited texture once for
                // every group, so its collects run in every group at once (ResRegCollect::everyGroup).
                // Each step is a no-op on a group that has nothing for it.
                void buildFixSectionEdits() {
                    this->fixSectionEdits.clear();
                    fixSectionIters_.clear();
                    texEditCollects_.clear();
                    texEditors_.clear();
                    for (auto& [name, factory] : std::vector<std::pair<std::string, IterEdit::EditFactory>>{
                            {"shaderFiles", [this](std::size_t g, Fixer::GraphGroups& groups) { return shaderFileEdits(g, groups); }},
                            {"fixFiles", [this](std::size_t g, Fixer::GraphGroups& groups) { return fixFileEdits(g, groups); }},
                            {"latch", [this](std::size_t g, Fixer::GraphGroups& groups) { return latchEdits(g, groups); }}}) {
                        fixSectionIters_.push_back(std::make_unique<IterEdit>(std::move(factory)));
                        fixSectionIters_.back()->name = name;
                        this->fixSectionEdits.push_back(fixSectionIters_.back().get());
                    }
                    addTexEditCollects();
                }

                // THE CUSTOMSHADER SECTIONS ARE A GRAPH, AND THEIR FILES ARE ITS RESOURCES (2026-10-16).
                //
                // Every .ini file the fix writes declares the shaders (they used to ride the appended
                // text, which every copy carries), so the graph rerouteShaderSections built is added to
                // each group, and the files it names are collected off it as any other resource is:
                // `ps-cb4 = Resource...` through FileAddCreate, which builds the buffer's section and
                // the file, and `ps = <file>.hlsl` -- a register whose value IS the file -- through
                // FileRefAdd. Both hand the service RemapFileAddResources, so the files are written,
                // counted and undone with the rest of the mod's resources.
                //
                // In every group there is, the copies included: a copy's group exists only once the slot
                // remap has made it.
                IterEdit::GroupEdits shaderFileEdits(std::size_t g, Fixer::GraphGroups& groups) {
                    IterEdit::GroupEdits result;
                    const GraphId shaders = fixSectionsGraphId(g);
                    if (!hasShaders_ || groups.getGraph(g, shaders.modObj) == nullptr) {
                        return result;
                    }

                    auto bufferCreate = std::make_unique<FileAddCreate<>>(GraphId(g, "", "rerouteShaderBuffers"), shaderBuffers_, resEditConfig());
                    auto sourceAdd = std::make_unique<FileRefAdd<>>(shaderSources_, resEditConfig());

                    // a `ref` names a buffer the fix parks a binding in (see restoringRuns), not a file
                    const Collector::ResPredicate namesFile = Collector::skipsKeywords();
                    // ONE collect over every register a shader's constants are bound at (the tattoo's ps-cb1 and the
                    // overlay's ps-cb4): a second collect sharing bufferCreate would build no resource of its own, and
                    // its binding named a buffer nothing declared (ResRegCollect::extraSrcRegs)
                    if (!shaderBufferRegs_.empty()) {
                        auto buffers = std::make_unique<Collector>();
                        buffers->srcRegs[shaders] = *shaderBufferRegs_.begin();
                        buffers->extraSrcRegs[shaders] = std::vector<std::string>(std::next(shaderBufferRegs_.begin()), shaderBufferRegs_.end());
                        buffers->resPredicates[shaders] = namesFile;
                        buffers->resEdits = {{"shaderBuffer", bufferCreate.get()}};
                        result.edits.push_back(std::move(buffers));
                    }

                    auto sources = std::make_unique<Collector>();
                    sources->srcRegs[shaders] = IniKeywords::Ps;
                    sources->resPredicates[shaders] = namesFile;
                    sources->resEdits = {{"shaderSource", sourceAdd.get()}};
                    result.edits.push_back(std::move(sources));

                    result.resEdits.push_back(std::move(bufferCreate));
                    result.resEdits.push_back(std::move(sourceAdd));
                    return result;
                }

                using ContentPart = IfContentPart<std::string, std::string>;

            private:
                // A FILE THAT ONLY WATCHES THE SOURCE (2026-10-13). A help overlay, a key menu or a skill
                // effect's recolour sits beside the mesh and matches the source's vb0 only to set a variable
                // while she is on screen (`$active = 1`, `$\CECChange\activeVB = 1`): no index window, nothing
                // drawn. The fixer used to leave it alone, so on the target it never fired -- Lynae1's F5
                // help image and skill-effect recolour, Lynae13's and Lynae14's key help. Each such section
                // is copied, under the fix's name, onto the TARGET's vb0, and nothing is drawn or
                // downloaded: what the GI merge's buildWatcher does. A file that also draws a component is
                // not a watcher (readMod found it), and its own watching sections ride along as written.
                bool buildWatcher() {
                    IniFile* ini = ctx_.getIniFile();
                    if (ini == nullptr || target_.vb0Hash.empty()) {
                        return false;
                    }

                    const ModObj watch("", WWMIParseObjs::Watch);
                    const std::vector<std::string> watching = sectionsOf(watch);
                    if (watching.empty()) {
                        return false;
                    }

                    // each section as the author wrote it, under the fix's name and the target's vb0 -- its
                    // conditions are the .ini's own -- copied before giveUp's removal of every group
                    this->fixSectionsZ3 = ini->getZ3Ctx();
                    const std::vector<Fixer::GroupEdit*> copies = copyOntoHash(
                        watch, watching, target_.vb0Hash, [](const std::string&) { return std::string(); }, false);
                    this->graphGroupEdits.insert(this->graphGroupEdits.begin(), copies.begin(), copies.end());

                    ctx_.log("only watches " + source_.name + ", so its " + std::to_string(watching.size())
                             + " watching section(s) are carried onto " + toModName_ + "'s vb0 and nothing is drawn");
                    return true;
                }

                // The sections the parser placed in its graph 'obj', in the order it placed them
                std::vector<std::string> sectionsOf(const ModObj& obj) const {
                    auto* parser = dynamic_cast<GIMIParser<>*>(this->getParser());
                    if (parser == nullptr) {
                        return {};
                    }

                    const auto found = parser->sectionTargets().find(obj);
                    return found == parser->sectionTargets().end() ? std::vector<std::string>() : found->second;
                }

                // THE MOD'S SECTIONS ON ANOTHER HASH, BY EDITS: each of 'sections' of the parser's graph 'obj'
                // copied into a graph of its own keeping only that section's lines (GraphCopy), renamed to the
                // fix's name (GraphRename), its every `hash` set to 'hash' (RegNewVals) and taken into the fix
                // (TemplateFixer::takeFixSections) under the comment 'commentOf' gives it -- in this call's place
                // among the fix's sections when 'inPlace'. The edits, in the order they run
                std::vector<Fixer::GroupEdit*> copyOntoHash(const ModObj& obj, const std::vector<std::string>& sections,
                                                             const std::string& hash,
                                                             const std::function<std::string(const std::string&)>& commentOf,
                                                             bool inPlace) {
                    std::vector<Fixer::GroupEdit*> copies;
                    std::vector<Fixer::GroupEdit*> takes;
                    std::vector<ObjGroupEdit::IniEdits> perGroup(1);
                    for (const std::string& section : sections) {
                        const std::string name = remapFixName(section);
                        const ModObj copyObj("", name);
                        auto copy = std::make_unique<GraphCopy<>>(
                            GraphId(0, obj.first, obj.second), GraphId(0, copyObj.first, copyObj.second),
                            [section](const IfTemplate<std::string, std::string>& owner, const IfContentPart<std::string, std::string>&,
                                      std::size_t) {
                                return owner.name == section;
                            });
                        copies.push_back(copy.get());
                        toggleOwned_.push_back(std::move(copy));

                        auto rename = std::make_unique<GraphRename<>>([section, name](const std::string& old) {
                            return old == section ? name : old;
                        });
                        auto onHash = std::make_unique<RegNewVals<>>(
                            std::vector<std::pair<std::string, RegNewVals<>::NewValSpec>>{{std::string(IniKeywords::Hash), RegNewVals<>::NewVal(hash)}});

                        std::vector<ObjGroupEdit::PartEdit*>& edits = perGroup[0].edits[copyObj];
                        edits.push_back(adapt(std::make_unique<GraphPartEdit<>>(rename.get())));
                        edits.push_back(adapt(std::make_unique<RegPartEdit<>>(onHash.get())));
                        toggleOwned_.push_back(std::move(rename));
                        toggleOwned_.push_back(std::move(onHash));

                        takes.push_back(this->takeFixSections(copyObj, {name}, commentOf(section), inPlace));
                    }

                    if (sections.empty()) {
                        return copies;
                    }

                    auto edits = std::make_unique<ObjGroupEdit>(std::move(perGroup), false);
                    copies.push_back(edits.get());
                    toggleOwned_.push_back(std::move(edits));
                    copies.insert(copies.end(), takes.begin(), takes.end());
                    return copies;
                }

            private:
                // THE REMAP RUNS ONCE A FRAME, AT THE FIRST FIX SECTION THAT DRAWS (2026-10-06).
                //
                // WWMI's own design binds every draw of a frame ONE complete skeleton, the one its
                // [Present] snapshots: the whole character is a frame behind, uniformly. The remap used
                // to run at the top of every slot's merge list instead, from a merge buffer holding some
                // windows from this frame and the rest from the last -- so a slot's first pass skinned
                // its own bones a frame late, its later passes did not, and bones of other slots
                // depended on draw order. Still, nothing differs; in motion the parts of one body are
                // drawn a frame apart, which TAA leaves as a dark shell trailing her (reported on
                // Lynae1 as "a second shadow ghost body, only when she moves").
                //
                // The host flags its own frames: every section sets `$object_detected = 1` and its
                // [Present] resets it with `post`. So a fix section that runs a merge list remaps
                // first when the flag is still 0 -- the frame's first -- and the rest of the frame
                // binds that one result. A section that runs a merge list without setting the flag sets it
                // here, before its first `if`. Only for a host whose [Present] does the
                // reset: without it the flag would stay 1 and the remap would never run again.
                //
                // Two RegIfTemplateAdds over every section of every graph, after every other edit (the
                // copies exist then); the appended sections that merge carry the latch as they are built.
                bool latchesRemap() const {
                    return targetPast256_ && drawnInfo_.hostResetsDetected;
                }

                // `if $object_detected == 0` / `run = <the skeleton remap>` / `endif`, then `$object_detected = 1`
                // when 'setsDetected'
                std::unique_ptr<IfTemplate<std::string, std::string>> latchTemplate(bool setsDetected) {
                    std::vector<std::unique_ptr<IfTemplatePart>> parts;
                    parts.push_back(std::make_unique<IfPredPart>("if " + IniKeywords::WWMIObjectDetected + " == 0", IfPredPartType::If, z3_));
                    parts.push_back(std::make_unique<ContentPart>(
                        Kvps{{IniKeywords::Run, remapFixName(WWMINames::RemapMergedSkeletonList)}}, 1));
                    parts.push_back(std::make_unique<IfPredPart>("endif", IfPredPartType::EndIf, z3_));
                    if (setsDetected) {
                        parts.push_back(std::make_unique<ContentPart>(Kvps{{IniKeywords::WWMIObjectDetected, "1"}}, 0));
                    }

                    std::unique_ptr<IfTemplate<std::string, std::string>> latch = makeSection("", {});
                    latch->parts() = std::move(parts);
                    latch->rebuild();
                    return latch;
                }

                IterEdit::GroupEdits latchEdits(std::size_t g, Fixer::GraphGroups& groups) {
                    IterEdit::GroupEdits result;
                    if (!latchesRemap()) {
                        return result;
                    }

                    if (beforeDetected_ == nullptr) {
                        std::unordered_set<std::string> merges = {remapFixName(WWMINames::MergeWindowList)};
                        for (std::size_t slot = 0; slot < target_.slots.size(); ++slot) {
                            merges.insert(mergeListName(static_cast<int>(slot)));
                        }

                        const auto runsMerge = [merges](const IfTemplate<std::string, std::string>& section) {
                            for (const std::unique_ptr<IfTemplatePart>& part : section.parts()) {
                                const auto* content = dynamic_cast<const ContentPart*>(part.get());
                                for (std::size_t k = 0; content != nullptr && k < content->size(); ++k) {
                                    const auto& [key, value] = content->getByInd(static_cast<long long>(k));
                                    if (key == IniKeywords::Run && merges.count(std::string(StringTools::strip(value))) > 0) {
                                        return true;
                                    }
                                }
                            }

                            return false;
                        };

                        // a section two graphs share is latched once, as its rendered text was
                        const auto once = [this, runsMerge](const std::string&, const IfTemplate<std::string, std::string>& section) {
                            return runsMerge(section) && latched_.insert(&section).second;
                        };

                        latch_ = latchTemplate(false);
                        latchAndSet_ = latchTemplate(true);
                        beforeDetected_ = std::make_unique<RegIfTemplateAdd<>>(latch_.get(), RegIfTemplateAdd<>::Position::BeforeTopKey,
                                                                               IniKeywords::WWMIObjectDetected);
                        beforeDetected_->value = "1";
                        beforeDetected_->everySection = true;
                        beforeDetected_->sectionFilter = once;
                        beforeIf_ = std::make_unique<RegIfTemplateAdd<>>(latchAndSet_.get(), RegIfTemplateAdd<>::Position::BeforeFirstIf);
                        beforeIf_->everySection = true;
                        beforeIf_->sectionFilter = once;
                        beforeDetectedAdapter_ = std::make_unique<GraphPartEdit<>>(beforeDetected_.get());
                        beforeIfAdapter_ = std::make_unique<GraphPartEdit<>>(beforeIf_.get());
                    }

                    latched_.clear();
                    for (GraphPartEdit<>* adapter : {beforeDetectedAdapter_.get(), beforeIfAdapter_.get()}) {
                        std::vector<ObjGroupEdit::IniEdits> edits(g + 1);
                        for (const ModObj& obj : groups.modObjs(g)) {
                            edits[g].edits[obj] = {adapter};
                        }
                        result.edits.push_back(std::make_unique<ObjGroupEdit>(std::move(edits)));
                    }

                    return result;
                }

            private:

                ModObj slotObj(int component) const {
                    const auto alias = drawnInfo_.rerouteFrom.find(component);
                    if (alias != drawnInfo_.rerouteFrom.end()) {
                        component = alias->second;
                    }

                    return ModObj("", WWMINames::SlotPrefix + std::to_string(component));
                }

                // The version WWMIFixerConfig::sourceVersionByVb0 files the mod's own vb0 under, read
                // off its slot sections -- see that field. Read once: the sections do not change
                // during a fix, and this is asked before readMod has collected them.
                std::optional<std::string> versionOfModVb0() const {
                    if (modVb0Version_.has_value()) {
                        return *modVb0Version_;
                    }

                    modVb0Version_ = std::optional<std::string>();
                    if (this->getParser() == nullptr) {
                        return std::nullopt;
                    }

                    for (const std::string& section : ownSections()) {
                        if (!ownVal(section, IniKeywords::MatchFirstIndex).has_value()) {
                            continue;
                        }

                        const std::optional<std::string> hash = ownVal(section, IniKeywords::Hash);
                        if (!hash.has_value()) {
                            continue;
                        }

                        const auto found = config_.sourceVersionByVb0.find(StringTools::toLower(*hash));
                        if (found != config_.sourceVersionByVb0.end()) {
                            modVb0Version_ = found->second;
                            break;
                        }
                    }

                    return *modVb0Version_;
                }

                std::optional<Version> fromVersion() const {
                    std::optional<Version> version = ctx_.version();
                    if (!version.has_value() && !config_.sourceVersionByVb0.empty()) {
                        if (const std::optional<std::string> byVb0 = versionOfModVb0()) {
                            version = Version::parse(*byVb0);
                        }
                    }

                    if (!version.has_value()) {
                        // the SOURCE's own version, which is not the target's when the pair is not
                        // filed under one -- see WWMIFixerConfig::sourceVersion for what a
                        // reverse-then-forward lookup does with the wrong one
                        version = Version::parse(config_.sourceVersion.empty() ? config_.version
                                                                               : config_.sourceVersion);
                    }

                    return version;
                }

                std::optional<Version> toVersion() const {
                    const IniFile* ini = ctx_.getIniFile();
                    if (ini != nullptr && ini->toVersion.has_value()) {
                        return ini->toVersion;
                    }
                    if (!config_.version.empty()) {
                        return Version::parse(config_.version);
                    }

                    // the target's newest model, or a newer game version her draw passes are filed at
                    const std::optional<ModType> target = ModTypeIdTools::getModType(static_cast<int>(config_.targetId));
                    return target.has_value() ? WWMIFixerConfig::newestVersionOf(*target) : std::nullopt;
                }

                // ---- the two characters ----

                bool readCharacters(std::string& error) {
                    const ModType* source = ctx_.modType();
                    if (source == nullptr) {
                        error = "the fixer has no source mod type";
                        return false;
                    }

                    targetType_ = ModTypeIdTools::getModType(static_cast<int>(config_.targetId));
                    if (!targetType_.has_value()) {
                        error = "the library has no mod type for the target";
                        return false;
                    }

                    std::optional<Character> src = readCharacter(*source, fromVersion(), WWMINames::SlotPrefix, error);
                    if (!src.has_value()) {
                        return false;
                    }

                    std::optional<Character> dst = readCharacter(*targetType_, toVersion(), WWMINames::SlotPrefix, error);
                    if (!dst.has_value()) {
                        return false;
                    }

                    source_ = std::move(*src);
                    target_ = std::move(*dst);
                    return true;
                }

                // The config's game data, off the versioned asset tables at this run's versions (WWMIFixerConfig::withGameData):
                // the source's vb0 versions first, since the source's version is read off the mod's vb0
                bool readGameData(std::string& error) {
                    try {
                        if (config_.sourceVersionFromVb0) {
                            config_.sourceVersionByVb0 = WWMIFixerConfig::vb0VersionsOf(*ctx_.modType());
                            modVb0Version_.reset();
                        }
                        config_ = config_.withGameData(*ctx_.modType(), *targetType_, fromVersion(), toVersion());
                    } catch (const std::exception& e) {
                        error = e.what();
                        return false;
                    }
                    return true;
                }

                // ---- what the mod's .ini has: its slot sections, its vertex count, its folders ----

                bool readMod(std::string& error) {
                    IniFile* ini = ctx_.getIniFile();
                    if (ini == nullptr) {
                        error = "the fixer has no .ini file";
                        return false;
                    }

                    // THE MOD'S ANATOMY IS THE PARSER'S: which section is which source component, what each draws,
                    // which components the mod hides, the reroutes, and which components keep RabbitFX or are drawn by
                    // their own shader -- WWMIFixerConfig::drawnOf, over the parser's data and graphs. A fixer is built
                    // after its parser has parsed.
                    const IniParseData& data = parseData();
                    WWMIFixerConfig::SectionTargets targets;
                    if (auto* parser = dynamic_cast<GIMIParser<>*>(this->getParser())) {
                        targets = IniParseData::sectionTargetsOf(parser->sectionTargets());
                    }
                    drawnInfo_ = config_.drawnOf(data, targets, static_cast<int>(source_.slots.size()), source_.name,
                                                 source_.vb0Hash, toModName_);
                    if (!drawnInfo_.error.empty()) {
                        error = drawnInfo_.error;
                        return false;
                    }

                    // the one part of what the mod draws a later step changes: settleOwnMeshes drops own meshes
                    ownMeshes_ = drawnInfo_.ownMeshes;
                    rerouteShaders_.clear();
                    overlays_.clear();
                    markLines_.clear();
                    for (const int component : drawnInfo_.ownShader) {
                        rerouteShaders_[component] = &*config_.plan.at(component).shader;
                    }
                    for (const int component : drawnInfo_.overlaid) {
                        overlays_[component] = &*config_.plan.at(component).overlay;
                    }
                    for (const int component : drawnInfo_.markLines) {
                        markLines_[component] = &config_.plan.at(component).markLines;
                    }
                    for (const std::string& note : drawnInfo_.notes) {
                        ctx_.log(note);
                    }

                    if (const auto* row = data.firstCollected(IniParseRows::Declared, IniKeywords::Constants, MeshVertexCountKey)) {
                        const std::optional<std::string> count = std::string(StringTools::strip(row->value));
                        {
                            try {
                                meshVertexCount_ = std::stoll(*count);
                            } catch (const std::exception&) {
                                // 0 is not a harmless fallback: it turns the zero shape-key stream
                                // OFF (no stream is made for a count <= 0), which is the wavy-vertices
                                // fault that stream exists to remove.
                                ctx_.log("this mod's " + MeshVertexCountKey + " is `" + *count
                                     + "`, which is not a number, so the zero shape-key stream is"
                                     + " not written");
                                meshVertexCount_ = 0;
                            }
                        }
                    }

                    // THE MOD'S SHAPE-KEY BUFFERS, for WWMIFixerConfig::shapeKeyOrder. Read and checked
                    // here, because the edit that binds the reordered copies is built from what this
                    // finds: a copy that could not be written must never be bound.
                    shapeKeyFiles_.clear();
                    if (!config_.shapeKeyOrder.empty()) {
                        for (const std::string& resource : ShapeKeyResources) {
                            const auto declared = [&data, &resource](const std::string& key) {
                                return data.firstCollected(IniParseRows::Declared, resource, key);
                            };
                            const auto valueOf = [&declared](const std::string& key, const std::string& otherwise) {
                                const auto* row = declared(key);
                                return row == nullptr ? otherwise : std::string(StringTools::strip(row->value));
                            };

                            const auto* filename = declared(IniKeywords::Filename);
                            if (filename == nullptr) {
                                continue;
                            }

                            ShapeKeyFile file;
                            file.section = filename->section;
                            file.path = std::string(StringTools::strip(filename->value));
                            file.type = valueOf(IniKeywords::Type, IniKeywords::Buffer);
                            file.format = valueOf(IniKeywords::Format, "");
                            file.stride = valueOf(IniKeywords::Stride, "");
                            if (!file.path.empty()) {
                                shapeKeyFiles_[resource] = file;
                            }
                        }

                        if (shapeKeyFiles_.size() != ShapeKeyResources.size()) {
                            shapeKeyFiles_.clear();
                        } else if (std::string why = WWMIShapeKeys::checkReorder(shapeKeyFiles(false), config_.shapeKeyOrder); !why.empty()) {
                            ctx_.log("this mod's shape keys are left in " + source_.name + "'s order (" + why
                                 + "), so the target's expressions drive the wrong ones");
                            shapeKeyFiles_.clear();
                        }
                    }

                    // Beside the .ini until the mod says otherwise -- see modFile. A mod whose
                    // buffers sit next to its .ini (Chisa12: GUID names, `.assets` buffers) had a
                    // `Meshes/` built for it holding nothing but the fix's own output.
                    meshFolder_.clear();
                    {
                        std::optional<std::string> file = data.resourceFile(IniKeywords::WWMIBlendBuffer);
                        if (file.has_value()) {
                            const std::string forward = FileService::iniPathToRel(*file);

                            // `parentOf` answers "" for a file with no folder part, which is the
                            // mod root -- exactly what meshFolder_ means by empty.
                            meshFolder_ = FileService::parentOf(forward);

                            // Kept whole, because the name of the blend the FIX writes is derived
                            // from it and not from the character -- see blendFixedFile().
                            blendSourceFile_ = *file;

                            // The mod's OWN blend line, derived from its own file rather than the
                            // `stride` it declares: a declaration can disagree with the bytes, and
                            // this is the number WWMI's BlendRemapper is handed.
                            const std::string blendPath =
                                FileService::absPathOfRelPath(forward, ctx_.getIniFile()->getFolder());
                            const std::string positionPath =
                                positionBufPath();
                            srcBlendInfluences_ = WWMIBlend::influencesOf(blendPath, meshVertexCount_, positionPath);
                            // ...and what the fix WRITES, which is the target's when its draws read
                            // a different number (WWMIFixerConfig::targetInfluences)
                            blendInfluences_ = config_.targetInfluences != 0 ? config_.targetInfluences
                                                                             : srcBlendInfluences_;
                            blendStride_ = blendInfluences_ * 2;
                        }
                    }

                    return true;
                }

                // ---- the textures ----

                // What the mod's textures are is the parser's answer (IniParseData::textureRoles): a texture is
                // identified by a hash and a register, which is section classification. Without it the fix gives up,
                // since every role would bind the game's texture. The fix's own textures -- a download, a created
                // texture -- go beside the .ini.
                void readTextures() {
                    const TextureRoles* roles = nullptr;
                    if (this->getParser() != nullptr) {
                        roles = this->getParser()->parseData().textureRoles();
                    }

                    if (roles == nullptr) {
                        // Every WuWa character's parse row carries its textures, so this is a
                        // configuration error rather than a mod's doing -- and a fix that cannot
                        // tell what any texture is would bind the game's over all of them.
                        giveUp("this mod's textures were not identified: the character's parse row "
                               "carries no textures, so every role would fall back to a download");
                    }
                }

                // WHAT EACH ROLE RESOLVES TO is the config's decision (WWMIFixerConfig::roleResources), from the
                // parser's data: the mod's file per (role, component) and the resource binding it, the resources the
                // fix declares for the mod's files, the stand-ins, and the roles left to the game
                void absorbRoles(const WWMIFixerConfig::RoleResources& roles) {
                    resourceOfSlotRole_ = roles.ofSlotRole;
                    fileOfRole_.clear();
                    fileOfRole_.insert(roles.fileOfRole.begin(), roles.fileOfRole.end());
                    resourceOfRole_ = roles.ofRole;
                    declaredName_.clear();
                    declared_.clear();
                    for (const WWMIFixerConfig::DeclaredFile& declared : roles.declared) {
                        declaredName_[declared.file] = declared.resource;
                        declared_.emplace_back(declared.file, declared.rel);
                    }
                    fallbacks_.clear();
                    for (const auto& [role, download] : roles.downloads) {
                        fallbacks_[role] = Fallback{download.url, download.file, download.file, download.resource};
                    }
                }

                // The name an edit of the mod's OWN texture is written under is per role, so two mesh .ini files in one
                // folder (an outfit pack: two variants sharing one Textures folder) wrote their edits to the
                // same files, and whichever was fixed last drew in both. When another .ini the game loads
                // there overrides one of the hashes this one draws a mesh of (a section with
                // `match_first_index`), this file's edits are tagged with its own name. A lone mod and a
                // folder whose other files only watch or recolour keep the plain names; a DISABLED variant
                // is not loaded, so the enabled file keeps its names and the disabled one tags its own.
                // (Edits of downloads, created textures and downloads are the same bytes from every file.)
                const std::string& editFileTag() {
                    if (editTagDone_) {
                        return editTag_;
                    }
                    editTagDone_ = true;

                    const std::string self = ctx_.getIniFile()->getFile().value_or("");
                    if (self.empty()) {
                        return editTag_;
                    }

                    IniFile* selfIni = ctx_.getIniFile();
                    const std::set<std::string> own = meshHashesIn(*selfIni);
                    if (own.empty()) {
                        return editTag_;
                    }

                    bool shared = false;
                    const std::filesystem::path selfPath = FileService::strToPath(self);
                    // the other files of the run that write into this one's folder, as they read with no fix in them:
                    // files of one folder are where two edits' names collide
                    const std::vector<std::string> runFiles = selfIni->sharedState == nullptr
                        ? std::vector<std::string>{} : selfIni->sharedState->files();
                    for (const std::string& other : runFiles) {
                        IniFile* otherIni = selfIni->sharedState->file(other);
                        const std::filesystem::path otherPath = FileService::strToPath(other);
                        std::error_code same;
                        if (otherIni == nullptr || otherPath.parent_path() != selfPath.parent_path()
                                || std::filesystem::equivalent(otherPath, selfPath, same)) {
                            continue;
                        }
                        for (const std::string& hash : meshHashesIn(*otherIni)) {
                            if (own.count(hash) > 0) {
                                shared = true;
                                break;
                            }
                        }
                        if (shared) {
                            break;
                        }
                    }
                    if (!shared) {
                        return editTag_;
                    }

                    // the file's name, letters and digits only
                    const std::string stem = FileService::pathToStr(FileService::strToPath(self).stem());
                    for (const char c : stem) {
                        if (std::isalnum(static_cast<unsigned char>(c))) {
                            editTag_ += c;
                        }
                    }
                    if (editTag_.empty()) {
                        editTag_ = "Ini";
                    }
                    editTag_[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(editTag_[0])));
                    ctx_.log("another .ini in this folder draws the same mesh: this file's texture edits are tagged "
                             + editTag_);
                    return editTag_;
                }

                // The hashes of the sections of an .ini that draw a mesh (they carry `match_first_index`)
                static std::set<std::string> meshHashesIn(IniFile& ini) {
                    std::set<std::string> result;
                    for (const auto& [name, tpl] : ini.getIfTemplates()) {
                        if (tpl == nullptr || !tpl->firstValMatching([](const std::string& key) { return IniNamingTools::sameKey(key, IniKeywords::MatchFirstIndex); }).has_value()) {
                            continue;
                        }

                        const std::optional<std::string> hash = tpl->firstValMatching([](const std::string& key) { return IniNamingTools::sameKey(key, IniKeywords::Hash); });
                        if (hash.has_value() && !hash->empty()) {
                            result.insert(StringTools::toLower(*hash));
                        }
                    }

                    return result;
                }

                // THE MOD'S OWN LINES ARE THE PARSER'S ROWS (IniParseRows::Sections): every key of every section, each
                // line once, in the order it is written (IniParseData::sectionRows)
                using Row = IniParseData::CollectedRow;

                // The sections, in the order they are written
                const std::vector<std::string>& ownSections() const {
                    return parseData().collectedSections(IniParseRows::Sections);
                }

                // The section's own lines, in order; empty for a section with none
                const std::vector<const Row*>& ownRows(const std::string& section) const {
                    return parseData().sectionRows(IniParseRows::Sections, section);
                }

                // The first value of 'key' in the section's own lines, stripped
                std::optional<std::string> ownVal(const std::string& section, const std::string& key) const {
                    for (const Row* row : ownRows(section)) {
                        if (row->key == key) {
                            return std::string(StringTools::strip(row->value));
                        }
                    }
                    return std::nullopt;
                }

                // ---- the edits ----

                // ---- what a LINKED .ini sets on this file's draws (IniParseData::linkedLines) ----

                // Each line the parser carried, on a component the mod draws: its condition (reading the fix's copies of
                // the linked file's own variables), and its value naming the file through a resource of this .ini --
                // the mod's own, or one the fix declares as a RemapRef so an undo leaves the file. A condition on the
                // linked file's own variables reads the fix's copy of the state that sets them (copyLinkedState): a
                // file with no namespace names its variables after its path, and this one cannot read them.
                void absorbLinkedLines(const WWMIFixerConfig::LinkedLines& linked) {
                    linkedLines_.clear();
                    for (const WWMIFixerConfig::LinkedState& state : linked.states) {
                        std::set<std::string> vars = state.vars;
                        copyLinkedState(*linkedIni(state.ini), vars, state.suffix);
                    }
                    for (const WWMIFixerConfig::DeclaredFile& declared : linked.declared) {
                        declaredName_.emplace(declared.file, declared.resource);
                        declared_.emplace_back(declared.file, declared.rel);
                    }
                    for (const auto& [component, lines] : linked.lines) {
                        for (const WWMIFixerConfig::CarriedLine& line : lines) {
                            linkedLines_[component].push_back(CarriedLine{line.condition, line.key, line.role, line.value});
                        }
                    }
                }

                // The linked file 'path' names, among the run's files linked to this one
                IniFile* linkedIni(const std::string& path) const {
                    for (IniFile* linked : ctx_.getIniFile()->linkedInis()) {
                        if (linked != nullptr && linked->getFile().value_or("") == path) {
                            return linked;
                        }
                    }
                    return nullptr;
                }

                // THE STATE A LINKED FILE KEEPS FOR 'vars' (IniFile::varState), copied into the fix under the variables'
                // new names (GraphVarRename) -- a section other than [Constants] and [Present] under a name of the fix's
                // own too. 'vars' grows to the state's closure, and a variable the file never declares is declared, as
                // 3DMigoto starts it, at 0
                void copyLinkedState(IniFile& linked, std::set<std::string>& vars, const std::string& suffix) {
                    IniFile::VarState state = linked.varState(vars);
                    vars = state.vars;

                    std::map<std::string, std::string> renames;
                    for (const std::string& var : vars) {
                        renames[var] = var + suffix;
                    }
                    for (std::unique_ptr<Section>& section : state.sections) {
                        GraphVarRename<>::renameIn(*section, renames, &z3_);
                        if (!StringTools::equalsIgnoreCase(section->name, IniKeywords::Constants)
                                && !StringTools::equalsIgnoreCase(section->name, IniKeywords::Present)) {
                            section->name = remapFixName(section->name);
                        }
                        addFixSection("linkedState", std::move(section));
                    }

                    std::vector<Kvp> undeclared;
                    for (const std::string& var : state.undeclared) {
                        undeclared.emplace_back(IniKeywords::Global + " $" + renames.at(var), "0");
                    }
                    if (!undeclared.empty()) {
                        addFixSection("linkedState", SectionText(z3_, IniKeywords::Constants).keys(undeclared).build());
                    }
                }

                // ---- a component drawn from the mod's OWN mesh files (WWMIFixerConfig::OwnMesh, read by drawnOf) ----

                // After the blend width: only the path with the fix's own merge list binds the target's
                // skeleton for such a draw -- otherwise its own list keeps binding the SOURCE's -- so elsewhere
                // the component is left as it was, and said so. Each kept one's blend is a fix file, remapped
                // into the target's local ids (WWMIBlend::componentBlend) through the row and its anchor chains,
                // as the main blend is
                void settleOwnMeshes(const ModType* source, const std::optional<Version>& from, const std::optional<Version>& to) {
                    // a component the plan drops (Changli's tattoo) is drawn by nothing: no blend for it
                    for (auto it = ownMeshes_.begin(); it != ownMeshes_.end();) {
                        it = config_.plan.count(it->first) > 0 ? std::next(it) : ownMeshes_.erase(it);
                    }

                    if (ownMeshes_.empty()) {
                        return;
                    }

                    if (!(targetPast256_ || drawnInfo_.legacy)) {
                        for (const auto& [component, mesh] : ownMeshes_) {
                            ctx_.log("source component " + std::to_string(component) + " draws from its own mesh ("
                                     + mesh.list + "), which this fix remaps only onto a target past 256 bones");
                        }
                        ownMeshes_.clear();
                        return;
                    }

                    const std::optional<VGRemap> row = source == nullptr ? std::nullopt : source->getVGRemap(toModName_, from, to);
                    if (!row.has_value()) {
                        ownMeshes_.clear();
                        return;
                    }

                    std::unordered_map<long long, long long> remap = row->getRemap();
                    for (const auto& [root, members] : config_.anchorChains) {
                        const auto at = remap.find(root);
                        if (at == remap.end()) {
                            continue;
                        }
                        for (long long member : members) {
                            remap[member] = at->second;
                        }
                    }
                    const VGRemap anchored(std::move(remap));

                    const std::string folder = ctx_.getIniFile()->getFolder();
                    // by value: the fix file is written after this returns
                    const auto pathOf = [folder](const std::string& rel) {
                        return FileService::absPathOfRelPath(FileService::iniPathToRel(rel), folder);
                    };
                    for (auto it = ownMeshes_.begin(); it != ownMeshes_.end();) {
                        const int component = it->first;
                        const OwnMesh mesh = it->second;
                        const auto vgMap = config_.sourceVgMaps.find(component);
                        const std::optional<WWMIBlend::Layout> layout = WWMIBlend::layoutOf(pathOf(mesh.blendFile), pathOf(mesh.positionFile));
                        if (!layout.has_value() || vgMap == config_.sourceVgMaps.end()) {
                            ctx_.log("could not remap the own-mesh blend of source component " + std::to_string(component)
                                     + (!layout.has_value() ? ": its blend and position buffers do not agree on a vertex count"
                                                            : ": the config carries no sourceVgMaps for it"));
                            it = ownMeshes_.erase(it);
                            continue;
                        }

                        const std::size_t outInfluences = std::max(layout->influences, config_.targetInfluences);
                        addFixFile(ownMeshBlendResource(component),
                                   {{IniKeywords::Type, IniKeywords::Buffer}, {IniKeywords::Format, IniKeywords::FormatR8UInt},
                                    {IniKeywords::Stride, std::to_string(outInfluences * 2)}},
                                   ownMeshBlendFile(component),
                                   [this, component, mesh, pathOf, vgMap = vgMap->second, anchored, outInfluences]() {
                                       std::set<long long> unmapped;
                                       ByteVec bytes = WWMIBlend::componentBlend(pathOf(mesh.blendFile), pathOf(mesh.positionFile),
                                                                                 mesh.vgOffset, vgMap, anchored, outInfluences, &unmapped);
                                       if (!unmapped.empty()) {
                                           ctx_.log("source component " + std::to_string(component) + "'s own-mesh blend names "
                                                    + std::to_string(unmapped.size()) + " bone(s) the row does not map; they read local 0");
                                       }
                                       return bytes;
                                   },
                                   {IniKeywords::WWMIBlendReg});
                        ++it;
                    }
                }

                // Not "...Blend...": the blend rename (getRemapBlendName) rewrites a value carrying it,
                // which moved the binding away from this declaration
                std::string ownMeshBlendResource(int component) const {
                    return remapFixName(IniKeywords::Resource + WWMINames::OwnMeshSkin + std::to_string(component));
                }

                // Beside the mod's own blend, so the component's buffers stay together
                std::string ownMeshBlendFile(int component) const {
                    const std::string rel = FileService::iniPathToRel(ownMeshes_.at(component).blendFile);
                    const std::size_t slash = rel.find_last_of("/\\");
                    const std::string folder = slash == std::string::npos ? std::string() : rel.substr(0, slash);
                    return modFile(folder, toModName_ + IniKeywords::Remap + WWMINames::OwnMeshBlend + std::to_string(component) + ".buf");
                }

                // A buffer of the mod's, as its .ini names it ("" when it declares none) -- the parser's
                // IniParseData::resourceFile. Off the .ini rather than as sibling names: a mod-manager
                // -packaged mod names every file by GUID with a `.assets` extension, so there is no
                // `Position.buf` beside anything -- built as a sibling, the path did not exist, the
                // remapped blend was never written, and the .ini still bound vb4 to it, which drew
                // NOTHING of the character. Found without regard to case: one mod spells it
                // ResourceTexCoordBuffer.
                std::string modBuffer(const std::string& resource) const {
                    return parseData().resourceFile(resource).value_or("");
                }

                // WHERE the 16-bit ids live, off the .ini rather than a sibling filename: a
                // mod-manager-packaged mod names every file by GUID with a `.assets` extension,
                // so the WWMI export's own name is not beside Blend.buf and the search finds
                // nothing -- and the fallback that then runs is a no-op dressed as a fix.
                std::optional<std::string> vertexVGFile() const {
                    return parseData().resourceFile(IniKeywords::WWMIBlendRemapVertexVGBuffer);
                }

                /**
                 * @brief
                 @rst
                 The absolute path of the mod's own ``Position.buf``, however its ``.ini`` names it
                 @endrst
                 *
                 * @throws std::runtime_error If the mod's ``.ini`` declares no such resource, which is
                 *      its own failure and not "the file is missing"
                 *
                 * @return The path
                 */
                std::string positionBufPath() const {
                    const std::string positionFile = modBuffer(IniKeywords::WWMIPositionBuffer);
                    if (positionFile.empty()) {
                        throw std::runtime_error(
                            "cannot fix this mod: its .ini declares no [" + IniKeywords::WWMIPositionBuffer
                            + "], so there is no mesh to read a vertex count from");
                    }

                    return FileService::absPathOfRelPath(
                        positionFile, ctx_.getIniFile()->getFolder());
                }

                /**
                 * @brief
                 @rst
                 One path the fix WRITES, relative to the mod's ``.ini`` -- inside 'folder' when the
                 mod keeps that kind of file in one, beside the ``.ini`` when it does not
                 @endrst
                 *
                 * @param folder #meshFolder_, empty for the mod's own root
                 * @param name The file's name
                 *
                 * @return The path, with no leading separator when there is no folder
                 */
                static std::string modFile(const std::string& folder, const std::string& name) {
                    return folder.empty() ? name : folder + "/" + name;
                }

                ModObj targetSlotObj(int slot) const {
                    return ModObj(toModName_, WWMINames::SlotPrefix + std::to_string(slot));
                }

                // The target object a source component's sections are remapped onto: its slot's, or
                // -- for a component drawn as a second section on a shared slot in the mod's own file
                // (SourceComponent::sameFileSection) -- one of its own, so the two claimants do not
                // collide in one group. The sections keep the source's names either way.
                ModObj targetObjOf(int component) const {
                    const WWMIFixerConfig::SourceComponent& planned = config_.plan.at(component);
                    if (!planned.sameFileSection) {
                        return targetSlotObj(planned.slot);
                    }

                    return ModObj(toModName_, WWMINames::SlotPrefix + std::to_string(planned.slot) + "_"
                                                  + std::to_string(component));
                }

                // Does the TARGET's merged skeleton pass what an 8-bit blend index can name, and can
                // one WWMI blend remap hold the row? Sets #targetPast256_ and #blendRemapBones_,
                // which every later phase reads.
                void absorbBlendWidth(const WWMIFixerConfig::BlendWidth& width) {
                    targetPast256_ = width.targetPast256;
                    blendRemapBones_ = width.bones;
                }

                void buildEdits() {
                    buildTexcoordCopy();
                    const ModType* source = ctx_.modType();
                    const std::optional<Version> from = fromVersion();
                    const std::optional<Version> to = toVersion();

                    assetRemap_ = std::make_unique<RegAssetRemap<>>(
                        std::vector<std::pair<std::string, RegAssetRemap<>::AssetSpec>>{
                            {IniKeywords::Hash, RegAssetRemap<>::AssetSpec(ctx_.modTypeHashes(), IniKeywords::HashNotFound)},
                            {IniKeywords::WWMIShapeKeyChecksum, RegAssetRemap<>::AssetSpec(source->shapeKeyChecksums.get(), ChecksumNotFound)},
                            // AND THE PER-BATCH FORM (2026-10-05). A newer WWMI Tools export loads its keys in
                            // batches (WWMI's Core/WWMI/ShapeKeys.ini reads batch0 and batch1) and names the
                            // checksum per batch. Only the bare key above was remapped, so such a mod's remap
                            // kept the SOURCE's checksum, ShapeKeyOverrider matched no buffer of the target,
                            // and a mod driving a hundred custom keys (Lynae9) drew its body as spikes. The
                            // batch's dispatch size needs no remap: on a mismatch WWMI falls back to
                            // dispatching at least the draw's own thread-group count.
                            {IniKeywords::WWMIShapeKeyChecksum + "_batch0", RegAssetRemap<>::AssetSpec(source->shapeKeyChecksums.get(), ChecksumNotFound)},
                            {IniKeywords::WWMIShapeKeyChecksum + "_batch1", RegAssetRemap<>::AssetSpec(source->shapeKeyChecksums.get(), ChecksumNotFound)}},
                        toModName_, ctx_.modTypeName().value_or(""), from, to);
                    assetAdapter_ = std::make_unique<RegPartEdit<>>(assetRemap_.get());
                    // see WWMIFixerConfig::shapeKeyDispatchSize
                    if (!config_.shapeKeyDispatchSize.empty()) {
                        const std::string key = IniKeywords::WWMIShapeKeyDispatchSizeYOriginalBatch;
                        dispatchEdit_ = std::make_unique<RegNewVals<>>(
                            std::vector<std::pair<std::string, RegNewVals<>::NewValSpec>>{
                                {key + "0", RegNewVals<>::NewVal(config_.shapeKeyDispatchSize)},
                                {key + "1", RegNewVals<>::NewVal(config_.shapeKeyDispatchSize)}});
                        dispatchAdapter_ = std::make_unique<RegPartEdit<>>(dispatchEdit_.get());
                    }

                    // see WWMIFixerConfig::shapeKeyOrder: the fix's shape-key lists bind the reordered
                    // copies; any other value of those registers stays
                    if (!shapeKeyFiles_.empty()) {
                        const RegNewVals<>::OldValProducer rebind =
                            [this](const std::string& old, const ModType*) -> std::string {
                                const std::string value(StringTools::strip(old));
                                for (const std::string& resource : ShapeKeyResources) {
                                    if (StringTools::equalsIgnoreCase(value, shapeKeyFiles_.at(resource).section)) {
                                        return remapFixName(resource);
                                    }
                                }
                                return old;
                            };

                        std::vector<std::pair<std::string, RegNewVals<>::NewValSpec>> vals;
                        for (const std::string& reg : {IniKeywords::WWMIShapeKeyOffsetReg, IniKeywords::WWMIShapeKeyVertexIdReg,
                                                       IniKeywords::WWMIShapeKeyVertexOffsetReg}) {
                            vals.emplace_back(reg, RegNewVals<>::NewVal(rebind));
                        }

                        shapeKeyEdit_ = std::make_unique<RegNewVals<>>(std::move(vals));
                        shapeKeyAdapter_ = std::make_unique<RegPartEdit<>>(shapeKeyEdit_.get());
                    }

                    // In this order because each reads what the one before it set: everything after the bypasses is
                    // per drawn slot (the "fileGroups" decision).
                    buildBypasses();
                    buildSlotValues();
                    settleOwnMeshes(source, vgRemapVersion(from), to);
                    buildComponentAdditions();
                    buildRerouteFilters();
                    buildSlotRemap();
                    buildPerGroupEdits();
                    buildBlendCollects(source, vgRemapVersion(from), to);
                }

                // The version of the VGRemaps row the mod's blend is read with: the numbering the parser read off
                // the mod's own geometry (IniParseData::blendVersion), else 'from'
                std::optional<Version> vgRemapVersion(const std::optional<Version>& from) {
                    if (drawnInfo_.legacy) {
                        return from;
                    }
                    const std::optional<std::string>& version = parseData().blendVersion();
                    return version.has_value() ? std::optional<Version>(Version::parse(*version)) : from;
                }

                /** @brief The fix's own list that puts the captured bindings back -- see bypassRestore */
                std::string restoreListName() {
                    return WWMISections::restoreListName(toModName_);
                }

                // Which of the mod's own shared-resource bindings the fix has to put back, and the
                // resource each is parked in. Read off the mod's OWN two lists rather than
                // configured: whatever the override binds and the cleanup does not already restore.
                void buildBypasses() {
                    bypasses_ = WWMISections::bypasses(parseData(), IniParseRows::Sections, WWMINames::OverrideSharedResourcesList,
                                                       WWMINames::CleanupSharedResourcesList, toModName_);
                }

                // Lines each remapped section drops (WWMIFixerConfig::removedLines) -- see WWMIFixerConfig::removedRegs for
                // why each kind is there.
                void absorbRemovals(const std::map<int, std::vector<WWMIFixerConfig::RegRemoval>>& removedLines) {
                    regRemoves_.clear();
                    removeAdapters_.clear();
                    for (const auto& [component, lines] : removedLines) {
                        // each value matched with or without `ref`: an identity mod writes `vs-cb4 = ResourceMergedSkeleton`
                        // and WWMI Tools `vs-cb4 = ref ResourceMergedSkeleton`
                        std::vector<std::pair<std::string, std::optional<RegRemove<>::RemoveKeyCheck>>> keys;
                        keys.reserve(lines.size());
                        for (const WWMIFixerConfig::RegRemoval& removal : lines) {
                            keys.emplace_back(removal.reg, removal.valuePrefix.empty()
                                                               ? std::nullopt
                                                               : std::optional<RegRemove<>::RemoveKeyCheck>(
                                                                     RegRemove<>::valueStartsWith(removal.valuePrefix)));
                        }
                        auto remove = std::make_unique<RegRemove<>>(std::move(keys));
                        removeAdapters_[component] = std::make_unique<RegPartEdit<>>(remove.get());
                        regRemoves_.push_back(std::move(remove));
                    }
                }

                // The groups (WWMIFixerConfig::fileGroups): one remapped section per target draw per file, in the
                // source components' numeric order, which is the order the remap below creates the target graphs in
                void absorbGroups(const WWMIFixerConfig::FileGroups& fileGroups) {
                    groups_ = fileGroups.groups;
                    drawnSlots_.clear();
                    for (const std::vector<int>& group : groups_) {
                        for (const int component : group) {
                            drawnSlots_.insert(config_.plan.at(component).slot);
                        }
                    }
                }

                // Per target slot: its numbers, written over the source's.
                void buildSlotValues() {
                    for (int slot : drawnSlots_) {
                        const Slot& s = target_.slots.at(static_cast<std::size_t>(slot));
                        auto newVals = std::make_unique<RegNewVals<>>(
                            std::vector<std::pair<std::string, RegNewVals<>::NewValSpec>>{
                                {IniKeywords::MatchFirstIndex, RegNewVals<>::NewVal(s.indexOffset)},
                                {IniKeywords::MatchIndexCount, RegNewVals<>::NewVal(s.indexCount)},
                                {IniKeywords::WWMIVgOffset, RegNewVals<>::NewVal(s.vgOffset)},
                                {IniKeywords::WWMIVgCount, RegNewVals<>::NewVal(s.vgCount)}},
                            false);
                        auto adapter = std::make_unique<RegPartEdit<>>(newVals.get());
                        newValsOf_[slot] = adapter.get();
                        newVals_.push_back(std::move(newVals));
                        regAdapters_.push_back(std::move(adapter));
                    }

                }

                // Per drawn source component: the zero stream and its texture command list right
                // after the shared-resource override. No after-register on the add: `drawindexed`
                // as one is a MUST fact, and behind a `$draw_x` toggle no draw is certain, which
                // parked the additions inside the first toggle.
                void buildComponentAdditions() {
                    for (const auto& entry : config_.plan) {
                        const int component = entry.first;
                        if (drawnInfo_.present.count(component) == 0) {
                            continue;
                        }

                        const WWMIFixerConfig::SourceComponent& planned = entry.second;
                        RegSurroundedAdd<>::Additions additions;
                        if (drawnInfo_.legacy || targetPast256_) {
                            // The fix's own merge list, not the mod's: for a legacy mod because it
                            // has none, and for a target past 256 bones because the mod's own
                            // merged skeleton is declared for 256 and the target's bones run past it.
                            additions.emplace_back(IniKeywords::Run, mergeListName(planned.slot));
                        }

                        const bool ownMesh = ownMeshes_.count(component) > 0;
                        if (ownMesh) {
                            // its own remapped blend, already in the target's local ids: no compute pass, and
                            // not the main mesh's blend or texcoord copy -- its own list binds its own buffers,
                            // and those are a different vertex count
                            additions.emplace_back(IniKeywords::WWMIBlendReg, ownMeshBlendResource(component));
                        } else if (targetPast256_) {
                            // After the merge list, so the private skeleton exists, and after the
                            // shared override, whose `vb4` this rebinds to the remapped blend.
                            additions.emplace_back(IniKeywords::Run, blendRemapInitList());
                        }

                        if (config_.zeroShapeKeyStream && meshVertexCount_ > 0) {
                            additions.emplace_back(IniKeywords::WWMIShapeKeyStreamReg, remapFixName(IniKeywords::Resource + ShapeKeyZero));
                        }

                        if (texcoordResource_.has_value() && !ownMesh) {
                            additions.emplace_back(IniKeywords::WWMITexcoordReg, *texcoordResource_);
                        }

                        // THE COMPONENT'S TEXTURE LIST (WWMIFixerConfig::textureLists / toggleLists)
                        const std::string passGuard = passCondition(config_.slotPasses.at(static_cast<std::size_t>(planned.slot)));
                        const TextureLines main = textureLinesOf("main", component);

                        if (!main.lines.empty()) {
                            const std::string cmdList = textureListName(component);
                            addTextureList(cmdList, passGuard, main, component);

                            // THE ROLE'S BINDINGS ARE DEFAULTS, SET BEFORE THE MOD'S OWN TEXTURE LINES
                            // (2026-10-28): right after the match keys, so the carried lines -- re-keyed to
                            // the target's registers -- override them wherever the mod binds a register,
                            // and a branch the mod leaves unbound keeps the default. Run after those lines
                            // instead, the list overrode the mod's own toggles with one variant.
                            auto top = std::make_unique<RegSurroundedAdd<>>(
                                RegSurroundedAdd<>::Additions{{std::string(IniKeywords::Run), cmdList}},
                                RegSurroundedAdd<>::RegMap{{std::string(IniKeywords::MatchFirstIndex),
                                                            [](const std::string&) { return true; }}},
                                RegSurroundedAdd<>::RegMap{}, false);
                            auto topAdapter = std::make_unique<GraphPartEdit<>>(top.get());
                            objEdit("textureListCalls", component, topAdapter.get());
                            surroundedAdds_.push_back(std::move(top));
                            graphAdapters_.push_back(std::move(topAdapter));
                        }

                        // WHAT A LINKED .ini SETS ON THIS DRAW (the "linkedLines" step): RabbitFX's glow map for every pass,
                        // and a texture under a condition at the register this component's plan binds its role at,
                        // on the passes the texture list binds it on -- after the textures and before the section's own
                        // `run = CommandList\RabbitFX\Run`
                        if (const auto linked = linkedLines_.find(component); linked != linkedLines_.end() && !linked->second.empty()) {
                            const std::string glowList = remapFixName(IniKeywords::CommandList + source_.name
                                                                      + TextTools::capitalize(WWMINames::SlotPrefix) + std::to_string(component) + WWMINames::Glow);
                            SectionText list(z3_, glowList);
                            for (const CarriedLine& line : linked->second) {
                                if (line.key.empty()) {
                                    continue;
                                }
                                if (line.condition.empty()) {
                                    list.key(line.key, line.value);
                                } else {
                                    list.open(line.condition).key(line.key, line.value).close();
                                }
                            }
                            bool gated = false;
                            for (const CarriedLine& line : linked->second) {
                                const std::optional<std::string> reg = line.key.empty()
                                    ? config_.registerOf(bindingsOf(component), line.role) : std::nullopt;
                                if (!reg.has_value()) {
                                    continue;               // a glow map, or a role this slot's plan does not bind
                                }
                                if (!gated) {
                                    list.open(passGuard);
                                    gated = true;
                                }
                                if (line.condition.empty()) {
                                    list.key(*reg, line.value);
                                } else {
                                    list.open(line.condition).key(*reg, line.value).close();
                                }
                            }
                            if (gated) {
                                list.close();
                            }
                            addFixSection("glowLists", list.build());
                            additions.emplace_back(IniKeywords::Run, glowList);
                        }

                        // GENERATION 0: THE DRAWS THIS SECTION MAKES BEFORE IT BINDS ANYTHING
                        // (2026-10-03). They rendered with the SOURCE's own game textures, so that
                        // is what they are given -- see IniParseData::lateBindings and the "gameDefaults" step. Run with the
                        // additions, after the list above, so it wins for those draws.
                        if (textureLists_.find("game", component) != nullptr) {
                            const TextureLines game = textureLinesOf("game", component);
                            if (!game.lines.empty()) {
                                const std::string gameList = textureListName(component, GameSuffix);
                                addTextureList(gameList, passGuard, game, component);
                                additions.emplace_back(IniKeywords::Run, gameList);
                            }
                        }

                        // A slot's OTHER passes bind the same art at DIFFERENT registers, so each
                        // gets its own guarded list beside the plan's -- see
                        // WWMIFixerConfig::extraPassRegs.
                        const auto extra = config_.extraPassRegs.find(planned.slot);
                        if (extra != config_.extraPassRegs.end()) {
                            int n = 0;
                            for (const auto& [pass, regs] : extra->second) {
                                (void)regs;
                                // its bindings for this source (Binding::srcComponent), and `ib = null` where the
                                // draw is suppressed on the pass (WWMIFixerConfig::extraPassNoDraw)
                                const TextureLines passLines = textureLinesOf("pass", component, n);
                                if (!passLines.lines.empty()) {
                                    const std::string extraList = textureListName(component, "", WWMINames::Pass + std::to_string(n));
                                    addTextureList(extraList, passCondition(pass), passLines, component);
                                    additions.emplace_back(IniKeywords::Run, extraList);
                                }
                                ++n;
                            }
                        }

                        // THE CLEANUP LIST IS MADE THE INVERSE OF THE OVERRIDE LIST (2026-10-02).
                        // Hung on every component because both lists are reached through the
                        // component's own graph; the edit is idempotent, so being visited once per
                        // component writes one capture and one restore.
                        if (!bypasses_.empty()) {
                            for (auto& [bypass, sections] : bypassRestore(listNames(WWMINames::OverrideSharedResourcesList),
                                                                          listNames(WWMINames::CleanupSharedResourcesList),
                                                                          bypasses_, restoreListName())) {
                                auto adapter = std::make_unique<RegPartEdit<>>(
                                    bypass.get(), [sections = sections](const std::string& name) { return namesSection(sections, name); });
                                objEdit("sharedBypass", component, adapter.get());
                                bypassEdits_.push_back(std::move(bypass));
                                regAdapters_.push_back(std::move(adapter));
                            }
                        }

                        // THE MOD'S OWN `ps-t` LINES, RE-KEYED BY ROLE (2026-10-02), each to the register this
                        // component's plan row reads its texture's role at -- the layout of the fix's own list,
                        // which they sit after. Before the twin, because it reads what the mod wrote and the
                        // twin only adds draws. See CarriedLines.
                        {
                            // what WWMIFixerConfig::modTexLines made of each of the mod's own lines (the "modTexLines" step)
                            CarriedLines lines;
                            if (const auto planned = modTexLines_.find(component); planned != modTexLines_.end()) {
                                for (const WWMIFixerConfig::ModTexLine& line : planned->second) {
                                    lines.emplace(std::make_pair(line.key, line.value),
                                                  line.dropped ? std::optional<Kvp>() : std::optional<Kvp>(Kvp{line.reg, line.newValue}));
                                }
                            }

                            auto carried = carriedTexRegs(IniKeywords::TexRegPrefix, std::move(lines));
                            auto adapter = std::make_unique<RegPartEdit<>>(carried.get());
                            objEdit("modTexLines", component, adapter.get());
                            carriedEdits_.push_back(std::move(carried));
                            regAdapters_.push_back(std::move(adapter));
                        }

                        // AND GENERATION 1 NEEDS NOTHING ADDED. The mod's own `ps-t` lines, re-keyed
                        // by role below, are that generation -- and a register they do NOT rebind
                        // keeps what generation 0 left, which is the source's own game texture and
                        // exactly what the unfixed mod rendered with there. Re-running the list
                        // above at that point was written first and is wrong twice over: the carried
                        // lines overwrite it immediately, and where they do not, it would replace a
                        // game texture the mod deliberately kept with the mod's art.
                        if (!additions.empty()) {
                            // The remap has already renamed the called list by the time this runs,
                            // so the anchor is matched under either name -- and a component drawn
                            // from its own mesh runs its own list in place of the shared one.
                            std::vector<std::string> sharedLists = listNames(WWMINames::OverrideSharedResourcesList);
                            const auto own = ownMeshes_.find(component);
                            if (own != ownMeshes_.end()) {
                                for (const std::string& name : listNames(own->second.list)) {
                                    sharedLists.push_back(name);
                                }
                            }
                            auto add = std::make_unique<RegSurroundedAdd<>>(
                                std::move(additions),
                                RegSurroundedAdd<>::RegMap{{IniKeywords::Run, [sharedLists](const std::string& value) {
                                    return std::find(sharedLists.begin(), sharedLists.end(), value) != sharedLists.end();
                                }}},
                                RegSurroundedAdd<>::RegMap{}, false);
                            auto adapter = std::make_unique<GraphPartEdit<>>(add.get());
                            objEdit("additions", component, adapter.get());
                            surroundedAdds_.push_back(std::move(add));
                            graphAdapters_.push_back(std::move(adapter));
                        }
                    }

                }

                // ---- WWMIFixerConfig::drawReroutes ----

                // Every name a mod's command list can carry in the remapped graphs: its own, the
                // fix's, and each reroute copy's (see buildSlotRemap)
                std::vector<std::string> listNames(const std::string& list) const {
                    std::vector<std::string> names{list, remapFixName(list)};
                    for (const auto& entry : drawnInfo_.rerouteFrom) {
                        names.push_back(remapFixName(list + WWMINames::Reroute + std::to_string(entry.first)));
                    }

                    return names;
                }

                // (Which draws of each reroute's source component go elsewhere is WWMIFixerConfig::drawnOf's
                // reading: a reroute that takes draws becomes a present component of its own, sharing the source's
                // sections and every texture it reads.)

                // ---- WWMIFixerConfig::SourceComponent::rabbitFX ----
                // The routing flags RabbitFX's SetTextures sets on the SOURCE's own draw of each
                // component that keeps RabbitFX, from what that draw has BOUND at ps-t0..t3 -- not
                // from the maps the mod hands RabbitFX, which it only swaps in afterwards. Per register:
                // the mod's own `ps-tN` line (unless only a toggle sets it), else the mod's `this`
                // override of the game texture there, else the game texture itself (gameTags)
                void absorbRabbitFX(const std::map<int, std::array<int, 4>>& flags) {
                    rabbitFXFlags_ = flags;
                    rabbitFXPins_.clear();
                    rabbitFXPinAdapters_.clear();
                    for (const auto& [component, componentFlags] : rabbitFXFlags_) {
                        (void)componentFlags;
                        // a `run =` of the pin list right after each `run = CommandList\RabbitFX\SetTextures`,
                        // which sets RabbitFX's routing flags
                        auto pin = std::make_unique<AnchorAdd>(
                            std::vector<std::string>{IniKeywords::Run},
                            [pinList = rabbitFXPinList(component)](const std::string&, const std::string& value) -> std::optional<Kvps> {
                                if (!StringTools::equalsIgnoreCase(StringTools::strip(value), IniKeywords::RabbitFXSetTexturesPath)) {
                                    return std::nullopt;
                                }
                                return Kvps{{IniKeywords::Run, pinList}};
                            },
                            AnchorAdd::Placement::After);
                        rabbitFXPinAdapters_[component] = std::make_unique<RegPartEdit<>>(pin.get());
                        rabbitFXPins_.push_back(std::move(pin));
                    }
                }

                std::string rabbitFXPinList(int component) const {
                    return remapFixName(IniKeywords::CommandList + WWMINames::RabbitFXPin + std::to_string(component));
                }

                void rabbitFXPinSections() {
                    for (const auto& [component, flags] : rabbitFXFlags_) {
                        const int slot = config_.plan.at(component).slot;
                        addFixSection("rabbitFXPinLists", SectionText(z3_, rabbitFXPinList(component))
                                   .open(passCondition(config_.slotPasses.at(static_cast<std::size_t>(slot))))
                                   .keys({{RabbitFX::FlagKeys[0], std::to_string(flags[0])}, {RabbitFX::FlagKeys[1], std::to_string(flags[1])},
                                          {RabbitFX::FlagKeys[2], std::to_string(flags[2])}, {RabbitFX::FlagKeys[3], std::to_string(flags[3])}})
                                   .close()
                                   .build());
                    }
                }

                // Whether a component's plan entry draws it with its shader -- see Shader::yieldToRabbitFX
                bool shaderUsed(int component) const {
                    return config_.shaderDraws(component, usesRabbitFX(component));
                }

                // A component's bindings: its plan entry's, and its shader's while the shader draws it
                std::vector<WWMIFixerConfig::Binding> bindingsOf(int component) const {
                    return config_.bindingsOf(component, shaderUsed(component));
                }

                // Whether the mod's sections of a component name RabbitFX (its resources or its SetTextures)
                bool usesRabbitFX(int component) const {
                    return drawnInfo_.present.count(component) > 0 && parseData().libraries(slotObj(component)).count(RabbitFX::Name) > 0;
                }

                // A reroute's copy keeps only its draws; its source's copy keeps everything else
                void buildRerouteFilters() {
                    for (const auto& [as, from] : drawnInfo_.rerouteFrom) {
                        const std::set<std::pair<long long, long long>> taken = drawnInfo_.reroutedDraws.at(as);
                        const auto rangeOf = [](const std::string& value) { return IniParseData::drawRangeOf(value); };

                        // the source keeps a draw that is not taken (and anything it cannot read)
                        auto dropTaken = std::make_unique<RegRemove<>>(std::vector<std::pair<std::string, std::optional<RegRemove<>::RemoveKeyCheck>>>{
                            {IniKeywords::DrawIndexed, RegRemove<>::RemoveKeyCheck([taken, rangeOf](long long, const std::string& value) {
                                const auto range = rangeOf(value);
                                return range.has_value() && taken.count(*range) > 0;
                            })}});
                        // ...and the reroute keeps only the taken ones
                        auto keepTaken = std::make_unique<RegRemove<>>(std::vector<std::pair<std::string, std::optional<RegRemove<>::RemoveKeyCheck>>>{
                            {IniKeywords::DrawIndexed, RegRemove<>::RemoveKeyCheck([taken, rangeOf](long long, const std::string& value) {
                                const auto range = rangeOf(value);
                                return !range.has_value() || taken.count(*range) == 0;
                            })}});

                        auto dropAdapter = std::make_unique<RegPartEdit<>>(dropTaken.get());
                        auto keepAdapter = std::make_unique<RegPartEdit<>>(keepTaken.get());
                        objEdit("rerouteFilters", from, dropAdapter.get());
                        objEdit("rerouteFilters", as, keepAdapter.get());
                        rerouteRemoves_.push_back(std::move(dropTaken));
                        rerouteRemoves_.push_back(std::move(keepTaken));
                        regAdapters_.push_back(std::move(dropAdapter));
                        regAdapters_.push_back(std::move(keepAdapter));
                    }

                    std::map<std::pair<long long, long long>, std::string> sectionOf;
                    for (const auto& [as, shader] : rerouteShaders_) {
                        for (const auto& range : shaderDraws(as)) {
                            sectionOf.emplace(range, rerouteShaderSection(as, range));
                        }
                    }

                    rerouteShaderEdit_.reset();
                    rerouteShaderAdapter_.reset();
                    if (!sectionOf.empty()) {
                        rerouteShaderEdit_ = drawRunAdd(std::move(sectionOf), AnchorAdd::Placement::Replace);
                        rerouteShaderAdapter_ = std::make_unique<RegPartEdit<>>(rerouteShaderEdit_.get());
                    }

                    std::map<std::pair<long long, long long>, std::string> markOf;
                    for (const auto& entry : markLines_) {
                        for (const auto& range : shaderDraws(entry.first)) {
                            markOf.emplace(range, markSection(entry.first, range));
                        }
                    }

                    offEdits_.clear();
                    offAdapters_.clear();
                    for (const auto& [component, planned] : config_.plan) {
                        if (planned.offPasses.empty() || rerouteShaders_.count(component) > 0 || drawnInfo_.present.count(component) == 0) {
                            continue;
                        }

                        std::map<std::pair<long long, long long>, std::string> runOf;
                        for (const auto& range : shaderDraws(component)) {
                            const Kvp run = offDraw(component, range);
                            if (run.first == IniKeywords::Run) {
                                runOf.emplace(range, run.second);
                            }
                        }

                        if (!runOf.empty()) {
                            auto edit = drawRunAdd(std::move(runOf), AnchorAdd::Placement::Replace);
                            offAdapters_[component] = std::make_unique<RegPartEdit<>>(edit.get());
                            offEdits_.push_back(std::move(edit));
                        }
                    }

                    markEdit_.reset();
                    markAdapter_.reset();
                    if (!markOf.empty()) {
                        markEdit_ = drawRunAdd(std::move(markOf), AnchorAdd::Placement::After);
                        markAdapter_ = std::make_unique<RegPartEdit<>>(markEdit_.get());
                    }

                    // SourceComponent::overlay, the same way: its list runs after each draw
                    std::map<std::pair<long long, long long>, std::string> overlayOf;
                    for (const auto& entry : overlays_) {
                        for (const auto& range : shaderDraws(entry.first)) {
                            overlayOf.emplace(range, overlaySection(entry.first, range));
                        }
                    }
                    overlayEdit_.reset();
                    overlayAdapter_.reset();
                    if (!overlayOf.empty()) {
                        overlayEdit_ = drawRunAdd(std::move(overlayOf), AnchorAdd::Placement::After);
                        overlayAdapter_ = std::make_unique<RegPartEdit<>>(overlayEdit_.get());
                    }

                    // Shader::after: each draw of the component drawn first runs one list, which makes the
                    // draws held back for it
                    // (the cleanup list under its own name and under the fix's, which the list renames
                    // may already have given it)
                    afterDispatch_.clear();
                    afterEdits_.clear();
                    afterAdapters_.clear();
                    for (const auto& [as, shader] : rerouteShaders_) {
                        (void)as;
                        for (const auto& [first, draws] : awaited(*shader)) {
                            (void)draws;
                            if (afterDispatch_.count(first) > 0) {
                                continue;
                            }

                            const std::string list = remapFixName(IniKeywords::CommandList + WWMINames::DrawAfter + std::to_string(first));
                            afterDispatch_[first] = list;
                            afterEdits_[first] = runBeforeRuns(
                                std::vector<std::string>{WWMINames::CleanupSharedResourcesList, remapFixName(WWMINames::CleanupSharedResourcesList)}, list,
                                afterLatch(first));
                            afterAdapters_[first] = std::make_unique<RegPartEdit<>>(afterEdits_[first].get());
                        }
                    }
                }

                // ---- WWMIFixerConfig::SourceComponent::overlay ----
                std::string overlaySection(int component, const std::pair<long long, long long>& range,
                                           const std::string& kind = IniKeywords::CommandList, const std::string& suffix = "") const {
                    const std::set<std::pair<long long, long long>> draws = shaderDraws(component);
                    const auto at = static_cast<std::size_t>(std::distance(draws.begin(), draws.find(range)));
                    return remapFixName(kind + WWMINames::Overlay + std::to_string(component) + WWMINames::Draw + std::to_string(at) + suffix);
                }

                // ---- WWMIFixerConfig::SourceComponent::markLines ----
                std::string markSection(int component, const std::pair<long long, long long>& range,
                                        const std::string& kind = IniKeywords::CommandList, const std::string& suffix = "") const {
                    const std::set<std::pair<long long, long long>> draws = shaderDraws(component);
                    const auto at = static_cast<std::size_t>(std::distance(draws.begin(), draws.find(range)));
                    return remapFixName(kind + WWMINames::Mark + std::to_string(component) + WWMINames::Draw + std::to_string(at) + suffix);
                }

                // ---- WWMIFixerConfig::DrawReroute::shader ----
                //
                // Each rerouted draw becomes a `run =` of a list that draws it with the shader on the
                // plan's passes and as it was on any other (the slot's outline pass draws it too). The
                // files and the buffer resources go by the shader's NAME, so two reroutes drawn with one
                // shader share them
                // The draws a component with its own shader makes: a reroute's taken ones, or every
                // draw of the component
                std::set<std::pair<long long, long long>> shaderDraws(int component) const {
                    const auto taken = drawnInfo_.reroutedDraws.find(component);
                    if (taken != drawnInfo_.reroutedDraws.end()) {
                        return taken->second;
                    }

                    std::set<std::pair<long long, long long>> draws;
                    const auto ranges = drawnInfo_.drawRanges.find(component);
                    if (ranges != drawnInfo_.drawRanges.end()) {
                        draws.insert(ranges->second.begin(), ranges->second.end());
                    }
                    return draws;
                }

                // Shader::after: the components a shader waits for that the mod draws, with their draws a pass
                std::vector<std::pair<int, int>> awaited(const WWMIFixerConfig::Shader& shader) const {
                    std::vector<std::pair<int, int>> result;
                    if (!shader.after.has_value()) {
                        return result;
                    }

                    if (drawnInfo_.present.count(shader.after->component) > 0) {
                        result.emplace_back(shader.after->component, shader.after->draws);
                    }
                    for (const auto& wait : shader.after->also) {
                        if (drawnInfo_.present.count(wait.first) > 0) {
                            result.push_back(wait);
                        }
                    }
                    return result;
                }

                // Whether a shader drawn for this mod binds a capture off the slot (Shader::stashed)
                bool stashRead(int slot) const {
                    for (const auto& [as, shader] : rerouteShaders_) {
                        (void)as;
                        for (const WWMIFixerConfig::Shader::Stashed& stash : shader->stashed) {
                            if (stash.slot == slot) {
                                return true;
                            }
                        }
                    }
                    return false;
                }

                std::string stashResource(int slot, const std::string& reg) const {
                    return remapFixName(IniKeywords::Resource + WWMINames::Stash + std::to_string(slot) + IniNamingTools::getRegTag(reg));
                }


                // A Shader's lines as its CustomShader writes them: its captures (Shader::stashed), its roles' textures
                // (Shader::gameRoles; one with no resource binds nothing), then its literal lines. 'resolved' false: every
                // register they assign, for the backups around the run
                std::vector<Kvp> shaderLines(const WWMIFixerConfig::Shader& shader, bool resolved) const {
                    std::vector<Kvp> out;
                    for (const WWMIFixerConfig::Shader::Stashed& stash : shader.stashed) {
                        out.emplace_back(stash.reg, resolved ? IniKeywords::Ref + " " + stashResource(stash.slot, stash.slotReg) : std::string());
                    }
                    for (const auto& [reg, role] : shader.gameRoles) {
                        const std::string* resource = sharedResourceFor(role);
                        if (!resolved) {
                            out.emplace_back(reg, "");
                        } else if (resource != nullptr) {
                            out.emplace_back(reg, *resource);
                        }
                    }
                    out.insert(out.end(), shader.lines.begin(), shader.lines.end());
                    return out;
                }

                std::string afterLatch(int component) const {
                    return "$" + remapFixName(WWMINames::DrawAfter + std::to_string(component) + WWMINames::Latch);
                }

                std::string rerouteShaderSection(int as, const std::pair<long long, long long>& range,
                                                 const std::string& kind = IniKeywords::CommandList,
                                                 const std::string& suffix = "") const {
                    const std::set<std::pair<long long, long long>> draws = shaderDraws(as);
                    const auto at = static_cast<std::size_t>(std::distance(draws.begin(), draws.find(range)));
                    return remapFixName(kind + rerouteShaders_.at(as)->name + std::to_string(as) + WWMINames::Draw + std::to_string(at) + suffix);
                }

                std::string rerouteShaderFile(const WWMIFixerConfig::Shader& shader) const {
                    return modFile(meshFolder_, toModName_ + IniKeywords::Remap + shader.name + ".hlsl");
                }

                // A pass's own shader (Shader::passPs), beside the shader's
                std::string reroutePassShaderFile(const WWMIFixerConfig::Shader& shader, std::size_t pass) const {
                    return modFile(meshFolder_, toModName_ + IniKeywords::Remap + shader.name + WWMINames::Pass + std::to_string(pass) + ".hlsl");
                }

                std::string rerouteBufferResource(const WWMIFixerConfig::Shader& shader, const std::string& reg) const {
                    return remapFixName(IniKeywords::Resource + shader.name + IniNamingTools::getRegTag(reg));
                }

                std::string rerouteBufferFile(const WWMIFixerConfig::Shader& shader, const std::string& reg) const {
                    return modFile(meshFolder_, toModName_ + IniKeywords::Remap + shader.name + IniNamingTools::getRegTag(reg) + ".buf");
                }

                std::string shaderBackup(const std::string& reg) const {
                    return remapFixName(IniKeywords::Resource + WWMINames::ShaderBackup + IniNamingTools::getRegTag(reg));
                }

                // A CustomShader's register bindings outlive it: 3DMigoto restores its shaders and render state,
                // not the resources its section assigns, so a later draw of the same game draw -- another
                // component on the slot, drawn by the game's shader -- would read them (Lynae8's body on the skin,
                // shaded with Lynae's material constants, rendered black). Each register the section assigns is
                // saved before the run and put back after it.
                // (CustomShaderBackups)
                std::vector<std::pair<std::string, std::string>> restoringRuns(const std::vector<std::pair<std::string, std::string>>& runs,
                                                                   const std::vector<std::pair<std::string, std::string>>& lines) {
                    return shaderBackups_.wrap(runs, lines);
                }

                // The sections: one CustomShader per rerouted draw, and the shader's constant buffers.
                // The shader's source file is named by its `ps =`, which an undo removes with the block.
                void rerouteShaderSections() {
                    std::set<std::string> declared;
                    shaderBackups_.clear();
                    hasShaders_ = false;
                    shaderSources_.clear();
                    shaderBuffers_.clear();
                    shaderBufferRegs_.clear();
                    for (const auto& [as, shader] : rerouteShaders_) {
                        const std::string onPlan = passCondition(config_.slotPasses.at(static_cast<std::size_t>(config_.plan.at(as).slot)));
                        const std::size_t passCount = std::max<std::size_t>(1, shader->passes.size());

                        // what every draw of the shader assigns, and each pass's lines (WWMISections::ShaderDraw)
                        std::vector<Kvp> assigned;
                        std::vector<std::vector<Kvp>> passes(passCount);
                        for (std::size_t p = 0; p < passCount; ++p) {
                            if (!shader->passes.empty()) {
                                assigned.insert(assigned.end(), shader->passes[p].begin(), shader->passes[p].end());
                            }
                            const bool ownPs = p < shader->passPs.size() && !shader->passPs[p].empty();
                            passes[p].emplace_back(IniKeywords::Ps, ownPs ? reroutePassShaderFile(*shader, p) : rerouteShaderFile(*shader));
                            for (const auto& entry : shader->constantBuffers) {
                                passes[p].emplace_back(entry.first, rerouteBufferResource(*shader, entry.first));
                            }
                            for (const Kvp& line : shaderLines(*shader, true)) {
                                passes[p].push_back(line);
                            }
                            if (!shader->passes.empty()) {
                                passes[p].insert(passes[p].end(), shader->passes[p].begin(), shader->passes[p].end());
                            }
                        }
                        for (const auto& entry : shader->constantBuffers) {
                            assigned.emplace_back(entry.first, "");
                        }
                        for (const Kvp& line : shaderLines(*shader, false)) {
                            assigned.push_back(line);
                        }

                        // Shader::after: the draws a pass makes of the components the shader waits for
                        const std::vector<std::pair<int, int>> waits = awaited(*shader);
                        int total = 0;
                        for (const auto& wait : waits) {
                            total += wait.second;
                        }

                        for (const auto& range : shaderDraws(as)) {
                            WWMISections::ShaderDraw draw;
                            draw.name = [this, as = as, range](const std::string& kind, const std::string& suffix) {
                                return rerouteShaderSection(as, range, kind, suffix);
                            };
                            draw.draw = std::to_string(range.first) + ", " + std::to_string(range.second) + ", 0";
                            draw.onPlan = onPlan;
                            draw.onlyWith = shader->onlyWith;
                            draw.fallback = offDraw(as, range);
                            draw.passes = passes;
                            draw.assigned = assigned;
                            if (!waits.empty()) {
                                draw.afterTarget = shader->after->target;
                                draw.afterDraws = total;
                                draw.afterCarried = shader->after->carried;
                            }

                            for (auto& section : WWMISections::shaderDrawSections(z3_, draw, shaderBackups_)) {
                                addFixSection("shaderDraws", std::move(section));
                            }
                            for (const Kvp& constant : WWMISections::shaderDrawConstants(draw)) {
                                afterConstants_.push_back(constant);
                            }
                            hasShaders_ = true;
                        }

                        if (!declared.insert(shader->name).second) {
                            continue;
                        }

                        // what the graph's `ps` and `ps-cbN` lines name -- see shaderFileEdits
                        shaderSources_[rerouteShaderFile(*shader)] = ByteVec(shader->ps.begin(), shader->ps.end());
                        for (std::size_t p = 0; p < shader->passPs.size(); ++p) {
                            if (!shader->passPs[p].empty()) {
                                shaderSources_[reroutePassShaderFile(*shader, p)] = ByteVec(shader->passPs[p].begin(), shader->passPs[p].end());
                            }
                        }
                        for (const auto& [reg, words] : shader->constantBuffers) {
                            const BufUnSignedInt word32;
                            ByteVec bytes;
                            bytes.reserve(words.size() * 4);
                            for (std::uint32_t word : words) {
                                const ByteVec encoded = word32.encode(BufValue(static_cast<unsigned long long>(word)));
                                bytes.insert(bytes.end(), encoded.begin(), encoded.end());
                            }

                            shaderBufferRegs_.insert(reg);
                            shaderBuffers_[rerouteBufferResource(*shader, reg)] = FileAddCreate<>::File{
                                {{IniKeywords::Type, IniKeywords::Buffer}, {IniKeywords::Format, IniKeywords::FormatR32G32B32A32FloatDxgi}},
                                rerouteBufferFile(*shader, reg), std::move(bytes)};
                        }
                    }

                    for (const auto& [first, dispatch] : afterDispatch_) {
                        std::vector<Kvp> afterRuns;
                        for (const auto& [as, shader] : rerouteShaders_) {
                            for (const auto& wait : awaited(*shader)) {
                                if (wait.first != first) {
                                    continue;
                                }
                                for (const auto& range : shaderDraws(as)) {
                                    afterRuns.emplace_back(IniKeywords::Run, rerouteShaderSection(as, range, IniKeywords::CommandList, "After"));
                                }
                            }
                        }

                        const std::string latch = afterLatch(first);
                        afterConstants_.emplace_back(IniKeywords::Global + " " + latch, "0");
                        addFixSection("shaderDraws", SectionText(z3_, dispatch).open(latch + " == 0").key(latch, "1").keys(afterRuns).close().build());
                    }

                    if (!afterConstants_.empty()) {
                        // a second [Constants] in a file is merged into the first; inside the fix block
                        // an undo removes it with the block
                        addFixSection("shaderDraws", SectionText(z3_, IniKeywords::Constants).keys(afterConstants_).build());
                        afterConstants_.clear();
                    }

                    for (const auto& [component, lines] : markLines_) {
                        const std::string onPlan = passCondition(config_.slotPasses.at(static_cast<std::size_t>(config_.plan.at(component).slot)));
                        for (const auto& range : shaderDraws(component)) {
                            // a shader draw with no fallback: the mod's own draw of the range is kept, and the
                            // mark is drawn after it on the slot's passes
                            WWMISections::ShaderDraw mark;
                            mark.name = [this, component, range](const std::string& kind, const std::string& suffix) {
                                return markSection(component, range, kind, suffix);
                            };
                            mark.draw = std::to_string(range.first) + ", " + std::to_string(range.second) + ", 0";
                            mark.onPlan = onPlan;
                            mark.passes = {{{IniKeywords::Ps, "null"}}};
                            mark.passes[0].insert(mark.passes[0].end(), lines->begin(), lines->end());
                            mark.assigned = *lines;
                            for (auto& section : WWMISections::shaderDrawSections(z3_, mark, shaderBackups_)) {
                                addFixSection("shaderDraws", std::move(section));
                            }
                            hasShaders_ = true;
                        }
                    }

                    // SourceComponent::overlay: after each draw, its list binds the overlay's registers (the
                    // mod's own files, unedited: the edits are for the TARGET's shader), runs the CustomShader and
                    // puts back every register either assigned -- a shader draw with no fallback, as a mark is
                    for (const auto& [component, overlay] : overlays_) {
                        const std::string onPlan = passCondition(config_.slotPasses.at(static_cast<std::size_t>(config_.plan.at(component).slot)));
                        // its bindings (WWMIFixerConfig::textureLists' "overlay" list), each register assigned
                        const std::vector<Kvp> bindings = textureLinesOf("overlay", component).lines;
                        std::vector<Kvp> assigned;
                        if (const WWMIFixerConfig::TextureList* list = textureLists_.find("overlay", component)) {
                            for (const WWMIFixerConfig::TextureLine& line : list->lines) {
                                assigned.emplace_back(line.reg, "");
                            }
                        }
                        for (const auto& entry : overlay->constantBuffers) {
                            assigned.emplace_back(entry.first, "");
                        }
                        for (const Kvp& line : shaderLines(*overlay, false)) {
                            assigned.push_back(line);
                        }

                        std::vector<Kvp> pass{{IniKeywords::Ps, rerouteShaderFile(*overlay)}};
                        for (const auto& entry : overlay->constantBuffers) {
                            pass.emplace_back(entry.first, rerouteBufferResource(*overlay, entry.first));
                        }
                        for (const Kvp& line : shaderLines(*overlay, true)) {
                            pass.push_back(line);
                        }

                        for (const auto& range : shaderDraws(component)) {
                            WWMISections::ShaderDraw draw;
                            draw.name = [this, component = component, range](const std::string& kind, const std::string& suffix) {
                                return overlaySection(component, range, kind, suffix);
                            };
                            draw.draw = std::to_string(range.first) + ", " + std::to_string(range.second) + ", 0";
                            draw.onPlan = onPlan;
                            draw.onlyWith = overlay->onlyWith;
                            draw.passes = {pass};
                            draw.assigned = assigned;
                            draw.before = bindings;
                            for (auto& section : WWMISections::shaderDrawSections(z3_, draw, shaderBackups_)) {
                                addFixSection("shaderDraws", std::move(section));
                            }
                            hasShaders_ = true;
                        }

                        // its source file and constant buffers, once a shader
                        if (shaderSources_.count(rerouteShaderFile(*overlay)) > 0) {
                            continue;
                        }
                        shaderSources_[rerouteShaderFile(*overlay)] = ByteVec(overlay->ps.begin(), overlay->ps.end());
                        for (const auto& [reg, words] : overlay->constantBuffers) {
                            const BufUnSignedInt word32;
                            ByteVec bytes;
                            bytes.reserve(words.size() * 4);
                            for (std::uint32_t word : words) {
                                const ByteVec encoded = word32.encode(BufValue(static_cast<unsigned long long>(word)));
                                bytes.insert(bytes.end(), encoded.begin(), encoded.end());
                            }

                            shaderBufferRegs_.insert(reg);
                            shaderBuffers_[rerouteBufferResource(*overlay, reg)] = FileAddCreate<>::File{
                                {{IniKeywords::Type, IniKeywords::Buffer}, {IniKeywords::Format, IniKeywords::FormatR32G32B32A32FloatDxgi}},
                                rerouteBufferFile(*overlay, reg), std::move(bytes)};
                        }
                    }

                    for (const std::string& reg : shaderBackups_.registers()) {
                        addFixSection("shaderDraws", shaderBackups_.section(reg));
                    }

                    for (const auto& [name, gated] : offLists_) {
                        addFixSection("shaderDraws", SectionText(z3_, name).open(gated.first).key(IniKeywords::DrawIndexed, gated.second).close().build());
                    }
                }

                void buildSlotRemap() {
                    // Members rather than locals: buildPerGroupEdits below reads both, and the two
                    // phases were one function when they shared them by scope.
                    hashOnlyObjs_.clear();
                    if (auto* parser = dynamic_cast<GIMIParser<>*>(this->getParser())) {
                        for (const ModObj& obj : parser->modObjs()) {
                            if (!StringTools::startsWith(obj.second, WWMINames::SlotPrefix) && obj.first != IniGraphModObjKeywords::Texture
                                    && !isCopiedObj(obj)) {
                                hashOnlyObjs_.push_back(obj);
                            }
                        }
                    }

                    hiddenObjs_ = std::unordered_set<std::string>(config_.hiddenObjs.begin(), config_.hiddenObjs.end());
                    for (const ModObj& obj : hashOnlyObjs_) {
                        if (hiddenObjs_.count(obj.second) > 0) {
                            this->hiddenModObjs.insert(obj);
                        }
                    }

                    // THE "slotRemap" STEP: each planned component's source graph onto its target slot (planTable), with
                    // what this mod adds to it (slotRemapTable)
                    slotRemap_ = std::make_unique<GraphGroupRemap<>>(slotRemapTable(planTable()));
                }

                // Each component WWMIFixerConfig::plan names: its source graph ("", <slotPrefix><component>) onto its target
                // slot's graph (<target>, <slotPrefix><slot>), or <slot>_<component> for a SourceComponent::sameFileSection,
                // its sections renamed by IniNamingTools::getRemapFixName
                GraphGroupRemap<>::RemapList planTable() const {
                    const GraphGroupRemap<>::RenameFunc rename = [this](const std::string& name) { return remapFixName(name); };
                    GraphGroupRemap<>::RemapList remap;
                    for (const auto& [component, planned] : config_.plan) {
                        const std::string slot = WWMINames::SlotPrefix + std::to_string(planned.slot);
                        const ModObj to(toModName_, planned.sameFileSection ? slot + "_" + std::to_string(component) : slot);
                        remap.emplace_back(GraphId(0, "", WWMINames::SlotPrefix + std::to_string(component)),
                                           std::vector<GraphGroupRemap<>::RemapTarget>{GraphGroupRemap<>::RemapTarget(GraphId(0, to.first, to.second),
                                                                                                                      rename)});
                    }
                    return remap;
                }

                // THE SLOT REMAP OF THIS MOD (the "slotRemap" step's table): 'table' is the plan's (planTable), and this mod
                // adds to it:
                //
                //   * each reroute it draws: its source component's graph again, onto the reroute's target, under the
                //     reroute's own names so the two copies do not collide in the mod's file
                //   * each component whose section draws nothing (readMod): no target, so it goes nowhere -- left out, its
                //     graph would be rendered into the fix block under the mod's own section names
                //   * each of its other hashed objects (the bone-data marker, the shape-key overrides): onto itself, once
                //     per file (see below), unless the config hides it
                //
                // Every section is named after the TARGET, so a target never collides with a source still to be moved;
                // two claimants of one slot collide with each other, which is what makes the further groups
                // (GraphGroupRemap's own rule).
                GraphGroupRemap<>::RemapList slotRemapTable(const GraphGroupRemap<>::RemapList& table) const {
                    const GraphGroupRemap<>::RenameFunc rename = [this](const std::string& name) { return remapFixName(name); };
                    const auto own = [&table](const ModObj& src) {
                        return std::find_if(table.begin(), table.end(), [&src](const auto& entry) {
                            return entry.first.iniIndex == 0 && entry.first.modObj == src;
                        });
                    };

                    std::set<std::size_t> used;
                    GraphGroupRemap<>::RemapList remap;
                    for (const auto& entry : drawnInfo_.present) {
                        if (drawnInfo_.rerouteFrom.count(entry.first) > 0) {
                            continue;                       // a target of its source's graph, below
                        }

                        const ModObj src = slotObj(entry.first);
                        std::vector<GraphGroupRemap<>::RemapTarget> targets;
                        if (const auto found = own(src); found != table.end()) {
                            targets = found->second;
                            used.insert(static_cast<std::size_t>(found - table.begin()));
                        }

                        for (const auto& [as, from] : drawnInfo_.rerouteFrom) {
                            if (from != entry.first) {
                                continue;
                            }

                            const ModObj target = targetObjOf(as);
                            const std::string tag = WWMINames::Reroute + std::to_string(as);
                            targets.emplace_back(GraphId(0, target.first, target.second),
                                                 [this, tag](const std::string& name) { return remapFixName(name + tag); });
                        }

                        remap.emplace_back(GraphId(0, src.first, src.second), std::move(targets));
                    }

                    // the table's other entries -- components this mod does not have, a user's own -- as they are; a
                    // reroute's graph is its source's, and a silent component's goes nowhere (below)
                    for (std::size_t i = 0; i < table.size(); ++i) {
                        const int component = sourceComponent(table[i].first.modObj);
                        const bool silent = std::find(drawnInfo_.silent.begin(), drawnInfo_.silent.end(), component) != drawnInfo_.silent.end();
                        if (used.count(i) == 0 && !silent && drawnInfo_.rerouteFrom.count(component) == 0) {
                            remap.push_back(table[i]);
                        }
                    }

                    for (int component : drawnInfo_.silent) {
                        const ModObj src = slotObj(component);
                        remap.emplace_back(GraphId(0, src.first, src.second), std::vector<GraphGroupRemap<>::RemapTarget>{});
                    }

                    for (const ModObj& obj : hashOnlyObjs_) {
                        if (own(obj) != table.end()) {
                            continue;                       // the table's own, above
                        }

                        std::vector<GraphGroupRemap<>::RemapTarget> targets;
                        if (hiddenObjs_.count(obj.second) == 0) {
                            // ONCE, in the mod's own file, when the target is past 256 bones
                            // (2026-10-06). These are the bone-data marker and the shape-key
                            // overrides: sections on the GAME's hashes that set WWMI's shared state
                            // and skip the game's own shape-key dispatch. A copy repeating them runs
                            // the shape-key pipeline a second time, into its own buffers, on the same
                            // dispatch -- LynaePeppermint -> Lynae with two source components on her
                            // jacket slot drew the whole model as giant polygons until the copy's
                            // were removed, and drew it correctly without them. The marker is a
                            // filter on the game's buffer, so one file setting it serves every file.
                            //
                            // AND THE SHAPE-KEY OVERRIDES ONCE WHATEVER THE TARGET (2026-10-10): the
                            // reason above is about the shape-key pipeline, not about bones, and a fix
                            // that keeps the mod's shape keys (hiddenObjs empty) onto a target under 256
                            // bones -- Sanhua's -- writes copies too
                            const bool shapeKeys = obj.second == WWMINames::ShapeKeyOffsetsObj || obj.second == WWMINames::ShapeKeyScaleObj;
                            const std::size_t copies = shapeKeys ? std::size_t(1) : groups_.size();
                            for (std::size_t g = 0; g < copies; ++g) {
                                targets.emplace_back(GraphId(0, obj.first, obj.second), rename);
                            }
                        }

                        remap.emplace_back(GraphId(0, obj.first, obj.second), std::move(targets));
                    }

                    return remap;
                }

                // The component a source slot object names (<slotPrefix><N>), or -1 for any other object
                int sourceComponent(const ModObj& obj) const {
                    return WWMIParserConfig::slotWithPrefix(obj, WWMINames::SlotPrefix).value_or(-1);
                }

                // The edits, per group, keyed by the target objects the remap created.
                // THE PER-OBJECT EDITS, ONE NAMED STEP PER KIND (ObjEditKinds), each a GraphGroupEdit keyed by the target
                // object a source component is remapped onto -- per group, so each copy's objects get theirs. An object's
                // edits run in kind order, as they would in one edit.
                void buildPerGroupEdits() {
                    const auto partsOf = [this](const std::string& kind, int component) -> std::vector<PartEdit*> {
                        if (const auto built = objEdits_.find(kind); built != objEdits_.end()) {
                            const auto found = built->second.find(component);
                            return found == built->second.end() ? std::vector<PartEdit*>{} : found->second;
                        }
                        const auto one = [](PartEdit* edit) { return edit == nullptr ? std::vector<PartEdit*>{} : std::vector<PartEdit*>{edit}; };
                        if (kind == "removedLines") {
                            const auto remove = removeAdapters_.find(component);
                            return remove == removeAdapters_.end() ? std::vector<PartEdit*>{} : one(remove->second.get());
                        }
                        if (kind == "rabbitFXPins") {
                            return rabbitFXFlags_.count(component) > 0 ? one(rabbitFXPinAdapters_.at(component).get()) : std::vector<PartEdit*>{};
                        }
                        if (kind == "slotValues") {
                            return one(newValsOf_.at(config_.plan.at(component).slot));
                        }
                        if (kind == "hashes") {
                            return one(assetAdapter_.get());
                        }
                        if (kind == "dispatchSize") {
                            return one(dispatchAdapter_.get());
                        }
                        if (kind == "shapeKeyLists") {
                            return one(shapeKeyAdapter_.get());
                        }
                        if (kind == "afterShaders") {
                            return afterAdapters_.count(component) > 0 ? one(afterAdapters_.at(component).get()) : std::vector<PartEdit*>{};
                        }
                        if (kind == "markLines") {           // before the reroute: it reads the drawindexed
                            return markLines_.count(component) > 0 ? one(markAdapter_.get()) : std::vector<PartEdit*>{};
                        }
                        if (kind == "overlays") {            // SourceComponent::overlay, the same way
                            return overlays_.count(component) > 0 ? one(overlayAdapter_.get()) : std::vector<PartEdit*>{};
                        }
                        if (kind == "rerouteShaders") {      // last: every anchor above reads the draw
                            return rerouteShaders_.count(component) > 0 ? one(rerouteShaderAdapter_.get()) : std::vector<PartEdit*>{};
                        }
                        if (kind == "offPasses") {           // SourceComponent::offPasses
                            return offAdapters_.count(component) > 0 ? one(offAdapters_.at(component).get()) : std::vector<PartEdit*>{};
                        }
                        return {};
                    };
                    // what a non-slot object of the mod (hashOnlyObjs_) gets: its hashes, and the shape-key settings
                    const std::set<std::string> objKinds{"hashes", "dispatchSize", "shapeKeyLists"};

                    kindEdits_.clear();
                    for (const std::string& kind : ObjEditKinds) {
                        std::vector<ObjGroupEdit::IniEdits> perGroup(groups_.size());
                        bool any = false;
                        for (std::size_t g = 0; g < groups_.size(); ++g) {
                            for (int component : groups_[g]) {
                                std::vector<PartEdit*> parts = partsOf(kind, component);
                                if (parts.empty()) {
                                    continue;
                                }
                                const ModObj obj = targetObjOf(component);
                                perGroup[g].edits[obj] = std::move(parts);
                                perGroup[g].trackKeys[obj] = false;
                                any = true;
                            }
                            if (objKinds.count(kind) == 0) {
                                continue;
                            }
                            for (const ModObj& obj : hashOnlyObjs_) {
                                if (hiddenObjs_.count(obj.second) > 0) {
                                    continue;
                                }
                                const std::vector<PartEdit*> parts = partsOf(kind, -1);     // the same edit for every object
                                if (!parts.empty()) {
                                    perGroup[g].edits[obj] = parts;
                                    perGroup[g].trackKeys[obj] = false;
                                    any = true;
                                }
                            }
                        }
                        if (any) {
                            auto edit = std::make_unique<ObjGroupEdit>(std::move(perGroup), false);
                            edit->name = kind;
                            kindEdits_[kind] = std::move(edit);
                        }
                    }
                }

                // an object edit of 'kind' for 'component' (see ObjEditKinds)
                void objEdit(const std::string& kind, int component, PartEdit* edit) {
                    objEdits_[kind][component].push_back(edit);
                }

                // The blend: the register bound in the shared override, collected out of every
                // drawn slot section -- one collect PER GROUP, so every copy declares the
                // RemapBlend resource its own sections bind, as a GIMI merge's copies do (a
                // collect is addressed by GraphId, whose iniIndex is the group).
                void buildBlendCollects(const ModType* source, const std::optional<Version>& from,
                                         const std::optional<Version>& to) {
                    for (std::size_t g = 0; g < groups_.size(); ++g) {
                        std::function<bool(RemapBlendResource&)> lift;
                        if (targetPast256_) {
                            // The target cannot be named by an 8-bit blend index, so the fix writes
                            // WWMI's blend remap. Takes precedence over both lifts below: each of
                            // them writes 8-bit ids only, which for this target is the silent
                            // truncation this exists to stop (81004 weighted slots of one real mod,
                            // not one of them landing on the bone the row asks for).
                            const std::string vgRel = FileService::iniPathToRel(vertexVGFile().value_or(""));
                            const std::string vgPath = vgRel.empty()
                                ? std::string()
                                : FileService::absPathOfRelPath(vgRel, ctx_.getIniFile()->getFolder());
                            const std::string posPath =
                                positionBufPath();
                            const WWMIBlend::RemapFiles out{
                                FileService::absPathOfRelPath(blendRemapFile("VertexVG"), ctx_.getIniFile()->getFolder()),
                                FileService::absPathOfRelPath(blendRemapFile("Forward"), ctx_.getIniFile()->getFolder()),
                                FileService::absPathOfRelPath(blendRemapFile("Reverse"), ctx_.getIniFile()->getFolder())};
                            std::optional<WWMIBlend::LegacyLift> legacy;
                            if (drawnInfo_.legacy) {
                                legacy = WWMIBlend::LegacyLift{FileService::absPathOfRelPath(modBuffer(IniKeywords::WWMIIndexBuffer), ctx_.getIniFile()->getFolder()),
                                                               drawnInfo_.drawRanges, config_.sourceVgMaps};
                            }

                            lift = [vgPath, posPath, out, legacy](RemapBlendResource& resource) {
                                return orBail(resource, [&] {
                                    WWMIBlend::writeRemap(resource.srcPath, resource.fixedPath, vgPath, posPath,
                                                          resource.vgRemap, out, legacy);
                                });
                            };
                        } else if (const std::optional<std::string> vertexVG = vertexVGFile()) {
                            const std::string vgRel = FileService::iniPathToRel(*vertexVG);
                            const std::string vgPath =
                                FileService::absPathOfRelPath(vgRel, ctx_.getIniFile()->getFolder());
                            const std::string posPath =
                                positionBufPath();
                            lift = [vgPath, posPath](RemapBlendResource& resource) {
                                return orBail(resource, [&] {
                                    WWMIBlend::remapThroughVertexVG(resource.srcPath, resource.fixedPath, vgPath, posPath,
                                                                    resource.vgRemap);
                                });
                            };
                        } else if (drawnInfo_.legacy) {
                            const std::string indexPath = FileService::absPathOfRelPath(modBuffer(IniKeywords::WWMIIndexBuffer), ctx_.getIniFile()->getFolder());
                            const std::string positionPath =
                            positionBufPath();
                            const WWMIBlend::LegacyLift legacy{indexPath, drawnInfo_.drawRanges, config_.sourceVgMaps};
                            lift = [positionPath, legacy](RemapBlendResource& resource) {
                                return orBail(resource, [&] {
                                    WWMIBlend::liftLegacy(resource.srcPath, resource.fixedPath, positionPath, legacy,
                                                          resource.vgRemap);
                                });
                            };
                        }

                        BaseResEdit<>::ResEditConfig resConfig = resEditConfig();

                        // The library's vertex-group row over the WWMI layout: the line is N ids then N
                        // weights, N read off the mod's own Blend.buf
                        auto replace = std::make_unique<VGRemapBlendReplace<>>(GraphId(g, "", "blend"), std::move(resConfig), source, from, to);
                        replace->fixFunc = std::move(lift);
                        replace->anchorChains = config_.anchorChains;

                        // A target whose draws read a different number of influences than the mod carries
                        // (WWMIFixerConfig::targetInfluences): whatever wrote the blend above, it is then made the
                        // target's width -- padded, or each vertex keeping its heaviest slots -- and the generated
                        // section, a copy of the mod's own, is told the new stride. Past 256 bones the blend's ids
                        // live in BlendRemapVertexVG.buf, which names the bones and keeps the same slots.
                        replace->influences = config_.targetInfluences;
                        if (targetPast256_) {
                            const std::string vgOut = FileService::absPathOfRelPath(blendRemapFile("VertexVG"), ctx_.getIniFile()->getFolder());
                            replace->pairedIds = [vgOut](const std::string&) {
                                return std::vector<VGRemapBlendReplace<>::PairedIds>{{vgOut, 2}};
                            };
                        }
                        replace->elements = [vertices = meshVertexCount_, positionPath = positionBufPath()](const std::string& srcPath) {
                            return WWMIBlend::blendElements(WWMIBlend::influencesOf(srcPath, vertices, positionPath));
                        };
                        auto collect = std::make_unique<Collector>();
                        for (int component : groups_[g]) {
                            const ModObj obj = targetObjOf(component);
                            collect->srcRegs[GraphId(g, obj.first, obj.second)] = IniKeywords::WWMIBlendReg;

                            // A MOD OF A CHARACTER PAST 256 BONES BINDS THE BLEND REGISTER TWICE:
                            // `vb4 = ResourceBlendBuffer` when no blend remap is active, and
                            // `vb4 = ref ResourceBlendBufferOverride` when one is. Only the first
                            // names a FILE -- the second is a buffer WWMI's BlendRemapper fills at
                            // load -- so take that one and leave the other. Without this the run
                            // dies looking for a section called `Resourceref Resource...` and the
                            // whole mod is skipped.
                            //
                            // A mod that binds it once, plainly, is unaffected: the predicate passes.
                            //
                            // ...and a component drawn from its OWN mesh binds its own blend, which this
                            // collect cannot remap (its vertex count is not the mesh's): see settleOwnMeshes.
                            const Collector::ResPredicate keywords = Collector::skipsKeywords();
                            std::set<std::string> ownBlends;
                            for (const auto& [ownComponent, mesh] : ownMeshes_) {
                                ownBlends.insert(StringTools::toLower(mesh.blendResource));
                                ownBlends.insert(StringTools::toLower(ownMeshBlendResource(ownComponent)));
                            }
                            collect->resPredicates[GraphId(g, obj.first, obj.second)] = ownBlends.empty() ? keywords
                                : Collector::ResPredicate([keywords, ownBlends](const std::string& key, const std::string& value,
                                                                                const Collector::IterData& data) {
                                      return keywords(key, value, data)
                                          && ownBlends.count(StringTools::toLower(std::string(StringTools::strip(value)))) == 0;
                                  });
                        }

                        collect->resEdits = {{BlendResGroupMember, replace.get()}};
                        blendReplaces_.push_back(std::move(replace));
                        blendCollects_.push_back(std::move(collect));
                    }

                    buildBlendRebind();
                }

                // A MOD OF A CHARACTER PAST 256 BONES BINDS THE BLEND REGISTER TWICE: `vb4 =
                // ResourceBlendBuffer` (a file) and `vb4 = ref ResourceBlendBufferOverride` (a buffer
                // WWMI's BlendRemapper fills at load). The collect takes only the first -- the second
                // names no file, and collecting it kills the run looking for a section called
                // `Resourceref Resource...` -- so without this the second keeps pointing at the
                // SOURCE's own override, which is an inverse of the whole remap.
                //
                // So every `ref` value of the blend register is rebound to the remapped blend: the name
                // the blend edit gives the one file the mod binds there (every such mod of the WuWa
                // corpus binds one, 66 files, 2026-11-04). A mod binding several leaves its `ref` lines
                // alone. The edit is idempotent and changes no file binding, so it rides the per-group
                // edits on each drawn component's graph, which reaches the shared lists through `run =`.
                void buildBlendRebind() {
                    if (IniKeywords::WWMIBlendReg.empty() || blendReplaces_.empty()) {
                        return;
                    }

                    std::set<std::string> files;
                    for (const std::string& section : ownSections()) {
                        for (const Row* row : ownRows(section)) {
                            if (IniNamingTools::sameKey(row->key, IniKeywords::WWMIBlendReg)
                                    && !IniNamingTools::hasKeywordPrefix(row->value, IniKeywords::Ref)) {
                                files.insert(std::string(StringTools::strip(row->value)));
                            }
                        }
                    }

                    const std::optional<std::string> remapped = files.size() == 1
                        ? blendReplaces_.front()->getFixResourceName(*files.begin(), toModName_) : std::nullopt;
                    if (!remapped.has_value()) {
                        return;
                    }

                    rebind_ = std::make_unique<RegNewVals<>>(std::vector<std::pair<std::string, RegNewVals<>::NewValSpec>>{
                        {IniKeywords::WWMIBlendReg, std::make_pair(RegNewVals<>::NewVal(*remapped), RegNewVals<>::ModTypePredicate(
                            [](const std::string& oldValue, const ModType*) {
                                return IniNamingTools::hasKeywordPrefix(oldValue, IniKeywords::Ref);
                            }))}});
                    rebindAdapter_ = std::make_unique<RegPartEdit<>>(rebind_.get());

                    std::vector<ObjGroupEdit::IniEdits> perGroup(groups_.size());
                    for (std::size_t g = 0; g < groups_.size(); ++g) {
                        for (int component : groups_[g]) {
                            const ModObj obj = targetObjOf(component);
                            perGroup[g].edits[obj] = {rebindAdapter_.get()};
                            perGroup[g].trackKeys[obj] = false;
                        }
                    }
                    rebindEdits_ = std::make_unique<ObjGroupEdit>(std::move(perGroup), false);
                }

                // ---- what the WRITTEN text must say ----

                // THE TOGGLE LISTS ARE THE MOD'S OWN TEXTURE OVERRIDE, AS EDITS (2026-10-28). For each
                // toggle WWMIFixerConfig::toggleLists made:
                //   1. GraphCopy the parser's graph holding the override, keeping only that section's
                //      lines and none of 3dmigoto's matching keys (meaningless in a list)
                //   2. GraphRename it to the list's name
                //   3. RegRemove every `this` naming a resource that resolves to nothing, and RegRemap
                //      `this` to the register the target's draw reads
                //   4. for a one-variant toggle, RegFillMissing a top cover binding the game's texture
                //   5. take the section into the fix (TemplateFixer::takeFixSections)
                // and then GraphRemove every texture graph, which the fix never writes out -- they are
                // the mod's own sections, which its own file keeps.
                void buildToggleLists() {
                    toggleEdits_.clear();
                    std::vector<GraphCopy<>*> copies;
                    std::vector<Fixer::GroupEdit*> takes;
                    std::vector<ObjGroupEdit::IniEdits> perGroup(1);

                    for (const WWMIFixerConfig::ToggleList& list : textureLists_.toggles) {
                        const ModObj listObj(IniGraphModObjKeywords::Texture, list.name);
                        const std::string section = list.section;
                        auto copy = std::make_unique<GraphCopy<>>(
                            GraphId(0, list.graph.first, list.graph.second), GraphId(0, listObj.first, listObj.second),
                            [section](const IfTemplate<std::string, std::string>& owner, const IfContentPart<std::string, std::string>& part,
                                      std::size_t i) {
                                return owner.name == section
                                    && IniKeywords::MatchKeys.count(StringTools::toLower(part.getByInd(static_cast<long long>(i)).first)) == 0;
                            });
                        copies.push_back(copy.get());
                        toggleOwned_.push_back(std::move(copy));

                        auto rename = std::make_unique<GraphRename<>>([section, name = list.name](const std::string& old) {
                            return old == section ? name : old;
                        });

                        std::vector<std::pair<std::string, std::optional<RegRemove<>::RemoveKeyCheck>>> drops;
                        std::vector<std::pair<std::string, RegRemap<>::KeyRemapValue>> remaps;
                        const std::set<std::string> dead = list.deadValues;
                        const RegRemove<>::RemoveKeyCheck isDead = [dead](long long, const std::string& val) {
                            return dead.count(val) > 0;
                        };
                        for (const std::string& key : list.thisKeys) {
                            drops.emplace_back(key, isDead);
                            remaps.emplace_back(key, RegRemap<>::KeyRemapValue(RemapList<std::string, std::string>{list.reg}));
                        }
                        auto remove = std::make_unique<RegRemove<>>(std::move(drops));
                        auto remap = std::make_unique<RegRemap<>>(std::move(remaps));

                        std::vector<ObjGroupEdit::PartEdit*>& edits = perGroup[0].edits[listObj];
                        edits.push_back(adapt(std::make_unique<GraphPartEdit<>>(rename.get())));
                        edits.push_back(adapt(std::make_unique<RegPartEdit<>>(remove.get())));
                        edits.push_back(adapt(std::make_unique<RegPartEdit<>>(remap.get())));
                        toggleOwned_.push_back(std::move(rename));
                        toggleOwned_.push_back(std::move(remove));
                        toggleOwned_.push_back(std::move(remap));

                        if (list.offBinding.has_value()) {
                            auto cover = std::make_unique<RegFillMissing<>>(
                                list.reg, RegFillMissing<>::makeFillMissing(list.reg, *list.offBinding, true), RegFillMissingMode::TopdownCover);
                            cover->countMissingElse = true;     // the toggle binds only while it is on
                            edits.push_back(adapt(std::make_unique<GraphPartEdit<>>(cover.get())));
                            toggleOwned_.push_back(std::move(cover));
                        }
                        perGroup[0].trackKeys[listObj] = false;

                        takes.push_back(this->takeFixSections(listObj, {list.name}));
                    }

                    std::vector<GraphId> textureGraphs;
                    if (auto* parser = dynamic_cast<GIMIParser<>*>(this->getParser())) {
                        for (const ModObj& obj : parser->modObjs()) {
                            if (obj.first == IniGraphModObjKeywords::Texture) {
                                textureGraphs.emplace_back(0, obj.first, obj.second);
                            }
                        }
                    }

                    for (GraphCopy<>* copy : copies) {
                        toggleEdits_.push_back(copy);
                    }
                    if (!textureLists_.toggles.empty()) {
                        auto listEdits = std::make_unique<ObjGroupEdit>(std::move(perGroup), false);
                        toggleEdits_.push_back(listEdits.get());
                        toggleOwned_.push_back(std::move(listEdits));
                    }
                    for (Fixer::GroupEdit* take : takes) {
                        toggleEdits_.push_back(take);
                    }

                    auto removeTextures = std::make_unique<GraphRemove<>>(std::move(textureGraphs));
                    toggleEdits_.push_back(removeTextures.get());
                    toggleOwned_.push_back(std::move(removeTextures));
                }

                ObjGroupEdit::PartEdit* adapt(std::unique_ptr<ObjGroupEdit::PartEdit> adapter) {
                    toggleAdapters_.push_back(std::move(adapter));
                    return toggleAdapters_.back().get();
                }

                // The mod's own sections on another mesh of the source, copied onto the target's mesh
                // that plays the same part -- see WWMIFixerConfig::meshCounterparts
                void meshCounterpartSections() {
                    copyEdits_.clear();
                    for (const std::string& part : config_.meshCounterparts) {
                        const std::optional<std::string> hash = targetType_->hashes == nullptr
                            ? std::nullopt : targetType_->hashes->get({targetType_->name, part}, toVersion(), false);
                        if (!hash.has_value()) {
                            ctx_.log("the library files no '" + part + "' hash for " + toModName_ + ", so the mod's sections on "
                                     + source_.name + "'s are not carried over");
                            continue;
                        }

                        const ModObj obj("", part);
                        const std::vector<Fixer::GroupEdit*> copies = copyOntoHash(
                            obj, sectionsOf(obj), StringTools::toLower(*hash),
                            [this](const std::string& name) { return name + " of the mod, on " + toModName_ + "'s counterpart of that mesh"; },
                            true);
                        copyEdits_.insert(copyEdits_.end(), copies.begin(), copies.end());
                    }

                    // the parser's graphs the copies came from are the mod's own sections, which its own file keeps
                    std::vector<GraphId> copied;
                    if (auto* parser = dynamic_cast<GIMIParser<>*>(this->getParser())) {
                        for (const ModObj& obj : parser->modObjs()) {
                            if (isCopiedObj(obj)) {
                                copied.emplace_back(0, obj.first, obj.second);
                            }
                        }
                    }
                    auto removeCopied = std::make_unique<GraphRemove<>>(std::move(copied));
                    copyEdits_.push_back(removeCopied.get());
                    toggleOwned_.push_back(std::move(removeCopied));
                }

                // A graph of the parser's whose sections the fix copies rather than remaps: the sections that only
                // watch the source, and those of a mesh WWMIFixerConfig::meshCounterparts names
                bool isCopiedObj(const ModObj& obj) const {
                    return obj.first.empty() && (obj.second == WWMIParseObjs::Watch
                        || std::find(config_.meshCounterparts.begin(), config_.meshCounterparts.end(), obj.second) != config_.meshCounterparts.end());
                }

                const std::string* sharedResourceFor(const std::string& role) const {
                    for (const auto& entry : drawnInfo_.present) {
                        const std::string* resource = resourceFor(role, entry.first);
                        if (resource != nullptr) {
                            return resource;
                        }
                    }

                    return nullptr;
                }

                const std::string* resourceFor(const std::string& role, int component) const {
                    auto slotRole = resourceOfSlotRole_.find({role, component});
                    if (slotRole != resourceOfSlotRole_.end()) {
                        return &slotRole->second;
                    }

                    // a reroute drawn by its own shader reads the download itself, not a texture edit
                    // of it (see rerouteShaders_)
                    if (rerouteShaders_.count(component) > 0) {
                        const auto download = fallbacks_.find(role);
                        if (download != fallbacks_.end()) {
                            return &download->second.resource;
                        }
                    }

                    auto shared = resourceOfRole_.find(role);
                    return shared == resourceOfRole_.end() ? nullptr : &shared->second;
                }

                // The components a WWMI-Tools export name is tagged for: `Components-0-2 t=<hash>.dds`
                // is {0, 2}. Empty for a file named anything else

                // The shaders TAGGED for a pass: its own pixel shader by default, or the vertex
                // shaders it is drawn with -- see WWMIFixerConfig::passVertexShaders.
                std::vector<std::string> taggedFor(const std::string& pass) const {
                    if (config_.passVertexShaders.empty()) {
                        return {pass};
                    }

                    const auto at = config_.passVertexShaders.find(pass);
                    if (at == config_.passVertexShaders.end() || at->second.empty()) {
                        // Not a fallback to tagging the pixel shader: that would be silent, and it
                        // is the bug the map exists to avoid. A pass reaches here only when a
                        // compiled config is written wrong, so it is loud and immediate -- the same
                        // check the prototype makes with an assert.
                        throw std::runtime_error(
                            "WWMIFixer: pass " + pass + " has no vertex shader in passVertexShaders. "
                            "Read the pair off a frame dump's draw table; tagging its pixel shader "
                            "instead would switch RabbitFX off for every mod using that shader.");
                    }

                    return at->second;
                }

                // The `.ini` condition true on a draw of 'pass'
                // The condition for a set of passes: the OR over the shaders they are gated by,
                // each named ONCE. Two passes of one slot can share a vertex shader -- Chisa's bangs
                // and hair are both gated by d83a54772fc666f9 -- and joining their conditions wrote
                // `vs == X || vs == X`. Harmless, but it is noise in every diff and it would hide the
                // real version of the same shape: two DIFFERENT shaders resolving to one index.
                std::string passCondition(const std::vector<std::string>& passes) {
                    passFilter("");
                    return passTags_.condition(config_.passVertexShaders.empty() ? IniKeywords::Ps : IniKeywords::Vs, taggedFor(passes));
                }

                // The shaders tagged for any of 'passes', in order
                std::vector<std::string> taggedFor(const std::vector<std::string>& passes) const {
                    std::vector<std::string> tagged;
                    for (const std::string& pass : passes) {
                        for (const std::string& shader : taggedFor(pass)) {
                            tagged.push_back(shader);
                        }
                    }
                    return tagged;
                }

                std::string passCondition(const std::string& pass) {
                    return passCondition(std::vector<std::string>{pass});
                }

                // The `.ini` condition true on a draw of none of 'passes' -- see SourceComponent::offPasses
                std::string passNotCondition(const std::vector<std::string>& passes) {
                    passFilter("");
                    return passTags_.condition(config_.passVertexShaders.empty() ? IniKeywords::Ps : IniKeywords::Vs, taggedFor(passes), true);
                }

                // A component's draw, kept off the passes its SourceComponent::offPasses names: a `run =` of a
                // list that draws it only off them (its section written by rerouteShaderSections)
                Kvp offDraw(int component, const std::pair<long long, long long>& range) {
                    const std::string draw = std::to_string(range.first) + ", " + std::to_string(range.second) + ", 0";
                    const auto planned = config_.plan.find(component);
                    if (planned == config_.plan.end() || planned->second.offPasses.empty()) {
                        return {IniKeywords::DrawIndexed, draw};
                    }

                    const std::string condition = passNotCondition(planned->second.offPasses);
                    if (condition.empty()) {
                        return {IniKeywords::DrawIndexed, draw};
                    }

                    const std::string name = remapFixName(IniKeywords::CommandList + WWMINames::OffPass + std::to_string(component) + WWMINames::Draw
                                                     + std::to_string(range.first) + "_" + std::to_string(range.second));
                    offLists_[name] = {condition, draw};
                    return {IniKeywords::Run, name};
                }

                std::string passFilter(const std::string& pass) {
                    // One filter_index per distinct shader, in order of first appearance over the
                    // slots -- except for a shader config.filterIndices names, which takes the value
                    // it is given there. 3dmigoto keys a [ShaderOverride] by its shader hash across
                    // EVERY loaded .ini, so two pairs that tag the same shader (both directions of one
                    // pair always do: the hair, face and eye shaders are the same on a skin and its
                    // character) must agree on its value, or whichever file loads last wins and the
                    // other mod's `if ps == ...` never matches -- its textures silently unbound.
                    if (!passTagsBuilt_) {
                        passTagsBuilt_ = true;
                        // every pass either table names -- an extraPassRegs pass with no
                        // filter_index has nothing to match on, so its command list never fires and
                        // that slot draws with the game's textures on that pass
                        std::vector<std::vector<std::string>> allPasses = config_.slotPasses;
                        for (const auto& [mesh, byPass] : config_.sharedMeshes) {
                            (void)mesh;
                            allPasses.push_back(passNamesOf(byPass));
                        }

                        for (const auto& [slot, byPass] : config_.extraPassRegs) {
                            (void)slot;
                            allPasses.push_back(passNamesOf(byPass));
                        }

                        // each pass's tagged shaders, in that order, each with its filter_index (refused when it has none)
                        std::vector<std::string> shaders;
                        for (const auto& passes : allPasses) {
                            for (const std::string& shader : taggedFor(passes)) {
                                shaders.push_back(shader);
                            }
                        }
                        passTags_.addGiven(shaders, {config_.filterIndices.begin(), config_.filterIndices.end()});
                    }

                    return passTags_.filterIndexOf(pass).value_or("");
                }

                // The merge list the fix supplies per target slot for a legacy mod
                std::string mergeListName(int slot) const {
                    return WWMISections::mergeListName(runtimeSkeleton(), slot);
                }

                // What WWMISections writes the fix's skeleton and blend remap from
                WWMISections::Skeleton runtimeSkeleton() const {
                    WWMISections::Skeleton skeleton;
                    skeleton.name = [this](const std::string& name) { return remapFixName(name); };
                    skeleton.cb4Hash = target_.cb4Hash;
                    skeleton.mergedSkeletonSlots = config_.mergedSkeletonSlots;
                    skeleton.remapped = targetPast256_;
                    skeleton.latched = drawnInfo_.hostResetsDetected;
                    skeleton.currentPoseInCb3Only = config_.currentPoseInCb3Only;
                    skeleton.blendRemapBones = blendRemapBones_;
                    skeleton.blendRemapForward = remapFixName(IniKeywords::WWMIBlendRemapForwardBuffer);
                    for (const Slot& slot : target_.slots) {
                        skeleton.slotWindows.emplace_back(slot.vgOffset, slot.vgCount);
                    }
                    return skeleton;
                }

                WWMISections::BlendRemap runtimeBlendRemap() const {
                    WWMISections::BlendRemap blendRemap;
                    blendRemap.name = [this](const std::string& name) { return remapFixName(name); };
                    blendRemap.blendFile = blendFixedFile();
                    blendRemap.remapFile = [this](const std::string& which) { return blendRemapFile(which); };
                    blendRemap.blendStride = blendStride_;
                    blendRemap.influences = blendInfluences_;
                    blendRemap.blendReg = IniKeywords::WWMIBlendReg;
                    return blendRemap;
                }

                // The skeleton's sections (WWMISections::skeletonSections), and the blend remap's after them
                void legacySkeletonSections() {
                    for (auto& section : WWMISections::skeletonSections(z3_, runtimeSkeleton())) {
                        addFixSection("skeletonSections", std::move(section));
                    }

                    if (targetPast256_) {
                        blendRemapSections();
                    }
                }

                // The blend remap's resources, and the one-time run of WWMI's BlendRemapper that
                // rewrites the remapped blend's ids into the remap's LOCAL space. Guarded by a
                // `local`, so it costs one dispatch on the first drawn frame and nothing after.
                //
                // The strided view is the fix's own declaration of the same file rather than the
                // collect's resource: `copy_desc` needs a buffer with the blend's stride, and
                // declaring it here keeps this text independent of what the collect happened to
                // name its resource.
                void blendRemapSections() {
                    for (auto& section : WWMISections::blendRemapSections(z3_, runtimeBlendRemap())) {
                        addFixSection("blendRemapSections", std::move(section));
                    }
                }

                // ---- the fix's own sections ----

                void buildAppended() {
                    for (const auto& entry : declared_) {
                        // a declared file with no path given relative to the .ini (DeclaredFile::rel) is put there
                        const std::string rel = !entry.second.empty() ? entry.second
                            : FileService::pathToIniStr(FileService::strToPath(
                                  FileService::getRelPath(entry.first, ctx_.getIniFile()->getFolder())));
                        addFixSection("declaredFiles", SectionText(z3_, declaredName_[entry.first])
                                   .key(IniKeywords::Filename, rel)
                                   .build());
                    }

                    // its OWN name -- see Fallback::resource. resourceOfRole_ may by now be the
                    // resource of a texture EDIT of this role, and writing the download's section
                    // under that name declares it twice, raw file and edited file.
                    for (const auto& entry : fallbacks_) {
                        addFixDownload(entry.second);
                    }

                    // ...and the same for a role the mod DOES serve, downloaded for the draws that
                    // happen before its section binds anything -- see IniParseData::lateBindings.
                    for (const auto& entry : gameFallbacks_) {
                        addFixDownload(entry.second);
                    }

                    // the zero shape-key stream: a buffer of zeros, one line a vertex
                    if (config_.zeroShapeKeyStream && meshVertexCount_ > 0) {
                        const std::size_t size = static_cast<std::size_t>(meshVertexCount_) * static_cast<std::size_t>(WWMINames::ShapeKeyStreamStride);
                        addFixFile(remapFixName(IniKeywords::Resource + ShapeKeyZero),
                                   {{IniKeywords::Type, IniKeywords::Buffer},
                                    {IniKeywords::Format, IniKeywords::FormatR32G32B32Float},
                                    {IniKeywords::Stride, std::to_string(WWMINames::ShapeKeyStreamStride)}},
                                   zeroStreamFile(), [size]() { return ByteVec(size, 0); }, {IniKeywords::WWMIShapeKeyStreamReg});
                    }

                    // see WWMIFixerConfig::shapeKeyOrder: the mod's own declarations, on the reordered copies
                    if (!shapeKeyFiles_.empty()) {
                        auto reordered = std::make_shared<std::optional<WWMIShapeKeys::Bytes>>();
                        const auto bytesOf = [this, reordered](std::size_t which) {
                            if (!reordered->has_value()) {
                                *reordered = WWMIShapeKeys::reordered(shapeKeyFiles(false), config_.shapeKeyOrder);
                            }
                            const WWMIShapeKeys::Bytes& bytes = **reordered;
                            return which == 0 ? bytes.offsets : which == 1 ? bytes.vertexIds : bytes.vertexOffsets;
                        };

                        for (std::size_t which = 0; which < ShapeKeyResources.size(); ++which) {
                            const std::string& resource = ShapeKeyResources[which];
                            const ShapeKeyFile& file = shapeKeyFiles_.at(resource);
                            Kvps keys{{IniKeywords::Type, file.type}};
                            if (!file.format.empty()) {
                                keys.emplace_back(IniKeywords::Format, file.format);
                            }
                            if (!file.stride.empty()) {
                                keys.emplace_back(IniKeywords::Stride, file.stride);
                            }
                            addFixFile(remapFixName(resource), std::move(keys), shapeKeyOrderFile(resource),
                                       [bytesOf, which]() { return bytesOf(which); },
                                       {IniKeywords::WWMIShapeKeyOffsetReg, IniKeywords::WWMIShapeKeyVertexIdReg, IniKeywords::WWMIShapeKeyVertexOffsetReg});
                        }
                    }

                    // the bypass pair's resources and its restore list (WWMISections::bypassSections)
                    for (auto& section : WWMISections::bypassSections(z3_, bypasses_, restoreListName())) {
                        addFixSection("bypassSections", std::move(section));
                    }


                    for (std::size_t slot = 0; slot < target_.slots.size(); ++slot) {
                        if (drawnSlots_.count(static_cast<int>(slot)) > 0) {
                            continue;
                        }

                        const Slot& s = target_.slots[slot];
                        auto label = config_.targetLabels.find(static_cast<int>(slot));
                        const std::string labelText = label == config_.targetLabels.end() ? std::to_string(slot) : label->second;
                        WWMISections::Hide hide;
                        hide.name = IniKeywords::TextureOverride + toModName_ + TextTools::capitalize(WWMINames::SlotPrefix)
                                    + std::to_string(slot) + IniKeywords::Remap + "Hide";
                        hide.comment = "; nothing of the mod is drawn through " + toModName_ + "'s " + labelText
                                       + " slot: the skin's own geometry is skipped and its bones still merged";
                        hide.vb0Hash = target_.vb0Hash;
                        hide.indexOffset = s.indexOffset;
                        hide.indexCount = s.indexCount;
                        hide.vgOffset = s.vgOffset;
                        hide.vgCount = s.vgCount;
                        hide.slot = std::to_string(slot);
                        hide.legacyMergeList = drawnInfo_.legacy ? mergeListName(static_cast<int>(slot)) : std::string();
                        hide.mergeWindowList = targetPast256_ ? remapFixName(WWMINames::MergeWindowList) : std::string();
                        hide.mergeSkeletonList = remapFixName(WWMINames::MergeSkeletonList);

                        // WWMIFixerConfig::stashRegs: the game's own bindings on this slot, for a shader
                        // drawn on another
                        auto stash = config_.stashRegs.find(static_cast<int>(slot));
                        if (stash != config_.stashRegs.end() && !stashRead(static_cast<int>(slot))) {
                            stash = config_.stashRegs.end();      // no shader drawn for this mod reads it
                        }
                        if (stash != config_.stashRegs.end()) {
                            for (const std::string& reg : stash->second) {
                                hide.stashed.emplace_back(stashResource(static_cast<int>(slot), reg), IniKeywords::Ref + " " + reg);
                            }
                        }

                        addFixSection("hideSections", WWMISections::hideSection(z3_, hide));

                        if (stash != config_.stashRegs.end()) {
                            for (const std::string& reg : stash->second) {
                                addFixSection("stashResources", SectionText(z3_, stashResource(static_cast<int>(slot), reg)).build());
                            }
                        }
                    }

                    if (drawnInfo_.legacy || targetPast256_) {
                        legacySkeletonSections();
                    }

                    passFilter("");
                    for (std::size_t i = 0; i < passTags_.tags().size(); ++i) {
                        addFixSection("passTags", passTags_.section(i, remapFixName(WWMINames::ShaderOverridePass + std::to_string(i))));
                    }

                    // Other meshes the character draws -- see WWMIFixerConfig::sharedMeshes
                    std::size_t meshNum = 0;
                    for (const auto& [meshHash, byPass] : config_.sharedMeshes) {
                        SectionText mesh(z3_, remapFixName(IniKeywords::TextureOverride + source_.name
                                                      + "SharedMesh" + std::to_string(meshNum)));
                        mesh.key(IniKeywords::Hash, meshHash);

                        bool drew = false;
                        int passNum = 0;
                        for (const auto& [pass, bindings] : byPass) {
                            (void)bindings;
                            const std::vector<Kvp> lines = textureLinesOf("sharedMesh", static_cast<int>(meshNum), passNum++).lines;
                            if (!lines.empty()) {
                                mesh.open(passCondition(pass)).keys(lines).close();
                                drew = true;
                            }
                        }

                        if (drew) {
                            addFixSection("sharedMeshes", mesh.build());
                        }

                        ++meshNum;
                    }

                    // The target's meshes the mod replaces -- see WWMIFixerConfig::hiddenMeshes
                    for (std::size_t i = 0; i < config_.hiddenMeshes.size(); ++i) {
                        addFixSection("hiddenMeshes", SectionText(z3_, remapFixName(IniKeywords::TextureOverride + toModName_
                                                                    + "HiddenMesh" + std::to_string(i)))
                                          .prefix("; a mesh of " + toModName_ + "'s own outfit the mod replaces: skipped")
                                          .key(IniKeywords::Hash, config_.hiddenMeshes[i])
                                          .open(IniKeywords::WWMIModEnabled)
                                          .key(IniKeywords::Handling, IniKeywords::Skip)
                                          .close()
                                          .build());
                    }

                    meshCounterpartSections();



                    // a texture of one colour for each role the fix invents -- see WWMIFixerConfig::createdTextures
                    fixTextures_.clear();
                    for (const WWMIFixerConfig::CreatedTexture& created : config_.createdTextures) {
                        fixTextures_.push_back(FixTexture{resourceOfRole_[created.role], createdTextureFile(created),
                                                          TexCreator(created.size, created.size, created.colour, false, false)});
                    }


                    rerouteShaderSections();
                    rabbitFXPinSections();

                    // The fix's sections merge too -- a hidden slot's -- and
                    // latchEdits reaches them as it does the mod's. A hide section left out would set
                    // `$object_detected` itself, and on a frame where it runs first the remap would not run
                }

                // The three blend remap buffers, named so the undo takes them: a file the fix
                // wrote carries the target's name and `Remap`, and the resource section that names
                // it lives inside the fix's block.
                std::string blendRemapInitList() const {
                    return WWMISections::blendRemapInitList(runtimeBlendRemap());
                }

                // The remapped blend as the collect writes it, which is what the BlendRemapper reads
                // ASK FOR THE NAME, DO NOT ASSUME IT. The blend the fix writes goes through the
                // resource machinery, which names it from the MOD's own file
                // (IniNamingTools::getFixedBlendFile: the file's stem with the element keyword
                // replaced, or appended when the stem does not contain it). For an ordinary mod
                // whose blend is `Meshes/Blend.buf` that lands on `Meshes\<Mod>RemapBlend.buf`,
                // which is what this used to hardcode -- and for a MOD-MANAGER-PACKAGED mod, whose
                // buffers are GUIDs under an `.assets` extension, it lands on
                // `Meshes\<guid><Mod>RemapBlend.buf` and the hardcoded name names nothing.
                //
                // The 2026-09-28 audit built exactly that mod (chisaParfaitSynth.py's SynthPackaged)
                // and found 2 of its 88 references dangling -- both of them the blend, which is the
                // one buffer every draw needs. In game that is the 2026-09-25 symptom: a packaged
                // mod rendering nothing but its weapon. No real ChisaParfait mod is packaged, so
                // nothing in hand could have shown it.
                std::string blendFixedFile() const {
                    return IniNamingTools::getFixedBlendFile(blendSourceFile_, toModName_);
                }

                std::string blendRemapFile(const std::string& which) const {
                    return modFile(meshFolder_, IniNamingTools::getRemapElementName("", WWMINames::BlendRemap + which, toModName_) + ".buf");
                }

                // The reordered copy of one of the mod's shape-key buffers, beside the original
                std::string shapeKeyOrderFile(const std::string& resource) const {
                    std::string name = resource.substr(IniKeywords::Resource.size());
                    if (StringTools::endsWith(name, "Buffer")) {
                        name = name.substr(0, name.size() - std::string("Buffer").size());
                    }
                    const std::string folder = FileService::parentOf(FileService::iniPathToRel(shapeKeyFiles_.at(resource).path));
                    return modFile(folder, IniNamingTools::getRemapElementName("", name, toModName_) + ".buf");
                }

                std::string zeroStreamFile() const {
                    return modFile(meshFolder_, IniNamingTools::getRemapElementName("", ShapeKeyZero, toModName_) + ".buf");
                }

                std::string createdTextureFile(const WWMIFixerConfig::CreatedTexture& created) const {
                    return IniNamingTools::getModSuffixedName(created.role, IniKeywords::RemapTex, toModName_) + FileExt::DDS;
                }

                // ---- the fix's own files: each made by a FileAddCreate, collected where the fix binds it ----

                // One file the fix makes: the section naming it (its keys, then `filename =`), its bytes,
                // made when the file is written, and the registers that bind it
                void addFixFile(const std::string& section, Kvps keys, const std::string& file,
                                std::function<ByteVec()> makeBytes, std::initializer_list<std::string> regs) {
                    FileAddCreate<>::File entry;
                    entry.keys = std::move(keys);
                    entry.file = file;
                    entry.makeBytes = std::move(makeBytes);
                    fixFiles_[section] = std::move(entry);
                    fixFileRegs_.insert(regs.begin(), regs.end());
                }

                // Collected from every graph of the group, after the group edits have bound them -- which
                // declares each file's section and hands the service the resource that writes it, so the
                // file is written, counted and undone with the rest of the mod's
                IterEdit::GroupEdits fixFileEdits(std::size_t g, Fixer::GraphGroups& groups) {
                    IterEdit::GroupEdits result;
                    if (!fixFiles_.empty()) {
                        const std::vector<std::string> regs(fixFileRegs_.begin(), fixFileRegs_.end());
                        auto create = std::make_unique<FileAddCreate<>>(GraphId(g, "", "fixFiles"), fixFiles_, resEditConfig());
                        result.edits.push_back(collectNamed(groups, g, *create, [this](const std::string& name) { return fixFiles_.count(name) > 0; },
                                                            regs));
                        result.resEdits.push_back(std::move(create));
                    }

                    // the downloads and the created textures, bound at whichever register the fix's lists put
                    // their role on
                    if (!fixDownloads_.empty()) {
                        auto create = std::make_unique<FileDownloadCreate<>>(GraphId(g, "", "fixDownloads"), fixDownloads_, resEditConfig());
                        result.edits.push_back(collectNamed(groups, g, *create, [this](const std::string& name) { return fixDownloads_.count(name) > 0; },
                                                            {}));
                        result.resEdits.push_back(std::move(create));
                    }

                    for (const FixTexture& texture : fixTextures_) {
                        auto create = std::make_unique<TexCreate<>>(GraphId(g, "", "fixTexture" + texture.section), "", std::make_shared<TexCreator>(texture.creator),
                                                                           resEditConfig());
                        create->files = {{texture.section, texture.file}};
                        const std::string section = texture.section;
                        result.edits.push_back(collectNamed(groups, g, *create, [section](const std::string& name) { return name == section; },
                                                            {}));
                        result.resEdits.push_back(std::move(create));
                    }

                    return result;
                }

                // A collect of one create edit from every graph of group 'g', wherever a register binds a
                // resource 'named' accepts: at 'regs', or at any register when 'regs' is empty
                std::unique_ptr<Collector> collectNamed(Fixer::GraphGroups& groups, std::size_t g, Collector::ResEdit& create,
                                                        std::function<bool(const std::string&)> named, const std::vector<std::string>& regs) {
                    const Collector::ResPredicate listed = [named](const std::string&, const std::string& value, const auto&) {
                        return named(std::string(StringTools::strip(value)));
                    };

                    auto collect = std::make_unique<Collector>();
                    for (const ModObj& obj : groups.modObjs(g)) {
                        const GraphId id(g, obj.first, obj.second);
                        if (regs.empty()) {
                            collect->srcRegMatches[id] = [](const std::string&) { return true; };
                        } else {
                            collect->srcRegs[id] = regs.front();
                            collect->extraSrcRegs[id] = std::vector<std::string>(regs.begin() + 1, regs.end());
                        }
                        collect->resPredicates[id] = listed;
                    }
                    collect->resEdits = {{"fixFile", &create}};
                    return collect;
                }

                // The source's game texture bound for a planned role the mod has no file for:
                // the download's url, the name it is saved under, and its path relative to the .ini
                struct Fallback {
                    std::string url;
                    std::string fileName;
                    std::string relPath;
                    /**
                     * @brief
                     @rst
                     The section name this download DECLARES itself under.

                     Not ``resourceOfRole_[role]``: a texture edit of the same role overwrites that
                     with its OWN resource, on purpose, so every BINDING follows the edited file.
                     Reading it back here wrote the download's section under the edit's name, so the
                     name was declared twice -- once naming the raw download and once the edited
                     file -- and the raw one won. 21 such pairs across one corpus (2026-09-27).
                     @endrst
                     */
                    std::string resource;
                };

                // A download the fix makes, under the section its lists bind
                void addFixDownload(const Fallback& fallback) {
                    FileDownloadCreate<>::File file;
                    file.file = fallback.relPath;
                    file.url = fallback.url;
                    fixDownloads_[fallback.resource] = std::move(file);
                }

                // ---- WWMIFixerConfig::shapeKeyOrder ----

                // The mod's three shape-key buffers, or their reordered copies
                WWMIShapeKeys::Files shapeKeyFiles(bool reordered) const {
                    const std::string folder = ctx_.getIniFile()->getFolder();
                    const auto pathOf = [this, &folder, reordered](const std::string& resource) {
                        return FileService::absPathOfRelPath(
                            reordered ? shapeKeyOrderFile(resource) : FileService::iniPathToRel(shapeKeyFiles_.at(resource).path),
                            folder);
                    };

                    return WWMIShapeKeys::Files{pathOf(ShapeKeyResources[0]), pathOf(ShapeKeyResources[1]),
                                                pathOf(ShapeKeyResources[2])};
                }


                // The edits the fix makes to a role's texture before binding it -- see
                // WWMIFixerConfig::texEdits. Queued as resources rather than run here, so a role
                // whose file comes from the DOWNLOAD is edited after the download lands: editing a
                // file that is not there yet writes nothing and says nothing.
                // WHAT each texture edit will be called, and which of the mod's files it reads --
                // decided here rather than in addTexEdits(), which runs at fix time, AFTER
                // buildEdits() has already turned every role into a register addition. Setting
                // resourceOfRole_ there updated a map nothing read again, so every edit was written
                // to disk and bound by no section, with the summary reporting it as a success.
                //
                // Same split as the fallback downloads: the name is planned at read time and the
                // resource registered at fix time, so a parse alone still writes nothing.
                void absorbTexEditTargets(const WWMIFixerConfig::TexEditTargets& edits) {
                    for (const auto& [role, resources] : edits.targets) {
                        editTargets_[role].insert(resources.begin(), resources.end());
                    }
                }

                // THE SOURCE'S OWN GAME TEXTURE AS THE DEFAULT, where the mod's file reaches only some of a
                // component's draws -- see the stand-in rule above. Three cases, each the source's own texture
                // where the original drew with the GAME's:
                //
                //   1. the draws a section makes BEFORE it binds anything (IniParseData::lateBindings): they rendered with
                //      the textures the game had bound when it matched the draw (ChisaParfait3's hat and chest
                //      bandages). A role the mod has no file for is already the source's game texture (its
                //      stand-in, edits and all), so only a role the mod DOES serve needs a second file.
                //   2. a role bound only under a toggle that reaches none of the section's draws (Lynae11 binds
                //      its own material-code map only `if $Pants != 0`, and its default outfit drew with the
                //      game's) -- unless a `this` on a texture's hash binds it for every draw anyway (Lynae3).
                //      Which resources a component binds only under a toggle is the parser's
                //      (IniParseData::toggledBindings); a reroute reads its source component's.
                //   3. a role bound with ONE `this` under a toggle (IniParseData::TextureToggle::single): off, the
                //      original draws with the game's texture, which WWMIFixerConfig::toggleLists puts at the front of the role's list.
                void absorbGameDefaults(const WWMIFixerConfig::GameDefaults& defaults) {
                    for (const auto& [role, download] : defaults.downloads) {
                        gameFallbacks_.emplace(role, Fallback{download.url, download.file, download.file, download.resource});
                    }
                    for (const std::string& role : defaults.edited) {
                        editTargets_[role].insert(StringTools::toLower(defaults.ofRole.at(role)));
                    }
                    toggledGame_ = defaults.toggled;
                }

                using RoleRegs = std::vector<std::pair<std::string, std::string>>;

                // The lines of one texture list, and the (role, register) pairs among them
                struct TextureLines {
                    std::vector<Kvp> lines;
                    RoleRegs bound;

                    void bind(const WWMIFixerConfig::Binding& binding, Kvp line) {
                        lines.push_back(std::move(line));
                        bound.emplace_back(binding.role, binding.reg);
                    }
                };

                // THE TEXTURE LISTS, LINE BY LINE (WWMIFixerConfig::textureLists), with the mod's own toggles run in
                // the place of the lines binding a toggled texture (WWMIFixerConfig::toggleLists). Made once the roles,
                // the game defaults and the shaders are known; every list the fix writes is written from these.
                void absorbTextureLists(const WWMIFixerConfig::TextureLists& lists,
                                        const std::map<int, std::vector<WWMIFixerConfig::ModTexLine>>& modTexLines) {
                    textureLists_ = lists;
                    modTexLines_ = modTexLines;
                    listsOfRole_.clear();
                    rawLists_.clear();
                    // what each toggle's copy binds, for the texture edits (addTexEditCollects)
                    for (const WWMIFixerConfig::ToggleList& toggle : textureLists_.toggles) {
                        listsOfRole_[{toggle.role, toggle.reg}].insert(toggle.name);
                        if (toggle.raw) {
                            rawLists_.insert(toggle.name);
                        }
                    }
                }

                // A planned list's lines as the fix writes them -- `<reg> = <value>`, or `run = <toggle>` -- and the
                // (role, register) pairs it binds; nothing for a list the plan does not have
                TextureLines textureLinesOf(const std::string& kind, int component, int passNum = 0) const {
                    TextureLines result;
                    const WWMIFixerConfig::TextureList* list = textureLists_.find(kind, component, passNum);
                    if (list == nullptr) {
                        return result;
                    }

                    for (const WWMIFixerConfig::TextureLine& line : list->lines) {
                        result.lines.push_back(line.run.empty() ? Kvp{line.reg, line.value} : Kvp{std::string(IniKeywords::Run), line.run});
                        if (!line.role.empty()) {
                            result.bound.emplace_back(line.role, line.reg);
                        }
                    }
                    return result;
                }

                // `CommandList<Source>Component<N><infix>Textures<suffix>`, under the fix's name
                std::string textureListName(int component, const std::string& infix = "", const std::string& suffix = "") const {
                    return remapFixName(IniKeywords::CommandList + source_.name + TextTools::capitalize(WWMINames::SlotPrefix)
                                        + std::to_string(component) + infix + "Textures" + suffix);
                }

                // A texture list among the fix's sections -- its lines under 'condition' -- and the (role,
                // register) pairs it binds, which is what addTexEditCollects edits by
                void addTextureList(const std::string& name, const std::string& condition, const TextureLines& list, int component) {
                    addFixSection("textureLists", SectionText(z3_, name).open(condition).keys(list.lines).close().build());
                    if (rerouteShaders_.count(component) > 0) {
                        rawLists_.insert(name);
                    }
                    for (const auto& [role, reg] : list.bound) {
                        listsOfRole_[{role, reg}].insert(name);
                    }
                }

                // The texture edits (WWMIFixerConfig::texEdits), as collects over every binding of an
                // edited role: one TexReplace per edit, collected wherever the role's register
                // binds one of the resources the "texEditTargets" step recorded for it -- in the fix's texture lists
                // and the mod's own carried lines alike. A reroute drawn by its own shader keeps the
                // mod's art unedited (see rerouteShaders_)
                //
                // Each collect is configured on group 0's graphs and runs in EVERY group
                // (ResRegCollect::everyGroup), so an edited texture is built and named once for the
                // mod's file and its copies together
                void addTexEditCollects() {
                    if (editTargets_.empty()) {
                        return;
                    }

                    IniFile* ini = ctx_.getIniFile();
                    const std::string folder = ini->getFolder();
                    WWMIFixerConfig::TexEditContext context;
                    context.iniFolder = folder;
                    context.source = ctx_.modType();
                    context.target = targetType_.has_value() ? &*targetType_ : nullptr;
                    context.fromVersion = fromVersion();
                    context.toVersion = toVersion();
                    context.indexFile = FileService::absPathOfRelPath(modBuffer(IniKeywords::WWMIIndexBuffer), folder);
                    context.positionFile = positionBufPath();
                    context.texcoordFile = FileService::absPathOfRelPath(modBuffer(IniKeywords::WWMITexcoordBuffer), folder);
                    context.drawRanges = drawnInfo_.drawRanges;
                    for (const auto& entry : fileOfRole_) {
                        bool toggled = false;
                        for (const auto& t : toggledGame_) {
                            toggled = toggled || t.first.first == entry.first;
                        }

                        if (!toggled) {
                            context.fileOfRole[entry.first] = entry.second;
                        }
                    }
                    for (const auto& entry : fallbacks_) {
                        context.fileOfRole.emplace(entry.first, FileService::absPathOfRelPath(entry.second.relPath, folder));
                    }

                    for (const WWMIFixerConfig::TexEdit& edit : config_.texEdits) {
                        const auto targets = editTargets_.find(edit.role);
                        if (!edit.makeFilter || targets == editTargets_.end()) {
                            continue;
                        }

                        TexEditor::Filter filter = edit.makeFilter(context);
                        if (!filter) {
                            continue;
                        }

                        // the registers the target reads this role at, and the components binding it
                        std::map<std::string, std::set<int>> componentsAt;
                        for (const auto& [component, planned] : config_.plan) {
                            if (drawnInfo_.present.count(component) == 0 || rerouteShaders_.count(component) > 0) {
                                continue;
                            }
                            for (const WWMIFixerConfig::Binding& binding : bindingsOf(component)) {
                                if (binding.role == edit.role) {
                                    componentsAt[binding.reg].insert(component);
                                }
                            }
                            const auto extra = config_.extraPassRegs.find(planned.slot);
                            for (const auto& [pass, regs] : extra == config_.extraPassRegs.end()
                                                                ? decltype(extra->second){} : extra->second) {
                                (void)pass;
                                for (const WWMIFixerConfig::Binding& binding : regs) {
                                    if (binding.role == edit.role) {
                                        componentsAt[binding.reg].insert(component);
                                    }
                                }
                            }
                        }

                        auto editor = std::make_unique<TexReplace<>>(
                            GraphId(0, "", "texEdit" + TextTools::capitalize(edit.role) + edit.name),
                            std::make_shared<TexEditor>(std::vector<TexEditor::Filter>{std::move(filter)}, edit.compress), resEditConfig(), "resourceRemapTexEdit",
                            TextTools::capitalize(edit.role) + edit.name + editFileTag());
                        // the role is the binding's: a file the role's edit applies to, bound at the role's
                        // register -- in the fix's sections, by one of the role's own lists
                        const std::set<std::string>* resources = &targets->second;
                        const Collector::ResPredicate isTarget = [resources](const std::string&, const std::string& value,
                                                                             const Collector::IterData&) {
                            return resources->count(StringTools::toLower(std::string(StringTools::strip(value)))) > 0;
                        };

                        const std::string role = edit.role;
                        const Collector::ResPredicate inRoleList = [this, role, isTarget](const std::string& key, const std::string& value,
                                                                                         const Collector::IterData& data) {
                            const auto lists = listsOfRole_.find({role, key});
                            return lists != listsOfRole_.end() && lists->second.count(data.sectionName) > 0
                                   && rawLists_.count(data.sectionName) == 0 && isTarget(key, value, data);
                        };

                        // ONE collect over every register: the editor builds what it collects once
                        auto collect = std::make_unique<Collector>();
                        collect->everyGroup = true;
                        for (const auto& [reg, components] : componentsAt) {
                            collect->extraSrcRegs[fixSectionsGraphId(0)].push_back(reg);
                            for (int component : components) {
                                const ModObj obj = targetObjOf(component);
                                collect->extraSrcRegs[GraphId(0, obj.first, obj.second)].push_back(reg);
                                collect->resPredicates[GraphId(0, obj.first, obj.second)] = isTarget;
                            }
                        }
                        collect->resPredicates[fixSectionsGraphId(0)] = inRoleList;
                        collect->resEdits = {{"texEdit", editor.get()}};
                        collect->name = "texEdits";

                        this->fixSectionEdits.push_back(collect.get());
                        texEditCollects_.push_back(std::move(collect));
                        texEditors_.push_back(std::move(editor));
                    }
                }

                // Where the mod's texcoord buffer is and how wide one vertex of it is.
                struct TexcoordBuffer {
                    std::optional<std::string> format;  // as the .ini declares it
                    std::string rel;                    // as the .ini spells it
                    std::string path;                   // ...and on disk
                    std::size_t stride = 0;
                };

                // The mod's texcoord buffer, or nothing -- and when it is nothing, the log says
                // which of the four ways it was nothing.
                std::optional<TexcoordBuffer> findTexcoordBuffer() {
                    IniFile* ini = ctx_.getIniFile();
                    if (ini == nullptr) {
                        return std::nullopt;
                    }

                    // WITHOUT REGARD TO CASE: this mod spells it ResourceTexCoordBuffer, and asking
                    // for ResourceTexcoordBuffer finds nothing and says nothing.
                    const IniParseData& data = parseData();
                    const auto declared = [&data](const std::string& key) -> std::optional<std::string> {
                        const auto* row = data.firstCollected(IniParseRows::Declared, IniKeywords::WWMITexcoordBuffer, key);
                        return row == nullptr ? std::nullopt : std::optional<std::string>(StringTools::strip(row->value));
                    };

                    TexcoordBuffer found;
                    const bool isDeclared = std::any_of(data.collected(IniParseRows::Declared).begin(),
                                                        data.collected(IniParseRows::Declared).end(), [](const auto& row) {
                        return StringTools::equalsIgnoreCase(row.section, IniKeywords::WWMITexcoordBuffer);
                    });
                    if (!isDeclared) {
                        ctx_.log("this mod declares no " + IniKeywords::WWMITexcoordBuffer
                             + ", so its UVs are left alone");
                        return std::nullopt;
                    }

                    const std::optional<std::string> name = declared(IniKeywords::Filename);
                    if (!name.has_value()) {
                        ctx_.log(IniKeywords::WWMITexcoordBuffer + " names no file, so this mod's UVs are left alone");
                        return std::nullopt;
                    }

                    found.rel = FileService::iniPathToRel(*name);
                    found.path = FileService::absPathOfRelPath(found.rel, ini->getFolder());

                    // A DECLARED STRIDE THAT DOES NOT PARSE IS NOT A REASON TO GUESS ONE. This
                    // fell back to 16, which mis-reads every vertex of a buffer that is not 16 -- the
                    // mod said a number and the fix ignored it.
                    found.format = declared(IniKeywords::Format);
                    const std::optional<std::string> strideVal = declared(IniKeywords::Stride);
                    found.stride = DefaultTexcoordStride;
                    if (strideVal.has_value()) {
                        try {
                            found.stride = static_cast<std::size_t>(std::stoul(StringTools::strip(*strideVal).data()));
                        } catch (const std::exception&) {
                            ctx_.log(IniKeywords::WWMITexcoordBuffer + " declares a stride of `"
                                 + std::string(StringTools::strip(*strideVal))
                                 + "`, which is not a number, so this mod's UVs are left alone");
                            return std::nullopt;
                        }
                    }

                    if (found.stride < 4 || found.stride % 2 != 0) {
                        ctx_.log(IniKeywords::WWMITexcoordBuffer + " declares a stride of "
                             + std::to_string(found.stride)
                             + ", which cannot hold a UV pair of halves, so this mod's UVs are left alone");
                        return std::nullopt;
                    }

                    return found;
                }

                // The vertices TexcoordFile::clean must leave alone: the corners of every triangle whose corners sit in
                // different U tiles (TexcoordFile::straddling). Still read at 4 bytes per index, which is what the fold
                // has always assumed; `IbFile::bytesPerIndexOf(<the declared format>)` is how a mod declaring
                // `DXGI_FORMAT_R16_UINT` gets answered, and that is a BEHAVIOUR change rather than a refactor. An index
                // buffer that cannot be read keeps nothing, which folds more rather than less.
                std::vector<bool> straddlingVertices(const TexcoordFile& texcoord) const {
                    IniFile* ini = ctx_.getIniFile();
                    const std::string indexFile = modBuffer(IniKeywords::WWMIIndexBuffer);
                    if (indexFile.empty() || ini == nullptr) {
                        return std::vector<bool>(texcoord.us().size(), false);
                    }

                    try {
                        return texcoord.straddling(IbFile{FileService::absPathOfRelPath(
                            FileService::iniPathToRel(indexFile), ini->getFolder())}.getIndices());
                    } catch (const std::exception&) {
                        return std::vector<bool>(texcoord.us().size(), false);
                    }
                }

                // A remap-only copy of the mod's texcoord buffer -- see
                // WWMIFixerConfig::cleanTexcoords for what is wrong with the original and why
                // neither fault is the mod's bug, and TexcoordFile::clean for what the copy changes.
                //
                // NOT U BELOW 0 (2026-09-27). The fold exists for a mod that UVs half a part into the [1, 2) TILE
                // and relies on the sampler wrapping (47% of one Chisa mod's component 3). A U just BELOW zero is
                // the edge bleed around a UV island; folded, `-0.054` became `0.945`, and on Chisa13's black hair
                // dye every island's fringe sampled the far side of the atlas as a checkerboard of blonde blocks.
                // The fold is wrap-equivalent per VERTEX only while the pass WRAPS, and this one does not.
                void buildTexcoordCopy() {
                    if (!config_.cleanTexcoords || texcoordResource_.has_value()) {
                        return;
                    }

                    const std::optional<TexcoordBuffer> source = findTexcoordBuffer();
                    if (!source.has_value()) {
                        return;                         // findTexcoordBuffer said which way
                    }

                    const std::size_t stride = source->stride;
                    std::vector<bool> keep;
                    TexcoordFile::CleanStats stats;
                    try {
                        TexcoordFile texcoord{source->path, TexcoordFile::halfElements(stride)};
                        if (!texcoord.isValid() || texcoord.us().empty()) {
                            return;                     // not a whole number of vertices, or none: leave it alone
                        }

                        // Counted before anything is written, because the copy exists only when it
                        // differs -- a mod with clean texcoords keeps its own buffer and its own binding.
                        keep = straddlingVertices(texcoord);
                        stats = texcoord.clean(keep);
                    } catch (const std::exception& exception) {
                        ctx_.log(std::string("its Texcoord.buf could not be read: ") + exception.what()
                             + " -- so this mod's UVs are left alone");
                        return;
                    }

                    if (stats.cleared == 0 && stats.folded == 0) {
                        return;
                    }

                    // THE COPY IS ONE OF THE FIX'S FILES (2026-10-29): written by the service when the fix is,
                    // from the same clean -- not here, where a parse alone would write it
                    const std::string fixedRel =
                        modFile(meshFolder_, toModName_ + IniKeywords::Remap + "Texcoord.buf");
                    const std::optional<std::string>& format = source->format;
                    texcoordResource_ = remapFixName(IniKeywords::Resource + WWMINames::TexcoordNoNaN);
                    addFixFile(*texcoordResource_,
                               {{IniKeywords::Type, IniKeywords::Buffer},
                                {IniKeywords::Format, std::string(format.has_value()
                                                           ? StringTools::strip(*format)
                                                           : std::string_view(IniKeywords::FormatR16G16Float))},
                                {IniKeywords::Stride, std::to_string(stride)}},
                               fixedRel,
                               [path = source->path, stride, keep]() {
                                   TexcoordFile copy{path, TexcoordFile::halfElements(stride)};
                                   copy.clean(keep);
                                   return copy.getData();
                               },
                               {IniKeywords::WWMITexcoordReg});
                    ctx_.log("texcoords: " + std::to_string(stats.cleared) + " NaN halves set to 0 and "
                             + std::to_string(stats.folded) + " U values folded into [0, 1) in a remap-only copy");
                }


                WWMIFixerConfig config_;
                bool editTagDone_ = false;
                std::string editTag_;

                std::optional<ModType> targetType_;
                Character source_;
                Character target_;
                std::vector<std::vector<int>> groups_;
                std::vector<ModObj> hashOnlyObjs_;                    // ...and the mod's objects that are not slots

                WWMIFixerConfig::TextureLists textureLists_;            // the "textureLists" step
                std::map<int, std::vector<WWMIFixerConfig::ModTexLine>> modTexLines_;   // the "textureLists" step
                std::unordered_map<std::string, FileAddCreate<>::File> fixFiles_;   // see addFixFile
                std::set<std::string> fixFileRegs_;
                std::unordered_map<std::string, FileDownloadCreate<>::File> fixDownloads_;   // see addFixDownload
                struct FixTexture {
                    std::string section;
                    std::string file;
                    TexCreator creator;
                };
                std::vector<FixTexture> fixTextures_;
                std::vector<Fixer::GroupEdit*> copyEdits_;           // meshCounterpartSections' copies, run first
                std::vector<Fixer::GroupEdit*> toggleEdits_;
                std::vector<std::shared_ptr<void>> toggleOwned_;
                std::vector<std::unique_ptr<ObjGroupEdit::PartEdit>> toggleAdapters_;

                // fixSectionEdits, and what their edits borrow (see buildFixSectionEdits)
                std::vector<std::unique_ptr<IterEdit>> fixSectionIters_;
                std::unique_ptr<RegNewVals<>> rebind_;               // see buildBlendRebind
                std::unique_ptr<RegPartEdit<>> rebindAdapter_;
                std::unique_ptr<ObjGroupEdit> rebindEdits_;
                std::unique_ptr<IfTemplate<std::string, std::string>> latch_;
                std::unique_ptr<IfTemplate<std::string, std::string>> latchAndSet_;
                std::unique_ptr<RegIfTemplateAdd<>> beforeDetected_;
                std::unique_ptr<RegIfTemplateAdd<>> beforeIf_;
                std::unique_ptr<GraphPartEdit<>> beforeDetectedAdapter_;
                std::unique_ptr<GraphPartEdit<>> beforeIfAdapter_;
                std::set<const void*> latched_;         // the sections the group being latched has latched
                std::unordered_set<std::string> hiddenObjs_;          // of those, the ones commented out rather than copied
                std::set<int> drawnSlots_;
                long long meshVertexCount_ = 0;
                std::string meshFolder_;
                std::map<std::string, std::string> resourceOfRole_;   // role -> resource, for what every component shares (a created texture, a download)
                std::map<std::pair<std::string, int>, std::string> resourceOfSlotRole_;   // (role, source component) -> the resource that component binds
                mutable std::optional<std::optional<std::string>> modVb0Version_;   // versionOfModVb0's answer, once asked
                std::vector<std::pair<std::string, std::string>> declared_;   // (file, path relative to the .ini) for a file no resource of the .ini names
                std::map<std::string, std::string> declaredName_;      // that file -> the resource section the fix declares for it
                std::map<std::string, Fallback> fallbacks_;           // role -> the source's game texture, for a planned role the mod has no file for
                std::map<std::string, Fallback> gameFallbacks_;      // ...and the same download for a role the mod DOES serve, for the pre-binding draws of a split section
                std::map<std::pair<std::string, int>, std::string> toggledGame_;  // (role, source component) -> the game's own, for a role the mod binds only under a toggle

                ShaderFilterTags passTags_;                            // the shaders the passes are told apart by -- passFilter
                bool passTagsBuilt_ = false;



                // EMPTY UNTIL THE MOD'S OWN `.ini` SAYS OTHERWISE (2026-09-30): it used to start at
                // `Meshes/<Name>.buf`, so a mod declaring no such section was read at a path the fix
                // invented. The mod's other buffers are the parser's -- see modBuffer.
                std::string blendSourceFile_;                         // the mod's [ResourceBlendBuffer]; blendFixedFile() derives the fix's name FROM it
                // A source component the mod draws from mesh files of ITS OWN (Changli13's coat, `Meshes/Split`):
                // its section runs a `<sharedResourcesList>.<suffix>` list binding its own index, position and blend
                // buffers. The main blend collect cannot take that blend -- its vertex count is not the mesh's --
                // so it gets a remapped blend of its own (settleOwnMeshes), bound after the fix's merge list
                // (buildComponentAdditions).
                using OwnMesh = WWMIFixerConfig::OwnMesh;
                std::map<int, OwnMesh> ownMeshes_;
                // WHAT THE MOD DRAWS (WWMIFixerConfig::drawnOf), set by readMod and read by every later step, decide()
                // included: its sections per component (present), the silent ones, the draws, the reroutes, legacy and
                // the host's `$object_detected` reset. Its own meshes are copied to ownMeshes_, which settleOwnMeshes trims.
                WWMIFixerConfig::Drawn drawnInfo_;

                // A line a linked .ini sets on a draw, as the fix carries it -- see the "linkedLines" step
                struct CarriedLine {
                    std::string condition;      // reading the fix's copies of that file's variables
                    std::string key;            // the key to write (a glow map), or empty...
                    std::string role;           // ...for a texture role, written at the register this component's plan binds it
                    std::string value;          // `ref <a resource of this .ini>`
                };
                std::map<int, std::vector<CarriedLine>> linkedLines_;   // source component -> what is carried onto it
                std::map<int, const WWMIFixerConfig::Shader*> overlays_;   // a drawn component -> SourceComponent::overlay
                std::unique_ptr<AnchorAdd> overlayEdit_;
                std::unique_ptr<RegPartEdit<>> overlayAdapter_;
                std::map<int, const std::vector<std::pair<std::string, std::string>>*> markLines_;   // a drawn component -> SourceComponent::markLines
                CustomShaderBackups shaderBackups_{[this](const std::string& reg) { return shaderBackup(reg); }};   // what a CustomShader run saves and restores (restoringRuns)
                bool hasShaders_ = false;                                                        // a CustomShader section was added
                std::unordered_map<std::string, ByteVec> shaderSources_;                         // a `ps =` file -> its source
                std::unordered_map<std::string, FileAddCreate<>::File> shaderBuffers_;           // a `ps-cbN =` resource -> its section and file
                std::set<std::string> shaderBufferRegs_;                                         // ...and the registers that name them
                std::unique_ptr<AnchorAdd> markEdit_;
                std::unique_ptr<RegPartEdit<>> markAdapter_;
                std::map<int, std::string> afterDispatch_;     // a component drawn first -> the list its draws run (Shader::after)
                std::map<int, std::unique_ptr<AnchorAdd>> afterEdits_;
                std::map<int, std::unique_ptr<RegPartEdit<>>> afterAdapters_;
                std::vector<Kvp> afterConstants_;              // the variables the Shader::after lists keep
                std::map<int, const WWMIFixerConfig::Shader*> rerouteShaders_;   // a drawn component -> its plan entry's own shader (SourceComponent::shader)
                std::unique_ptr<AnchorAdd> rerouteShaderEdit_;
                std::unique_ptr<RegPartEdit<>> rerouteShaderAdapter_;
                // SourceComponent::offPasses: list name -> (condition, draw), and the edits that run them
                std::map<std::string, std::pair<std::string, std::string>> offLists_;
                std::vector<std::unique_ptr<AnchorAdd>> offEdits_;
                std::map<int, std::unique_ptr<RegPartEdit<>>> offAdapters_;
                std::vector<std::unique_ptr<RegRemove<>>> rerouteRemoves_;

                // Whether the TARGET's merged skeleton passes what an 8-bit blend index can name.
                // Read off the vertex group row's largest target id, which is a property of the
                // PAIR -- so the .ini and the buffers, written at different times, cannot disagree
                // about whether there is a blend remap.
                bool targetPast256_ = false;

                // How many bones the one remap holds -- the DISTINCT targets the vertex group row
                // names, so the .ini (written first) and the buffers agree without either having to
                // read the other. 182 for ChisaParfait -> Chisa.
                std::size_t blendRemapBones_ = 0;

                // The blend line the FIX writes: N ids + N weights, and its byte stride -- the mod's
                // own, unless WWMIFixerConfig::targetInfluences widens it
                std::size_t blendInfluences_ = 4;
                std::size_t blendStride_ = 8;
                // ...and the mod's own N, which the widening starts from
                std::size_t srcBlendInfluences_ = 4;

                std::unique_ptr<GraphGroupRemap<>> slotRemap_;
                std::unique_ptr<RegAssetRemap<>> assetRemap_;
                std::unique_ptr<RegPartEdit<>> assetAdapter_;
                std::unique_ptr<RegNewVals<>> dispatchEdit_;        // WWMIFixerConfig::shapeKeyDispatchSize
                std::unique_ptr<RegPartEdit<>> dispatchAdapter_;

                struct ShapeKeyFile {
                    std::string section;                            // the mod's resource section, as it is spelled
                    std::string path;
                    std::string type;
                    std::string format;
                    std::string stride;
                };
                std::map<std::string, ShapeKeyFile> shapeKeyFiles_;  // WWMIFixerConfig::shapeKeyOrder, by ShapeKeyResources
                std::unique_ptr<RegNewVals<>> shapeKeyEdit_;
                std::unique_ptr<RegPartEdit<>> shapeKeyAdapter_;
                std::vector<std::unique_ptr<RegRemove<>>> regRemoves_;
                std::map<int, std::unique_ptr<RegPartEdit<>>> removeAdapters_;    // per component: WWMIFixerConfig::removedLines
                std::map<int, std::array<int, 4>> rabbitFXFlags_;                 // SourceComponent::rabbitFX: component -> the source's x187..w187
                std::vector<std::unique_ptr<AnchorAdd>> rabbitFXPins_;
                std::map<int, std::unique_ptr<RegPartEdit<>>> rabbitFXPinAdapters_;
                std::map<int, PartEdit*> newValsOf_;
                std::vector<std::unique_ptr<RegSurroundedAdd<>>> surroundedAdds_;
                std::vector<std::unique_ptr<GraphPartEdit<>>> graphAdapters_;
                std::vector<std::unique_ptr<RegNewVals<>>> newVals_;
                std::vector<std::unique_ptr<RegPartEdit<>>> regAdapters_;
                std::vector<std::unique_ptr<AnchorAdd>> carriedEdits_;   // ...and the same for the carried-register edits
                std::vector<std::unique_ptr<BaseRegEdit<>>> bypassEdits_;     // ...and for the cleanup-list edits
                Bypasses bypasses_;                            // register -> the resource its binding is parked in
                std::map<std::string, std::map<int, std::vector<PartEdit*>>> objEdits_;   // kind -> component -> its edits (objEdit)
                std::map<std::string, std::unique_ptr<ObjGroupEdit>> kindEdits_;       // each kind's step (buildPerGroupEdits)
                std::vector<std::unique_ptr<VGRemapBlendReplace<>>> blendReplaces_;
                std::optional<std::string> texcoordResource_;                        // the cleaned texcoord copy's resource
                std::unordered_map<std::string, std::string> fileOfRole_;            // role -> the file it resolved to
                std::map<std::string, std::set<std::string>> editTargets_;           // an edited role -> the resources (lowered) its edit applies to
                std::set<std::string> rawLists_;                                     // the fix's lists for a reroute drawn by its own shader: the mod's art, unedited
                std::map<std::pair<std::string, std::string>, std::set<std::string>> listsOfRole_;   // (role, register) -> the fix's lists binding that role there
                std::vector<std::unique_ptr<Collector>> texEditCollects_;            // addTexEditCollects' collects, among fixSectionEdits
                std::vector<std::unique_ptr<TexReplace<>>> texEditors_;        // ...and the edits they build with
                std::vector<std::unique_ptr<Collector>> blendCollects_;
        };
    }


    std::shared_ptr<BaseIniFixer<>> WWMIFixFactory::operator()(BaseIniParser<>* parser, const std::string& toModName,
                                                              std::optional<int> modTypeId) const {
        return std::make_shared<WWMIFixerImpl>(parser, toModName, modTypeId, config);
    }

    IniFixBuilder::Factory makeWWMIFixer(WWMIFixerConfig config) {
        return WWMIFixFactory{std::move(config)};
    }

    std::optional<WWMIFixerConfig> WWMIFixerConfig::ofFactory(const IniFixBuilder::Factory& factory) {
        const WWMIFixFactory* made = factory.target<WWMIFixFactory>();
        return made != nullptr ? std::optional<WWMIFixerConfig>(made->config) : std::nullopt;
    }

    namespace {
        // A game shader's code and constant buffers off GameShaders: the first of 'types' that has a row
        std::string gameShaderCode(const std::string& game, const std::vector<std::pair<const ModType*, std::optional<Version>>>& owners) {
            for (const auto& [owner, version] : owners) {
                if (const std::optional<std::string> code = owner->getGameShader(game, version)) {
                    return *code;
                }
            }
            throw std::invalid_argument("WWMIFixerConfig: no GameShaders row for the shader " + game);
        }

        bool fillShader(WWMIFixerConfig::Shader& shader, const std::vector<std::pair<const ModType*, std::optional<Version>>>& owners) {
            for (std::size_t i = 0; i < shader.passGame.size(); ++i) {
                if (shader.passGame[i].empty()) {
                    continue;
                }
                if (shader.passPs.size() <= i) {
                    shader.passPs.resize(i + 1);
                }
                shader.passPs[i] = gameShaderCode(shader.passGame[i], owners);
            }
            if (shader.game.empty()) {
                return false;
            }
            for (const auto& [owner, version] : owners) {
                const std::optional<std::string> code = owner->getGameShader(shader.game, version);
                if (!code.has_value()) {
                    continue;
                }
                shader.ps = *code;
                shader.constantBuffers.clear();
                for (const std::string& stage : {IniKeywords::Ps, IniKeywords::Vs}) {
                    for (int reg = 0; reg < 16; ++reg) {
                        const std::string buffer = stage + "-cb" + std::to_string(reg);
                        if (std::optional<std::vector<std::uint32_t>> words = owner->getGameShaderBuffer(shader.game, buffer, version)) {
                            shader.constantBuffers[buffer] = std::move(*words);
                        }
                    }
                }
                return true;
            }
            throw std::invalid_argument("WWMIFixerConfig: no GameShaders row for the shader " + shader.game);
        }

        // A map keyed by a target pass, its keys written as a pass NAME (other, other2, ...) turned into the pass's
        // pixel shader off DrawPasses; a key that is already a shader stays
        template <typename V>
        std::map<std::string, V> passKeysToShaders(const std::map<std::string, V>& byPass, const ModType& target,
                                                   const std::string& component, const std::optional<Version>& version) {
            std::map<std::string, V> out;
            for (const auto& [key, value] : byPass) {
                std::string shader = key;
                if (StringTools::startsWith(key, "other")) {
                    const std::optional<std::string> ps = target.getDrawPass(component, key + ".ps", version);
                    if (!ps.has_value()) {
                        throw std::invalid_argument("WWMIFixerConfig: no DrawPasses row for " + target.name + " " + component + " " + key);
                    }
                    shader = *ps;
                }
                if (!out.emplace(shader, value).second) {
                    throw std::invalid_argument("WWMIFixerConfig: " + target.name + " " + component + "'s pass " + shader
                                                + " is keyed twice (by its name and by its shader)");
                }
            }
            return out;
        }
    }

    int WWMIFixerConfig::mergedSkeletonSlotsFor(long long bones) {
        const long long step = WWMINames::MergedSkeletonStep;
        return static_cast<int>(std::max(1LL, (3 * bones + step - 1) / step) * step);
    }

    WWMIFixerConfig::Shader WWMIFixerConfig::Shader::ofGame(const std::string& game) {
        Shader shader;
        shader.name = game;
        shader.game = game;
        return shader;
    }

    std::map<std::string, std::string> WWMIFixerConfig::vb0VersionsOf(const ModType& source) {
        std::map<std::string, std::string> out;
        if (source.hashes == nullptr) {
            return out;
        }
        source.hashes->getRepo().forEachEntry([&source, &out](const std::vector<std::string>& keys, const Version& version, const std::string& value) {
            if (keys.size() >= 2 && keys.front() == source.name && keys.back() == Vb0HashKey) {
                out[StringTools::toLower(value)] = version.toString();
            }
        });
        return out;
    }

    std::optional<Version> WWMIFixerConfig::newestVersionOf(const ModType& target) {
        std::optional<Version> newest;
        const auto take = [&newest](const Version& version) {
            if (!newest.has_value() || *newest < version) {
                newest = version;
            }
        };
        for (const auto& [hash, version] : vb0VersionsOf(target)) {
            (void)hash;
            if (const std::optional<Version> parsed = Version::parse(version)) {
                take(*parsed);
            }
        }
        if (target.drawPasses != nullptr) {
            target.drawPasses->getRepo().forEachEntry([&target, &take](const std::vector<std::string>& keys, const Version& version, const std::string&) {
                if (!keys.empty() && keys.front() == target.name) {
                    take(version);
                }
            });
        }
        return newest;
    }

    WWMIFixerConfig WWMIFixerConfig::withGameData(const ModType& source, const ModType& target, const std::optional<Version>& fromVersion,
                                                  const std::optional<Version>& toVersion) const {
        WWMIFixerConfig config = *this;

        // ---- where the source's game textures are downloaded from: her game's folder, her own, files named after her ----
        if (config.sourceTextures.downloadGameFolder.empty()) {
            config.sourceTextures.downloadGameFolder = GameTypeIdTools::getName(static_cast<GameTypeId>(source.gameTypeId));
        }
        if (config.sourceTextures.downloadCharFolder.empty()) {
            config.sourceTextures.downloadCharFolder = source.name;
        }
        if (config.sourceTextures.downloadPrefix.empty()) {
            config.sourceTextures.downloadPrefix = source.name;
        }

        if (config.sourceVersionFromVb0) {
            config.sourceVersionByVb0 = vb0VersionsOf(source);
        }

        // ---- the target's draw passes: each slot's main passes, their vertex shaders and tags, and its other passes ----
        const std::size_t slots = target.getComponentCount(WWMINames::SlotPrefix, toVersion);
        std::vector<std::vector<std::string>> slotPasses(slots);
        bool anyPass = false;
        for (std::size_t slot = 0; slot < slots; ++slot) {
            const std::string component = WWMINames::SlotPrefix + std::to_string(slot);
            for (const std::string kind : {"main", "other"}) {
                for (const std::string& name : target.getDrawPassNames(component, kind, toVersion)) {
                    const std::optional<std::string> ps = target.getDrawPass(component, name + ".ps", toVersion);
                    if (!ps.has_value()) {
                        break;
                    }
                    if (kind == "main") {
                        slotPasses[slot].push_back(*ps);
                        anyPass = true;
                    }
                    const std::vector<std::string> shaders = target.getDrawPassShaders(component, name, toVersion);
                    if (!shaders.empty()) {
                        config.passVertexShaders[*ps] = shaders;
                        const std::vector<std::string> values = target.getDrawPassTags(component, name, toVersion);
                        for (std::size_t k = 0; k < shaders.size() && k < values.size(); ++k) {
                            config.filterIndices[shaders[k]] = values[k];
                        }
                    }
                }
            }
        }
        while (!slotPasses.empty() && slotPasses.back().empty()) {
            slotPasses.pop_back();       // a component past the last one any pass draws (the fix plans no slot there)
        }
        if (anyPass) {
            config.slotPasses = std::move(slotPasses);
        }

        // ...and the SOURCE's own passes' tags, for the source shaders a fix draws with (a shader holds one filter_index
        // across every loaded .ini, so its own rows' value is the one to keep); the target's win where both have one. A pass
        // runs in the game the fix is made for, so the source's passes are read at the fix's version, not the mod's
        const std::optional<Version>& passVersion = toVersion.has_value() ? toVersion : fromVersion;
        for (std::size_t slot = 0, count = source.getComponentCount(WWMINames::SlotPrefix, fromVersion); slot < count; ++slot) {
            const std::string component = WWMINames::SlotPrefix + std::to_string(slot);
            for (const std::string kind : {"main", "other"}) {
                for (const std::string& name : source.getDrawPassNames(component, kind, passVersion)) {
                    const std::vector<std::string> shaders = source.getDrawPassShaders(component, name, passVersion);
                    const std::vector<std::string> values = source.getDrawPassTags(component, name, passVersion);
                    for (std::size_t k = 0; k < shaders.size() && k < values.size(); ++k) {
                        config.filterIndices.emplace(shaders[k], values[k]);
                    }
                }
            }
        }

        // ---- the registers the config names on a target slot are her layout at her newest model; a fix made for a later
        // game version moves each to where the target's TextureRegisters put its role there (a game update that gives a
        // shader one more texture moves every register after it). A register no role of hers names stays ----
        {
            std::optional<Version> layoutAt;
            for (const auto& [hash, version] : vb0VersionsOf(target)) {
                (void)hash;
                const std::optional<Version> parsed = Version::parse(version);
                if (parsed.has_value() && (!layoutAt.has_value() || *layoutAt < *parsed)) {
                    layoutAt = parsed;
                }
            }
            if (layoutAt.has_value() && toVersion.has_value() && *layoutAt < *toVersion) {
                const auto moved = [&](int slot, const std::string& reg, const std::string& drawPass) {
                    const std::string component = WWMINames::SlotPrefix + std::to_string(slot);
                    const std::optional<std::string> role = target.getTextureRole(component, reg, layoutAt, drawPass);
                    if (!role.has_value()) {
                        return reg;
                    }
                    const std::optional<std::string> now = target.getTextureRegister(component, drawPass.empty() ? *role : *role + "@" + drawPass,
                                                                                       toVersion);
                    return now.has_value() && StringTools::startsWith(*now, IniKeywords::TexRegPrefix) ? *now : reg;
                };
                for (auto& [component, entry] : config.plan) {
                    (void)component;
                    for (Binding& binding : entry.bindings) {
                        binding.reg = moved(entry.slot, binding.reg, "");
                    }
                }
                for (auto& [slot, byPass] : config.extraPassRegs) {
                    const std::string component = WWMINames::SlotPrefix + std::to_string(slot);
                    for (auto& [key, bindings] : byPass) {
                        std::string pass = StringTools::startsWith(key, "other") ? key : "";
                        for (const std::string& name : pass.empty() ? target.getDrawPassNames(component, "other", layoutAt) : std::vector<std::string>{}) {
                            if (target.getDrawPass(component, name + ".ps", layoutAt) == key) {
                                pass = name;
                                break;
                            }
                        }
                        if (pass.empty()) {
                            continue;
                        }
                        for (Binding& binding : bindings) {
                            binding.reg = moved(slot, binding.reg, pass);
                        }
                    }
                }
            }
        }

        for (auto& [slot, byPass] : config.extraPassRegs) {
            byPass = passKeysToShaders(byPass, target, WWMINames::SlotPrefix + std::to_string(slot), toVersion);
        }
        for (auto& [slot, byPass] : config.extraPassNoDraw) {
            std::map<std::string, bool> keyed;
            for (const std::string& key : byPass) {
                keyed.emplace(key, true);
            }
            byPass.clear();
            for (const auto& [shader, unused] : passKeysToShaders(keyed, target, WWMINames::SlotPrefix + std::to_string(slot), toVersion)) {
                (void)unused;
                byPass.insert(shader);
            }
        }

        // ---- the passes a part is kept off, by name (its target slot's "other", ...) ----
        for (auto& [component, entry] : config.plan) {
            (void)component;
            std::map<std::string, bool> keyed;
            for (const std::string& key : entry.offPasses) {
                keyed.emplace(key, true);
            }
            entry.offPasses.clear();
            for (const auto& [shader, unused] : passKeysToShaders(keyed, target, WWMINames::SlotPrefix + std::to_string(entry.slot), toVersion)) {
                (void)unused;
                entry.offPasses.push_back(shader);
            }
        }

        // ---- the target's meshes the mod replaces, by their HashData type (a vb0 hash is kept as it is) ----
        for (std::string& mesh : config.hiddenMeshes) {
            if (const std::optional<std::string> vb0 = target.hashes != nullptr ? target.hashes->get({target.name, mesh}, toVersion, false)
                                                                                : std::nullopt) {
                mesh = StringTools::toLower(*vb0);
            }
        }

        // ---- the target's other meshes, by their HashData type: each one's vb0, its passes (DrawPasses' main, main2, ...)
        // with their vertex shaders and tags, and what each pass binds (TextureRegisters: '<role>' on the first pass,
        // '<role>@<pass>' on another) ----
        {
            std::map<std::string, std::map<std::string, std::vector<Binding>>> meshes;
            for (const auto& [key, byPass] : config.sharedMeshes) {
                const std::optional<std::string> vb0 = byPass.empty() && target.hashes != nullptr
                    ? target.hashes->get({target.name, key}, toVersion, false) : std::nullopt;
                if (!vb0.has_value()) {
                    meshes.emplace(key, byPass);           // a mesh given by its vb0, with its passes spelled out
                    continue;
                }

                std::vector<std::string> types;          // the mesh's TextureRegisters rows
                if (target.textureRegisters != nullptr) {
                    target.textureRegisters->getRepo().forEachEntry([&](const std::vector<std::string>& keys, const Version&, const std::string&) {
                        if (keys.size() == 3 && keys[0] == target.name && keys[1] == key
                                && std::find(types.begin(), types.end(), keys[2]) == types.end()) {
                            types.push_back(keys[2]);
                        }
                    });
                }
                const auto regNumber = [](const std::string& reg) {
                    const std::size_t at = reg.find_last_not_of("0123456789");
                    return at + 1 < reg.size() ? std::stoi(reg.substr(at + 1)) : 0;
                };

                std::map<std::string, std::vector<Binding>> passes;
                for (const std::string& name : target.getDrawPassNames(key, "main", toVersion)) {
                    const std::optional<std::string> ps = target.getDrawPass(key, name + ".ps", toVersion);
                    if (!ps.has_value()) {
                        break;
                    }
                    const std::vector<std::string> shaders = target.getDrawPassShaders(key, name, toVersion);
                    if (!shaders.empty()) {
                        config.passVertexShaders.emplace(*ps, shaders);
                        const std::vector<std::string> values = target.getDrawPassTags(key, name, toVersion);
                        for (std::size_t k = 0; k < shaders.size() && k < values.size(); ++k) {
                            config.filterIndices.emplace(shaders[k], values[k]);
                        }
                    }

                    std::vector<Binding> bindings;
                    for (const std::string& type : types) {
                        const std::size_t at = type.rfind('@');
                        const std::string onPass = at == std::string::npos ? "main" : type.substr(at + 1);
                        if (onPass != name) {
                            continue;
                        }
                        if (const std::optional<std::string> reg = target.textureRegisters->get({target.name, key, type}, toVersion, false)) {
                            bindings.emplace_back(*reg, at == std::string::npos ? type : type.substr(0, at));
                        }
                    }
                    std::stable_sort(bindings.begin(), bindings.end(), [&regNumber](const Binding& a, const Binding& b) {
                        return regNumber(a.reg) < regNumber(b.reg);
                    });
                    passes.emplace(*ps, std::move(bindings));
                }
                if (passes.empty()) {
                    throw std::invalid_argument("WWMIFixerConfig: no DrawPasses row for " + target.name + "'s " + key + " mesh");
                }
                meshes.emplace(StringTools::toLower(*vb0), std::move(passes));
            }
            config.sharedMeshes = std::move(meshes);
        }

        // ---- the source's game textures' thumbprints ----
        if (std::unordered_map<std::string, std::vector<std::uint8_t>> thumbprints = source.getTextureThumbprints(fromVersion); !thumbprints.empty()) {
            config.sourceTextures.textureThumbprints = std::move(thumbprints);
        }

        // ---- the shape keys ----
        if (const std::optional<std::string> dispatch = target.getShapeKeyDispatchSize(toVersion)) {
            config.shapeKeyDispatchSize = *dispatch;
        }
        if (const std::optional<std::vector<long long>> order = source.getShapeKeyRemap(target.name, fromVersion, toVersion)) {
            config.shapeKeyOrder = *order;
        }

        // ---- the game shaders the parts are drawn with: the source's, else the target's ----
        const std::vector<std::pair<const ModType*, std::optional<Version>>> owners = {{&source, fromVersion}, {&target, toVersion}};
        for (auto& [component, planned] : config.plan) {
            (void)component;
            if (planned.shader.has_value()) {
                fillShader(*planned.shader, owners);
            }
            if (planned.overlay.has_value()) {
                fillShader(*planned.overlay, owners);
            }
        }

        // ---- the colours the invented textures take ----
        for (CreatedTexture& created : config.createdTextures) {
            if (created.material.first.empty() && created.material.second.empty()) {
                continue;
            }
            const std::optional<Colour> colour = target.getMaterialColour(created.material.first, created.material.second, toVersion);
            if (!colour.has_value()) {
                throw std::invalid_argument("WWMIFixerConfig: no MaterialColours row for " + target.name + " " + created.material.first
                                            + " " + created.material.second);
            }
            created.colour = *colour;
        }

        // ---- the source's vertex group maps, at her oldest model (a mod from before the merged skeleton) ----
        std::optional<Version> oldest;
        for (const auto& [hash, version] : vb0VersionsOf(source)) {
            (void)hash;
            const std::optional<Version> parsed = Version::parse(version);
            if (parsed.has_value() && (!oldest.has_value() || *parsed < *oldest)) {
                oldest = parsed;
            }
        }
        if (oldest.has_value()) {
            std::map<int, std::vector<int>> vgMaps;
            const std::size_t components = source.getComponentCount(WWMINames::SlotPrefix, oldest);
            for (std::size_t component = 0; component < components; ++component) {
                const std::optional<VGRemap> row = source.getVGRemap(source.name, oldest, oldest, WWMINames::SlotPrefix + std::to_string(component),
                                                                     std::string(""));
                if (!row.has_value()) {
                    continue;
                }
                std::vector<int>& out = vgMaps[static_cast<int>(component)];
                const auto& remap = row->getRemap();
                out.resize(remap.size());
                for (const auto& [from, to] : remap) {
                    if (from >= 0 && static_cast<std::size_t>(from) < out.size()) {
                        out[static_cast<std::size_t>(from)] = static_cast<int>(to);
                    }
                }
            }
            if (!vgMaps.empty()) {
                config.sourceVgMaps = std::move(vgMaps);
            }
        }

        // ---- RabbitFX's tags of the source's own game textures, on each part whose routing is pinned: what her draw of the
        //      part binds at ps-t0 .. ps-t3 (DrawPasses' main.ps-tN) and how the game creates it (TextureDescs). A reroute
        //      (DrawReroute::as) is its source component's draw ----
        for (auto& [key, planned] : config.plan) {
            if (!planned.rabbitFX.has_value() || !planned.rabbitFX->gameTags.empty()) {
                continue;
            }
            const std::string obj = WWMINames::SlotPrefix + std::to_string(config.componentOf(key));
            for (const std::string& reg : RabbitFX::Registers) {
                const std::optional<std::string> texture = source.getDrawPass(obj, "main." + reg, fromVersion);
                if (!texture.has_value()) {
                    throw std::invalid_argument("WWMIFixerConfig: no DrawPasses row for " + source.name + " " + obj + " main." + reg);
                }
                const auto desc = source.getTextureDesc(*texture, fromVersion);
                if (!desc.has_value()) {
                    throw std::invalid_argument("WWMIFixerConfig: no TextureDescs row for " + source.name + " texture " + *texture);
                }
                planned.rabbitFX->gameTags[reg] = RabbitFX::tagOf(std::get<0>(*desc), std::get<1>(*desc), std::get<2>(*desc), std::get<3>(*desc));
            }
        }

        // ---- the merged skeleton the fix supplies a legacy mod: three float4 a bone of the target's, in WWMI Tools' steps ----
        if (config.mergedSkeletonSlots <= 0) {
            config.mergedSkeletonSlots = mergedSkeletonSlotsFor(target.getMergedBoneCount(WWMINames::SlotPrefix, toVersion));
        }

        // ---- the registers each copied game shader binds a role at: its own draw's (the source's TextureRegisters for
        // the component it was read from), and the target slot's for what it captures or refs off a slot ----
        const auto registerOf = [&config](const ModType& owner, const std::optional<Version>& version, int component,
                                          const std::string& role) {
            const std::string key = WWMINames::SlotPrefix + std::to_string(component);
            const std::optional<std::string> reg = owner.getTextureRegister(key, role, version);
            if (!reg.has_value()) {
                throw std::invalid_argument("WWMIFixerConfig: no TextureRegisters row for " + owner.name + " " + key + " " + role);
            }
            return *reg;
        };
        const auto fillRegisters = [&](Shader& shader, int key, int slot) {
            const int own = shader.component >= 0 ? shader.component : config.componentOf(key);
            for (Binding& binding : shader.registers) {
                if (binding.reg.empty()) {
                    binding.reg = registerOf(source, fromVersion, own, binding.role);
                }
            }
            for (auto& [reg, role] : shader.gameRoles) {
                if (reg.empty()) {
                    reg = registerOf(source, fromVersion, own, role);
                }
            }
            for (Shader::Stashed& stash : shader.stashed) {
                if (stash.reg.empty()) {
                    stash.reg = registerOf(source, fromVersion, own, stash.role);
                }
                if (stash.slotReg.empty()) {
                    stash.slotReg = registerOf(target, toVersion, stash.slot, stash.role);
                }
            }
            std::vector<std::pair<std::string, std::string>> refs;
            for (const std::string& role : shader.refRoles) {
                std::pair<std::string, std::string> line(registerOf(source, fromVersion, own, role),
                                                         IniKeywords::Ref + " " + registerOf(target, toVersion, slot, role));
                if (std::find(shader.lines.begin(), shader.lines.end(), line) == shader.lines.end()) {     // filled once
                    refs.push_back(std::move(line));
                }
            }
            shader.lines.insert(shader.lines.begin(), refs.begin(), refs.end());
        };
        for (auto& [key, planned] : config.plan) {
            for (std::optional<Shader>* shader : {&planned.shader, &planned.overlay}) {
                if (shader->has_value()) {
                    fillRegisters(**shader, key, planned.slot);
                }
            }
        }

        // ---- what each texture role IS: both characters' TextureKinds rows, and every role the config names has one ----
        for (const auto& [owner, at] : std::vector<std::pair<const ModType*, std::optional<Version>>>{{&source, fromVersion}, {&target, toVersion}}) {
            for (const auto& [role, kind] : owner->getTextureKinds(at)) {
                const auto [have, added] = config.roleKinds.emplace(role, kind);
                if (!added && have->second != kind) {
                    throw std::invalid_argument("WWMIFixerConfig: TextureKinds give " + role + " two kinds, " + have->second + " and " + kind);
                }
            }
        }
        {
            std::set<std::string> named;
            const auto name = [&named](const std::string& role) {
                if (!role.empty() && role != IniKeywords::Null) {
                    named.insert(role);
                }
            };
            for (const auto& [key, planned] : config.plan) {
                (void)key;
                for (const Binding& binding : planned.bindings) {
                    name(binding.role);
                }
                for (const std::optional<Shader>* shader : {&planned.shader, &planned.overlay}) {
                    if (!shader->has_value()) {
                        continue;
                    }
                    for (const Binding& binding : (*shader)->registers) {
                        name(binding.role);
                    }
                    for (const auto& [reg, role] : (*shader)->gameRoles) {
                        (void)reg;
                        name(role);
                    }
                    for (const Shader::Stashed& stash : (*shader)->stashed) {
                        name(stash.role);
                    }
                    for (const std::string& role : (*shader)->refRoles) {
                        name(role);
                    }
                }
            }
            for (const auto& [slot, byPass] : config.extraPassRegs) {
                (void)slot;
                for (const auto& [pass, bindings] : byPass) {
                    (void)pass;
                    for (const Binding& binding : bindings) {
                        name(binding.role);
                    }
                }
            }
            for (const TexEdit& texEdit : config.texEdits) {
                name(texEdit.role);
            }
            for (const CreatedTexture& created : config.createdTextures) {
                name(created.role);
            }
            for (const auto& [hash, role] : config.sourceTextures.roles) {
                (void)hash;
                name(role);
            }
            for (const auto& [slot, byReg] : config.sourceTextures.registerRoles) {
                (void)slot;
                for (const auto& [reg, role] : byReg) {
                    (void)reg;
                    name(role);
                }
            }
            for (const std::string& role : named) {
                if (config.roleKinds.count(role) == 0) {
                    throw std::invalid_argument("WWMIFixerConfig: no TextureKinds row for the role " + role + " (" + source.name + " -> "
                                                + target.name + ")");
                }
            }
        }

        // ---- the game textures captured off a slot: what the shaders' captures name ----
        for (const auto& [key, planned] : config.plan) {
            (void)key;
            for (const std::optional<Shader>* shader : {&planned.shader, &planned.overlay}) {
                if (!shader->has_value()) {
                    continue;
                }
                for (const Shader::Stashed& stash : (*shader)->stashed) {
                    std::vector<std::string>& regs = config.stashRegs[stash.slot];
                    if (std::find(regs.begin(), regs.end(), stash.slotReg) == regs.end()) {
                        regs.push_back(stash.slotReg);
                    }
                }
            }
        }
        return config;
    }
}
