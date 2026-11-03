#include "AGRemapCore/data/IniFixData/WWMIFixer.h"
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
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <fstream>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <system_error>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "AGRemapCore/constants/FileExt.h"
#include "AGRemapCore/constants/IniKeywords.h"
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
#include "AGRemapCore/model/iftemplate/IfTemplateRender.h"
#include "AGRemapCore/tools/z3/Z3Context.h"
#include "AGRemapCore/model/IniNamingTools.h"
#include "AGRemapCore/model/IniSectionText.h"
#include "AGRemapCore/model/assets/Hashes.h"
#include "AGRemapCore/model/Version.h"
#include "AGRemapCore/model/buffers/BufElementType.h"
#include "AGRemapCore/model/buffers/BufFloat.h"
#include "AGRemapCore/model/buffers/BufInt.h"
#include "AGRemapCore/model/buffers/WWMIBlend.h"
#include "AGRemapCore/model/buffers/WWMIShapeKeys.h"
#include "AGRemapCore/model/files/BinaryFile.h"
#include "AGRemapCore/model/files/IbFile.h"
#include "AGRemapCore/model/files/TexcoordFile.h"
#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/model/files/IniSharedState.h"
#include "AGRemapCore/model/files/TextureFile.h"
#include "AGRemapCore/model/textures/TexThumbprint.h"
#include "AGRemapCore/model/iftemplate/IfTemplateRender.h"
#include "AGRemapCore/model/iniresources/RemapBlendResource.h"
#include "AGRemapCore/model/iniresources/RemapIniResource.h"
#include "AGRemapCore/model/iniresources/RemapTexResource.h"
#include "AGRemapCore/model/strategies/ModType.h"
#include "AGRemapCore/model/strategies/iniFixers/GIMIFixer.h"
#include "AGRemapCore/model/strategies/iniFixers/IniFileFixContext.h"
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
#include "AGRemapCore/tools/DownloadTools.h"
#include "AGRemapCore/tools/files/FileDownload.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphGroupRemove.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphRemove.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphCreate.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/ResRegCollect.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/FileAddEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/FileDownloadEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/TexEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/BlendEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/VGRemapBlendEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegAdd.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegAnchorAdd.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegAssetRemap.h"
#include "AGRemapCore/model/iftemplate/ListValue.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegNewVals.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegRemap.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegRemove.h"

#include <stdexcept>
#include "AGRemapCore/model/strategies/iniParsers/GIMIParser.h"
#include "AGRemapCore/model/strategies/texEditors/TexCreator.h"
#include "AGRemapCore/model/iniresources/RemapFileResource.h"
#include "AGRemapCore/tools/NumTools.h"
#include "AGRemapCore/tools/ListTools.h"
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

        // Hash-data keys and .ini registers spelled literally, as the rest of data/ spells them.
        const std::string Vb0HashKey = "vb0";
        const std::string ShapeKeyType = "shapekeys";
        const std::string MeshVertexCountKey = IniKeywords::Global + " " + IniKeywords::WWMIMeshVertexCount;

        // What a ResGroupCollect files the blend's edit under -- this library's vocabulary, not an
        // `.ini` key, so it does not belong in IniKeywords
        const std::string BlendResGroupMember = "blend";

        // A WWMI Tools export's shape-key buffers -- see WWMIFixerConfig::shapeKeyOrder
        const std::vector<std::string> ShapeKeyResources = {
            IniKeywords::WWMIShapeKeyOffsetBuffer, IniKeywords::WWMIShapeKeyVertexIdBuffer, IniKeywords::WWMIShapeKeyVertexOffsetBuffer};



        const std::string Cb4HashKey = "cb4";

        // THE HALF CODEC THAT USED TO LIVE HERE IS GONE (2026-09-29). It was a second
        // implementation of `model/buffers/BufFloat.cpp`'s, kept apart because that one TRUNCATES
        // where a folded UV needs round-half-to-even -- and both files carried a comment telling
        // the next reader not to merge them.
        //
        // That was a missing PARAMETER, not two different jobs: `BufFloat16::Rounding::NearestEven`
        // is the mode, and under it decode-then-encode is an exact identity over all 65536 half bit
        // patterns, which is what lets `BufFile::fix` -- it re-encodes every line, touched or not --
        // own this buffer. See `core/tests/BufFloat16_Rounding_test.cpp`.
        const std::string ShapeKeyZero = "ShapeKeyZero";

        // The two elements of a vector buffer line the mirrored twin negates the first of
        const std::string MirrorNormalElement = "NORMAL";
        const std::string MirrorTangentElement = "TANGENT";

        // Marks everything belonging to the SOURCE's own game texture for a role the mod also
        // serves -- the download, its edits and the list that binds them. See
        // `WWMIFixer::planGameDefaults`.
        const std::string GameSuffix = "Game";
        const std::string ChecksumNotFound = "ChecksumNotFound";

        // How a file's name may say what type of texture it is, when nothing else does.

        // How far above the .ini file's own folder the mod may reach for its textures: each level
        // climbed has to hold a .ini file of its own (a LOD folder's parent holding the file that
        // declares the textures), so a library of many mods is never indexed as one.


        // The WWMI blend line: N R8 bone indices then N R8 weights (Metadata.json's export_format
        // 'Blend'); the library's default BlendFile layout is GIMI's 32-byte one.
        //
        // N IS PER CHARACTER AND HAS TO BE DERIVED. Sanhua's line is 8 bytes (four influences) and
        // Chisa, ChisaParfait, Augusta, Iuno and Galbrena carry EIGHT (16 bytes). Fixed at four,
        // BufFile reads a 16-byte line as TWO lines -- so BlendFile::remapIndices remaps ids 0-3
        // gated on ids 4-7 read as weights, and then remaps the real WEIGHTS 0-3 through the vertex
        // group table as if they were bone ids. The result is a blend with corrupted weights,
        // written with no error anywhere. Every ChisaParfait mod is 8-influence and none carries a
        // blend remap, so ChisaParfait -> Chisa meets it on every mod.
        //
        // The format name is 3dmigoto's label, read back only when a buffer is written as dump text
        // (VbFile). WWMI Tools' own `.fmt` calls the 8-wide element `R8_UINT`, so that is what an
        // 8-influence line is called here; four keeps the name it has always had, so the shipped
        // characters' output cannot move.

        // What a Texcoord.buf's bytes per vertex are taken to be when the resource declares
        // none of its own.
        constexpr std::size_t DefaultTexcoordStride = 16;


        // The D3D semantic names every line of these buffers is keyed by. A filter that asks for
        // the wrong one finds nothing, leaves every vertex alone and reports success, so they are
        // written once.

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

        // The layout the blend and Position.buf sizes imply, or bail's message saying they imply none
        WWMIBlend::Layout blendShape(const RemapBlendResource& resource, const std::string& positionPath) {
            const std::optional<WWMIBlend::Layout> layout = WWMIBlend::layoutOf(resource.srcPath, positionPath);
            if (!layout.has_value()) {
                bail(resource, "its Blend.buf and Position.buf do not agree on a vertex count and an"
                               " even number of influences per vertex");
            }

            return layout.value_or(WWMIBlend::Layout{});
        }


        using AnchorAdd = RegAnchorAdd<>;
        using Kvp = std::pair<std::string, std::string>;
        using Kvps = std::vector<std::pair<std::string, std::string>>;
        using DrawRange = std::pair<long long, long long>;


        // The (count, first) of a `drawindexed = <count>, <first>, <base>[, ...]` line, every field a whole number
        // (ListValue::numbers); with 'zeroBase', only a line of exactly those three whose base is 0. Nothing for
        // `auto`, a field that is not a number, or a negative count or first
        std::optional<DrawRange> drawRangeOf(const std::string& value, bool zeroBase = false) {
            const std::optional<std::vector<long long>> numbers = ListValue(value).numbers();
            if (!numbers.has_value() || numbers->size() < 3 || (*numbers)[0] < 0 || (*numbers)[1] < 0 || (zeroBase && (numbers->size() != 3 || (*numbers)[2] != 0))) {
                return std::nullopt;
            }

            return DrawRange((*numbers)[0], (*numbers)[1]);
        }


        // A `run =` of the section 'sectionOf' names for each draw it knows, in place of the draw
        // (a rerouted draw, drawn by the reroute's own pixel shader -- see
        // WWMIFixerConfig::DrawReroute::shader -- or a draw suppressed on some passes, see
        // SourceComponent::offPasses) or right after it (SourceComponent::markLines). A draw it does
        // not name, or one with a base vertex, is left as it is.
        std::unique_ptr<AnchorAdd> drawRunAdd(std::map<DrawRange, std::string> sectionOf, AnchorAdd::Placement placement) {
            return std::make_unique<AnchorAdd>(
                std::vector<std::string>{IniKeywords::DrawIndexed},
                [sectionOf = std::move(sectionOf)](const std::string&, const std::string& value) -> std::optional<Kvps> {
                    const std::optional<DrawRange> range = drawRangeOf(value, true);
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


        /**
         * @brief
         @rst
         Puts a back-face twin after EVERY ``drawindexed`` of a part -- see
         :cpp:member:`WWMIFixerConfig::mirroredComponents`
         @endrst
         *
         * Once per path is not enough: ChisaParfait2 draws its skirt as five ranges, each inside its
         * own ``if $Variable... == 1``, and a twin at the end of the section would be drawn whatever
         * the toggles say. Beside the draw it mirrors, it is drawn exactly when that draw is.
         *
         * `ib` and the vector register are restored after each twin, so the next toggle's draw is
         * not left reading the mirrored buffers -- the mod's own names, which the shared-resource
         * list binds and which the graph remap does not rename. A twin draws the same count at
         * `first - base`, which is where that triangle sits in the mirrored copy of the component's
         * span; `auto`, or a range this cannot read, gets none.
         */
        std::unique_ptr<AnchorAdd> mirrorTwin(std::string mirrorIndex, std::string mirrorVector, std::string ownIndex,
                                              std::string ownVector, std::string vectorReg, long long base) {
            return std::make_unique<AnchorAdd>(
                std::vector<std::string>{IniKeywords::DrawIndexed},
                [mirrorIndex = std::move(mirrorIndex), mirrorVector = std::move(mirrorVector), ownIndex = std::move(ownIndex),
                 ownVector = std::move(ownVector), vectorReg = std::move(vectorReg), base]
                (const std::string&, const std::string& value) -> std::optional<Kvps> {
                    const std::optional<DrawRange> range = drawRangeOf(value);
                    if (!range.has_value()) {
                        return std::nullopt;
                    }

                    const std::string twin = std::to_string(range->first) + ", " + std::to_string(range->second - base) + ", 0";

                    return Kvps{{IniKeywords::Ib, mirrorIndex}, {vectorReg, mirrorVector}, {IniKeywords::DrawIndexed, twin},
                                {IniKeywords::Ib, ownIndex}, {vectorReg, ownVector}};
                },
                AnchorAdd::Placement::After);
        }


        /** @brief register -> the resource its previous binding is parked in -- see bypassRestore */
        using Bypasses = std::vector<std::pair<std::string, std::string>>;

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
         * @brief What a config's role name says the texture IS, with the part it belongs to stripped
         *
         * `upperDiffuse`, `lowerDiffuse` and `panelDiffuse` all have the kind `Diffuse`. The names
         * are the config's own, so the convention is this repo's -- see CarriedLines.
         */
        std::string roleKind(const std::string& role) {
            for (std::size_t i = 0; i < role.size(); ++i) {
                if (std::isupper(static_cast<unsigned char>(role[i])) != 0) {
                    return role.substr(i);
                }
            }

            return "";
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
            while (true) {
                const std::string slot = slotPrefix + std::to_string(out.slots.size());
                std::optional<std::string> first = modType.indices->get({modType.name, "", slot}, version, false);
                if (!first.has_value()) {
                    break;
                }

                std::optional<std::string> count = modType.getIndexCount(slot, "", version);
                std::optional<std::string> vgOffset = modType.getVGOffset(slot, "", version);
                std::optional<std::string> vgCount = modType.getVGCount(slot, "", version);
                if (!count.has_value() || !vgOffset.has_value() || !vgCount.has_value()) {
                    error = "the library has " + modType.name + "'s " + slot + " match_first_index but not all of its"
                            " match_index_count / vg_offset / vg_count";
                    return std::nullopt;
                }

                out.slots.push_back(Slot{*first, *count, *vgOffset, *vgCount});
            }

            if (out.slots.empty()) {
                error = "the library has no draw slots (Indices rows typed " + slotPrefix + "N) for " + modType.name;
                return std::nullopt;
            }

            return out;
        }


        // ---- the mod's textures, by role ----
        //
        // `TextureRoles::Role` (now model/strategies/iniParsers/TextureRoles.h) and `TextureIndex` MOVED to data/WWMITextureFacts.h (2026-09-29), with the
        // constants and the `Component<N>_<Type>` table they read. Sorting a mod's files into roles
        // is CLASSIFICATION -- it reads only the SOURCE's own textures and never the target, the
        // plan or the fix -- so it belongs where a parser can reach it too, which is the point of
        // the header. The fixer still constructs it here; moving WHEN it runs is the next step.


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
                    const bool characters = readCharacters(error);
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

                    readTextures();
                    readReroutes();
                    readShaders();
                    planTexEdits();
                    planGameDefaults();
                    readRabbitFX();                     // after the toggles are known, and reading the pre-edit names
                    buildEdits();
                    buildAppended();
                    buildToggleLists();

                    this->graphGroupEdits.clear();
                    for (Fixer::GroupEdit* edit : copyEdits_) {
                        this->graphGroupEdits.push_back(edit);
                    }
                    for (Fixer::GroupEdit* edit : toggleEdits_) {
                        this->graphGroupEdits.push_back(edit);
                    }
                    this->graphGroupEdits.push_back(slotRemap_.get());
                    this->graphGroupEdits.push_back(mainEdits_.get());
                    for (auto& collect : blendCollects_) {
                        this->graphGroupEdits.push_back(collect.get());
                    }
                    buildFixSectionEdits();

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
                // THE EDITS ON THE FIX'S OWN SECTIONS, run on every group once the slot remap has made
                // the copies and the fix's sections are in them (TemplateFixer::fixSectionEdits), in order:
                //
                //   1. the CustomShader files, collected off the shader graph   (shaderFileEdits)
                //   2. the fix's buffers, downloads and created textures         (fixFileEdits)
                //   3. the blend override, rebound to the remapped blend         (blendRebindEdits)
                //   4. the frame's skeleton remap, latched on $object_detected   (latchEdits)
                //   5. the texture edits, one collect per edit                   (addTexEditCollects)
                //
                // Steps 1-4 are IterGraphGroupEdits: a graph is named by its group's index, and the
                // number of groups is decided at fix time. Step 5 builds each edited texture once for
                // every group, so its collects run in every group at once (ResRegCollect::everyGroup).
                // Each step is a no-op on a group that has nothing for it.
                void buildFixSectionEdits() {
                    this->fixSectionEdits.clear();
                    fixSectionIters_.clear();
                    texEditCollects_.clear();
                    texEditors_.clear();
                    for (IterEdit::EditFactory factory : std::initializer_list<IterEdit::EditFactory>{
                            [this](std::size_t g, Fixer::GraphGroups& groups) { return shaderFileEdits(g, groups); },
                            [this](std::size_t g, Fixer::GraphGroups& groups) { return fixFileEdits(g, groups); },
                            [this](std::size_t g, Fixer::GraphGroups& groups) { return blendRebindEdits(g, groups); },
                            [this](std::size_t g, Fixer::GraphGroups& groups) { return latchEdits(g, groups); }}) {
                        fixSectionIters_.push_back(std::make_unique<IterEdit>(std::move(factory)));
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
                    for (const std::string& reg : shaderBufferRegs_) {
                        auto buffers = std::make_unique<Collector>();
                        buffers->srcRegs[shaders] = reg;
                        buffers->resPredicates[shaders] = namesFile;
                        buffers->resEdits = {{"shaderBuffer", bufferCreate.get()}};
                        result.edits.push_back(std::move(buffers));
                    }

                    auto sources = std::make_unique<Collector>();
                    sources->srcRegs[shaders] = "ps";
                    sources->resPredicates[shaders] = namesFile;
                    sources->resEdits = {{"shaderSource", sourceAdd.get()}};
                    result.edits.push_back(std::move(sources));

                    result.resEdits.push_back(std::move(bufferCreate));
                    result.resEdits.push_back(std::move(sourceAdd));
                    return result;
                }

                // One CustomShader section, among the fix's own
                void addShaderSection(const std::string& name, const std::vector<std::pair<std::string, std::string>>& keys) {
                    addFixSection(makeSection(name, keys));
                    hasShaders_ = true;
                }

                // A MOD OF A CHARACTER PAST 256 BONES BINDS THE BLEND REGISTER TWICE: `vb4 =
                // ResourceBlendBuffer` (a file) and `vb4 = ref ResourceBlendBufferOverride` (a buffer
                // WWMI's BlendRemapper fills at load). The collect takes only the first -- the second
                // names no file, and collecting it kills the run looking for a section called
                // `Resourceref Resource...` -- so without this the second keeps pointing at the
                // SOURCE's own override, which is an inverse of the whole remap.
                //
                // Here rather than at render time: this runs after every group edit, including the
                // GraphGroupRemap that makes the copies, so the copies are in the graphs and the
                // rewrite is one RegNewVals over them. It used to be a pass over the fix's own
                // rendered TEXT -- `getline`, find the `=`, splice lines back -- which is the section
                // model written out and read straight back in. Fires on 19 of the 58 WuWa mod folders
                // on disk, all of them this character's (2026-09-29).

                IterEdit::GroupEdits blendRebindEdits(std::size_t g, Fixer::GraphGroups& groups) {
                    IterEdit::GroupEdits result;
                    if (config_.blendReg.empty()) {
                        return result;
                    }

                    // The remapped blend as the GRAPH spells it, which is what the collect wrote -- read
                    // rather than rebuilt, so the two cannot drift apart.
                    rebindTo_.clear();
                    forEachPart(groups, g, [&](ContentPart& part) {
                        if (!rebindTo_.empty()) {
                            return;
                        }

                        for (const std::string& value : part.getVals(config_.blendReg)) {
                            if (!IniNamingTools::hasKeywordPrefix(value, IniKeywords::Ref) && value.find(IniKeywords::Remap) != std::string::npos) {
                                rebindTo_ = std::string(StringTools::strip(value));
                                break;
                            }
                        }
                    });

                    if (rebindTo_.empty()) {
                        return result;                  // nothing remapped this group's blend
                    }

                    if (rebind_ == nullptr) {
                        rebind_ = std::make_unique<RegNewVals<>>(std::vector<std::pair<std::string, RegNewVals<>::NewValSpec>>{{config_.blendReg, RegNewVals<>::NewVal(
                            RegNewVals<>::OldValProducer([this](const std::string& oldValue, const ModType*) {
                                return IniNamingTools::hasKeywordPrefix(oldValue, IniKeywords::Ref) ? rebindTo_ : oldValue;
                            }))}});
                        rebindAdapter_ = std::make_unique<RegPartEdit<>>(rebind_.get());
                    }

                    // the mod's sections only, not the fix's own
                    std::vector<ObjGroupEdit::IniEdits> edits(g + 1);
                    for (const ModObj& obj : groups.modObjs(g)) {
                        if (obj != fixSectionsGraphId(g).modObj) {
                            edits[g].edits[obj] = {rebindAdapter_.get()};
                        }
                    }

                    result.edits.push_back(std::make_unique<ObjGroupEdit>(std::move(edits)));
                    return result;
                }

                using ContentPart = IfContentPart<std::string, std::string>;

                // Every content part of every section of every graph in one group.
                template <typename Fn>
                void forEachPart(Fixer::GraphGroups& groups, std::size_t groupInd, Fn&& fn) const {
                    const GraphId fixSections = fixSectionsGraphId(groupInd);
                    for (const ModObj& obj : groups.modObjs(groupInd)) {
                        auto* graph = groups.getGraph(groupInd, obj);
                        if (graph == nullptr || obj == fixSections.modObj) {
                            continue;                   // the mod's sections only, not the fix's own
                        }

                        for (const auto& entry : graph->sections()) {
                            if (entry.second == nullptr) {
                                continue;
                            }

                            for (const std::unique_ptr<IfTemplatePart>& part : entry.second->parts()) {
                                auto* content = dynamic_cast<ContentPart*>(part.get());
                                if (content != nullptr) {
                                    fn(*content);
                                }
                            }
                        }
                    }
                }

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
                // Whether the host's [Present] resets `$object_detected` each frame: the line WWMI Tools
                // writes there, `post $object_detected = 0`, anywhere inside that section. A [Present]
                // declared more than once is ONE section, as 3DMigoto merges it (Changli4 has three, the
                // reset in the second) -- see IniFile::mergesRepeatedSection.
                bool presentResetsObjectDetected() const {
                    for (const std::string& section : ownSections()) {
                        if (!StringTools::equalsIgnoreCase(section, IniKeywords::Present)) {
                            continue;
                        }

                        for (const Row* row : ownRows(section)) {
                            if (StringTools::startsWith(StringTools::toLower(row->key), IniKeywords::Post + " " + IniKeywords::WWMIObjectDetected)
                                    && StringTools::strip(row->value) == "0") {
                                return true;
                            }
                        }
                    }

                    return false;
                }

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
                // binds that one result. A section of the fix's that merges without setting the flag
                // (a copy's merge-only section) sets it here. Only for a host whose [Present] does the
                // reset: without it the flag would stay 1 and the remap would never run again.
                //
                // Two RegIfTemplateAdds over every section of every graph, after every other edit (the
                // copies exist then); the appended sections that merge carry the latch as they are built.
                bool latchesRemap() const {
                    return targetPast256_ && hostResetsDetected_;
                }

                // `if $object_detected == 0` / `run = <the skeleton remap>` / `endif`, then `$object_detected = 1`
                // when 'setsDetected'
                std::unique_ptr<IfTemplate<std::string, std::string>> latchTemplate(bool setsDetected) {
                    std::vector<std::unique_ptr<IfTemplatePart>> parts;
                    parts.push_back(std::make_unique<IfPredPart>("if " + IniKeywords::WWMIObjectDetected + " == 0", IfPredPartType::If, z3_));
                    parts.push_back(std::make_unique<ContentPart>(
                        Kvps{{IniKeywords::Run, remapFixName("CommandListRemapMergedSkeleton")}}, 1));
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
                        std::unordered_set<std::string> merges = {remapFixName("CommandListMergeWindow")};
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

                /**
                 * @brief Whether a section is one a PREVIOUS run of this fix wrote
                 *
                 * The variant scans read what the MOD binds, and a mod that has been fixed before
                 * still carries the last run's sections when they are read. Counting those makes
                 * the fix's output depend on its own previous output: the first run of
                 * the register toggles (IniParseData::registerToggleOf) gave `upperDiffuse: 2 variants bound by
                 * TextureOverrideComponent3 ps-t3` on a clean mod and `3 variants bound by
                 * TextureOverrideComponent3<Fix> ps-t2` on the same mod fixed once already, which
                 * is a different set of edited copies for the same input.
                 */
                static bool isFixSection(const std::string& name, const std::string& fixSuffix) {
                    return !fixSuffix.empty() && StringTools::endsWithIgnoreCase(name, fixSuffix);
                }

                ModObj slotObj(int component) const {
                    const auto alias = rerouteFrom_.find(component);
                    if (alias != rerouteFrom_.end()) {
                        component = alias->second;
                    }

                    return ModObj("", config_.slotPrefix + std::to_string(component));
                }

                // Is 'hash' a vb0 the library files under the SOURCE -- at any game version, not
                // only the one `fromVersion()` resolves to?
                //
                // GlobalIniClassifiers registers every version's vb0 for a reason it states: a
                // mod's .ini carries whichever version's hashes its author dumped, and nothing can
                // tell which that was. Matching only the resolved one here made the two disagree
                // the moment a character's vb0 moved -- ChisaParfait's did at WuWa 3.7 -- so a mod
                // of the other generation classified as her and then found no section of its own.
                // The caller's slot test is what keeps this safe: the section must still carry a
                // match_first_index one of the source's slots declares AT `fromVersion()`, so this
                // can only turn a rejection into a match where the geometry agrees too.
                bool isSourceVb0(const std::string& hash) const {
                    if (hash == source_.vb0Hash) {
                        return true;
                    }

                    const ModType* source = ctx_.modType();
                    if (source == nullptr || source->hashes == nullptr) {
                        return false;
                    }

                    return source->hashes->hasFrom(hash, std::nullopt,
                                                   {std::optional<std::string>(source->name),
                                                    std::optional<std::string>(Vb0HashKey)});
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

                    return Version::parse(config_.version);
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

                    std::optional<Character> src = readCharacter(*source, fromVersion(), config_.slotPrefix, error);
                    if (!src.has_value()) {
                        return false;
                    }

                    std::optional<Character> dst = readCharacter(*targetType_, toVersion(), config_.slotPrefix, error);
                    if (!dst.has_value()) {
                        return false;
                    }

                    source_ = std::move(*src);
                    target_ = std::move(*dst);

                    // The names a fix of this .ini could have written its sections under -- the
                    // source's and the target's. IniFileRemoveContext::modTypeNames builds the same
                    // list for the undo, from every type the .ini classified as and everything each
                    // remaps onto; a fixer is built for ONE pair, which is this one.
                    remapNames_ = {source_.name, target_.name};
                    return true;
                }

                // ---- what the mod's .ini has: its slot sections, its vertex count, its folders ----

                bool readMod(std::string& error) {
                    IniFile* ini = ctx_.getIniFile();
                    if (ini == nullptr) {
                        error = "the fixer has no .ini file";
                        return false;
                    }

                    hostResetsDetected_ = presentResetsObjectDetected();

                    // THE MOD'S ANATOMY IS THE PARSER'S (2026-10-28): which section is which source component,
                    // what each draws and binds after drawing, which components the mod hides, and the files
                    // its resources name -- see IniParseData. A fixer is built after its parser has parsed.
                    const IniParseData& data = parseData();
                    silent_.clear();
                    if (auto* parser = dynamic_cast<GIMIParser<>*>(this->getParser())) {
                        for (const auto& [obj, sections] : parser->sectionTargets()) {
                            const std::optional<int> component = sourceComponentOf(obj);
                            if (!component.has_value() || sections.empty()) {
                                continue;
                            }

                            // A COMPONENT THE MOD HIDES IS AN UNDRAWN SLOT (2026-10-10): its section only skips
                            // the game's geometry. Remapped like a drawing one, it got none of the fix's additions
                            // and the target slot's bone window was never merged (Lynae4's legs collapsed to
                            // spikes); left out, it gets the hide section every undrawn slot gets.
                            if (data.isUndrawn(obj)) {
                                silent_.push_back(*component);
                                continue;
                            }

                            present_[*component] = sections;
                            graphOf_[*component] = obj;

                            // Its draw ranges, read for EVERY mod: the remap of a toggled draw through the split
                            // and TexEditContext::drawRanges (what a texture edit knows a component covers) both
                            // need them
                            if (!data.drawRanges(obj).empty()) {
                                drawRanges_[*component] = data.drawRanges(obj);
                            }
                            if (!data.drawParts(obj).empty()) {
                                drawParts_[*component] = data.drawParts(obj);
                            }

                            // TWO BINDING GENERATIONS IN ONE SECTION (2026-10-03): draws before the section's
                            // own `ps-t` lines render with the SOURCE's atlas. ChisaParfait3's hat and chest
                            // bandages rendered as flat skin until each draw got its own list.
                            if (!data.lateBindings(obj).empty()) {
                                lateBindRegs_[*component] = data.lateBindings(obj);
                            }
                        }
                    }

                    if (present_.empty() && silent_.empty()) {
                        error = "no [TextureOverrideComponent*] section on any vb0 hash the library"
                                " files under " + source_.name + " (" + source_.vb0Hash + " at the version asked)";
                        return false;
                    }

                    // A mod from before WWMI's merged skeleton: no section of it carries a vg_offset,
                    // because a draw could then only address its own component's bones -- so its blend
                    // holds each component's OWN indices. See WWMIFixerConfig::sourceVgMaps.
                    legacy_ = true;
                    std::vector<std::string> slotSections;
                    for (const auto& entry : present_) {
                        slotSections.insert(slotSections.end(), entry.second.begin(), entry.second.end());
                    }
                    if (auto* parser = dynamic_cast<GIMIParser<>*>(this->getParser())) {
                        for (int component : silent_) {
                            const auto found = parser->sectionTargets().find(ModObj("", config_.slotPrefix + std::to_string(component)));
                            if (found != parser->sectionTargets().end()) {
                                slotSections.insert(slotSections.end(), found->second.begin(), found->second.end());
                            }
                        }
                    }
                    for (const std::string& section : slotSections) {
                        if (data.firstCollected(IniParseRows::Objs, section, IniKeywords::WWMIVgOffset) != nullptr) {
                            legacy_ = false;
                        }
                    }

                    if (legacy_) {
                        if (config_.sourceVgMaps.empty()) {
                            error = "this mod is from before WWMI's merged skeleton (no `" + IniKeywords::WWMIVgOffset + "` in any of its sections), "
                                    "so its blend holds each component's OWN bone indices -- and " + source_.name
                                    + "'s config carries no sourceVgMaps to lift them with. Remapping them as merged indices would "
                                      "scramble every bone of the body, so this mod is left alone";
                            return false;
                        }

                        ctx_.log("this mod is from before WWMI's merged skeleton: its blend is read per component and lifted "
                             "through " + source_.name + "'s vg_map, and the fix supplies the merged skeleton it lacks");
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

                    // ITS BUFFERS, off the .ini rather than as sibling names: a mod-manager-packaged mod names
                    // every file by GUID with a `.assets` extension, so there is no `Position.buf` beside
                    // anything -- built as a sibling, the path did not exist, the remapped blend was never
                    // written, and the .ini still bound vb4 to it, which drew NOTHING of the character.
                    // Found without regard to case: one mod spells it ResourceTexCoordBuffer.
                    for (const auto& [section, into] : std::vector<std::pair<std::string, std::string*>>{
                             {IniKeywords::WWMIIndexBuffer, &indexFile_}, {IniKeywords::WWMIPositionBuffer, &positionFile_},
                             {IniKeywords::WWMITexcoordBuffer, &texcoordFile_}, {IniKeywords::WWMIVectorBuffer, &vectorFile_}}) {
                        if (std::optional<std::string> file = data.resourceFile(section)) {
                            *into = *file;
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

                    // WHERE the 16-bit ids live, off the .ini rather than a sibling filename: a
                    // mod-manager-packaged mod names every file by GUID with a `.assets` extension,
                    // so the WWMI export's own name is not beside Blend.buf and the search finds
                    // nothing -- and the fallback that then runs is a no-op dressed as a fix.
                    for (const auto& [section, file] : data.resourceFiles()) {
                        std::string name = section;
                        name.erase(std::remove(name.begin(), name.end(), '_'), name.end());
                        if (name.find("blendremapvertexvg") != std::string::npos) {
                            vertexVGFile_ = file;
                            break;
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

                // ---- the textures: the run's index, and what this .ini binds of it ----

                // WHAT readTextures WORKS OUT, IN THE ORDER IT WORKS IT OUT.
                //
                // A struct rather than members: this is scan state, dead the moment the scan ends,
                // and passing it explicitly is what makes the order the phases run in visible at the
                // call site. They were one 650-line function sharing these by scope, which is how
                // `best` came to mean two different things in it.
                struct TextureScan {
                    const IniParseData* data = nullptr;     // the parser's answer: each file's roles, and each slot's file per role
                    const TextureRoles* roles = nullptr;
                    std::string iniFolder;
                    std::string iniPath;

                    // a tie between two files is reported once, however many components are offered the role
                    std::set<std::string> saidAmbiguous;
                };

                void readTextures() {
                    TextureScan scan;
                    IniFile* ini = ctx_.getIniFile();
                    scan.iniFolder = ini->getFolder();
                    scan.iniPath = ini->getFile().value_or("");
                    // WHAT THE MOD'S TEXTURES ARE IS THE PARSER'S ANSWER, NOT THIS ONE'S.
                    // A texture is identified by a HASH and a REGISTER -- the hash being the
                    // texture's own (the section IS that texture) or the mesh's (the section is a
                    // draw the classifier placed, and the register says what it binds) -- which is
                    // section classification, so the parser does it and this asks.
                    //
                    // What it replaced walked the folder, climbing up to three parents, and guessed
                    // each file from its name, a hash inside its name, or its pixels. A file the mod
                    // never binds was therefore a candidate: Chisa7 keeps spare colourways beside
                    // the installed one and a spare won `upperDiffuse`, which is what turned a
                    // kimono red. Over ten mods the scan offered 51 such files; none of them is a
                    // candidate now (2026-09-29).
                    if (this->getParser() != nullptr) {
                        scan.data = &this->getParser()->parseData();
                        scan.roles = scan.data->textureRoles();
                    }

                    if (scan.roles == nullptr) {
                        // Every WuWa character's parse row carries its textures, so this is a
                        // configuration error rather than a mod's doing -- and a fix that cannot
                        // tell what any texture is would bind the game's over all of them.
                        giveUp("this mod's textures were not identified: the character's parse row "
                               "carries no textures, so every role would fall back to a download");
                        return;
                    }

                    // which resource names each file is the parser's (IniParseData::resourceOfFile); the other
                    // way, so an edit can reach EVERY variant a toggled role binds rather than only the one
                    // fileOfRole_ resolved to
                    for (const auto& entry : scan.roles->resourcesOf(scan.iniPath)) {
                        fileOfResource_.emplace(StringTools::toLower(entry.first), entry.second);
                    }

                    // THE FIX'S OWN TEXTURES -- a download, a created texture -- GO BESIDE THE .ini, the
                    // library's default (an edit goes beside the file it edits). Picking the folder most of
                    // the mod's textures sat in was a vote, and a mod whose textures are spread evenly
                    // over its folders gave it no answer (the maintainer, 2026-10-31).
                    dumpTextureRoles(scan);
                    readRefusals(scan);
                    applyTextureRoles(scan);
                }

                // AGREMAP_WWMI_ROLES=1: one line per file per role, so the identification this does
                // can be diffed against the one the parser is taking over. Off by default; the
                // compiled fixer is silent by request.
                void dumpTextureRoles(const TextureScan& scan) {
                    if (std::getenv("AGREMAP_WWMI_ROLES") != nullptr) {
                        for (const auto& entry : scan.roles->rolesOf()) {
                            for (const TextureRoles::Role& role : entry.second) {
                                std::fprintf(stderr, "WWMIROLE\t%s\t%s\t%s\t%s\n", scan.iniPath.c_str(),
                                             FileService::pathKey(entry.first).c_str(),
                                             role.role.c_str(), role.how.c_str());
                            }
                        }
                    }

                }

                // The files the parser refused for a role (TextureRoleChooser): each is said (a carried
                // line naming one asks IniParseData::refusedRoleOf). A role in
                // WWMIFixerConfig::flatLeftToGame with nothing of the mod's left is left to the game:
                // no fallback download either.
                void readRefusals(TextureScan& scan) {
                    std::set<std::string> refused;
                    for (const IniParseData::Rejection& rejection : scan.data->rejections()) {
                        const bool toGame = config_.flatLeftToGame.count(rejection.role) > 0;
                        refused.insert(rejection.role);
                        ctx_.log(FileService::getRelPath(rejection.file, scan.iniFolder) + " " + rejection.why + "; "
                                 + (toGame ? "left to the game" : "the source's own is used instead"));
                    }

                    for (const std::string& role : refused) {
                        if (config_.flatLeftToGame.count(role) > 0 && scan.data->roleCandidates(role).empty()) {
                            leftToGame_.insert(role);
                        }
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

                // Which of the mod's files serves one role on one source component, if any.
                //
                // The choice is per (role, source component), not per role -- but an ambiguity
                // BETWEEN TWO FILES is a property of the files, so it is reported once however many
                // components are offered the role (hence TextureScan::saidAmbiguous).
                void assignRole(TextureScan& scan, const std::string& role, int component) {
                        {
                            const IniParseData::RoleFile* chosen = scan.data->roleFile(role, {"", config_.slotPrefix + std::to_string(component)});
                            if (chosen == nullptr || resourceOfSlotRole_.count({role, component}) > 0) {
                                return;
                            }

                            const std::string& best = chosen->file;
                            if (!chosen->tiedFile.empty()
                                    && scan.saidAmbiguous.insert(role + "\n" + best + "\n" + chosen->tiedFile).second) {
                                // Two shipped textures equally close on one role: the first is bound
                                // and only a measurement can say which is right -- say so loudly.
                                ctx_.log("WARNING: " + FileService::getRelPath(chosen->tiedFile, scan.iniFolder)
                                         + " also has the role " + role + " (" + chosen->tiedHow
                                         + "), already taken by " + FileService::getRelPath(best, scan.iniFolder)
                                         + " (" + chosen->how + "); the first one is bound");
                            }

                            // AGREMAP_WWMI_PICK=1: which FILE the fix chose for each (role,
                            // component), which is the thing a change to the ranking moves.
                            if (std::getenv("AGREMAP_WWMI_PICK") != nullptr) {
                                std::fprintf(stderr, "WWMIPICK\t%s\t%s\t%d\t%s\n", scan.iniPath.c_str(),
                                             role.c_str(), component,
                                             FileService::pathKey(best).c_str());
                            }

                            fileOfRole_[role] = best;
                            const std::optional<std::string> own = scan.data->resourceOfFile(best);

                            // ...unless a PREVIOUS run of the fix is what declared it. A fix undoes
                            // first and the undo removes every section a fix named, so binding
                            // `[Resource<Role><Target>RemapRef]` -- which exists precisely to point
                            // at one of the MOD'S files -- writes a reference the same run deletes:
                            // 65 dangling `ps-t` bindings across the two corpus mods that arrive
                            // already fixed (Chisa9, Sanhua3), each naming a section nothing
                            // declares, which in game leaves whatever was last in the register. The
                            // file is still the mod's, so it falls through and gets a fresh
                            // declaration below rather than being dropped (2026-09-30).
                            //
                            // IniNamingTools::looksRemapped is the undo's own test, so the two
                            // cannot disagree about what is about to be removed.
                            if (own.has_value() && !IniNamingTools::looksRemapped(*own, remapNames_)) {
                                resourceOfSlotRole_[{role, component}] = *own;
                                return;
                            }

                            // A file no resource of this .ini names gets a resource section of the fix's
                            // own -- named with RemapRef, not RemapFix: the section sits inside the fix's
                            // block and names one of the MOD's files, and an undo deletes what a RemapFix
                            // section names. Two files of one role each get their own.
                            auto declaredName = declaredName_.find(best);
                            if (declaredName == declaredName_.end()) {
                                std::string name = IniKeywords::Resource + TextTools::capitalize(role) + toModName_ + IniKeywords::RemapRef;
                                for (std::size_t n = 2; usedDeclaredNames_.count(name) > 0; ++n) {
                                    name = IniKeywords::Resource + TextTools::capitalize(role) + std::to_string(n) + toModName_ + IniKeywords::RemapRef;
                                }

                                usedDeclaredNames_.insert(name);
                                declaredName = declaredName_.emplace(best, name).first;
                                const std::string rel = FileService::pathToIniStr(
                                    FileService::strToPath(FileService::getRelPath(best, scan.iniFolder)));
                                declared_.emplace_back(best, rel);
                            }

                            resourceOfSlotRole_[{role, component}] = declaredName->second;
                        }
                }

                // Every role offered to every component that could serve it -- the plan's, then the ones
                // only a slot's OTHER passes or a shared mesh bind (Chisa's accessoryNormal: until these were
                // asked it could only come from the download, bound over the mod's own normal map) -- then the
                // roles the fix invents, and a stand-in for every role no file of the mod's serves.
                void applyTextureRoles(TextureScan& scan) {
                    for (const auto& planned : config_.plan) {
                        if (present_.count(planned.first) > 0) {
                            for (const WWMIFixerConfig::Binding& binding : bindingsOf(planned.first)) {
                                assignRole(scan, binding.role, planned.first);
                            }
                        }
                    }
                    for (const std::string& role : passOnlyRoles()) {
                        for (const auto& entry : present_) {
                            assignRole(scan, role, entry.first);    // a no-op on a role resolved or one no file serves
                        }
                    }

                    for (const WWMIFixerConfig::CreatedTexture& created : config_.createdTextures) {
                        resourceOfRole_[created.role] = remapFixName(IniKeywords::Resource + created.role);
                    }

                    if (config_.sourceTextures.downloadCharFolder.empty()) {
                        return;
                    }

                    // a planned role a component has no file for; a role a shader names in a `role <name>` line
                    // (a texture of the source's no mod ships: her lighting ramp -- a reroute's plan entry counts
                    // when the component it draws from is present); a role only other passes bind, that no
                    // component owns
                    for (const auto& planned : config_.plan) {
                        if (present_.count(planned.first) > 0) {
                            for (const WWMIFixerConfig::Binding& binding : bindingsOf(planned.first)) {
                                if (resourceOfSlotRole_.count({binding.role, planned.first}) == 0) {
                                    standIn(binding.role);
                                }
                            }
                        }

                        bool drawn = present_.count(planned.first) > 0;
                        for (const auto& reroute : config_.drawReroutes) {
                            drawn = drawn || (reroute.as == planned.first && present_.count(reroute.from) > 0);
                        }
                        for (const std::string& role : drawn ? shaderLineRoles(planned.first) : std::vector<std::string>{}) {
                            standIn(role);
                        }
                    }
                    for (const std::string& role : passOnlyRoles()) {
                        const bool owned = std::any_of(resourceOfSlotRole_.begin(), resourceOfSlotRole_.end(),
                                                       [&role](const auto& entry) { return entry.first.first == role; });
                        if (!owned) {
                            standIn(role);
                        }
                    }
                }

                // ---- THE STAND-IN RULE ----
                //
                // What binds a role where the mod's own file does not reach, in two cases:
                //   - the mod has NO file for it: the texture the fix invents (createdTextures, set above), else
                //     the SOURCE's own game texture, downloaded -- the mod's UVs are the source's, and the
                //     target's texture at them is wrong by construction (#standIn)
                //   - its file reaches only SOME draws: a role bound only under a toggle, a one-variant toggle, the
                //     draws a section makes before its first binding. The source's own game texture is the default
                //     there, and the mod's own lines rebind its file inside their branches (#planGameDefaults)
                // A role in WWMIFixerConfig::flatLeftToGame with nothing of the mod's left gets neither: the game's
                // own is what the mod rendered with.

                // The source's own game texture standing in for 'role', which no file of the mod's serves
                void standIn(const std::string& role) {
                    const auto fallback = config_.sourceTextures.fallbackTextures.find(role);
                    if (resourceOfRole_.count(role) == 0 && leftToGame_.count(role) == 0
                            && fallback != config_.sourceTextures.fallbackTextures.end()) {
                        registerFallback(role, fallback->second);
                    }
                }

                // ---- the edits ----

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
                    if (positionFile_.empty()) {
                        throw std::runtime_error(
                            "cannot fix this mod: its .ini declares no [" + IniKeywords::WWMIPositionBuffer
                            + "], so there is no mesh to read a vertex count from");
                    }

                    return FileService::absPathOfRelPath(
                        positionFile_, ctx_.getIniFile()->getFolder());
                }

                static std::string modFile(const std::string& folder, const std::string& name) {
                    return folder.empty() ? name : folder + "/" + name;
                }

                ModObj targetSlotObj(int slot) const {
                    return ModObj(toModName_, config_.slotPrefix + std::to_string(slot));
                }

                // Whether the fix binds 'role' on a component: its plan row's, or a role only its slot's
                // other passes or a shared mesh bind (passOnlyRoles)
                bool bindsRole(int component, const std::string& role) {
                    for (const WWMIFixerConfig::Binding& binding : bindingsOf(component)) {
                        if (binding.role == role) {
                            return true;
                        }
                    }
                    const std::vector<std::string> extra = passOnlyRoles();
                    return std::find(extra.begin(), extra.end(), role) != extra.end();
                }

                // The source component of one of the parser's slot objects, or nothing
                std::optional<int> sourceComponentOf(const ModObj& obj) const {
                    if (!obj.first.empty() || !StringTools::startsWith(obj.second, config_.slotPrefix)) {
                        return std::nullopt;
                    }

                    try {
                        const int component = std::stoi(obj.second.substr(config_.slotPrefix.size()));
                        if (component >= 0 && static_cast<std::size_t>(component) < source_.slots.size()) {
                            return component;
                        }
                    } catch (const std::exception&) {
                    }
                    return std::nullopt;
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

                    return ModObj(toModName_, config_.slotPrefix + std::to_string(planned.slot) + "_"
                                                  + std::to_string(component));
                }

                // Does the TARGET's merged skeleton pass what an 8-bit blend index can name, and can
                // one WWMI blend remap hold the row? Sets #targetPast256_ and #blendRemapBones_,
                // which every later phase reads.
                void readBlendWidth(const ModType* source, const std::optional<Version>& from,
                                     const std::optional<Version>& to) {
                    // Does the TARGET's merged skeleton pass what an 8-bit blend index can name?
                    // The row's largest target id answers it, and it is a property of the PAIR --
                    // so the .ini and the buffers, written at different moments, cannot disagree
                    // about whether there is a blend remap. Chisa reaches 418; ChisaParfait, going
                    // the other way, stops at 250 and needs none.
                    if (source != nullptr) {
                        const std::optional<VGRemap> row = source->getVGRemap(toModName_, from, to);
                        if (row.has_value()) {
                            std::set<long long> targets;
                            for (const auto& [srcBone, dstBone] : row->getRemap()) {
                                (void)srcBone;
                                targets.insert(dstBone);
                                if (dstBone > WWMIBlend::MaxByteBone) {
                                    targetPast256_ = true;
                                }
                            }

                            blendRemapBones_ = targets.size();

                            // A row naming more distinct targets than one remap holds would need
                            // WWMI's per-component scheme back.
                            //
                            // THIS SAID "refused rather than truncated" AND THEN TRUNCATED (noted
                            // 2026-09-29). Clearing `targetPast256_` sends the fix to an 8-bit lift,
                            // whose ids are one byte -- so every bone past 255 lands on bone 0, the
                            // exact outcome the comment claimed to be avoiding. `writeBlendRemap`
                            // does refuse (it throws), but it is unreachable once this has turned
                            // the path off.
                            //
                            // The line now says what happens rather than the opposite. Which of the
                            // two guards should win -- fall back and lose the far bones, or refuse
                            // the mod outright -- is the maintainer's call, and both are in game.
                            if (targetPast256_ && blendRemapBones_ > WWMIBlend::RemapSize) {
                                ctx_.log("the vertex group row names " + std::to_string(blendRemapBones_)
                                     + " distinct target bones, past the "
                                     + std::to_string(WWMIBlend::RemapSize)
                                     + " one blend remap holds; falling back to an 8-bit blend, so"
                                     + " every bone past " + std::to_string(WWMIBlend::MaxByteBone)
                                     + " will land on bone 0");
                                targetPast256_ = false;
                            }
                        }
                    }
                }

                void buildEdits() {
                    buildTexcoordCopy();
                    const ModType* source = ctx_.modType();
                    const std::optional<Version> from = fromVersion();
                    const std::optional<Version> to = toVersion();

                    readBlendWidth(source, from, to);

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
                        for (const char* reg : {"cs-t33", "cs-t0", "cs-t1"}) {
                            vals.emplace_back(reg, RegNewVals<>::NewVal(rebind));
                        }

                        shapeKeyEdit_ = std::make_unique<RegNewVals<>>(std::move(vals));
                        shapeKeyAdapter_ = std::make_unique<RegPartEdit<>>(shapeKeyEdit_.get());
                    }

                    // In this order because each reads what the one before it set: the removals
                    // need the blend width, the groups decide which slots are drawn, and everything
                    // after that is per drawn slot.
                    buildBypasses();
                    buildRegRemovals();
                    buildGroups();
                    buildSlotValues();
                    buildComponentAdditions();
                    buildRerouteFilters();
                    buildSlotRemap();
                    buildPerGroupEdits();
                    buildBlendCollects(source, vgRemapVersion(from), to);
                }

                // The version of the VGRemaps row the mod's blend is read with: the numbering the parser read off
                // the mod's own geometry (IniParseData::blendVersion), else 'from'
                std::optional<Version> vgRemapVersion(const std::optional<Version>& from) {
                    if (legacy_) {
                        return from;
                    }
                    const std::optional<std::string>& version = parseData().blendVersion();
                    return version.has_value() ? std::optional<Version>(Version::parse(*version)) : from;
                }

                /** @brief The fix's own list that puts the captured bindings back -- see bypassRestore */
                std::string restoreListName() {
                    return remapFixName(IniKeywords::CommandList + "RestoreSharedResources");
                }

                // Which of the mod's own shared-resource bindings the fix has to put back, and the
                // resource each is parked in. Read off the mod's OWN two lists rather than
                // configured: whatever the override binds and the cleanup does not already restore.
                void buildBypasses() {
                    bypasses_.clear();
                    if (ownRows(config_.sharedResourcesList).empty() || ownRows(config_.cleanupResourcesList).empty()) {
                        return;            // a legacy mod with no such pair: nothing to be symmetric with
                    }

                    std::vector<std::string> bound = buffersBoundBy(config_.sharedResourcesList);
                    const std::vector<std::string> already = buffersBoundBy(config_.cleanupResourcesList);
                    for (const std::string& reg : bound) {
                        if (std::find(already.begin(), already.end(), reg) != already.end()) {
                            continue;      // `vb0`, which WWMI's own pair already handles
                        }

                        bypasses_.emplace_back(reg, remapFixName(IniKeywords::Resource + "Bypass"
                                                            + IniNamingTools::getRegTag(reg)));
                    }
                }

                /**
                 * @brief The MESH buffer registers a command list assigns to
                 *
                 * `ib` and `vb<n>` only. A `Resource... = ref vb0` capture line is an assignment to a
                 * RESOURCE and is not one of these, which is what keeps the pair's own bookkeeping out
                 * of the answer.
                 */
                std::vector<std::string> buffersBoundBy(const std::string& list) const {
                    std::vector<std::string> regs;
                    for (const Row* row : ownRows(list)) {
                        const std::string key = StringTools::toLower(StringTools::strip(row->key));
                        bool buffer = (key == IniKeywords::Ib);
                        if (!buffer && key.size() > 2 && key.compare(0, 2, "vb") == 0) {
                            buffer = std::all_of(key.begin() + 2, key.end(),
                                                 [](unsigned char c) { return std::isdigit(c) != 0; });
                        }

                        if (buffer) {
                            ListTools::pushDistinct(regs, key);
                        }
                    }

                    return regs;
                }

                // Lines a remapped section drops -- see WWMIFixerConfig::removedRegs for why each
                // kind is there. Built once and hung on every remapped slot section.
                void buildRegRemovals() {
                    std::vector<WWMIFixerConfig::RegRemoval> removals = config_.removedRegs;
                    if (targetPast256_) {
                        removals.insert(removals.end(), config_.remappedSkeletonRemovals.begin(),
                                        config_.remappedSkeletonRemovals.end());
                    }

                    if (!removals.empty()) {
                        // each value matched with or without `ref`: an identity mod writes `vs-cb4 = ResourceMergedSkeleton`
                        // and WWMI Tools `vs-cb4 = ref ResourceMergedSkeleton`, and ChisaFixer.cpp's
                        // `{"ResourceBlendBufferOverride", "ref"}` drops a binding however it reaches its resource
                        std::vector<std::pair<std::string, std::optional<RegRemove<>::RemoveKeyCheck>>> keys;
                        keys.reserve(removals.size());
                        for (const WWMIFixerConfig::RegRemoval& removal : removals) {
                            keys.emplace_back(removal.reg, removal.valuePrefix.empty()
                                                               ? std::nullopt
                                                               : std::optional<RegRemove<>::RemoveKeyCheck>(
                                                                     RegRemove<>::valueStartsWith(removal.valuePrefix)));
                        }

                        // the same without RabbitFX's own lines, for a component that keeps them
                        // (SourceComponent::rabbitFX)
                        std::vector<std::pair<std::string, std::optional<RegRemove<>::RemoveKeyCheck>>> keepRabbitFX;
                        for (std::size_t i = 0; i < removals.size(); ++i) {
                            if (!isRabbitFXRemoval(removals[i])) {
                                keepRabbitFX.push_back(keys[i]);
                            }
                        }

                        regRemove_ = std::make_unique<RegRemove<>>(std::move(keys));
                        removeAdapter_ = std::make_unique<RegPartEdit<>>(regRemove_.get());
                        if (!keepRabbitFX.empty()) {
                            regRemoveKeepRabbitFX_ = std::make_unique<RegRemove<>>(std::move(keepRabbitFX));
                            removeKeepRabbitFXAdapter_ = std::make_unique<RegPartEdit<>>(regRemoveKeepRabbitFX_.get());
                        }
                    }

                }

                // The groups: one remapped section per target draw per file. The first source
                // component claiming a slot stays in the mod's own .ini, the second lands in the
                // first copy, and so on -- in the source components' numeric order, which is the
                // order the remap below creates the target graphs in and so the order collisions
                // are resolved in.
                void buildGroups() {
                    std::map<int, std::size_t> claimants;
                    for (const auto& entry : present_) {
                        auto planned = config_.plan.find(entry.first);
                        if (planned == config_.plan.end()) {
                            dropped_.push_back(entry.first);
                            continue;
                        }

                        // a second section in the mod's own file (SourceComponent::sameFileSection)
                        // claims no copy
                        const std::size_t group = planned->second.sameFileSection ? 0 : claimants[planned->second.slot]++;
                        if (groups_.size() <= group) {
                            groups_.resize(group + 1);
                        }

                        groups_[group].push_back(entry.first);
                        drawnSlots_.insert(planned->second.slot);
                    }

                    if (groups_.empty()) {
                        groups_.resize(1);
                    }

                    // A COMPONENT THE PLAN DOES NOT NAME IS THROWN AWAY, AND USED TO SAY NOTHING.
                    // `dropped_` was collected here and read nowhere, so a mod carrying geometry
                    // this character has no slot for lost it with every line of the run reporting
                    // success -- the part is missing in game and the log that would have named it
                    // was a vector nobody printed.
                    if (!dropped_.empty()) {
                        std::string names;
                        for (const int component : dropped_) {
                            names += (names.empty() ? "" : ", ") + config_.slotPrefix
                                     + std::to_string(component);

                            // `sourceLabels` exists for exactly this and had never been read:
                            // `component3` tells a user nothing that `component3 (torso, arms,
                            // ribbons)` does not tell them better.
                            const auto label = config_.sourceLabels.find(component);
                            if (label != config_.sourceLabels.end() && !label->second.empty()) {
                                names += " (" + label->second + ")";
                            }
                        }

                        ctx_.log("this mod draws " + std::to_string(dropped_.size())
                             + (dropped_.size() == 1 ? " component " : " components ") + names
                             + " that " + source_.name + "'s plan has no target slot for -- "
                             + (dropped_.size() == 1 ? "it is" : "they are") + " not remapped");
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
                        if (present_.count(component) == 0) {
                            continue;
                        }

                        const WWMIFixerConfig::SourceComponent& planned = entry.second;
                        RegSurroundedAdd<>::Additions additions;
                        if (legacy_ || targetPast256_) {
                            // The fix's own merge list, not the mod's: for a legacy mod because it
                            // has none, and for a target past 256 bones because the mod's own
                            // merged skeleton is declared for 256 and the target's bones run past it.
                            additions.emplace_back(IniKeywords::Run, mergeListName(planned.slot));
                        }

                        if (targetPast256_) {
                            // After the merge list, so the private skeleton exists, and after the
                            // shared override, whose `vb4` this rebinds to the remapped blend.
                            additions.emplace_back(IniKeywords::Run, blendRemapInitList());
                        }

                        if (config_.zeroShapeKeyStream && meshVertexCount_ > 0) {
                            additions.emplace_back(config_.shapeKeyStreamReg, remapFixName(IniKeywords::Resource + ShapeKeyZero));
                        }

                        if (texcoordResource_.has_value()) {
                            additions.emplace_back(config_.texcoordReg, *texcoordResource_);
                        }

                        // THE COMPONENT'S TEXTURE LIST: each role of its plan row bound at the target's register
                        // -- a role the mod toggles against the game's texture by its toggle (planGameDefaults),
                        // `null` as nothing, and the rest by bindLine
                        const std::string passGuard = passCondition(config_.slotPasses.at(static_cast<std::size_t>(planned.slot)));
                        TextureLines main;
                        for (const WWMIFixerConfig::Binding& binding : bindingsOf(component)) {
                            if (binding.role == IniKeywords::Null) {
                                main.lines.push_back(nullLine(binding.reg));
                            } else if (const auto toggled = toggledGame_.find({binding.role, component}); toggled != toggledGame_.end()) {
                                main.bind(binding, {binding.reg, toggled->second});
                            } else if (const std::string* resource = resourceFor(binding.role, component)) {
                                main.bind(binding, bindLine(binding.role, binding.reg, *resource, rerouteShaders_.count(component) > 0));
                            }
                        }

                        // the registers the list binds -- a toggled role's `run =` line binds its register too
                        std::set<std::string> listRegs;
                        for (const Kvp& kvp : main.lines) {
                            listRegs.insert(StringTools::toLower(kvp.first));
                        }
                        for (const auto& [role, reg] : main.bound) {
                            listRegs.insert(StringTools::toLower(reg));
                        }

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
                            editsOf_[component].push_back(topAdapter.get());
                            surroundedAdds_.push_back(std::move(top));
                            graphAdapters_.push_back(std::move(topAdapter));
                        }

                        // GENERATION 0: THE DRAWS THIS SECTION MAKES BEFORE IT BINDS ANYTHING
                        // (2026-10-03). They rendered with the SOURCE's own game textures, so that
                        // is what they are given -- see lateBindRegs_ and planGameDefaults. Run with the
                        // additions, after the list above, so it wins for those draws.
                        if (lateBindRegs_.count(component) > 0 && !gameResourceOfRole_.empty()) {
                            TextureLines game;
                            for (const WWMIFixerConfig::Binding& binding : bindingsOf(component)) {
                                if (binding.role != IniKeywords::Null && gameResourceOfRole_.count(binding.role) > 0) {
                                    game.bind(binding, bindLine(binding.role, binding.reg, gameResourceFor(binding.role, component),
                                                                rerouteShaders_.count(component) > 0));
                                }
                            }

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
                                TextureLines passLines;
                                for (const WWMIFixerConfig::Binding& binding : regs) {
                                    // A binding for ONE source, on a slot two sources merge onto --
                                    // see Binding::srcComponent. The default -1 takes every source,
                                    // which is what every config written before 2026-09-28 means.
                                    if (binding.srcComponent >= 0 && binding.srcComponent != component) {
                                        continue;
                                    }

                                    if (const std::string* resource = resourceFor(binding.role, component)) {
                                        passLines.bind(binding, bindLine(binding.role, binding.reg, *resource,
                                                                         rerouteShaders_.count(component) > 0));
                                    }
                                }

                                // THE DRAW, SUPPRESSED ON THIS PASS (2026-10-03) -- see
                                // WWMIFixerConfig::extraPassNoDraw. Inside the list, which is
                                // already gated on the pass, so it reaches that pass alone.
                                const auto noDraw = config_.extraPassNoDraw.find(planned.slot);
                                if (noDraw != config_.extraPassNoDraw.end() && noDraw->second.count(pass) > 0) {
                                    passLines.lines.emplace_back(IniKeywords::Ib, IniKeywords::Null);
                                }

                                if (!passLines.lines.empty()) {
                                    const std::string extraList = textureListName(component, "", "Pass" + std::to_string(n));
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
                            for (auto& [bypass, sections] : bypassRestore(listNames(config_.sharedResourcesList),
                                                                          listNames(config_.cleanupResourcesList),
                                                                          bypasses_, restoreListName())) {
                                auto adapter = std::make_unique<RegPartEdit<>>(
                                    bypass.get(), [sections = sections](const std::string& name) { return namesSection(sections, name); });
                                editsOf_[component].push_back(adapter.get());
                                bypassEdits_.push_back(std::move(bypass));
                                regAdapters_.push_back(std::move(adapter));
                            }
                        }

                        // THE MOD'S OWN `ps-t` LINES, RE-KEYED BY ROLE (2026-10-02), each to the register this
                        // component's plan row reads its texture's role at -- the layout of the fix's own list,
                        // which they sit after. Before the twin, because it reads what the mod wrote and the
                        // twin only adds draws. See CarriedLines.
                        {
                            std::map<std::string, std::string> regOfRole;
                            for (const WWMIFixerConfig::Binding& binding : bindingsOf(component)) {
                                if (binding.role != IniKeywords::Null) {
                                    regOfRole.emplace(binding.role, binding.reg);
                                }
                            }

                            // The register for a role: the row's own, or else the one the row binds the same
                            // KIND at, for a role of another component. `ResourceTexture4_3` is a `lowerDiffuse`
                            // and this row names `panelDiffuse`; both are the config's OWN names, so their shared
                            // tail is a convention this repo controls rather than a guess about the mod. Only
                            // where the kind is unambiguous in the row -- `irisDiffuse` and `eyeDiffuse` share
                            // one, at different registers.
                            const auto regOf = [&regOfRole](const std::string& role) -> std::optional<std::string> {
                                if (const auto found = regOfRole.find(role); found != regOfRole.end()) {
                                    return found->second;
                                }

                                const std::string kind = roleKind(role);
                                std::optional<std::string> byKind;
                                for (const auto& [rowRole, reg] : regOfRole) {
                                    if (kind.empty() || roleKind(rowRole) != kind) {
                                        continue;
                                    }
                                    if (byKind.has_value() && *byKind != reg) {
                                        return std::nullopt;
                                    }
                                    byKind = reg;
                                }
                                return byKind;
                            };

                            // see WWMIFixerConfig::carryByRegisterRole: the role the source reads at each register
                            // -- a reroute reads its source component's layout, it is that component's art
                            const auto alias = rerouteFrom_.find(component);
                            const int ownSource = alias != rerouteFrom_.end() ? alias->second : component;
                            std::optional<std::map<std::string, std::string>> slotRoles;
                            if (config_.carryByRegisterRole) {
                                slotRoles.emplace();
                                const auto own = config_.sourceTextures.registerRoles.find(ownSource);
                                if (own != config_.sourceTextures.registerRoles.end()) {
                                    for (const auto& [reg, role] : own->second) {
                                        slotRoles->emplace(StringTools::toLower(reg), role);
                                    }
                                }
                            }

                            const IniParseData& data = parseData();
                            // the line in this one's place -- no answer leaves it alone, an empty line drops it
                            const auto fateOf = [&](const std::string& key, const std::string& value) -> std::optional<std::optional<Kvp>> {
                                // `ps-t1 = ref ResourceFoo` names the same resource as `ps-t1 = ResourceFoo`
                                std::string resource = value;
                                const std::string ref = StringTools::toLower(IniKeywords::Ref);
                                if (StringTools::startsWith(StringTools::toLower(resource), ref + " ")) {
                                    resource = std::string(StringTools::lstrip(resource.substr(ref.size())));
                                }

                                // THE ROLE IS THE PARSER'S ANSWER (2026-10-29): a (role, component) it chose the
                                // resource's file for (IniParseData::rolesOfResource; a toggle's variants count),
                                // among the roles this fix binds on a component it draws -- this component's
                                // first, then any. One file may serve several components under different roles:
                                // Lynae9 draws one sweater through her jacket (5) and her props (6).
                                std::optional<std::string> own;
                                std::optional<std::string> role;
                                for (const IniParseData::ResourceRole& chosen : data.rolesOfResource(StringTools::toLower(resource))) {
                                    const std::optional<int> of = sourceComponentOf(chosen.modObj);
                                    if (!of.has_value() || present_.count(*of) == 0 || !bindsRole(*of, chosen.role)) {
                                        continue;
                                    }
                                    if (*of == ownSource) {
                                        own = chosen.role;
                                    } else if (!role.has_value()) {
                                        role = chosen.role;
                                    }
                                }
                                if (own.has_value()) {
                                    role = own;
                                }

                                // ...or the role the parser REFUSED the file for (2026-10-03): it was chosen for
                                // nothing precisely because it was refused, and a line left alone puts the refused
                                // file back (ChisaParfait3's flat `3_1.dds` across her whole body)
                                if (!role.has_value()) {
                                    role = data.refusedRoleOf(StringTools::toLower(resource));
                                }

                                // A TEXTURE PUT INTO ANOTHER KIND OF SLOT ON PURPOSE goes by the slot: to the
                                // target's register for the role the source reads at this one, its file unchanged
                                // -- or left where it is when the source's register has no role here
                                if (role.has_value() && slotRoles.has_value()) {
                                    const auto slotRole = slotRoles->find(key);
                                    if (slotRole == slotRoles->end() || roleKind(slotRole->second) != roleKind(*role)) {
                                        const std::optional<std::string> slotReg =
                                            slotRole != slotRoles->end() ? regOf(slotRole->second) : std::nullopt;
                                        if (slotReg.has_value() && !StringTools::equalsIgnoreCase(*slotReg, key)) {
                                            return std::optional<Kvp>(Kvp{*slotReg, value});
                                        }
                                        return std::nullopt;
                                    }
                                }

                                // LEFT ALONE rather than dropped when the role or its register is unknown: a
                                // register this fix has no opinion about -- `ps-t17 = ResourceClothFX` is one, and
                                // the first cut of this edit took it out of two Sanhua mods. ...UNLESS THE FIX'S LIST
                                // BINDS THAT REGISTER (2026-10-28): then it means a role of the TARGET's, and a file
                                // the scan cannot place there is art for another slot of the source's layout
                                // (ChisaParfait2's `ps-t2` texture on Chisa's diffuse register), so the line goes
                                // and the list's default stands.
                                const std::optional<std::string> reg = role.has_value() ? regOf(*role) : std::nullopt;
                                if (!reg.has_value()) {
                                    return listRegs.count(key) > 0 ? std::optional<std::optional<Kvp>>(std::optional<Kvp>())
                                                                   : std::nullopt;
                                }

                                // The mod's own resource (its texture edit, if any, is collected from the line --
                                // addTexEditCollects), except that A FILE REFUSED FOR THIS ROLE DOES NOT GO BACK:
                                // the line follows what the role resolved to -- the source's own, downloaded and
                                // edited -- or, where nothing stands in (`flatLeftToGame`), goes, leaving the
                                // game's own texture bound as the mod did on its own character. A rerouted
                                // component keeps the mod's own art (rerouteShaders_).
                                if (rerouteShaders_.count(component) == 0 && data.refusedRoleOf(resource) == role) {
                                    const std::string* resolved = resourceFor(*role, component);
                                    return resolved != nullptr ? std::optional<Kvp>(Kvp{*reg, *resolved}) : std::optional<Kvp>();
                                }
                                return std::optional<Kvp>(Kvp{*reg, resource});
                            };

                            CarriedLines lines;
                            const std::string prefix = StringTools::toLower(config_.texRegPrefix);
                            for (const std::string& section : ownSections()) {
                                for (const Row* row : ownRows(section)) {
                                    const std::string key = StringTools::toLower(std::string(StringTools::strip(row->key)));
                                    const std::string value(StringTools::strip(row->value));
                                    if (!StringTools::startsWith(key, prefix) || lines.count({key, value}) > 0) {
                                        continue;
                                    }
                                    if (auto fate = fateOf(key, value)) {
                                        lines.emplace(std::make_pair(key, value), std::move(*fate));
                                    }
                                }
                            }

                            auto carried = carriedTexRegs(config_.texRegPrefix, std::move(lines));
                            auto adapter = std::make_unique<RegPartEdit<>>(carried.get());
                            editsOf_[component].push_back(adapter.get());
                            carriedEdits_.push_back(std::move(carried));
                            regAdapters_.push_back(std::move(adapter));
                        }

                        // The mirrored twin, AFTER the component's own draw -- an empty
                        // predicate on `drawindexed` accepts any value, and `latest = false` puts
                        // the block at the earliest position that follows one. Its own add, because
                        // everything in `additions` goes BEFORE the draw.
                        if (mirroredSet().count(component) > 0) {
                            auto twin = mirrorTwin(
                                remapFixName(IniKeywords::Resource + "MirrorIndex" + std::to_string(component)),
                                remapFixName(IniKeywords::Resource + "MirrorVector"),
                                IniKeywords::WWMIIndexBuffer, IniKeywords::WWMIVectorBuffer, config_.vectorReg,
                                mirrorSpan(component).first);
                            auto adapter = std::make_unique<RegPartEdit<>>(twin.get());
                            editsOf_[component].push_back(adapter.get());
                            twinEdits_.push_back(std::move(twin));
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
                            // so the anchor is matched under either name.
                            const std::vector<std::string> sharedLists = listNames(config_.sharedResourcesList);
                            auto add = std::make_unique<RegSurroundedAdd<>>(
                                std::move(additions),
                                RegSurroundedAdd<>::RegMap{{IniKeywords::Run, [sharedLists](const std::string& value) {
                                    return std::find(sharedLists.begin(), sharedLists.end(), value) != sharedLists.end();
                                }}},
                                RegSurroundedAdd<>::RegMap{}, false);
                            auto adapter = std::make_unique<GraphPartEdit<>>(add.get());
                            editsOf_[component].push_back(adapter.get());
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
                    for (const auto& entry : rerouteFrom_) {
                        names.push_back(remapFixName(list + "Reroute" + std::to_string(entry.first)));
                    }

                    return names;
                }

                // Which draws of each reroute's source component go elsewhere: the ones the parser picked
                // for the reroute's split (WWMIParserConfig::drawSplits). A reroute that takes draws becomes a
                // present component of its own, sharing the source's sections and every texture it reads.
                void readReroutes() {
                    if (config_.drawReroutes.empty() || hasGivenUp()) {
                        return;
                    }

                    const IniParseData& data = parseData();
                    for (const WWMIFixerConfig::DrawReroute& reroute : config_.drawReroutes) {
                        if (present_.count(reroute.from) == 0 || config_.plan.count(reroute.as) == 0
                                || drawRanges_.count(reroute.from) == 0 || present_.count(reroute.as) > 0) {
                            continue;
                        }

                        const std::vector<IniParseData::DrawRange>& picked =
                            data.drawRanges(ModObj(reroute.split, config_.slotPrefix + std::to_string(reroute.from)));
                        const std::set<std::pair<long long, long long>> taken(picked.begin(), picked.end());
                        if (taken.empty()) {
                            continue;
                        }

                        // the reroute is a component of its own from here on
                        rerouteFrom_[reroute.as] = reroute.from;
                        reroutedDraws_[reroute.as] = taken;
                        present_[reroute.as] = present_.at(reroute.from);
                        graphOf_[reroute.as] = graphOf_.at(reroute.from);
                        drawParts_[reroute.as] = drawParts_[reroute.from];
                        for (const auto& range : taken) {
                            drawRanges_[reroute.as].push_back(range);
                        }

                        auto& own = drawRanges_[reroute.from];
                        own.erase(std::remove_if(own.begin(), own.end(),
                                                 [&taken](const std::pair<long long, long long>& r) { return taken.count(r) > 0; }),
                                  own.end());

                        const auto copyFor = [&reroute](auto& byRoleComponent) {
                            std::vector<std::pair<std::pair<std::string, int>, std::string>> add;
                            for (const auto& entry : byRoleComponent) {
                                if (entry.first.second == reroute.from) {
                                    add.push_back({{entry.first.first, reroute.as}, entry.second});
                                }
                            }
                            for (auto& entry : add) {
                                byRoleComponent.insert(std::move(entry));
                            }
                        };
                        copyFor(resourceOfSlotRole_);
                        copyFor(toggledGame_);
                        const auto late = lateBindRegs_.find(reroute.from);
                        if (late != lateBindRegs_.end()) {
                            lateBindRegs_[reroute.as] = late->second;
                        }

                        ctx_.log(std::to_string(taken.size()) + " of " + config_.slotPrefix + std::to_string(reroute.from)
                             + "'s draws are in the split " + reroute.split + ", and go through target slot "
                             + std::to_string(config_.plan.at(reroute.as).slot) + " instead");
                    }
                }

                // ---- WWMIFixerConfig::SourceComponent::rabbitFX ----
                static bool isRabbitFXRemoval(const WWMIFixerConfig::RegRemoval& removal) {
                    return RabbitFX::isOwnKVP(removal.reg, removal.valuePrefix);
                }

                // The routing flags RabbitFX's SetTextures sets on the SOURCE's own draw of each
                // component that keeps RabbitFX, from what that draw has BOUND at ps-t0..t3 -- not
                // from the maps the mod hands RabbitFX, which it only swaps in afterwards. Per register:
                // the mod's own `ps-tN` line (unless only a toggle sets it), else the mod's `this`
                // override of the game texture there, else the game texture itself (gameTags)
                void readRabbitFX() {
                    rabbitFXFlags_.clear();
                    rabbitFXPins_.clear();
                    rabbitFXPinAdapters_.clear();
                    if (hasGivenUp()) {
                        return;
                    }

                    const std::string folder = ctx_.getIniFile()->getFolder();


                    const auto tagOfResource = [this, &folder](const std::string& resource) -> std::optional<int> {
                        const std::string name = IniNamingTools::removeRefPrefix(resource);
                        const auto file = fileOfResource_.find(StringTools::toLower(name));
                        if (file == fileOfResource_.end()) {
                            return std::nullopt;
                        }
                        return RabbitFX::textureTag(FileService::absPathOfRelPath(file->second, folder));
                    };

                    for (const auto& [component, planned] : config_.plan) {
                        if (!planned.rabbitFX.has_value() || present_.count(component) == 0) {
                            continue;
                        }

                        const auto alias = rerouteFrom_.find(component);
                        const int source = alias != rerouteFrom_.end() ? alias->second : component;
                        if (!callsSetTextures(source)) {
                            continue;                           // nothing of RabbitFX's to keep or pin
                        }

                        const auto regs = config_.sourceTextures.registerRoles.find(source);
                        std::array<int, 4> tags{0, 0, 0, 0};
                        for (int k = 0; k < 4; ++k) {
                            const std::string reg = "ps-t" + std::to_string(k);
                            const auto game = planned.rabbitFX->gameTags.find(reg);
                            tags[static_cast<std::size_t>(k)] = game != planned.rabbitFX->gameTags.end() ? game->second : 0;
                            if (regs == config_.sourceTextures.registerRoles.end()) {
                                continue;
                            }

                            const auto role = regs->second.find(reg);
                            if (role == regs->second.end()) {
                                continue;
                            }

                            // the mod's own line at this register, in its base state
                            if (bindsRegister(source, reg) && toggledGame_.count({role->second, source}) == 0) {
                                const std::string* resource = nullptr;
                                const auto current = resourceOfSlotRole_.find({role->second, source});
                                if (current != resourceOfSlotRole_.end()) {
                                    resource = &current->second;
                                }
                                if (resource != nullptr) {
                                    const std::optional<int> tag = tagOfResource(*resource);
                                    if (tag.has_value()) {
                                        tags[static_cast<std::size_t>(k)] = *tag;
                                        continue;
                                    }
                                }
                            }

                            // ...else the game's texture there, or the mod's `this` in its place
                            const auto gameHash = config_.sourceTextures.fallbackTextures.find(role->second);
                            if (gameHash != config_.sourceTextures.fallbackTextures.end()) {
                                // the mod's `this` override of that texture, if any (IniParseData::thisOverrideOf)
                                const std::optional<std::string> over = parseData().thisOverrideOf(gameHash->second);
                                if (over.has_value()) {
                                    const std::optional<int> tag = tagOfResource(*over);
                                    if (tag.has_value()) {
                                        tags[static_cast<std::size_t>(k)] = *tag;
                                    }
                                }
                            }
                        }

                        rabbitFXFlags_[component] = RabbitFX::routingFlags(tags);
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

                // Whether the component's own sections bind `reg` themselves
                bool bindsRegister(int component, const std::string& reg) const {
                    const auto sections = present_.find(component);
                    return sections != present_.end()
                           && std::any_of(sections->second.begin(), sections->second.end(), [&](const std::string& name) {
                                  const auto& rows = ownRows(name);
                                  return std::any_of(rows.begin(), rows.end(), [&reg](const Row* row) {
                                      return IniNamingTools::sameKey(row->key, reg);
                                  });
                              });
                }

                // Whether the component's own sections run RabbitFX's SetTextures, directly or through
                // a command list they call
                bool callsSetTextures(int component) const {
                    const auto obj = graphOf_.find(component);
                    if (obj == graphOf_.end()) {
                        return false;
                    }

                    // the component's graph: its sections and every one they `run =`
                    const auto& rows = parseData().collected(IniParseRows::Objs);
                    return std::any_of(rows.begin(), rows.end(), [&obj](const Row& row) {
                        return row.graphId.modObj == obj->second && row.key == IniKeywords::Run
                               && StringTools::equalsIgnoreCase(StringTools::strip(row.value), IniKeywords::RabbitFXSetTexturesPath);
                    });
                }

                std::string rabbitFXPinList(int component) const {
                    return remapFixName(IniKeywords::CommandList + "RabbitFXPin" + std::to_string(component));
                }

                void rabbitFXPinSections() {
                    for (const auto& [component, flags] : rabbitFXFlags_) {
                        const int slot = config_.plan.at(component).slot;
                        addFixSection(SectionText(z3_, rabbitFXPinList(component))
                                   .open(passCondition(config_.slotPasses.at(static_cast<std::size_t>(slot))))
                                   .keys({{RabbitFX::FlagKeys[0], std::to_string(flags[0])}, {RabbitFX::FlagKeys[1], std::to_string(flags[1])},
                                          {RabbitFX::FlagKeys[2], std::to_string(flags[2])}, {RabbitFX::FlagKeys[3], std::to_string(flags[3])}})
                                   .close()
                                   .build());
                    }
                }

                // Every drawn component whose plan entry carries its own shader (a reroute's `as`
                // included, which readReroutes has just made present)
                void readShaders() {
                    rerouteShaders_.clear();
                    if (hasGivenUp()) {
                        return;
                    }

                    markLines_.clear();
                    for (const auto& [component, planned] : config_.plan) {
                        if (planned.shader.has_value() && present_.count(component) > 0) {
                            // Shader::yieldToRabbitFX
                            if (!shaderUsed(component)) {
                                ctx_.log("component " + std::to_string(component) + " hands its maps to RabbitFX, so it keeps "
                                         + toModName_ + "'s own shader rather than " + planned.shader->name);
                                continue;
                            }
                            rerouteShaders_[component] = &*planned.shader;
                        }
                        if (!planned.markLines.empty() && present_.count(component) > 0) {
                            markLines_[component] = &planned.markLines;
                        }
                    }
                }

                // Whether a component's plan entry draws it with its shader -- see Shader::yieldToRabbitFX
                bool shaderUsed(int component) const {
                    const auto planned = config_.plan.find(component);
                    if (planned == config_.plan.end() || !planned->second.shader.has_value()) {
                        return false;
                    }
                    return !(planned->second.shader->yieldToRabbitFX && usesRabbitFX(component));
                }

                // A component's bindings: its plan entry's, and its shader's while the shader draws it
                std::vector<WWMIFixerConfig::Binding> bindingsOf(int component) const {
                    const auto planned = config_.plan.find(component);
                    if (planned == config_.plan.end()) {
                        return {};
                    }
                    std::vector<WWMIFixerConfig::Binding> result = planned->second.bindings;
                    if (shaderUsed(component)) {
                        // a shader's binding takes the register's place in the plan's
                        for (const WWMIFixerConfig::Binding& extra : planned->second.shader->registers) {
                            result.erase(std::remove_if(result.begin(), result.end(),
                                                        [&extra](const WWMIFixerConfig::Binding& b) {
                                                            return StringTools::equalsIgnoreCase(b.reg, extra.reg);
                                                        }),
                                         result.end());
                            result.push_back(extra);
                        }
                    }
                    return result;
                }

                // Whether the mod's sections of a component name RabbitFX (its resources or its SetTextures)
                bool usesRabbitFX(int component) const {
                    return present_.count(component) > 0 && parseData().libraries(slotObj(component)).count(RabbitFX::Name) > 0;
                }

                // A reroute's copy keeps only its draws; its source's copy keeps everything else
                void buildRerouteFilters() {
                    for (const auto& [as, from] : rerouteFrom_) {
                        const std::set<std::pair<long long, long long>> taken = reroutedDraws_.at(as);
                        const auto rangeOf = [](const std::string& value) { return drawRangeOf(value); };

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
                        editsOf_[from].push_back(dropAdapter.get());
                        editsOf_[as].push_back(keepAdapter.get());
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
                        if (planned.offPasses.empty() || rerouteShaders_.count(component) > 0 || present_.count(component) == 0) {
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

                            const std::string list = remapFixName(IniKeywords::CommandList + "DrawAfter" + std::to_string(first));
                            afterDispatch_[first] = list;
                            afterEdits_[first] = runBeforeRuns(
                                std::vector<std::string>{config_.cleanupResourcesList, remapFixName(config_.cleanupResourcesList)}, list,
                                afterLatch(first));
                            afterAdapters_[first] = std::make_unique<RegPartEdit<>>(afterEdits_[first].get());
                        }
                    }
                }

                // ---- WWMIFixerConfig::SourceComponent::markLines ----
                std::string markSection(int component, const std::pair<long long, long long>& range,
                                        const std::string& kind = IniKeywords::CommandList) const {
                    const std::set<std::pair<long long, long long>> draws = shaderDraws(component);
                    const auto at = static_cast<std::size_t>(std::distance(draws.begin(), draws.find(range)));
                    return remapFixName(kind + "Mark" + std::to_string(component) + "Draw" + std::to_string(at));
                }

                // The source's own game texture of a role, as a component binds it: edited for the
                // target's shader, or the download itself for a component drawn with its own shader
                std::string gameResourceFor(const std::string& role, int component) const {
                    if (rerouteShaders_.count(component) > 0) {
                        const auto raw = gameFallbacks_.find(role);
                        if (raw != gameFallbacks_.end()) {
                            return raw->second.resource;
                        }
                    }

                    return gameResourceOfRole_.at(role);
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
                    const auto taken = reroutedDraws_.find(component);
                    if (taken != reroutedDraws_.end()) {
                        return taken->second;
                    }

                    std::set<std::pair<long long, long long>> draws;
                    const auto ranges = drawRanges_.find(component);
                    if (ranges != drawRanges_.end()) {
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

                    if (present_.count(shader.after->component) > 0) {
                        result.emplace_back(shader.after->component, shader.after->draws);
                    }
                    for (const auto& wait : shader.after->also) {
                        if (present_.count(wait.first) > 0) {
                            result.push_back(wait);
                        }
                    }
                    return result;
                }

                // Whether a shader drawn for this mod binds a capture off the slot (a `stash <slot> ...` line)
                bool stashRead(int slot) const {
                    const std::string wanted = "stash " + std::to_string(slot) + " ";
                    for (const auto& [as, shader] : rerouteShaders_) {
                        (void)as;
                        for (const auto& line : shader->lines) {
                            if (StringTools::startsWith(std::string(StringTools::strip(line.second)), wanted)) {
                                return true;
                            }
                        }
                    }
                    return false;
                }

                std::string stashResource(int slot, const std::string& reg) const {
                    return remapFixName(IniKeywords::Resource + "Stash" + std::to_string(slot) + IniNamingTools::getRegTag(reg));
                }

                // A Shader line `X = stash <slot> <reg>` binds WWMIFixerConfig::stashRegs' capture
                std::pair<std::string, std::string> stashLine(const std::pair<std::string, std::string>& line) const {
                    const std::string value(StringTools::strip(line.second));
                    std::istringstream words(value);
                    std::string word;
                    std::string reg;
                    int slot = -1;
                    if (!(words >> word) || word != "stash" || !(words >> slot) || !(words >> reg)) {
                        return line;
                    }
                    return {line.first, IniKeywords::Ref + " " + stashResource(slot, reg)};
                }

                // The role a Shader line `X = role <name>` names, if it is one
                static std::optional<std::string> lineRole(const std::pair<std::string, std::string>& line) {
                    std::istringstream words{std::string(StringTools::strip(line.second))};
                    std::string word;
                    std::string role;
                    if (!(words >> word) || word != "role" || !(words >> role)) {
                        return std::nullopt;
                    }
                    return role;
                }

                // Every role the shader drawing a component names in `role <name>` lines
                std::vector<std::string> shaderLineRoles(int component) const {
                    std::vector<std::string> roles;
                    const auto planned = config_.plan.find(component);
                    if (planned == config_.plan.end() || !planned->second.shader.has_value()) {
                        return roles;
                    }
                    const WWMIFixerConfig::Shader& shader = *planned->second.shader;
                    auto collect = [&roles](const std::vector<std::pair<std::string, std::string>>& lines) {
                        for (const auto& line : lines) {
                            if (std::optional<std::string> role = lineRole(line)) {
                                roles.push_back(*role);
                            }
                        }
                    };
                    collect(shader.lines);
                    for (const auto& pass : shader.passes) {
                        collect(pass);
                    }
                    return roles;
                }

                // A Shader line as it is written: a `stash` capture or a `role` resolved to its resource
                // (a role with no resource -- no file and no download -- keeps the register as the slot has it)
                std::optional<std::pair<std::string, std::string>> shaderLine(const std::pair<std::string, std::string>& line) const {
                    if (std::optional<std::string> role = lineRole(line)) {
                        const std::string* resource = sharedResourceFor(*role);
                        if (resource == nullptr) {
                            return std::nullopt;
                        }
                        return std::make_pair(line.first, *resource);
                    }
                    return stashLine(line);
                }

                std::string afterLatch(int component) const {
                    return "$" + remapFixName("DrawAfter" + std::to_string(component) + "Latch");
                }

                std::string rerouteShaderSection(int as, const std::pair<long long, long long>& range,
                                                 const std::string& kind = IniKeywords::CommandList,
                                                 const std::string& suffix = "") const {
                    const std::set<std::pair<long long, long long>> draws = shaderDraws(as);
                    const auto at = static_cast<std::size_t>(std::distance(draws.begin(), draws.find(range)));
                    return remapFixName(kind + rerouteShaders_.at(as)->name + std::to_string(as) + "Draw" + std::to_string(at) + suffix);
                }

                std::string rerouteShaderFile(const WWMIFixerConfig::Shader& shader) const {
                    return modFile(meshFolder_, toModName_ + IniKeywords::Remap + shader.name + ".hlsl");
                }

                // A pass's own shader (Shader::passPs), beside the shader's
                std::string reroutePassShaderFile(const WWMIFixerConfig::Shader& shader, std::size_t pass) const {
                    return modFile(meshFolder_, toModName_ + IniKeywords::Remap + shader.name + "Pass" + std::to_string(pass) + ".hlsl");
                }

                std::string rerouteBufferResource(const WWMIFixerConfig::Shader& shader, const std::string& reg) const {
                    return remapFixName(IniKeywords::Resource + shader.name + IniNamingTools::getRegTag(reg));
                }

                std::string rerouteBufferFile(const WWMIFixerConfig::Shader& shader, const std::string& reg) const {
                    return modFile(meshFolder_, toModName_ + IniKeywords::Remap + shader.name + IniNamingTools::getRegTag(reg) + ".buf");
                }

                std::string shaderBackup(const std::string& reg) const {
                    return remapFixName(IniKeywords::Resource + std::string("ShaderBackup") + IniNamingTools::getRegTag(reg));
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
                        for (const auto& range : shaderDraws(as)) {
                            const std::string draw = std::to_string(range.first) + ", " + std::to_string(range.second) + ", 0";
                            const std::string custom = rerouteShaderSection(as, range, "CustomShader");
                            const Kvp fallback = offDraw(as, range);
                            const std::size_t passCount = std::max<std::size_t>(1, shader->passes.size());
                            const auto passName = [&](std::size_t p) {
                                return p == 0 ? custom : custom + "Pass" + std::to_string(p);
                            };

                            std::vector<Kvp> runs;
                            std::vector<Kvp> assigned;
                            for (std::size_t p = 0; p < passCount; ++p) {
                                runs.emplace_back(IniKeywords::Run, passName(p));
                                if (!shader->passes.empty()) {
                                    assigned.insert(assigned.end(), shader->passes[p].begin(), shader->passes[p].end());
                                }
                            }
                            for (const auto& entry : shader->constantBuffers) {
                                assigned.emplace_back(entry.first, "");
                            }
                            for (const auto& line : shader->lines) {
                                assigned.push_back(line);
                            }
                            runs = restoringRuns(runs, assigned);

                            SectionText list(z3_, rerouteShaderSection(as, range));
                            const std::vector<std::pair<int, int>> waits = awaited(*shader);
                            if (waits.empty()) {
                                if (shader->onlyWith.empty()) {
                                    addFixSection(list.open(onPlan).keys(runs).orElse().key(fallback.first, fallback.second).close().build());
                                } else {
                                    addFixSection(list.open(shader->onlyWith + " !== null")
                                                   .open(onPlan).keys(runs).orElse().key(fallback.first, fallback.second).close()
                                               .orElse().key(fallback.first, fallback.second)
                                               .close()
                                               .build());
                                }
                            } else {
                                // Shader::after -- in each pass (told apart by the target resource) every
                                // awaited component's game draw counts once; the draw is made by whichever
                                // comes last, the deferred draw itself or the last awaited one
                                const WWMIFixerConfig::Shader::After& after = *shader->after;
                                const std::string target = after.target;
                                int total = 0;
                                for (const auto& wait : waits) {
                                    total += wait.second;
                                }
                                const std::string frame = "$" + rerouteShaderSection(as, range, "", "Frame");
                                const std::string count = "$" + rerouteShaderSection(as, range, "", "Count");
                                const std::string seen = rerouteShaderSection(as, range, IniKeywords::Resource, "Seen");
                                const std::string pending = rerouteShaderSection(as, range, IniKeywords::Resource, "Pending");
                                const auto carried = [&](const std::string& reg) {
                                    return rerouteShaderSection(as, range, IniKeywords::Resource, "Carried" + IniNamingTools::getRegTag(reg));
                                };
                                const std::vector<Kvp> reset{{frame, "time"}, {seen, "null"}, {pending, "null"}, {count, "0"}};
                                const std::string ready = seen + " === " + target + " && " + count + " >= " + std::to_string(total);

                                std::vector<Kvp> hold{{pending, IniKeywords::Ref + " " + target}};
                                std::vector<Kvp> release;
                                for (const std::string& reg : after.carried) {
                                    hold.emplace_back(carried(reg), IniKeywords::Ref + " " + reg);
                                    release.emplace_back(reg, IniKeywords::Ref + " " + carried(reg));
                                }
                                for (const Kvp& run : runs) {
                                    release.push_back(run);
                                }
                                release.emplace_back(pending, "null");

                                list.open(frame + " != time").keys(reset).close()
                                    .open(target + " === null");
                                if (shader->onlyWith.empty()) {
                                    list.open(onPlan).keys(runs).orElse().key(fallback.first, fallback.second).close();
                                } else {
                                    list.key(fallback.first, fallback.second);
                                }
                                addFixSection(list.orElse()
                                               .open(onPlan)
                                                   .open(ready).keys(runs)
                                                   .orElse().keys(hold)
                                                   .close()
                                               .orElse().key(fallback.first, fallback.second)
                                               .close()
                                           .close()
                                           .build());

                                // what each awaited component's game draws run (through its DrawAfter list)
                                addFixSection(SectionText(z3_, rerouteShaderSection(as, range, IniKeywords::CommandList, "After"))
                                           .open(frame + " != time").keys(reset).close()
                                           .open(target + " !== null")
                                               .open(seen + " !== " + target).key(seen, IniKeywords::Ref + " " + target).key(count, "0").close()
                                               .key(count, count + " + 1")
                                               .open(pending + " === " + target + " && " + count + " >= " + std::to_string(total)).keys(release).close()
                                           .close()
                                           .build());

                                afterConstants_.emplace_back(IniKeywords::Global + " " + frame, "-1");
                                afterConstants_.emplace_back(IniKeywords::Global + " " + count, "0");
                                for (const std::string& name : {seen, pending}) {
                                    addFixSection(SectionText(z3_, name).build());
                                }
                                for (const std::string& reg : after.carried) {
                                    addFixSection(SectionText(z3_, carried(reg)).build());
                                }
                            }

                            for (std::size_t p = 0; p < passCount; ++p) {
                                const bool ownPs = p < shader->passPs.size() && !shader->passPs[p].empty();
                                std::vector<Kvp> lines{{"ps", ownPs ? reroutePassShaderFile(*shader, p) : rerouteShaderFile(*shader)}};
                                for (const auto& entry : shader->constantBuffers) {
                                    lines.emplace_back(entry.first, rerouteBufferResource(*shader, entry.first));
                                }
                                for (const auto& line : shader->lines) {
                                    if (auto written = shaderLine(line)) {
                                        lines.push_back(*written);
                                    }
                                }
                                if (!shader->passes.empty()) {
                                    for (const auto& line : shader->passes[p]) {
                                        if (auto written = shaderLine(line)) {
                                            lines.push_back(*written);
                                        }
                                    }
                                }
                                lines.emplace_back(IniKeywords::DrawIndexed, draw);
                                addShaderSection(passName(p), lines);
                            }
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
                                {{IniKeywords::Type, IniKeywords::Buffer}, {IniKeywords::Format, "DXGI_FORMAT_R32G32B32A32_FLOAT"}},
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
                        addFixSection(SectionText(z3_, dispatch).open(latch + " == 0").key(latch, "1").keys(afterRuns).close().build());
                    }

                    if (!afterConstants_.empty()) {
                        // a second [Constants] in a file is merged into the first; inside the fix block
                        // an undo removes it with the block
                        addFixSection(SectionText(z3_, IniKeywords::Constants).keys(afterConstants_).build());
                        afterConstants_.clear();
                    }

                    for (const auto& [component, lines] : markLines_) {
                        const std::string onPlan = passCondition(config_.slotPasses.at(static_cast<std::size_t>(config_.plan.at(component).slot)));
                        for (const auto& range : shaderDraws(component)) {
                            const std::string draw = std::to_string(range.first) + ", " + std::to_string(range.second) + ", 0";
                            const std::string custom = markSection(component, range, "CustomShader");
                            addFixSection(SectionText(z3_, markSection(component, range))
                                       .open(onPlan)
                                       .keys(restoringRuns({{IniKeywords::Run, custom}}, *lines))
                                       .close()
                                       .build());

                            std::vector<Kvp> keys{{"ps", "null"}};
                            for (const auto& line : *lines) {
                                keys.push_back(line);
                            }
                            keys.emplace_back(IniKeywords::DrawIndexed, draw);
                            addShaderSection(custom, keys);
                        }
                    }

                    for (const std::string& reg : shaderBackups_.registers()) {
                        addFixSection(shaderBackups_.section(reg));
                    }

                    for (const auto& [name, gated] : offLists_) {
                        addFixSection(SectionText(z3_, name).open(gated.first).key(IniKeywords::DrawIndexed, gated.second).close().build());
                    }
                }

                void buildSlotRemap() {
                    // Members rather than locals: buildPerGroupEdits below reads both, and the two
                    // phases were one function when they shared them by scope.
                    hashOnlyObjs_.clear();
                    if (auto* parser = dynamic_cast<GIMIParser<>*>(this->getParser())) {
                        for (const ModObj& obj : parser->modObjs()) {
                            if (!StringTools::startsWith(obj.second, config_.slotPrefix) && obj.first != IniGraphModObjKeywords::Texture
                                    && !isCopiedObj(obj)) {
                                hashOnlyObjs_.push_back(obj);
                            }
                        }
                    }

                    hiddenObjs_ = std::unordered_set<std::string>(config_.hiddenObjs.begin(), config_.hiddenObjs.end());
                    const std::vector<ModObj>& hashOnlyObjs = hashOnlyObjs_;
                    const std::unordered_set<std::string>& hidden = hiddenObjs_;

                    // The remap: every source slot section onto its target slot's object, named
                    // after the TARGET so a target never collides with a source still to be moved;
                    // two claimants of one slot collide with each other, which is what makes the
                    // further groups (GraphGroupRemap's own rule). Every section keeps its own name
                    // plus the fix suffix, as the prototype's output has it.
                    const GraphGroupRemap<>::RenameFunc rename = [this](const std::string& name) { return remapFixName(name); };
                    GraphGroupRemap<>::RemapList remap;
                    for (const auto& entry : present_) {
                        if (rerouteFrom_.count(entry.first) > 0) {
                            continue;                       // a target of its source's graph, below
                        }

                        std::vector<GraphGroupRemap<>::RemapTarget> targets;
                        auto planned = config_.plan.find(entry.first);
                        if (planned != config_.plan.end()) {
                            const ModObj target = targetObjOf(entry.first);
                            targets.emplace_back(GraphId(0, target.first, target.second), rename);
                        }

                        // ...and each reroute of it: the same sections again under their own names,
                        // so the two copies do not collide in the mod's file
                        for (const auto& [as, from] : rerouteFrom_) {
                            if (from != entry.first) {
                                continue;
                            }

                            const ModObj target = targetObjOf(as);
                            const std::string tag = "Reroute" + std::to_string(as);
                            targets.emplace_back(GraphId(0, target.first, target.second),
                                                 [this, tag](const std::string& name) { return remapFixName(name + tag); });
                        }

                        const ModObj src = slotObj(entry.first);
                        remap.emplace_back(GraphId(0, src.first, src.second), std::move(targets));
                    }

                    // A component whose section draws nothing (see readMod) goes nowhere, as one the
                    // plan does not name does: left out of the remap, its graph would be rendered
                    // into the fix block under the mod's own section names
                    for (int component : silent_) {
                        const ModObj src = slotObj(component);
                        remap.emplace_back(GraphId(0, src.first, src.second), std::vector<GraphGroupRemap<>::RemapTarget>{});
                    }

                    for (const ModObj& obj : hashOnlyObjs) {
                        std::vector<GraphGroupRemap<>::RemapTarget> targets;
                        if (hidden.count(obj.second) > 0) {
                            this->hiddenModObjs.insert(obj);
                        } else {
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
                            const bool shapeKeys = obj.second == "shapekeyOffsets" || obj.second == "shapekeyScale";
                            const std::size_t copies = (targetPast256_ && config_.copiesShareSkeleton) || shapeKeys
                                ? std::size_t(1) : groups_.size();
                            for (std::size_t g = 0; g < copies; ++g) {
                                targets.emplace_back(GraphId(0, obj.first, obj.second), rename);
                            }
                        }

                        remap.emplace_back(GraphId(0, obj.first, obj.second), std::move(targets));
                    }

                    slotRemap_ = std::make_unique<GraphGroupRemap<>>(std::move(remap));

                }

                // The edits, per group, keyed by the target objects the remap created.
                void buildPerGroupEdits() {
                    const std::vector<ModObj>& hashOnlyObjs = hashOnlyObjs_;
                    const std::unordered_set<std::string>& hidden = hiddenObjs_;
                    std::vector<ObjGroupEdit::IniEdits> perGroup(groups_.size());
                    for (std::size_t g = 0; g < groups_.size(); ++g) {
                        for (int component : groups_[g]) {
                            const int slot = config_.plan.at(component).slot;
                            const ModObj obj = targetObjOf(component);
                            std::vector<PartEdit*> edits = editsOf_[component];
                            const bool keepsRabbitFX = rabbitFXFlags_.count(component) > 0;
                            if (keepsRabbitFX) {
                                if (removeKeepRabbitFXAdapter_ != nullptr) {
                                    edits.push_back(removeKeepRabbitFXAdapter_.get());
                                }
                                edits.push_back(rabbitFXPinAdapters_.at(component).get());
                            } else if (removeAdapter_ != nullptr) {
                                edits.push_back(removeAdapter_.get());
                            }

                            edits.push_back(newValsOf_.at(slot));
                            edits.push_back(assetAdapter_.get());
                            if (dispatchAdapter_ != nullptr) {
                                edits.push_back(dispatchAdapter_.get());
                            }

                            if (shapeKeyAdapter_ != nullptr) {
                                edits.push_back(shapeKeyAdapter_.get());
                            }

                            if (afterAdapters_.count(component) > 0) {
                                edits.push_back(afterAdapters_.at(component).get());
                            }

                            if (markLines_.count(component) > 0 && markAdapter_ != nullptr) {
                                edits.push_back(markAdapter_.get());            // before the reroute: it reads the drawindexed
                            }

                            if (rerouteShaders_.count(component) > 0 && rerouteShaderAdapter_ != nullptr) {
                                edits.push_back(rerouteShaderAdapter_.get());   // last: every anchor above reads the draw
                            }

                            if (offAdapters_.count(component) > 0) {
                                edits.push_back(offAdapters_.at(component).get());   // SourceComponent::offPasses
                            }

                            perGroup[g].edits[obj] = std::move(edits);
                            perGroup[g].trackKeys[obj] = false;
                        }

                        for (const ModObj& obj : hashOnlyObjs) {
                            if (hidden.count(obj.second) > 0) {
                                continue;
                            }

                            perGroup[g].edits[obj] = {assetAdapter_.get()};
                            if (dispatchAdapter_ != nullptr) {
                                perGroup[g].edits[obj].push_back(dispatchAdapter_.get());
                            }

                            if (shapeKeyAdapter_ != nullptr) {
                                perGroup[g].edits[obj].push_back(shapeKeyAdapter_.get());
                            }
                            perGroup[g].trackKeys[obj] = false;
                        }
                    }

                    mainEdits_ = std::make_unique<ObjGroupEdit>(std::move(perGroup), false);

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
                            const std::string vgRel = FileService::iniPathToRel(vertexVGFile_.value_or(""));
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
                            if (legacy_) {
                                legacy = WWMIBlend::LegacyLift{FileService::absPathOfRelPath(indexFile_, ctx_.getIniFile()->getFolder()),
                                                               drawRanges_, config_.sourceVgMaps};
                            }

                            lift = [vgPath, posPath, out, legacy](RemapBlendResource& resource) {
                                return orBail(resource, [&] {
                                    WWMIBlend::writeRemap(resource.srcPath, resource.fixedPath, vgPath, posPath,
                                                          resource.vgRemap, out, legacy);
                                });
                            };
                        } else if (vertexVGFile_.has_value()) {
                            const std::string vgRel = FileService::iniPathToRel(*vertexVGFile_);
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
                        } else if (legacy_) {
                            const std::string indexPath = FileService::absPathOfRelPath(indexFile_, ctx_.getIniFile()->getFolder());
                            const std::string positionPath =
                            positionBufPath();
                            const WWMIBlend::LegacyLift legacy{indexPath, drawRanges_, config_.sourceVgMaps};
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
                            collect->srcRegs[GraphId(g, obj.first, obj.second)] = config_.blendReg;

                            // A MOD OF A CHARACTER PAST 256 BONES BINDS THE BLEND REGISTER TWICE:
                            // `vb4 = ResourceBlendBuffer` when no blend remap is active, and
                            // `vb4 = ref ResourceBlendBufferOverride` when one is. Only the first
                            // names a FILE -- the second is a buffer WWMI's BlendRemapper fills at
                            // load -- so take that one and leave the other. Without this the run
                            // dies looking for a section called `Resourceref Resource...` and the
                            // whole mod is skipped.
                            //
                            // A mod that binds it once, plainly, is unaffected: the predicate passes.
                            collect->resPredicates[GraphId(g, obj.first, obj.second)] = Collector::skipsKeywords();
                        }

                        collect->resEdits = {{BlendResGroupMember, replace.get()}};
                        blendReplaces_.push_back(std::move(replace));
                        blendCollects_.push_back(std::move(collect));
                    }
                }

                // ---- what the WRITTEN text must say ----


                // Every role bound only by a slot's other passes or by a shared mesh -- the ones
                // config_.plan does not name
                std::vector<std::string> passOnlyRoles() const {
                    std::vector<std::string> roles;
                    for (const auto& [slot, byPass] : config_.extraPassRegs) {
                        bool drawn = false;
                        for (const auto& planned : config_.plan) {
                            if (planned.second.slot == slot && present_.count(planned.first) > 0) {
                                drawn = true;
                                break;
                            }
                        }

                        if (!drawn) {
                            continue;
                        }

                        for (const auto& [pass, binds] : byPass) {
                            (void)pass;
                            for (const WWMIFixerConfig::Binding& binding : binds) {
                                roles.push_back(binding.role);
                            }
                        }
                    }

                    for (const auto& [mesh, byPass] : config_.sharedMeshes) {
                        (void)mesh;
                        for (const auto& [pass, binds] : byPass) {
                            (void)pass;
                            for (const WWMIFixerConfig::Binding& binding : binds) {
                                roles.push_back(binding.role);
                            }
                        }
                    }

                    std::vector<std::string> out;
                    for (const std::string& role : roles) {
                        ListTools::pushDistinct(out, role);
                    }

                    return out;
                }

                // The line binding one role at one register inside a fix-written texture list.
                //
                // `<reg> = <resource>` normally. For a role the mod binds behind its own toggle, a
                // `run =` into a copy of the mod's [TextureOverrideTexture*] instead, with 3dmigoto's
                // matching keys dropped and `this` renamed to the register -- so the toggle comes
                // across untouched rather than collapsing to its first variant.
                /**
                 * @brief `<reg> = null` -- bind NOTHING, rather than letting the game's own
                 *        texture serve the register (see WWMIFixerConfig::Binding::role)
                 */

                Kvp nullLine(const std::string& reg) {
                    return {reg, std::string(IniKeywords::Null)};
                }

                // `raw`: the list keeps the mod's own resources, edited or not -- see rerouteShaders_
                Kvp bindLine(const std::string& role, const std::string& reg, const std::string& resource, bool raw = false) {
                    const Kvp direct{reg, resource};

                    const std::string& base = resource;
                    const IniParseData::TextureToggle* toggle = parseData().thisToggleOf(StringTools::toLower(base));
                    if (toggle == nullptr) {
                        return direct;
                    }

                    const std::string cacheRole = raw ? role + "\x01" : role;
                    const auto cached = roleLists_.find({cacheRole, reg});
                    if (cached != roleLists_.end()) {
                        return {std::string(IniKeywords::Run), cached->second};
                    }

                    if (ownRows(toggle->section).empty()) {
                        return direct;
                    }

                    if (toggle->graph.first.empty()) {
                        return direct;                      // a section the parser placed in no graph
                    }

                    const std::string name =
                        remapFixName(IniKeywords::CommandList + source_.name + TextTools::capitalize(role) + IniNamingTools::getRegTag(reg)
                                + (raw ? "Raw" : ""));

                    // 1. A BRANCH NAMING A RESOURCE THE MOD NEVER DECLARES IS THE AUTHOR'S TYPO, AND
                    //    COPYING IT CAN ONLY MAKE THINGS WORSE (2026-09-28). SanhuaExorcist4's
                    //    `_injured` override says `ResourceTexture7.1` / `.2` where it declares
                    //    `ResourceTexture7a` / `7b`; this list runs AFTER the mod's own component
                    //    section, whose own $yifu toggle binds all three correctly, so a dead branch
                    //    replaces a good binding with nothing. Dropped, the mod's own stands for
                    //    those values and the fix binds the one branch that resolves.
                    //    ...and a resource the FIX declares is not dead either. A value may already
                    //    have been swapped for an edited resource of ours, which is in none of the
                    //    MOD's maps -- so an earlier form of this test dropped every branch of a
                    //    toggled role that carries an edit and the whole list collapsed to the
                    //    single direct binding of the resolved variant. Chisa7 toggles its hair
                    //    normal between `Components-1 t=d8ed7611.dds` and `... A.dds`; on a clean fix
                    //    the second repack was written, declared and bound by NOTHING, so at
                    //    `$Char != 0` the hair took variant 0's normal map. (The lists now bind the
                    //    mod's own names and the edits are collected from them -- addTexEditCollects.)
                    const RegRemove<>::RemoveKeyCheck isDead =
                        [this](long long, const std::string& val) {
                            const std::string bound = StringTools::toLower(val);
                            return fileOfResource_.count(bound) == 0 && ownRows(val).empty();
                        };

                    // The spellings of `this` the section writes -- `THIS = ResourceX` is legal 3dmigoto and
                    // a regEdit matches a key exactly -- and whether any binding outlives the dead ones
                    std::vector<std::string> thisKeys;
                    std::size_t bindings = 0;
                    for (const Row* row : ownRows(toggle->section)) {
                        if (StringTools::toLower(row->key) == IniKeywords::This) {
                            ListTools::pushDistinct(thisKeys, row->key);
                            bindings += isDead(0, row->value) ? 0 : 1;
                        }
                    }

                    if (bindings == 0) {
                        return direct;
                    }

                    if (raw) {
                        rawLists_.insert(name);
                    }
                    listsOfRole_[{role, reg}].insert(name);

                    // built by buildToggleLists, as edits over a copy of the parser's graph of the section
                    ToggleList list{name, toggle->graph, toggle->section, reg, std::move(thisKeys), isDead, std::nullopt};
                    if (toggle->single) {
                        // a one-variant toggle: off, the original draws with the game's texture
                        const auto game = gameResourceOfRole_.find(role);
                        if (game != gameResourceOfRole_.end()) {
                            list.offBinding = game->second;
                        }
                    }

                    toggleLists_.push_back(std::move(list));
                    roleLists_.emplace(std::pair<std::string, std::string>{cacheRole, reg}, name);
                    return {std::string(IniKeywords::Run), name};
                }

                // THE TOGGLE LISTS ARE THE MOD'S OWN TEXTURE OVERRIDE, AS EDITS (2026-10-28). For each
                // one bindLine queued:
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

                    for (const ToggleList& list : toggleLists_) {
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
                        for (const std::string& key : list.thisKeys) {
                            drops.emplace_back(key, list.isDead);
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
                    if (!toggleLists_.empty()) {
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
                    for (const auto& entry : present_) {
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
                    return passTags_.condition(config_.passVertexShaders.empty() ? "ps" : "vs", taggedFor(passes));
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
                    return passTags_.condition(config_.passVertexShaders.empty() ? "ps" : "vs", taggedFor(passes), true);
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

                    const std::string name = remapFixName(IniKeywords::CommandList + "OffPass" + std::to_string(component) + "Draw"
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
                        std::size_t i = 0;
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

                        for (const auto& passes : allPasses) {
                            for (const std::string& p : passes) {

                                for (const std::string& tagged : taggedFor(p)) {
                                    if (passTags_.filterIndexOf(tagged).has_value()) {
                                        continue;
                                    }

                                    auto given = config_.filterIndices.find(tagged);
                                    if (given != config_.filterIndices.end()) {
                                        passTags_.add(tagged, given->second);
                                    } else {
                                        passTags_.add(tagged, NumTools::formatDouble(config_.filterBase + config_.filterStep * static_cast<double>(i)));
                                        ++i;
                                    }
                                }
                            }
                        }
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
                    skeleton.boneDataFilter = config_.boneDataFilter;
                    skeleton.mergedSkeletonSlots = config_.mergedSkeletonSlots;
                    skeleton.remapped = targetPast256_;
                    skeleton.latched = hostResetsDetected_;
                    skeleton.currentPoseInCb3Only = config_.currentPoseInCb3Only;
                    skeleton.bindPrevPoseAlways = config_.bindPrevPoseAlways;
                    skeleton.blendRemapBones = blendRemapBones_;
                    skeleton.blendRemapForward = remapFixName(IniKeywords::Resource + std::string("BlendRemapForwardBuffer"));
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
                    blendRemap.blendReg = config_.blendReg;
                    return blendRemap;
                }

                // The skeleton's sections (WWMISections::skeletonSections), and the blend remap's after them
                void legacySkeletonSections() {
                    for (auto& section : WWMISections::skeletonSections(z3_, runtimeSkeleton())) {
                        addFixSection(std::move(section));
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
                        addFixSection(std::move(section));
                    }
                }

                // ---- the fix's own sections ----

                void buildAppended() {
                    for (const auto& entry : declared_) {
                        addFixSection(SectionText(z3_, declaredName_[entry.first])
                                   .key(IniKeywords::Filename, entry.second)
                                   .build());
                    }

                    // its OWN name -- see Fallback::resource. resourceOfRole_ may by now be the
                    // resource of a texture EDIT of this role, and writing the download's section
                    // under that name declares it twice, raw file and edited file.
                    for (const auto& entry : fallbacks_) {
                        addFixDownload(entry.second);
                    }

                    // ...and the same for a role the mod DOES serve, downloaded for the draws that
                    // happen before its section binds anything -- see lateBindRegs_.
                    for (const auto& entry : gameFallbacks_) {
                        addFixDownload(entry.second);
                    }

                    // the zero shape-key stream: a buffer of zeros, one line a vertex
                    if (config_.zeroShapeKeyStream && meshVertexCount_ > 0) {
                        const std::size_t size = static_cast<std::size_t>(meshVertexCount_) * static_cast<std::size_t>(config_.shapeKeyStride);
                        addFixFile(remapFixName(IniKeywords::Resource + ShapeKeyZero),
                                   {{IniKeywords::Type, IniKeywords::Buffer},
                                    {IniKeywords::Format, "DXGI_FORMAT_R32G32B32_FLOAT"},
                                    {IniKeywords::Stride, std::to_string(config_.shapeKeyStride)}},
                                   zeroStreamFile(), [size]() { return ByteVec(size, 0); }, {config_.shapeKeyStreamReg});
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
                                       [bytesOf, which]() { return bytesOf(which); }, {"cs-t33", "cs-t0", "cs-t1"});
                        }
                    }

                    // An empty section, like the mod's own [ResourceBypassVB0]: it is a handle for
                    // `= ref <reg>` to park a binding in, never a file.
                    for (const auto& [reg, bypass] : bypasses_) {
                        (void)reg;
                        addFixSection(SectionText(z3_, bypass).build());
                    }

                    if (!bypasses_.empty()) {
                        SectionText restore(z3_, restoreListName());
                        for (const auto& [reg, bypass] : bypasses_) {
                            restore.key(reg, IniKeywords::Ref + " " + bypass);
                        }

                        addFixSection(restore.build());
                    }

                    // the mirrored twin's buffers -- see WWMIFixerConfig::mirroredComponents
                    const std::set<int> mirrored = mirroredSet();
                    if (mirrored.empty() && !config_.mirroredComponents.empty()) {
                        ctx_.log("no component could carry a mirrored twin -- a mod needs one draw"
                             " range per mirrored component and an [" + IniKeywords::WWMIIndexBuffer
                             + "] and [" + IniKeywords::WWMIVectorBuffer + "] of its own");
                    }
                    if (!mirrored.empty()) {
                        addFixFile(remapFixName(IniKeywords::Resource + "MirrorVector"),
                                   {{IniKeywords::Type, IniKeywords::Buffer},
                                    {IniKeywords::Format, "DXGI_FORMAT_R8G8B8A8_SNORM"},
                                    {IniKeywords::Stride, "8"}},
                                   mirrorVectorFile(), [this]() { return mirrorVectorBytes(); }, {config_.vectorReg});
                    }

                    for (int component : mirrored) {
                        addFixFile(remapFixName(IniKeywords::Resource + "MirrorIndex" + std::to_string(component)),
                                   {{IniKeywords::Type, IniKeywords::Buffer},
                                    {IniKeywords::Format, "DXGI_FORMAT_R32_UINT"},
                                    {IniKeywords::Stride, "12"}},
                                   mirrorIndexFile(component), [this, component]() { return mirrorIndexBytes(component); },
                                   {IniKeywords::Ib});
                    }


                    for (std::size_t slot = 0; slot < target_.slots.size(); ++slot) {
                        if (drawnSlots_.count(static_cast<int>(slot)) > 0) {
                            continue;
                        }

                        const Slot& s = target_.slots[slot];
                        auto label = config_.targetLabels.find(static_cast<int>(slot));
                        const std::string labelText = label == config_.targetLabels.end() ? std::to_string(slot) : label->second;
                        WWMISections::Hide hide;
                        hide.name = IniKeywords::TextureOverride + toModName_ + TextTools::capitalize(config_.slotPrefix)
                                    + std::to_string(slot) + IniKeywords::Remap + "Hide";
                        hide.comment = "; nothing of the mod is drawn through " + toModName_ + "'s " + labelText
                                       + " slot: the skin's own geometry is skipped and its bones still merged";
                        hide.vb0Hash = target_.vb0Hash;
                        hide.indexOffset = s.indexOffset;
                        hide.indexCount = s.indexCount;
                        hide.vgOffset = s.vgOffset;
                        hide.vgCount = s.vgCount;
                        hide.slot = std::to_string(slot);
                        hide.legacyMergeList = legacy_ ? mergeListName(static_cast<int>(slot)) : std::string();
                        hide.mergeWindowList = targetPast256_ ? remapFixName("CommandListMergeWindow") : std::string();
                        hide.mergeSkeletonList = remapFixName("CommandListMergeSkeleton");

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

                        addFixSection(WWMISections::hideSection(z3_, hide));

                        if (stash != config_.stashRegs.end()) {
                            for (const std::string& reg : stash->second) {
                                addFixSection(SectionText(z3_, stashResource(static_cast<int>(slot), reg)).build());
                            }
                        }
                    }

                    if (legacy_ || targetPast256_) {
                        legacySkeletonSections();
                    }

                    passFilter("");
                    for (std::size_t i = 0; i < passTags_.tags().size(); ++i) {
                        addFixSection(passTags_.section(i, remapFixName("ShaderOverridePass" + std::to_string(i))));
                    }

                    // Other meshes the character draws -- see WWMIFixerConfig::sharedMeshes
                    std::size_t meshNum = 0;
                    for (const auto& [meshHash, byPass] : config_.sharedMeshes) {
                        SectionText mesh(z3_, remapFixName(IniKeywords::TextureOverride + source_.name
                                                      + "SharedMesh" + std::to_string(meshNum)));
                        mesh.key(IniKeywords::Hash, meshHash);

                        bool drew = false;
                        for (const auto& [pass, bindings] : byPass) {
                            std::vector<Kvp> lines;
                            for (const WWMIFixerConfig::Binding& binding : bindings) {
                                const std::string* resource = sharedResourceFor(binding.role);
                                if (resource != nullptr) {
                                    lines.push_back(bindLine(binding.role, binding.reg, *resource));
                                }
                            }

                            if (!lines.empty()) {
                                mesh.open(passCondition(pass)).keys(lines).close();
                                drew = true;
                            }
                        }

                        if (drew) {
                            addFixSection(mesh.build());
                        }

                        ++meshNum;
                    }

                    // The target's meshes the mod replaces -- see WWMIFixerConfig::hiddenMeshes
                    for (std::size_t i = 0; i < config_.hiddenMeshes.size(); ++i) {
                        addFixSection(SectionText(z3_, remapFixName(IniKeywords::TextureOverride + toModName_
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

                    buildCopyMerges();

                    // The fix's sections merge too -- a hidden slot's, and a copy's merge-only ones -- and
                    // latchEdits reaches them as it does the mod's. A hide section left out would set
                    // `$object_detected` itself, and on a frame where it runs first the remap would not run
                }

                // A COPY'S SKELETON HAS TO RECEIVE EVERY SLOT'S BONES (2026-10-06).
                //
                // Each generated .ini declares the fix's merged skeleton for itself, and only the
                // sections THAT file matches merge into it. The mod's own file draws every slot it
                // claims and so fills the whole skeleton; a copy draws one or two slots and leaves
                // every other window zero. Its geometry then skins the bones of the other slots
                // against zero matrices and collapses to the origin -- which on LynaePeppermint ->
                // Lynae, two source components merged onto her jacket slot, drew huge polygons over
                // the whole scene. So each copy gets a merge-only section for every target slot it
                // does not draw: no skip, no draw, only the window into that file's skeleton.
                void buildCopyMerges() {
                    if (!targetPast256_ || !config_.copiesShareSkeleton || groups_.size() < 2) {
                        return;
                    }

                    for (std::size_t g = 1; g < groups_.size(); ++g) {
                        std::set<int> own;
                        for (int component : groups_[g]) {
                            own.insert(config_.plan.at(component).slot);
                        }

                        for (std::size_t slot = 0; slot < target_.slots.size(); ++slot) {
                            const int s = static_cast<int>(slot);
                            if (own.count(s) > 0 || drawnSlots_.count(s) == 0) {
                                continue;            // drawn here, or already a hide section's job
                            }

                            const Slot& window = target_.slots[slot];
                            SectionText merge(z3_, IniKeywords::TextureOverride + toModName_
                                                       + TextTools::capitalize(config_.slotPrefix)
                                                       + std::to_string(slot) + IniKeywords::Remap + "Merge");
                            merge.prefix("; another file draws this slot: only its bones are merged into this file's skeleton")
                                .keys({{IniKeywords::Hash, target_.vb0Hash},
                                       {IniKeywords::MatchFirstIndex, window.indexOffset},
                                       {IniKeywords::MatchIndexCount, window.indexCount}});
                            merge.open(IniKeywords::WWMIModEnabled)
                                .keys({{IniKeywords::WWMIVgOffset, window.vgOffset},
                                       {IniKeywords::WWMIVgCount, window.vgCount},
                                       {IniKeywords::Run, remapFixName("CommandListMergeWindow")}})
                                .close();
                            addFixSection(merge.build(), g);
                        }

                    }
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
                    return modFile(meshFolder_, IniNamingTools::getRemapElementName("", "BlendRemap" + which, toModName_) + ".buf");
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

                std::string mirrorIndexFile(int component) const {
                    return modFile(meshFolder_, IniNamingTools::getRemapElementName("", "MirrorIndex" + std::to_string(component), toModName_)
                                                    + ".buf");
                }

                std::string mirrorVectorFile() const {
                    return modFile(meshFolder_, IniNamingTools::getRemapElementName("", "MirrorVector", toModName_) + ".buf");
                }

                /**
                 * @brief
                 @rst
                 The components a mirrored twin is actually written for -- :cpp:member:`
                 WWMIFixerConfig::mirroredComponents` minus the ones this mod cannot carry one for
                 @endrst
                 *
                 * Computed rather than stored: it is read while the edits are built, while the
                 * files are written and while the ``.ini`` is rendered, and those run in that order
                 * but through different entry points -- a member set in one of them is a member
                 * read before it was filled in another.
                 *
                 * @return The components
                 */
                std::set<int> mirroredSet() const {
                    std::set<int> out;
                    if (config_.mirroredComponents.empty() || indexFile_.empty() || vectorFile_.empty()) {
                        return out;
                    }

                    for (int component : config_.mirroredComponents) {
                        if (present_.count(component) == 0) {
                            continue;
                        }

                        const auto ranges = drawRanges_.find(component);
                        if (ranges == drawRanges_.end() || ranges->second.empty()) {
                            continue;
                        }

                        out.insert(component);
                    }

                    return out;
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


                // The mirrored twin's two buffers -- see WWMIFixerConfig::mirroredComponents. The
                // index one per component, holding that component's window wound the other way; the
                // vector one once, the mod's own normals negated. Both are byte transforms of files
                // the mod already has, so they go out as bytes rather than through BufFile's element
                // declarations -- and the write is READ BACK, which is the lesson the zero stream
                // above carries: a discarded write result is how a run reports editing files it
                // never wrote.

                // R8G8B8A8_SNORM, 8 bytes a vertex: the normal in 0..2 and the tangent in 4..7. -128
                // has no positive counterpart in SNORM, so it clamps rather than wrapping to itself.
                ByteVec mirrorVectorBytes() const {
                    const std::string folder = ctx_.getIniFile()->getFolder();
                    const auto snorm8 = [] { return std::make_unique<BufSignedInt>("SignedInt8", 1, false); };
                    std::vector<std::unique_ptr<BufElementType>> elements;
                    elements.push_back(BufElementType::repeated(MirrorNormalElement, 4, snorm8, "R8G8B8A8_SNORM"));
                    elements.push_back(BufElementType::repeated(MirrorTangentElement, 4, snorm8, "R8G8B8A8_SNORM"));
                    const BufFile::Filter negateNormal = [](const BufLineData& line, long long, double, long long) {
                        BufLineData out = line;
                        auto normal = out.find(MirrorNormalElement);
                        for (std::size_t c = 0; normal != out.end() && c < 3 && c < normal->second.size(); ++c) {
                            normal->second[c] = std::max(-127LL, std::min(127LL, -bufValueAsInt(normal->second[c])));
                        }

                        return out;
                    };

                    try {
                        BufFile vectors{FileService::absPathOfRelPath(vectorFile_, folder), std::move(elements)};
                        return std::get<ByteVec>(vectors.fix(std::nullopt, {negateNormal}));
                    } catch (const std::exception& exception) {
                        throw std::runtime_error("cannot build the mirrored vector buffer " + mirrorVectorFile() + ": " + exception.what());
                    }
                }

                // The span a component's draws cover, as (first index, index count). Several
                // ranges are one object split into pieces, so the twin buffer is their whole span
                // and each twin draw indexes into it at `its first - the span's first`.
                std::pair<long long, long long> mirrorSpan(int component) const {
                    const auto& ranges = drawRanges_.at(component);
                    long long low = ranges.front().second;
                    long long high = ranges.front().second + ranges.front().first;
                    for (const auto& range : ranges) {
                        low = std::min(low, range.second);
                        high = std::max(high, range.second + range.first);
                    }

                    return {low, high - low};
                }

                ByteVec mirrorIndexBytes(int component) const {
                    const std::string folder = ctx_.getIniFile()->getFolder();
                    const auto span = mirrorSpan(component);
                    const long long first = span.first;
                    const long long count = span.second;

                    // Reversing each triangle's winding is the whole of the twin: the SAME vertices,
                    // presented to the rasterizer the other way.
                    ByteVec out;
                    try {
                        out = IbFile{FileService::absPathOfRelPath(indexFile_, folder)}.windingReversed(
                            static_cast<std::size_t>(first), static_cast<std::size_t>(count));
                    } catch (const std::out_of_range&) {
                        throw std::runtime_error("component " + std::to_string(component)
                                                 + " draws past the end of this mod's index buffer");
                    }

                    return out;
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
                void planTexEdits() {
                    if (config_.texEdits.empty() || hasGivenUp()) {
                        return;
                    }

                    for (const WWMIFixerConfig::TexEdit& edit : config_.texEdits) {
                        if (!edit.makeFilter || sharedResourceFor(edit.role) == nullptr) {
                            continue;
                        }

                        // the source's own textures the edit compares against -- downloaded, never bound
                        for (const std::string& role : edit.sourceGameRoles) {
                            if (!addGameDownload(role)) {
                                ctx_.log("the " + edit.name + " edit of " + edit.role + " reads " + source_.name
                                     + "'s own " + role + ", and fallbackTextures has no hash for it");
                            }
                        }

                        // the mod's own file, or -- for a role it has none for -- the one the
                        // fallback download lands
                        if (fileOfRole_.count(edit.role) == 0 && fallbacks_.count(edit.role) == 0) {
                            continue;
                        }

                        // The resource the edit reads -- every binding of it in the role's register
                        // is collected and edited after the group edits (addTexEditCollects)
                        const std::string* was = sharedResourceFor(edit.role);
                        if (was != nullptr) {
                            noteEdited(*was, edit.role);

                            // EVERY variant of the role, not just the one fileOfRole_ resolved to.
                            // An edit reads one file, so a mod that binds its mask two ways -- Chisa12
                            // does, on $sockscolor -- had the second variant carried through
                            // unrepacked and shaded as the wrong material. Harmless while nothing
                            // could reach that variant; a live defect once the toggle chains could.
                            //
                            // The resolved variant keeps the name it already had, so no shipped
                            // output moves; the others are indexed after it.
                            std::size_t n = 1;
                            std::string groupLabel;
                            const std::string wasKey = StringTools::toLower(*was);

                            const auto emitVariants =
                                [&](const std::vector<std::string>& group, const std::string& from) {
                                    for (const std::string& variant : group) {
                                        if (StringTools::equalsIgnoreCase(variant, *was)) {
                                            continue;
                                        }

                                        // ALREADY CLAIMED BY ANOTHER ROLE'S EDIT. On a merged mesh
                                        // one register carries several source components' art, so a
                                        // file can sit in two groups; the first role to claim it
                                        // keeps it, because a second claim would run that role's
                                        // filter over it -- a mask repack over a diffuse. A role of
                                        // the SAME kind does claim it (2026-10-12): Lynae9's sweater
                                        // chain is her jacket's diffuse and her props' diffuse, and
                                        // with only the jacket's edits the props drew four of the
                                        // five variants ungraded
                                        const auto claimed = editedRolesOf_.find(StringTools::toLower(variant));
                                        if (claimed != editedRolesOf_.end()) {
                                            const bool otherKind =
                                                std::any_of(claimed->second.begin(), claimed->second.end(),
                                                            [&edit](const std::string& by) {
                                                                return by == edit.role || roleKind(by) != roleKind(edit.role);
                                                            });
                                            if (otherKind) {
                                                continue;
                                            }
                                        }

                                        const auto file = fileOfResource_.find(StringTools::toLower(variant));
                                        if (file == fileOfResource_.end()) {
                                            continue;
                                        }

                                        ++n;
                                        noteEdited(variant, edit.role);

                                        if (groupLabel.empty()) {
                                            groupLabel = from;
                                        }
                                    }
                                };

                            if (const IniParseData::TextureToggle* toggle = parseData().thisToggleOf(wasKey)) {
                                emitVariants(toggle->variants, toggle->section);
                            }

                            // ...and the same toggle written as a register binding -- see
                            // IniParseData::registerToggleOf. Both shapes feed one numbering, so a mod that
                            // uses both does not get two `...2RemapTex` files.
                            if (const IniParseData::TextureToggle* toggle = parseData().registerToggleOf(wasKey)) {
                                emitVariants(toggle->variants, toggle->section);
                            }

                            if (n > 1) {
                                ctx_.log(edit.role + ": " + std::to_string(n) + " variants bound by "
                                         + groupLabel + ", each given its own " + edit.name + " edit");
                            }
                        }
                    }
                }

                // ONE FILE, SEVERAL EDITED ROLES (2026-10-12): Lynae9 binds one mask and one code map in
                // her lower body AND her jacket, each edited per role, so a resource keeps every role
                // edited from it -- and each role the resources its edit applies to
                void noteEdited(const std::string& resource, const std::string& role) {
                    const std::string key = StringTools::toLower(resource);
                    editTargets_[role].insert(key);
                    std::vector<std::string>& roles = editedRolesOf_[key];
                    if (std::find(roles.begin(), roles.end(), role) == roles.end()) {
                        roles.push_back(role);
                    }
                }

                // The source's own game texture of 'role', downloaded, with every edit of the role run
                // on it -- the resource lands in gameResourceOfRole_. Once per role; false when the
                // character's facts give it no hash. The target's shader reads it the same way whoever
                // binds it, so a mask still needs its repack and a diffuse its alpha clamp.
                bool planGameResource(const std::string& role) {
                    if (gameResourceOfRole_.count(role) > 0) {
                        return true;
                    }

                    if (!addGameDownload(role)) {
                        return false;
                    }

                    gameResourceOfRole_[role] = gameFallbacks_.at(role).resource;
                    editTargets_[role].insert(StringTools::toLower(gameFallbacks_.at(role).resource));
                    return true;
                }

                // THE SOURCE'S OWN GAME TEXTURE AS THE DEFAULT, where the mod's file reaches only some of a
                // component's draws -- see the stand-in rule above. Three cases, each the source's own texture
                // where the original drew with the GAME's:
                //
                //   1. the draws a section makes BEFORE it binds anything (#lateBindRegs_): they rendered with
                //      the textures the game had bound when it matched the draw (ChisaParfait3's hat and chest
                //      bandages). A role the mod has no file for is already the source's game texture (its
                //      stand-in, edits and all), so only a role the mod DOES serve needs a second file.
                //   2. a role bound only under a toggle that reaches none of the section's draws (Lynae11 binds
                //      its own material-code map only `if $Pants != 0`, and its default outfit drew with the
                //      game's) -- unless a `this` on a texture's hash binds it for every draw anyway (Lynae3).
                //      Which resources a component binds only under a toggle is the parser's
                //      (IniParseData::toggledBindings); a reroute reads its source component's.
                //   3. a role bound with ONE `this` under a toggle (IniParseData::TextureToggle::single): off, the
                //      original draws with the game's texture, which bindLine puts at the front of the role's list.
                void planGameDefaults() {
                    if (hasGivenUp() || config_.sourceTextures.downloadCharFolder.empty()) {
                        return;
                    }

                    // 1. the draws before the first binding
                    std::set<std::string> lateRoles;
                    for (const auto& entry : lateBindRegs_) {
                        const auto planned = config_.plan.find(entry.first);
                        for (const WWMIFixerConfig::Binding& binding : planned != config_.plan.end() ? bindingsOf(planned->first)
                                                                                                    : std::vector<WWMIFixerConfig::Binding>{}) {
                            if (binding.role != IniKeywords::Null && leftToGame_.count(binding.role) == 0) {
                                lateRoles.insert(binding.role);
                            }
                        }
                    }

                    std::vector<std::string> downloaded;
                    for (const std::string& role : lateRoles) {
                        if (fileOfRole_.count(role) == 0) {
                            if (const std::string* resolved = sharedResourceFor(role)) {
                                gameResourceOfRole_[role] = *resolved;       // already the source's, edits and all
                            }
                        } else if (planGameResource(role)) {
                            downloaded.push_back(role);
                        } else {
                            ctx_.log("the draws " + config_.slotPrefix + " sections make before binding anything need "
                                     + source_.name + "'s own " + role + ", and fallbackTextures has no hash for it -- those"
                                     " draws keep the target's texture at the mod's UVs");
                        }
                    }
                    if (!downloaded.empty()) {
                        std::string names;
                        for (const std::string& role : downloaded) {
                            names += (names.empty() ? "" : ", ") + role;
                        }
                        ctx_.log(std::to_string(lateBindRegs_.size()) + " component section(s) draw before binding a texture"
                                 " register, so those draws take " + source_.name + "'s own " + names + " rather than the mod's");
                    }

                    // 2. a role bound only under a toggle, and 3. a one-variant toggle
                    const IniParseData& data = parseData();
                    for (int pass = 2; pass <= 3; ++pass) {
                        for (const auto& [component, sections] : present_) {
                            (void)sections;
                            const auto planned = config_.plan.find(component);
                            if (planned == config_.plan.end()) {
                                continue;
                            }

                            const auto alias = rerouteFrom_.find(component);
                            const std::set<std::string>& toggledOnly = data.toggledBindings(
                                ModObj("", config_.slotPrefix + std::to_string(alias != rerouteFrom_.end() ? alias->second : component)));
                            if (pass == 2 && toggledOnly.empty()) {
                                continue;
                            }

                            for (const WWMIFixerConfig::Binding& binding : bindingsOf(planned->first)) {
                                const std::string* resource = resourceFor(binding.role, component);
                                if (binding.role == IniKeywords::Null || leftToGame_.count(binding.role) > 0 || resource == nullptr
                                        || (pass == 2 && fileOfRole_.count(binding.role) == 0)) {
                                    continue;
                                }

                                const std::string base = StringTools::toLower(*resource);
                                if (pass == 3) {
                                    const IniParseData::TextureToggle* toggle = data.thisToggleOf(base);
                                    if (toggle != nullptr && toggle->single) {
                                        planGameResource(binding.role);
                                    }
                                    continue;
                                }

                                if (toggledOnly.count(base) == 0 || data.isThisBound(base)) {
                                    continue;               // bound unconditionally, or some other way (a `this` override)
                                }
                                if (!planGameResource(binding.role)) {
                                    ctx_.log(config_.slotPrefix + std::to_string(component) + " binds its " + binding.role
                                             + " only under a toggle, and fallbackTextures has no hash for " + source_.name
                                             + "'s own -- the mod's file stays bound in every branch");
                                    continue;
                                }
                                toggledGame_[{binding.role, component}] = gameResourceFor(binding.role, component);
                                ctx_.log(config_.slotPrefix + std::to_string(component) + " binds its " + binding.role
                                         + " only under a toggle; elsewhere " + source_.name + "'s own is bound, as the game does");
                            }
                        }
                    }
                }

                // The source's own game texture of 'role' as a download (gameFallbacks_), once; false
                // when the character's facts give it no hash or no download folder. Declaring and
                // downloading it binds nothing -- gameResourceOfRole_ is what binds.
                bool addGameDownload(const std::string& role) {
                    const WWMITextureFacts& source = config_.sourceTextures;
                    if (source.downloadCharFolder.empty()) {
                        return false;
                    }

                    if (gameFallbacks_.count(role) > 0) {
                        return true;
                    }

                    const auto hash = source.fallbackTextures.find(role);
                    if (hash == source.fallbackTextures.end()) {
                        return false;
                    }

                    gameFallbacks_[role] = fallbackOf(TextTools::capitalize(role) + GameSuffix, hash->second);
                    return true;
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

                // `CommandList<Source>Component<N><infix>Textures<suffix>`, under the fix's name
                std::string textureListName(int component, const std::string& infix = "", const std::string& suffix = "") const {
                    return remapFixName(IniKeywords::CommandList + source_.name + TextTools::capitalize(config_.slotPrefix)
                                        + std::to_string(component) + infix + "Textures" + suffix);
                }

                // A texture list among the fix's sections -- its lines under 'condition' -- and the (role,
                // register) pairs it binds, which is what addTexEditCollects edits by
                void addTextureList(const std::string& name, const std::string& condition, const TextureLines& list, int component) {
                    addFixSection(SectionText(z3_, name).open(condition).keys(list.lines).close().build());
                    if (rerouteShaders_.count(component) > 0) {
                        rawLists_.insert(name);
                    }
                    for (const auto& [role, reg] : list.bound) {
                        listsOfRole_[{role, reg}].insert(name);
                    }
                }

                // The texture edits (WWMIFixerConfig::texEdits), as collects over every binding of an
                // edited role: one TexReplace per edit, collected wherever the role's register
                // binds one of the resources planTexEdits recorded for it -- in the fix's texture lists
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
                    context.indexFile = FileService::absPathOfRelPath(indexFile_, folder);
                    context.positionFile = positionBufPath();
                    context.texcoordFile = FileService::absPathOfRelPath(texcoordFile_, folder);
                    context.drawRanges = drawRanges_;
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
                    for (const auto& entry : gameFallbacks_) {
                        context.gameFileOfRole[entry.first] = FileService::absPathOfRelPath(entry.second.relPath, folder);
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
                            if (present_.count(component) == 0 || rerouteShaders_.count(component) > 0) {
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
                    if (indexFile_.empty() || ini == nullptr) {
                        return std::vector<bool>(texcoord.us().size(), false);
                    }

                    try {
                        return texcoord.straddling(IbFile{FileService::absPathOfRelPath(
                            FileService::iniPathToRel(indexFile_), ini->getFolder())}.getIndices());
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
                    texcoordResource_ = remapFixName(IniKeywords::Resource + "TexcoordNoNaN");
                    addFixFile(*texcoordResource_,
                               {{IniKeywords::Type, IniKeywords::Buffer},
                                {IniKeywords::Format, std::string(format.has_value()
                                                           ? StringTools::strip(*format)
                                                           : std::string_view("DXGI_FORMAT_R16G16_FLOAT"))},
                                {IniKeywords::Stride, std::to_string(stride)}},
                               fixedRel,
                               [path = source->path, stride, keep]() {
                                   TexcoordFile copy{path, TexcoordFile::halfElements(stride)};
                                   copy.clean(keep);
                                   return copy.getData();
                               },
                               {config_.texcoordReg});
                    ctx_.log("texcoords: " + std::to_string(stats.cleared) + " NaN halves set to 0 and "
                             + std::to_string(stats.folded) + " U values folded into [0, 1) in a remap-only copy");
                }

                /**
                 * @brief
                 @rst
                 Registers the download of the SOURCE's own game texture for one role, and points
                 every component with no file of its own at it
                 @endrst
                 *
                 * @param role The role with no file of the mod behind it
                 * @param hash The source's texture hash of that role
                 */
                void registerFallback(const std::string& role, const std::string& hash) {
                    fallbacks_[role] = fallbackOf(TextTools::capitalize(role), hash);
                    resourceOfRole_[role] = fallbacks_[role].resource;
                }

                // The source's game texture of 'hash', downloaded as `<prefix><kind>RemapDL.dds` under the resource
                // IniNamingTools names for it
                Fallback fallbackOf(const std::string& kind, const std::string& hash) const {
                    const WWMITextureFacts& source = config_.sourceTextures;
                    const std::string fileName = DownloadTools::fixedFileName(source.downloadPrefix, kind, FileExt::DDS);
                    return Fallback{
                        DownloadTools::url(source.downloadGameFolder, source.downloadCharFolder, source.downloadVersionFolder,
                                           source.downloadPrefix, "Texture" + hash, FileExt::DDS),
                        fileName, fileName, IniNamingTools::getRemapDLResourceName(source.downloadPrefix + kind)};
                }


                WWMIFixerConfig config_;
                bool editTagDone_ = false;
                std::string editTag_;

                std::optional<ModType> targetType_;
                Character source_;
                Character target_;
                std::map<int, std::vector<std::string>> present_;     // source component -> its sections
                std::map<int, ModObj> graphOf_;                       // source component -> the parser's graph of those sections
                std::vector<int> silent_;                              // components whose sections draw nothing
                std::vector<int> dropped_;
                std::vector<std::vector<int>> groups_;
                std::vector<ModObj> hashOnlyObjs_;                    // ...and the mod's objects that are not slots

                // see buildToggleLists
                struct ToggleList {
                    std::string name;                                   // the fix's list
                    ModObj graph;                                       // the parser's graph holding the mod's override
                    std::string section;                                // the mod's override
                    std::string reg;                                    // the register the list binds
                    std::vector<std::string> thisKeys;                  // the spellings of `this` the override writes
                    RegRemove<>::RemoveKeyCheck isDead;                 // a `this` naming nothing
                    std::optional<std::string> offBinding;              // a one-variant toggle's binding while off
                };
                std::vector<ToggleList> toggleLists_;
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
                std::string rebindTo_;                  // the group being rebound's remapped blend
                std::unique_ptr<RegNewVals<>> rebind_;
                std::unique_ptr<RegPartEdit<>> rebindAdapter_;
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
                std::vector<std::string> remapNames_;   // the mod names a fix of this .ini could have named its sections after
                mutable std::optional<std::optional<std::string>> modVb0Version_;   // versionOfModVb0's answer, once asked
                std::vector<std::pair<std::string, std::string>> declared_;   // (file, path relative to the .ini) for a file no resource of the .ini names
                std::map<std::string, std::string> declaredName_;      // that file -> the resource section the fix declares for it
                std::set<std::string> usedDeclaredNames_;
                std::map<std::string, Fallback> fallbacks_;           // role -> the source's game texture, for a planned role the mod has no file for
                std::map<std::string, Fallback> gameFallbacks_;      // ...and the same download for a role the mod DOES serve, for the pre-binding draws of a split section
                std::map<std::string, std::string> gameResourceOfRole_;  // role -> what a pre-binding draw binds: the source's own game texture, edited
                std::map<std::pair<std::string, int>, std::string> toggledGame_;  // (role, source component) -> the game's own, for a role the mod binds only under a toggle
                std::map<int, std::vector<std::string>> lateBindRegs_;   // source component -> the registers its own section binds AFTER it has already drawn
                std::set<std::string> leftToGame_;                    // roles whose only file was flat and whose config says not to stand anything in

                ShaderFilterTags passTags_;                            // the shaders the passes are told apart by -- passFilter
                bool passTagsBuilt_ = false;



                bool legacy_ = false;                                 // a mod from before WWMI's merged skeleton
                // EMPTY UNTIL THE MOD'S OWN `.ini` SAYS OTHERWISE (2026-09-30). Each of these used to
                // start at `Meshes/<Name>.buf`, so a mod declaring no such section was read at a path
                // the fix invented -- the same assumption about a mod's folder structure that
                // `DefaultTextureFolder` / `DefaultMeshFolder` made on the write side. A mod-manager
                // -packaged mod names every file by GUID with a `.assets` extension and has no
                // `Meshes/` at all.
                std::string indexFile_;                               // the mod's own [ResourceIndexBuffer]
                std::string positionFile_;                            // ...and [ResourcePositionBuffer]
                std::string blendSourceFile_;                         // ...and [ResourceBlendBuffer]; blendFixedFile() derives the fix's name FROM it
                std::string texcoordFile_;                            // ...and [ResourceTexcoordBuffer]
                std::string vectorFile_;                              // ...and [ResourceVectorBuffer]; the normals the mirrored twin negates
                std::map<int, std::vector<std::pair<long long, long long>>> drawRanges_;   // source component -> its (index count, first index) draws
                std::map<int, std::set<std::size_t>> drawParts_;      // ...and which PARTS those draws sit in -- see mirroredSet()
                std::map<int, int> rerouteFrom_;                      // WWMIFixerConfig::DrawReroute: its `as` -> its `from`, for a reroute that took draws
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
                std::map<int, std::set<std::pair<long long, long long>>> reroutedDraws_;   // ...and those draws, keyed by `as`
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
                bool hostResetsDetected_ = false;                     // the host's [Present] does `post $object_detected = 0`

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
                std::unique_ptr<RegRemove<>> regRemove_;
                std::unique_ptr<RegPartEdit<>> removeAdapter_;
                std::unique_ptr<RegRemove<>> regRemoveKeepRabbitFX_;               // regRemove_ without RabbitFX's lines
                std::unique_ptr<RegPartEdit<>> removeKeepRabbitFXAdapter_;
                std::map<int, std::array<int, 4>> rabbitFXFlags_;                 // SourceComponent::rabbitFX: component -> the source's x187..w187
                std::vector<std::unique_ptr<AnchorAdd>> rabbitFXPins_;
                std::map<int, std::unique_ptr<RegPartEdit<>>> rabbitFXPinAdapters_;
                std::map<int, PartEdit*> newValsOf_;
                std::vector<std::unique_ptr<RegSurroundedAdd<>>> surroundedAdds_;
                std::vector<std::unique_ptr<GraphPartEdit<>>> graphAdapters_;
                std::vector<std::unique_ptr<RegNewVals<>>> newVals_;
                std::vector<std::unique_ptr<RegPartEdit<>>> regAdapters_;
                std::vector<std::unique_ptr<AnchorAdd>> twinEdits_;   // owned beside the adapters that wrap them
                std::vector<std::unique_ptr<AnchorAdd>> carriedEdits_;   // ...and the same for the carried-register edits
                std::vector<std::unique_ptr<BaseRegEdit<>>> bypassEdits_;     // ...and for the cleanup-list edits
                Bypasses bypasses_;                            // register -> the resource its binding is parked in
                std::map<int, std::vector<PartEdit*>> editsOf_;
                std::unique_ptr<ObjGroupEdit> mainEdits_;
                std::vector<std::unique_ptr<VGRemapBlendReplace<>>> blendReplaces_;
                std::optional<std::string> vertexVGFile_;                            // WWMI's 16-bit ids, as the .ini names them
                std::optional<std::string> texcoordResource_;                        // the cleaned texcoord copy's resource
                std::unordered_map<std::string, std::string> fileOfRole_;            // role -> the file it resolved to
                // One texture edit, named and sourced at read time and registered at fix time

                std::unordered_map<std::string, std::string> fileOfResource_;         // the mod's resource -> the file it names
                std::unordered_map<std::string, std::vector<std::string>> editedRolesOf_;  // the mod's resource -> every role edited from it
                std::map<std::string, std::set<std::string>> editTargets_;           // an edited role -> the resources (lowered) its edit applies to
                std::set<std::string> rawLists_;                                     // the fix's lists for a reroute drawn by its own shader: the mod's art, unedited
                std::map<std::pair<std::string, std::string>, std::set<std::string>> listsOfRole_;   // (role, register) -> the fix's lists binding that role there
                std::vector<std::unique_ptr<Collector>> texEditCollects_;            // addTexEditCollects' collects, among fixSectionEdits
                std::vector<std::unique_ptr<TexReplace<>>> texEditors_;        // ...and the edits they build with
                std::map<std::pair<std::string, std::string>, std::string> roleLists_;   // (role, register) -> the copied list's name
                std::vector<std::unique_ptr<Collector>> blendCollects_;
        };
    }


    IniFixBuilder::Factory makeWWMIFixer(WWMIFixerConfig config) {
        return [config](BaseIniParser<>* parser, const std::string& toModName, std::optional<int> modTypeId) {
            return std::make_shared<WWMIFixerImpl>(parser, toModName, modTypeId, config);
        };
    }
}
