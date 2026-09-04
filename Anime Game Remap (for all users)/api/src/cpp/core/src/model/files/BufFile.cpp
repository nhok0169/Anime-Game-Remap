#include "AGRemapCore/model/files/BufFile.h"

#include <algorithm>
#include <charconv>
#include <cmath>
#include <cstdlib>
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

    namespace {

        // How many significant digits 3dmigoto prints -- FLT_DECIMAL_DIG, ie. exactly enough to
        // round-trip a 32 bit float, which is what every value in a .buf file ultimately is
        constexpr int DumpFloatPrecision = 9;

        // Formats a value the way 3dmigoto -- the C program that writes real frame analysis dumps --
        // does: printf's "%.9g".
        //
        // Deliberately *not* Python's str()/repr(). The pure-Python converter this replaced used
        // repr, which is shortest-round-trip of a double and always keeps a ".0"; a genuine dump
        // instead reads "0.0405528881", "-1" and "1, 0, 0, 0". Matching the real thing means 9
        // significant digits, general format (so trailing zeros are dropped and no ".0" is forced),
        // and an exponent once it gets small or large enough.
        //
        // The value is rounded through a 32 bit float first, because 3dmigoto holds it as one. For
        // anything decoded straight out of a float32 that is a no-op, but it matters for a UNORM
        // colour, whose decode divides by 255 in double: 128/255 prints as 0.501960784 from a
        // double and 0.501960814 from the float 3dmigoto actually has.
        void appendDoubleAsDumpStr(std::string& out, double value) {
            if (std::isnan(value)) {
                out += "nan";
                return;
            }

            if (std::isinf(value)) {
                out += (value < 0) ? "-inf" : "inf";
                return;
            }

            char buffer[64];
            double rounded = static_cast<double>(static_cast<float>(value));
            auto converted = std::to_chars(buffer, buffer + sizeof(buffer), rounded,
                                           std::chars_format::general, DumpFloatPrecision);

            out.append(buffer, static_cast<std::size_t>(converted.ptr - buffer));
        }

        // One decoded value as a dump renders it -- an integer prints plainly, a floating point
        // value goes through appendDoubleAsDumpStr above.
        void appendBufValueAsDumpStr(std::string& out, const BufValue& value) {
            std::visit([&out](auto&& raw) {
                using T = std::decay_t<decltype(raw)>;
                if constexpr (std::is_same_v<T, double>) {
                    appendDoubleAsDumpStr(out, raw);
                } else {
                    char buffer[32];
                    auto converted = std::to_chars(buffer, buffer + sizeof(buffer), raw);
                    out.append(buffer, static_cast<std::size_t>(converted.ptr - buffer));
                }
            }, value);
        }

        // Python's f"{value:03d}" -- zero padded to 3 digits, and left alone once past that
        void appendPaddedOffset(std::string& out, std::size_t offset) {
            char buffer[32];
            auto converted = std::to_chars(buffer, buffer + sizeof(buffer), offset);
            std::size_t length = static_cast<std::size_t>(converted.ptr - buffer);

            for (std::size_t i = length; i < 3; ++i) {
                out += '0';
            }

            out.append(buffer, length);
        }

        // Writes one decoded value into whichever alternative the column holds. The two can
        // disagree -- eg. a caller handing encodeAll a column of floats for an integer data type --
        // in which case the value is converted, matching how BufBaseInt::encode already coerces
        // whichever BufValue alternative it is given.
        void storeColumnValue(BufFile::BufColumn& column, std::size_t ind, const BufValue& value) {
            std::visit([&value, ind](auto& values) {
                using T = typename std::decay_t<decltype(values)>::value_type;
                values[ind] = std::visit([](auto&& raw) { return static_cast<T>(raw); }, value);
            }, column);
        }

        // The inverse of storeColumnValue -- reads one line's value out of a column, as the
        // BufValue alternative matching that column's own type. Out-of-range lines read as 0, so a
        // short column (one the caller filled for fewer lines than the longest) still encodes.
        BufValue loadColumnValue(const BufFile::BufColumn& column, std::size_t ind) {
            return std::visit([ind](const auto& values) -> BufValue {
                if (ind >= values.size()) {
                    return BufValue(static_cast<long long>(0));
                }
                return BufValue(values[ind]);
            }, column);
        }

    }

    std::vector<BufFile::BufColumnData> BufFile::decodeAll() const {
        std::vector<BufColumnData> result;
        if (bytesPerLine_ == 0) {
            return result;
        }

        std::size_t lineCount = data_.size() / bytesPerLine_;

        // The column layout is a property of the elements, not of the bytes, so it is settled once
        // here rather than re-derived per line: walking the elements in declaration order gives
        // exactly the order decodeLine lays the bytes out in, which is what lets the decoding loop
        // below address its columns positionally instead of hashing an element key per value.
        ByteVec probe;
        for (std::size_t i = 0; i < elements_.size(); ++i) {
            const auto& dataTypes = elements_[i]->getDataTypes();
            for (std::size_t j = 0; j < dataTypes.size(); ++j) {
                BufColumnData column;
                column.elementKey = elementKeys_[i];
                column.valueInd = j;

                // Which alternative this column uses comes from decoding a zero-filled probe --
                // a BufDataType always returns the same alternative regardless of input, and this
                // avoids needing a separate "what type are you" virtual on BufDataType itself.
                probe.assign(dataTypes[j]->getSize(), 0);
                switch (dataTypes[j]->decode(probe).index()) {
                    case 0:
                        column.values = std::vector<long long>(lineCount);
                        break;
                    case 1:
                        column.values = std::vector<unsigned long long>(lineCount);
                        break;
                    default:
                        column.values = std::vector<double>(lineCount);
                        break;
                }

                result.push_back(std::move(column));
            }
        }

        // One reused scratch buffer for every value's bytes, rather than a fresh allocation per
        // scalar -- 'assign' keeps the capacity it already grew to.
        ByteVec slice;
        for (std::size_t lineInd = 0; lineInd < lineCount; ++lineInd) {
            std::size_t byteInd = lineInd * bytesPerLine_;
            std::size_t colInd = 0;

            for (const auto& element : elements_) {
                for (const auto& dataType : element->getDataTypes()) {
                    std::size_t size = dataType->getSize();

                    // Clamped the same way decodeLine clamps its own slices -- see its comment on
                    // why a too-short line must not form a past-the-end iterator.
                    std::size_t clampedStart = std::min(byteInd, data_.size());
                    std::size_t clampedEnd = std::min(byteInd + size, data_.size());
                    slice.assign(data_.begin() + static_cast<std::ptrdiff_t>(clampedStart),
                                 data_.begin() + static_cast<std::ptrdiff_t>(clampedEnd));

                    storeColumnValue(result[colInd].values, lineInd, dataType->decode(slice));

                    byteInd += size;
                    ++colInd;
                }
            }
        }

        return result;
    }

    void BufFile::merge(const std::vector<const BufFile*>& bufFiles) {
        std::vector<std::unique_ptr<BufElementType>> mergedElements;
        std::size_t lineCount = 0;
        bool hasLineCount = false;

        for (const BufFile* bufFile : bufFiles) {
            if (bufFile == nullptr) {
                continue;
            }

            // Deep-copied rather than moved: a source keeps its own elements and stays usable
            // afterwards, the same shareable-value contract BufElementType's copy constructor
            // documents.
            for (const auto& element : bufFile->elements_) {
                mergedElements.push_back(std::make_unique<BufElementType>(*element));
            }

            std::size_t fileLineCount = (bufFile->bytesPerLine_ == 0) ? 0 : bufFile->data_.size() / bufFile->bytesPerLine_;
            lineCount = hasLineCount ? std::min(lineCount, fileLineCount) : fileLineCount;
            hasLineCount = true;
        }

        std::size_t mergedBytesPerLine = 0;
        for (const auto& element : mergedElements) {
            mergedBytesPerLine += element->getSize();
        }

        ByteVec merged;
        merged.reserve(lineCount * mergedBytesPerLine);

        for (std::size_t lineInd = 0; lineInd < lineCount; ++lineInd) {
            for (const BufFile* bufFile : bufFiles) {
                if (bufFile == nullptr) {
                    continue;
                }

                std::size_t startInd = lineInd * bufFile->bytesPerLine_;
                merged.insert(merged.end(),
                              bufFile->data_.begin() + static_cast<std::ptrdiff_t>(startInd),
                              bufFile->data_.begin() + static_cast<std::ptrdiff_t>(startInd + bufFile->bytesPerLine_));
            }
        }

        // Elements first: read() validates the new bytes against #getBytesPerLine, which only
        // setElements recomputes.
        setElements(std::move(mergedElements));
        setSrc(merged);
        read();
    }

    std::string BufFile::getDumpStr(const std::string& prefix) const {
        std::string result;
        if (bytesPerLine_ == 0) {
            return result;
        }

        std::size_t lineCount = data_.size() / bytesPerLine_;

        // A mid-sized mod's dump runs to tens of megabytes, and growing that by repeated += means
        // recopying the whole thing on every doubling. The estimate only has to be in the right
        // ballpark to make those recopies rare.
        std::size_t valueCount = 0;
        for (const auto& element : elements_) {
            valueCount += element->getDataTypes().size();
        }

        result.reserve(lineCount * ((elements_.size() * (prefix.size() + 24)) + (valueCount * 14) + 1));

        ByteVec slice;
        char lineIndBuffer[32];

        for (std::size_t lineInd = 0; lineInd < lineCount; ++lineInd) {
            std::size_t byteInd = lineInd * bytesPerLine_;
            std::size_t offset = 0;

            auto lineIndConverted = std::to_chars(lineIndBuffer, lineIndBuffer + sizeof(lineIndBuffer), lineInd);
            std::size_t lineIndLength = static_cast<std::size_t>(lineIndConverted.ptr - lineIndBuffer);

            for (std::size_t i = 0; i < elements_.size(); ++i) {
                const auto& element = elements_[i];

                result += prefix;
                result += '[';
                result.append(lineIndBuffer, lineIndLength);
                result += "]+";
                appendPaddedOffset(result, offset);
                result += ' ';
                result += elementKeys_[i];
                result += ": ";

                const auto& dataTypes = element->getDataTypes();
                for (std::size_t j = 0; j < dataTypes.size(); ++j) {
                    if (j > 0) {
                        result += ", ";
                    }

                    std::size_t size = dataTypes[j]->getSize();
                    std::size_t clampedStart = std::min(byteInd, data_.size());
                    std::size_t clampedEnd = std::min(byteInd + size, data_.size());
                    slice.assign(data_.begin() + static_cast<std::ptrdiff_t>(clampedStart),
                                 data_.begin() + static_cast<std::ptrdiff_t>(clampedEnd));

                    appendBufValueAsDumpStr(result, dataTypes[j]->decode(slice));
                    byteInd += size;
                }

                result += '\n';
                offset += element->getSize();
            }

            // Blank line *between* lines, not after the last one -- matching what a real 3dmigoto
            // dump does (its file ends right after the final entry)
            if (lineInd + 1 < lineCount) {
                result += '\n';
            }
        }

        return result;
    }

    std::string BufFile::getFlatDumpStr(const std::string& valueSep) const {
        std::string result;
        if (bytesPerLine_ == 0) {
            return result;
        }

        std::size_t lineCount = data_.size() / bytesPerLine_;

        std::size_t valueCount = 0;
        for (const auto& element : elements_) {
            valueCount += element->getDataTypes().size();
        }

        // Same reasoning as getDumpStr's own reserve
        result.reserve(lineCount * ((valueCount * (14 + valueSep.size())) + 1));

        ByteVec slice;

        for (std::size_t lineInd = 0; lineInd < lineCount; ++lineInd) {
            std::size_t byteInd = lineInd * bytesPerLine_;
            bool isFirstValue = true;

            for (const auto& element : elements_) {
                for (const auto& dataType : element->getDataTypes()) {
                    if (!isFirstValue) {
                        result += valueSep;
                    }
                    isFirstValue = false;

                    std::size_t size = dataType->getSize();
                    std::size_t clampedStart = std::min(byteInd, data_.size());
                    std::size_t clampedEnd = std::min(byteInd + size, data_.size());
                    slice.assign(data_.begin() + static_cast<std::ptrdiff_t>(clampedStart),
                                 data_.begin() + static_cast<std::ptrdiff_t>(clampedEnd));

                    appendBufValueAsDumpStr(result, dataType->decode(slice));
                    byteInd += size;
                }
            }

            result += '\n';
        }

        return result;
    }

    namespace {

        // Which BufValue alternative each of a file's data types uses, flattened across its
        // elements in declaration order -- 0 signed, 1 unsigned, 2 floating point. Settled from a
        // zero-filled probe decode for the same reason decodeAll does it that way: the alternative
        // is a property of the data type, not of any particular bytes.
        std::vector<int> getDataTypeKinds(const std::vector<std::unique_ptr<BufElementType>>& elements) {
            std::vector<int> result;
            ByteVec probe;

            for (const auto& element : elements) {
                for (const auto& dataType : element->getDataTypes()) {
                    probe.assign(dataType->getSize(), 0);
                    result.push_back(static_cast<int>(dataType->decode(probe).index()));
                }
            }

            return result;
        }

        // Every data type of a file, flattened across its elements in declaration order
        std::vector<const BufDataType*> getFlatDataTypes(const std::vector<std::unique_ptr<BufElementType>>& elements) {
            std::vector<const BufDataType*> result;

            for (const auto& element : elements) {
                for (const auto& dataType : element->getDataTypes()) {
                    result.push_back(dataType.get());
                }
            }

            return result;
        }

        // One text value as the BufValue alternative its data type expects. Parsed through
        // from_chars rather than strtod/strtoll, since that needs no null terminator (so the text
        // can stay a view into the dump) and raises nothing on a malformed value -- anything
        // unparseable is simply left at 0 rather than failing the whole read.
        BufValue parseDumpValue(const char* begin, const char* end, int kind) {
            // leading/trailing whitespace is not from_chars' job
            while (begin != end && (*begin == ' ' || *begin == '\t' || *begin == '\r')) {
                ++begin;
            }

            while (end != begin && (*(end - 1) == ' ' || *(end - 1) == '\t' || *(end - 1) == '\r')) {
                --end;
            }

            if (kind == 0) {
                long long value = 0;
                std::from_chars(begin, end, value);
                return BufValue(value);
            }

            if (kind == 1) {
                unsigned long long value = 0;
                std::from_chars(begin, end, value);
                return BufValue(value);
            }

            double value = 0.0;
            std::from_chars(begin, end, value);
            return BufValue(value);
        }

        BufValue zeroDumpValue(int kind) {
            if (kind == 0) {
                return BufValue(static_cast<long long>(0));
            }

            if (kind == 1) {
                return BufValue(static_cast<unsigned long long>(0));
            }

            return BufValue(0.0);
        }

    }

    void BufFile::readDumpStr(const std::string& text) {
        std::vector<const BufDataType*> dataTypes = getFlatDataTypes(elements_);
        std::vector<int> kinds = getDataTypeKinds(elements_);

        // A whole dump file is accepted, not just the data section -- everything through the
        // 'vertex-data:' marker a real header ends with is skipped.
        std::size_t start = 0;
        std::size_t markerInd = text.find("vertex-data:");
        if (markerInd != std::string::npos) {
            std::size_t lineEnd = text.find('\n', markerInd);
            start = (lineEnd == std::string::npos) ? text.size() : lineEnd + 1;
        }

        ByteVec encoded;
        std::vector<std::pair<const char*, const char*>> blockValues;

        // Encodes one blank-line-separated block (one line of the file) against every data type,
        // zero-filling a short block so the stride never breaks
        auto encodeBlock = [&]() {
            if (blockValues.empty()) {
                return;
            }

            for (std::size_t i = 0; i < dataTypes.size(); ++i) {
                BufValue value = (i < blockValues.size())
                    ? parseDumpValue(blockValues[i].first, blockValues[i].second, kinds[i])
                    : zeroDumpValue(kinds[i]);

                ByteVec bytes = dataTypes[i]->encode(value);
                encoded.insert(encoded.end(), bytes.begin(), bytes.end());
            }

            blockValues.clear();
        };

        std::size_t lineStart = start;
        while (lineStart <= text.size()) {
            std::size_t lineEnd = text.find('\n', lineStart);
            if (lineEnd == std::string::npos) {
                lineEnd = text.size();
            }

            const char* begin = text.data() + lineStart;
            const char* end = text.data() + lineEnd;

            const char* trimmed = begin;
            while (trimmed != end && (*trimmed == ' ' || *trimmed == '\t' || *trimmed == '\r')) {
                ++trimmed;
            }

            if (trimmed == end) {
                encodeBlock();
            } else {
                // The values are whatever follows the last ':', so the "prefix[i]+offset key:" part
                // of a real entry is skipped without having to match it
                const char* valuesBegin = begin;
                for (const char* cursor = end; cursor != begin; --cursor) {
                    if (*(cursor - 1) == ':') {
                        valuesBegin = cursor;
                        break;
                    }
                }

                const char* valueStart = valuesBegin;
                for (const char* cursor = valuesBegin; cursor <= end; ++cursor) {
                    if (cursor == end || *cursor == ',') {
                        blockValues.emplace_back(valueStart, cursor);
                        valueStart = cursor + 1;
                    }
                }
            }

            if (lineEnd == text.size()) {
                break;
            }

            lineStart = lineEnd + 1;
        }

        encodeBlock();

        setSrc(encoded);
        read();
    }

    void BufFile::readFlatDumpStr(const std::string& text, const std::string& valueSep) {
        std::vector<const BufDataType*> dataTypes = getFlatDataTypes(elements_);
        std::vector<int> kinds = getDataTypeKinds(elements_);

        ByteVec encoded;
        std::vector<std::pair<const char*, const char*>> lineValues;

        std::size_t lineStart = 0;
        while (lineStart <= text.size()) {
            std::size_t lineEnd = text.find('\n', lineStart);
            if (lineEnd == std::string::npos) {
                lineEnd = text.size();
            }

            const char* begin = text.data() + lineStart;
            const char* end = text.data() + lineEnd;

            const char* trimmed = begin;
            while (trimmed != end && (*trimmed == ' ' || *trimmed == '\t' || *trimmed == '\r')) {
                ++trimmed;
            }

            // A header line is told apart by carrying a ':' -- every line of a .ib dump's header
            // has one ("byte offset: 0", "topology: trianglelist") and none of its data lines do
            bool isHeaderLine = (std::find(trimmed, end, ':') != end);

            if (trimmed != end && !isHeaderLine) {
                lineValues.clear();

                const char* valueStart = trimmed;
                const char* cursor = trimmed;
                while (cursor <= end) {
                    bool isSeparator = (cursor != end) && !valueSep.empty() &&
                                       (static_cast<std::size_t>(end - cursor) >= valueSep.size()) &&
                                       (std::equal(valueSep.begin(), valueSep.end(), cursor));

                    if (cursor == end || isSeparator) {
                        if (cursor != valueStart) {
                            lineValues.emplace_back(valueStart, cursor);
                        }

                        if (cursor == end) {
                            break;
                        }

                        cursor += valueSep.size();
                        valueStart = cursor;
                        continue;
                    }

                    ++cursor;
                }

                for (std::size_t i = 0; i < dataTypes.size(); ++i) {
                    BufValue value = (i < lineValues.size())
                        ? parseDumpValue(lineValues[i].first, lineValues[i].second, kinds[i])
                        : zeroDumpValue(kinds[i]);

                    ByteVec bytes = dataTypes[i]->encode(value);
                    encoded.insert(encoded.end(), bytes.begin(), bytes.end());
                }
            }

            if (lineEnd == text.size()) {
                break;
            }

            lineStart = lineEnd + 1;
        }

        setSrc(encoded);
        read();
    }

    void BufFile::encodeAll(const std::vector<BufColumnData>& columns) {
        std::size_t lineCount = 0;
        for (const BufColumnData& column : columns) {
            std::size_t size = std::visit([](const auto& values) { return values.size(); }, column.values);
            lineCount = std::max(lineCount, size);
        }

        // Every (element key, value index) is resolved to its column once, up front, so the
        // encoding loop below addresses them positionally -- the same reason decodeAll builds its
        // columns in declaration order. A data type with no matching column stays null and encodes
        // as 0 rather than failing the whole call.
        std::vector<const BufColumnData*> ordered;
        for (std::size_t i = 0; i < elements_.size(); ++i) {
            const auto& dataTypes = elements_[i]->getDataTypes();
            for (std::size_t j = 0; j < dataTypes.size(); ++j) {
                const BufColumnData* found = nullptr;
                for (const BufColumnData& column : columns) {
                    if (column.valueInd == j && column.elementKey == elementKeys_[i]) {
                        found = &column;
                        break;
                    }
                }

                ordered.push_back(found);
            }
        }

        ByteVec encoded;
        encoded.reserve(lineCount * bytesPerLine_);

        for (std::size_t lineInd = 0; lineInd < lineCount; ++lineInd) {
            std::size_t colInd = 0;

            for (const auto& element : elements_) {
                for (const auto& dataType : element->getDataTypes()) {
                    const BufColumnData* column = ordered[colInd];
                    BufValue value = (column == nullptr) ? BufValue(static_cast<long long>(0))
                                                          : loadColumnValue(column->values, lineInd);

                    ByteVec bytes = dataType->encode(value);
                    encoded.insert(encoded.end(), bytes.begin(), bytes.end());
                    ++colInd;
                }
            }
        }

        setSrc(encoded);
        read();
    }
}
