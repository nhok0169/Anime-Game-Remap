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

#include "AGRemapCore/model/buffers/WWMIShapeKeys.h"

#include <cstring>
#include <set>
#include <stdexcept>
#include <utility>

#include "AGRemapCore/model/files/BinaryFile.h"
#include "AGRemapCore/tools/files/FileService.h"


namespace AGRemapCore {

    namespace {
        ByteVec readWhole(const std::string& path, const std::string& what) {
            try {
                return BinaryFile(path).read();
            } catch (const std::exception&) {
                throw std::runtime_error("cannot read " + what + " at " + path);
            }
        }

        void writeWhole(const std::string& path, const ByteVec& bytes, const std::string& what) {
            FileService::makeFolderFor(path);
            if (!BinaryFile::write(path, bytes)) {
                throw std::runtime_error("cannot write " + what + " to " + path);
            }

            if (FileService::fileSize(path) != static_cast<std::uintmax_t>(bytes.size())) {
                throw std::runtime_error("wrote " + what + " to " + path + " and it is not there");
            }
        }
    }


    WWMIShapeKeys::Layout WWMIShapeKeys::layoutOf(const Files& files) {
        Layout layout;
        const ByteVec firsts = readWhole(files.offsets, "this mod's shape-key offsets");
        const auto idBytes = FileService::fileSize(files.vertexIds).value_or(0);
        const auto offsetBytes = FileService::fileSize(files.vertexOffsets).value_or(0);
        if (firsts.size() % 4 != 0 || idBytes % 4 != 0 || idBytes == 0) {
            return layout;
        }

        layout.entries = static_cast<std::size_t>(idBytes / 4);
        if (offsetBytes % layout.entries != 0) {
            return layout;
        }

        layout.offsetWidth = static_cast<std::size_t>(offsetBytes / layout.entries);
        for (std::size_t i = 0; i + 3 < firsts.size(); i += 4) {
            std::uint32_t v = 0;
            std::memcpy(&v, firsts.data() + i, 4);
            layout.firsts.push_back(v);
        }
        return layout;
    }

    std::string WWMIShapeKeys::checkReorder(const Files& files, const std::vector<long long>& order) {
        try {
            const Layout layout = layoutOf(files);
            if (layout.firsts.empty() || layout.offsetWidth == 0) {
                return "its shape-key buffers are not the layout WWMI Tools writes";
            }
            if (layout.firsts.size() < order.size()) {
                return "it has fewer shape-key slots than the target";
            }
            for (std::size_t i = 0; i < layout.firsts.size(); ++i) {
                if (layout.firsts[i] > layout.entries || (i > 0 && layout.firsts[i] < layout.firsts[i - 1])) {
                    return "its shape-key offsets are out of order";
                }
            }
            for (long long key : order) {
                if (key >= static_cast<long long>(layout.firsts.size())) {
                    return "the shape-key order names a slot it does not have";
                }
            }
        } catch (const std::exception& exception) {
            return exception.what();
        }
        return "";
    }

    void WWMIShapeKeys::reorder(const Files& files, const std::vector<long long>& order, const Files& out) {
        const Layout layout = layoutOf(files);
        const ByteVec ids = readWhole(files.vertexIds, "this mod's shape-key vertex ids");
        const ByteVec offsets = readWhole(files.vertexOffsets, "this mod's shape-key vertex offsets");

        const std::size_t slots = layout.firsts.size();
        auto range = [&layout, slots](std::size_t key) {
            const std::size_t first = layout.firsts[key];
            const std::size_t end = key + 1 < slots ? layout.firsts[key + 1] : layout.entries;
            return std::make_pair(first, end);
        };

        ByteVec outIds;
        ByteVec outOffsets;
        std::vector<std::uint32_t> outFirsts;
        outIds.reserve(ids.size());
        outOffsets.reserve(offsets.size());
        auto append = [&](std::size_t key, bool zeroed) {
            const auto [first, end] = range(key);
            outIds.insert(outIds.end(), ids.begin() + static_cast<std::ptrdiff_t>(first * 4),
                          ids.begin() + static_cast<std::ptrdiff_t>(end * 4));
            if (zeroed) {
                outOffsets.insert(outOffsets.end(), (end - first) * layout.offsetWidth, 0);
            } else {
                outOffsets.insert(outOffsets.end(),
                                  offsets.begin() + static_cast<std::ptrdiff_t>(first * layout.offsetWidth),
                                  offsets.begin() + static_cast<std::ptrdiff_t>(end * layout.offsetWidth));
            }
        };

        const std::size_t ordered = order.size();
        std::set<std::size_t> taken;
        for (long long key : order) {
            if (key >= 0) {
                taken.insert(static_cast<std::size_t>(key));
            }
        }

        for (std::size_t slot = 0; slot < slots; ++slot) {
            outFirsts.push_back(static_cast<std::uint32_t>(outIds.size() / 4));
            if (slot < ordered) {
                const long long key = order[slot];
                if (key >= 0) {
                    append(static_cast<std::size_t>(key), false);
                }

                // the keys no slot takes, kept with nothing to move so the entry count holds
                if (slot + 1 == ordered) {
                    for (std::size_t other = 0; other < ordered; ++other) {
                        if (taken.count(other) == 0) {
                            append(other, true);
                        }
                    }
                }
            } else {
                append(slot, false);
            }
        }

        if (outIds.size() != ids.size() || outOffsets.size() != offsets.size()) {
            throw std::runtime_error("reordering this mod's shape keys changed their entry count ("
                                     + std::to_string(ids.size() / 4) + " -> "
                                     + std::to_string(outIds.size() / 4) + ")");
        }

        ByteVec firstBytes(outFirsts.size() * 4);
        std::memcpy(firstBytes.data(), outFirsts.data(), firstBytes.size());
        writeWhole(out.offsets, firstBytes, "the reordered shape-key offsets");
        writeWhole(out.vertexIds, outIds, "the reordered shape-key vertex ids");
        writeWhole(out.vertexOffsets, outOffsets, "the reordered shape-key vertex offsets");
    }
}
