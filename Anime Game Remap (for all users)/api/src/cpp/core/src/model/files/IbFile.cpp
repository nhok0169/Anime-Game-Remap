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

#include "AGRemapCore/model/files/IbFile.h"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <utility>

#include "AGRemapCore/model/buffers/BufInt.h"


namespace AGRemapCore {

    const std::string IbFile::TriangleBufElementKey = "Triangle";
    const std::size_t IbFile::VerticesPerTriangle = 3;

    std::vector<std::unique_ptr<BufElementType>> IbFile::defaultElements(std::size_t bytesPerIndex) {
        const bool narrow = (bytesPerIndex == 2);

        std::vector<std::unique_ptr<BufDataType>> dataTypes;
        for (std::size_t i = 0; i < VerticesPerTriangle; ++i) {
            dataTypes.push_back(narrow ? std::make_unique<BufUnSignedInt>("UnsignedInt16", 2)
                                       : std::make_unique<BufUnSignedInt>());
        }

        std::vector<std::unique_ptr<BufElementType>> result;
        result.push_back(std::make_unique<BufElementType>(TriangleBufElementKey,
                                                          narrow ? "R16G16B16_UINT" : "R32G32B32_UINT",
                                                          std::move(dataTypes)));

        return result;
    }

    IbFile::IbFile(BinarySrc src, std::size_t bytesPerIndex):
        BufFile(std::move(src), defaultElements(bytesPerIndex), "Ib") {}

    std::size_t IbFile::bytesPerIndexOf(const std::string& dxgiFormat) {
        return (dxgiFormat.find("R16") != std::string::npos) ? 2 : 4;
    }

    std::size_t IbFile::getTriangleCount() const {
        std::size_t bytesPerLine = getBytesPerLine();
        if (bytesPerLine == 0) {
            return 0;
        }

        return getData().size() / bytesPerLine;
    }

    std::size_t IbFile::getIndexCount() const {
        return getTriangleCount() * VerticesPerTriangle;
    }

    std::vector<long long> IbFile::getIndices() const {
        std::vector<long long> indices;
        indices.reserve(getIndexCount());
        for (const std::vector<long long>& triangle : rows(TriangleBufElementKey, VerticesPerTriangle)) {
            indices.insert(indices.end(), triangle.begin(), triangle.end());
        }

        return indices;
    }

    ByteVec IbFile::windingReversed(std::size_t firstIndex, std::size_t indexCount) const {
        const std::size_t indexBytes = getBytesPerLine() / VerticesPerTriangle;
        const ByteVec& data = getData();
        if (indexBytes == 0 || (firstIndex + indexCount) * indexBytes > data.size()) {
            throw std::out_of_range("the span of " + std::to_string(indexCount) + " indices at "
                                    + std::to_string(firstIndex) + " runs past the end of the index buffer");
        }

        // The corners' bytes are copied rather than decoded, so the width is the file's own
        ByteVec out(indexCount * indexBytes, 0);
        const auto width = static_cast<std::ptrdiff_t>(indexBytes);
        for (std::size_t t = 0; t + 2 < indexCount; t += VerticesPerTriangle) {
            const auto from = data.begin() + static_cast<std::ptrdiff_t>((firstIndex + t) * indexBytes);
            const auto to = out.begin() + static_cast<std::ptrdiff_t>(t * indexBytes);
            std::copy(from, from + width, to);
            std::copy(from + 2 * width, from + 3 * width, to + width);
            std::copy(from + width, from + 2 * width, to + 2 * width);
        }

        return out;
    }

    std::string IbFile::makeDumpHeader(long long firstIndex) const {
        std::string result = "byte offset: 0\nfirst index: ";
        result += std::to_string(firstIndex);
        result += "\nindex count: ";
        result += std::to_string(getIndexCount());

        // The format a real dump names here is always the 16 bit one, even though the .ib file
        // itself stores 32 bit indices -- matching what 3dmigoto writes rather than what it reads
        result += "\ntopology: trianglelist\nformat: DXGI_FORMAT_R16_UINT\n";

        return result;
    }

    std::string IbFile::getDumpStr(long long firstIndex) const {
        std::string result = makeDumpHeader(firstIndex);
        result += '\n';
        result += getFlatDumpStr();

        return result;
    }

    void IbFile::readDumpStr(const std::string& text) {
        readFlatDumpStr(text);
    }
}
