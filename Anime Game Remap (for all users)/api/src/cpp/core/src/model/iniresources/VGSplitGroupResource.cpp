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

#include "AGRemapCore/model/iniresources/VGSplitGroupResource.h"

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

        // A whole vertex buffer, every line edited, then only the kept lines
        ByteVec filterVertexBuffer(const std::string& path, std::size_t vertexCount, const std::vector<std::size_t>& kept,
                                   const VGSplitGroupConfig::LineEdit& edit) {
            BinaryFile file(path);
            ByteVec data = file.read();
            if (vertexCount == 0 || data.size() % vertexCount != 0) {
                throw std::invalid_argument("'" + path + "' is " + std::to_string(data.size()) + " bytes, not a whole number of "
                                            + std::to_string(vertexCount) + " lines");
            }
            std::size_t stride = data.size() / vertexCount;

            if (edit) {
                ByteVec edited;
                edited.reserve(data.size());
                for (std::size_t i = 0; i < vertexCount; ++i) {
                    ByteVec line(data.begin() + static_cast<std::ptrdiff_t>(i * stride),
                                 data.begin() + static_cast<std::ptrdiff_t>((i + 1) * stride));
                    ByteVec out = edit(line);
                    if (out.size() != stride) {
                        throw std::invalid_argument("a line edit of '" + path + "' changed a line's size");
                    }
                    edited.insert(edited.end(), out.begin(), out.end());
                }
                data = std::move(edited);
            }

            return VGComponentSplit::keepLines(data, stride, kept);
        }
    }


    bool fixVGSplitGroup(IniGroupedResource& group, const VGSplitGroupConfig& config, BaseLogger* logger) {
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
            throw std::invalid_argument("the group '" + group.name + "' has no Blend.buf member to split by");
        }

        if (logger != nullptr) {
            logger->log("Splitting the buffers of " + FileService::pathToStr(FileService::strToPath(blend->srcPath).filename())
                        + " for " + config.component + "...");
        }

        // Every drawn object's index buffer, whether or not this group holds it
        std::vector<std::string> ibPaths = config.ibPaths;
        if (ibPaths.empty()) {
            for (IniFixResource* ib : ibs) {
                ibPaths.push_back(ib->srcPath);
            }
        }

        BlendFile blendFile(blend->srcPath);
        auto [weights, indices] = VGComponentSplit::readBlend(blendFile);

        std::vector<VGComponentSplit::Triangles> triangles;
        for (const std::string& path : ibPaths) {
            IbFile ibFile(path);
            triangles.push_back(VGComponentSplit::readIb(ibFile));
        }

        VGComponentSplit split(std::move(weights), std::move(indices), std::move(triangles), config.specs);
        VGComponentBuffers buffers = split.split(config.component);

        writeBytes(blend->fixedPath, VGComponentSplit::encodeBlend(buffers.weights, buffers.indices));

        for (IniFixResource* ib : ibs) {
            std::size_t which = ibPaths.size();
            for (std::size_t i = 0; i < ibPaths.size(); ++i) {
                if (samePath(ibPaths[i], ib->srcPath)) {
                    which = i;
                    break;
                }
            }
            if (which == ibPaths.size()) {
                throw std::invalid_argument("'" + ib->srcPath + "' is not one of the index buffers the split was given");
            }
            writeBytes(ib->fixedPath, VGComponentSplit::encodeIb(buffers.ibs[which]));
        }

        if (position != nullptr) {
            writeBytes(position->fixedPath,
                       filterVertexBuffer(position->srcPath, split.vertexCount(), buffers.vertices, config.positionLineEdit));
        }

        if (texcoord != nullptr) {
            writeBytes(texcoord->fixedPath,
                       filterVertexBuffer(texcoord->srcPath, split.vertexCount(), buffers.vertices, config.texcoordLineEdit));
        }

        return true;
    }


    VGSplitGroupResource::VGSplitGroupResource(std::string name, std::unordered_map<std::string, std::unique_ptr<IniResource>> resources,
                                                VGSplitGroupConfig config, std::function<bool(IniGroupedResource&)> fixFunc, bool isBuilt):
        RemapIniGroupedResource(std::move(name), std::move(resources), std::move(fixFunc), isBuilt), config(std::move(config)) {}


    bool VGSplitGroupResource::_fix() {
        return fixVGSplitGroup(*this, config, logger.get());
    }
}
