#include "AGRemapCore/model/files/IbFile.h"

#include <utility>

#include "AGRemapCore/model/buffers/BufInt.h"


namespace AGRemapCore {

    const std::string IbFile::TriangleBufElementKey = "Triangle";
    const std::size_t IbFile::VerticesPerTriangle = 3;

    std::vector<std::unique_ptr<BufElementType>> IbFile::defaultElements() {
        std::vector<std::unique_ptr<BufDataType>> dataTypes;
        for (std::size_t i = 0; i < VerticesPerTriangle; ++i) {
            dataTypes.push_back(std::make_unique<BufUnSignedInt>());
        }

        std::vector<std::unique_ptr<BufElementType>> result;
        result.push_back(std::make_unique<BufElementType>(TriangleBufElementKey, "R32G32B32_UINT", std::move(dataTypes)));

        return result;
    }

    IbFile::IbFile(BinarySrc src): BufFile(std::move(src), defaultElements(), "Ib") {}

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
