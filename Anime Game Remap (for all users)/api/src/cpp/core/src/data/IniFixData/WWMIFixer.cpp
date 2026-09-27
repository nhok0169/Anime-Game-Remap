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

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniFixData/ModBranches.h"
#include "AGRemapCore/model/IniNamingTools.h"
#include "AGRemapCore/model/assets/Hashes.h"
#include "AGRemapCore/model/Version.h"
#include "AGRemapCore/model/buffers/BufElementType.h"
#include "AGRemapCore/model/buffers/BufInt.h"
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
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegRemove.h"

#include <stdexcept>
#include "AGRemapCore/model/strategies/iniParsers/GIMIParser.h"
#include "AGRemapCore/model/strategies/texEditors/TexCreator.h"
#include "AGRemapCore/tools/NumTools.h"
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
        const std::string MatchIndexCountKey = "match_index_count";
        const std::string MeshVertexCountKey = "global $mesh_vertex_count";
        const std::string ConstantsSection = "Constants";
        const std::string BlendBufferResource = "ResourceBlendBuffer";
        const std::string IndexBufferResource = "ResourceIndexBuffer";
        const std::string PositionBufferResource = "ResourcePositionBuffer";
        const std::string TexcoordBufferResource = "ResourceTexcoordBuffer";
        const std::string Cb4HashKey = "cb4";
        constexpr std::size_t WWMIBlendStride = 8;      // four R8 bone indices then four R8 weights

        // IEEE half <-> float, for the texcoord copy. A WWMI texcoord buffer is halves, and the two
        // faults it can carry -- a NaN in the second UV, a U outside [0, 1) -- are read and written
        // in that format rather than converted through the whole buffer.
        // A half-float pair of this fix's OWN, deliberately not model/buffers/BufFloat.cpp's pair of
        // the same name -- see the note there. These two are written for one job: reading and
        // rewriting a folded texture coordinate, matching numpy's rounding rather than the
        // truncation a general decoder does. Sharing either one moves UVs.
        float halfToFloat(std::uint16_t bits) {
            const int sign = (bits >> 15) & 0x1;
            const int exponent = (bits >> 10) & 0x1F;
            const int mantissa = bits & 0x3FF;
            float value = 0.0f;
            if (exponent == 0) {
                value = std::ldexp(static_cast<float>(mantissa), -24);
            } else if (exponent != 0x1F) {
                value = std::ldexp(static_cast<float>(mantissa + 1024), exponent - 25);
            }

            return sign ? -value : value;
        }

        std::uint16_t floatToHalf(float value) {
            if (!(value > 0.0f)) {
                return 0;                        // this is only ever handed a folded U in [0, 1)
            }

            int exponent = 0;
            const float scaled = std::frexp(value, &exponent);        // value = scaled * 2^exponent
            const int biased = exponent + 14;
            if (biased <= 0) {
                return 0;
            }

            if (biased >= 0x1F) {
                return 0x7BFF;                   // the largest finite half
            }

            // round-half-to-EVEN, which is what numpy's float16 cast does and so what the
            // prototype's copy holds. lround rounds half away from zero, and the two differ
            // on 104 of 1,508,336 halves by one ULP -- harmless in a UV, but a permanent
            // source of noise in the A/B that would hide a real difference later.
            const int mantissa = static_cast<int>(std::nearbyint(scaled * 2048.0f)) - 1024;
            return static_cast<std::uint16_t>((biased << 10) | (mantissa & 0x3FF));
        }
        const std::string ShapeKeyZero = "ShapeKeyZero";
        const std::string ChecksumNotFound = "ChecksumNotFound";
        const std::string DefaultTextureFolder = "Textures";
        const std::string DefaultMeshFolder = "Meshes";
        const std::string TextureOverridePrefix = "TextureOverride";
        const std::string TextureOverrideTexturePrefix = "TextureOverrideTexture";
        const std::string ResourcePrefix = "Resource";
        const std::string ThisKey = "this";
        const std::string DdsExt = ".dds";
        const std::string IniExt = ".ini";

        // How a file's name may say what type of texture it is, when nothing else does.
        const std::unordered_map<std::string, std::string> TypeOfSuffix = {
            {"diffuse", "diffuse"}, {"albedo", "diffuse"}, {"base", "diffuse"}, {"color", "diffuse"}, {"colour", "diffuse"}, {"d", "diffuse"},
            {"lm", "mask"}, {"lightmap", "mask"}, {"mask", "mask"}, {"m", "mask"},
            {"nm", "normal"}, {"normal", "normal"}, {"normalmap", "normal"}, {"n", "normal"}};
        const std::regex FileHashPattern(R"(t=([0-9a-fA-F]{8})\.dds$)", std::regex::icase);
        const std::regex ComponentFilePattern(R"(component[\s_-]*(\d+)[\s_-]+([a-z]+)\.dds$)", std::regex::icase);

        // How far above the .ini file's own folder the mod may reach for its textures: each level
        // climbed has to hold a .ini file of its own (a LOD folder's parent holding the file that
        // declares the textures), so a library of many mods is never indexed as one.
        const int MaxFolderClimb = 3;


        // The 8-byte WWMI blend line: four R8 bone indices then four R8 weights (Metadata.json's
        // export_format 'Blend'); the library's default BlendFile layout is GIMI's 32-byte one.
        std::vector<std::unique_ptr<BufElementType>> wwmiBlendElements() {
            std::vector<std::unique_ptr<BufElementType>> elements;
            for (const char* name : {"BLENDINDICES", "BLENDWEIGHT"}) {
                std::vector<std::unique_ptr<BufDataType>> types;
                for (int i = 0; i < 4; ++i) {
                    types.push_back(std::make_unique<BufUnSignedInt>("UnsignedInt8", 1, false));
                }

                elements.push_back(std::make_unique<BufElementType>(name, "R8G8B8A8_UINT", std::move(types)));
            }

            return elements;
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
                                 std::map<long long, std::vector<long long>> anchorChains = {}):
                    Base(std::move(resModObj), std::move(config), "blend"),
                    modType_(modType), fromVersion_(std::move(fromVersion)), toVersion_(std::move(toVersion)),
                    fixFunc_(std::move(fixFunc)), anchorChains_(std::move(anchorChains)) {}

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
                        fixFunc_, wwmiBlendElements());
                    resource->logger = ctx.logger();
                    ctx.storeResource(fileKey, std::move(resource));
                }

            private:
                const ModType* modType_;
                std::optional<Version> fromVersion_;
                std::optional<Version> toVersion_;
                std::function<bool(RemapBlendResource&)> fixFunc_;
                std::map<long long, std::vector<long long>> anchorChains_;
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
            std::ifstream vgIn(FileService::strToPath(vertexVGPath), std::ios::binary);
            std::ifstream blendIn(FileService::strToPath(resource.srcPath), std::ios::binary);
            if (!vgIn.is_open() || !blendIn.is_open()) {
                return false;
            }

            std::vector<std::uint8_t> vg((std::istreambuf_iterator<char>(vgIn)), std::istreambuf_iterator<char>());
            std::vector<std::uint8_t> blend((std::istreambuf_iterator<char>(blendIn)), std::istreambuf_iterator<char>());
            vgIn.close();
            blendIn.close();

            // The layout is derived, not assumed -- hardcoding it is what made the legacy lift
            // silently do nothing on these same mods.
            std::error_code err;
            const std::uintmax_t positionSize =
                std::filesystem::file_size(FileService::strToPath(positionPath), err);
            if (err || positionSize < 12) {
                return false;
            }

            const std::size_t vertices = static_cast<std::size_t>(positionSize / 12);
            if (vertices == 0 || blend.empty() || blend.size() % vertices != 0) {
                return false;
            }

            const std::size_t stride = blend.size() / vertices;      // N ids + N weights, a byte each
            if (stride < 2 || stride % 2 != 0) {
                return false;
            }

            const std::size_t influences = stride / 2;
            if (vg.size() != vertices * influences * 2) {            // the same ids as uint16
                return false;
            }

            const std::unordered_map<long long, long long>& row = resource.vgRemap.getRemap();
            std::vector<std::uint8_t> out(blend);
            for (std::size_t vertex = 0; vertex < vertices; ++vertex) {
                for (std::size_t b = 0; b < influences; ++b) {
                    const std::size_t at = vertex * stride + b;
                    if (blend[at + influences] == 0) {
                        continue;                                     // a weight-zero slot
                    }

                    std::uint16_t trueId = 0;
                    std::memcpy(&trueId, vg.data() + (vertex * influences + b) * 2, 2);
                    const auto target = row.find(static_cast<long long>(trueId));
                    if (target != row.end()) {
                        out[at] = static_cast<std::uint8_t>(target->second);
                    }
                }
            }

            std::ofstream fixed(FileService::strToPath(resource.fixedPath), std::ios::binary);
            if (!fixed.is_open()) {
                return false;
            }

            fixed.write(reinterpret_cast<const char*>(out.data()), static_cast<std::streamsize>(out.size()));
            return true;
        }


        bool liftLegacyBlend(RemapBlendResource& resource, const std::string& indexPath,
                             const std::string& positionPath,
                             const std::map<int, std::vector<std::pair<long long, long long>>>& drawRanges,
                             const std::map<int, std::vector<int>>& vgMaps) {
            std::ifstream indices(FileService::strToPath(indexPath), std::ios::binary);
            std::ifstream blendIn(FileService::strToPath(resource.srcPath), std::ios::binary);
            if (!indices.is_open() || !blendIn.is_open()) {
                return false;
            }

            std::vector<char> indexBytes((std::istreambuf_iterator<char>(indices)), std::istreambuf_iterator<char>());
            std::vector<unsigned char> blend((std::istreambuf_iterator<char>(blendIn)), std::istreambuf_iterator<char>());
            // THE LAYOUT IS DERIVED, NOT ASSUMED. WWMIBlendStride is four R8 ids then four R8
            // weights, which is one WWMI layout and not the only one: Chisa's mods carry EIGHT of
            // each. Reading a 16-byte vertex as two 8-byte ones gives twice the vertex count, so
            // componentOf is indexed by a vertex id that means nothing, most vertices get no
            // component at all and their ids pass through unlifted -- local id 5 of the skirt read
            // as merged bone 5, the jumbled mesh this function exists to prevent.
            std::error_code sizeErr;
            const std::uintmax_t positionSize =
                std::filesystem::file_size(FileService::strToPath(positionPath), sizeErr);
            if (sizeErr || positionSize < 12) {
                return false;
            }

            const std::size_t vertices = static_cast<std::size_t>(positionSize / 12);
            if (vertices == 0 || blend.size() % vertices != 0) {
                return false;
            }

            const std::size_t stride = blend.size() / vertices;
            if (stride < 2 || stride % 2 != 0) {
                return false;
            }

            const std::size_t influences = stride / 2;
            const std::size_t indexCount = indexBytes.size() / 4;

            // which component draws each vertex
            std::vector<int> componentOf(vertices, -1);
            for (const auto& entry : drawRanges) {
                for (const auto& range : entry.second) {
                    for (long long k = range.second; k < range.second + range.first && k >= 0; ++k) {
                        if (static_cast<std::size_t>(k) >= indexCount) {
                            break;
                        }

                        std::uint32_t vertex = 0;
                        std::memcpy(&vertex, indexBytes.data() + static_cast<std::size_t>(k) * 4, 4);
                        if (vertex < vertices) {
                            componentOf[vertex] = entry.first;
                        }
                    }
                }
            }

            const std::unordered_map<long long, long long>& row = resource.vgRemap.getRemap();
            std::size_t unmapped = 0;
            for (std::size_t vertex = 0; vertex < vertices; ++vertex) {
                const int component = componentOf[vertex];
                auto vgMap = vgMaps.find(component);
                if (component < 0 || vgMap == vgMaps.end()) {
                    continue;
                }

                for (std::size_t b = 0; b < influences; ++b) {
                    const std::size_t at = vertex * stride + b;
                    if (blend[at + influences] == 0) {
                        continue;                       // a weight-zero slot: the library leaves those alone too
                    }

                    const std::size_t local = blend[at];
                    if (local >= vgMap->second.size()) {
                        ++unmapped;
                        continue;
                    }

                    auto target = row.find(vgMap->second[local]);
                    if (target == row.end()) {
                        ++unmapped;
                        continue;
                    }

                    blend[at] = static_cast<unsigned char>(target->second);
                }
            }

            std::ofstream out(FileService::strToPath(resource.fixedPath), std::ios::binary);
            if (!out.is_open()) {
                return false;
            }

            out.write(reinterpret_cast<const char*>(blend.data()), static_cast<std::streamsize>(blend.size()));
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

        struct TextureRole {
            std::string role;
            std::string how;
        };

        /**
         * Every .dds under the mod's root with the roles it plays, and every .ini's resource
         * sections -> files. A file plays EVERY role its hashes name: a mod declares one file
         * under two hashes when one atlas serves two components (Upper_D.dds as both the arm
         * skin's and the bodice's diffuse), and taking only the first left the second component
         * unbound, drawing with the TARGET's own textures (2026-09-19). The root is the .ini file's own folder, climbed while the parent
         * holds a .ini file of its own (LOD folders under a file that declares their textures).
         */
        class TextureIndex {
            public:
                TextureIndex(const std::string& iniFolder, const WWMIFixerConfig& config, const std::string& remapTexKeyword,
                             const Hashes* libraryHashes, const std::string& sourceName) {
                    root_ = findRoot(iniFolder);

                    // The roles this config knows what to do with -- the gate on the library
                    // lookup below, so a `vb0` or `cb4` row can never be read as a texture role.
                    std::unordered_set<std::string> knownRoles;
                    for (const auto& entry : config.roles) {
                        knownRoles.insert(entry.second);
                    }

                    // A ROLE IS ASKED OF THE LIBRARY FIRST. `config.roles` is written from one
                    // generation of the character's textures and a mod carries whatever hash its
                    // author dumped, so a mod a version or two old matched almost nothing and
                    // downloaded the GAME's texture for role after role -- someone's painted outfit
                    // rendering as the vanilla one, with "downloaded 13 files" in the summary.
                    // HashData files them typed by role at every generation.
                    //
                    // One call, no version: ModMappedAssets keys its buckets by the ASSET, so a
                    // hash's buckets are its own generations and a hash that only ever existed at
                    // 3.0 has exactly one.
                    auto roleOfHash = [&config, &knownRoles, libraryHashes, &sourceName]
                                      (const std::string& hash) -> std::optional<TextureRole> {
                        if (libraryHashes != nullptr && !sourceName.empty()) {
                            std::optional<std::vector<std::string>> key =
                                libraryHashes->getKey(hash, std::nullopt, {sourceName, std::nullopt}, false);
                            if (key.has_value() && key->size() > 1 && knownRoles.count((*key)[1]) > 0) {
                                return TextureRole{(*key)[1], "hash " + hash + " (the library's history)"};
                            }
                        }

                        auto role = config.roles.find(hash);
                        if (role != config.roles.end()) {
                            return TextureRole{role->second, "hash " + hash};
                        }

                        return std::nullopt;
                    };

                    std::unordered_map<std::string, std::vector<std::string>> hashesOfFile;
                    std::vector<std::string> ddsFiles;
                    const std::string remapTex = StringTools::toLower(remapTexKeyword);
                    const std::string remapFix = StringTools::toLower(IniKeywords::RemapFix);
                    for (const std::string& file : FileService::getFilesAndDirs(root_, true).first) {
                        if (isDisabled(file)) {
                            continue;
                        }

                        const std::string name = StringTools::toLower(FileService::baseName(file));
                        if (StringTools::endsWith(name, DdsExt)) {
                            if (name.find(remapTex) == std::string::npos) {
                                const std::string key = FileService::pathKey(file);
                                ddsFiles.push_back(key);
                                real_[key] = file;
                            }

                            continue;
                        }

                        if (!StringTools::endsWith(name, IniExt) || name.find(remapFix) != std::string::npos) {
                            continue;
                        }

                        const std::string folder = FileService::parentOf(file);
                        std::vector<std::pair<std::string, std::string>> resources;     // in declaration order
                        std::unordered_map<std::string, std::string> fileOfResource;
                        const std::vector<IniScanSection> sections = IniScan::scan(file);
                        for (const IniScanSection& section : sections) {
                            if (!StringTools::startsWith(section.name, ResourcePrefix)
                                || StringTools::toLower(section.name).find(remapFix) != std::string::npos) {
                                continue;
                            }

                            std::optional<std::string> fileName = IniScan::firstVal(section, IniKeywords::Filename);
                            if (fileName.has_value() && StringTools::endsWith(StringTools::toLower(*fileName), DdsExt)) {
                                const std::string key = FileService::pathKey(FileService::absPathOfRelPath(*fileName, folder));
                                resources.emplace_back(section.name, key);
                                fileOfResource[section.name] = key;
                            }
                        }

                        for (const IniScanSection& section : sections) {
                            if (!StringTools::startsWith(section.name, TextureOverrideTexturePrefix)) {
                                continue;
                            }

                            std::optional<std::string> hash = IniScan::firstVal(section, IniKeywords::Hash);
                            std::optional<std::string> resource = IniScan::firstVal(section, ThisKey);
                            if (hash.has_value() && resource.has_value() && fileOfResource.count(*resource) > 0) {
                                hashesOfFile[fileOfResource[*resource]].push_back(StringTools::toLower(*hash));
                            }
                        }

                        resourcesByIni_[FileService::pathKey(file)] = std::move(resources);
                    }

                    std::vector<std::string> pending;
                    for (const std::string& file : ddsFiles) {
                        std::vector<std::string> hashes = hashesOfFile[file];
                        std::smatch match;
                        if (std::regex_search(file, match, FileHashPattern)) {
                            hashes.push_back(StringTools::toLower(match[1].str()));
                        }

                        std::vector<TextureRole> roles;
                        for (const std::string& hash : hashes) {
                            std::optional<TextureRole> role = roleOfHash(hash);
                            if (!role.has_value()) {
                                continue;
                            }

                            const bool known = std::any_of(roles.begin(), roles.end(),
                                                           [&](const TextureRole& r) { return r.role == role->role; });
                            if (!known) {
                                roles.push_back(*role);
                            }
                        }

                        if (!roles.empty()) {
                            rolesOf_[file] = std::move(roles);
                            ++byHash_;
                        } else {
                            pending.push_back(file);
                        }
                    }

                    for (const std::string& file : pending) {
                        std::optional<std::string> hash;
                        if (config.identifyTexture) {
                            hash = config.identifyTexture(real_[file]);
                        }

                        if (!hash.has_value()) {
                            hash = TexThumbprint::identifyFile(real_[file], config.textureThumbprints, config.thumbprintSize,
                                                      config.identityMin, config.identityGap);
                        }

                        if (hash.has_value()) {
                            std::optional<TextureRole> role = roleOfHash(StringTools::toLower(*hash));
                            if (role.has_value()) {
                                rolesOf_[file] = {TextureRole{role->role, "the game's own " + *hash + " by its pixels"}};
                                ++byPixels_;
                                continue;
                            }
                        }

                        std::smatch match;
                        const std::string name = FileService::baseName(file);
                        if (std::regex_search(name, match, ComponentFilePattern)) {
                            const int component = std::stoi(match[1].str());
                            auto type = TypeOfSuffix.find(StringTools::toLower(match[2].str()));
                            auto typeRoles = config.typeRoles.find(component);
                            if (type != TypeOfSuffix.end() && typeRoles != config.typeRoles.end()) {
                                auto role = typeRoles->second.find(type->second);
                                if (role != typeRoles->second.end()) {
                                    rolesOf_[file] = {TextureRole{role->second, "its name"}};
                                    ++byName_;
                                    continue;
                                }
                            }
                        }

                        unresolved_.push_back(file);
                    }
                }

                const std::string& root() const { return root_; }
                std::size_t fileCount() const { return real_.size(); }
                std::size_t byHash() const { return byHash_; }
                std::size_t byPixels() const { return byPixels_; }
                std::size_t byName() const { return byName_; }
                const std::vector<std::string>& unresolved() const { return unresolved_; }
                const std::unordered_map<std::string, std::vector<TextureRole>>& rolesOf() const { return rolesOf_; }

                // The file's real spelling, for what gets written into the .ini.
                std::string real(const std::string& key) const {
                    auto it = real_.find(key);
                    return it == real_.end() ? key : it->second;
                }

                // (resource section, file key) in the .ini's own declaration order: the FIRST resource
                // naming a file is the one bound, as the prototype binds it
                const std::vector<std::pair<std::string, std::string>>& resourcesOf(const std::string& iniPath) const {
                    static const std::vector<std::pair<std::string, std::string>> none;
                    auto it = resourcesByIni_.find(FileService::pathKey(iniPath));
                    return it == resourcesByIni_.end() ? none : it->second;
                }

            private:
                static std::string findRoot(const std::string& iniFolder) {
                    std::string folder = iniFolder;
                    for (int i = 0; i < MaxFolderClimb; ++i) {
                        const std::string parent = FileService::parentOf(folder);
                        if (parent.empty() || parent == folder) {
                            break;
                        }

                        bool holdsIni = false;
                        for (const std::string& file : FileService::getFilesAndDirs(parent, false).first) {
                            const std::string name = FileService::baseName(file);
                            if (StringTools::endsWith(StringTools::toLower(name), IniExt) && !IniNamingTools::isDisabled(name)) {
                                holdsIni = true;
                                break;
                            }
                        }

                        if (!holdsIni) {
                            break;
                        }

                        folder = parent;
                    }

                    return folder;
                }

                // A DISABLED-prefixed folder or file, anywhere under the root: the game ignores it.
                bool isDisabled(const std::string& file) const {
                    const std::string rel = FileService::pathKey(FileService::getRelPath(file, root_));
                    std::size_t start = 0;
                    while (start <= rel.size()) {
                        const std::size_t end = rel.find('/', start);
                        const std::string part = rel.substr(start, end == std::string::npos ? std::string::npos : end - start);
                        if (IniNamingTools::isDisabled(part)) {
                            return true;
                        }

                        if (end == std::string::npos) {
                            break;
                        }

                        start = end + 1;
                    }

                    return false;
                }

                std::string root_;
                std::unordered_map<std::string, std::string> real_;
                std::unordered_map<std::string, std::vector<std::pair<std::string, std::string>>> resourcesByIni_;
                std::unordered_map<std::string, std::vector<TextureRole>> rolesOf_;
                std::vector<std::string> unresolved_;
                std::size_t byHash_ = 0;
                std::size_t byPixels_ = 0;
                std::size_t byName_ = 0;
        };


        // ---- the fixer ----

        class WWMIFixerImpl: public Fixer {
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
                        if (StringTools::startsWith(entry.first, TextureOverridePrefix)) {
                            this->copyHiddenSectionNames.insert(entry.first);
                        }
                    }
                }

            protected:
                // GIMIFixer's own hands every group edit a nullptr .ini file, which stops a collect
                // ever building anything -- see GIMICharFixerImpl. The files the fix writes outside
                // the resource system (the zero stream) and the resources it adds by hand (the
                // created textures) go in here too, at fix time, so a parse alone writes nothing.
                // Every group's text, checked against what the edits were meant to write -- see verified()
                std::string groupToStr(std::size_t groupInd) const override {
                    return verified(Fixer::groupToStr(groupInd));
                }

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
                            std::string forward = *file;
                            std::replace(forward.begin(), forward.end(), '\\', '/');
                            const std::size_t slash = forward.rfind('/');
                            if (slash != std::string::npos && slash > 0) {
                                meshFolder_ = forward.substr(0, slash);
                            }
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
                            || !StringTools::startsWith(entry.first, TextureOverrideTexturePrefix)) {
                            continue;
                        }

                        std::vector<std::string> bound;
                        for (const auto& part : entry.second->parts()) {
                            const auto* content =
                                dynamic_cast<const IfTemplate<std::string, std::string>::ContentPart*>(part.get());
                            if (content == nullptr) {
                                continue;
                            }

                            for (const std::string& val : content->getVals(ThisKey)) {
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

                        for (const std::string& resource : bound) {
                            conditionalOwner_[StringTools::toLower(resource)] = entry.first;
                        }

                        variantsOf_[entry.first] = bound;
                    }
                }

                void readTextures() {
                    IniFile* ini = ctx_.getIniFile();
                    const std::string iniFolder = ini->getFolder();
                    const std::string iniPath = ini->getFile().value_or("");
                    readConditionalBindings();
                    const ModType* sourceType = ctx_.modType();
                    index_ = std::make_unique<TextureIndex>(
                        iniFolder, config_, IniKeywords::RemapTex,
                        sourceType != nullptr ? sourceType->hashes.get() : nullptr,
                        sourceType != nullptr ? sourceType->name : std::string());

                    std::unordered_map<std::string, std::string> resourceOfFile;
                    for (const auto& entry : index_->resourcesOf(iniPath)) {
                        resourceOfFile.emplace(entry.second, entry.first);
                        // ...and the other way, so an edit can reach EVERY variant a toggled role
                        // binds rather than only the one fileOfRole_ resolved to
                        fileOfResource_.emplace(StringTools::toLower(entry.first), index_->real(entry.second));
                    }

                    textureFolder_ = DefaultTextureFolder;
                    if (!resourceOfFile.empty()) {
                        const std::string rel = FileService::getRelPath(index_->real(resourceOfFile.begin()->first), iniFolder);
                        std::string forward = rel;
                        std::replace(forward.begin(), forward.end(), '\\', '/');
                        const std::size_t slash = forward.rfind('/');
                        if (slash != std::string::npos && slash > 0) {
                            textureFolder_ = forward.substr(0, slash);
                        }
                    }

                    // role -> (file, how the role was decided), every role of every file
                    std::map<std::string, std::vector<std::pair<std::string, std::string>>> byRole;
                    for (const auto& entry : index_->rolesOf()) {
                        for (const TextureRole& role : entry.second) {
                            byRole[role.role].emplace_back(entry.first, role.how);
                        }
                    }

                    // ...and the roles the mod's OWN sections name by the register they bind at --
                    // see WWMIFixerConfig::sourceRegisterRoles. Added as candidates beside the
                    // others, which is what the ranking below expects.
                    if (!config_.sourceRegisterRoles.empty()) {
                        std::unordered_map<std::string, std::string> fileOfResource;
                        for (const auto& entry : index_->resourcesOf(iniPath)) {
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
                                    std::string name(StringTools::strip(*bound));
                                    const std::string lower = StringTools::toLower(name);
                                    if (StringTools::startsWith(lower, "ref ")) {
                                        name = std::string(StringTools::strip(std::string_view(name).substr(4)));
                                    }

                                    const auto file = fileOfResource.find(StringTools::toLower(name));
                                    if (file == fileOfResource.end()) {
                                        continue;
                                    }

                                    byRole[role].emplace_back(
                                        file->second, "the " + reg + " its own section binds it at");
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
                            if (!TexThumbprint::channelIsConstant(index_->real(candidate.first), 0)) {
                                varying.push_back(candidate);
                                continue;
                            }

                            ctx_.log(FileService::getRelPath(index_->real(candidate.first), iniFolder)
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
                        std::string rel = FileService::pathKey(FileService::getRelPath(index_->real(file), iniFolder));
                        std::size_t ups = 0;
                        std::size_t pos = 0;
                        while ((pos = rel.find("../", pos)) != std::string::npos) {
                            ++ups;
                            pos += 3;
                        }

                        const std::vector<int> tag = componentTag(index_->real(file));
                        int specificity = 2;
                        if (tag.size() == 1 && tag.front() == component) {
                            specificity = 0;
                        } else if (std::find(tag.begin(), tag.end(), component) != tag.end()) {
                            specificity = 1;
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
                            std::sort(candidates.begin(), candidates.end(),
                                      [&](const auto& a, const auto& b) { return rank(a.first, component) < rank(b.first, component); });
                            const std::string& best = candidates.front().first;
                            if (candidates.size() > 1) {
                                const auto first = rank(best, component);
                                const auto second = rank(candidates[1].first, component);
                                if (std::get<0>(first) == std::get<0>(second) && std::get<1>(first) == std::get<1>(second)
                                    && std::get<2>(first) == std::get<2>(second)
                                    && saidAmbiguous.insert(role + "\n" + best + "\n" + candidates[1].first).second) {
                                    // Two shipped textures equally close on one role: the first is bound
                                    // and only a measurement can say which is right -- say so loudly.
                                    ctx_.log("WARNING: " + FileService::getRelPath(index_->real(candidates[1].first), index_->root())
                                             + " also has the role " + role + " (" + candidates[1].second
                                             + "), already taken by " + FileService::getRelPath(index_->real(best), index_->root())
                                             + " (" + candidates.front().second + "); the first one is bound");
                                }
                            }

                            fileOfRole_[role] = index_->real(best);
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
                                std::string name = ResourcePrefix + TextTools::capitalize(role) + toModName_ + IniKeywords::RemapRef;
                                for (std::size_t n = 2; usedDeclaredNames_.count(name) > 0; ++n) {
                                    name = ResourcePrefix + TextTools::capitalize(role) + std::to_string(n) + toModName_ + IniKeywords::RemapRef;
                                }

                                usedDeclaredNames_.insert(name);
                                declaredName = declaredName_.emplace(best, name).first;
                                std::string rel = FileService::getRelPath(index_->real(best), iniFolder);
                                std::replace(rel.begin(), rel.end(), '/', '\\');
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
                        resourceOfRole_[created.role] = fixName(ResourcePrefix + created.role);
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
                                const std::string fileName = DownloadTools::fixedFileName(config_.downloadPrefix, kind, DdsExt);
                                fallbacks_[binding.role] = Fallback{
                                    DownloadTools::downloadFolder() + "/"
                                        + DownloadTools::urlPath(config_.downloadGameFolder, config_.downloadCharFolder,
                                                                 config_.downloadVersionFolder, config_.downloadPrefix,
                                                                 "Texture" + fallback->second, DdsExt),
                                    fileName, textureFolder_ + "/" + fileName};
                                resourceOfRole_[binding.role] = ResourcePrefix + config_.downloadPrefix + kind + IniKeywords::RemapDL;
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
                            const std::string fileName = DownloadTools::fixedFileName(config_.downloadPrefix, kind, DdsExt);
                            fallbacks_[role] = Fallback{
                                DownloadTools::downloadFolder() + "/"
                                    + DownloadTools::urlPath(config_.downloadGameFolder, config_.downloadCharFolder,
                                                             config_.downloadVersionFolder, config_.downloadPrefix,
                                                             "Texture" + fallback->second, DdsExt),
                                fileName, textureFolder_ + "/" + fileName};
                            resourceOfRole_[role] = ResourcePrefix + config_.downloadPrefix + kind + IniKeywords::RemapDL;
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

                    assetRemap_ = std::make_unique<RegAssetRemap<>>(
                        std::vector<std::pair<std::string, RegAssetRemap<>::AssetSpec>>{
                            {IniKeywords::Hash, RegAssetRemap<>::AssetSpec(ctx_.modTypeHashes(), IniKeywords::HashNotFound)},
                            {ShapeKeyChecksumKey, RegAssetRemap<>::AssetSpec(source->shapeKeyChecksums.get(), ChecksumNotFound)}},
                        toModName_, ctx_.modTypeName().value_or(""), from, to);
                    assetAdapter_ = std::make_unique<RegPartEdit<>>(assetRemap_.get());

                    // Lines a remapped section drops -- see WWMIFixerConfig::removedRegs for why
                    // each kind is there. Built once and hung on every remapped slot section.
                    if (!config_.removedRegs.empty()) {
                        std::vector<std::pair<std::string, std::optional<RegRemove<>::RemoveKeyCheck>>> keys;
                        keys.reserve(config_.removedRegs.size());
                        for (const WWMIFixerConfig::RegRemoval& removal : config_.removedRegs) {
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

                    // Per target slot: its numbers, written over the source's.
                    for (int slot : drawnSlots_) {
                        const Slot& s = target_.slots.at(static_cast<std::size_t>(slot));
                        auto newVals = std::make_unique<RegNewVals<>>(
                            std::vector<std::pair<std::string, RegNewVals<>::NewValSpec>>{
                                {IniKeywords::MatchFirstIndex, RegNewVals<>::NewVal(s.indexOffset)},
                                {MatchIndexCountKey, RegNewVals<>::NewVal(s.indexCount)},
                                {VgOffsetKey, RegNewVals<>::NewVal(s.vgOffset)},
                                {VgCountKey, RegNewVals<>::NewVal(s.vgCount)}},
                            false);
                        for (const auto& planned : config_.plan) {
                            if (planned.second.slot != slot || present_.count(planned.first) == 0) {
                                continue;
                            }

                            for (const std::string& section : present_.at(planned.first)) {
                                expected_[ModBranches::looseKey(fixName(section))].values = {
                                    {IniKeywords::MatchFirstIndex, s.indexOffset}, {MatchIndexCountKey, s.indexCount},
                                    {VgOffsetKey, s.vgOffset}, {VgCountKey, s.vgCount}};
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
                        if (legacy_) {
                            additions.emplace_back(IniKeywords::Run, mergeListName(planned.slot));
                        }

                        if (config_.zeroShapeKeyStream && meshVertexCount_ > 0) {
                            additions.emplace_back(config_.shapeKeyStreamReg, fixName(ResourcePrefix + ShapeKeyZero));
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
                            const std::string cmdList = fixName("CommandList" + source_.name + TextTools::capitalize(config_.slotPrefix)
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
                                    fixName("CommandList" + source_.name + TextTools::capitalize(config_.slotPrefix)
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
                            for (const std::string& section : present_.at(component)) {
                                expected_[ModBranches::looseKey(fixName(section))].additions = additions;
                            }

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
                        if (vertexVGFile_.has_value()) {
                            std::string vgRel = *vertexVGFile_;
                            std::replace(vgRel.begin(), vgRel.end(), '\\', '/');
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
                                                                          std::move(lift), config_.anchorChains);
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
                                    return !StringTools::startsWith(
                                        StringTools::toLower(StringTools::lstrip(value)), "ref ");
                                };
                        }

                        collect->resEdits = {{"blend", replace.get()}};
                        blendReplaces_.push_back(std::move(replace));
                        blendCollects_.push_back(std::move(collect));
                    }
                }

                // ---- what the WRITTEN text must say ----

                /**
                 * A graph edit that cannot reach a line does nothing and says nothing, so the text this
                 * fixer produced is checked against what it MEANT to produce before it is handed back.
                 *
                 * SanhuaExorcist4's author ends the torso's section with `run = CustomShader1` and puts
                 * the section's closing `endif`s inside THAT section, so its `if` blocks do not balance.
                 * For that one section the `RegNewVals` left `vg_offset` / `vg_count` at the SOURCE's
                 * 22 / 105 (the target's bone data then merged into the wrong window of the merged
                 * skeleton: every torso vertex on a bone nothing wrote), the `RegSurroundedAdd` added
                 * neither the zero stream nor the texture list, and a copy of the shared-resource
                 * override kept binding the mod's own blend. The flat lines of the same section were
                 * rewritten correctly, which is what makes it so quiet: the section looks retargeted.
                 * A mod's text is the modder's, and 3dmigoto accepts what the graph model cannot edit.
                 */
                // verified() reports through this: the context's log is not const, and rendering a group is
                void note(const std::string& message) const {
                    const_cast<IniFileFixContext&>(ctx_).log(message);
                }

                std::string verified(const std::string& text) const {
                    std::vector<std::string> lines;
                    std::string current;
                    for (const char c : text) {
                        if (c == '\n') {
                            lines.push_back(current);
                            current.clear();
                        } else {
                            current += c;
                        }
                    }

                    lines.push_back(current);
                    std::string sectionKey;
                    std::vector<std::size_t> anchors;                  // where each section's additions belong
                    std::map<std::size_t, std::vector<std::string>> inserts;
                    std::string blendResource;                         // the remapped blend, as the text spells it

                    // the remapped blend's own name, to rebind any copy that kept the mod's
                    for (const std::string& line : lines) {
                        const std::pair<std::string, std::string> kvp = keyValueOf(line);
                        if (kvp.first == config_.blendReg && kvp.second.find(IniKeywords::Remap) != std::string::npos) {
                            blendResource = kvp.second;
                            break;
                        }
                    }

                    std::set<std::string> held;
                    std::size_t anchorAt = std::string::npos;
                    auto closeSection = [&]() {
                        auto expectation = expected_.find(sectionKey);
                        if (expectation == expected_.end() || anchorAt == std::string::npos) {
                            return;
                        }

                        std::vector<std::string> missing;
                        std::string names;
                        for (const auto& addition : expectation->second.additions) {
                            if (held.count(addition.first + " = " + addition.second) == 0) {
                                missing.push_back(indentOf(lines[anchorAt]) + addition.first + " = " + addition.second);
                                names += (names.empty() ? "" : ", ") + addition.second;
                            }
                        }

                        // One line per SECTION, and it does not name a cause. It used to say the
                        // section's `if` blocks do not balance, which is true of the mod that
                        // motivated this check and false of others -- Chisa1's component 5 balances
                        // exactly and still lands here. All the verifier knows is that the graph edit
                        // did not reach the line and that this put it where it belongs.
                        if (!missing.empty()) {
                            note("a graph edit did not place " + std::to_string(missing.size())
                                 + (missing.size() == 1 ? " line (" : " lines (") + names + ") in "
                                 + sectionKey + "; added after the shared-resource override");
                        }

                        if (!missing.empty()) {
                            inserts[anchorAt] = std::move(missing);
                        }
                    };

                    for (std::size_t k = 0; k < lines.size(); ++k) {
                        const std::string name = sectionNameOf(lines[k]);
                        if (!name.empty()) {
                            closeSection();
                            sectionKey = ModBranches::looseKey(name);
                            held.clear();
                            anchorAt = std::string::npos;
                            continue;
                        }

                        const std::pair<std::string, std::string> kvp = keyValueOf(lines[k]);
                        if (kvp.first.empty()) {
                            continue;
                        }

                        // every copy of the shared-resource override binds the REMAPPED blend
                        if (kvp.first == config_.blendReg && !blendResource.empty() && kvp.second != blendResource) {
                            lines[k] = indentOf(lines[k]) + kvp.first + " = " + blendResource;
                            note("a copy of the shared-resource override bound " + kvp.second
                                     + "; rebound to the remapped blend " + blendResource);
                            continue;
                        }

                        auto expectation = expected_.find(sectionKey);
                        if (expectation == expected_.end()) {
                            continue;
                        }

                        held.insert(kvp.first + " = " + kvp.second);
                        if (kvp.first == IniKeywords::Run && kvp.second.find(config_.sharedResourcesList) != std::string::npos
                            && anchorAt == std::string::npos) {
                            anchorAt = k;
                        }

                        for (const auto& value : expectation->second.values) {
                            if (kvp.first == value.first && kvp.second != value.second) {
                                lines[k] = indentOf(lines[k]) + kvp.first + " = " + value.second;
                                // no cause named, for the same reason as the placement note above
                                note("a graph edit did not rewrite `" + kvp.first + "` in " + sectionKey
                                         + "; corrected " + kvp.second + " -> " + value.second);
                            }
                        }
                    }

                    closeSection();
                    if (inserts.empty()) {
                        std::string out;
                        for (std::size_t k = 0; k < lines.size(); ++k) {
                            out += (k == 0 ? "" : "\n") + lines[k];
                        }

                        return out;
                    }

                    std::string out;
                    for (std::size_t k = 0; k < lines.size(); ++k) {
                        out += (k == 0 ? "" : "\n") + lines[k];
                        auto added = inserts.find(k);
                        if (added != inserts.end()) {
                            for (const std::string& line : added->second) {
                                out += "\n" + line;
                            }
                        }
                    }

                    return out;
                }

                // `[Name]` -> `Name`, for a line that is a section header
                static std::string sectionNameOf(const std::string& line) {
                    const std::string trimmed{StringTools::strip(line)};
                    if (trimmed.size() < 3 || trimmed.front() != '[' || trimmed.back() != ']') {
                        return "";
                    }

                    return trimmed.substr(1, trimmed.size() - 2);
                }

                // `key = value` -> {key, value}, for a line that is one and is not commented out
                static std::pair<std::string, std::string> keyValueOf(const std::string& line) {
                    const std::string trimmed{StringTools::strip(line)};
                    const std::size_t equals = trimmed.find('=');
                    if (trimmed.empty() || trimmed.front() == ';' || equals == std::string::npos) {
                        return {"", ""};
                    }

                    return {std::string{StringTools::strip(trimmed.substr(0, equals))},
                            std::string{StringTools::strip(trimmed.substr(equals + 1))}};
                }

                static std::string indentOf(const std::string& line) {
                    return line.substr(0, line.size() - std::string{StringTools::lstrip(line)}.size());
                }

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
                        if (std::find(out.begin(), out.end(), role) == out.end()) {
                            out.push_back(role);
                        }
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
                        fixName("CommandList" + source_.name + TextTools::capitalize(role) + IniNamingTools::getRegTag(reg));
                    std::string body;
                    bool anyBinding = false;
                    std::istringstream lines(renderIfTemplate(*tpl->second, "", true));
                    std::string line;
                    while (std::getline(lines, line)) {
                        // renderIfTemplate writes the section's OWN header first, and left in it
                        // closes the list and REDEFINES the mod's section inside the fix block
                        const std::string_view bare = StringTools::strip(line);
                        if (!bare.empty() && bare.front() == '[') {
                            continue;
                        }

                        const std::size_t equals = line.find('=');
                        const std::string key = equals == std::string::npos
                                                    ? std::string()
                                                    : StringTools::toLower(std::string(StringTools::strip(line.substr(0, equals))));
                        if (IniKeywords::MatchKeys.count(key) > 0) {
                            continue;                       // 3dmigoto's matching keys mean nothing in a list
                        }

                        if (key == ThisKey) {
                            const std::string indent = line.substr(0, line.size() - StringTools::lstrip(line).size());
                            std::string val(StringTools::strip(line.substr(equals + 1)));
                            const auto swap = editedResourceOf_.find(StringTools::toLower(val));
                            if (swap != editedResourceOf_.end()) {
                                val = swap->second;
                            }

                            body += indent + reg + " = " + val + "\n";
                            anyBinding = true;
                        } else {
                            body += line + "\n";
                        }

                    }

                    if (!anyBinding) {
                        return direct;
                    }

                    roleLists_.emplace(std::pair<std::string, std::string>{role, reg}, name);
                    // Their own vector, emitted at the END of buildAppended: textureLists_ is
                    // written out before the shared-mesh loop runs, so a list created there would be
                    // dropped. Section order in an .ini does not matter.
                    //
                    // The blank line is what the dropped matching keys leave behind, which is how the
                    // prototype renders it too.
                    roleListTexts_.push_back("[" + name + "]\n\n" + body);
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
                std::string legacySkeletonSections() const {
                    const std::string merged = fixName(ResourcePrefix + std::string("MergedSkeleton"));
                    const std::string mergedRW = fixName(ResourcePrefix + std::string("MergedSkeletonRW"));
                    const std::string extra = fixName(ResourcePrefix + std::string("ExtraMergedSkeleton"));
                    const std::string extraRW = fixName(ResourcePrefix + std::string("ExtraMergedSkeletonRW"));
                    std::string out = "[" + merged + "]\n\n[" + extra + "]\n\n";
                    for (const std::string& name : {mergedRW, extraRW}) {
                        out += "[" + name + "]\ntype = RWBuffer\nformat = R32G32B32A32_FLOAT\narray = "
                               + std::to_string(config_.mergedSkeletonSlots) + "\n\n";
                    }

                    out += "[" + fixName("TextureOverrideMarkBoneDataCB") + "]\n" + IniKeywords::Hash + " = " + target_.cb4Hash
                           + "\nmatch_priority = 0\nfilter_index = " + config_.boneDataFilter + "\n\n";

                    for (std::size_t slot = 0; slot < target_.slots.size(); ++slot) {
                        const Slot& s = target_.slots[slot];
                        out += "[" + mergeListName(static_cast<int>(slot)) + "]\n";
                        for (const auto& cb : {std::make_tuple(std::string("vs-cb4"), mergedRW, merged),
                                               std::make_tuple(std::string("vs-cb3"), extraRW, extra)}) {
                            out += "if " + std::get<0>(cb) + " == " + config_.boneDataFilter + "\n"
                                   + "    " + VgOffsetKey + " = " + s.vgOffset + "\n"
                                   + "    " + VgCountKey + " = " + s.vgCount + "\n"
                                   + "    $\\WWMIv1\\custom_mesh_scale = 1.00\n"
                                   + "    cs-cb8 = ref " + std::get<0>(cb) + "\n"
                                   + "    cs-u6 = " + std::get<1>(cb) + "\n"
                                   + "    run = CustomShader\\WWMIv1\\SkeletonMerger\n"
                                   + "    " + std::get<2>(cb) + " = copy " + std::get<1>(cb) + "\n"
                                   + "    " + std::get<0>(cb) + " = " + std::get<2>(cb) + "\nendif\n";
                        }

                        out += "\n";
                    }

                    return out;
                }

                // ---- the fix's own sections ----

                void buildAppended() {
                    std::string out;
                    for (const auto& entry : declared_) {
                        out += "[" + declaredName_[entry.first] + "]\n" + IniKeywords::Filename + " = " + entry.second + "\n\n";
                    }

                    for (const auto& entry : fallbacks_) {
                        out += "[" + resourceOfRole_[entry.first] + "]\n" + IniKeywords::Filename + " = " + entry.second.relPath + "\n\n";
                    }

                    if (config_.zeroShapeKeyStream && meshVertexCount_ > 0) {
                        out += "[" + fixName(ResourcePrefix + ShapeKeyZero) + "]\ntype = Buffer\nformat = DXGI_FORMAT_R32G32B32_FLOAT\nstride = "
                               + std::to_string(config_.shapeKeyStride) + "\n" + IniKeywords::Filename + " = " + zeroStreamFile() + "\n\n";
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
                        out += "; nothing of the mod is drawn through " + toModName_ + "'s " + labelText
                               + " slot: the skin's own geometry is skipped and its bones still merged\n"
                               + "[TextureOverride" + toModName_ + TextTools::capitalize(config_.slotPrefix) + std::to_string(slot) + IniKeywords::Remap + "Hide]\n"
                               + IniKeywords::Hash + " = " + target_.vb0Hash + "\n"
                               + IniKeywords::MatchFirstIndex + " = " + s.indexOffset + "\n"
                               + MatchIndexCountKey + " = " + s.indexCount + "\n"
                               + "$object_detected = 1\nif $mod_enabled\n"
                               + (legacy_
                                      ? "    run = " + mergeListName(slot) + "\n    handling = skip\n"
                                      : "    local " + state + "\n    if " + state + " != $state_id\n"
                                            + "        " + state + " = $state_id\n        " + VgOffsetKey + " = " + s.vgOffset
                                            + "\n        " + VgCountKey + " = " + s.vgCount + "\n"
                                            + "        run = " + fixName("CommandListMergeSkeleton") + "\n    endif\n"
                                            + "    if ResourceMergedSkeleton !== null\n        handling = skip\n    endif\n")
                               + "endif\n\n";
                    }

                    if (legacy_) {
                        out += legacySkeletonSections();
                    }

                    passFilter("");
                    std::size_t i = 0;
                    for (const std::string& pass : passOrder_) {
                        out += "[" + fixName("ShaderOverridePass" + std::to_string(i)) + "]\n" + IniKeywords::Hash + " = " + pass
                               + "\nfilter_index = " + passFilters_[pass] + "\n\n";
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
                            out += "[" + fixName("TextureOverride" + source_.name + "SharedMesh"
                                                 + std::to_string(meshNum)) + "]\n"
                                   + IniKeywords::Hash + " = " + meshHash + "\n" + body + "\n";
                        }

                        ++meshNum;
                    }

                    if (texcoordSection_.has_value()) {
                        out += *texcoordSection_;
                    }

                    for (const auto& edited : editedResources_) {
                        out += "[" + edited.first + "]\n" + IniKeywords::Filename + " = " + edited.second + "\n\n";
                    }

                    for (const WWMIFixerConfig::CreatedTexture& created : config_.createdTextures) {
                        out += "[" + resourceOfRole_[created.role] + "]\n" + IniKeywords::Filename + " = " + createdTextureFile(created) + "\n\n";
                    }

                    for (const std::string& list : roleListTexts_) {
                        out += list + "\n";
                    }

                    this->appendedSections = std::string(StringTools::rstrip(out));
                }

                std::string zeroStreamFile() const {
                    return meshFolder_ + "/" + toModName_ + IniKeywords::Remap + ShapeKeyZero + ".buf";
                }

                std::string createdTextureFile(const WWMIFixerConfig::CreatedTexture& created) const {
                    return textureFolder_ + "/" + created.role + toModName_ + IniKeywords::RemapTex + DdsExt;
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
                                                     + TextTools::capitalize(edit.role) + edit.name + IniKeywords::RemapTex + DdsExt;
                        plannedEdits_.push_back(PlannedEdit{&edit, source, fixedRel});

                        // every binding of the role follows the edited file
                        const std::string resource = fixName(ResourcePrefix + TextTools::capitalize(edit.role) + edit.name
                                                             + IniKeywords::RemapTex);

                        // Which of the mod's resources this replaces, so a copied toggle chain can
                        // swap it in.
                        const std::string* was = sharedResourceFor(edit.role);
                        if (was != nullptr) {
                            editedResourceOf_[StringTools::toLower(*was)] = resource;
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
                                        + edit.name + suffix + IniKeywords::RemapTex + DdsExt;
                                    const std::string variantResource =
                                        fixName(ResourcePrefix + TextTools::capitalize(edit.role) + edit.name + suffix
                                                + IniKeywords::RemapTex);
                                    plannedEdits_.push_back(PlannedEdit{&edit, file->second, variantRel});
                                    editedResources_.emplace_back(variantResource, variantRel);
                                    editedResourceOf_[StringTools::toLower(variant)] = variantResource;
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
                            && StringTools::equalsIgnoreCase(entry.first, "ResourceTexcoordBuffer")) {
                            resource = entry.second.get();
                            break;
                        }
                    }

                    if (resource == nullptr) {
                        return;
                    }

                    const std::optional<std::string> name = ModBranches::firstVal(*resource, IniKeywords::Filename);
                    if (!name.has_value()) {
                        return;
                    }

                    std::string rel = *name;
                    std::replace(rel.begin(), rel.end(), '\\', '/');
                    const std::string path = FileService::absPathOfRelPath(rel, ini->getFolder());
                    std::ifstream in(FileService::strToPath(path), std::ios::binary);
                    if (!in.is_open()) {
                        return;
                    }

                    std::vector<std::uint8_t> bytes((std::istreambuf_iterator<char>(in)),
                                                    std::istreambuf_iterator<char>());
                    in.close();
                    const std::optional<std::string> strideVal = ModBranches::firstVal(*resource, "stride");
                    std::size_t stride = 16;
                    if (strideVal.has_value()) {
                        try {
                            stride = static_cast<std::size_t>(std::stoul(StringTools::strip(*strideVal).data()));
                        } catch (const std::exception&) {
                            stride = 16;
                        }
                    }

                    if (stride < 4 || stride % 2 != 0 || bytes.size() % stride != 0) {
                        return;
                    }

                    const std::size_t perVertex = stride / 2;
                    const std::size_t vertices = bytes.size() / stride;
                    auto halfAt = [&bytes](std::size_t i) {
                        std::uint16_t bits = 0;
                        std::memcpy(&bits, bytes.data() + i * 2, 2);
                        return bits;
                    };
                    auto setHalf = [&bytes](std::size_t i, std::uint16_t bits) {
                        std::memcpy(bytes.data() + i * 2, &bits, 2);
                    };

                    std::size_t cleared = 0;
                    for (std::size_t i = 0; i < bytes.size() / 2; ++i) {
                        const std::uint16_t bits = halfAt(i);
                        if (((bits >> 10) & 0x1F) == 0x1F && (bits & 0x3FF) != 0) {
                            setHalf(i, 0);
                            ++cleared;
                        }
                    }

                    // U outside [0, 1), except on a triangle whose vertices straddle a tile
                    std::vector<float> u(vertices, 0.0f);
                    std::vector<bool> needs(vertices, false);
                    for (std::size_t v = 0; v < vertices; ++v) {
                        u[v] = halfToFloat(halfAt(v * perVertex));
                        needs[v] = (u[v] >= 1.0f) || (u[v] < 0.0f);
                    }

                    std::size_t folded = 0;
                    if (std::find(needs.begin(), needs.end(), true) != needs.end()) {
                        std::vector<bool> keep(vertices, false);
                        const std::string indexPath = FileService::absPathOfRelPath(indexFile_, ini->getFolder());
                        std::ifstream ib(FileService::strToPath(indexPath), std::ios::binary);
                        if (ib.is_open()) {
                            std::vector<std::uint8_t> raw((std::istreambuf_iterator<char>(ib)),
                                                          std::istreambuf_iterator<char>());
                            const std::size_t count = raw.size() / 4;
                            for (std::size_t t = 0; t + 2 < count; t += 3) {
                                std::uint32_t tri[3] = {0, 0, 0};
                                std::memcpy(tri, raw.data() + t * 4, 12);
                                if (tri[0] >= vertices || tri[1] >= vertices || tri[2] >= vertices) {
                                    continue;
                                }

                                const float a = std::floor(u[tri[0]]);
                                const float b = std::floor(u[tri[1]]);
                                const float c = std::floor(u[tri[2]]);
                                if (a != b || b != c) {
                                    keep[tri[0]] = true;
                                    keep[tri[1]] = true;
                                    keep[tri[2]] = true;
                                }
                            }
                        }

                        for (std::size_t v = 0; v < vertices; ++v) {
                            if (needs[v] && !keep[v]) {
                                float wrapped = std::fmod(u[v], 1.0f);
                                if (wrapped < 0.0f) {
                                    wrapped += 1.0f;
                                }

                                setHalf(v * perVertex, floatToHalf(wrapped));
                                ++folded;
                            }
                        }
                    }

                    if (cleared == 0 && folded == 0) {
                        return;
                    }

                    const std::string fixedRel = meshFolder_ + "/" + toModName_ + IniKeywords::Remap + "Texcoord.buf";
                    const std::string fixedPath = FileService::absPathOfRelPath(fixedRel, ini->getFolder());
                    std::error_code err;
                    std::filesystem::create_directories(FileService::strToPath(fixedPath).parent_path(), err);
                    std::ofstream out(FileService::strToPath(fixedPath), std::ios::binary);
                    if (!out.is_open()) {
                        return;
                    }

                    out.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
                    out.close();

                    const std::optional<std::string> format = ModBranches::firstVal(*resource, "format");
                    texcoordResource_ = fixName(ResourcePrefix + "TexcoordNoNaN");
                    texcoordSection_ = "[" + *texcoordResource_ + "]\ntype = Buffer\nformat = "
                                       + std::string(format.has_value() ? StringTools::strip(*format)
                                                                        : std::string_view("DXGI_FORMAT_R16G16_FLOAT"))
                                       + "\nstride = " + std::to_string(stride) + "\n"
                                       + IniKeywords::Filename + " = " + fixedRel + "\n\n";
                    ctx_.log("texcoords: " + std::to_string(cleared) + " NaN halves set to 0 and "
                             + std::to_string(folded) + " U values folded into [0, 1) in a remap-only copy");
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

                std::unique_ptr<TextureIndex> index_;
                std::map<std::string, std::string> resourceOfRole_;   // role -> resource, for what every component shares (a created texture, a download)
                std::map<std::pair<std::string, int>, std::string> resourceOfSlotRole_;   // (role, source component) -> the resource that component binds
                std::vector<std::pair<std::string, std::string>> declared_;   // (file, path relative to the .ini) for a file no resource of the .ini names
                std::map<std::string, std::string> declaredName_;      // that file -> the resource section the fix declares for it
                std::set<std::string> usedDeclaredNames_;
                std::map<std::string, Fallback> fallbacks_;           // role -> the source's game texture, for a planned role the mod has no file for
                std::set<std::string> leftToGame_;                    // roles whose only file was flat and whose config says not to stand anything in
                std::vector<std::string> textureLists_;
                std::unordered_map<std::string, std::string> passFilters_;

                // what a remapped slot section must hold once it is written -- see verified()
                struct Expectation {
                    std::vector<std::pair<std::string, std::string>> values;
                    std::vector<std::pair<std::string, std::string>> additions;
                };

                std::map<std::string, Expectation> expected_;

                bool legacy_ = false;                                 // a mod from before WWMI's merged skeleton
                std::string indexFile_ = "Meshes/Index.buf";           // as the mod's own [ResourceIndexBuffer] names it
                std::string positionFile_ = "Meshes/Position.buf";     // ...and [ResourcePositionBuffer]
                std::string texcoordFile_ = "Meshes/TexCoord.buf";     // ...and [ResourceTexcoordBuffer]
                std::map<int, std::vector<std::pair<long long, long long>>> drawRanges_;   // source component -> its (index count, first index) draws
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
