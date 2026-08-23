#include "AGRemapCore/model/files/BufFile.h"

#include <fstream>
#include <stdexcept>
#include <unordered_map>
#include <utility>

#include "AGRemapCore/model/files/BufFileErrors.h"


namespace AGRemapCore {
    BufFile::BufFile(BinarySrc src, std::vector<std::unique_ptr<BufElementType>> elements, std::string fileType):
        BinaryFile(std::move(src)), fileType_(std::move(fileType)) {

        setElements(std::move(elements));

        // A virtual call made from within a base class's own constructor can never reach a more-
        // derived override (this resolves statically to BufFile::read() regardless of the actual
        // runtime type) -- fine today since neither BlendFile nor PositionFile override read(),
        // but worth knowing before adding a subclass that does (see FilteredTokenizer.cpp's own
        // comment on the same C++ constructor-dispatch limitation, hit for a different class).
        read();
    }

    const std::string& BufFile::getFileType() const {
        return fileType_;
    }

    void BufFile::setFileType(std::string fileType) {
        fileType_ = std::move(fileType);
    }

    const std::vector<std::unique_ptr<BufElementType>>& BufFile::getElements() const {
        return elements_;
    }

    void BufFile::setElements(std::vector<std::unique_ptr<BufElementType>> elements) {
        elements_ = std::move(elements);
        elementKeys_.clear();
        elementKeys_.reserve(elements_.size());
        bytesPerLine_ = 0;

        std::unordered_map<std::string, long long> elementsInd;
        for (const auto& element : elements_) {
            const std::string& elementName = element->getName();
            std::string elementKey = elementName;

            auto it = elementsInd.find(elementName);
            if (it == elementsInd.end()) {
                elementsInd[elementName] = 0;
            } else {
                ++(it->second);
                elementKey += std::to_string(it->second);
            }

            elementKeys_.push_back(std::move(elementKey));
            bytesPerLine_ += element->getSize();
        }
    }

    std::size_t BufFile::getBytesPerLine() const {
        return bytesPerLine_;
    }

    bool BufFile::isValid() const {
        // A per-line size of 0 (no elements) would be a division-by-zero in the pure-Python
        // original (int % 0 raises ZeroDivisionError there too) -- treated here as "valid only if
        // there's also no data", avoiding undefined behaviour from a literal '% 0' rather than
        // silently choosing some other interpretation of an input nothing in this codebase
        // actually constructs.
        if (bytesPerLine_ == 0) {
            return data_.empty();
        }
        return data_.size() % bytesPerLine_ == 0;
    }

    ByteVec BufFile::read() {
        BinaryFile::read();

        if (!isValid()) {
            if (std::holds_alternative<std::string>(src_)) {
                throw BufFileNotRecognized(std::get<std::string>(src_), fileType_);
            }
            throw BadBufData(fileType_);
        }

        return data_;
    }

    BufLineData BufFile::decodeLine(const ByteVec& src) const {
        BufLineData result;

        std::size_t startInd = 0;
        for (std::size_t i = 0; i < elements_.size(); ++i) {
            const auto& element = elements_[i];
            std::size_t elementSize = element->getSize();
            std::size_t endInd = startInd + elementSize;

            // Clamped (rather than a direct src.begin() + startInd) so a too-short 'src' behaves
            // like Python's forgiving slice semantics (src[startInd:endInd] on a short bytes
            // object just returns fewer/no bytes) instead of forming a past-the-end iterator,
            // which -- unlike Python -- is undefined behaviour in C++.
            std::size_t clampedStart = std::min(startInd, src.size());
            std::size_t clampedEnd = std::min(endInd, src.size());

            ByteVec slice(src.begin() + static_cast<std::ptrdiff_t>(clampedStart),
                          src.begin() + static_cast<std::ptrdiff_t>(clampedEnd));
            result[elementKeys_[i]] = element->decode(slice);
            startInd = endInd;
        }

        return result;
    }

    ByteVec BufFile::encodeLine(const BufLineData& src) const {
        ByteVec result;

        for (std::size_t i = 0; i < elements_.size(); ++i) {
            auto it = src.find(elementKeys_[i]);
            if (it == src.end()) {
                continue;
            }

            ByteVec encoded = elements_[i]->encode(it->second);
            result.insert(result.end(), encoded.begin(), encoded.end());
        }

        return result;
    }

    BufFile::FixResult BufFile::fix(const std::optional<std::string>& fixedFile, const std::vector<Filter>& filters) {
        ByteVec result;
        std::size_t dataLen = data_.size();

        for (std::size_t i = 0; i < dataLen; i += bytesPerLine_) {
            std::size_t lineEnd = std::min(i + bytesPerLine_, dataLen);
            ByteVec line(data_.begin() + static_cast<std::ptrdiff_t>(i), data_.begin() + static_cast<std::ptrdiff_t>(lineEnd));

            BufLineData decodedValues = decodeLine(line);

            // Deliberately a double, not an integer division -- matches the pure-Python
            // original's 'i / self._bytesPerLine' (true division), see #Filter's own doc comment.
            double lineInd = static_cast<double>(i) / static_cast<double>(bytesPerLine_);

            for (const Filter& filter : filters) {
                decodedValues = filter(decodedValues, static_cast<long long>(i), lineInd, static_cast<long long>(bytesPerLine_));
            }

            ByteVec encoded = encodeLine(decodedValues);
            result.insert(result.end(), encoded.begin(), encoded.end());
        }

        if (fixedFile.has_value()) {
            std::ofstream file(*fixedFile, std::ios::binary);
            if (!file) {
                throw std::runtime_error("Unable to open file for writing: " + *fixedFile);
            }
            if (!result.empty()) {
                file.write(reinterpret_cast<const char*>(result.data()), static_cast<std::streamsize>(result.size()));
            }
            return *fixedFile;
        }

        return result;
    }
}
