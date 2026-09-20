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
#include "AGRemapCore/model/Version.h"
#include "AGRemapCore/model/buffers/BufElementType.h"
#include "AGRemapCore/model/buffers/BufInt.h"
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
#include "AGRemapCore/model/strategies/iniParsers/GIMIParser.h"
#include "AGRemapCore/model/strategies/texEditors/TexCreator.h"
#include "AGRemapCore/tools/StringTools.h"
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
        const std::string Cb4HashKey = "cb4";
        constexpr std::size_t WWMIBlendStride = 8;      // four R8 bone indices then four R8 weights
        const std::string ShapeKeyZero = "ShapeKeyZero";
        const std::string ChecksumNotFound = "ChecksumNotFound";
        const std::string DefaultTextureFolder = "Textures";
        const std::string DefaultMeshFolder = "Meshes";
        const std::string DisabledPrefix = "disabled";
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


        // ---- small helpers ----

        std::string lowerKey(const std::string& path) {
            std::string out = StringTools::toLower(path);
            std::replace(out.begin(), out.end(), '\\', '/');
            return out;
        }

        bool startsWithDisabled(const std::string& name) {
            return StringTools::startsWith(StringTools::toLower(name), DisabledPrefix);
        }

        std::string baseName(const std::string& path) {
            return FileService::pathToStr(FileService::strToPath(path).filename());
        }

        std::string parentOf(const std::string& path) {
            return FileService::pathToStr(FileService::strToPath(path).parent_path());
        }

        std::string capitalized(const std::string& word) {
            if (word.empty()) {
                return word;
            }

            std::string out = word;
            out[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(out[0])));
            return out;
        }

        std::string formatFilter(double value) {
            char buffer[32];
            std::snprintf(buffer, sizeof(buffer), "%.4f", value);
            std::string out(buffer);
            while (!out.empty() && out.back() == '0') {
                out.pop_back();
            }

            if (!out.empty() && out.back() == '.') {
                out.pop_back();
            }

            return out;
        }

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

        // ---- pixel identity: a file's thumbprint, correlated against the game textures' ----

        // The same arithmetic as Tools/Misc/Diagnostics/wwmiTextureThumbs.py: (r + g + b) / 3 per
        // pixel, box-averaged over blocks of (width / n) x (height / n), rounded.
        std::optional<std::vector<double>> thumbprintOf(const std::string& path, int n) {
            TextureFile texture(path);
            try {
                texture.open();
            } catch (const std::exception&) {
                return std::nullopt;
            }

            const int width = texture.getWidth();
            const int height = texture.getHeight();
            if (!texture.hasImage() || n <= 0 || width < n || height < n) {
                return std::nullopt;
            }

            const std::vector<std::uint8_t>& pixels = texture.getPixels();
            const int bw = width / n;
            const int bh = height / n;
            std::vector<double> thumb(static_cast<std::size_t>(n) * static_cast<std::size_t>(n), 0.0);
            for (int by = 0; by < n; ++by) {
                for (int bx = 0; bx < n; ++bx) {
                    double sum = 0.0;
                    for (int y = by * bh; y < (by + 1) * bh; ++y) {
                        const std::uint8_t* row = pixels.data() + (static_cast<std::size_t>(y) * static_cast<std::size_t>(width) + static_cast<std::size_t>(bx) * static_cast<std::size_t>(bw)) * 4;
                        for (int x = 0; x < bw; ++x) {
                            sum += (static_cast<double>(row[x * 4]) + static_cast<double>(row[x * 4 + 1]) + static_cast<double>(row[x * 4 + 2])) / 3.0;
                        }
                    }

                    thumb[static_cast<std::size_t>(by) * static_cast<std::size_t>(n) + static_cast<std::size_t>(bx)]
                        = std::round(sum / (static_cast<double>(bw) * static_cast<double>(bh)));
                }
            }

            return thumb;
        }

        double correlation(const std::vector<double>& a, const std::vector<std::uint8_t>& b) {
            if (a.size() != b.size() || a.empty()) {
                return 0.0;
            }

            double meanA = 0.0;
            double meanB = 0.0;
            for (std::size_t i = 0; i < a.size(); ++i) {
                meanA += a[i];
                meanB += static_cast<double>(b[i]);
            }

            meanA /= static_cast<double>(a.size());
            meanB /= static_cast<double>(a.size());
            double dot = 0.0;
            double normA = 0.0;
            double normB = 0.0;
            for (std::size_t i = 0; i < a.size(); ++i) {
                const double da = a[i] - meanA;
                const double db = static_cast<double>(b[i]) - meanB;
                dot += da * db;
                normA += da * da;
                normB += db * db;
            }

            const double norm = std::sqrt(normA) * std::sqrt(normB);
            return norm > 0.0 ? dot / norm : 0.0;
        }

        // The hash of the game texture 'file' IS, by its thumbprint -- or nothing.
        std::optional<std::string> identifyByThumbprint(const std::string& file, const WWMIFixerConfig& config) {
            if (config.textureThumbprints.empty()) {
                return std::nullopt;
            }

            std::optional<std::vector<double>> thumb = thumbprintOf(file, config.thumbprintSize);
            if (!thumb.has_value()) {
                return std::nullopt;
            }

            std::string best;
            double bestScore = -2.0;
            double secondScore = -2.0;
            for (const auto& entry : config.textureThumbprints) {
                const double score = correlation(*thumb, entry.second);
                if (score > bestScore) {
                    secondScore = bestScore;
                    bestScore = score;
                    best = entry.first;
                } else if (score > secondScore) {
                    secondScore = score;
                }
            }

            if (bestScore >= config.identityMin && secondScore < config.identityGap) {
                return best;
            }

            return std::nullopt;
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
                                 std::function<bool(RemapBlendResource&)> fixFunc = {}):
                    Base(std::move(resModObj), std::move(config), "blend"),
                    modType_(modType), fromVersion_(std::move(fromVersion)), toVersion_(std::move(toVersion)),
                    fixFunc_(std::move(fixFunc)) {}

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
        bool liftLegacyBlend(RemapBlendResource& resource, const std::string& indexPath,
                             const std::map<int, std::vector<std::pair<long long, long long>>>& drawRanges,
                             const std::map<int, std::vector<int>>& vgMaps) {
            std::ifstream indices(FileService::strToPath(indexPath), std::ios::binary);
            std::ifstream blendIn(FileService::strToPath(resource.srcPath), std::ios::binary);
            if (!indices.is_open() || !blendIn.is_open()) {
                return false;
            }

            std::vector<char> indexBytes((std::istreambuf_iterator<char>(indices)), std::istreambuf_iterator<char>());
            std::vector<unsigned char> blend((std::istreambuf_iterator<char>(blendIn)), std::istreambuf_iterator<char>());
            const std::size_t vertices = blend.size() / WWMIBlendStride;
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

                for (std::size_t b = 0; b < 4; ++b) {
                    const std::size_t at = vertex * WWMIBlendStride + b;
                    if (blend[at + 4] == 0) {
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

        struct IniSection {
            std::string name;
            std::vector<std::pair<std::string, std::string>> kvps;
        };

        // A plain line scan: enough to read `filename =`, `hash =` and `this =` out of any .ini of
        // the mod, including the ones the API never parses (a namespaced companion file).
        std::vector<IniSection> scanIni(const std::string& path) {
            std::vector<IniSection> sections;
            std::ifstream in(FileService::strToPath(path));
            std::string line;
            while (std::getline(in, line)) {
                std::string stripped = std::string(StringTools::strip(line));
                if (stripped.empty() || stripped[0] == ';') {
                    continue;
                }

                if (stripped.front() == '[' && stripped.back() == ']') {
                    sections.push_back(IniSection{stripped.substr(1, stripped.size() - 2), {}});
                    continue;
                }

                if (sections.empty()) {
                    continue;
                }

                const std::size_t eq = stripped.find('=');
                if (eq == std::string::npos) {
                    continue;
                }

                sections.back().kvps.emplace_back(std::string(StringTools::strip(stripped.substr(0, eq))),
                                                  std::string(StringTools::strip(stripped.substr(eq + 1))));
            }

            return sections;
        }

        std::optional<std::string> firstKvp(const IniSection& section, const std::string& key) {
            for (const auto& kvp : section.kvps) {
                if (kvp.first == key) {
                    return kvp.second;
                }
            }

            return std::nullopt;
        }

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
                TextureIndex(const std::string& iniFolder, const WWMIFixerConfig& config, const std::string& remapTexKeyword) {
                    root_ = findRoot(iniFolder);

                    std::unordered_map<std::string, std::vector<std::string>> hashesOfFile;
                    std::vector<std::string> ddsFiles;
                    const std::string remapTex = StringTools::toLower(remapTexKeyword);
                    const std::string remapFix = StringTools::toLower(IniKeywords::RemapFix);
                    for (const std::string& file : FileService::getFilesAndDirs(root_, true).first) {
                        if (isDisabled(file)) {
                            continue;
                        }

                        const std::string name = StringTools::toLower(baseName(file));
                        if (StringTools::endsWith(name, DdsExt)) {
                            if (name.find(remapTex) == std::string::npos) {
                                const std::string key = lowerKey(file);
                                ddsFiles.push_back(key);
                                real_[key] = file;
                            }

                            continue;
                        }

                        if (!StringTools::endsWith(name, IniExt) || name.find(remapFix) != std::string::npos) {
                            continue;
                        }

                        const std::string folder = parentOf(file);
                        std::vector<std::pair<std::string, std::string>> resources;     // in declaration order
                        std::unordered_map<std::string, std::string> fileOfResource;
                        const std::vector<IniSection> sections = scanIni(file);
                        for (const IniSection& section : sections) {
                            if (!StringTools::startsWith(section.name, ResourcePrefix)
                                || StringTools::toLower(section.name).find(remapFix) != std::string::npos) {
                                continue;
                            }

                            std::optional<std::string> fileName = firstKvp(section, IniKeywords::Filename);
                            if (fileName.has_value() && StringTools::endsWith(StringTools::toLower(*fileName), DdsExt)) {
                                const std::string key = lowerKey(FileService::absPathOfRelPath(*fileName, folder));
                                resources.emplace_back(section.name, key);
                                fileOfResource[section.name] = key;
                            }
                        }

                        for (const IniSection& section : sections) {
                            if (!StringTools::startsWith(section.name, TextureOverrideTexturePrefix)) {
                                continue;
                            }

                            std::optional<std::string> hash = firstKvp(section, IniKeywords::Hash);
                            std::optional<std::string> resource = firstKvp(section, ThisKey);
                            if (hash.has_value() && resource.has_value() && fileOfResource.count(*resource) > 0) {
                                hashesOfFile[fileOfResource[*resource]].push_back(StringTools::toLower(*hash));
                            }
                        }

                        resourcesByIni_[lowerKey(file)] = std::move(resources);
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
                            auto role = config.roles.find(hash);
                            if (role == config.roles.end()) {
                                continue;
                            }

                            const bool known = std::any_of(roles.begin(), roles.end(),
                                                           [&](const TextureRole& r) { return r.role == role->second; });
                            if (!known) {
                                roles.push_back(TextureRole{role->second, "hash " + hash});
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
                            hash = identifyByThumbprint(real_[file], config);
                        }

                        if (hash.has_value()) {
                            auto role = config.roles.find(StringTools::toLower(*hash));
                            if (role != config.roles.end()) {
                                rolesOf_[file] = {TextureRole{role->second, "the game's own " + *hash + " by its pixels"}};
                                ++byPixels_;
                                continue;
                            }
                        }

                        std::smatch match;
                        const std::string name = baseName(file);
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
                    auto it = resourcesByIni_.find(lowerKey(iniPath));
                    return it == resourcesByIni_.end() ? none : it->second;
                }

            private:
                static std::string findRoot(const std::string& iniFolder) {
                    std::string folder = iniFolder;
                    for (int i = 0; i < MaxFolderClimb; ++i) {
                        const std::string parent = parentOf(folder);
                        if (parent.empty() || parent == folder) {
                            break;
                        }

                        bool holdsIni = false;
                        for (const std::string& file : FileService::getFilesAndDirs(parent, false).first) {
                            const std::string name = baseName(file);
                            if (StringTools::endsWith(StringTools::toLower(name), IniExt) && !startsWithDisabled(name)) {
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
                    const std::string rel = lowerKey(FileService::getRelPath(file, root_));
                    std::size_t start = 0;
                    while (start <= rel.size()) {
                        const std::size_t end = rel.find('/', start);
                        const std::string part = rel.substr(start, end == std::string::npos ? std::string::npos : end - start);
                        if (StringTools::startsWith(part, DisabledPrefix)) {
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
                        version = Version::parse(config_.version);
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

                        // its own draw ranges, for the per-component lift of the blend
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

                void readTextures() {
                    IniFile* ini = ctx_.getIniFile();
                    const std::string iniFolder = ini->getFolder();
                    const std::string iniPath = ini->getFile().value_or("");
                    index_ = std::make_unique<TextureIndex>(iniFolder, config_, IniKeywords::RemapTex);

                    std::unordered_map<std::string, std::string> resourceOfFile;
                    for (const auto& entry : index_->resourcesOf(iniPath)) {
                        resourceOfFile.emplace(entry.second, entry.first);
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
                        std::string rel = lowerKey(FileService::getRelPath(index_->real(file), iniFolder));
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

                    // The choice is per (role, source component), not per role
                    for (const auto& planned : config_.plan) {
                        const int component = planned.first;
                        if (present_.count(component) == 0) {
                            continue;
                        }

                        for (const WWMIFixerConfig::Binding& binding : planned.second.bindings) {
                            auto found = byRole.find(binding.role);
                            if (found == byRole.end() || found->second.empty()
                                || resourceOfSlotRole_.count({binding.role, component}) > 0) {
                                continue;
                            }

                            std::vector<std::pair<std::string, std::string>> candidates = found->second;
                            std::sort(candidates.begin(), candidates.end(),
                                      [&](const auto& a, const auto& b) { return rank(a.first, component) < rank(b.first, component); });
                            const std::string& best = candidates.front().first;
                            if (candidates.size() > 1) {
                                const auto first = rank(best, component);
                                const auto second = rank(candidates[1].first, component);
                                if (std::get<0>(first) == std::get<0>(second) && std::get<1>(first) == std::get<1>(second)
                                    && std::get<2>(first) == std::get<2>(second)) {
                                    // Two shipped textures equally close on one role: the first is bound
                                    // and only a measurement can say which is right -- say so loudly.
                                    ctx_.log("WARNING: " + FileService::getRelPath(index_->real(candidates[1].first), index_->root())
                                             + " also has the role " + binding.role + " (" + candidates[1].second
                                             + "), already taken by " + FileService::getRelPath(index_->real(best), index_->root())
                                             + " (" + candidates.front().second + "); the first one is bound");
                                }
                            }

                            auto own = resourceOfFile.find(best);
                            if (own != resourceOfFile.end()) {
                                resourceOfSlotRole_[{binding.role, component}] = own->second;
                                continue;
                            }

                            // A file no resource of this .ini names gets a resource section of the fix's
                            // own -- named with RemapRef, not RemapFix: the section sits inside the fix's
                            // block and names one of the MOD's files, and an undo deletes what a RemapFix
                            // section names. Two files of one role each get their own.
                            auto declaredName = declaredName_.find(best);
                            if (declaredName == declaredName_.end()) {
                                std::string name = ResourcePrefix + capitalized(binding.role) + toModName_ + IniKeywords::RemapRef;
                                for (std::size_t n = 2; usedDeclaredNames_.count(name) > 0; ++n) {
                                    name = ResourcePrefix + capitalized(binding.role) + std::to_string(n) + toModName_ + IniKeywords::RemapRef;
                                }

                                usedDeclaredNames_.insert(name);
                                declaredName = declaredName_.emplace(best, name).first;
                                std::string rel = FileService::getRelPath(index_->real(best), iniFolder);
                                std::replace(rel.begin(), rel.end(), '/', '\\');
                                declared_.emplace_back(best, rel);
                            }

                            resourceOfSlotRole_[{binding.role, component}] = declaredName->second;
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
                                if (fallback == config_.fallbackTextures.end()) {
                                    continue;
                                }

                                const std::string kind = capitalized(binding.role);
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
                    }
                }

                // ---- the edits ----

                ModObj targetSlotObj(int slot) const {
                    return ModObj(toModName_, config_.slotPrefix + std::to_string(slot));
                }

                void buildEdits() {
                    const ModType* source = ctx_.modType();
                    const std::optional<Version> from = fromVersion();
                    const std::optional<Version> to = toVersion();

                    assetRemap_ = std::make_unique<RegAssetRemap<>>(
                        std::vector<std::pair<std::string, RegAssetRemap<>::AssetSpec>>{
                            {IniKeywords::Hash, RegAssetRemap<>::AssetSpec(ctx_.modTypeHashes(), IniKeywords::HashNotFound)},
                            {ShapeKeyChecksumKey, RegAssetRemap<>::AssetSpec(source->shapeKeyChecksums.get(), ChecksumNotFound)}},
                        toModName_, ctx_.modTypeName().value_or(""), from, to);
                    assetAdapter_ = std::make_unique<RegPartEdit<>>(assetRemap_.get());

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
                                expected_[lowerKey(fixName(section))].values = {
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

                        std::vector<std::string> bindings;
                        for (const WWMIFixerConfig::Binding& binding : planned.bindings) {
                            const std::string* resource = resourceFor(binding.role, component);
                            if (resource != nullptr) {
                                bindings.push_back("    " + binding.reg + " = " + *resource);
                            }
                        }

                        if (!bindings.empty()) {
                            const std::string cmdList = fixName("CommandList" + source_.name + capitalized(config_.slotPrefix)
                                                                + std::to_string(component) + "Textures");
                            std::string condition;
                            for (const std::string& pass : config_.slotPasses.at(static_cast<std::size_t>(planned.slot))) {
                                condition += (condition.empty() ? "" : " || ") + std::string("ps == ") + passFilter(pass);
                            }

                            std::string text = "[" + cmdList + "]\nif " + condition + "\n";
                            for (const std::string& binding : bindings) {
                                text += binding + "\n";
                            }

                            text += "endif\n";
                            textureLists_.push_back(text);
                            additions.emplace_back(IniKeywords::Run, cmdList);
                        }

                        if (!additions.empty()) {
                            for (const std::string& section : present_.at(component)) {
                                expected_[lowerKey(fixName(section))].additions = additions;
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
                        if (legacy_) {
                            const std::string indexPath = FileService::absPathOfRelPath(indexFile_, ctx_.getIniFile()->getFolder());
                            const std::map<int, std::vector<std::pair<long long, long long>>> ranges = drawRanges_;
                            const std::map<int, std::vector<int>> maps = config_.sourceVgMaps;
                            lift = [indexPath, ranges, maps](RemapBlendResource& resource) {
                                return liftLegacyBlend(resource, indexPath, ranges, maps);
                            };
                        }

                        auto replace = std::make_unique<WWMIBlendReplace>(GraphId(g, "", "blend"), makeResEditConfig(), source, from, to,
                                                                          std::move(lift));
                        auto collect = std::make_unique<Collector>();
                        for (int component : groups_[g]) {
                            const ModObj obj = targetSlotObj(config_.plan.at(component).slot);
                            collect->srcRegs[GraphId(g, obj.first, obj.second)] = config_.blendReg;
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
                        for (const auto& addition : expectation->second.additions) {
                            if (held.count(addition.first + " = " + addition.second) == 0) {
                                missing.push_back(indentOf(lines[anchorAt]) + addition.first + " = " + addition.second);
                                note("a graph edit could not place `" + addition.first + " = " + addition.second
                                         + "` in " + sectionKey + " (its `if` blocks do not balance); added after the shared-resource override");
                            }
                        }

                        if (!missing.empty()) {
                            inserts[anchorAt] = std::move(missing);
                        }
                    };

                    for (std::size_t k = 0; k < lines.size(); ++k) {
                        const std::string name = sectionNameOf(lines[k]);
                        if (!name.empty()) {
                            closeSection();
                            sectionKey = lowerKey(name);
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
                                note("a graph edit could not rewrite `" + kvp.first + "` in " + sectionKey
                                         + " (its `if` blocks do not balance): " + kvp.second + " -> " + value.second);
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
                    const std::string name = StringTools::toLower(baseName(file));
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
                        for (const auto& passes : config_.slotPasses) {
                            for (const std::string& p : passes) {
                                if (passFilters_.count(p) != 0) {
                                    continue;
                                }

                                auto given = config_.filterIndices.find(p);
                                if (given != config_.filterIndices.end()) {
                                    passFilters_[p] = given->second;
                                } else {
                                    passFilters_[p] = formatFilter(config_.filterBase + config_.filterStep * static_cast<double>(i));
                                    ++i;
                                }

                                passOrder_.push_back(p);
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
                               + "[TextureOverride" + toModName_ + capitalized(config_.slotPrefix) + std::to_string(slot) + IniKeywords::Remap + "Hide]\n"
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

                    for (const WWMIFixerConfig::CreatedTexture& created : config_.createdTextures) {
                        out += "[" + resourceOfRole_[created.role] + "]\n" + IniKeywords::Filename + " = " + createdTextureFile(created) + "\n\n";
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
                std::map<int, std::vector<std::pair<long long, long long>>> drawRanges_;   // source component -> its (index count, first index) draws
                std::vector<std::string> passOrder_;

                std::unique_ptr<GraphGroupRemap<>> slotRemap_;
                std::unique_ptr<RegAssetRemap<>> assetRemap_;
                std::unique_ptr<RegPartEdit<>> assetAdapter_;
                std::map<int, PartEdit*> newValsOf_;
                std::vector<std::unique_ptr<RegSurroundedAdd<>>> surroundedAdds_;
                std::vector<std::unique_ptr<GraphPartEdit<>>> graphAdapters_;
                std::vector<std::unique_ptr<RegNewVals<>>> newVals_;
                std::vector<std::unique_ptr<RegPartEdit<>>> regAdapters_;
                std::map<int, std::vector<PartEdit*>> editsOf_;
                std::unique_ptr<ObjGroupEdit> mainEdits_;
                std::vector<std::unique_ptr<WWMIBlendReplace>> blendReplaces_;
                std::vector<std::unique_ptr<Collector>> blendCollects_;
        };
    }


    IniFixBuilder::Factory makeWWMIFixer(WWMIFixerConfig config) {
        return [config](BaseIniParser<>* parser, const std::string& toModName, std::optional<int> modTypeId) {
            return std::make_shared<WWMIFixerImpl>(parser, toModName, modTypeId, config);
        };
    }
}
