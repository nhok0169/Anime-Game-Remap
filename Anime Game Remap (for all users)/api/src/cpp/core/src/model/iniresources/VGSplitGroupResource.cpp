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

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <utility>

#include "AGRemapCore/constants/IniKeywords.h"
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
                // A LINE EDIT MAY CHANGE THE STRIDE, as long as it changes every line the same way.
                // This used to throw on any size change, which made it impossible to hand a target
                // component a buffer of ITS width: a 12-byte Texcoord (COLOR + TEXCOORD) onto a slot
                // whose shader also reads TEXCOORD1 at offset 12 puts every such read into the NEXT
                // vertex's COLOR. Widening is the writer's job because nothing downstream can resize
                // a line -- see GIMIComponentFixerConfig::Component::texcoordStride.
                ByteVec edited;
                edited.reserve(data.size());
                std::size_t outStride = 0;

                for (std::size_t i = 0; i < vertexCount; ++i) {
                    ByteVec line(data.begin() + static_cast<std::ptrdiff_t>(i * stride),
                                 data.begin() + static_cast<std::ptrdiff_t>((i + 1) * stride));
                    ByteVec out = edit(line);

                    if (i == 0) {
                        outStride = out.size();
                        if (outStride == 0) {
                            throw std::invalid_argument("a line edit of '" + path + "' returned an empty line");
                        }
                        edited.reserve(vertexCount * outStride);
                    } else if (out.size() != outStride) {
                        throw std::invalid_argument("a line edit of '" + path + "' changed a line's size inconsistently: "
                                                    + std::to_string(outStride) + " bytes then " + std::to_string(out.size()));
                    }

                    edited.insert(edited.end(), out.begin(), out.end());
                }

                data = std::move(edited);
                stride = outStride;
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

        // Each source vertex's push -- see VGSplitGroupConfig::pushAway. Read off the source blend before the
        // split takes it, applied to the written lines by their source vertex.
        // A COPY: the split below takes weights and indices by move, and a push read after it read nothing --
        // the first compiled push wrote every vertex where it was (caught by its unit test, not in game).
        const auto pushWeights = config.pushAway.empty() ? decltype(weights){} : weights;
        const auto pushIndices = config.pushAway.empty() ? decltype(indices){} : indices;
        std::vector<std::array<float, 3>> pushes;
        const auto pushOf = [&](const ByteVec& positions, std::size_t stride) {
            pushes.assign(pushWeights.size(), {0.0f, 0.0f, 0.0f});
            for (std::size_t v = 0; v < pushWeights.size() && (v + 1) * stride <= positions.size(); ++v) {
                float pos[3];
                std::memcpy(pos, positions.data() + v * stride, sizeof(pos));
                for (const VGPushAway& push : config.pushAway) {
                    if ((push.side > 0 && pos[0] <= 0.0f) || (push.side < 0 && pos[0] >= 0.0f)) {
                        continue;
                    }
                    double share = 0.0;
                    for (std::size_t k = 0; k < 4; ++k) {
                        if (std::find(push.groups.begin(), push.groups.end(), pushIndices[v][k]) != push.groups.end()) {
                            share += pushWeights[v][k];
                        }
                    }
                    const float dx = pos[0] - push.from[0];
                    const float dz = pos[2] - push.from[2];
                    const float len = std::sqrt(dx * dx + dz * dz);
                    if (share <= 0.0 || len <= 1e-6f) {
                        continue;
                    }
                    const float amount = static_cast<float>(share) * push.distance / len;
                    pushes[v][0] += dx * amount;
                    pushes[v][2] += dz * amount;
                }
            }
        };

        std::vector<VGComponentSplit::Triangles> triangles;
        for (const std::string& path : ibPaths) {
            // The DECLARED width -- see VGSplitGroupConfig::ibBytesPerIndex for why it cannot be
            // worked out from the file instead.
            auto widthIt = config.ibBytesPerIndex.find(path);
            IbFile ibFile(path, widthIt == config.ibBytesPerIndex.end() ? 4 : widthIt->second);
            triangles.push_back(VGComponentSplit::readIb(ibFile));
        }

        // Which source vertices draw no outline -- see VGSplitGroupConfig::innerOutline. On the SOURCE mesh and before
        // the split takes the triangles: every object's buffer covers, so a Main layer under a Bang lock is found.
        std::vector<bool> noOutline;
        if (config.innerOutline.has_value() && texcoord != nullptr) {
            if (position == nullptr) {
                if (logger != nullptr) {
                    logger->log("No Position.buf in the group, so every inner layer keeps its outline");
                }
            } else {
                BinaryFile srcPositions(position->srcPath);
                const ByteVec src = srcPositions.read();
                const std::size_t count = std::max<std::size_t>(weights.size(), 1);
                std::vector<InnerLayerOutline::Vec3> points;
                std::vector<InnerLayerOutline::Vec3> normals;
                if (src.size() % count == 0 && src.size() / count >= 24) {
                    InnerLayerOutline::readPositions(src, src.size() / count, points, normals);
                } else if (logger != nullptr) {
                    // no normals to follow (a Position.buf is 40 bytes a vertex in every GIMI mod; this is a stub)
                    logger->log("The Position.buf has no normals, so every inner layer keeps its outline");
                }

                std::vector<const InnerLayerOutline::Triangles*> occluders;
                std::vector<const InnerLayerOutline::Triangles*> targets;
                for (std::size_t i = 0; i < triangles.size(); ++i) {
                    occluders.push_back(&triangles[i]);
                    if (config.innerOutlineIbs.empty()
                            || std::find(config.innerOutlineIbs.begin(), config.innerOutlineIbs.end(), i) != config.innerOutlineIbs.end()) {
                        targets.push_back(&triangles[i]);
                    }
                }
                noOutline = config.innerOutline->find(points, normals, occluders, targets);
            }
        }

        VGComponentSplit split(std::move(weights), std::move(indices), std::move(triangles), config.specs);

        // The mod's positions, for a mirrored layer that skips BACKED triangles -- see
        // VGComponentSpec::mirrorBackedReach. The fixer's own split reads them the same way, or the .ini's counts
        // would describe buffers these are not.
        if (split.needsGeometry(config.component)) {
            bool read = false;
            if (position != nullptr) {
                BinaryFile srcPositions(position->srcPath);
                read = split.readGeometry(srcPositions.read());
            }
            if (!read) {
                if (logger != nullptr) {
                    logger->log("No normals in the group's Position.buf, so every triangle of the inner layer is mirrored");
                }
            }
        }
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
                // A PARSER DOWNLOAD the split was not given (2026-09-22): a merged master binds an ib
                // in an `if $swapvar == 0 / else if == 1` chain with no `else`, and the parser covers
                // the fall-through with the game's buffer, referenced unconditionally -- so the collect
                // hands it to EVERY group beside the mod's own. That path is never taken, and the game's
                // indices do not address the mod's vertices anyway, so it is written EMPTY: the reference
                // resolves and the path draws nothing. Anything else missing is still an error.
                const std::string fileName = FileService::pathToStr(FileService::strToPath(ib->srcPath).filename());
                if (fileName.find(IniKeywords::RemapDL) != std::string::npos) {
                    writeBytes(ib->fixedPath, {});
                    continue;
                }
                throw std::invalid_argument("'" + ib->srcPath + "' is not one of the index buffers the split was given");
            }
            writeBytes(ib->fixedPath, VGComponentSplit::encodeIb(buffers.ibs[which]));
        }

        if (position != nullptr) {
            ByteVec lines = filterVertexBuffer(position->srcPath, split.vertexCount(), buffers.vertices, config.positionLineEdit);

            // The pushes, by each written line's source vertex -- see VGSplitGroupConfig::pushAway.
            if (!config.pushAway.empty() && !buffers.vertices.empty()) {
                BinaryFile srcPositions(position->srcPath);
                const ByteVec src = srcPositions.read();
                const std::size_t stride = lines.size() / buffers.vertices.size();
                pushOf(src, src.size() / std::max<std::size_t>(split.vertexCount(), 1));
                for (std::size_t i = 0; i < buffers.vertices.size(); ++i) {
                    const std::size_t v = buffers.vertices[i];
                    if (v >= pushes.size() || stride < 12) {
                        continue;
                    }
                    float pos[3];
                    std::memcpy(pos, lines.data() + i * stride, sizeof(pos));
                    for (std::size_t k = 0; k < 3; ++k) {
                        pos[k] += pushes[v][k];
                    }
                    std::memcpy(lines.data() + i * stride, pos, sizeof(pos));
                }
            }

            // The inner layer's copies, turned round -- see VGComponentSpec::mirroredIbs.
            if (config.mirrorLineEdit && !buffers.mirrored.empty() && !buffers.vertices.empty()) {
                const std::size_t stride = lines.size() / buffers.vertices.size();
                for (std::size_t i = 0; i < buffers.mirrored.size() && i < buffers.vertices.size(); ++i) {
                    if (!buffers.mirrored[i]) {
                        continue;
                    }
                    const auto from = lines.begin() + static_cast<std::ptrdiff_t>(i * stride);
                    const ByteVec line(from, from + static_cast<std::ptrdiff_t>(stride));
                    // Kept short of a lining behind it -- see VGComponentBuffers::mirrorLimits
                    const float limit = i < buffers.mirrorLimits.size() ? buffers.mirrorLimits[i] : -1.0f;
                    ByteVec edited = (limit >= 0.0f && limit < config.mirrorOffset)
                        ? VGComponentSplit::mirrorPositionLine(line, limit) : config.mirrorLineEdit(line);
                    if (edited.size() != stride) {
                        throw std::invalid_argument("a mirror line edit of '" + position->srcPath + "' changed a line's size");
                    }
                    std::copy(edited.begin(), edited.end(), from);
                }
            }

            writeBytes(position->fixedPath, lines);
        }

        if (texcoord != nullptr) {
            ByteVec lines = filterVertexBuffer(texcoord->srcPath, split.vertexCount(), buffers.vertices, config.texcoordLineEdit);

            // The inner layers' outline width (the vertex colour's alpha, byte 3), by each line's source vertex
            if (!noOutline.empty() && !buffers.vertices.empty()) {
                const std::size_t stride = lines.size() / buffers.vertices.size();
                for (std::size_t i = 0; i < buffers.vertices.size() && stride >= 4; ++i) {
                    const std::size_t v = buffers.vertices[i];
                    if (v < noOutline.size() && noOutline[v]) {
                        lines[i * stride + 3] = 0;
                    }
                }
            }

            writeBytes(texcoord->fixedPath, lines);
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
