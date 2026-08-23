#include "AGRemapCore/model/files/BinaryFile.h"

#include <fstream>
#include <stdexcept>
#include <utility>


namespace AGRemapCore {
    BinaryFile::BinaryFile(BinarySrc src): src_(std::move(src)) {}

    const BinarySrc& BinaryFile::getSrc() const {
        return src_;
    }

    void BinaryFile::setSrc(BinarySrc src) {
        src_ = std::move(src);
    }

    const ByteVec& BinaryFile::getData() const {
        return data_;
    }

    ByteVec BinaryFile::read() {
        if (std::holds_alternative<std::string>(src_)) {
            const std::string& path = std::get<std::string>(src_);
            std::ifstream file(path, std::ios::binary | std::ios::ate);
            if (!file) {
                throw std::runtime_error("Unable to open file: " + path);
            }

            std::streamsize size = file.tellg();
            file.seekg(0, std::ios::beg);

            data_.resize(static_cast<std::size_t>(size));
            if (size > 0 && !file.read(reinterpret_cast<char*>(data_.data()), size)) {
                throw std::runtime_error("Unable to read file: " + path);
            }
        } else {
            data_ = std::get<ByteVec>(src_);
        }

        return data_;
    }
}
