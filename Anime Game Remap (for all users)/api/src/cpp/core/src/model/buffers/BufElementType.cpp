#include "AGRemapCore/model/buffers/BufElementType.h"

#include <algorithm>
#include <utility>


namespace AGRemapCore {
    std::size_t BufElementType::computeSize(const std::vector<std::unique_ptr<BufDataType>>& dataTypes) {
        std::size_t size = 0;
        for (const auto& dataType : dataTypes) {
            size += dataType->getSize();
        }
        return size;
    }

    std::vector<std::unique_ptr<BufDataType>> BufElementType::cloneAll(const std::vector<std::unique_ptr<BufDataType>>& dataTypes) {
        std::vector<std::unique_ptr<BufDataType>> result;
        result.reserve(dataTypes.size());
        for (const auto& dataType : dataTypes) {
            result.push_back(dataType->clone());
        }
        return result;
    }

    BufElementType::BufElementType(std::string name, std::string formatName, std::vector<std::unique_ptr<BufDataType>> dataTypes):
        BufType(std::move(name)), formatName_(std::move(formatName)), dataTypes_(std::move(dataTypes)), size_(computeSize(dataTypes_)) {}

    BufElementType::BufElementType(const BufElementType& other):
        BufType(other), formatName_(other.formatName_), dataTypes_(cloneAll(other.dataTypes_)), size_(other.size_) {}

    BufElementType& BufElementType::operator=(const BufElementType& other) {
        if (this != &other) {
            BufType::operator=(other);
            formatName_ = other.formatName_;
            dataTypes_ = cloneAll(other.dataTypes_);
            size_ = other.size_;
        }
        return *this;
    }

    const std::string& BufElementType::getFormatName() const {
        return formatName_;
    }

    void BufElementType::setFormatName(std::string formatName) {
        formatName_ = std::move(formatName);
    }

    const std::vector<std::unique_ptr<BufDataType>>& BufElementType::getDataTypes() const {
        return dataTypes_;
    }

    void BufElementType::setDataTypes(std::vector<std::unique_ptr<BufDataType>> dataTypes) {
        dataTypes_ = std::move(dataTypes);
        size_ = computeSize(dataTypes_);
    }

    std::size_t BufElementType::getSize() const {
        return size_;
    }

    std::vector<BufValue> BufElementType::decode(const ByteVec& src) const {
        std::vector<BufValue> result;
        result.reserve(dataTypes_.size());

        std::size_t byteStart = 0;
        for (const auto& dataType : dataTypes_) {
            std::size_t byteEnd = byteStart + dataType->getSize();

            // Clamped for the same reason as BufFile::decodeLine's identical guard -- a
            // past-the-end iterator (rather than Python's forgiving slice semantics) is undefined
            // behaviour in C++.
            std::size_t clampedStart = std::min(byteStart, src.size());
            std::size_t clampedEnd = std::min(byteEnd, src.size());

            ByteVec slice(src.begin() + static_cast<std::ptrdiff_t>(clampedStart),
                          src.begin() + static_cast<std::ptrdiff_t>(clampedEnd));
            result.push_back(dataType->decode(slice));
            byteStart = byteEnd;
        }

        return result;
    }

    ByteVec BufElementType::encode(const std::vector<BufValue>& src) const {
        ByteVec result;
        std::size_t minLen = std::min(dataTypes_.size(), src.size());

        for (std::size_t i = 0; i < minLen; ++i) {
            ByteVec encoded = dataTypes_[i]->encode(src[i]);
            result.insert(result.end(), encoded.begin(), encoded.end());
        }

        return result;
    }
}
