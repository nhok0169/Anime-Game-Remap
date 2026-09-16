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

#include "AGRemapCore/model/iniresources/VGMergeGroupResource.h"

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <utility>

#include "AGRemapCore/model/files/BinaryFile.h"
#include "AGRemapCore/model/files/BlendFile.h"
#include "AGRemapCore/model/files/IbFile.h"
#include "AGRemapCore/tools/files/FileService.h"
#include "AGRemapCore/view/BaseLogger.h"

namespace AGRemapCore {
    namespace {
        constexpr const char* BlendType = "blend";
        constexpr const char* PositionType = "position";
        constexpr const char* TexcoordType = "texcoord";
        constexpr const char* IbType = "buf";

        bool samePath(const std::string& a, const std::string& b) {
            std::error_code ec;
            std::filesystem::path pa = std::filesystem::absolute(FileService::strToPath(a), ec);
            std::filesystem::path pb = std::filesystem::absolute(FileService::strToPath(b), ec);
            return pa.lexically_normal() == pb.lexically_normal();
        }

        void writeBytes(const std::string& path, const ByteVec& bytes) {
            std::ofstream file(FileService::strToPath(path), std::ios::binary);
            if (!file) {
                throw std::runtime_error("could not write '" + path + "'");
            }
            file.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
        }
    }


    bool fixVGMergeGroup(IniGroupedResource& group, const VGMergeGroupConfig& config, BaseLogger* logger) {
        IniFixResource* blend = nullptr;
        IniFixResource* position = nullptr;
        IniFixResource* texcoord = nullptr;
        std::vector<IniFixResource*> ibs;

        for (IniResource* member : group.memberResources()) {
            auto* fixResource = dynamic_cast<IniFixResource*>(member);
            if (fixResource == nullptr) {
                continue;
            }
            if (member->type == BlendType) {
                blend = fixResource;
            } else if (member->type == PositionType) {
                position = fixResource;
            } else if (member->type == TexcoordType) {
                texcoord = fixResource;
            } else if (member->type == IbType) {
                ibs.push_back(fixResource);
            }
        }

        if (blend == nullptr) {
            throw std::invalid_argument("the group '" + group.name + "' has no Blend.buf member to merge into");
        }
        if (config.components.empty()) {
            throw std::invalid_argument("the group '" + group.name + "' was given no components to merge");
        }

        if (logger != nullptr) {
            std::string names;
            for (const VGMergeComponentFiles& component : config.components) {
                names += (names.empty() ? "" : ", ") + component.spec.name;
            }
            logger->log("Merging the " + names + " buffers of "
                        + FileService::pathToStr(FileService::strToPath(blend->srcPath).filename()) + "...");
        }

        std::vector<VGComponentMerge::Component> components;
        components.reserve(config.components.size());
        for (const VGMergeComponentFiles& files : config.components) {
            VGComponentMerge::Component component;
            component.spec = files.spec;

            BlendFile blendFile(files.blendPath);
            auto [weights, indices] = VGComponentSplit::readBlend(blendFile);
            component.weights = std::move(weights);
            component.indices = std::move(indices);

            BinaryFile positionFile(files.positionPath);
            component.position = positionFile.read();
            BinaryFile texcoordFile(files.texcoordPath);
            component.texcoord = texcoordFile.read();

            components.push_back(std::move(component));
        }

        VGComponentMerge merge(std::move(components));

        // A group whose bones have nowhere to go writes a NEGATIVE index and the model kinks there,
        // which nothing downstream reports -- so say so here
        if (logger != nullptr && !merge.stats().unmapped.empty()) {
            std::string groups;
            for (const auto& entry : merge.stats().unmapped) {
                groups += (groups.empty() ? "" : ", ") + entry.first + " " + std::to_string(entry.second);
            }
            logger->log("WARNING: these vertex groups carry weight and have no remap, so they become negative bone"
                        " indices and the model will kink there: " + groups);
        }

        writeBytes(blend->fixedPath, merge.blend());
        if (position != nullptr) {
            writeBytes(position->fixedPath, merge.position());
        }
        if (texcoord != nullptr) {
            writeBytes(texcoord->fixedPath, merge.texcoord());
        }

        for (IniFixResource* ib : ibs) {
            const VGMergeObject* object = nullptr;
            for (const VGMergeObject& candidate : config.objects) {
                if (samePath(candidate.srcPath, ib->srcPath)) {
                    object = &candidate;
                    break;
                }
            }
            if (object == nullptr) {
                throw std::invalid_argument("'" + ib->srcPath + "' is not the index buffer of any object the merge was given");
            }

            std::vector<std::pair<std::string, VGComponentSplit::Triangles>> members;
            members.reserve(object->members.size());
            for (const auto& member : object->members) {
                // The DECLARED width -- see VGMergeGroupConfig::ibBytesPerIndex.
                auto widthIt = config.ibBytesPerIndex.find(member.second);
                IbFile ibFile(member.second, widthIt == config.ibBytesPerIndex.end() ? 4 : widthIt->second);
                members.emplace_back(member.first, VGComponentSplit::readIb(ibFile));
            }
            writeBytes(ib->fixedPath, VGComponentSplit::encodeIb(merge.mergeIbs(members)));
        }

        return true;
    }


    VGMergeGroupResource::VGMergeGroupResource(std::string name, std::unordered_map<std::string, std::unique_ptr<IniResource>> resources,
                                                VGMergeGroupConfig config, std::function<bool(IniGroupedResource&)> fixFunc, bool isBuilt):
        RemapIniGroupedResource(std::move(name), std::move(resources), std::move(fixFunc), isBuilt), config(std::move(config)) {}


    bool VGMergeGroupResource::_fix() {
        return fixVGMergeGroup(*this, config, logger.get());
    }
}
