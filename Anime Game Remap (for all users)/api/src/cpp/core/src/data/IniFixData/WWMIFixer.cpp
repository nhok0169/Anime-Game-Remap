#include "AGRemapCore/data/IniFixData/WWMIFixer.h"

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

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <cstring>
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
#include "AGRemapCore/data/IniFixData/ModBranches.h"
#include "AGRemapCore/data/WWMITextureFacts.h"
#include "AGRemapCore/data/WWMITextureRoles.h"
#include "AGRemapCore/data/IniParseData/WWMIParser.h"
#include "AGRemapCore/model/iftemplate/IfContentPart.h"
#include "AGRemapCore/model/iftemplate/IfPredPart.h"
#include "AGRemapCore/model/iftemplate/IfTemplate.h"
#include "AGRemapCore/model/iftemplate/IfTemplateRender.h"
#include "AGRemapCore/tools/z3/Z3Context.h"
#include "AGRemapCore/model/IniNamingTools.h"
#include "AGRemapCore/model/assets/Hashes.h"
#include "AGRemapCore/model/Version.h"
#include "AGRemapCore/model/buffers/BufElementType.h"
#include "AGRemapCore/model/buffers/BufFloat.h"
#include "AGRemapCore/model/buffers/BufInt.h"
#include "AGRemapCore/model/files/IbFile.h"
#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/model/files/TextureFile.h"
#include "AGRemapCore/model/files/IniScan.h"
#include "AGRemapCore/model/textures/TexThumbprint.h"
#include "AGRemapCore/model/iftemplate/IfTemplateRender.h"
#include "AGRemapCore/model/iniresources/RemapBlendResource.h"
#include "AGRemapCore/model/iniresources/RemapIniResource.h"
#include "AGRemapCore/model/iniresources/RemapTexResource.h"
#include "AGRemapCore/model/strategies/ModType.h"
#include "AGRemapCore/model/strategies/iniFixers/GIMIFixer.h"
#include "AGRemapCore/model/strategies/iniFixers/IniFileFixContext.h"
#include "AGRemapCore/model/strategies/iniFixers/graphEdits/GraphRename.h"
#include "AGRemapCore/model/strategies/iniFixers/graphEdits/RegSurroundedAdd.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphGroupEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphGroupPartEdits.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphGroupRemap.h"
#include "AGRemapCore/tools/DownloadTools.h"
#include "AGRemapCore/tools/files/FileDownload.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphGroupRemove.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphRemove.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/ResRegCollect.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/BlendEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegAssetRemap.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegNewVals.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegRemap.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegRemove.h"

#include <stdexcept>
#include "AGRemapCore/model/strategies/iniParsers/GIMIParser.h"
#include "AGRemapCore/model/strategies/texEditors/TexCreator.h"
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

        // Hash-data keys and .ini registers spelled literally, as the rest of data/ spells them.
        const std::string Vb0HashKey = "vb0";
        const std::string ShapeKeyChecksumKey = "$\\WWMIv1\\shapekey_checksum";
        const std::string ShapeKeyType = "shapekeys";
        const std::string VgOffsetKey = "$\\WWMIv1\\vg_offset";
        const std::string VgCountKey = "$\\WWMIv1\\vg_count";
        const std::string MeshVertexCountKey = "global $mesh_vertex_count";
        const std::string ConstantsSection = "Constants";
        const std::string BlendBufferResource = "ResourceBlendBuffer";
        const std::string IndexBufferResource = "ResourceIndexBuffer";
        const std::string PositionBufferResource = "ResourcePositionBuffer";
        const std::string TexcoordBufferResource = "ResourceTexcoordBuffer";

        // ONE OF THE FIX'S OWN SECTIONS, BUILT RATHER THAN CONCATENATED (2026-09-29).
        //
        // `IfTemplate` is a section, `IfContentPart` a run of `key = value` lines, `IfPredPart` an
        // `if` / `endif`, and `renderIfTemplate` turns the three back into text -- including the
        // `[name]` header and the indentation. This file used to assemble all of that as strings,
        // twenty section headers' worth, which is the same structure written twice: once as the
        // model every other part of the fix is edited through, and once as text here.
        //
        // The renderer indents with a TAB where the string version used four spaces. That is the
        // only output difference, it is inside the fix's own block, and it makes the file
        // self-consistent -- every section that reaches the page through the section graph is
        // already rendered by this same function.
        class SectionText {
            public:
                SectionText(Z3Context& z3Ctx, std::string name): z3_(z3Ctx), name_(std::move(name)) {}

                // A run of `key = value` lines. A value of "" is a line with no `=` at all --
                // 3dmigoto's `local $var` -- which is how IfContentPart already reads one.
                SectionText& keys(const std::vector<std::pair<std::string, std::string>>& kvps) {
                    parts_.push_back(std::make_unique<IfContentPart<std::string, std::string>>(kvps, depth_));
                    return *this;
                }

                SectionText& key(const std::string& k, const std::string& v = "") {
                    return keys({{k, v}});
                }

                SectionText& open(const std::string& predicate) {
                    parts_.push_back(std::make_unique<IfPredPart>("if " + predicate, IfPredPartType::If, z3_));
                    ++depth_;
                    return *this;
                }

                // A comment above the `[name]` line. IfTemplate carries it, so even this is
                // the model's rather than text glued on the front.
                SectionText& prefix(std::string text) {
                    prefix_ = std::move(text);
                    return *this;
                }

                SectionText& close() {
                    if (depth_ > 0) {
                        --depth_;
                    }

                    parts_.push_back(std::make_unique<IfPredPart>("endif", IfPredPartType::EndIf, z3_));
                    return *this;
                }

                // The blank line after a section is this file's own convention, not the renderer's.
                std::string str() {
                    IfTemplate<std::string, std::string> section{
                        std::move(parts_),
                        IfTemplateRunConfig<std::string, std::string>{
                            IniKeywords::Run,
                            [](const std::string& val) { return val; },
                            [](const std::string& name) { return name; }},
                        name_,
                        IfTemplate<std::string, std::string>::TreeKind::NonEmptyNode,
                        prefix_};

                    return renderIfTemplate(section) + "\n\n";
                }

            private:
                Z3Context& z3_;
                std::string name_;
                std::string prefix_;
                std::vector<std::unique_ptr<IfTemplatePart>> parts_;
                int depth_ = 0;
        };


        // The one element a texcoord line decodes into: `stride / 2` halves, named so
        // the BufFile filters can find it in the decoded line.
        const std::string TexcoordElement = "Texcoord";
        const std::string Cb4HashKey = "cb4";
        constexpr std::size_t WWMIBlendStride = 8;      // four R8 bone indices then four R8 weights

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
        const std::string ChecksumNotFound = "ChecksumNotFound";
        const std::string DefaultTextureFolder = "Textures";
        const std::string DefaultMeshFolder = "Meshes";

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
        // A Position.buf is three floats a vertex, and dividing by that is how every lift below
        // learns the vertex count the rest of its work is built on. It was the bare digits at seven
        // sites across four functions until 2026-09-29.
        constexpr std::uintmax_t WWMIPositionStride = 12;

        // What a Texcoord.buf's bytes per vertex are taken to be when the resource declares
        // none of its own.
        constexpr std::size_t DefaultTexcoordStride = 16;

        // The highest bone an 8-bit blend index can name, and the entries in one WWMI blend remap.
        // Past the first, a character needs the remap; past the second, one remap cannot hold the
        // row (see readMod and writeBlendRemap, which check the same fact).
        constexpr long long WWMIMaxByteBone = 255;
        constexpr std::size_t WWMIBlendRemapSize = 512;

        // The D3D semantic names every line of these buffers is keyed by. A filter that asks for
        // the wrong one finds nothing, leaves every vertex alone and reports success, so they are
        // written once.
        const std::string WWMIBlendIndicesKey = "BLENDINDICES";
        const std::string WWMIBlendWeightKey = "BLENDWEIGHT";

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


        std::vector<std::unique_ptr<BufElementType>> wwmiBlendElements(std::size_t influences) {
            std::vector<std::unique_ptr<BufElementType>> elements;
            const std::string format = influences == 4 ? "R8G8B8A8_UINT" : "R8_UINT";
            for (const std::string& name : {WWMIBlendIndicesKey, WWMIBlendWeightKey}) {
                std::vector<std::unique_ptr<BufDataType>> types;
                for (std::size_t i = 0; i < influences; ++i) {
                    types.push_back(std::make_unique<BufUnSignedInt>("UnsignedInt8", 1, false));
                }

                elements.push_back(std::make_unique<BufElementType>(name, format, std::move(types)));
            }

            return elements;
        }

        // The SAME ids as 16-bit, which is what a character past 256 bones keeps in
        // BlendRemapVertexVG.buf -- one element of `influences` of them, no weights. Built here
        // rather than inline at each use: two functions need it, and the pair's whole risk is the
        // two disagreeing about the width.
        std::vector<std::unique_ptr<BufElementType>> wwmiVertexVGElements(std::size_t influences) {
            std::vector<std::unique_ptr<BufDataType>> wide;
            for (std::size_t i = 0; i < influences; ++i) {
                wide.push_back(std::make_unique<BufUnSignedInt>("UnsignedInt16", 2, false));
            }

            std::vector<std::unique_ptr<BufElementType>> elements;
            elements.push_back(
                std::make_unique<BufElementType>(WWMIBlendIndicesKey, "R16_UINT", std::move(wide)));

            return elements;
        }


        // How many bone influences a vertex the mod's OWN blend carries: its byte stride over the
        // vertex count, halved. 'vertices' is the mod's declared `global $mesh_vertex_count`, and the
        // Position.buf stands in when it has none (a mod-manager-packaged mod declares [Constants]
        // more than once, and the count can come back 0).
        //
        // Falls back to FOUR whenever it cannot be derived, which is exactly the behaviour before
        // this existed -- so a derivation that fails cannot move a shipped character's output.
        std::size_t wwmiBlendInfluences(const std::string& blendPath, long long vertices,
                                        const std::string& positionPath) {
            constexpr std::size_t Fallback = 4;
            std::error_code err;
            if (vertices <= 0 && !positionPath.empty()) {
                const std::uintmax_t positionSize =
                    std::filesystem::file_size(FileService::strToPath(positionPath), err);
                if (!err && positionSize >= 12) {
                    vertices = static_cast<long long>(positionSize / WWMIPositionStride);
                }
            }

            if (vertices <= 0) {
                return Fallback;
            }

            const std::uintmax_t blendSize =
                std::filesystem::file_size(FileService::strToPath(blendPath), err);
            if (err || blendSize == 0 || blendSize % static_cast<std::uintmax_t>(vertices) != 0) {
                return Fallback;
            }

            const std::uintmax_t stride = blendSize / static_cast<std::uintmax_t>(vertices);
            if (stride < 2 || stride % 2 != 0) {
                return Fallback;
            }

            return static_cast<std::size_t>(stride / 2);
        }

        BaseResEdit<>::ResEditConfig makeResEditConfig() {
            return BaseResEdit<>::ResEditConfig{
                IniKeywords::Filename,
                [](const std::string& value) { return value; },
                [](const std::string& file) { return file; }
            };
        }


        // ---- the blend: the library's vertex-group row, over the WWMI layout ----

        /**
         * RemapBlendReplace's buildResModel builds a straight copy; VGRemapBlendReplace's the GIMI
         * layout. This one is the WuWa layout over the same vertex-group lookup.
         */
        class WWMIBlendReplace: public RemapBlendReplace<> {
            public:
                using Base = RemapBlendReplace<>;
                using GraphId = Base::GraphId;
                using ResEditConfig = Base::ResEditConfig;
                using Context = Base::Context;

                WWMIBlendReplace(GraphId resModObj, ResEditConfig config, const ModType* modType,
                                 std::optional<Version> fromVersion, std::optional<Version> toVersion,
                                 std::function<bool(RemapBlendResource&)> fixFunc = {},
                                 std::map<long long, std::vector<long long>> anchorChains = {},
                                 long long vertices = 0, std::string positionPath = {}):
                    Base(std::move(resModObj), std::move(config), "blend"),
                    modType_(modType), fromVersion_(std::move(fromVersion)), toVersion_(std::move(toVersion)),
                    fixFunc_(std::move(fixFunc)), anchorChains_(std::move(anchorChains)),
                    vertices_(vertices), positionPath_(std::move(positionPath)) {}

            protected:
                void buildResModel(const std::string& resType, const std::string& srcPath, const std::string& fixedPath,
                                   const std::string& modName, const std::string& fileKey, Context& ctx) override {
                    (void)resType;
                    std::optional<VGRemap> vgRemap;
                    if (modType_ != nullptr) {
                        vgRemap = modType_->getVGRemap(modName, fromVersion_, toVersion_);
                    }

                    if (!vgRemap.has_value()) {
                        Base::buildResModel(this->resType, srcPath, fixedPath, modName, fileKey, ctx);
                        return;
                    }

                    // Pin each chain to whatever its ROOT maps to -- see
                    // WWMIFixerConfig::anchorChains. A root the row has no entry for is left alone
                    // rather than guessed at: that would write a target of 0, which is a real bone.
                    if (!anchorChains_.empty()) {
                        std::unordered_map<long long, long long> row = vgRemap->getRemap();
                        for (const auto& [root, members] : anchorChains_) {
                            const auto at = row.find(root);
                            if (at == row.end()) {
                                continue;
                            }

                            for (long long member : members) {
                                row[member] = at->second;
                            }
                        }

                        vgRemap->setRemap(std::move(row));
                    }

                    auto resource = std::make_unique<RemapBlendResource>(
                        ctx.iniFolder(), srcPath, fixedPath, std::move(*vgRemap), this->resType,
                        fixFunc_,
                        wwmiBlendElements(wwmiBlendInfluences(srcPath, vertices_, positionPath_)));
                    resource->logger = ctx.logger();
                    ctx.storeResource(fileKey, std::move(resource));
                }

            private:
                const ModType* modType_;
                std::optional<Version> fromVersion_;
                std::optional<Version> toVersion_;
                std::function<bool(RemapBlendResource&)> fixFunc_;
                std::map<long long, std::vector<long long>> anchorChains_;
                long long vertices_ = 0;
                std::string positionPath_;
        };


        /**
         * The blend of a mod from before WWMI's merged skeleton, remapped.
         *
         * Such a mod's `Blend.buf` holds each component's OWN bone indices (a draw could then only
         * address the bones the game hands it for that component), so each vertex is read as local to
         * the component that DRAWS it, lifted into the source's merged skeleton through that
         * component's `vg_map`, and only then sent through the library's row. Read as merged indices
         * instead, every bone of the body goes somewhere else -- the mod a noodle mess in game.
         *
         * 'drawRanges' is each component's (index count, first index) draws, over 'indexPath'.
         */
        // The remapped blend of a mod whose merged skeleton passes 256 bones: its true ids are
        // the 16-bit ones in BlendRemapVertexVG, and Blend.buf holds them TRUNCATED. The weights
        // come from Blend.buf unchanged; VertexVG holds every vertex, not only the remapped
        // components'.
        bool remapFromVertexVG(RemapBlendResource& resource, const std::string& vertexVGPath,
                               const std::string& positionPath) {
            // Both buffers through BufFile: integers, so decode-then-encode is exact and the
            // re-encode of the lines nothing touches cannot move them.
            std::error_code sizeErr;
            const std::uintmax_t blendSize =
                std::filesystem::file_size(FileService::strToPath(resource.srcPath), sizeErr);
            if (sizeErr || blendSize == 0) {
                return bail(resource, "its Blend.buf is empty or unreadable");
            }

            // The layout is derived, not assumed -- hardcoding it is what made the legacy lift
            // silently do nothing on these same mods.
            std::error_code err;
            const std::uintmax_t positionSize =
                std::filesystem::file_size(FileService::strToPath(positionPath), err);
            if (err || positionSize < WWMIPositionStride) {
                return bail(resource, "its Position.buf is missing or too short to give a vertex count");
            }

            const std::size_t vertices = static_cast<std::size_t>(positionSize / WWMIPositionStride);
            if (vertices == 0 || blendSize % vertices != 0) {
                return bail(resource, "its Blend.buf does not divide evenly by its vertex count");
            }

            const std::size_t stride = static_cast<std::size_t>(blendSize / vertices);   // N ids + N weights, a byte each
            if (stride < 2 || stride % 2 != 0) {
                return bail(resource, "its Blend.buf's bytes per vertex are not an even number of influences");
            }

            const std::size_t influences = stride / 2;

            std::vector<std::vector<unsigned long long>> trueIds;
            trueIds.reserve(vertices);
            const BufFile::Filter collect =
                [&trueIds, influences](const BufLineData& line, long long, double, long long) {
                    std::vector<unsigned long long> ids;
                    const auto at = line.find(WWMIBlendIndicesKey);
                    if (at != line.end()) {
                        for (const BufValue& value : at->second) {
                            ids.push_back(std::holds_alternative<unsigned long long>(value)
                                              ? std::get<unsigned long long>(value) : 0);
                        }
                    }

                    ids.resize(influences, 0);
                    trueIds.push_back(std::move(ids));
                    return line;
                };

            try {
                BufFile vgFile{vertexVGPath, wwmiVertexVGElements(influences)};
                if (!vgFile.isValid()) {
                    return bail(resource, "its BlendRemapVertexVG.buf could not be read");
                }

                vgFile.fix(std::nullopt, {collect});
            } catch (const std::exception& exception) {
                return bail(resource, std::string("its BlendRemapVertexVG.buf could not be read: ") + exception.what());
            }

            if (trueIds.size() != vertices) {
                return bail(resource, "its BlendRemapVertexVG.buf holds a different number of vertices than its Position.buf");
            }

            const std::unordered_map<long long, long long>& row = resource.vgRemap.getRemap();
            const BufFile::Filter remap =
                [&trueIds, &row, vertices, influences](const BufLineData& line, long long,
                                                       double index, long long) {
                    BufLineData out = line;
                    const auto vertex = static_cast<std::size_t>(index);
                    if (vertex >= vertices) {
                        return out;
                    }

                    const auto ids = out.find(WWMIBlendIndicesKey);
                    const auto weights = out.find(WWMIBlendWeightKey);
                    if (ids == out.end() || weights == out.end()) {
                        return out;
                    }

                    for (std::size_t b = 0; b < influences && b < ids->second.size()
                                            && b < weights->second.size(); ++b) {
                        if (!std::holds_alternative<unsigned long long>(weights->second[b])
                                || std::get<unsigned long long>(weights->second[b]) == 0) {
                            continue;                                 // a weight-zero slot
                        }

                        const auto target = row.find(static_cast<long long>(trueIds[vertex][b]));
                        if (target != row.end()) {
                            ids->second[b] = static_cast<unsigned long long>(target->second);
                        }
                    }

                    return out;
                };

            try {
                BufFile blend{resource.srcPath, wwmiBlendElements(influences)};
                if (!blend.isValid()) {
                    return bail(resource, "its Blend.buf could not be read");
                }

                blend.fix(resource.fixedPath, {remap});
            } catch (const std::exception& exception) {
                return bail(resource, std::string("its Blend.buf could not be read: ") + exception.what());
            }

            return true;
        }


        // ---- a target past 256 merged bones ------------------------------------------------
        //
        // Where the blend is remapped ONTO a character whose merged skeleton passes 256 slots, the
        // 8-bit ids of a WWMI Blend.buf cannot name the bones the row asks for -- so the fix has to
        // write WWMI's own blend remap, exactly as WWMI Tools writes one for such a character's own
        // mods (blender_export/data_models/data_model_wwmi.py's build_blend_remap, and this repo's
        // Tools/Misc/Prototypes/wwmiIdentityMod.py, whose output renders correctly in game).
        //
        // Four files come out of one pass, because they are four views of one computation:
        //   * the remapped blend      -- the mapped ids TRUNCATED to 8 bits, plus the mod's own
        //                                weights untouched. What a component with no remap reads.
        //   * ...BlendRemapVertexVG   -- every vertex's mapped ids at their full 16 bits
        //   * ...BlendRemapForward    -- 512 uint16 per remapped component, local -> merged
        //   * ...BlendRemapReverse    -- 512 per remap, merged -> local
        //
        // ONE remap for the whole mesh, shared by every component -- where WWMI Tools writes one
        // per component. Its scheme exists so that a mesh using more than 256 bones can still give
        // each component a set that fits; it needs each component's VERTEX SET, and the two answers
        // available here disagree on a real mod -- the section's declared window against the mod's
        // own toggled `drawindexed` ranges. Measured, on the three real ChisaParfait mods: 16433 of
        // one component's 83582 weighted slots landed on the wrong bone, and a mod with a component
        // it never draws shifted every later remap index past the end. The identity mod passed all
        // of it, being the easy case as ever.
        //
        // The union over the whole mesh has no such ambiguity, and it is bounded by the ROW rather
        // than hoped about: this pair's names 182 distinct targets against a remap's 256 entries,
        // and 173-182 are used across the four mods in hand. A union that does NOT fit is refused
        // rather than truncated -- see the check below.
        struct BlendRemapOut {
            std::string vertexVG;
            std::string forward;
            std::string reverse;
        };

        bool writeBlendRemap(RemapBlendResource& resource, const std::string& srcVertexVGPath,
                             const std::string& positionPath, const BlendRemapOut& out) {

            // The layout is derived, never assumed -- hardcoding it is what made the legacy lift
            // silently do nothing on these same mods, and what read an 8-influence line as two. The
            // sizes come first because they are what says how many influences a line has, which is
            // what the BufFile's elements are built from.
            std::error_code err;
            const std::uintmax_t positionSize =
                std::filesystem::file_size(FileService::strToPath(positionPath), err);
            const std::uintmax_t blendSize =
                std::filesystem::file_size(FileService::strToPath(resource.srcPath), err);
            if (err || positionSize < WWMIPositionStride || blendSize == 0) {
                return bail(resource, "its Position.buf or Blend.buf is missing, empty or too short");
            }

            const std::size_t vertices = static_cast<std::size_t>(positionSize / WWMIPositionStride);
            if (vertices == 0 || blendSize % vertices != 0) {
                return bail(resource, "its Blend.buf does not divide evenly by its vertex count");
            }

            const std::size_t stride = static_cast<std::size_t>(blendSize / vertices);   // N ids + N weights, a byte each
            if (stride < 2 || stride % 2 != 0) {
                return bail(resource, "its Blend.buf's bytes per vertex are not an even number of influences");
            }

            const std::size_t influences = stride / 2;

            // Both buffers through BufFile: integers, so decode-then-encode is exact and the
            // re-encode of the lines nothing touches cannot move them.
            std::vector<std::vector<unsigned long long>> ids;        // per vertex, `influences` of them
            std::vector<std::vector<unsigned long long>> weights;
            ids.reserve(vertices);
            weights.reserve(vertices);

            const BufFile::Filter readBlend =
                [&ids, &weights, influences](const BufLineData& line, long long, double, long long) {
                    std::vector<unsigned long long> lineIds;
                    std::vector<unsigned long long> lineWeights;

                    const auto atIds = line.find(WWMIBlendIndicesKey);
                    if (atIds != line.end()) {
                        for (const BufValue& value : atIds->second) {
                            lineIds.push_back(std::holds_alternative<unsigned long long>(value)
                                                  ? std::get<unsigned long long>(value) : 0);
                        }
                    }

                    const auto atWeights = line.find(WWMIBlendWeightKey);
                    if (atWeights != line.end()) {
                        for (const BufValue& value : atWeights->second) {
                            lineWeights.push_back(std::holds_alternative<unsigned long long>(value)
                                                      ? std::get<unsigned long long>(value) : 0);
                        }
                    }

                    lineIds.resize(influences, 0);
                    lineWeights.resize(influences, 0);
                    ids.push_back(std::move(lineIds));
                    weights.push_back(std::move(lineWeights));
                    return line;
                };

            try {
                BufFile blendFile{resource.srcPath, wwmiBlendElements(influences)};
                if (!blendFile.isValid()) {
                    return bail(resource, "its Blend.buf could not be read");
                }

                blendFile.fix(std::nullopt, {readBlend});
            } catch (const std::exception& exception) {
                return bail(resource, std::string("its Blend.buf could not be read: ") + exception.what());
            }

            if (ids.size() != vertices) {
                return bail(resource, "its Blend.buf holds a different number of vertices than its Position.buf");
            }

            // The TRUE ids: the mod's own 16-bit ones when it carries a blend remap of its own,
            // otherwise Blend.buf's, which are the whole truth for a source under 256 bones.
            std::vector<std::uint16_t> trueIds(vertices * influences, 0);
            bool haveVertexVG = false;
            if (!srcVertexVGPath.empty()) {
                std::size_t at = 0;
                const BufFile::Filter readVG =
                    [&trueIds, &at, influences](const BufLineData& line, long long, double, long long) {
                        const auto found = line.find(WWMIBlendIndicesKey);
                        if (found != line.end()) {
                            for (std::size_t b = 0; b < influences && b < found->second.size(); ++b) {
                                if (at + b < trueIds.size()
                                        && std::holds_alternative<unsigned long long>(found->second[b])) {
                                    trueIds[at + b] =
                                        static_cast<std::uint16_t>(std::get<unsigned long long>(found->second[b]));
                                }
                            }
                        }

                        at += influences;
                        return line;
                    };

                try {
                    BufFile vgFile{srcVertexVGPath, wwmiVertexVGElements(influences)};
                    if (vgFile.isValid()) {
                        vgFile.fix(std::nullopt, {readVG});
                        haveVertexVG = at == trueIds.size();
                    }
                } catch (const std::exception&) {
                    haveVertexVG = false;                  // its own ids are optional; Blend.buf's stand
                }
            }

            if (!haveVertexVG) {
                for (std::size_t vertex = 0; vertex < vertices; ++vertex) {
                    for (std::size_t b = 0; b < influences; ++b) {
                        trueIds[vertex * influences + b] =
                            static_cast<std::uint16_t>(ids[vertex][b]);
                    }
                }
            }

            // ---- map every id through the library's row --------------------------------------
            const std::unordered_map<long long, long long>& row = resource.vgRemap.getRemap();
            std::vector<std::uint16_t> mapped(trueIds);
            for (std::size_t vertex = 0; vertex < vertices; ++vertex) {
                for (std::size_t b = 0; b < influences; ++b) {
                    const std::size_t at = vertex * influences + b;
                    if (weights[vertex][b] == 0) {
                        continue;                                     // a weight-zero slot
                    }

                    const auto target = row.find(static_cast<long long>(trueIds[at]));
                    if (target != row.end() && target->second >= 0
                            && static_cast<std::size_t>(target->second) < WWMIBlendRemapSize) {
                        mapped[at] = static_cast<std::uint16_t>(target->second);
                    }
                }
            }

            // ---- the blend, with the mapped ids truncated -------------------------------------
            // Truncated on purpose: Blend.buf holds a byte per id, and the merged index that does not
            // fit is what BlendRemapVertexVG.buf below carries in full.
            const BufFile::Filter writeIds =
                [&mapped, vertices, influences](const BufLineData& line, long long, double index, long long) {
                    BufLineData out = line;
                    const auto vertex = static_cast<std::size_t>(index);
                    const auto found = out.find(WWMIBlendIndicesKey);
                    if (vertex >= vertices || found == out.end()) {
                        return out;
                    }

                    for (std::size_t b = 0; b < influences && b < found->second.size(); ++b) {
                        found->second[b] = static_cast<unsigned long long>(
                            mapped[vertex * influences + b] & 0xFF);
                    }

                    return out;
                };

            try {
                BufFile blendOut{resource.srcPath, wwmiBlendElements(influences)};
                if (!blendOut.isValid()) {
                    return bail(resource, "its Blend.buf could not be rewritten");
                }

                blendOut.fix(resource.fixedPath, {writeIds});
            } catch (const std::exception& exception) {
                return bail(resource, std::string("its Blend.buf could not be rewritten: ") + exception.what());
            }

            const auto write = [&resource](const std::string& path, const void* data, std::size_t bytes) {
                std::error_code dirErr;
                std::filesystem::create_directories(
                    FileService::strToPath(path).parent_path(), dirErr);
                std::ofstream file(FileService::strToPath(path), std::ios::binary);
                if (!file.is_open()) {
                    return bail(resource, "could not open " + path + " to write");
                }

                file.write(reinterpret_cast<const char*>(data), static_cast<std::streamsize>(bytes));
                return true;
            };

            if (!write(out.vertexVG, mapped.data(), mapped.size() * 2)) {
                return bail(resource, "its BlendRemapVertexVG.buf could not be written");
            }

            // ---- the one map, over the ROW's distinct targets ----------------------------------
            // Not over the bones this mod happens to weight: the .ini naming the remap's bone count
            // is written BEFORE this runs, so the two have to derive it from the same thing, and the
            // row is the only thing neither can change. An entry no vertex reaches costs two bytes.
            std::set<std::uint16_t> used;
            for (const auto& [srcBone, dstBone] : row) {
                (void)srcBone;
                if (dstBone >= 0 && static_cast<std::size_t>(dstBone) < WWMIBlendRemapSize) {
                    used.insert(static_cast<std::uint16_t>(dstBone));
                }
            }

            if (used.size() > WWMIBlendRemapSize) {
                // Refused rather than truncated: a silently short remap sends every bone past the
                // cut to local 0, which is a limb pinned to the root and nothing in the output to
                // say so. Cannot happen for a pair whose row names fewer distinct targets than a
                // remap holds, which is checked when the row is read.
                //
                // ...and the refusal said nothing either, until 2026-09-29. This one is a fault in
                // the LIBRARY's vertex group row rather than in the mod, so it would otherwise be
                // undiagnosable from the outside: the part simply does not render.
                return bail(resource, "the vertex group row names " + std::to_string(used.size())
                                      + " distinct target bones, more than the "
                                      + std::to_string(WWMIBlendRemapSize) + " a WWMI blend remap holds");
            }

            std::vector<std::uint16_t> forward(WWMIBlendRemapSize, 0);
            std::vector<std::uint16_t> reverse(WWMIBlendRemapSize, 0);
            std::uint16_t local = 0;
            for (const std::uint16_t merged : used) {
                forward[local] = merged;
                reverse[merged] = local;
                ++local;
            }

            return write(out.forward, forward.data(), forward.size() * 2)
                   && write(out.reverse, reverse.data(), reverse.size() * 2);
        }


        bool liftLegacyBlend(RemapBlendResource& resource, const std::string& indexPath,
                             const std::string& positionPath,
                             const std::map<int, std::vector<std::pair<long long, long long>>>& drawRanges,
                             const std::map<int, std::vector<int>>& vgMaps) {
            // The index buffer through IbFile: it decodes the triangles, and a flat list of its
            // vertex ids is those triples in order -- which is what a draw range indexes into.
            std::vector<std::uint32_t> indexList;
            try {
                IbFile indices{indexPath};
                const BufFile::Filter collect =
                    [&indexList](const BufLineData& line, long long, double, long long) {
                        const auto at = line.find(IbFile::TriangleBufElementKey);
                        if (at != line.end()) {
                            for (const BufValue& value : at->second) {
                                if (std::holds_alternative<unsigned long long>(value)) {
                                    indexList.push_back(static_cast<std::uint32_t>(
                                        std::get<unsigned long long>(value)));
                                }
                            }
                        }

                        return line;
                    };

                indices.fix(std::nullopt, {collect});
            } catch (const std::exception& exception) {
                return bail(resource, std::string("its index buffer could not be read: ") + exception.what());
            }
            // THE LAYOUT IS DERIVED, NOT ASSUMED. WWMIBlendStride is four R8 ids then four R8
            // weights, which is one WWMI layout and not the only one: Chisa's mods carry EIGHT of
            // each. Reading a 16-byte vertex as two 8-byte ones gives twice the vertex count, so
            // componentOf is indexed by a vertex id that means nothing, most vertices get no
            // component at all and their ids pass through unlifted -- local id 5 of the skirt read
            // as merged bone 5, the jumbled mesh this function exists to prevent.
            std::error_code sizeErr;
            const std::uintmax_t positionSize =
                std::filesystem::file_size(FileService::strToPath(positionPath), sizeErr);
            if (sizeErr || positionSize < WWMIPositionStride) {
                return bail(resource, "its Position.buf is missing or too short to give a vertex count");
            }

            const std::size_t vertices = static_cast<std::size_t>(positionSize / WWMIPositionStride);
            const std::uintmax_t blendSize =
                std::filesystem::file_size(FileService::strToPath(resource.srcPath), sizeErr);
            if (sizeErr || vertices == 0 || blendSize == 0 || blendSize % vertices != 0) {
                return bail(resource, "its Blend.buf does not divide evenly by the vertex count its Position.buf gives");
            }

            const std::size_t stride = static_cast<std::size_t>(blendSize / vertices);
            if (stride < 2 || stride % 2 != 0) {
                return bail(resource, "its Blend.buf's bytes per vertex are not an even number of influences");
            }

            // The same answer `wwmiBlendInfluences` gives, from the same two file sizes -- asked of
            // it rather than restated. Its FALLBACK of 4 is the one thing not shared: it exists so a
            // failed derivation cannot move a shipped character's output, and here a failed
            // derivation must lift nothing at all, which is what the guards above already decided.
            const std::size_t influences = wwmiBlendInfluences(resource.srcPath,
                                                              static_cast<long long>(vertices), positionPath);
            if (influences != stride / 2) {
                return bail(resource, "its Blend.buf's influences per vertex disagree with its byte stride");
            }

            const std::size_t indexCount = indexList.size();

            // which component draws each vertex
            std::vector<int> componentOf(vertices, -1);
            for (const auto& entry : drawRanges) {
                for (const auto& range : entry.second) {
                    for (long long k = range.second; k < range.second + range.first && k >= 0; ++k) {
                        if (static_cast<std::size_t>(k) >= indexCount) {
                            break;
                        }

                        const std::uint32_t vertex = indexList[static_cast<std::size_t>(k)];
                        if (vertex < vertices) {
                            componentOf[vertex] = entry.first;
                        }
                    }
                }
            }

            // A blend line is `wwmiBlendElements`' two elements -- `influences` ids then `influences`
            // weights, one unsigned byte each -- so BufFile hands the filter the ids and the weights
            // already separated, and its decode/encode of an integer is exact. That last part is
            // what makes this refactor provable where the texcoord one is not.
            BufFile blend{resource.srcPath, wwmiBlendElements(influences)};
            if (!blend.isValid()) {
                return bail(resource, "its Blend.buf could not be read for the legacy lift");
            }

            const std::unordered_map<long long, long long>& row = resource.vgRemap.getRemap();
            const BufFile::Filter lift =
                [&componentOf, &vgMaps, &row, vertices, influences](const BufLineData& line, long long,
                                                                    double index, long long) {
                    BufLineData out = line;
                    const auto vertex = static_cast<std::size_t>(index);
                    if (vertex >= vertices) {
                        return out;
                    }

                    const int component = componentOf[vertex];
                    auto vgMap = vgMaps.find(component);
                    if (component < 0 || vgMap == vgMaps.end()) {
                        return out;
                    }

                    const auto ids = out.find(WWMIBlendIndicesKey);
                    const auto weights = out.find(WWMIBlendWeightKey);
                    if (ids == out.end() || weights == out.end()) {
                        return out;
                    }

                    for (std::size_t b = 0; b < influences && b < ids->second.size()
                                            && b < weights->second.size(); ++b) {
                        if (!std::holds_alternative<unsigned long long>(weights->second[b])
                                || std::get<unsigned long long>(weights->second[b]) == 0) {
                            continue;                   // a weight-zero slot: the library leaves those alone too
                        }

                        if (!std::holds_alternative<unsigned long long>(ids->second[b])) {
                            continue;
                        }

                        const auto local = static_cast<std::size_t>(
                            std::get<unsigned long long>(ids->second[b]));
                        if (local >= vgMap->second.size()) {
                            continue;
                        }

                        auto target = row.find(vgMap->second[local]);
                        if (target == row.end()) {
                            continue;
                        }

                        ids->second[b] = static_cast<unsigned long long>(target->second);
                    }

                    return out;
                };

            try {
                blend.fix(resource.fixedPath, {lift});
            } catch (const std::exception& exception) {
                return bail(resource, std::string("its lifted Blend.buf could not be written: ")
                                      + exception.what());
            }

            return true;
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
        // `WWMITextureRoles::Role` and `TextureIndex` MOVED to data/WWMITextureFacts.h (2026-09-29), with the
        // constants and the `Component<N>_<Type>` table they read. Sorting a mod's files into roles
        // is CLASSIFICATION -- it reads only the SOURCE's own textures and never the target, the
        // plan or the fix -- so it belongs where a parser can reach it too, which is the point of
        // the header. The fixer still constructs it here; moving WHEN it runs is the next step.


        // ---- the fixer ----

        class WWMIFixerImpl: public Fixer {
            // DECLARED FIRST, and it has to be: everything Z3 hands out belongs to this
            // context and must not outlive it, which member destruction order decides.
            // ModBranches carries the same note about its own.
            Z3Context z3_;

            public:
                WWMIFixerImpl(BaseIniParser<>* parser, const std::string& toModName, std::optional<int> modTypeId,
                              WWMIFixerConfig config):
                    Fixer(parser, nullptr, {},
                          toModName.empty() ? std::optional<std::vector<std::string>>(std::nullopt)
                                            : std::optional<std::vector<std::string>>({toModName}),
                          nullptr, makeConfig()),
                    ctx_(parser != nullptr ? parser->getIniFile() : nullptr, modTypeId), toModName_(toModName),
                    config_(std::move(config)) {
                    this->setCtx(&ctx_);
                    this->copyPreamble = config_.copyPreamble;

                    std::string error;
                    if (!readCharacters(error) || !readMod(error)) {
                        if (!error.empty()) {
                            ctx_.log(error + " -- so this .ini is left alone");
                        }

                        giveUp();
                        return;
                    }

                    readTextures();
                    planTexEdits();
                    buildEdits();
                    buildAppended();

                    this->graphGroupEdits.clear();
                    this->graphGroupEdits.push_back(slotRemap_.get());
                    this->graphGroupEdits.push_back(mainEdits_.get());
                    for (auto& collect : blendCollects_) {
                        this->graphGroupEdits.push_back(collect.get());
                    }

                    // The copies are the prototype's shape: each carries the fix's own sections (the
                    // texture command lists, the shader tags, the resources) and none of the mod's
                    // own draw sections live -- the mod's own file keeps serving the mod on its
                    // source character, and a copy exists only for one more claimant of a draw.
                    this->appendedSectionsInCopies = true;
                    for (const auto& entry : ctx_.getIniFile()->getIfTemplates()) {
                        if (StringTools::startsWith(entry.first, IniKeywords::TextureOverride)) {
                            this->copyHiddenSectionNames.insert(entry.first);
                        }
                    }
                }

            protected:
                // GIMIFixer's own hands every group edit a nullptr .ini file, which stops a collect
                // ever building anything -- see GIMICharFixerImpl. The files the fix writes outside
                // the resource system (the zero stream) and the resources it adds by hand (the
                // created textures) go in here too, at fix time, so a parse alone writes nothing.
                void applyGraphGroupEdits(const std::string& modName) override {
                    if (this->graphGroups() == nullptr) {
                        return;
                    }

                    if (!gaveUp_) {
                        writeZeroStream();
                        addCreatedTextures();
                        addFallbackDownloads();
                        addTexEdits();
                    }

                    for (Fixer::GroupEdit* edit : this->graphGroupEdits) {
                        if (edit != nullptr) {
                            edit->editFromIni(*this->graphGroups(), ctx_.getIniFile(), nullptr, modName);
                        }
                    }

                    rebindBlendOverride();
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
                // The context's log is not const where the callers are, so this is the one place
                // that casts it away.
                void note(const std::string& message) const {
                    const_cast<IniFileFixContext&>(ctx_).log(message);
                }

                void rebindBlendOverride() {
                    if (gaveUp_ || config_.blendReg.empty() || this->graphGroups() == nullptr) {
                        return;
                    }

                    Fixer::GraphGroups& groups = *this->graphGroups();
                    for (std::size_t g = 0; g < groups.size(); ++g) {
                        // The remapped blend as the GRAPH spells it, which is what the collect wrote
                        // -- read rather than rebuilt, so the two cannot drift apart.
                        std::string remapped;
                        forEachPart(groups, g, [&](ContentPart& part) {
                            if (!remapped.empty()) {
                                return;
                            }

                            for (const std::string& value : part.getVals(config_.blendReg)) {
                                if (!IniNamingTools::hasRefPrefix(value)
                                        && value.find(IniKeywords::Remap) != std::string::npos) {
                                    remapped = std::string(StringTools::strip(value));
                                    break;
                                }
                            }
                        });

                        if (remapped.empty()) {
                            continue;                   // nothing remapped this group's blend
                        }

                        RegNewVals<> rebind({{config_.blendReg, RegNewVals<>::NewVal(
                            RegNewVals<>::OldValProducer(
                                [&remapped](const std::string& oldValue, const ModType*) {
                                    return IniNamingTools::hasRefPrefix(oldValue) ? remapped : oldValue;
                                }))}});

                        forEachPart(groups, g, [&](ContentPart& part) {
                            rebind.edit(part, "");
                        });
                    }
                }

                using ContentPart = IfContentPart<std::string, std::string>;

                // Every content part of every section of every graph in one group.
                template <typename Fn>
                void forEachPart(Fixer::GraphGroups& groups, std::size_t groupInd, Fn&& fn) const {
                    for (const ModObj& obj : groups.modObjs(groupInd)) {
                        auto* graph = groups.getGraph(groupInd, obj);
                        if (graph == nullptr) {
                            continue;
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
                using FixerConfig = Fixer::FixerConfig;

                static FixerConfig makeConfig() {
                    FixerConfig config{};
                    config.sectionToStr = &renderIfTemplate;
                    return config;
                }

                // Gives up: the fixer writes NOTHING (see GraphGroupRemove and GIMIComponentFixerImpl).
                void giveUp() {
                    gaveUp_ = true;
                    this->graphGroupEdits = {&removeEveryGroup_};
                }

                std::string fixName(const std::string& name) const {
                    return IniNamingTools::getRemapFixName(name, toModName_);
                }

                ModObj slotObj(int component) const {
                    return ModObj("", config_.slotPrefix + std::to_string(component));
                }

                std::optional<Version> fromVersion() const {
                    std::optional<Version> version = ctx_.version();
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
                    return true;
                }

                // ---- what the mod's .ini has: its slot sections, its vertex count, its folders ----

                bool readMod(std::string& error) {
                    IniFile* ini = ctx_.getIniFile();
                    if (ini == nullptr) {
                        error = "the fixer has no .ini file";
                        return false;
                    }

                    // The fixer is built BEFORE the parser parses, so the sections are found by hash
                    // over IniFile::getIfTemplates rather than through the parser's graphs.
                    const auto& templates = ini->getIfTemplates();
                    for (const auto& entry : templates) {
                        if (entry.second == nullptr) {
                            continue;
                        }

                        const IfTemplate<std::string, std::string>& tpl = *entry.second;
                        std::optional<std::string> hash = ModBranches::firstVal(tpl, IniKeywords::Hash);
                        std::optional<std::string> index = ModBranches::firstVal(tpl, IniKeywords::MatchFirstIndex);
                        if (!hash.has_value() || !index.has_value() || StringTools::toLower(*hash) != source_.vb0Hash) {
                            continue;
                        }

                        for (std::size_t i = 0; i < source_.slots.size(); ++i) {
                            if (source_.slots[i].indexOffset == *index) {
                                present_[static_cast<int>(i)].push_back(entry.first);
                                std::size_t draws = 0;
                                for (const auto& part : tpl.parts()) {
                                    (void)part;
                                }

                                (void)draws;
                            }
                        }
                    }

                    if (present_.empty()) {
                        error = "no [TextureOverrideComponent*] section on " + source_.name + "'s hash " + source_.vb0Hash;
                        return false;
                    }

                    // A mod from before WWMI's merged skeleton: no section of it carries a vg_offset,
                    // because a draw could then only address its own component's bones -- so its blend
                    // holds each component's OWN indices. See WWMIFixerConfig::sourceVgMaps.
                    // Its own draw ranges, per source component. Read for EVERY mod, not only a
                    // legacy one: this began as the per-component lift of a legacy blend, and two
                    // later readers want the same ranges for reasons that have nothing to do with the
                    // blend layout -- the remap of a toggled draw through the split, and
                    // TexEditContext::drawRanges, which is how a texture edit knows what a component
                    // covers. Left inside `if (legacy_)` the map was empty on every mod past 256
                    // bones, so the ribbon's colour-grade island found no geometry and the edit
                    // graded nothing while still writing its file and reporting a success.
                    for (const auto& entry : present_) {
                        for (const std::string& section : entry.second) {
                            auto tpl = templates.find(section);
                            if (tpl == templates.end() || tpl->second == nullptr) {
                                continue;
                            }

                            for (const auto& part : tpl->second->parts()) {
                                const auto* content = dynamic_cast<const IfTemplate<std::string, std::string>::ContentPart*>(part.get());
                                if (content == nullptr) {
                                    continue;
                                }

                                for (const std::string& draw : content->getVals(IniKeywords::DrawIndexed)) {
                                    const std::size_t comma = draw.find(',');
                                    const std::size_t second = draw.find(',', comma + 1);
                                    if (comma == std::string::npos || second == std::string::npos) {
                                        continue;
                                    }

                                    try {
                                        drawRanges_[entry.first].emplace_back(
                                            std::stoll(std::string(StringTools::strip(draw.substr(0, comma)))),
                                            std::stoll(std::string(StringTools::strip(draw.substr(comma + 1, second - comma - 1)))));
                                    } catch (const std::exception&) {
                                        // Draw ranges decide which triangles of a toggled object are
                                        // drawn, so one dropped in silence is a piece of the mesh
                                        // that stops being remapped for no stated reason.
                                        note("could not read the draw range `"
                                             + std::string(StringTools::strip(draw)) + "` in ["
                                             + section + "], so it is not remapped");
                                        continue;
                                    }
                                }
                            }
                        }
                    }

                    legacy_ = true;
                    for (const auto& entry : present_) {
                        for (const std::string& section : entry.second) {
                            auto tpl = templates.find(section);
                            if (tpl != templates.end() && tpl->second != nullptr
                                && ModBranches::firstVal(*tpl->second, VgOffsetKey).has_value()) {
                                legacy_ = false;
                            }
                        }
                    }

                    if (legacy_) {
                        if (config_.sourceVgMaps.empty()) {
                            error = "this mod is from before WWMI's merged skeleton (no `" + VgOffsetKey + "` in any of its sections), "
                                    "so its blend holds each component's OWN bone indices -- and " + source_.name
                                    + "'s config carries no sourceVgMaps to lift them with. Remapping them as merged indices would "
                                      "scramble every bone of the body, so this mod is left alone";
                            return false;
                        }

                        note("this mod is from before WWMI's merged skeleton: its blend is read per component and lifted "
                             "through " + source_.name + "'s vg_map, and the fix supplies the merged skeleton it lacks");
                    }

                    auto constants = templates.find(ConstantsSection);
                    if (constants != templates.end() && constants->second != nullptr) {
                        std::optional<std::string> count = ModBranches::firstVal(*constants->second, MeshVertexCountKey);
                        if (count.has_value()) {
                            try {
                                meshVertexCount_ = std::stoll(*count);
                            } catch (const std::exception&) {
                                // 0 is not a harmless fallback: it turns the zero shape-key stream
                                // OFF (writeZeroStream returns on <= 0), which is the wavy-vertices
                                // fault that stream exists to remove.
                                note("this mod's " + MeshVertexCountKey + " is `" + *count
                                     + "`, which is not a number, so the zero shape-key stream is"
                                     + " not written");
                                meshVertexCount_ = 0;
                            }
                        }
                    }

                    auto index = templates.find(IndexBufferResource);
                    if (index != templates.end() && index->second != nullptr) {
                        std::optional<std::string> file = ModBranches::firstVal(*index->second, IniKeywords::Filename);
                        if (file.has_value()) {
                            indexFile_ = *file;
                        }
                    }

                    // ...and the position and texcoord buffers, off the .ini for the same reason the
                    // index buffer is: a mod-manager-packaged mod names every file by GUID with a
                    // `.assets` extension, so there is no `Position.buf` beside anything. Built as a
                    // sibling name, the path did not exist, the remapped blend was never written, and
                    // the .ini still bound vb4 to it -- which drew NOTHING of the character.
                    //
                    // WITHOUT REGARD TO CASE: one mod spells it ResourceTexCoordBuffer.
                    for (const auto& entry : templates) {
                        if (entry.second == nullptr) {
                            continue;
                        }

                        std::string* into = nullptr;
                        if (StringTools::equalsIgnoreCase(entry.first, PositionBufferResource)) {
                            into = &positionFile_;
                        } else if (StringTools::equalsIgnoreCase(entry.first, TexcoordBufferResource)) {
                            into = &texcoordFile_;
                        }

                        if (into == nullptr) {
                            continue;
                        }

                        std::optional<std::string> file = ModBranches::firstVal(*entry.second, IniKeywords::Filename);
                        if (file.has_value()) {
                            *into = *file;
                        }
                    }

                    // WHERE the 16-bit ids live, off the .ini rather than a sibling filename: a
                    // mod-manager-packaged mod names every file by GUID with a `.assets` extension,
                    // so the WWMI export's own name is not beside Blend.buf and the search finds
                    // nothing -- and the fallback that then runs is a no-op dressed as a fix.
                    for (const auto& entry : templates) {
                        if (entry.second == nullptr) {
                            continue;
                        }

                        std::string name = StringTools::toLower(entry.first);
                        name.erase(std::remove(name.begin(), name.end(), '_'), name.end());
                        if (name.find("blendremapvertexvg") == std::string::npos) {
                            continue;
                        }

                        std::optional<std::string> file = ModBranches::firstVal(*entry.second, IniKeywords::Filename);
                        if (file.has_value()) {
                            vertexVGFile_ = *file;
                            break;
                        }
                    }

                    meshFolder_ = DefaultMeshFolder;
                    auto blend = templates.find(BlendBufferResource);
                    if (blend != templates.end() && blend->second != nullptr) {
                        std::optional<std::string> file = ModBranches::firstVal(*blend->second, IniKeywords::Filename);
                        if (file.has_value()) {
                            const std::string forward = FileService::iniPathToRel(*file);
                            const std::size_t slash = forward.rfind('/');
                            if (slash != std::string::npos && slash > 0) {
                                meshFolder_ = forward.substr(0, slash);
                            }

                            // Kept whole, because the name of the blend the FIX writes is derived
                            // from it and not from the character -- see blendFixedFile().
                            blendSourceFile_ = *file;

                            // The mod's OWN blend line, derived from its own file rather than the
                            // `stride` it declares: a declaration can disagree with the bytes, and
                            // this is the number WWMI's BlendRemapper is handed.
                            const std::string blendPath =
                                FileService::absPathOfRelPath(forward, ctx_.getIniFile()->getFolder());
                            const std::string positionPath =
                                FileService::absPathOfRelPath(positionFile_, ctx_.getIniFile()->getFolder());
                            blendInfluences_ = wwmiBlendInfluences(blendPath, meshVertexCount_, positionPath);
                            blendStride_ = blendInfluences_ * 2;
                        }
                    }

                    return true;
                }

                // ---- the textures: the run's index, and what this .ini binds of it ----

                // WHICH of the mod's own resources it binds in SEVERAL VARIANTS. A toggle mod writes `if $hair == 0 / this = X / else / this = XA` in its
                // [TextureOverrideTexture*], and a single `<reg> = <resource>` line in the fix can
                // only ever carry one of those -- so the mod's texture toggle stopped working while
                // the toggled `drawindexed` in its own section kept switching the geometry.
                //
                // A section that binds unconditionally with one value is not recorded, and keeps its
                // direct register line: every Sanhua mod is that shape, and this must not move output
                // that has been verified in game.
                void readConditionalBindings() {
                    IniFile* ini = ctx_.getIniFile();
                    if (ini == nullptr) {
                        return;
                    }

                    for (const auto& entry : ini->getIfTemplates()) {
                        if (entry.second == nullptr
                            || !StringTools::startsWith(entry.first, (IniKeywords::TextureOverride + "Texture"))) {
                            continue;
                        }

                        std::vector<std::string> bound;
                        for (const auto& part : entry.second->parts()) {
                            const auto* content =
                                dynamic_cast<const IfTemplate<std::string, std::string>::ContentPart*>(part.get());
                            if (content == nullptr) {
                                continue;
                            }

                            for (const std::string& val : content->getVals(IniKeywords::This)) {
                                bound.push_back(std::string(StringTools::strip(val)));
                            }
                        }

                        // SEVERAL resources, which is what a toggle is. A section binding ONE
                        // resource keeps its direct register line even when it sits under an `if`:
                        // every Sanhua mod wraps its one `this` in `if $object_detected`, which the
                        // remapped section has already set to 1 by the time the list runs, so a copy
                        // of it would be the same binding in more lines -- and moving output that has
                        // been verified in game to say the same thing is not worth it.
                        //
                        // The gap that leaves: one `this` under a REAL condition and no else (`if
                        // $hair == 0 / this = X / endif`) still binds X unconditionally. That is what
                        // the fix did before this, so nothing regresses -- but a mod that renders
                        // wrong on one toggle with a single-variant role is where to look next.
                        if (bound.size() < 2) {
                            continue;
                        }

                        // TWO SECTIONS MAY BIND ONE RESOURCE BEHIND A TOGGLE, AND ONLY ONE OF THEM
                        // MAY BE WHOLE (2026-09-28). SanhuaExorcist4 selects `7 / 7a / 7b` in
                        // `[TextureOverrideTexture7]` and `7 / 7.1 / 7.2` in
                        // `[TextureOverrideTexture7_injured]`, where the author typed names the mod
                        // never declares. Last-writer-wins gave the copy the typo'd one -- one live
                        // branch of three, so on the other two values the TARGET's own diffuse stayed
                        // bound at the mod's UVs. Count the branches that name a section this .ini
                        // has, and keep the section that answers for most of its own variants.
                        const auto& templates = ini->getIfTemplates();
                        const auto resolves = [&templates](const std::vector<std::string>& vals) {
                            std::size_t n = 0;
                            for (const std::string& val : vals) {
                                if (templates.count(val) > 0) {
                                    ++n;
                                }
                            }

                            return n;
                        };

                        const std::size_t score = resolves(bound);
                        for (const std::string& resource : bound) {
                            const std::string key = StringTools::toLower(resource);
                            const auto owner = conditionalOwner_.find(key);
                            if (owner != conditionalOwner_.end()) {
                                const auto held = variantsOf_.find(owner->second);
                                if (held != variantsOf_.end() && resolves(held->second) >= score) {
                                    continue;
                                }
                            }

                            conditionalOwner_[key] = entry.first;
                        }

                        variantsOf_[entry.first] = bound;
                    }
                }

                void readTextures() {
                    IniFile* ini = ctx_.getIniFile();
                    const std::string iniFolder = ini->getFolder();
                    const std::string iniPath = ini->getFile().value_or("");
                    readConditionalBindings();
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
                    const WWMITextureRoles* textureRoles = nullptr;
                    if (const auto* facts = dynamic_cast<const WWMIParseFacts*>(this->getParser())) {
                        textureRoles = facts->textureRoles();
                    }

                    if (textureRoles == nullptr) {
                        // Every WuWa character's parse row carries its textures, so this is a
                        // configuration error rather than a mod's doing -- and a fix that cannot
                        // tell what any texture is would bind the game's over all of them.
                        ctx_.log("this mod's textures were not identified: the character's parse row "
                                 "carries no textures, so every role would fall back to a download");
                        giveUp();
                        return;
                    }

                    std::unordered_map<std::string, std::string> resourceOfFile;
                    for (const auto& entry : textureRoles->resourcesOf(iniPath)) {
                        resourceOfFile.emplace(entry.second, entry.first);
                        // ...and the other way, so an edit can reach EVERY variant a toggled role
                        // binds rather than only the one fileOfRole_ resolved to
                        fileOfResource_.emplace(StringTools::toLower(entry.first), entry.second);
                    }

                    // WHERE THE FIX WRITES ITS OWN TEXTURES: the folder MOST of the mod's textures
                    // are already in, and never an absolute one.
                    //
                    // This used to read `resourceOfFile.begin()`, which on an unordered_map is
                    // whichever entry the table hashed first -- so a mod whose textures are not all
                    // in one folder got a folder picked at random, and the pick could move between
                    // runs. That is what made Chisa13's mod.ini come out with four different hashes
                    // over four corpus sweeps while six isolated runs agreed.
                    //
                    // And the folder-scanning index this replaced keyed everything by a
                    // LOWERCASED path and answered with its argument when the key was absent, so an
                    // absolute path came back lowercased -- which `getRelPath` cannot relativise
                    // against a real-cased `iniFolder`. The folder then came out absolute and every
                    // download, edit and created texture was written into the mod's .ini as
                    // `c:/users/.../textures/...`: broken the moment the mod moves, and the fixing
                    // machine's paths in someone else's file. A path that is not relative is
                    // refused, which leaves `Textures` -- what a mod with no textures of its own
                    // already gets (2026-09-29).
                    textureFolder_ = DefaultTextureFolder;
                    std::map<std::string, std::size_t> folderCounts;
                    for (const auto& entry : resourceOfFile) {
                        const std::string rel =
                            FileService::iniPathToRel(FileService::getRelPath(entry.first, iniFolder));
                        if (FileService::strToPath(rel).is_absolute()) {
                            continue;                   // the index could not place it: not a folder of this mod
                        }

                        const std::size_t slash = rel.rfind('/');
                        if (slash != std::string::npos && slash > 0) {
                            ++folderCounts[rel.substr(0, slash)];
                        }
                    }

                    // most textures wins; a tie goes to the lowest path, so the answer is the mod's
                    // rather than an iteration order's
                    std::size_t best = 0;
                    for (const auto& entry : folderCounts) {
                        if (entry.second > best) {
                            best = entry.second;
                            textureFolder_ = entry.first;
                        }
                    }

                    // AGREMAP_WWMI_ROLES=1: one line per file per role, so the identification
                    // this does can be diffed against the one the parser is taking over. Off by
                    // default; the compiled fixer is silent by request.
                    if (std::getenv("AGREMAP_WWMI_ROLES") != nullptr) {
                        for (const auto& entry : textureRoles->rolesOf()) {
                            for (const WWMITextureRoles::Role& role : entry.second) {
                                std::fprintf(stderr, "WWMIROLE\t%s\t%s\t%s\t%s\n", iniPath.c_str(),
                                             FileService::pathKey(entry.first).c_str(),
                                             role.role.c_str(), role.how.c_str());
                            }
                        }
                    }

                    // ...and what the PARSER decided, by hash + register, for the same mod. Both
                    // under one env var so the two can be diffed exactly rather than approximated.
                    if (std::getenv("AGREMAP_WWMI_ROLES") != nullptr) {
                        if (const auto* facts = dynamic_cast<const WWMIParseFacts*>(this->getParser())) {
                            if (const WWMITextureRoles* roles = facts->textureRoles()) {
                                for (const auto& entry : roles->rolesOf()) {
                                    for (const auto& role : entry.second) {
                                        std::fprintf(stderr, "WWMIROLE2\t%s\t%s\t%s\t%s\n", iniPath.c_str(),
                                                     FileService::pathKey(entry.first).c_str(),
                                                     role.role.c_str(), role.how.c_str());
                                    }
                                }
                            } else {
                                std::fprintf(stderr, "WWMIROLE2\t%s\t(no parser roles)\t\t\n", iniPath.c_str());
                            }
                        } else {
                            std::fprintf(stderr, "WWMIROLE2\t%s\t(parser is not WWMIParseFacts)\t\t\n",
                                         iniPath.c_str());
                        }
                    }

                    // role -> (file, how the role was decided), every role of every file
                    std::map<std::string, std::vector<std::pair<std::string, std::string>>> byRole;
                    for (const auto& entry : textureRoles->rolesOf()) {
                        for (const WWMITextureRoles::Role& role : entry.second) {
                            byRole[role.role].emplace_back(entry.first, role.how);
                        }
                    }

                    // ...and the roles the mod's OWN sections name by the register they bind at --
                    // see WWMIFixerConfig::sourceRegisterRoles. Added as candidates beside the
                    // others, which is what the ranking below expects.
                    // file -> source component -> the roles that component's own register
                    // bindings offer it for. One file offered two roles of ONE slot is the alias
                    // the mask drop below is about.
                    std::map<std::string, std::map<int, std::set<std::string>>> regRolesOfFile;
                    if (!config_.sourceRegisterRoles.empty()) {
                        std::unordered_map<std::string, std::string> fileOfResource;
                        for (const auto& entry : textureRoles->resourcesOf(iniPath)) {
                            fileOfResource.emplace(StringTools::toLower(entry.first), entry.second);
                        }

                        const auto& templates = ini->getIfTemplates();
                        for (const auto& entry : present_) {
                            const auto layout = config_.sourceRegisterRoles.find(entry.first);
                            if (layout == config_.sourceRegisterRoles.end()) {
                                continue;
                            }

                            for (const std::string& section : entry.second) {
                                const auto tpl = templates.find(section);
                                if (tpl == templates.end() || tpl->second == nullptr) {
                                    continue;
                                }

                                for (const auto& [reg, role] : layout->second) {
                                    std::optional<std::string> bound = ModBranches::firstValLoose(*tpl->second, reg);
                                    if (!bound.has_value()) {
                                        continue;
                                    }

                                    // `ps-t1 = ref ResourceFoo` names the same resource as
                                    // `ps-t1 = ResourceFoo`
                                    const std::string name = IniNamingTools::removeRefPrefix(*bound);

                                    const auto file = fileOfResource.find(StringTools::toLower(name));
                                    if (file == fileOfResource.end()) {
                                        continue;
                                    }

                                    byRole[role].emplace_back(
                                        file->second, "the " + reg + " its own section binds it at");
                                    regRolesOfFile[file->second][entry.first].insert(role);
                                }
                            }
                        }
                    }

                    // ONE ENTRY PER FILE. A file is routinely found for a role more than one way
                    // -- by its hash AND by the register its own section binds it at -- and two
                    // entries naming one path rank identically, which made the "two textures are
                    // equally good answers" warning below fire with the SAME file on both sides of
                    // it. Keep the first way it was decided, which is the more specific one.
                    for (auto& entry : byRole) {
                        std::vector<std::pair<std::string, std::string>> unique;
                        for (const auto& candidate : entry.second) {
                            const bool seen = std::any_of(unique.begin(), unique.end(),
                                                          [&](const auto& kept) { return kept.first == candidate.first; });
                            if (!seen) {
                                unique.push_back(candidate);
                            }
                        }

                        entry.second = std::move(unique);
                    }

                    // A MASK ROLE SATISFIED BY THE FILE THAT ALSO SERVES THE SLOT'S NORMAL IS
                    // NOT A MASK (2026-09-27). RabbitFX's Lightmap is this fix's material mask, and
                    // a mod may point its Lightmap and its Normalmap at ONE resource -- Chisa13
                    // does, for both hair slots. The mask role then resolves to a real file, the
                    // flat test below never fires, and the target's shader reads SLOPE data as
                    // material codes. Measured: what it bound is statistically indistinguishable
                    // from Chisa's own normal maps (`d8ed7611` R mean 8.6 / G 52.8 / B 41.1 against
                    // `e921181d` 8.6 / 52.8 / 41.0; `d0d2cc80` against `9ccd7ea7` likewise) where a
                    // real mask of hers is R = 255, G = 0, B = 126 flat. Chisa13 was the only mod of
                    // 50 in the corpus binding a hair mask at all -- the other 17 Chisa mods emit
                    // none and are confirmed in game -- so this puts the aliased mod on their path.
                    //
                    // Narrow on purpose. Only a region-marking role can be dropped, and only when
                    // the SAME source component offers that same file for another role as well: a
                    // file legitimately plays every role its HASHES name, across components, and
                    // that is untouched here. The role then takes the ordinary "the mod has no file
                    // for this role" path, exactly as a flat one does.
                    for (auto& entry : byRole) {
                        const bool toGame = config_.flatLeftToGame.count(entry.first) > 0;
                        if (!toGame && config_.flatFallsBackToSource.count(entry.first) == 0) {
                            continue;
                        }

                        std::vector<std::pair<std::string, std::string>> kept;
                        for (const auto& candidate : entry.second) {
                            const auto perComponent = regRolesOfFile.find(candidate.first);
                            bool aliased = false;
                            if (perComponent != regRolesOfFile.end()) {
                                for (const auto& slot : perComponent->second) {
                                    if (slot.second.count(entry.first) > 0 && slot.second.size() > 1) {
                                        aliased = true;
                                        break;
                                    }
                                }
                            }

                            if (!aliased) {
                                kept.push_back(candidate);
                                continue;
                            }

                            ctx_.log(FileService::getRelPath(candidate.first, iniFolder)
                                     + " is bound for another role of its own slot too, so it is not"
                                     + " the mod's " + entry.first + "; "
                                     + (toGame ? "left to the game" : "the source's own is used instead"));
                        }

                        if (kept.empty() && !entry.second.empty() && toGame) {
                            leftToGame_.insert(entry.first);
                        }

                        entry.second = std::move(kept);
                    }

                    // A FLAT candidate for a region-marking role is not a usable file. Dropping it
                    // here rather than special-casing it later is what makes it take the ordinary
                    // "the mod has no file for this role" path -- and which path that is, a download
                    // of the source's own or nothing at all, is the difference between
                    // WWMIFixerConfig::flatFallsBackToSource and WWMIFixerConfig::flatLeftToGame.
                    for (auto& entry : byRole) {
                        const bool toGame = config_.flatLeftToGame.count(entry.first) > 0;
                        if (!toGame && config_.flatFallsBackToSource.count(entry.first) == 0) {
                            continue;
                        }

                        std::vector<std::pair<std::string, std::string>> varying;
                        for (const auto& candidate : entry.second) {
                            // channel 0: the material code, which is what says where the regions are
                            if (!TexThumbprint::channelIsConstant(candidate.first, 0)) {
                                varying.push_back(candidate);
                                continue;
                            }

                            ctx_.log(FileService::getRelPath(candidate.first, iniFolder)
                                     + " is a flat " + entry.first + ", which marks no regions; "
                                     + (toGame ? "left to the game" : "the source's own is used instead"));
                        }

                        if (varying.empty() && !entry.second.empty() && toGame) {
                            // Nothing of the mod's survives for this role AND nothing may stand in
                            // for it, so the fallback download is suppressed below
                            leftToGame_.insert(entry.first);
                        }

                        entry.second = std::move(varying);
                    }

                    // Which source component each role BELONGS to, off the two config tables that
                    // are written per component. Not the same question as which component's slot is
                    // asking for it: Chisa's accessory slot and four of her extra passes bind
                    // `frontHairDiffuse`, which is component 0's.
                    std::unordered_map<std::string, std::set<int>> componentsOfRole;
                    for (const auto& entry : config_.sourceRegisterRoles) {
                        for (const auto& reg : entry.second) {
                            componentsOfRole[reg.second].insert(entry.first);
                        }
                    }

                    for (const auto& entry : config_.typeRoles) {
                        for (const auto& type : entry.second) {
                            componentsOfRole[type.second].insert(entry.first);
                        }
                    }

                    // ...and whether this mod's tags are in the SOURCE's numbering at all. A
                    // `Components-<N>` tag is written by the exporter in the numbering of the
                    // character the mod was made for, which for a mod installed on the other half
                    // of a pair is the other character's: Sanhua has seven components and
                    // SanhuaExorcist six, so a Sanhua mod fixed as the Exorcist carries a
                    // `Components-6` her plan has no component for. A tag naming a component the
                    // source does not have says the whole numbering is somebody else's, so the
                    // refusal below is switched off for the file -- rank() reads the same tag and
                    // can only mis-PREFER, where a refusal deletes.
                    std::set<int> sourceComponents;
                    for (const auto& entry : config_.plan) {
                        sourceComponents.insert(entry.first);
                    }

                    bool tagsAreOurs = true;
                    for (const auto& entry : byRole) {
                        for (const auto& candidate : entry.second) {
                            for (int c : componentTag(candidate.first)) {
                                if (sourceComponents.count(c) == 0) {
                                    tagsAreOurs = false;
                                }
                            }
                        }
                    }

                    // How well a file serves ONE source component: first how specifically its
                    // WWMI-Tools `Components-<a>-<b>... t=<hash>.dds` name is tagged for that component,
                    // then whether this .ini already has a resource for it, then its distance.
                    //
                    // A hash override binds ONE file per hash wherever it is drawn, so an exporter that
                    // writes both a per-component texture and a shared one under the same hash leaves the
                    // per-component art unbound in the mod's own .ini: sanhua_qiming ships its bangs' own
                    // mask as `Components-0 t=d153e37f.dds` and the game's shared mask as
                    // `Components-0-1-2-3-4 t=d153e37f.dds`, and binds the second. Sampled at the mod's
                    // own atlas UVs that put wrong-coloured patches over the fringe in game (2026-09-19).
                    // A register binding is per component and can honour the specific one.
                    auto rank = [&](const std::string& file, int component) {
                        std::string rel = FileService::pathKey(FileService::getRelPath(file, iniFolder));
                        std::size_t ups = 0;
                        std::size_t pos = 0;
                        while ((pos = rel.find("../", pos)) != std::string::npos) {
                            ++ups;
                            pos += 3;
                        }

                        // AN UNTAGGED FILE MAKES NO CLAIM, AND IS NOT EVIDENCE AGAINST (2026-09-28).
                        // `Components-<i>-<j>... t=<hash>.dds` is WWMI-Tools' export name and no mod
                        // author is obliged to keep it -- a mod-manager-packaged mod names every
                        // texture a GUID, and `Component3.dds`, `Upper_D.dds` and `wumao.dds` are all
                        // in this corpus. Ranking "no tag" the same as "tagged for other components"
                        // put every file of such a mod in the worst bucket, where one leftover
                        // vanilla `Components-<this> t=<hash>.dds` outranked all of them.
                        const std::vector<int> tag = componentTag(file);
                        int specificity = 2;                               // no tag: says nothing
                        if (tag.size() == 1 && tag.front() == component) {
                            specificity = 0;                               // exactly this component
                        } else if (std::find(tag.begin(), tag.end(), component) != tag.end()) {
                            specificity = 1;                               // this one among others
                        } else if (!tag.empty()) {
                            specificity = 3;                               // tagged, and NOT this one
                        }

                        return std::make_tuple(specificity, resourceOfFile.count(file) > 0 ? 0 : 1, ups, rel.size(), rel);
                    };

                    // The choice is per (role, source component), not per role -- but an ambiguity
                    // BETWEEN TWO FILES is a property of the files, so it is reported once however
                    // many components are offered the role.
                    std::set<std::string> saidAmbiguous;
                    auto assign = [&](const std::string& role, int component) {
                        {
                            auto found = byRole.find(role);
                            if (found == byRole.end() || found->second.empty()
                                || resourceOfSlotRole_.count({role, component}) > 0) {
                                return;
                            }

                            std::vector<std::pair<std::string, std::string>> candidates = found->second;

                            // A file the exporter TAGGED for other components is not this ROLE's
                            // texture, whatever hash the mod aliased onto it (2026-09-28). Chisa2
                            // binds one file -- its component-3 kimono atlas
                            // `Components-3 t=4c7e5ddf.dds` -- under SEVEN hashes, a shotgun so the
                            // mod survives a game version bump; one of them, `6616fe2c`, is
                            // genuinely a lowerDiffuse, so once that generation was filed in
                            // HashData the kimono was bound at the lower body's ps-t3 and the whole
                            // garment rendered red in game. The hash row is right; the mod's
                            // aliasing is what is not.
                            //
                            // "A file plays EVERY role its hashes name" is unchanged -- that is how
                            // one atlas serves two components, and such a file's name LISTS both
                            // (`Components-2-4 t=<hash>.dds`). Refused only when the name names
                            // components and the role's own is not among them, so a file with no
                            // tag (`Upper_D.dds`, a GUID, `Component3.dds`) and a role no config
                            // table places are both untouched. Dropping every candidate is a real
                            // answer too: the mod ships nothing for that role, which is what the
                            // fallback download is for.
                            auto roleComponents = componentsOfRole.find(role);
                            if (tagsAreOurs && roleComponents != componentsOfRole.end()) {
                                candidates.erase(
                                    std::remove_if(candidates.begin(), candidates.end(),
                                                   [&](const std::pair<std::string, std::string>& candidate) {
                                                       // Only a candidate a HASH put here. A role read
                                                       // off the mod's own component section -- its
                                                       // `ps-tN` or its `Resource\RabbitFX\...` line --
                                                       // is the author saying what that component is
                                                       // textured with, which outranks a file name:
                                                       // Chisa2's component 5 binds the kimono atlas
                                                       // itself, deliberately.
                                                       //
                                                       // Not readable off `how`: byRole is built
                                                       // hash-first and deduplicated to one entry per
                                                       // file keeping the FIRST way it was decided, so a
                                                       // file found both ways carries the hash's. Ask
                                                       // regRolesOfFile, which is that route's own map.
                                                       if (!StringTools::startsWith(candidate.second, "hash ")) {
                                                           return false;
                                                       }

                                                       auto regRoles = regRolesOfFile.find(candidate.first);
                                                       if (regRoles != regRolesOfFile.end()) {
                                                           for (const auto& perComponent : regRoles->second) {
                                                               if (perComponent.second.count(role) > 0) {
                                                                   return false;
                                                               }
                                                           }
                                                       }

                                                       const std::vector<int> tag = componentTag(candidate.first);
                                                       if (tag.empty()) {
                                                           return false;
                                                       }

                                                       return std::none_of(tag.begin(), tag.end(),
                                                                           [&](int c) { return roleComponents->second.count(c) > 0; });
                                                   }),
                                    candidates.end());
                                if (candidates.empty()) {
                                    return;
                                }
                            }

                            // A file NAMED for a hash of this role is the self-consistent choice and
                            // beats everything below (2026-09-27). A mod may declare a texture under
                            // a hash its own name disagrees with: Chisa13 binds its hair DIFFUSE
                            // (`Components-1 t=23b680fe.dds`) in a SECOND TextureOverride carrying
                            // the hair normal's hash `d8ed7611`, beside the correct
                            // `Components-1 t=d8ed7611.dds`. Both then rank identically -- same
                            // component tag, both with a resource, same folder, same NAME LENGTH --
                            // so the winner was the last tiebreak, which is alphabetical, and
                            // "23b680fe" sorts first. The diffuse was bound as the normal map; once
                            // the hair's ps-t5 was actually bound and repacked (R and B zeroed) that
                            // rendered the hair GREEN.
                            // Each candidate carries WHY it matched -- "hash <h> (...)" when a hash
                            // put it here. A file whose own name contradicts that hash is demoted;
                            // everything else keeps the order below, so the only behaviour that
                            // moves is this one contradiction.
                            auto contradictsItsHash = [&](const std::pair<std::string, std::string>& candidate) {
                                static const std::string prefix = "hash ";
                                if (!StringTools::startsWith(candidate.second, prefix)) {
                                    return 0;               // not matched by hash: nothing to contradict
                                }

                                std::string hash = candidate.second.substr(prefix.size());
                                const std::size_t end = hash.find(' ');
                                if (end != std::string::npos) {
                                    hash = hash.substr(0, end);
                                }

                                const std::string name = StringTools::toLower(
                                    FileService::baseName(candidate.first));
                                return name.find(StringTools::toLower(hash)) == std::string::npos ? 1 : 0;
                            };

                            // THE DEMOTION BELONGS AFTER rank()'s FIRST TWO ELEMENTS, NOT ABOVE ALL
                            // OF THEM (2026-09-28). rank() already orders by how specifically a
                            // file's `Components-<list>` name is tagged for this component, and then
                            // by whether this .ini DECLARES a resource for it -- a file nothing names
                            // is a spare the user copies in by hand. Sorting the 2026-09-27
                            // contradiction above both let a spare win on a name:
                            //
                            // Chisa2 ships its active upper atlas as `Textures/Components-3
                            // t=4c7e5ddf.dds` (its `[ResourceBase]`) with three spare colourways
                            // beside it under `2Color Variation/{Black,Red,White}/`. Its own file
                            // takes upperDiffuse from `2970cef1`, one of SEVEN hashes it aliases onto
                            // that resource, and its name carries `4c7e5ddf` -- so it was demoted and
                            // the untouched RED spare, whose name and hash agree, was bound over it.
                            // In game: the kimono all red, which is what was reported.
                            //
                            // Specificity stays FIRST, which is what keeps the 2026-09-19 rule:
                            // sanhua_qiming ships its bangs' own mask as `Components-0
                            // t=d153e37f.dds` and the game's shared one as `Components-0-1-2-3-4
                            // t=d153e37f.dds`, declaring only the shared one, and the per-component
                            // file must still win. Hoisting the declared test above specificity moved
                            // 56 bindings onto different bytes across Sanhua and Chisa.
                            std::sort(candidates.begin(), candidates.end(),
                                      [&](const auto& a, const auto& b) {
                                          const int badA = contradictsItsHash(a);
                                          const int badB = contradictsItsHash(b);
                                          if (badA != badB) {
                                              return badA < badB;
                                          }

                                          return rank(a.first, component) < rank(b.first, component);
                                      });
                            const std::string& best = candidates.front().first;
                            if (candidates.size() > 1) {
                                const auto first = rank(best, component);
                                const auto second = rank(candidates[1].first, component);
                                if (contradictsItsHash(candidates.front()) == contradictsItsHash(candidates[1])
                                    && std::get<0>(first) == std::get<0>(second) && std::get<1>(first) == std::get<1>(second)
                                    && std::get<2>(first) == std::get<2>(second)
                                    && saidAmbiguous.insert(role + "\n" + best + "\n" + candidates[1].first).second) {
                                    // Two shipped textures equally close on one role: the first is bound
                                    // and only a measurement can say which is right -- say so loudly.
                                    ctx_.log("WARNING: " + FileService::getRelPath(candidates[1].first, iniFolder)
                                             + " also has the role " + role + " (" + candidates[1].second
                                             + "), already taken by " + FileService::getRelPath(best, iniFolder)
                                             + " (" + candidates.front().second + "); the first one is bound");
                                }
                            }

                            fileOfRole_[role] = best;
                            auto own = resourceOfFile.find(best);
                            if (own != resourceOfFile.end()) {
                                resourceOfSlotRole_[{role, component}] = own->second;
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
                                    FileService::strToPath(FileService::getRelPath(best, iniFolder)));
                                declared_.emplace_back(best, rel);
                            }

                            resourceOfSlotRole_[{role, component}] = declaredName->second;
                        }
                    };

                    for (const auto& planned : config_.plan) {
                        if (present_.count(planned.first) == 0) {
                            continue;
                        }

                        for (const WWMIFixerConfig::Binding& binding : planned.second.bindings) {
                            assign(binding.role, planned.first);
                        }
                    }

                    // ...and the roles the plan does not name, which only a slot's OTHER passes or a
                    // shared mesh bind. Until these were asked, a role like Chisa's accessoryNormal
                    // could only ever come from the fallback DOWNLOAD -- so the fix fetched the
                    // game's normal map and bound it over the one the mod ships, at the mod's UVs.
                    // assign() no-ops on a role already resolved or one no file serves, so offering
                    // every present component is safe.
                    for (const std::string& role : passOnlyRoles()) {
                        for (const auto& entry : present_) {
                            assign(role, entry.first);
                        }
                    }

                    for (const WWMIFixerConfig::CreatedTexture& created : config_.createdTextures) {
                        resourceOfRole_[created.role] = fixName(IniKeywords::Resource + created.role);
                    }

                    // A planned role the mod has NO file for: the SOURCE's own game texture, as a
                    // download named RemapDL (the file is the fix's, so an undo may delete it). The
                    // mod's UVs are the source's, and the target's texture -- what the register
                    // samples on the target's draw when nothing binds it -- is wrong by construction.
                    if (!config_.downloadCharFolder.empty()) {
                        for (const auto& entry : config_.plan) {
                            if (present_.count(entry.first) == 0) {
                                continue;
                            }

                            for (const WWMIFixerConfig::Binding& binding : entry.second.bindings) {
                                if (resourceOfSlotRole_.count({binding.role, entry.first}) > 0
                                    || resourceOfRole_.count(binding.role) > 0) {
                                    continue;
                                }

                                auto fallback = config_.fallbackTextures.find(binding.role);
                                if (fallback == config_.fallbackTextures.end()
                                        || leftToGame_.count(binding.role) > 0) {
                                    continue;
                                }

                                const std::string kind = TextTools::capitalize(binding.role);
                                const std::string fileName = DownloadTools::fixedFileName(config_.downloadPrefix, kind, FileExt::DDS);
                                const std::string resource = IniKeywords::Resource + config_.downloadPrefix + kind + IniKeywords::RemapDL;
                                fallbacks_[binding.role] = Fallback{
                                    DownloadTools::downloadFolder() + "/"
                                        + DownloadTools::urlPath(config_.downloadGameFolder, config_.downloadCharFolder,
                                                                 config_.downloadVersionFolder, config_.downloadPrefix,
                                                                 "Texture" + fallback->second, FileExt::DDS),
                                    fileName, textureFolder_ + "/" + fileName, resource};
                                resourceOfRole_[binding.role] = resource;
                            }
                        }

                        // ...and the roles only a slot's OTHER passes or a shared mesh bind. The
                        // loop above walks the plan, which is each component's MAIN pass, so a role
                        // bound nowhere else never downloaded: Chisa's accessoryNormal is bound only
                        // by slot 5's two side-panel passes, and her ribbon rendered with the GAME's
                        // normal map at the mod's UVs.
                        //
                        // The plan's per-component test is a different question from "does any file
                        // of the mod serve this role", so it is left alone and this asks the shared
                        // one.
                        for (const std::string& role : passOnlyRoles()) {
                            if (resourceOfRole_.count(role) > 0) {
                                continue;
                            }

                            bool owned = false;
                            for (const auto& entry : resourceOfSlotRole_) {
                                if (entry.first.first == role) {
                                    owned = true;
                                    break;
                                }
                            }

                            auto fallback = config_.fallbackTextures.find(role);
                            if (owned || fallback == config_.fallbackTextures.end()
                                    || leftToGame_.count(role) > 0) {
                                continue;
                            }

                            const std::string kind = TextTools::capitalize(role);
                            const std::string fileName = DownloadTools::fixedFileName(config_.downloadPrefix, kind, FileExt::DDS);
                            const std::string resource = IniKeywords::Resource + config_.downloadPrefix + kind + IniKeywords::RemapDL;
                            fallbacks_[role] = Fallback{
                                DownloadTools::downloadFolder() + "/"
                                    + DownloadTools::urlPath(config_.downloadGameFolder, config_.downloadCharFolder,
                                                             config_.downloadVersionFolder, config_.downloadPrefix,
                                                             "Texture" + fallback->second, FileExt::DDS),
                                fileName, textureFolder_ + "/" + fileName, resource};
                            resourceOfRole_[role] = resource;
                        }
                    }
                }

                // ---- the edits ----

                ModObj targetSlotObj(int slot) const {
                    return ModObj(toModName_, config_.slotPrefix + std::to_string(slot));
                }

                void buildEdits() {
                    buildTexcoordCopy();
                    const ModType* source = ctx_.modType();
                    const std::optional<Version> from = fromVersion();
                    const std::optional<Version> to = toVersion();

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
                                if (dstBone > WWMIMaxByteBone) {
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
                            if (targetPast256_ && blendRemapBones_ > WWMIBlendRemapSize) {
                                note("the vertex group row names " + std::to_string(blendRemapBones_)
                                     + " distinct target bones, past the "
                                     + std::to_string(WWMIBlendRemapSize)
                                     + " one blend remap holds; falling back to an 8-bit blend, so"
                                     + " every bone past " + std::to_string(WWMIMaxByteBone)
                                     + " will land on bone 0");
                                targetPast256_ = false;
                            }
                        }
                    }


                    assetRemap_ = std::make_unique<RegAssetRemap<>>(
                        std::vector<std::pair<std::string, RegAssetRemap<>::AssetSpec>>{
                            {IniKeywords::Hash, RegAssetRemap<>::AssetSpec(ctx_.modTypeHashes(), IniKeywords::HashNotFound)},
                            {ShapeKeyChecksumKey, RegAssetRemap<>::AssetSpec(source->shapeKeyChecksums.get(), ChecksumNotFound)}},
                        toModName_, ctx_.modTypeName().value_or(""), from, to);
                    assetAdapter_ = std::make_unique<RegPartEdit<>>(assetRemap_.get());

                    // Lines a remapped section drops -- see WWMIFixerConfig::removedRegs for why
                    // each kind is there. Built once and hung on every remapped slot section.
                    std::vector<WWMIFixerConfig::RegRemoval> removals = config_.removedRegs;
                    if (targetPast256_) {
                        // The MOD's own merge list, dropped from the remapped sections. It writes
                        // the TARGET's bones -- Chisa's slot 3 starts at merged offset 142 and runs
                        // to 269 -- into the mod's own merged skeleton, which every ChisaParfait mod
                        // declares for 256 bones. The writes past the end are dropped by D3D, but
                        // the ones that land corrupt the SOURCE's skeleton whenever both characters
                        // are on screen at once. The fix's own merge list (CommandListMergeSlot<N>)
                        // does the same work into a buffer sized for the target, and is added to
                        // these sections beside it.
                        //
                        // Matched by prefix, which covers the name both before and after the group
                        // remap renames it. A mod whose merge list is named something else keeps it,
                        // which is wasted work rather than a wrong picture.
                        removals.push_back(WWMIFixerConfig::RegRemoval{IniKeywords::Run, "commandlistmergeskeleton"});
                    }

                    if (!removals.empty()) {
                        std::vector<std::pair<std::string, std::optional<RegRemove<>::RemoveKeyCheck>>> keys;
                        keys.reserve(removals.size());
                        for (const WWMIFixerConfig::RegRemoval& removal : removals) {
                            if (removal.valuePrefix.empty()) {
                                keys.emplace_back(removal.reg, std::nullopt);
                                continue;
                            }

                            const std::string prefix = StringTools::toLower(removal.valuePrefix);
                            keys.emplace_back(removal.reg, RegRemove<>::RemoveKeyCheck(
                                [prefix](long long, const std::string& value) {
                                    return StringTools::startsWith(
                                        StringTools::toLower(StringTools::lstrip(value)), prefix);
                                }));
                        }

                        regRemove_ = std::make_unique<RegRemove<>>(std::move(keys));
                        removeAdapter_ = std::make_unique<RegPartEdit<>>(regRemove_.get());
                    }

                    // The groups: one remapped section per target draw per file. The first source
                    // component claiming a slot stays in the mod's own .ini, the second lands in the
                    // first copy, and so on -- in the source components' numeric order, which is the
                    // order the remap below creates the target graphs in and so the order collisions
                    // are resolved in.
                    std::map<int, std::size_t> claimants;
                    for (const auto& entry : present_) {
                        auto planned = config_.plan.find(entry.first);
                        if (planned == config_.plan.end()) {
                            dropped_.push_back(entry.first);
                            continue;
                        }

                        const std::size_t group = claimants[planned->second.slot]++;
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

                        note("this mod draws " + std::to_string(dropped_.size())
                             + (dropped_.size() == 1 ? " component " : " components ") + names
                             + " that " + source_.name + "'s plan has no target slot for -- "
                             + (dropped_.size() == 1 ? "it is" : "they are") + " not remapped");
                    }


                    // Per target slot: its numbers, written over the source's.
                    for (int slot : drawnSlots_) {
                        const Slot& s = target_.slots.at(static_cast<std::size_t>(slot));
                        auto newVals = std::make_unique<RegNewVals<>>(
                            std::vector<std::pair<std::string, RegNewVals<>::NewValSpec>>{
                                {IniKeywords::MatchFirstIndex, RegNewVals<>::NewVal(s.indexOffset)},
                                {IniKeywords::MatchIndexCount, RegNewVals<>::NewVal(s.indexCount)},
                                {VgOffsetKey, RegNewVals<>::NewVal(s.vgOffset)},
                                {VgCountKey, RegNewVals<>::NewVal(s.vgCount)}},
                            false);
                        for (const auto& planned : config_.plan) {
                            if (planned.second.slot != slot || present_.count(planned.first) == 0) {
                                continue;
                            }

                            for (const std::string& section : present_.at(planned.first)) {
                            }
                        }

                        auto adapter = std::make_unique<RegPartEdit<>>(newVals.get());
                        newValsOf_[slot] = adapter.get();
                        newVals_.push_back(std::move(newVals));
                        regAdapters_.push_back(std::move(adapter));
                    }

                    // Per drawn source component: the zero stream and its texture command list right
                    // after the shared-resource override. No after-register on the add: `drawindexed`
                    // as one is a MUST fact, and behind a `$draw_x` toggle no draw is certain, which
                    // parked the additions inside the first toggle.
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
                            additions.emplace_back(config_.shapeKeyStreamReg, fixName(IniKeywords::Resource + ShapeKeyZero));
                        }

                        if (texcoordResource_.has_value()) {
                            additions.emplace_back(config_.texcoordReg, *texcoordResource_);
                        }

                        std::vector<std::string> bindings;
                        for (const WWMIFixerConfig::Binding& binding : planned.bindings) {
                            if (binding.role == IniKeywords::Null) {
                                bindings.push_back(nullLine(binding.reg));
                                continue;
                            }

                            const std::string* resource = resourceFor(binding.role, component);
                            if (resource != nullptr) {
                                bindings.push_back(bindLine(binding.role, binding.reg, *resource));
                            }
                        }

                        if (!bindings.empty()) {
                            const std::string cmdList = fixName(IniKeywords::CommandList + source_.name + TextTools::capitalize(config_.slotPrefix)
                                                                + std::to_string(component) + "Textures");
                            const std::string condition =
                                passCondition(config_.slotPasses.at(static_cast<std::size_t>(planned.slot)));
                            std::string text = "[" + cmdList + "]\nif " + condition + "\n";
                            for (const std::string& binding : bindings) {
                                text += binding + "\n";
                            }

                            text += "endif\n";
                            textureLists_.push_back(text);
                            additions.emplace_back(IniKeywords::Run, cmdList);
                        }

                        // A slot's OTHER passes bind the same art at DIFFERENT registers, so each
                        // gets its own guarded list beside the plan's -- see
                        // WWMIFixerConfig::extraPassRegs.
                        const auto extra = config_.extraPassRegs.find(planned.slot);
                        if (extra != config_.extraPassRegs.end()) {
                            int n = 0;
                            for (const auto& [pass, regs] : extra->second) {
                                std::vector<std::string> extraBindings;
                                for (const WWMIFixerConfig::Binding& binding : regs) {
                                    // A binding for ONE source, on a slot two sources merge onto --
                                    // see Binding::srcComponent. The default -1 takes every source,
                                    // which is what every config written before 2026-09-28 means.
                                    if (binding.srcComponent >= 0 && binding.srcComponent != component) {
                                        continue;
                                    }

                                    const std::string* resource = resourceFor(binding.role, component);
                                    if (resource != nullptr) {
                                        extraBindings.push_back(bindLine(binding.role, binding.reg, *resource));
                                    }
                                }

                                if (extraBindings.empty()) {
                                    ++n;
                                    continue;
                                }

                                const std::string extraList =
                                    fixName(IniKeywords::CommandList + source_.name + TextTools::capitalize(config_.slotPrefix)
                                            + std::to_string(component) + "TexturesPass" + std::to_string(n));
                                std::string extraText = "[" + extraList + "]\nif " + passCondition(pass) + "\n";
                                for (const std::string& binding : extraBindings) {
                                    extraText += binding + "\n";
                                }

                                extraText += "endif\n";
                                textureLists_.push_back(extraText);
                                additions.emplace_back(IniKeywords::Run, extraList);
                                ++n;
                            }
                        }

                        if (!additions.empty()) {
                            // The remap has already renamed the called list by the time this runs,
                            // so the anchor is matched under either name.
                            const std::string sharedList = config_.sharedResourcesList;
                            const std::string renamedList = fixName(sharedList);
                            auto add = std::make_unique<RegSurroundedAdd<>>(
                                std::move(additions),
                                RegSurroundedAdd<>::RegMap{{IniKeywords::Run, [sharedList, renamedList](const std::string& value) {
                                    return value == sharedList || value == renamedList;
                                }}},
                                RegSurroundedAdd<>::RegMap{}, false);
                            auto adapter = std::make_unique<GraphPartEdit<>>(add.get());
                            editsOf_[component].push_back(adapter.get());
                            surroundedAdds_.push_back(std::move(add));
                            graphAdapters_.push_back(std::move(adapter));
                        }
                    }

                    // The hash-only objects: the hidden ones (the shape keys) commented out of the
                    // original and copied nowhere, the rest (the bone-data override) into every group.
                    std::vector<ModObj> hashOnlyObjs;
                    if (auto* parser = dynamic_cast<GIMIParser<>*>(this->getParser())) {
                        for (const ModObj& obj : parser->modObjs()) {
                            if (obj.second.rfind(config_.slotPrefix, 0) != 0) {
                                hashOnlyObjs.push_back(obj);
                            }
                        }
                    }

                    const std::unordered_set<std::string> hidden(config_.hiddenObjs.begin(), config_.hiddenObjs.end());

                    // The remap: every source slot section onto its target slot's object, named
                    // after the TARGET so a target never collides with a source still to be moved;
                    // two claimants of one slot collide with each other, which is what makes the
                    // further groups (GraphGroupRemap's own rule). Every section keeps its own name
                    // plus the fix suffix, as the prototype's output has it.
                    const GraphGroupRemap<>::RenameFunc rename = [this](const std::string& name) { return fixName(name); };
                    GraphGroupRemap<>::RemapList remap;
                    for (const auto& entry : present_) {
                        std::vector<GraphGroupRemap<>::RemapTarget> targets;
                        auto planned = config_.plan.find(entry.first);
                        if (planned != config_.plan.end()) {
                            const ModObj target = targetSlotObj(planned->second.slot);
                            targets.emplace_back(GraphId(0, target.first, target.second), rename);
                        }

                        const ModObj src = slotObj(entry.first);
                        remap.emplace_back(GraphId(0, src.first, src.second), std::move(targets));
                    }

                    for (const ModObj& obj : hashOnlyObjs) {
                        std::vector<GraphGroupRemap<>::RemapTarget> targets;
                        if (hidden.count(obj.second) > 0) {
                            this->hiddenModObjs.insert(obj);
                        } else {
                            for (std::size_t g = 0; g < groups_.size(); ++g) {
                                targets.emplace_back(GraphId(0, obj.first, obj.second), rename);
                            }
                        }

                        remap.emplace_back(GraphId(0, obj.first, obj.second), std::move(targets));
                    }

                    slotRemap_ = std::make_unique<GraphGroupRemap<>>(std::move(remap));

                    // The edits, per group, keyed by the target objects the remap created.
                    std::vector<ObjGroupEdit::IniEdits> perGroup(groups_.size());
                    for (std::size_t g = 0; g < groups_.size(); ++g) {
                        for (int component : groups_[g]) {
                            const int slot = config_.plan.at(component).slot;
                            const ModObj obj = targetSlotObj(slot);
                            std::vector<PartEdit*> edits = editsOf_[component];
                            if (removeAdapter_ != nullptr) {
                                edits.push_back(removeAdapter_.get());
                            }

                            edits.push_back(newValsOf_.at(slot));
                            edits.push_back(assetAdapter_.get());
                            perGroup[g].edits[obj] = std::move(edits);
                            perGroup[g].trackKeys[obj] = false;
                        }

                        for (const ModObj& obj : hashOnlyObjs) {
                            if (hidden.count(obj.second) > 0) {
                                continue;
                            }

                            perGroup[g].edits[obj] = {assetAdapter_.get()};
                            perGroup[g].trackKeys[obj] = false;
                            if (g == 0) {
                                }
                        }
                    }

                    mainEdits_ = std::make_unique<ObjGroupEdit>(std::move(perGroup), false);

                    // The blend: the register bound in the shared override, collected out of every
                    // drawn slot section -- one collect PER GROUP, so every copy declares the
                    // RemapBlend resource its own sections bind, as a GIMI merge's copies do (a
                    // collect is addressed by GraphId, whose iniIndex is the group).
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
                                FileService::absPathOfRelPath(positionFile_, ctx_.getIniFile()->getFolder());
                            const BlendRemapOut out{
                                FileService::absPathOfRelPath(blendRemapFile("VertexVG"), ctx_.getIniFile()->getFolder()),
                                FileService::absPathOfRelPath(blendRemapFile("Forward"), ctx_.getIniFile()->getFolder()),
                                FileService::absPathOfRelPath(blendRemapFile("Reverse"), ctx_.getIniFile()->getFolder())};
                            lift = [vgPath, posPath, out](RemapBlendResource& resource) {
                                return writeBlendRemap(resource, vgPath, posPath, out);
                            };
                        } else if (vertexVGFile_.has_value()) {
                            const std::string vgRel = FileService::iniPathToRel(*vertexVGFile_);
                            const std::string vgPath =
                                FileService::absPathOfRelPath(vgRel, ctx_.getIniFile()->getFolder());
                            const std::string posPath =
                                FileService::absPathOfRelPath(positionFile_, ctx_.getIniFile()->getFolder());
                            lift = [vgPath, posPath](RemapBlendResource& resource) {
                                return remapFromVertexVG(resource, vgPath, posPath);
                            };
                        } else if (legacy_) {
                            const std::string indexPath = FileService::absPathOfRelPath(indexFile_, ctx_.getIniFile()->getFolder());
                            const std::string positionPath =
                            FileService::absPathOfRelPath(positionFile_, ctx_.getIniFile()->getFolder());
                            const std::map<int, std::vector<std::pair<long long, long long>>> ranges = drawRanges_;
                            const std::map<int, std::vector<int>> maps = config_.sourceVgMaps;
                            lift = [indexPath, positionPath, ranges, maps](RemapBlendResource& resource) {
                                return liftLegacyBlend(resource, indexPath, positionPath, ranges, maps);
                            };
                        }

                        auto replace = std::make_unique<WWMIBlendReplace>(GraphId(g, "", "blend"), makeResEditConfig(), source, from, to,
                                                                          std::move(lift), config_.anchorChains,
                                                                          meshVertexCount_,
                                                                          FileService::absPathOfRelPath(positionFile_, ctx_.getIniFile()->getFolder()));
                        auto collect = std::make_unique<Collector>();
                        for (int component : groups_[g]) {
                            const ModObj obj = targetSlotObj(config_.plan.at(component).slot);
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
                            collect->resPredicates[GraphId(g, obj.first, obj.second)] =
                                [](const std::string&, const std::string& value, const Collector::IterData&) {
                                    return !IniNamingTools::hasRefPrefix(value);
                                };
                        }

                        collect->resEdits = {{"blend", replace.get()}};
                        blendReplaces_.push_back(std::move(replace));
                        blendCollects_.push_back(std::move(collect));
                    }
                }

                // ---- what the WRITTEN text must say ----


                // The resource a component binds for a role: its own choice (readTextures), else what
                // every component shares -- a created texture, or a download of the source's own
                // A shared mesh has no component, so take whichever present component owns the art
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
                std::string nullLine(const std::string& reg) {
                    return "    " + reg + " = " + std::string(IniKeywords::Null);
                }

                std::string bindLine(const std::string& role, const std::string& reg, const std::string& resource) {
                    const std::string direct = "    " + reg + " = " + resource;

                    // An edit's output is not one of the mod's resources, so ask under the resource
                    // the edit READ
                    const auto edited = sourceOfEdited_.find(StringTools::toLower(resource));
                    const std::string base = edited == sourceOfEdited_.end() ? resource : edited->second;
                    const auto owner = conditionalOwner_.find(StringTools::toLower(base));
                    if (owner == conditionalOwner_.end()) {
                        return direct;
                    }

                    const auto cached = roleLists_.find({role, reg});
                    if (cached != roleLists_.end()) {
                        return "    " + std::string(IniKeywords::Run) + " = " + cached->second;
                    }

                    IniFile* ini = ctx_.getIniFile();
                    const auto& templates = ini->getIfTemplates();
                    const auto tpl = templates.find(owner->second);
                    if (tpl == templates.end() || tpl->second == nullptr) {
                        return direct;
                    }

                    const std::string name =
                        fixName(IniKeywords::CommandList + source_.name + TextTools::capitalize(role) + IniNamingTools::getRegTag(reg));

                    // A COPY OF THE MOD'S OWN SECTION, EDITED THROUGH THE SAME regEdits AS THE REST
                    // OF THE FIX. This used to render the section to text and rewrite the lines,
                    // which is the section model written out and read straight back in.
                    std::unique_ptr<IfTemplate<std::string, std::string>> list = tpl->second->deepcopy();
                    list->name = name;
                    list->prefix = "";                      // the mod's own comment is about the mod's section

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
                    //    `$Char != 0` the hair took variant 0's normal map. `sourceOfEdited_` is
                    //    keyed by exactly those names.
                    const RegRemove<>::RemoveKeyCheck isDead =
                        [this, &templates](long long, const std::string& val) {
                            const std::string bound = StringTools::toLower(val);
                            return editedResourceOf_.count(bound) == 0 && sourceOfEdited_.count(bound) == 0
                                   && fileOfResource_.count(bound) == 0 && templates.count(val) == 0;
                        };

                    // 2. `this = <a resource>` becomes `this = <our edited copy of it>` -- but ONLY
                    //    for the role the edit was registered for. One file can serve several roles
                    //    -- Chisa13 points RabbitFX's Lightmap AND Normalmap at one resource, and
                    //    this fix reads that Lightmap as the material MASK -- so a swap keyed on the
                    //    RESOURCE alone put the repacked normal map on the mask register too:
                    //    material code 0 over the whole head and A = 255 where the target's mask
                    //    carries 0 (2026-09-27).
                    const RegNewVals<>::OldValProducer toEdited =
                        [this, &role](const std::string& oldValue, const ModType*) {
                            const std::string bound = StringTools::toLower(oldValue);
                            const auto swap = editedResourceOf_.find(bound);
                            const auto owns = editedRoleOf_.find(bound);
                            if (swap != editedResourceOf_.end()
                                    && owns != editedRoleOf_.end() && owns->second == role) {
                                return swap->second;
                            }

                            return oldValue;
                        };

                    std::size_t bindings = 0;
                    for (const std::unique_ptr<IfTemplatePart>& part : list->parts()) {
                        auto* content = dynamic_cast<IfContentPart<std::string, std::string>*>(part.get());
                        if (content == nullptr) {
                            continue;                       // an `if` / `endif`, which the renderer keeps
                        }

                        // The rules are matched against the part's OWN keys rather than against the
                        // literal spellings: `THIS = ResourceX` is legal 3dmigoto and a regEdit
                        // matches a key exactly, so the exact key is what it is handed.
                        std::vector<std::string> binds;
                        std::vector<std::pair<std::string, std::optional<RegRemove<>::RemoveKeyCheck>>> drops;
                        std::unordered_set<std::string> seen;
                        for (const auto& item : content->items()) {
                            if (!seen.insert(item.key).second) {
                                continue;
                            }

                            const std::string key = StringTools::toLower(item.key);
                            if (IniKeywords::MatchKeys.count(key) > 0) {
                                drops.emplace_back(item.key, std::nullopt);   // meaningless in a list
                            } else if (key == IniKeywords::This) {
                                drops.emplace_back(item.key, isDead);
                                binds.push_back(item.key);
                            }
                        }

                        RegRemove<>(std::move(drops)).edit(*content, name);

                        for (const std::string& key : binds) {
                            RegNewVals<>({{key, RegNewVals<>::NewVal(toEdited)}}).edit(*content, name);

                            // 3. ...and the key itself is the register the target's draw reads.
                            bindings += content->count(key);
                            RegRemap<>({{key, RegRemap<>::KeyRemapValue(
                                RemapList<std::string, std::string>{reg})}})
                                .edit(*content, name);
                        }
                    }

                    if (bindings == 0) {
                        return direct;
                    }

                    list->rebuild();
                    roleLists_.emplace(std::pair<std::string, std::string>{role, reg}, name);
                    // Their own vector, emitted at the END of buildAppended: textureLists_ is
                    // written out before the shared-mesh loop runs, so a list created there would be
                    // dropped. Section order in an .ini does not matter.
                    // `renderIfTemplate` returns no trailing newline; the consumer adds the blank line
                    roleListTexts_.push_back(renderIfTemplate(*list) + "\n");
                    return "    " + std::string(IniKeywords::Run) + " = " + name;
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

                    auto shared = resourceOfRole_.find(role);
                    return shared == resourceOfRole_.end() ? nullptr : &shared->second;
                }

                // The components a WWMI-Tools export name is tagged for: `Components-0-2 t=<hash>.dds`
                // is {0, 2}. Empty for a file named anything else
                static std::vector<int> componentTag(const std::string& file) {
                    const std::string name = StringTools::toLower(FileService::baseName(file));
                    const std::string prefix = "components-";
                    const std::size_t end = name.find(" t=");
                    if (!StringTools::startsWith(name, prefix) || end == std::string::npos) {
                        return {};
                    }

                    std::vector<int> tag;
                    std::string digits;
                    for (std::size_t i = prefix.size(); i <= end; ++i) {
                        const char c = (i < end) ? name[i] : '-';
                        if (c >= '0' && c <= '9') {
                            digits += c;
                        } else if (c == '-') {
                            if (digits.empty()) {
                                return {};
                            }

                            tag.push_back(std::stoi(digits));
                            digits.clear();
                        } else {
                            return {};
                        }
                    }

                    return tag;
                }

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
                    const std::string reg = config_.passVertexShaders.empty() ? "ps" : "vs";
                    std::vector<std::string> named;
                    std::string out;
                    for (const std::string& pass : passes) {
                        for (const std::string& tagged : taggedFor(pass)) {
                            if (std::find(named.begin(), named.end(), tagged) != named.end()) {
                                continue;
                            }

                            named.push_back(tagged);
                            out += (out.empty() ? "" : " || ") + reg + " == " + passFilters_[tagged];
                        }
                    }

                    return out;
                }

                std::string passCondition(const std::string& pass) {
                    return passCondition(std::vector<std::string>{pass});
                }

                std::string passFilter(const std::string& pass) {
                    // One filter_index per distinct shader, in order of first appearance over the
                    // slots -- except for a shader config.filterIndices names, which takes the value
                    // it is given there. 3dmigoto keys a [ShaderOverride] by its shader hash across
                    // EVERY loaded .ini, so two pairs that tag the same shader (both directions of one
                    // pair always do: the hair, face and eye shaders are the same on a skin and its
                    // character) must agree on its value, or whichever file loads last wins and the
                    // other mod's `if ps == ...` never matches -- its textures silently unbound.
                    if (passFilters_.empty()) {
                        std::size_t i = 0;
                        // every pass either table names -- an extraPassRegs pass with no
                        // filter_index has nothing to match on, so its command list never fires and
                        // that slot draws with the game's textures on that pass
                        std::vector<std::vector<std::string>> allPasses = config_.slotPasses;
                        for (const auto& [mesh, byPass] : config_.sharedMeshes) {
                            (void)mesh;
                            std::vector<std::string> names;
                            names.reserve(byPass.size());
                            for (const auto& [pass, regs] : byPass) {
                                (void)regs;
                                names.push_back(pass);
                            }

                            allPasses.push_back(std::move(names));
                        }

                        for (const auto& [slot, byPass] : config_.extraPassRegs) {
                            (void)slot;
                            std::vector<std::string> names;
                            names.reserve(byPass.size());
                            for (const auto& [pass, regs] : byPass) {
                                (void)regs;
                                names.push_back(pass);
                            }

                            allPasses.push_back(std::move(names));
                        }

                        for (const auto& passes : allPasses) {
                            for (const std::string& p : passes) {

                                for (const std::string& tagged : taggedFor(p)) {
                                    if (passFilters_.count(tagged) != 0) {
                                        continue;
                                    }

                                    auto given = config_.filterIndices.find(tagged);
                                    if (given != config_.filterIndices.end()) {
                                        passFilters_[tagged] = given->second;
                                    } else {
                                        passFilters_[tagged] = NumTools::formatDouble(config_.filterBase + config_.filterStep * static_cast<double>(i));
                                        ++i;
                                    }

                                    passOrder_.push_back(tagged);
                                }
                            }
                        }
                    }

                    return passFilters_[pass];
                }

                // The merge list the fix supplies per target slot for a legacy mod
                std::string mergeListName(int slot) const {
                    return fixName("CommandListMergeSlot" + std::to_string(slot));
                }

                // What a mod from before WWMI's merged skeleton has none of: the two skeleton buffers and
                // their read-only copies, the marker on the game's bone-data constant buffer, and one
                // merge list per target slot -- each gated on the marker, so a pass with something else in
                // that slot cannot merge junk into the skeleton.
                std::string legacySkeletonSections() {
                    const std::string merged = fixName(IniKeywords::Resource + std::string("MergedSkeleton"));
                    const std::string mergedRW = fixName(IniKeywords::Resource + std::string("MergedSkeletonRW"));
                    const std::string extra = fixName(IniKeywords::Resource + std::string("ExtraMergedSkeleton"));
                    const std::string extraRW = fixName(IniKeywords::Resource + std::string("ExtraMergedSkeletonRW"));
                    std::string out = SectionText(z3_, merged).str() + SectionText(z3_, extra).str();
                    for (const std::string& name : {mergedRW, extraRW}) {
                        out += SectionText(z3_, name)
                                   .keys({{IniKeywords::Type, "RWBuffer"},
                                          {IniKeywords::Format, "R32G32B32A32_FLOAT"},
                                          {"array", std::to_string(config_.mergedSkeletonSlots)}})
                                   .str();
                    }

                    out += SectionText(z3_, fixName("TextureOverrideMarkBoneDataCB"))
                               .keys({{IniKeywords::Hash, target_.cb4Hash},
                                      {"match_priority", "0"},
                                      {"filter_index", config_.boneDataFilter}})
                               .str();

                    // The private skeleton a blend remap needs, gathered right after the merge:
                    // SkeletonRemapper writes remapped[i] = merged[forward[i]], so a vertex whose
                    // blend names LOCAL i reaches the bone forward[i] -- which is how an 8-bit index
                    // addresses a 420-slot skeleton at all. Bound in place of the merged one.
                    const std::string remappedRW = fixName(IniKeywords::Resource + std::string("RemappedSkeletonRW"));
                    const std::string remapped = fixName(IniKeywords::Resource + std::string("RemappedSkeleton"));
                    const std::string extraRemappedRW = fixName(IniKeywords::Resource + std::string("ExtraRemappedSkeletonRW"));
                    const std::string extraRemapped = fixName(IniKeywords::Resource + std::string("ExtraRemappedSkeleton"));
                    if (targetPast256_) {
                        for (const std::string& name : {remapped, extraRemapped, remappedRW, extraRemappedRW}) {
                            out += SectionText(z3_, name).str();
                        }
                    }

                    for (std::size_t slot = 0; slot < target_.slots.size(); ++slot) {
                        const Slot& s = target_.slots[slot];
                        SectionText mergeList(z3_, mergeListName(static_cast<int>(slot)));
                        for (const auto& cb : {std::make_tuple(std::string("vs-cb4"), mergedRW, merged, remappedRW, remapped),
                                               std::make_tuple(std::string("vs-cb3"), extraRW, extra, extraRemappedRW, extraRemapped)}) {
                            mergeList.open(std::get<0>(cb) + " == " + config_.boneDataFilter)
                                     .keys({{VgOffsetKey, s.vgOffset},
                                            {VgCountKey, s.vgCount},
                                            {"$\\WWMIv1\\custom_mesh_scale", "1.00"},
                                            {"cs-cb8", IniKeywords::Ref + " " + std::get<0>(cb)},
                                            {"cs-u6", std::get<1>(cb)},
                                            {IniKeywords::Run, "CustomShader\\WWMIv1\\SkeletonMerger"},
                                            {std::get<2>(cb), "copy " + std::get<1>(cb)}});

                            if (targetPast256_) {
                                mergeList.keys({{"cs-t37", fixName(IniKeywords::Resource + std::string("BlendRemapForwardBuffer"))},
                                                {"$\\WWMIv1\\blend_remap_id", "0"},
                                                {VgCountKey, std::to_string(blendRemapBones_)},
                                                {"cs-t38", std::get<2>(cb)},
                                                {"cs-u5", std::get<3>(cb)},
                                                {IniKeywords::Run, "CustomShader\\WWMIv1\\SkeletonRemapper"},
                                                {std::get<4>(cb), "copy " + std::get<3>(cb)},
                                                {std::get<0>(cb), std::get<4>(cb)}});
                            } else {
                                mergeList.key(std::get<0>(cb), std::get<2>(cb));
                            }

                            mergeList.close();
                        }

                        out += mergeList.str();
                    }

                    if (targetPast256_) {
                        out += blendRemapSections();
                    }

                    return out;
                }

                // The blend remap's resources, and the one-time run of WWMI's BlendRemapper that
                // rewrites the remapped blend's ids into the remap's LOCAL space. Guarded by a
                // `local`, so it costs one dispatch on the first drawn frame and nothing after.
                //
                // The strided view is the fix's own declaration of the same file rather than the
                // collect's resource: `copy_desc` needs a buffer with the blend's stride, and
                // declaring it here keeps this text independent of what the collect happened to
                // name its resource.
                std::string blendRemapSections() {
                    const std::string vertexVG = fixName(IniKeywords::Resource + std::string("BlendRemapVertexVGBuffer"));
                    const std::string forward = fixName(IniKeywords::Resource + std::string("BlendRemapForwardBuffer"));
                    const std::string reverse = fixName(IniKeywords::Resource + std::string("BlendRemapReverseBuffer"));
                    const std::string noStride = fixName(IniKeywords::Resource + std::string("BlendNoStride"));
                    const std::string strided = fixName(IniKeywords::Resource + std::string("BlendStrided"));
                    const std::string blendRW = fixName(IniKeywords::Resource + std::string("RemappedBlendBufferRW"));
                    const std::string blendOut = fixName(IniKeywords::Resource + std::string("RemappedBlendBuffer"));
                    const std::string blendPath = blendFixedFile();

                    std::string out;
                    // no stride on the three the compute shader reads: a Buffer declared with one
                    // cannot be addressed by a compute shader (wwmiIdentityMod.py's own note)
                    for (const auto& entry : {std::make_pair(vertexVG, blendRemapFile("VertexVG")),
                                              std::make_pair(forward, blendRemapFile("Forward")),
                                              std::make_pair(reverse, blendRemapFile("Reverse")),
                                              std::make_pair(noStride, blendPath)}) {
                        out += SectionText(z3_, entry.first)
                                   .keys({{IniKeywords::Type, "Buffer"},
                                          {IniKeywords::Format, entry.first == noStride ? "DXGI_FORMAT_R8_UINT"
                                                                             : "DXGI_FORMAT_R16_UINT"},
                                          {IniKeywords::Filename, entry.second}})
                                   .str();
                    }

                    out += SectionText(z3_, strided)
                               .keys({{IniKeywords::Type, "Buffer"},
                                      {IniKeywords::Format, "DXGI_FORMAT_R8_UINT"},
                                      {IniKeywords::Stride, std::to_string(blendStride_)},
                                      {IniKeywords::Filename, blendPath}})
                               .str();
                    out += SectionText(z3_, blendRW).str();
                    out += SectionText(z3_, blendOut).str();

                    out += SectionText(z3_, blendRemapInitList())
                               .key("local $blendRemapReady")
                               .open("!$blendRemapReady")
                               .keys({{"$\\WWMIv1\\custom_vertex_count", "$mesh_vertex_count"},
                                      {"$\\WWMIv1\\weights_per_vertex_count", std::to_string(blendInfluences_)},
                                      {"$\\WWMIv1\\blend_remap_id", "0"},
                                      {"cs-t34", IniKeywords::Ref + " " + reverse},
                                      {"cs-t35", IniKeywords::Ref + " " + vertexVG},
                                      {blendRW, "copy " + noStride},
                                      {"cs-u4", IniKeywords::Ref + " " + blendRW},
                                      {IniKeywords::Run, "CustomShader\\WWMIv1\\BlendRemapper"},
                                      {blendOut, "copy " + blendRW},
                                      {blendOut, "copy_desc " + strided},
                                      {"$blendRemapReady", "1"}})
                               .close()
                               .key(config_.blendReg, blendOut)
                               .str();
                    return out;
                }

                // ---- the fix's own sections ----

                void buildAppended() {
                    std::string out;
                    for (const auto& entry : declared_) {
                        out += SectionText(z3_, declaredName_[entry.first])
                                   .key(IniKeywords::Filename, entry.second)
                                   .str();
                    }

                    // its OWN name -- see Fallback::resource. resourceOfRole_ may by now be the
                    // resource of a texture EDIT of this role, and writing the download's section
                    // under that name declares it twice, raw file and edited file.
                    for (const auto& entry : fallbacks_) {
                        out += SectionText(z3_, entry.second.resource)
                                   .key(IniKeywords::Filename, entry.second.relPath)
                                   .str();
                    }

                    if (config_.zeroShapeKeyStream && meshVertexCount_ > 0) {
                        out += SectionText(z3_, fixName(IniKeywords::Resource + ShapeKeyZero))
                                   .keys({{IniKeywords::Type, "Buffer"},
                                          {IniKeywords::Format, "DXGI_FORMAT_R32G32B32_FLOAT"},
                                          {IniKeywords::Stride, std::to_string(config_.shapeKeyStride)},
                                          {IniKeywords::Filename, zeroStreamFile()}})
                                   .str();
                    }

                    for (const std::string& list : textureLists_) {
                        out += list + "\n";
                    }

                    for (std::size_t slot = 0; slot < target_.slots.size(); ++slot) {
                        if (drawnSlots_.count(static_cast<int>(slot)) > 0) {
                            continue;
                        }

                        const Slot& s = target_.slots[slot];
                        auto label = config_.targetLabels.find(static_cast<int>(slot));
                        const std::string labelText = label == config_.targetLabels.end() ? std::to_string(slot) : label->second;
                        const std::string state = "$state_id_" + std::to_string(slot);
                        SectionText hide(z3_, IniKeywords::TextureOverride + toModName_
                                                 + TextTools::capitalize(config_.slotPrefix)
                                                 + std::to_string(slot) + IniKeywords::Remap + "Hide");
                        hide.prefix("; nothing of the mod is drawn through " + toModName_ + "'s " + labelText
                                    + " slot: the skin's own geometry is skipped and its bones still merged")
                            .keys({{IniKeywords::Hash, target_.vb0Hash},
                                   {IniKeywords::MatchFirstIndex, s.indexOffset},
                                   {IniKeywords::MatchIndexCount, s.indexCount},
                                   {"$object_detected", "1"}})
                            .open("$mod_enabled");

                        if (legacy_) {
                            hide.keys({{IniKeywords::Run, mergeListName(slot)}, {"handling", "skip"}});
                        } else {
                            hide.key("local " + state)
                                .open(state + " != $state_id")
                                .keys({{state, "$state_id"},
                                       {VgOffsetKey, s.vgOffset},
                                       {VgCountKey, s.vgCount},
                                       {IniKeywords::Run, fixName("CommandListMergeSkeleton")}})
                                .close()
                                .open("ResourceMergedSkeleton !== null")
                                .key("handling", "skip")
                                .close();
                        }

                        out += hide.close().str();
                    }

                    if (legacy_ || targetPast256_) {
                        out += legacySkeletonSections();
                    }

                    passFilter("");
                    std::size_t i = 0;
                    for (const std::string& pass : passOrder_) {
                        out += SectionText(z3_, fixName("ShaderOverridePass" + std::to_string(i)))
                                   .keys({{IniKeywords::Hash, pass}, {"filter_index", passFilters_[pass]}})
                                   .str();
                        ++i;
                    }

                    // Other meshes the character draws -- see WWMIFixerConfig::sharedMeshes
                    std::size_t meshNum = 0;
                    for (const auto& [meshHash, byPass] : config_.sharedMeshes) {
                        std::string body;
                        for (const auto& [pass, bindings] : byPass) {
                            std::string lines;
                            for (const WWMIFixerConfig::Binding& binding : bindings) {
                                const std::string* resource = sharedResourceFor(binding.role);
                                if (resource != nullptr) {
                                    lines += bindLine(binding.role, binding.reg, *resource) + "\n";
                                }
                            }

                            if (!lines.empty()) {
                                body += "if " + passCondition(pass) + "\n" + lines + "endif\n";
                            }
                        }

                        if (!body.empty()) {
                            out += "[" + fixName(IniKeywords::TextureOverride + source_.name + "SharedMesh"
                                                 + std::to_string(meshNum)) + "]\n"
                                   + IniKeywords::Hash + " = " + meshHash + "\n" + body + "\n";
                        }

                        ++meshNum;
                    }

                    if (texcoordSection_.has_value()) {
                        out += *texcoordSection_;
                    }

                    for (const auto& edited : editedResources_) {
                        out += SectionText(z3_, edited.first)
                                   .key(IniKeywords::Filename, edited.second)
                                   .str();
                    }

                    for (const WWMIFixerConfig::CreatedTexture& created : config_.createdTextures) {
                        out += SectionText(z3_, resourceOfRole_[created.role])
                                   .key(IniKeywords::Filename, createdTextureFile(created))
                                   .str();
                    }

                    for (const std::string& list : roleListTexts_) {
                        out += list + "\n";
                    }

                    this->appendedSections = std::string(StringTools::rstrip(out));
                }

                // The three blend remap buffers, named so the undo takes them: a file the fix
                // wrote carries the target's name and `Remap`, and the resource section that names
                // it lives inside the fix's block.
                std::string blendRemapInitList() const {
                    return fixName("CommandListBlendRemap");
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
                    return meshFolder_ + "/" + toModName_ + IniKeywords::Remap + "BlendRemap" + which + ".buf";
                }

                std::string zeroStreamFile() const {
                    return meshFolder_ + "/" + toModName_ + IniKeywords::Remap + ShapeKeyZero + ".buf";
                }

                std::string createdTextureFile(const WWMIFixerConfig::CreatedTexture& created) const {
                    return textureFolder_ + "/" + created.role + toModName_ + IniKeywords::RemapTex + FileExt::DDS;
                }

                // ---- at fix time ----

                void writeZeroStream() {
                    if (!config_.zeroShapeKeyStream || meshVertexCount_ <= 0) {
                        return;
                    }

                    const std::string path = FileService::absPathOfRelPath(zeroStreamFile(), ctx_.getIniFile()->getFolder());
                    const std::filesystem::path fsPath = FileService::strToPath(path);
                    const std::uintmax_t size = static_cast<std::uintmax_t>(meshVertexCount_) * static_cast<std::uintmax_t>(config_.shapeKeyStride);
                    std::error_code err;
                    if (std::filesystem::is_regular_file(fsPath, err) && std::filesystem::file_size(fsPath, err) == size && !err) {
                        return;
                    }

                    std::filesystem::create_directories(fsPath.parent_path(), err);
                    std::ofstream out(fsPath, std::ios::binary);
                    const std::vector<char> zeros(static_cast<std::size_t>(size), 0);
                    out.write(zeros.data(), static_cast<std::streamsize>(zeros.size()));
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
                    if (config_.texEdits.empty() || gaveUp_) {
                        return;
                    }

                    const std::string folder = ctx_.getIniFile()->getFolder();
                    for (const WWMIFixerConfig::TexEdit& edit : config_.texEdits) {
                        if (!edit.makeFilter || sharedResourceFor(edit.role) == nullptr) {
                            continue;
                        }

                        // the mod's own file, or -- for a role it has none for -- the one the
                        // fallback download lands
                        std::string source;
                        const auto srcAt = fileOfRole_.find(edit.role);
                        if (srcAt != fileOfRole_.end()) {
                            source = srcAt->second;
                        } else {
                            const auto back = fallbacks_.find(edit.role);
                            if (back == fallbacks_.end()) {
                                continue;
                            }

                            source = FileService::absPathOfRelPath(back->second.relPath, folder);
                        }

                        const std::string fixedRel = textureFolder_ + "/" + config_.downloadPrefix
                                                     + TextTools::capitalize(edit.role) + edit.name + IniKeywords::RemapTex + FileExt::DDS;
                        plannedEdits_.push_back(PlannedEdit{&edit, source, fixedRel});

                        // every binding of the role follows the edited file
                        const std::string resource = fixName(IniKeywords::Resource + TextTools::capitalize(edit.role) + edit.name
                                                             + IniKeywords::RemapTex);

                        // Which of the mod's resources this replaces, so a copied toggle chain can
                        // swap it in.
                        const std::string* was = sharedResourceFor(edit.role);
                        if (was != nullptr) {
                            editedResourceOf_[StringTools::toLower(*was)] = resource;
                            editedRoleOf_[StringTools::toLower(*was)] = edit.role;
                            sourceOfEdited_[StringTools::toLower(resource)] = *was;

                            // EVERY variant of the role, not just the one fileOfRole_ resolved to.
                            // An edit reads one file, so a mod that binds its mask two ways -- Chisa12
                            // does, on $sockscolor -- had the second variant carried through
                            // unrepacked and shaded as the wrong material. Harmless while nothing
                            // could reach that variant; a live defect once the toggle chains could.
                            //
                            // The resolved variant keeps the name it already had, so no shipped
                            // output moves; the others are indexed after it.
                            const auto owner = conditionalOwner_.find(StringTools::toLower(*was));
                            if (owner != conditionalOwner_.end()) {
                                std::size_t n = 1;
                                for (const std::string& variant : variantsOf_[owner->second]) {
                                    if (StringTools::equalsIgnoreCase(variant, *was)) {
                                        continue;
                                    }

                                    const auto file = fileOfResource_.find(StringTools::toLower(variant));
                                    if (file == fileOfResource_.end()) {
                                        continue;
                                    }

                                    ++n;
                                    const std::string suffix = std::to_string(n);
                                    const std::string variantRel =
                                        textureFolder_ + "/" + config_.downloadPrefix + TextTools::capitalize(edit.role)
                                        + edit.name + suffix + IniKeywords::RemapTex + FileExt::DDS;
                                    const std::string variantResource =
                                        fixName(IniKeywords::Resource + TextTools::capitalize(edit.role) + edit.name + suffix
                                                + IniKeywords::RemapTex);
                                    plannedEdits_.push_back(PlannedEdit{&edit, file->second, variantRel});
                                    editedResources_.emplace_back(variantResource, variantRel);
                                    editedResourceOf_[StringTools::toLower(variant)] = variantResource;
                                    editedRoleOf_[StringTools::toLower(variant)] = edit.role;
                                    sourceOfEdited_[StringTools::toLower(variantResource)] = variant;
                                }

                                if (n > 1) {
                                    ctx_.log(edit.role + ": " + std::to_string(n) + " variants bound by "
                                             + owner->second + ", each given its own " + edit.name + " edit");
                                }
                            }
                        }
                        editedResources_.emplace_back(resource, fixedRel);
                        resourceOfRole_[edit.role] = resource;
                        for (auto& entry : resourceOfSlotRole_) {
                            if (entry.first.first == edit.role) {
                                entry.second = resource;
                            }
                        }
                    }
                }

                void addTexEdits() {
                    if (plannedEdits_.empty()) {
                        return;
                    }

                    IniFile* ini = ctx_.getIniFile();
                    const std::string folder = ini->getFolder();

                    WWMIFixerConfig::TexEditContext context;
                    context.iniFolder = folder;
                    context.indexFile = FileService::absPathOfRelPath(indexFile_, folder);
                    context.positionFile = FileService::absPathOfRelPath(positionFile_, folder);
                    context.texcoordFile = FileService::absPathOfRelPath(texcoordFile_, folder);
                    std::error_code err;

                    context.drawRanges = drawRanges_;
                    for (const auto& entry : fileOfRole_) {
                        context.fileOfRole[entry.first] = entry.second;
                    }

                    for (const auto& entry : fallbacks_) {
                        context.fileOfRole.emplace(
                            entry.first, FileService::absPathOfRelPath(entry.second.relPath, folder));
                    }

                    for (const PlannedEdit& planned : plannedEdits_) {
                        TexEditor::Filter filter = planned.edit->makeFilter(context);
                        if (!filter) {
                            continue;
                        }

                        const std::string fixedPath = FileService::absPathOfRelPath(planned.fixedRel, folder);
                        std::filesystem::create_directories(FileService::strToPath(fixedPath).parent_path(), err);
                        ini->getResources().push_back(std::make_unique<RemapTexEditResource>(
                            folder, planned.source, fixedPath,
                            TexEditor({std::move(filter)}, planned.edit->compress)));
                    }
                }

                // A remap-only copy of the mod's texcoord buffer -- see
                // WWMIFixerConfig::cleanTexcoords for what is wrong with the original and why
                // neither fault is the mod's bug.
                void buildTexcoordCopy() {
                    if (!config_.cleanTexcoords || texcoordResource_.has_value()) {
                        return;
                    }

                    IniFile* ini = ctx_.getIniFile();
                    if (ini == nullptr) {
                        return;
                    }

                    // WITHOUT REGARD TO CASE: this mod spells it ResourceTexCoordBuffer, and asking
                    // for ResourceTexcoordBuffer finds nothing and says nothing.
                    const IfTemplate<std::string, std::string>* resource = nullptr;
                    for (const auto& entry : ini->getIfTemplates()) {
                        if (entry.second != nullptr
                            && StringTools::equalsIgnoreCase(entry.first, TexcoordBufferResource)) {
                            resource = entry.second.get();
                            break;
                        }
                    }

                    if (resource == nullptr) {
                        note("this mod declares no " + TexcoordBufferResource
                             + ", so its UVs are left alone");
                        return;
                    }

                    const std::optional<std::string> name = ModBranches::firstVal(*resource, IniKeywords::Filename);
                    if (!name.has_value()) {
                        note(TexcoordBufferResource + " names no file, so this mod's UVs are left alone");
                        return;
                    }

                    const std::string rel = FileService::iniPathToRel(*name);
                    const std::string path = FileService::absPathOfRelPath(rel, ini->getFolder());
                    // A DECLARED STRIDE THAT DOES NOT PARSE IS NOT A REASON TO GUESS ONE. This
                    // fell back to 16, which mis-reads every vertex of a buffer that is not 16 -- the
                    // mod said a number and the fix ignored it.
                    const std::optional<std::string> strideVal = ModBranches::firstVal(*resource, IniKeywords::Stride);
                    std::size_t stride = DefaultTexcoordStride;
                    if (strideVal.has_value()) {
                        try {
                            stride = static_cast<std::size_t>(std::stoul(StringTools::strip(*strideVal).data()));
                        } catch (const std::exception&) {
                            note(TexcoordBufferResource + " declares a stride of `"
                                 + std::string(StringTools::strip(*strideVal))
                                 + "`, which is not a number, so this mod's UVs are left alone");
                            return;
                        }
                    }

                    if (stride < 4 || stride % 2 != 0) {
                        note(TexcoordBufferResource + " declares a stride of " + std::to_string(stride)
                             + ", which cannot hold a UV pair of halves, so this mod's UVs are left alone");
                        return;
                    }

                    // One element of `stride / 2` halves, rounded the way a folded UV needs. That
                    // mode is the whole reason this can be a BufFile at all -- see the note where
                    // this file's own half codec used to be.
                    const std::size_t perVertex = stride / 2;
                    auto texcoordElements = [perVertex]() {
                        std::vector<std::unique_ptr<BufDataType>> halves;
                        for (std::size_t i = 0; i < perVertex; ++i) {
                            halves.push_back(std::make_unique<BufFloat16>(false,
                                                                          BufFloat16::Rounding::NearestEven));
                        }

                        std::vector<std::unique_ptr<BufElementType>> elements;
                        elements.push_back(std::make_unique<BufElementType>(TexcoordElement, "",
                                                                           std::move(halves)));
                        return elements;
                    };

                    BufFile texcoord{path, texcoordElements()};
                    if (!texcoord.isValid()) {
                        return;                         // not a whole number of vertices: leave it alone
                    }

                    // Pass one: every vertex's U, with a NaN read as the 0 the clean pass will make
                    // it -- the straddle test below has to see the same values the write does.
                    std::vector<float> u;
                    const BufFile::Filter collect =
                        [&u](const BufLineData& line, long long, double, long long) {
                            const auto at = line.find(TexcoordElement);
                            if (at != line.end() && !at->second.empty()
                                    && std::holds_alternative<double>(at->second.front())) {
                                const double value = std::get<double>(at->second.front());
                                u.push_back(std::isnan(value) ? 0.0f : static_cast<float>(value));
                            } else {
                                u.push_back(0.0f);
                            }

                            return line;
                        };

                    try {
                        texcoord.fix(std::nullopt, {collect});
                    } catch (const std::exception& exception) {
                        note(std::string("its Texcoord.buf could not be read: ") + exception.what()
                             + " -- so this mod's UVs are left alone");
                        return;
                    }

                    const std::size_t vertices = u.size();
                    if (vertices == 0) {
                        return;
                    }

                    // U at or above 1, except on a triangle whose vertices straddle a tile.
                    //
                    // NOT U BELOW 0 (2026-09-27). The fold exists for a mod that UVs half a part
                    // into the [1, 2) TILE and relies on the sampler wrapping -- a large, coherent,
                    // deliberate region (47% of one Chisa mod's component 3). A U just BELOW zero is
                    // the opposite thing: the edge bleed an authoring tool leaves around a UV
                    // island, a fringe a few hundredths wide that a clamping sampler is meant to
                    // extend. Folding it sends those texels to the FAR SIDE of the atlas --
                    // `-0.054` became `0.945` -- so the fringe of every island sampled unrelated
                    // art. On Chisa13's black hair dye that drew a regular checkerboard of blonde
                    // blocks through the lock: 905 such vertices on the bangs and 2 elsewhere in
                    // the mesh, which is why one part of one mod showed it.
                    //
                    // The fold is wrap-equivalent per VERTEX, which is what was measured when it
                    // went in, and that holds only while the pass WRAPS. Neither character's own
                    // model leaves [0, 1), so the game never exercises its own address mode there
                    // and the two passes are free to differ -- and this one demonstrably does not
                    // wrap: the mod's own UVs and a fold restricted to `>= 1` both render the dye
                    // as one solid sweep, and the full fold does not. Leaving the fringe alone is
                    // also the pre-fold behaviour for it, so it cannot regress a mod that was
                    // right before the fold existed.
                    //
                    // Folding one corner of a straddling triangle would widen its U span from a few
                    // hundredths to nearly 1 and interpolate it backwards across the atlas, so such
                    // a vertex is left alone -- which also protects deliberate TILING.
                    //
                    // Still read at 4 bytes per index, which is what the hand-rolled loop assumed;
                    // `IbFile::bytesPerIndexOf(<the declared format>)` is how a mod declaring
                    // `DXGI_FORMAT_R16_UINT` gets answered, and that is a BEHAVIOUR change rather
                    // than a refactor. A failure here leaves `keep` false, which folds more rather
                    // than less -- the same answer the unopenable-file branch gave before.
                    std::vector<bool> keep(vertices, false);
                    if (!indexFile_.empty()) {
                        const std::string indexPath = FileService::absPathOfRelPath(
                            FileService::iniPathToRel(indexFile_), ini->getFolder());
                        try {
                            IbFile indices{indexPath};
                            const BufFile::Filter straddles =
                                [&keep, &u, vertices](const BufLineData& line, long long, double, long long) {
                                    const auto at = line.find(IbFile::TriangleBufElementKey);
                                    if (at == line.end() || at->second.size() < IbFile::VerticesPerTriangle) {
                                        return line;
                                    }

                                    std::size_t tri[3] = {0, 0, 0};
                                    for (std::size_t i = 0; i < IbFile::VerticesPerTriangle; ++i) {
                                        if (!std::holds_alternative<unsigned long long>(at->second[i])) {
                                            return line;
                                        }

                                        tri[i] = static_cast<std::size_t>(
                                            std::get<unsigned long long>(at->second[i]));
                                        if (tri[i] >= vertices) {
                                            return line;
                                        }
                                    }

                                    const float a = std::floor(u[tri[0]]);
                                    const float b = std::floor(u[tri[1]]);
                                    const float c = std::floor(u[tri[2]]);
                                    if (a != b || b != c) {
                                        keep[tri[0]] = true;
                                        keep[tri[1]] = true;
                                        keep[tri[2]] = true;
                                    }

                                    return line;
                                };

                            indices.fix(std::nullopt, {straddles});
                        } catch (const std::exception&) {
                            // no index buffer to read: every vertex stays foldable, as before
                        }
                    }

                    // A NaN in any half set to 0, and the fold in U. A NaN is `std::isnan` on the
                    // decoded value now rather than an exponent and mantissa test on the raw bits.
                    std::size_t cleared = 0;
                    std::size_t folded = 0;
                    const BufFile::Filter clean =
                        [&cleared, &folded, &keep, vertices](const BufLineData& line, long long,
                                                             double index, long long) {
                            BufLineData out = line;
                            const auto at = out.find(TexcoordElement);
                            if (at == out.end() || at->second.empty()) {
                                return out;
                            }

                            for (BufValue& value : at->second) {
                                if (std::holds_alternative<double>(value)
                                        && std::isnan(std::get<double>(value))) {
                                    value = 0.0;
                                    ++cleared;
                                }
                            }

                            if (!std::holds_alternative<double>(at->second.front())) {
                                return out;
                            }

                            const auto vertex = static_cast<std::size_t>(index);
                            const auto first = static_cast<float>(std::get<double>(at->second.front()));
                            if (first >= 1.0f && vertex < vertices && !keep[vertex]) {
                                at->second.front() = static_cast<double>(std::fmod(first, 1.0f));
                                ++folded;
                            }

                            return out;
                        };

                    // Counted before anything is written, because the copy exists only when it
                    // differs -- a mod with clean texcoords keeps its own buffer and its own binding.
                    try {
                        texcoord.fix(std::nullopt, {clean});
                    } catch (const std::exception& exception) {
                        note(std::string("its Texcoord.buf could not be read: ") + exception.what()
                             + " -- so this mod's UVs are left alone");
                        return;
                    }

                    const std::size_t clearedCount = cleared;
                    const std::size_t foldedCount = folded;
                    if (clearedCount == 0 && foldedCount == 0) {
                        return;
                    }

                    const std::string fixedRel = meshFolder_ + "/" + toModName_ + IniKeywords::Remap + "Texcoord.buf";
                    const std::string fixedPath = FileService::absPathOfRelPath(fixedRel, ini->getFolder());
                    std::error_code err;
                    std::filesystem::create_directories(FileService::strToPath(fixedPath).parent_path(), err);

                    // The same filter again, writing this time -- so the counters run up a second
                    // time and the snapshot above is what the log reports.
                    //
                    // `fix(path, ...)` RETURNS THE PATH ON SUCCESS: the string alternative of
                    // FixResult means "written to this file", not "failed", and a failure throws.
                    // Reading it as an error is what made an earlier version of this write the
                    // buffer correctly and then never bind it.
                    try {
                        texcoord.fix(fixedPath, {clean});
                    } catch (const std::exception& exception) {
                        note(std::string("its cleaned Texcoord.buf could not be measured: ") + exception.what()
                             + " -- so this mod's UVs are left alone");
                        return;
                    }

                    const std::optional<std::string> format = ModBranches::firstVal(*resource, IniKeywords::Format);
                    texcoordResource_ = fixName(IniKeywords::Resource + "TexcoordNoNaN");
                    texcoordSection_ = SectionText(z3_, *texcoordResource_)
                                           .keys({{IniKeywords::Type, "Buffer"},
                                                  {IniKeywords::Format, std::string(format.has_value()
                                                                             ? StringTools::strip(*format)
                                                                             : std::string_view("DXGI_FORMAT_R16G16_FLOAT"))},
                                                  {IniKeywords::Stride, std::to_string(stride)},
                                                  {IniKeywords::Filename, fixedRel}})
                                           .str();
                    ctx_.log("texcoords: " + std::to_string(clearedCount) + " NaN halves set to 0 and "
                             + std::to_string(foldedCount) + " U values folded into [0, 1) in a remap-only copy");
                }

                void addCreatedTextures() {
                    IniFile* ini = ctx_.getIniFile();
                    for (const WWMIFixerConfig::CreatedTexture& created : config_.createdTextures) {
                        const std::string path = FileService::absPathOfRelPath(createdTextureFile(created), ini->getFolder());
                        bool known = false;
                        for (const auto& resource : ini->getResources()) {
                            if (resource != nullptr && resource->srcPath == path) {
                                known = true;
                                break;
                            }
                        }

                        if (known) {
                            continue;
                        }

                        std::error_code err;
                        std::filesystem::create_directories(FileService::strToPath(path).parent_path(), err);
                        ini->getResources().push_back(std::make_unique<RemapTexAddResource>(
                            ini->getFolder(), path, TexCreator(created.size, created.size, created.colour, false, false)));
                    }
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

                void addFallbackDownloads() {
                    IniFile* ini = ctx_.getIniFile();
                    for (const auto& entry : fallbacks_) {
                        const std::string path = FileService::absPathOfRelPath(entry.second.relPath, ini->getFolder());
                        bool known = false;
                        for (const auto& resource : ini->getFileDownloads()) {
                            if (resource != nullptr && resource->srcPath == path) {
                                known = true;
                                break;
                            }
                        }

                        if (known) {
                            continue;
                        }

                        std::error_code err;
                        std::filesystem::create_directories(FileService::strToPath(path).parent_path(), err);
                        ini->getFileDownloads().push_back(std::make_unique<RemapIniDownload>(
                            ini->getFolder(), entry.second.relPath,
                            std::make_unique<FileDownload>(entry.second.url, entry.second.fileName)));
                    }
                }

                IniFileFixContext ctx_;
                std::string toModName_;
                WWMIFixerConfig config_;
                bool gaveUp_ = false;
                GraphGroupRemove<> removeEveryGroup_;

                std::optional<ModType> targetType_;
                Character source_;
                Character target_;
                std::map<int, std::vector<std::string>> present_;     // source component -> its sections
                std::vector<int> dropped_;
                std::vector<std::vector<int>> groups_;
                std::set<int> drawnSlots_;
                long long meshVertexCount_ = 0;
                std::string meshFolder_;
                std::string textureFolder_;
                std::map<std::string, std::string> resourceOfRole_;   // role -> resource, for what every component shares (a created texture, a download)
                std::map<std::pair<std::string, int>, std::string> resourceOfSlotRole_;   // (role, source component) -> the resource that component binds
                std::vector<std::pair<std::string, std::string>> declared_;   // (file, path relative to the .ini) for a file no resource of the .ini names
                std::map<std::string, std::string> declaredName_;      // that file -> the resource section the fix declares for it
                std::set<std::string> usedDeclaredNames_;
                std::map<std::string, Fallback> fallbacks_;           // role -> the source's game texture, for a planned role the mod has no file for
                std::set<std::string> leftToGame_;                    // roles whose only file was flat and whose config says not to stand anything in
                std::vector<std::string> textureLists_;
                std::unordered_map<std::string, std::string> passFilters_;



                bool legacy_ = false;                                 // a mod from before WWMI's merged skeleton
                std::string indexFile_ = "Meshes/Index.buf";           // as the mod's own [ResourceIndexBuffer] names it
                std::string positionFile_ = "Meshes/Position.buf";     // ...and [ResourcePositionBuffer]
                std::string blendSourceFile_ = "Meshes/Blend.buf";      // ...and [ResourceBlendBuffer]; blendFixedFile() derives the fix's name FROM it
                std::string texcoordFile_ = "Meshes/TexCoord.buf";     // ...and [ResourceTexcoordBuffer]
                std::map<int, std::vector<std::pair<long long, long long>>> drawRanges_;   // source component -> its (index count, first index) draws

                // Whether the TARGET's merged skeleton passes what an 8-bit blend index can name.
                // Read off the vertex group row's largest target id, which is a property of the
                // PAIR -- so the .ini and the buffers, written at different times, cannot disagree
                // about whether there is a blend remap.
                bool targetPast256_ = false;

                // How many bones the one remap holds -- the DISTINCT targets the vertex group row
                // names, so the .ini (written first) and the buffers agree without either having to
                // read the other. 182 for ChisaParfait -> Chisa.
                std::size_t blendRemapBones_ = 0;

                // The mod's own blend line: N ids + N weights, and its byte stride
                std::size_t blendInfluences_ = 4;
                std::size_t blendStride_ = 8;
                std::vector<std::string> passOrder_;

                std::unique_ptr<GraphGroupRemap<>> slotRemap_;
                std::unique_ptr<RegAssetRemap<>> assetRemap_;
                std::unique_ptr<RegPartEdit<>> assetAdapter_;
                std::unique_ptr<RegRemove<>> regRemove_;
                std::unique_ptr<RegPartEdit<>> removeAdapter_;
                std::map<int, PartEdit*> newValsOf_;
                std::vector<std::unique_ptr<RegSurroundedAdd<>>> surroundedAdds_;
                std::vector<std::unique_ptr<GraphPartEdit<>>> graphAdapters_;
                std::vector<std::unique_ptr<RegNewVals<>>> newVals_;
                std::vector<std::unique_ptr<RegPartEdit<>>> regAdapters_;
                std::map<int, std::vector<PartEdit*>> editsOf_;
                std::unique_ptr<ObjGroupEdit> mainEdits_;
                std::vector<std::unique_ptr<WWMIBlendReplace>> blendReplaces_;
                std::optional<std::string> vertexVGFile_;                            // WWMI's 16-bit ids, as the .ini names them
                std::optional<std::string> texcoordResource_;                        // the cleaned texcoord copy's resource
                std::optional<std::string> texcoordSection_;                         // ...and its section text
                std::unordered_map<std::string, std::string> fileOfRole_;            // role -> the file it resolved to
                // One texture edit, named and sourced at read time and registered at fix time
                struct PlannedEdit {
                    const WWMIFixerConfig::TexEdit* edit = nullptr;   // owned by config_, so stable
                    std::string source;                               // absolute, the mod's file or the fallback's
                    std::string fixedRel;                             // relative to the .ini
                };

                std::unordered_map<std::string, std::string> conditionalOwner_;      // the mod's resource -> the section that binds it behind a condition
                std::unordered_map<std::string, std::vector<std::string>> variantsOf_;  // that section -> every resource it binds, in order
                std::unordered_map<std::string, std::string> fileOfResource_;         // the mod's resource -> the file it names
                std::unordered_map<std::string, std::string> editedResourceOf_;       // the mod's resource -> the edited copy of it
                std::unordered_map<std::string, std::string> editedRoleOf_;           // ...and the ROLE that edit was registered for, since one file may serve several
                std::unordered_map<std::string, std::string> sourceOfEdited_;         // ...and back
                std::map<std::pair<std::string, std::string>, std::string> roleLists_;   // (role, register) -> the copied list's name
                std::vector<std::string> roleListTexts_;                              // ...and their section text
                std::vector<PlannedEdit> plannedEdits_;
                std::vector<std::pair<std::string, std::string>> editedResources_;   // (resource, path relative to the .ini)
                std::vector<std::unique_ptr<Collector>> blendCollects_;
        };
    }


    IniFixBuilder::Factory makeWWMIFixer(WWMIFixerConfig config) {
        return [config](BaseIniParser<>* parser, const std::string& toModName, std::optional<int> modTypeId) {
            return std::make_shared<WWMIFixerImpl>(parser, toModName, modTypeId, config);
        };
    }
}
