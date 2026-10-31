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

#ifndef AGRemapCore_VGRemapBlendEdit_TPP
#define AGRemapCore_VGRemapBlendEdit_TPP

#include "VGRemapBlendEdit.h"

#include <algorithm>
#include <stdexcept>

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/model/files/BinaryFile.h"
#include "AGRemapCore/model/files/BlendFile.h"
#include "AGRemapCore/tools/files/FileService.h"


namespace AGRemapCore {
    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    VGRemapBlendReplace<K, V, KeyHash, KeyEqual>::VGRemapBlendReplace(
            GraphId resModObj, ResEditConfig config, const ModType* modType,
            std::optional<Version> fromVersion, std::optional<Version> toVersion, std::string resType,
            std::optional<std::string> resSubType, std::optional<std::string> fromComp, std::optional<std::string> toComp):
        Base(std::move(resModObj), std::move(config), std::move(resType), std::move(resSubType),
              std::move(fromComp), std::move(toComp)),
        modType(modType), fromVersion(std::move(fromVersion)), toVersion(std::move(toVersion)) {}


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void VGRemapBlendReplace<K, V, KeyHash, KeyEqual>::buildResModel(
            const std::string& resType, const std::string& srcPath, const std::string& fixedPath,
            const std::string& modName, const std::string& fileKey, Context& ctx) {
        (void)resType;

        std::optional<VGRemap> vgRemap;
        if (modType != nullptr) {
            vgRemap = modType->getVGRemap(rowModName.value_or(modName), fromVersion, toVersion, this->fromComp, this->toComp);
        }

        // No remap is a real answer, not a reason to write a plain copy under a remapped name --
        // see this class's own note.
        if (!vgRemap.has_value()) {
            Base::buildResModel(this->resType, srcPath, fixedPath, modName, fileKey, ctx);
            return;
        }

        // Each chain to whatever its ROOT maps to. A root the row has no entry for is left alone rather
        // than guessed at: that would write a target of 0, which is a real bone.
        if (!anchorChains.empty()) {
            std::unordered_map<long long, long long> row = vgRemap->getRemap();
            for (const auto& [root, members] : anchorChains) {
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

        // The width the target reads: after the remap, made 'influences' wide, with the id buffers beside it
        // keeping the same slots
        std::function<bool(RemapBlendResource&)> fix = fixFunc;
        if (influences != 0 && widths(FileService::absPathOfRelPath(srcPath, ctx.iniFolder())).first != influences) {
            fix = [inner = fixFunc, layout = elements, paired = pairedIds, width = influences](RemapBlendResource& resource) {
                if (inner) {
                    if (!inner(resource)) {
                        return false;
                    }
                } else {
                    BlendFile blend = resource.createBlend();
                    blend.remap(resource.vgRemap, resource.fixedPath);
                }

                try {
                    BlendFile blend(resource.fixedPath, layout ? layout(resource.srcPath) : std::vector<std::unique_ptr<BufElementType>>{});
                    const std::size_t from = blend.influences();
                    const std::vector<PairedIds> ids = paired ? paired(resource.srcPath) : std::vector<PairedIds>{};

                    // the first id buffer names the bones a narrowed vertex's slots are merged by
                    std::vector<std::vector<long long>> bones;
                    std::vector<ByteVec> idBytes;
                    for (std::size_t i = 0; i < ids.size(); ++i) {
                        idBytes.push_back(BinaryFile(ids[i].path).read());
                        if (i == 0) {
                            const ByteVec& raw = idBytes.back();
                            const std::size_t line = from * ids[i].idBytes;
                            for (std::size_t at = 0; line != 0 && at + line <= raw.size(); at += line) {
                                std::vector<long long> row;
                                for (std::size_t k = 0; k < from; ++k) {
                                    long long id = 0;
                                    for (std::size_t b = 0; b < ids[i].idBytes; ++b) {
                                        id |= static_cast<long long>(raw[at + k * ids[i].idBytes + b]) << (8 * b);
                                    }
                                    row.push_back(id);
                                }
                                bones.push_back(std::move(row));
                            }
                        }
                    }

                    const BlendFile::KeptSlots kept = blend.setInfluences(width, ids.empty() ? nullptr : &bones);
                    if (!BinaryFile::write(resource.fixedPath, blend.getData())) {
                        throw std::runtime_error("could not write the blend " + std::to_string(width) + " wide");
                    }
                    for (std::size_t i = 0; i < ids.size(); ++i) {
                        if (!BinaryFile::write(ids[i].path, BlendFile::keepSlots(idBytes[i], ids[i].idBytes, from, kept))) {
                            throw std::runtime_error("could not write " + ids[i].path + " " + std::to_string(width) + " wide");
                        }
                    }
                } catch (const std::exception& exception) {
                    throw std::runtime_error("cannot fix " + FileService::pathToStr(FileService::strToPath(resource.srcPath).filename())
                                             + ": " + exception.what());
                }
                return true;
            };
        }

        auto resource = std::make_unique<RemapBlendResource>(
            ctx.iniFolder(), srcPath, fixedPath, std::move(*vgRemap), this->resType,
            std::move(fix),
            elements ? elements(srcPath) : std::vector<std::unique_ptr<BufElementType>>{});

        resource->logger = ctx.logger();
        ctx.storeResource(fileKey, std::move(resource));
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::pair<std::size_t, std::size_t> VGRemapBlendReplace<K, V, KeyHash, KeyEqual>::widths(const std::string& srcPath) const {
        std::vector<std::unique_ptr<BufElementType>> layout = elements ? elements(srcPath) : BlendFile::defaultElements();
        std::size_t from = 0;
        bool hasWeights = false;
        bool hasIndices = false;
        std::size_t weights = 0;
        std::size_t indices = 0;
        for (const auto& element : layout) {
            if (element->getName() == BlendFile::BlendWeightKey) {
                weights = element->getDataTypes().size();
                hasWeights = true;
            } else if (element->getName() == BlendFile::BlendIndicesKey) {
                indices = element->getDataTypes().size();
                hasIndices = true;
            }
        }
        from = hasWeights && hasIndices ? std::min(weights, indices) : 0;

        std::size_t lineBytes = 0;
        for (const auto& element : BlendFile::elementsWithInfluences(layout, influences)) {
            lineBytes += element->getSize();
        }
        return {from, lineBytes};
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::vector<std::pair<K, V>> VGRemapBlendReplace<K, V, KeyHash, KeyEqual>::fileKVPs(const std::string& srcPath) const {
        std::vector<std::pair<K, V>> result = Base::fileKVPs(srcPath);
        if (influences == 0) {
            return result;
        }

        const auto [from, lineBytes] = widths(srcPath);
        if (from != influences) {
            result.emplace_back(IniKeywords::Stride, std::to_string(lineBytes));
        }
        return result;
    }
}

#endif
