#include "AGRemapCore/model/files/VbFile.h"

#include <cctype>
#include <unordered_map>
#include <utility>

#include "AGRemapCore/model/buffers/BufFloat.h"
#include "AGRemapCore/model/buffers/BufInt.h"
#include "AGRemapCore/model/buffers/BufUnorm.h"


namespace AGRemapCore {

    namespace {

        // Every value a labelled header line carries, in the order the lines appear -- eg. every
        // "  SemanticName: POSITION" gives back "POSITION". Scanned by hand rather than through
        // std::regex, which for a fixed label like this is all cost and no benefit.
        std::vector<std::string> findHeaderValues(const std::string& text, const std::string& label) {
            std::vector<std::string> result;
            std::size_t ind = 0;

            while ((ind = text.find(label, ind)) != std::string::npos) {
                ind += label.size();

                while (ind < text.size() && (text[ind] == ' ' || text[ind] == '\t')) {
                    ++ind;
                }

                std::size_t start = ind;
                while (ind < text.size() && !std::isspace(static_cast<unsigned char>(text[ind]))) {
                    ++ind;
                }

                result.push_back(text.substr(start, ind - start));
            }

            return result;
        }

    }

    VbFile::VbFile(BinarySrc src, std::vector<std::unique_ptr<BufElementType>> elements):
        BufFile(std::move(src), std::move(elements), "Vb") {}

    std::size_t VbFile::getVertexCount() const {
        std::size_t bytesPerLine = getBytesPerLine();
        if (bytesPerLine == 0) {
            return 0;
        }

        return getData().size() / bytesPerLine;
    }

    std::string VbFile::makeDumpHeader() const {
        std::string result = "stride: ";
        result += std::to_string(getBytesPerLine());
        result += "\nfirst vertex: 0\nvertex count: ";
        result += std::to_string(getVertexCount());
        result += "\ntopology: trianglelist\n";

        const auto& elements = getElements();
        std::size_t currentSize = 0;
        std::unordered_map<std::string, long long> currentElementInds;

        for (std::size_t i = 0; i < elements.size(); ++i) {
            const auto& element = elements[i];
            const std::string& elementName = element->getName();
            long long elementInd = currentElementInds[elementName];

            result += "element[";
            result += std::to_string(i);
            result += "]:\n  SemanticName: ";
            result += elementName;
            result += "\n  SemanticIndex: ";
            result += std::to_string(elementInd);
            result += "\n  Format: ";
            result += element->getFormatName();
            result += "\n  InputSlot: 0\n  AlignedByteOffset: ";
            result += std::to_string(currentSize);
            result += "\n  InputSlotClass: per-vertex\n  InstanceDataStepRate: 0\n";

            currentSize += element->getSize();
            currentElementInds[elementName] = elementInd + 1;
        }

        result += "\nvertex-data:\n\n";
        return result;
    }

    std::string VbFile::getDumpStr(const std::string& prefix) const {
        std::string result = makeDumpHeader();
        result += BufFile::getDumpStr(prefix);

        return result;
    }

    void VbFile::readDumpStr(const std::string& text) {
        std::vector<std::unique_ptr<BufElementType>> elements = parseDumpHeader(text);
        if (!elements.empty()) {
            setElements(std::move(elements));
        }

        BufFile::readDumpStr(text);
    }

    std::vector<std::unique_ptr<BufDataType>> VbFile::parseFormatName(const std::string& formatName) {
        std::vector<std::unique_ptr<BufDataType>> result;

        std::size_t underscoreInd = formatName.find('_');
        if (underscoreInd == std::string::npos) {
            return result;
        }

        std::string channels = formatName.substr(0, underscoreInd);
        std::string suffix = formatName.substr(underscoreInd + 1);

        // Each channel is a letter followed by its bit width, eg. "R32G32B32" is three 32 bit ones
        std::vector<int> bitWidths;
        std::size_t ind = 0;
        while (ind < channels.size()) {
            if (!std::isalpha(static_cast<unsigned char>(channels[ind]))) {
                ++ind;
                continue;
            }

            ++ind;
            int bits = 0;
            bool hasDigits = false;

            while (ind < channels.size() && std::isdigit(static_cast<unsigned char>(channels[ind]))) {
                bits = (bits * 10) + (channels[ind] - '0');
                hasDigits = true;
                ++ind;
            }

            if (hasDigits) {
                bitWidths.push_back(bits);
            }
        }

        for (int bits : bitWidths) {
            std::size_t size = static_cast<std::size_t>(bits / 8);
            std::unique_ptr<BufDataType> dataType;

            if (size == 0) {
                // fall through to the "not understood" result below
            } else if (suffix == "FLOAT" && size == 4) {
                dataType = std::make_unique<BufFloat>();
            } else if (suffix == "FLOAT" && size == 2) {
                dataType = std::make_unique<BufFloat16>();
            } else if (suffix == "SINT") {
                dataType = std::make_unique<BufSignedInt>("SignedInt" + std::to_string(bits), size);
            } else if (suffix == "UINT") {
                dataType = std::make_unique<BufUnSignedInt>("UnsignedInt" + std::to_string(bits), size);
            } else if (suffix == "UNORM") {
                dataType = std::make_unique<BufUnorm>("UNORM" + std::to_string(bits), size);
            }

            // A format only partly understood is no more useful than one not understood at all --
            // encoding against half a layout would silently corrupt every line
            if (dataType == nullptr) {
                return {};
            }

            result.push_back(std::move(dataType));
        }

        return result;
    }

    std::vector<std::unique_ptr<BufElementType>> VbFile::parseDumpHeader(const std::string& text) {
        std::vector<std::unique_ptr<BufElementType>> result;

        std::size_t headerEnd = text.find("vertex-data:");
        if (headerEnd == std::string::npos) {
            return result;
        }

        std::string header = text.substr(0, headerEnd);
        std::vector<std::string> names = findHeaderValues(header, "SemanticName:");
        std::vector<std::string> formatNames = findHeaderValues(header, "Format:");

        if (names.empty() || names.size() != formatNames.size()) {
            return result;
        }

        for (std::size_t i = 0; i < names.size(); ++i) {
            std::vector<std::unique_ptr<BufDataType>> dataTypes = parseFormatName(formatNames[i]);
            if (dataTypes.empty()) {
                return {};
            }

            result.push_back(std::make_unique<BufElementType>(names[i], formatNames[i], std::move(dataTypes)));
        }

        return result;
    }
}
